/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu
///
/// MSVC C11 Polyfill
///
/// Fixes missing definitions in the Windows SDK/UCRT headers.

#ifndef GEST_POLYFILLS_MSVC
#define GEST_POLYFILLS_MSVC

#ifdef __STDC_NO_ATOMICS__
    #undef __STDC_NO_ATOMICS__
#endif

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>

#if defined(_MSC_VER)
    #ifndef _MAX_ALIGN_T_DEFINED
typedef union {
    long double __ld;
    long long __ll;
    double __d;
    void *__p;
    long long __max_align_nonce;
} max_align_t;
        #define _MAX_ALIGN_T_DEFINED
static_assert(
    alignof(max_align_t) == alignof(double),
    "max_align_t is not the same alignment as double"
);
    #endif

    typedef ptrdiff_t ssize_t;

    #ifdef INTELLISENSE_VSCODE
        #ifdef __cplusplus
            #define _Atomic(T) std::atomic<T>
        #else
            #if !defined(_Atomic)
                #define _Atomic(T) T
            #endif
        #endif
    #endif
#endif

#endif // GEST_POLYFILLS_MSVC
