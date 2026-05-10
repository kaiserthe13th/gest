#!/usr/bin/env -S python
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# SPDX-FileCopyrightText: 2026 Kerem Göksu

import argparse
import collections.abc
import glob
import json
import html
import pprint
from argparse import ArgumentParser
import os
import re
import sys
import yaml
from typing import Any, Callable, Self, NamedTuple
from license_expression import ExpressionError, get_spdx_licensing
from tqdm import tqdm


class FileSpec(NamedTuple):
    path: str
    format: str

    @classmethod
    def parse(cls, value: str) -> Self:
        if value.startswith("="):
            path = ""
            fmt = value[1:]
        elif "=" in value:
            path, fmt = value.rsplit("=", 1)
        else:
            path = value
            if os.path.basename(path).lower() == "notice":
                fmt = "xg"
            else:
                _, ext = os.path.splitext(path)
                fmt = ext[1:].lower() if ext else "txt"
        return cls(path, fmt)


spdx = get_spdx_licensing()


class TagAsStringLoader(yaml.SafeLoader):
    def __init__(self, stream) -> None:
        super().__init__(stream)
        self.add_multi_constructor("", self.catch_all_constructor)

    @staticmethod
    def catch_all_constructor(loader, tag_suffix, node):
        return tag_suffix + node.value


class XGParser:
    XG_MANIFEST_HEADER = "# --- XG MANIFEST ---\n"
    XG_HEADER = "# --- XG ---\n"
    XG_VERBATIM_HEADER = "# --- XG VERBATIM ---\n"
    XG_END_HEADER = "# --- XG END ---\n"

    _src: str
    _index: int

    def __init__(self, src: str) -> None:
        self._src = src
        self._index = 0

    def reset(self):
        self._index = 0

    def __iter__(self):
        return self

    def __next__(self) -> dict:
        try:
            # XG ... XG END
            start = self._src[self._index :].index(self.XG_HEADER) + len(self.XG_HEADER)
            stop = self._src[self._index + start :].index(self.XG_END_HEADER)
            yaml_part = "\n".join(
                line.removeprefix("# ").removeprefix("#")
                for line in self._src[
                    self._index + start : self._index + start + stop
                ].splitlines()
            )
            xg_object = yaml.load(yaml_part, Loader=TagAsStringLoader)
            self._index += start + stop + len(self.XG_END_HEADER)

            # XG VERBATIM ... XG END
            start = self._src[self._index :].index(self.XG_VERBATIM_HEADER) + len(
                self.XG_VERBATIM_HEADER
            )
            stop = self._src[self._index + start :].index(self.XG_END_HEADER)
            xg_object["Notice"] = self._src[
                self._index + start : self._index + start + stop
            ].removesuffix("\n")
            self._index += start + stop + len(self.XG_END_HEADER)

            return xg_object
        except (IndexError, ValueError):
            raise StopIteration


HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>[XG] NOTICE</title>
</head>
<body>
    <article>{}</article>
    <footer><code>
        Generated with <b>xg.py</b> brought to you by <b>Kerem Göksu</b><br>
        Xg-Version: 0
    </code></footer>
</body>
</html>"""

# ===


class _OnlyShowChoices(str):
    pass


OnlyShowChoices: _OnlyShowChoices = _OnlyShowChoices()


class _ShowUnderlyingChoices(str):
    pass


ShowUnderlyingChoices: _ShowUnderlyingChoices = _ShowUnderlyingChoices()

# ---


class _AliasAll(str):
    pass


AliasAll = _AliasAll()

# ---


class OptionSetCollectAction[T](argparse.Action):
    def __init__(
        self,
        option_strings: collections.abc.Sequence[str],
        dest: str,
        nargs: int | str | None = None,
        const: T | None = None,
        default: T | str | None = None,
        type: Callable[[str], T] | argparse.FileType | None = None,
        choices: collections.abc.Iterable[str] | None = None,
        required: bool = False,
        help: str | None = None,
        metavar: _OnlyShowChoices | str | None = None,
        deprecated: bool = False,
        aliases: collections.abc.Mapping[str, collections.abc.Sequence[str] | str]
        | None = None,
        removal_prefix: str | None = None,
    ) -> None:
        if nargs == 0:
            raise ValueError("nargs for option set collect actions must be != 0")
        if type is not None and type is not str:
            raise ValueError("type for option set collect actions must be str or None")

        if choices is not None:
            choices = [*choices]
            if isinstance(metavar, _OnlyShowChoices):
                metavar = "{" + ",".join(choices) + "}"  # for clutter management
                if removal_prefix is not None:
                    metavar = f"[{removal_prefix}]" + metavar
            if aliases is not None:
                all_choices = choices.copy()
                aliases = {
                    alias: (all_choices if isinstance(val, _AliasAll) else val)
                    for alias, val in aliases.items()
                }
                choices.extend(
                    key for key in aliases.keys()
                )  # this is required because argparse will error otherwise
            if metavar is None:
                metavar = "{" + ",".join(choices) + "}"  # for clutter management
                if removal_prefix is not None:
                    metavar = f"[{removal_prefix}]" + metavar
            if isinstance(metavar, _ShowUnderlyingChoices):
                metavar = None
            if removal_prefix is not None:
                choices.extend((removal_prefix + choice) for choice in choices.copy())
        if isinstance(metavar, _OnlyShowChoices):
            metavar = None

        super().__init__(
            option_strings,
            dest,
            nargs,
            const,
            default,
            type,
            choices,
            required,
            help,
            metavar,
            deprecated,
        )
        self.aliases = aliases or {}
        self.removal_prefix = removal_prefix

    def __call__(
        self,
        parser: ArgumentParser,
        namespace: argparse.Namespace,
        values: str | collections.abc.Sequence[str] | None,
        option_string: str | None = None,
    ) -> None:
        assert values is not None
        items: set[str] = getattr(namespace, self.dest, None) or set()
        if isinstance(values, str):
            values = [values]
        for value in values:
            insert = True
            if self.removal_prefix is None or not value.startswith(self.removal_prefix):
                vkeys = self.aliases.get(value, value)
            else:
                unprefixed_value = value.removeprefix(self.removal_prefix)
                vkeys = self.aliases.get(unprefixed_value, unprefixed_value)
                insert = False
            if isinstance(vkeys, str):
                vkeys = (vkeys,)
            for vkey in vkeys:
                if self.choices is not None and vkey not in self.choices:
                    parser.error(
                        f'value not in choices: ({", ".join(str(choice) for choice in self.choices)})'
                    )
                if insert:
                    items.add(vkey)
                else:
                    items.discard(vkey)
        setattr(namespace, self.dest, items)


# ===


SPDX_LICENSE_IDENTIFIER = "SPDX-License-Identifier"


def verify_spdx_licenses(notices: list[dict[str, Any]]) -> None:
    for notice in notices:
        if (spdx_ident := notice.get(SPDX_LICENSE_IDENTIFIER)) is not None:
            spdx_ident = str(spdx_ident)
            try:
                spdx.parse(spdx_ident, validate=True, strict=True)
            except ExpressionError as e:
                raise ValueError(e)
        else:
            print(
                f"WARNING: SPDX-License-Identifier not specified in {notice['Project-Name']}."
            )


def verify_unlicensed_files(notices: list[dict[str, Any]]) -> None:
    # Filter out directories immediately to work only with files
    unclaimed_files = {f for f in FILES if os.path.isfile(f)}

    for notice in notices:
        notice_files = notice.get("Files", [])

        # Normalize string input to a list
        if isinstance(notice_files, str):
            notice_files = [notice_files]

        include = set()
        exclude = set()

        for nglob in notice_files:
            # Clean the glob string
            clean_glob = nglob.split("#")[0].strip()
            if not clean_glob:
                continue

            is_negation = clean_glob.startswith("!")
            pattern = clean_glob[1:].strip() if is_negation else clean_glob

            # Expand the glob
            matched = set(glob.glob(pattern, recursive=True, include_hidden=True))

            if is_negation:
                exclude.update(matched)
            else:
                include.update(matched)

        # Identify files covered by this specific notice
        # Intersection ensures we only care about files that actually exist
        covered_by_this_notice = unclaimed_files.intersection(include)
        covered_by_this_notice -= exclude

        # Remove them from the master 'unclaimed' set
        unclaimed_files -= covered_by_this_notice

    # If anything remains, it's a violation
    if unclaimed_files:
        error_msg = "There are unlicensed files:\n" + "\n".join(
            f"- {f}" for f in sorted(unclaimed_files)
        )
        raise ValueError(error_msg)


def list_files() -> set[str]:
    all_paths = set(glob.glob("**", recursive=True, include_hidden=True))

    exclude = set()
    force_include = set()

    xg_exclude = set()
    xg_force_include = set()

    ignore_files = [
        (os.path.basename(p), p)
        for p in all_paths
        if os.path.basename(p) in {".gitignore", ".ignore", ".xgignore"}
    ]

    for file_basename, ignore_path in ignore_files:
        root_dir = os.path.dirname(ignore_path)
        with open(ignore_path) as f:
            exclude.add(ignore_path)
            for line in f:
                line = line.split("#")[0].strip()
                if not line:
                    continue

                is_negation = line.startswith("!")
                pattern = line[1:] if is_negation else line

                full_pattern = os.path.join(root_dir, pattern)
                matched = set(
                    glob.glob(full_pattern, recursive=True, include_hidden=True)
                )
                matched.update(
                    glob.glob(
                        os.path.join(full_pattern, "**"),
                        recursive=True,
                        include_hidden=True,
                    )
                )
                matched = {os.path.normpath(m) for m in matched}

                if file_basename == ".xgignore":
                    if is_negation:
                        xg_force_include.update(matched)
                    else:
                        xg_exclude.update(matched)
                else:
                    if is_negation:
                        force_include.update(matched)
                    else:
                        exclude.update(matched)

    result = {p for p in all_paths if os.path.isfile(p)}
    result -= exclude
    result |= force_include
    result -= xg_exclude
    result |= xg_force_include

    return result


SPDX_REGEX = re.compile(rf"{SPDX_LICENSE_IDENTIFIER}\s*:\s*([^\*/]*).*$")


def verify_license_matches(notices: list[dict[str, Any]]) -> None:
    # Filter out directories immediately to work only with files
    files = {f for f in FILES if os.path.isfile(f)}

    for notice in notices:
        project_name = notice.get("Project-Name", "<Unnamed Project>")
        notice_files = notice.get("Files", [])

        # Normalize string input to a list
        if isinstance(notice_files, str):
            notice_files = [notice_files]

        include = set()
        exclude = set()

        for nglob in notice_files:
            clean_glob = nglob.split("#")[0].strip()
            if not clean_glob:
                continue

            is_negation = clean_glob.startswith("!")
            pattern = clean_glob[1:].strip() if is_negation else clean_glob
            matched = set(glob.glob(pattern, recursive=True, include_hidden=True))

            if is_negation:
                exclude.update(matched)
            else:
                include.update(matched)

        covered_by_this_notice = files.intersection(include) - exclude

        if (spdx_ident := notice.get(SPDX_LICENSE_IDENTIFIER)) is not None:
            spdx_ident = str(spdx_ident)
            try:
                expr = spdx.parse(spdx_ident, validate=True, strict=True)
            except ExpressionError as e:
                raise ValueError(e)
        else:
            expr = None
            print(f"WARNING: SPDX-License-Identifier not specified in {project_name}.")

        errs = []

        # Add tqdm here to track file scanning progress
        pbar = tqdm(
            sorted(covered_by_this_notice),
            desc=f"Scanning {project_name[:20]}...",
            leave=False,
            unit="file",
        )

        for file in pbar:
            try:
                with open(file, errors="ignore") as f:
                    found_spdx = False
                    for _ in range(15):
                        line = f.readline()
                        if m := SPDX_REGEX.search(line):
                            found_spdx = True
                            file_license_expr = m.group(1).strip()
                            mexpr = spdx.parse(
                                file_license_expr, validate=True, strict=True
                            )

                            if expr is not None:
                                if (
                                    mexpr is not None
                                    and expr.simplify() != mexpr.simplify()
                                ):
                                    errs.append(
                                        f"License mismatch in `{file}`: file({mexpr}) != notice({expr})"
                                    )
                            elif mexpr is not None:
                                errs.append(
                                    f"`{file}` has a license ({mexpr}) but notice does not"
                                )
                            break

                    if not found_spdx and expr is not None:
                        errs.append(
                            f"`{file}` is missing an SPDX identifier (expected {expr})"
                        )

            except (ExpressionError, OSError) as e:
                errs.append(f"Error processing `{file}`: {e}")
            except Exception as e:
                pbar.close()
                raise e

        # Clear pbar before printing errors to avoid mangled output
        pbar.close()

        if errs:
            raise ValueError(
                f"Found {len(errs)} errors in {project_name}:\n" + "\n".join(errs)
            )

        print(
            f"INFO: Project `{project_name}` ({len(covered_by_this_notice)} files) matches {expr}."
        )


VERIFIERS = {
    "spdx-licenses": verify_spdx_licenses,
    "unlicensed-files": verify_unlicensed_files,
    "license-matches": verify_license_matches,
}
FILES = set()


def main():
    global FILES

    SEP = "-" * 80 + "\n"

    ap = ArgumentParser(
        "xg",
        description="xg is a program used to extract data from NOTICE.xg files to generate multiple formats from it.",
    )
    ap.add_argument("file", help="The XG notice input")
    ap.add_argument(
        "-d",
        "--dump",
        action="append",
        help="Dump into specified file. ([file.ext][=format] is the spec format). Can be used multiple times.",
        metavar="SPEC",
        dest="dumps",
        type=FileSpec.parse,
        default=[],
        required=False,
    )
    ap.add_argument(
        "-o",
        "--base-output",
        help="Set the base output file (without a suffix)",
        required=False,
    )
    ap.add_argument(
        "-v",
        "--verify",
        dest="verifiers",
        help="Enable/disable given verifier. Can be used multiple times.",
        removal_prefix="no-",
        action=OptionSetCollectAction,
        choices=["spdx-licenses", "unlicensed-files", "license-matches"],
        aliases={
            "all": AliasAll,
            "spdx": "spdx-licenses",
            "unlicensed": "unlicensed-files",
            "matches": "license-matches",
        },
        required=False,
    )
    # spdx.parse(, validate=True)
    args = ap.parse_args()

    with open(args.file) as f:
        src = f.read()
        notices = [notice for notice in XGParser(src)]

    os.chdir(os.path.dirname(args.file))
    FILES = list_files()

    verifiers = {VERIFIERS[i] for i in args.verifiers or set()}

    failed = False

    for verify in verifiers:
        try:
            verify(notices)
            print(f"{verify.__name__}: No errors.")
        except ValueError as e:
            print(f"{verify.__name__}: {e}")
            failed = True
            break

    for dump in args.dumps:
        try:
            if dump.path == "-":
                f = sys.stdout
            elif dump.path:
                f = open(dump.path, "w")
            else:
                root, _ = os.path.splitext(args.base_output or args.file)
                f = open(f"{root}.{dump.format}", "w")

            match dump.format.lower():
                case "print":
                    print(notices, file=f)
                case "pprint":
                    pprint.pprint(notices, stream=f)
                case "json" | "jsonc" | "json5":
                    json.dump(notices, f)
                case "yml" | "yaml":
                    print(yaml.safe_dump(notices), file=f)
                case "txt":
                    for notice in notices:
                        print(notice["Notice"], file=f)
                        print(SEP, file=f)
                case "xg":
                    print(end=XGParser.XG_MANIFEST_HEADER, file=f)
                    print("# Xg-Version: 0", file=f)
                    print(XGParser.XG_END_HEADER, file=f)

                    for notice in notices:
                        wo_notice = notice.copy()
                        del wo_notice["Notice"]
                        print(end=XGParser.XG_HEADER, file=f)
                        fxy = "\n".join(
                            "# " + line
                            for line in yaml.safe_dump(wo_notice).splitlines()
                        )
                        print(fxy, file=f)
                        print(XGParser.XG_END_HEADER, file=f)
                        print(end=XGParser.XG_VERBATIM_HEADER, file=f)
                        print(notice["Notice"], file=f)
                        print(XGParser.XG_END_HEADER, file=f)
                        print(SEP, file=f)
                case "html":
                    body = ""

                    for notice in notices:
                        body += "<pre>"
                        wo_notice = notice.copy()
                        del wo_notice["Notice"]
                        notice_data = yaml.dump(wo_notice)

                        def make_url(url):
                            return f"<a href={url!r}>{url}</a>"

                        body += "\n".join(
                            (
                                html.escape(line[: line.index("URL:") + 4])
                                + " "
                                + make_url(line[line.index("URL:") + 4 :].strip())
                                if "URL:" in line
                                else html.escape(line)
                            )
                            for line in notice_data.splitlines()
                        )
                        body += "</pre><hr><details><summary>Click to expand notice and/or license.</summary><pre>"
                        body += html.escape(notice["Notice"]) + "</pre></details><hr>"

                    print(HTML_TEMPLATE.format(body), file=f)
                case _:
                    ap.error(f"unknown format: {dump.format!r}")
        finally:
            if f is not sys.stdout:
                f.close()

    if failed:
        exit(1)


if __name__ == "__main__":
    main()
