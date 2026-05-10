/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/nanbox.h>

uint64_t GEST_QNAN = GEST_QNAN_MASK;
uint64_t GEST_NANBOX = GEST_NANBOX_MASK;

void gestInitialiseNanBoxPattern(void) {
    volatile double naturalNan = NAN;
    uint64_t nanBits;
    memcpy(&nanBits, (double *)&naturalNan, sizeof(naturalNan));
    // Handle old MIPS-style quiet NaNs as well:
    // The mask here does the following:
    // takes the inverse of the quiet bit surrounded by zeros,
    // and inverts that to punch a hole via an and-mask if the quiet bit was zero.
    GEST_QNAN = GEST_QNAN_MASK & ~(~nanBits & (1ULL << GEST_QUIET_BIT));
    GEST_NANBOX = GEST_QNAN | (1ULL << GEST_SIGN_BIT);
}
