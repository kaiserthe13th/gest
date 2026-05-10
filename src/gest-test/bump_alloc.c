/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest-test/test.h>
#include <gest/allocator.h>

GEST_TEST(bumpAlloc) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpAllocInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpAllocInst);
    GEST_ASSERT(bumpAllocInst.last == NULL);
    int *myInt = gestQuickAlloc(&bumpA, sizeof(int), alignof(int), 0);
    GEST_ASSERT(bumpAllocInst.last->capacity == GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY);
    GEST_ASSERT(bumpAllocInst.last->bump == sizeof(int));
    *myInt = 2;
    GEST_ASSERT(
        *(int *)((uint8_t *)bumpAllocInst.last + bumpAllocInst.last->paddedHeaderSize) == *myInt
    );
    *myInt *= 4;
    GEST_ASSERT(
        *(int *)((uint8_t *)bumpAllocInst.last + bumpAllocInst.last->paddedHeaderSize) == *myInt
    );
    gestAllocBumpFreeAll(&bumpAllocInst);
}

GEST_TEST(bumpAllocZeroed) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpAllocInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpAllocInst);
    GEST_ASSERT(bumpAllocInst.last == NULL);
    int *myInt = gestQuickAlloc(&bumpA, sizeof(int), alignof(int), GEST_ALLOC_ZEROED);
    GEST_ASSERT(*myInt == 0);
    gestAllocBumpFreeAll(&bumpAllocInst);
}

GEST_TEST(bumpAllocVeryBig) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpAllocInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpAllocInst);
    GEST_ASSERT(bumpAllocInst.last == NULL);
    int *myInt = gestAllocMany(&bumpA, int, GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY * 2, 0);
    GEST_ASSERT(bumpAllocInst.last->capacity == GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY);
    GEST_ASSERT(
        bumpAllocInst.last->previous->capacity
        == sizeof(int) * GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY * 2
    );
    GEST_ASSERT(
        bumpAllocInst.last->previous->bump
        == sizeof(int) * GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY * 2
    );
    gestAllocBumpFreeAll(&bumpAllocInst);
}

GEST_TEST(bumpAllocManySmall) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpAllocInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpAllocInst);
    GEST_ASSERT(bumpAllocInst.last == NULL);

    for (size_t i = 0; i < GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY / sizeof(int); i++) {
        gestAllocOne(&bumpA, int, 0);
    }

    void *previousLast = bumpAllocInst.last;
    GEST_ASSERT(previousLast != NULL);

    gestAllocOne(&bumpA, int, 0);
    GEST_ASSERT(previousLast != bumpAllocInst.last);
    GEST_ASSERT(bumpAllocInst.last != NULL);
    GEST_ASSERT(bumpAllocInst.last->previous == previousLast);

    gestAllocBumpFreeAll(&bumpAllocInst);
}

GEST_TEST(bumpAllocMixedAlignment) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpInst);

    gestQuickAlloc(&bumpA, 1, 1, 0);
    void *p = gestQuickAlloc(&bumpA, 4, 16, 0);

    GEST_ASSERT(((uintptr_t)p % 16) == 0);
    gestAllocBumpFreeAll(&bumpInst);
}

GEST_TEST(bumpAllocStackBehavior) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpInst);

    void *p1 = gestQuickAlloc(&bumpA, 16, 16, 0);
    void *p2 = gestQuickAlloc(&bumpA, 16, 16, 0);
    size_t bumpAfterP2 = bumpInst.last->bump;

    gestAlloc(&bumpA, p2, 16, 0, 16, 0);
    GEST_ASSERT(bumpInst.last->bump < bumpAfterP2);

    void *p3 = gestQuickAlloc(&bumpA, 16, 16, 0);
    GEST_ASSERT(p3 == p2);

    gestAllocBumpFreeAll(&bumpInst);
}

GEST_TEST(bumpAllocInPlaceRealloc) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpInst);

    void *p1 = gestQuickAlloc(&bumpA, 16, 16, 0);
    void *p1_new = gestAlloc(&bumpA, p1, 16, 64, 16, 0);
    GEST_ASSERT(p1 == p1_new);
    GEST_ASSERT(bumpInst.last->bump >= 64);

    gestAllocBumpFreeAll(&bumpInst);
}

GEST_TEST(bumpAllocNonLIFOSafety) {
    GestAllocator system = gestAllocSystem();
    GestBumpAllocator bumpInst = { .parentAllocator = &system };
    GestAllocator bumpA = gestAllocBump(&bumpInst);

    void *p1 = gestQuickAlloc(&bumpA, 16, 16, 0);
    void *p2 = gestQuickAlloc(&bumpA, 16, 16, 0);
    size_t bumpAfterP2 = bumpInst.last->bump;

    gestAlloc(&bumpA, p1, 16, 0, 16, 0);
    GEST_ASSERT(bumpInst.last->bump == bumpAfterP2);

    gestAllocBumpFreeAll(&bumpInst);
}

struct GesTestTest bumpAllocTests[] = {
    GEST_PUT_TEST(bumpAlloc),
    GEST_PUT_TEST(bumpAllocZeroed),
    GEST_PUT_TEST(bumpAllocVeryBig),
    GEST_PUT_TEST(bumpAllocManySmall),
    GEST_PUT_TEST(bumpAllocMixedAlignment),
    GEST_PUT_TEST(bumpAllocStackBehavior),
    GEST_PUT_TEST(bumpAllocInPlaceRealloc),
    GEST_PUT_TEST(bumpAllocNonLIFOSafety),
};

struct GesTestSuite bumpAllocSuite = {
    .name = "bumpAlloc",
    .tests = bumpAllocTests,
    .count = sizeof(bumpAllocTests) / sizeof(bumpAllocTests[0]),
};
