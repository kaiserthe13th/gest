/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest-test/test.h>

GEST_TEST(bswap8) {
    GEST_ASSERT_MSG(
        gestByteSwap8(0x14) == 0x14,
        "this should be really impossible, somehow a single byte changed itself"
    );
}

GEST_TEST(bswap16) {
    GEST_ASSERT(gestByteSwap16(0x1234) == 0x3412);
}

GEST_TEST(bswap32) {
    GEST_ASSERT(gestByteSwap32(0x12345678) == 0x78563412);
}

GEST_TEST(bswap64) {
    GEST_ASSERT(gestByteSwap64(0x0123456789ABCDEFULL) == 0xEFCDAB8967452301ULL);
}

struct GesTestTest bswapTests[] = {
    GEST_PUT_TEST(bswap8),
    GEST_PUT_TEST(bswap16),
    GEST_PUT_TEST(bswap32),
    GEST_PUT_TEST(bswap64),
};

struct GesTestSuite bswapSuite = {
    .name = "bswap",
    .tests = bswapTests,
    .count = sizeof(bswapTests) / sizeof(bswapTests[0]),
};
