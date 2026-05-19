/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_STR_H
#define GEST_STR_H

#include <assert.h>
#include <gest/allocator.h>
#include <gest/common.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

typedef struct GestStr GestStr;
typedef struct GestStrBuf GestStrBuf;

struct GestStr {
    size_t length;
    const uint8_t *chars;
};

#define gstrlit(s) ((GestStr){sizeof(s) - 1, s})

struct GestStrBuf {
    GestAllocator *alloc;
    size_t capacity;
    size_t length;
    uint8_t *chars;
};

typedef struct GestStr GestBaseStrIter;

inline static GestBaseStrIter gestStrToBaseIter(GestStr s) {
    return s;
}

inline static GestStrBuf gestEmptyStrBuf(GestAllocator *alloc) {
    return (GestStrBuf){.alloc = alloc};
}

inline static GestStrBuf gestStrBufFromStr(GestAllocator *alloc, GestStr s) {
    char *chars = gestAllocMany(&alloc, char, s.length, 0);
    memcpy(chars, s.chars, s.length);
    return (GestStrBuf){ .alloc = alloc, .capacity = s.length, .chars = chars, .length = s.length };
}

inline static GestStr gestStrFromStrBuf(GestStrBuf buf) {
    return (GestStr){.length = buf.length, .chars = buf.chars ? buf.chars : ""};
}

/// Finds the index at which `needle` is first encountered within `haystack`.
/// @param haystack The string being searched inside of.
/// @param needle The substring that is being searched.
/// @return the index at which in the haystack that the needle was found starting at if found, `-1` if not found
GEST_API ssize_t gestStrIndexSubstr(GestStr haystack, GestStr needle);
/// Checks whether `haystack` starts with `needle`.
/// @param haystack The string being searched inside of.
/// @param needle The substring that is being searched.
/// @return `1` if true, `0` if false
GEST_API int gestStrBeginsWith(GestStr haystack, GestStr needle);
/// Checks whether `haystack` ends with `needle`.
/// @param haystack The string being searched inside of.
/// @param needle The substring that is being searched.
/// @return `1` if true, `0` if false
GEST_API int gestStrEndsWith(GestStr haystack, GestStr needle);
/// If `haystack` starts with `needle`, returns a substring of haystack which skips it.
/// @param haystack The string being searched inside of.
/// @param needle The substring that is being searched.
/// @return The substring, or the haystack itself if nothing was stripped
GEST_API GestStr gestStrStripPrefix(GestStr haystack, GestStr needle);
/// If `haystack` ends with `needle`, returns a substring of haystack which stops just before it.
/// @param haystack The string being searched inside of.
/// @param needle The substring that is being searched.
/// @return The substring, or the haystack itself if nothing was stripped
GEST_API GestStr gestStrStripPostfix(GestStr haystack, GestStr needle);

GEST_API int gestStrBufPushU32Char(GestStrBuf *buf, GestU32Char codepoint);
GEST_API int gestStrBufPushStr(GestStrBuf *buf, const GestStr s);

#endif // GEST_STR_H
