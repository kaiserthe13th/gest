/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/error.h>
#include <gest/str.h>
#include <gest/unicode.h>

ssize_t gestStrIndexSubstr(GestStr haystack, GestStr needle) {
    if (needle.length == 0) return 0;
    if (needle.length > haystack.length) return -1;

#ifdef _DEBUG
    assert(haystack.chars && needle.chars);
#endif // _DEBUG

    size_t diff = haystack.length - needle.length;
    for (size_t i = 0; i <= diff; i++) {
        if (haystack.chars[i] == needle.chars[0]
            && memcmp(&haystack.chars[i], needle.chars, needle.length) == 0) {
            return i;
        }
    }
    return -1;
}

int gestStrBeginsWith(GestStr haystack, GestStr needle) {
    if (needle.length == 0) return 1;
    if (needle.length > haystack.length) return 0;

#ifdef _DEBUG
    assert(haystack.chars && needle.chars);
#endif // _DEBUG

    return !memcmp(haystack.chars, needle.chars, needle.length);
}

int gestStrEndsWith(GestStr haystack, GestStr needle) {
    if (needle.length == 0) return 1;
    if (needle.length > haystack.length) return 0;

#ifdef _DEBUG
    assert(haystack.chars && needle.chars);
#endif // _DEBUG

    return !memcmp(&haystack.chars[haystack.length - needle.length], needle.chars, needle.length);
}

GestStr gestStrStripPrefix(GestStr haystack, GestStr needle) {
    if (gestStrBeginsWith(haystack, needle)) {
        return (GestStr){ .length = haystack.length - needle.length,
                          .chars = &haystack.chars[needle.length] };
    }
    return haystack;
}

GestStr gestStrStripPostfix(GestStr haystack, GestStr needle) {
    if (gestStrEndsWith(haystack, needle)) {
        return (GestStr){ .length = haystack.length - needle.length, .chars = haystack.chars };
    }
    return haystack;
}

int gestStrBufPushU32Char(GestStrBuf *buf, GestU32Char codepoint) {
    struct GestUTF8EncodedChar__ utf8EncCh = gestUniC32EncodeUTF8(codepoint);
    const GestStr utf8Str = { .chars = utf8EncCh.enc, .length = utf8EncCh.length };
    return gestStrBufPushStr(buf, utf8Str);
}

inline int gestStrBufGrowIfNeeded__(GestStrBuf *buf, size_t requiredCapacity) {
    gestErrno = GEST_ENONE;
    size_t newCapacity = buf->capacity;
    while (requiredCapacity > newCapacity) {
        if (newCapacity >= SIZE_MAX / 2) return gestErrno = GEST_EARITH;
        newCapacity = newCapacity * 2 + 1;
    }
    if (newCapacity != buf->capacity) {
        uint8_t *newChars = gestAlloc(
            buf->alloc,
            buf->chars,
            buf->capacity * sizeof(uint8_t),
            newCapacity * sizeof(uint8_t),
            alignof(uint8_t),
            0
        );
        if (!newChars) return gestErrno;
        buf->chars = newChars;
        buf->capacity = newCapacity;
    }
    return GEST_ENONE;
}

int gestStrBufPushStr(GestStrBuf *buf, const GestStr s) {
    gestErrno = GEST_ENONE;
    // No-op for 0-length strings
    if (s.length == 0) return GEST_ENONE;
    if (s.length > SIZE_MAX - buf->length) return gestErrno = GEST_EARITH;
    if (gestStrBufGrowIfNeeded__(buf, buf->length + s.length)) return gestErrno;

    memcpy(&buf->chars[buf->length], s.chars, s.length);
    buf->length += s.length;
    return GEST_ENONE;
}
