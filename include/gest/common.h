/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_COMMON_H
#define GEST_COMMON_H

#include <gest/endian.h>
#include <gest/export.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION)
    #define IS_POSIX 1
#else
    #define IS_POSIX 0
#endif

#define GEST_STRIFY__(x) #x
/// Stringifies the input
#define GEST_STRIFY(x)      GEST_STRIFY__(x)
#define GEST_CONCAT__(x, y) x##y
/// Concatenates the parts
#define GEST_CONCAT(x, y) GEST_CONCAT__(x, y)

#define GESTX_ENUM(id)             id,
#define GESTX_ENUM_PAIR(id, value) id = value,
#define GESTX_ENUM_NAME_CASE(id) \
case id: return GEST_STRIFY(id);
#define GESTX_ENUM_NAME_CASE_PAIR(id, value) \
case id: return GEST_STRIFY(id);

#define GESTX_ENUM_WRAP(id, wrap)             wrap(id),
#define GESTX_ENUM_WRAP_PAIR(id, wrap, value) wrap(id) = value,
#define GESTX_ENUM_WRAP_NAME_CASE(id, wrap) \
case wrap(id): return GEST_STRIFY(id);
#define GESTX_ENUM_WRAP_NAME_CASE_PAIR(id, wrap, value) \
case wrap(id): return GEST_STRIFY(id);

enum {
#define GESTX_VERCHAN_WRAP(x) GEST_VERCHAN_##x
#define GESTX_VERCHAN_PAIRS(X) \
    X(UNKNOWN, GESTX_VERCHAN_WRAP, 0) \
    X(NIGHTLY, GESTX_VERCHAN_WRAP, 1) \
    X(ALPHA, GESTX_VERCHAN_WRAP, 2) \
    X(BETA, GESTX_VERCHAN_WRAP, 3) \
    X(RELEASE_CAND, GESTX_VERCHAN_WRAP, 4) \
    X(STABLE, GESTX_VERCHAN_WRAP, 0xF)

    GESTX_VERCHAN_PAIRS(GESTX_ENUM_WRAP_PAIR)
};

/// Creates a version bitfield from the major, minor, patch, channel and candidate
#define GEST_VER(major, minor, patch, channel, candidate) \
    (((major) << 24) | ((minor) << 16) | ((patch) << 8) | ((channel) << 4) | (candidate))

#ifndef GEST_VERSION_MAJOR
    #define GEST_VERSION_MAJOR 0
#endif
#ifndef GEST_VERSION_MINOR
    #define GEST_VERSION_MINOR 0
#endif
#ifndef GEST_VERSION_PATCH
    #define GEST_VERSION_PATCH 0
#endif
#ifndef GEST_VERSION_CHANNEL
    #define GEST_VERSION_CHANNEL GEST_VERCHAN_UNKNOWN
#endif
#ifndef GEST_VERSION_CANDIDATE
    #define GEST_VERSION_CANDIDATE 0
#endif

#define GEST_VERSION \
    GEST_VER( \
        GEST_VERSION_MAJOR, \
        GEST_VERSION_MINOR, \
        GEST_VERSION_PATCH, \
        GEST_VERSION_CHANNEL, \
        GEST_VERSION_CANDIDATE \
    )

#define GEST_MAX(a, b) ((a) >= (b) ? (a) : (b))
#define GEST_MIN(a, b) ((a) <= (b) ? (a) : (b))

#define GEST_ROTL(x, bits) (((x) << (bits)) | ((x) >> (sizeof(x) * 8 - (bits))))
#define GEST_ROTR(x, bits) GEST_ROTL(x, sizeof(x) * 8 - (bits))

#endif // GEST_COMMON_H
