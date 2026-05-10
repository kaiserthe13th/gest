/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest-test/test.h>
#include <gest/common.h>
#include <signal.h>

atomic_int testCount__ = 0;
atomic_int failCount__ = 0;
thread_local struct GesTestContext *lastTestContext__;
thread_local jmp_buf testJumpBuffer__;

void gestestSignalHandler(int sig) {
    if (lastTestContext__) {
        lastTestContext__->errorCode = sig;
        lastTestContext__->message = "Segmentation Fault";
        lastTestContext__->condText = "SIGSEGV";
        longjmp(testJumpBuffer__, 1);
    }
}

void gestSetupTests(void) {
    signal(SIGSEGV, gestestSignalHandler);
}

int gestestRunSuite(struct GesTestSuite *const suite) {
    if (suite->count == 0) {
        struct GesTestTest *curr = &suite->tests[0];
        int length;
        for (length = 0; curr->name != NULL; length++, curr++);
        suite->count = length;
    }
    if (!suite->tests_added__) {
        atomic_fetch_add_explicit(&testCount__, suite->count, memory_order_acq_rel);
    }
    printf("[%s] Running %d tests...\n", suite->name, suite->count);
    int fail = 0;
    for (int i = 0; i < suite->count; i++) {
        struct GesTestTest *curr = &suite->tests[i];
        struct GesTestContext context = { 0 };
        lastTestContext__ = &context;

        printf("[%s.%s] Running... \t\t", suite->name, curr->name);
        int sigMode = 0;
        if (setjmp(testJumpBuffer__) == 0) {
            curr->test(&context);
        } else {
            sigMode = 1;
        }
        if (context.errorCode) {
            fail++;
            printf("FAIL\n");
            if (sigMode)
                printf(
                    "(unknown):(unknown):\n  --> Exception: %s: %s\n",
                    context.condText,
                    context.message
                );
            else
                printf(
                    "%s:%d:\n  --> Assertion Failed: %s: %s\n",
                    context.file,
                    context.line,
                    context.condText,
                    context.message
                );
        } else {
            printf("pass\n");
        }
    }
    if (fail) {
        printf("[%s] There are %d FAILING tests.\n", suite->name, fail);
    }
    atomic_fetch_add_explicit(&failCount__, fail, memory_order_acq_rel);
    printf("[%s] %d/%d tests passed.\n", suite->name, suite->count - fail, suite->count);
    return fail;
}

int gestFinishTests(void) {
    int failCount = atomic_load_explicit(&failCount__, memory_order_acquire);
    int testCount = atomic_load_explicit(&testCount__, memory_order_acquire);
    printf("################################################################\n");

    if (failCount) {
        printf(":: There are %d FAILING tests.\n", failCount);
    }

    printf(":: %d/%d tests passed.\n", testCount - failCount, testCount);

    return failCount;
}
