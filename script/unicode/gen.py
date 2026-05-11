# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
from concurrent.futures import ProcessPoolExecutor
from dataclasses import dataclass
import datetime
import argparse
import io
from itertools import chain
from operator import attrgetter
import pathlib
import re
import sys
import time
from typing import IO, Self, cast

type Codepoint = int
type Bitfield = int


def unify_ranges(ranges: list[range]) -> list[range]:
    """
    Merge overlapping or adjacent ranges into a unified list of non-overlapping ranges.

    This function takes a list of potentially overlapping ranges and combines them into
    a minimal set of non-overlapping ranges sorted by start position. Useful for combining
    multiple codepoint ranges that may have gaps or overlaps.

    Args:
        ranges: A list of range objects to merge.

    Returns:
        A sorted list of non-overlapping merged ranges. Empty list if input is empty.
    """
    if not ranges:
        return []

    ranges = sorted(ranges, key=attrgetter("start"))
    merged = []
    curr = ranges[0]

    for i in range(1, len(ranges)):
        nx = ranges[i]
        if nx.start <= curr.stop:
            # Ranges are adjacent or overlapping
            
            if nx.stop > curr.stop:
                curr = range(curr.start, nx.stop)
        else:
            merged.append(curr)
            curr = nx

    merged.append(curr)
    return merged


def invert_ranges(ranges: list[range], within: range) -> list[range]:
    """
    Invert a list of ranges to get the complement within a larger range.

    Given a list of ranges and a bounding range, returns all the "gaps" (ranges not covered
    by the input ranges) within the bounding range.

    Args:
        ranges: A list of ranges to invert.
        within: The bounding range that defines the search space.

    Returns:
        A list of ranges representing all positions within `within` not covered by `ranges`.
    """
    result = []
    ranges = unify_ranges(ranges)
    curr = within.start
    for r in ranges:
        result.append(range(curr, r.start))
        curr = r.stop
    result.append(range(curr, within.stop))
    return result


def re_named_codepoint(name: str) -> str:
    """
    Generate a regex pattern for matching a named Unicode codepoint hexadecimal value.
    
    Creates a named capture group that matches 1-6 hexadecimal digits, suitable for
    parsing Unicode codepoint ranges in the standard Unicode data format.
    
    Args:
        name: The name of the capture group (e.g., 'start', 'end').
    
    Returns:
        A regex pattern string for a named group matching hex digits.
    """
    return rf"(?P<{name}>[0-9A-Fa-f]{{1,6}})"


CODEPOINT_RANGE_RE = r"^{}(?:\s*\.\.\s*{})?$".format(
    re_named_codepoint("start"), re_named_codepoint("end")
)
PAGE_WIDTH = 256


def parse_codepoint_range_str(codepoint: str) -> range | None:
    """
    Parse a Unicode codepoint range expression string into a Python range object.
    
    Converts range expressions like "0041" or "0041..005A" (from Unicode data files)
    into Python range objects. Returns None if the input is malformed.
    
    Args:
        codepoint: A codepoint range expression string (may be a single value or range).
    
    Returns:
        A range object representing the parsed codepoint range, or None if parsing fails.
    
    Example:
        parse_codepoint_range_str("0041") -> range(65, 66)  # Single character
        parse_codepoint_range_str("0041..005A") -> range(65, 91)  # Range A-Z
    """
    m = re.match(CODEPOINT_RANGE_RE, codepoint.strip())
    if m is None:
        return
    start = int(m.group("start"), base=16)
    end = m.group("end")
    if end is None:
        return range(start, start + 1)
    return range(start, int(end, base=16) + 1)


UNICODE_BITS = 21
BYTE_BITS = 8


def create_mapping_tables(
    ranges: list[range], level: int
) -> tuple[list[int], list[tuple[int, ...]], list[Bitfield]]:
    """
    Create optimized 3-level trie tables for fast Unicode property lookup.
    
    Generates a hierarchical trie structure that enables O(1) constant-time lookups
    for Unicode character properties. The trie consists of:
    - L1 Trie: Maps Unicode planes to L2 trie entries
    - L2 Trie: Maps pages to bitfield entries
    - Bitfields: 256-bit fields encoding character properties (packed into four 64-bit integers)
    
    Args:
        ranges: A list of codepoint ranges representing characters with a property.
        level: The L1 trie level (typically 8). Higher levels = larger L1 trie, smaller L2 tries.
    
    Returns:
        A tuple of (l1trie, l2trie, bitfields):
        - l1trie: List of 256 L1 trie entries mapping to L2 trie indices
        - l2trie: List of tuples representing L2 trie pages
        - bitfields: List of unique 256-bit bitfields used for property storage
    """
    # dict[_T, tuple[int, _T]] is a great way to make a makeshift ordered set
    uniq_bitfields: dict[Bitfield, tuple[int, Bitfield]] = {}
    l2trie_dict: dict[tuple[int, ...], tuple[int, list[int]]] = {}
    l1trie = [-1] * (1 << level)

    LEVEL1_BITS = UNICODE_BITS - level
    LEVEL2_BITS = LEVEL1_BITS - BYTE_BITS

    range_idx = 0

    # Filling L2 Trie
    for il2 in range(0, 1 << UNICODE_BITS, 1 << LEVEL1_BITS):
        l2trie_ls = [-1 for _ in range(1 << LEVEL2_BITS)]

        # Filling L1 Trie
        for il1 in range(0, 1 << LEVEL1_BITS, 1 << BYTE_BITS):
            bitfield = 0
            # Filling bitfield
            for j in range(1 << BYTE_BITS):
                cp = il2 + il1 + j
                while range_idx < len(ranges) and ranges[range_idx].stop <= cp:
                    range_idx += 1
                if range_idx < len(ranges) and ranges[range_idx].start <= cp:
                    bitfield |= 1 << j
                # bitfield |= in_range_list(cp, ranges) << j
            if bitfield not in uniq_bitfields:
                uniq_bitfields[bitfield] = len(uniq_bitfields), bitfield
            bitfield_index, _ = uniq_bitfields[bitfield]
            l2trie_ls[il1 >> 8] = bitfield_index

        # We need tuples for hashing purposes
        l2trie_tup = tuple(l2trie_ls)
        if l2trie_tup not in l2trie_dict:
            l2trie_dict[l2trie_tup] = len(l2trie_dict), l2trie_ls
        l2trie_index, _ = l2trie_dict[l2trie_tup]
        l1trie[il2 >> LEVEL1_BITS] = l2trie_index

    # Make everything a list
    bitfields = list(uniq_bitfields.keys())
    l2trie = list(l2trie_dict.keys())
    return l1trie, l2trie, bitfields


now = datetime.datetime.now()
generated_on = now.isoformat()
tzoffset = (now.astimezone().tzinfo or datetime.UTC).utcoffset(
    None
) or datetime.timedelta()
total_secs = int(tzoffset.total_seconds())
utc_tzoffset = f"UTC{'+' if total_secs >= 0 else '-'}{abs(total_secs) // 3600:02d}:{(abs(total_secs) % 3600) // 60:02d}"

HEADER_START = f"""
/// SPDX-License-Identifier: (Apache-2.0 WITH LLVM-exception) AND Unicode-3.0
///
/// ---------------------------------------------------------------------------
///
/// This file is automatically generated. DO NOT EDIT.
/// - Generated on: {generated_on} (ISO 8601 - {utc_tzoffset})
/// - Unicode version: 17.0.0
///
/// ---------------------------------------------------------------------------
///
/// CODE LICENSE:
/// The logic and generator code are licensed under Apache-2.0 WITH 
/// LLVM-exception. Copyright (c) 2026 Kerem Göksu.
///
/// DATA LICENSE:
/// This file contains data derived from the Unicode Character Database (UCD) 
/// and UAX #31. This data is licensed under the Unicode-3.0 license.
/// Copyright (c) 1991-2026 Unicode, Inc. 
///
/// For the full license text, see the NOTICE.xg file in this distribution 
/// or visit https://www.unicode.org/license.txt
///
/// ---------------------------------------------------------------------------
/// 
/// This file is intended for use within the Gest project, and is not designed
/// for external includes.

#ifndef GEST_UNICODE_H
#define GEST_UNICODE_H

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <gest/export.h>

typedef struct GestUniRange GestUniRange;
struct GestUniRange {{
    uint32_t start, end;
}};

static inline int gestUniRangeContains(GestUniRange range, uint32_t u32c) {{
    return (range).start <= (u32c) && (u32c) < (range).end;
}}
""".strip()

HEADER_END = """
#endif // GEST_UNICODE_H
""".lstrip()

IMPL_START = """
#include <gest/_generated/_unicode.h>

""".lstrip()

B64MASK = (1 << 64) - 1


def gen_uni(
    name: str,
    ranges: list[range],
    l1trie: list[int],
    l2trie: list[tuple[int, ...]],
    bitfields: list[Bitfield],
    level: int,
    file: IO,
    impl_file: IO,
):
    """
    Generate C header and implementation code for Unicode property checking functions.
    
    Outputs complete C code for checking whether a Unicode character has a specific property.
    Generates both slow O(logn) binary search and fast O(1) trie-based functions with and
    without bounds checking.
    
    Args:
        name: The property name (e.g., 'ID_Start', 'XID_Continue').
        ranges: List of codepoint ranges with the property.
        l1trie: L1 trie lookup table.
        l2trie: L2 trie lookup table.
        bitfields: Bitfield lookup table.
        level: The L1 trie level used in generation.
        file: Output file handle for C header declarations.
        impl_file: Output file handle for C function implementations.
    
    Raises:
        AssertionError: If trie structure exceeds implementation limits (256 entries).
    """
    assert (
        len(l1trie) == 256
    ), f"{name}: L1 Trie does not cover the full space required."
    assert (
        len(l2trie) < 256
    ), f"{name}: There are more than 256 L2 Trie entries, going beyond the generation limit."
    assert (
        len(bitfields) < 256
    ), f"{name}: There are more than 256 bitfield entries, going beyond the generation limit."
    bname = name
    name = name.upper()
    LEVEL1_BITS = UNICODE_BITS - level
    L1MASK = (1 << level) - 1
    LEVEL2_BITS = LEVEL1_BITS - BYTE_BITS
    L2MASK = (1 << LEVEL2_BITS) - 1
    camel_name = name.replace("_", " ").title().replace(" ", "")

    print(
        f"static const size_t GEST_UNI_{name}_RANGE_COUNT = {len(ranges)};", file=file
    )
    print(
        f"GEST_API extern const GestUniRange GEST_UNI_{name}_RANGE[{len(ranges)}];",
        file=file,
    )
    print(
        f"const GestUniRange GEST_UNI_{name}_RANGE[{len(ranges)}] = {{{', '.join(f'{{{r.start}, {r.stop}}}' for r in ranges)}}};",
        file=impl_file,
    )
    print(file=impl_file)
    # --- #
    print(
        f"/// Checks if the given character has the property `{bname}` using an O(logn) binary range search.",
        file=file,
    )
    print("/// @param u32c The character to check for.", file=file)
    print(
        f"/// @returns `1` if the property `{bname}` includes the character, otherwise `0`",
        file=file,
    )
    print(f"GEST_API extern int gestUniC32IsUni{camel_name}(uint32_t u32c);", file=file)
    print(f"int gestUniC32IsUni{camel_name}(uint32_t u32c) {{", file=impl_file)
    print(f"    size_t lo = 1, hi = GEST_UNI_{name}_RANGE_COUNT - 1;", file=impl_file)
    print(
        f"    if (gestUniRangeContains(GEST_UNI_{name}_RANGE[0], u32c)) return 1;",
        file=impl_file,
    )
    print("    while (lo <= hi) {", file=impl_file)
    print("        const size_t pivot=lo + (hi - lo) / 2;", file=impl_file)
    print(
        f"        if (gestUniRangeContains(GEST_UNI_{name}_RANGE[pivot], u32c)) return 1;",
        file=impl_file,
    )
    print(
        f"        else if (u32c < GEST_UNI_{name}_RANGE[pivot].start) hi = pivot - 1;",
        file=impl_file,
    )
    print("        else lo = pivot + 1;", file=impl_file)
    print("    }", file=impl_file)
    print("    return 0;", file=impl_file)
    print("}", file=impl_file)
    # --- #
    print(
        f"static const size_t GEST_UNI_{name}_L1TRIE_COUNT = {len(l1trie)};", file=file
    )
    print(
        f"GEST_API extern const uint8_t GEST_UNI_{name}_L1TRIE[{len(l1trie)}];",
        file=file,
    )
    print(
        f"const uint8_t GEST_UNI_{name}_L1TRIE[{len(l1trie)}] = {{\n    {',\n    '.join(str(i) for i in l1trie)}\n}};",
        file=impl_file,
    )
    print(
        f"static const size_t GEST_UNI_{name}_L2TRIE_COUNT = {len(l2trie)};", file=file
    )
    print(
        f"GEST_API extern const uint8_t GEST_UNI_{name}_L2TRIE[{len(l2trie)}][{len(l2trie[0])}];",
        file=file,
    )
    print(
        f"const uint8_t GEST_UNI_{name}_L2TRIE[{len(l2trie)}][{len(l2trie[0])}] = {{\n    {',\n    '.join('{' + ', '.join(str(i) for i in subtrie) + '}' for subtrie in l2trie)}\n}};",
        file=impl_file,
    )
    print(
        f"static const size_t GEST_UNI_{name}_BITF_COUNT = {len(bitfields)};", file=file
    )
    print(
        f"GEST_API extern const uint64_t GEST_UNI_{name}_BITF[{len(bitfields)}][4];",
        file=file,
    )
    print(
        f"const uint64_t GEST_UNI_{name}_BITF[{len(bitfields)}][4] = {{\n    {',\n    '.join(f'{{{(bitfield >> 192) & B64MASK}, {(bitfield >> 128) & B64MASK}, {(bitfield >> 64) & B64MASK}, {bitfield & B64MASK}}}' for bitfield in bitfields)}\n}};",
        file=impl_file,
    )
    # --- #
    print(
        f"/// Checks if the given character has the property `{bname}` using an O(1) 3-level trie.",
        file=file,
    )
    print(
        "/// @note This function does not have bounds checking and can cause static buffer overflows.",
        file=file,
    )
    print("/// @param u32c The character to check for.", file=file)
    print(
        f"/// @returns `1` if the property `{bname}` includes the character, otherwise `0`",
        file=file,
    )
    print(
        f"GEST_API extern int gestUniC32IsUni{camel_name}FastUnchecked(uint32_t u32c);",
        file=file,
    )
    print(
        f"int gestUniC32IsUni{camel_name}FastUnchecked(uint32_t u32c) {{",
        file=impl_file,
    )
    print(f"    return (GEST_UNI_{name}_BITF[", file=impl_file)
    print(
        f"        GEST_UNI_{name}_L2TRIE[GEST_UNI_{name}_L1TRIE[(u32c >> {LEVEL1_BITS}) & {L1MASK}]][(u32c >> {BYTE_BITS}) & {L2MASK}]",
        file=impl_file,
    )
    print("    ][(u32c >> 6) & 3] >> (u32c & 63)) & 1;", file=impl_file)
    print("}", file=impl_file)
    # --- #
    print(
        f"/// Checks if the given character has the property `{bname}` using an O(1) 3-level trie with bounds-checking.",
        file=file,
    )
    print("/// @param u32c The character to check for.", file=file)
    print(
        f"/// @returns `1` if the property `{bname}` includes the character, otherwise `0`",
        file=file,
    )
    print(
        f"GEST_API extern int gestUniC32IsUni{camel_name}Fast(uint32_t u32c);",
        file=file,
    )
    print(f"int gestUniC32IsUni{camel_name}Fast(uint32_t u32c) {{", file=impl_file)
    print("    assert(u32c < 0x110000);", file=impl_file)
    print(f"    return gestUniC32IsUni{camel_name}FastUnchecked(u32c);", file=impl_file)
    print("}", file=impl_file)


def gen_prop_mapping_tables_task(args_tuple):
    """
    Generate trie tables for a single Unicode property (designed for parallel execution).
    
    This is a worker function for multiprocessing that generates the trie structure
    for one Unicode property. It's wrapped to avoid pickling issues with ProcessPoolExecutor.
    
    Args:
        args_tuple: A tuple of (name, prop_ranges, trie_level).
    
    Returns:
        A tuple of (name, prop_ranges, l1trie, l2trie, bitfields).
    """
    name, prop_ranges, trie_level = args_tuple
    l1, l2, bitf = create_mapping_tables(prop_ranges, trie_level)
    return name, prop_ranges, l1, l2, bitf


@dataclass
class Decomposition:
    canonical: bool
    codepoints: tuple[Codepoint, ...]

    @classmethod
    def parse(cls, decomposition: str) -> Self | None:
        if not decomposition:
            return None
        canon = True
        if decomposition.startswith("<"):
            canon = False
            _, decomposition = decomposition.split(">", maxsplit=1)
        codepoints_str = decomposition.split()
        codepoints = tuple(
            cast(range, parse_codepoint_range_str(cps)).start for cps in codepoints_str
        )
        return cls(canon, codepoints)


@dataclass
class UnicodeDataRow:
    codepoint: Codepoint | range
    category: str
    ccc: int
    name: str
    decomposition: Decomposition | None = None


def main(args) -> None:
    """
    Main entry point: orchestrate the entire Unicode property code generation pipeline.
    
    Reads Unicode Character Database files, generates optimized lookup tables, and outputs
    C header and implementation files with Unicode property checking functions. Includes
    optional timing and size reporting.
    
    Args:
        args: Parsed command-line arguments with attributes:
            - data_dir: Directory containing Unicode data files (default: 'data')
            - trie_level: L1 trie level for optimization (default: 8)
            - header: Output file for C header (default: stdout)
            - code: Output file for C implementation (default: stdout)
            - time: Set of timing measurements to report ('all', 'read', 'generation', 'write')
            - size: Whether to report total trie size
    """
    file, impl_file = io.StringIO(), io.StringIO()

    now = 0.0
    if "read" in args.time:
        now = time.time()
    data_dir = pathlib.Path(args.data_dir).absolute()
    unicode_dir = data_dir / "unicode"
    composition_exclusions_path = unicode_dir / "CompositionExclusions.txt"
    derived_normalization_props_path = unicode_dir / "DerivedNormalizationProps.txt"
    derived_core_properties_path = unicode_dir / "DerivedCoreProperties.txt"
    unicode_data_path = unicode_dir / "UnicodeData.txt"

    unidata_list: list[UnicodeDataRow] = []

    with open(unicode_data_path) as unidata:
        prange_start = -1
        for line in unidata:
            (
                codepoint,
                charname,
                category,
                ccc_s,
                _bidi,
                decomposition,
                _dec,
                _dig,
                _num,
                _mirror,
                _u1name,
                _,
                _upper,
                _lower,
                _title,
            ) = line.split(";")
            cp: Codepoint = cast(range, parse_codepoint_range_str(codepoint)).start
            ccc = int(ccc_s)
            if charname.endswith(", First>"):
                prange_start = cp
            elif charname.endswith(", Last>"):
                charname = charname.removeprefix("<").removesuffix(", Last>")
                unidata_list.append(
                    UnicodeDataRow(range(prange_start, cp + 1), category, ccc, charname)
                )
            else:
                unidata_list.append(
                    UnicodeDataRow(
                        cp, category, ccc, charname, Decomposition.parse(decomposition)
                    )
                )
                cediname = charname.replace(" ", "_").replace("-", "_")
                if all(c.isalnum() or c == "_" for c in cediname):
                    print(f"#define GEST_UNI_U32C_{cediname} {cp}", file=file)

    composition_exclusions_base: list[range] = []

    with open(composition_exclusions_path) as f:
        for line in f:
            line = line.split("#", maxsplit=1)[0].strip()
            if not line:
                continue
            codepoint_range = parse_codepoint_range_str(line)
            if not codepoint_range:
                raise ValueError(f"Expected codepoint or codepoint range: {line}")
            composition_exclusions_base.append(codepoint_range)

    composition_exclusions = composition_exclusions_base.copy()
    derivation_normalisation_props = parse_ucd_properties(
        derived_normalization_props_path
    )
    derived_core_properties_props = parse_ucd_properties(derived_core_properties_path)

    composition_exclusions = unify_ranges(composition_exclusions)
    if "read" in args.time:
        print(f"read: Took {time.time() - now:.2f}s")

    props = [
        (k, v)
        for k, v in chain(
            derivation_normalisation_props.items(),
            derived_core_properties_props.items(),
        )
        if isinstance(v, list)
    ]

    nfc_qc: dict[str, list[range]] = cast(
        dict[str, list[range]], derivation_normalisation_props["NFC_QC"]
    )
    nfc_qc_yes = invert_ranges(nfc_qc["N"] + nfc_qc["M"], range(0x110000))
    nfc_qc_maybe: list[range] = nfc_qc["M"]
    tasks = [
        ("NFC_QC_Y", nfc_qc_yes, args.trie_level),
        ("NFC_QC_M", nfc_qc_maybe, args.trie_level),
    ] + [(prop_name, unify_ranges(prop), args.trie_level) for prop_name, prop in props]

    if "generation" in args.time:
        now = time.time()
    with ProcessPoolExecutor() as executor:
        results = executor.map(gen_prop_mapping_tables_task, tasks)
    if "generation" in args.time:
        print(f"generation: Took {time.time() - now:.2f}s")

    size = 0
    if "write" in args.time:
        now = time.time()

    for name, prop_ranges, l1, l2, bitf in results:
        gen_uni(
            name,
            prop_ranges,
            l1,
            l2,
            bitf,
            args.trie_level,
            file=file,
            impl_file=impl_file,
        )
        size += len(l1) + len(l2) * len(l2[0]) + len(bitf) * 32

    if args.size:
        print(f"Table sizes: {size}")

    # Output the result
    print(HEADER_START, file=args.header)
    print(file.getvalue(), file=args.header)
    print(IMPL_START, file=args.code)
    print(impl_file.getvalue(), file=args.code)
    print(HEADER_END, file=args.header)
    if "write" in args.time:
        print(f"write: Took {time.time() - now:.2f}s")


def parse_ucd_properties(path: pathlib.Path | str):
    """
    Parse Unicode Character Database property files into a structured dictionary.
    
    Reads UCD property files (DerivedNormalizationProps.txt, DerivedCoreProperties.txt)
    and extracts codepoint ranges for each property. Handles properties with and without
    property values (e.g., "N", "M" for NFC_QC).
    
    Args:
        path: Path to a UCD property file.
    
    Returns:
        A dictionary where:
        - Keys are property names (e.g., 'XID_Start')
        - Values are either:
          - list[range]: For properties without values
          - dict[str, list[range]]: For properties with values (keyed by value)
    """
    props: dict[str, list[range] | dict[str, list[range]]] = {}
    with open(path) as f:
        for line in f:
            line = line.split("#", maxsplit=1)[0].strip()
            if not line:
                continue
            range_str, key, *value = (i.strip() for i in line.split(";"))
            codepoint_range = parse_codepoint_range_str(range_str)
            if not codepoint_range:
                raise ValueError(f"Expected codepoint or codepoint range: {line}")
            # Some properties have values (like NFC_QC: N/M/Y), others don't
            if key not in props:
                # dict for valued properties, list for unvalued ones
                props[key] = {} if value else []
            inst = props[key]
            if isinstance(inst, list):
                inst.append(codepoint_range)
            else:
                if value[0] not in inst:
                    inst[value[0]] = []
                inst[value[0]].append(codepoint_range)
    return props


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Generates a unicode header for gest")
    ap.add_argument(
        "-D", "--data-dir", help="The location of the data dir", default="data"
    )
    ap.add_argument(
        "-l", "--trie-level", help="The level of the L1 trie", default=8, type=int
    )
    ap.add_argument(
        "-H",
        "--header",
        help="Output header",
        default=sys.stdout,
        type=lambda path: open(path, "w"),
    )
    ap.add_argument(
        "-c",
        "--code",
        help="Output C code",
        default=sys.stdout,
        type=lambda path: open(path, "w"),
    )
    ap.add_argument(
        "-T",
        "--time",
        default=[],
        choices=["all", "generation", "write", "read"],
        action="append",
    )
    ap.add_argument("-s", "--size", default=False, action="store_true")
    args = ap.parse_args()
    args.time = set(args.time)

    now = 0.0
    if "all" in args.time:
        now = time.time()
    try:
        main(args)
    finally:
        if args.header is not sys.stdout:
            args.header.close()
        if args.code is not sys.stdout:
            args.code.close()
    if "all" in args.time:
        print(f"all: Took {time.time() - now:.2f}s")
