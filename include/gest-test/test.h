/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_TEST_TEST_H
#define GEST_TEST_TEST_H

#include <gest/common.h>
#include <setjmp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <threads.h>

struct GesTestContext {
    int errorCode;
    int line;
    const char *file;
    const char *condText;
    const char *message;
};

struct GesTestTest {
    const char *name;
    void (*test)(struct GesTestContext *const testContext__);
};

struct GesTestSuite {
    int count;
    int tests_added__;
    struct GesTestTest *tests;
    const char *name;
};

#define GEST_TEST(name) void GEST_CONCAT(gestest_, name)(struct GesTestContext *const testContext__)
#define GEST_ASSERT_MSG(condition, msg) \
    do { \
        if (!(condition)) { \
            testContext__->errorCode = 1; \
            testContext__->line = __LINE__; \
            testContext__->file = __FILE__; \
            testContext__->condText = GEST_STRIFY(condition); \
            testContext__->message = (msg); \
            return; \
        } \
    } while (0)
#define GEST_ASSERT(condition) GEST_ASSERT_MSG(condition, "condition not fulfilled")
#define GEST_PUT_TESTT(name) \
    ((struct GesTestTest){ GEST_STRIFY(name), GEST_CONCAT(gestest_, name) })
#define GEST_PUT_TEST(name) { GEST_STRIFY(name), GEST_CONCAT(gestest_, name) }
#define GEST_END_TESTS      ((struct GesTestTest){ 0 })

GEST_API void gestSetupTests(void);
GEST_API int gestestRunSuite(struct GesTestSuite *suite);
GEST_API int gestFinishTests(void);

extern atomic_int testCount__;
extern atomic_int failCount__;

#endif // GEST_TEST_TEST_H
