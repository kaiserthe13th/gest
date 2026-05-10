/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_EXPORT_H
#define GEST_EXPORT_H

// Static Builds
#if defined(GEST_STATIC)
    #define GEST_API
    #define GEST_LOCAL

// Windows / Cygwin
#elif defined(_WIN32) || defined(__CYGWIN__)
    #if defined(GEST_BUILD)
        #if defined(__GNUC__)
            #define GEST_API __attribute__((dllexport))
        #else
            #define GEST_API __declspec(dllexport)
        #endif
    #else
        #if defined(__GNUC__)
            #define GEST_API __attribute__((dllimport))
        #else
            #define GEST_API __declspec(dllimport)
        #endif
    #endif
    #define GEST_LOCAL

// Modern GCC / Clang / Intel (Unix-like)
#elif defined(__GNUC__) && (__GNUC__ >= 4) || defined(__clang__)
    #define GEST_API   __attribute__((visibility("default")))
    #define GEST_LOCAL __attribute__((visibility("hidden")))

// Sun Solaris (SunPro Compiler)
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    #define GEST_API   __global
    #define GEST_LOCAL __hidden

// Fallback for everything else (GCC < 4, etc.)
#else
    #define GEST_API
    #define GEST_LOCAL
#endif

#endif // GEST_EXPORT_H