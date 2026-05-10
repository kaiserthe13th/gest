/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest-test/test.h>
#include <gest/endian.h>

extern struct GesTestSuite bswapSuite;
extern struct GesTestSuite bumpAllocSuite;

GEST_TEST(twoPlusTwo) {
    GEST_ASSERT_MSG(2 + 2 == 4, "broken math");
}

GEST_TEST(nonMixedEndianFloats) {
    GEST_ASSERT_MSG(gestGetEndianFloat() != -1, "unsupported float endianness");
}

int main() {
    printf("Testing...\n");

    struct GesTestTest basicInvariants[] = {
        GEST_PUT_TEST(twoPlusTwo),
        GEST_PUT_TEST(nonMixedEndianFloats),
    };

    struct GesTestSuite testestSuite = {
        .name = "basicInvariants",
        .tests = basicInvariants,
        .count = sizeof(basicInvariants) / sizeof(basicInvariants[0]),
    };

    gestestRunSuite(&testestSuite);
    gestestRunSuite(&bswapSuite);
    gestestRunSuite(&bumpAllocSuite);

    return gestFinishTests();
}
