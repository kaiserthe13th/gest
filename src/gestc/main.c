/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/gest.h>
#include <gest/nanbox.h>
#include <stdio.h>

int main() {
    gestInitialiseNanBoxPattern();
    assert((gestGetEndianFloat() != -1) && "gestc: unsupported float layout");
    assert(
        !gestIsNaNCanonicalised()
        && "gestc: platform does NaN canonicalisation: cannot guarantee lossless data"
    );

    printf("%08x\n", gestVersion());
    printf("%s\n", gestVersionString());
    printf("platform qnan = %08llx\n", GEST_QNAN);
    printf("platform nanbox = %08llx\n", GEST_NANBOX);
}
