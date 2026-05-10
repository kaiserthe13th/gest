#!/usr/bin/env -S python
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import os
import sys

import requests
import urllib.parse

units = ("B", "KiB", "MiB", "GiB")


def download_file(index, count, name, url, filename):
    # 1. Send a GET request with stream=True
    response = requests.get(url, stream=True)
    basename = os.path.basename(filename)

    # Get the total file size from headers (in bytes)
    expected_size = int(response.headers.get("content-length", -1))
    unit = 0
    while expected_size > 1024 and unit + 1 < len(units):
        expected_size /= 1024
        unit += 1

    if expected_size != -1:
        print(
            end=f"[{index}/{count}] Downloading {name} [{basename}]: Expected {expected_size:.2f}{units[unit]}... "
        )
    else:
        print(end=f"[{index}/{count}] Downloading {name} [{basename}]: Expected ?B... ")

    block_size = 1024
    final_size = 0
    with open(filename, "wb") as file:
        for data in response.iter_content(block_size):
            final_size += len(data)
            file.write(data)

    unit = 0
    while final_size > 1024 and unit + 1 < len(units):
        final_size /= 1024
        unit += 1

    print(f"Done! (Final Size: {final_size:.2f}{units[unit]}).")


def main(dir):
    BASE_URL_PATH = "https://www.unicode.org/Public/17.0.0/ucd/"

    files = [
        (
            "Composition Exclusions",
            "CompositionExclusions.txt",
            "CompositionExclusions.txt",
        ),
        (
            "Derived Normalization Props",
            "DerivedNormalizationProps.txt",
            "DerivedNormalizationProps.txt",
        ),
        (
            "Derived Core Properties",
            "DerivedCoreProperties.txt",
            "DerivedCoreProperties.txt",
        ),
        ("Unicode Data", "UnicodeData.txt", "UnicodeData.txt"),
    ]

    for i, (name, urlpath, localfilename) in enumerate(files):
        download_file(
            i + 1,
            len(files),
            name,
            urllib.parse.urljoin(BASE_URL_PATH, urlpath),
            os.path.join(dir, localfilename),
        )


if __name__ == "__main__":
    main(sys.argv[1])
