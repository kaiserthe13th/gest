/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/error.h>
#include <gest/unicode.h>
#include <gest/str.h>

#define GEST_UTF8_1BYTE_SEQ_PR 0x00
#define GEST_UTF8_2BYTE_SEQ_PR 0x80
#define GEST_UTF8_3BYTE_SEQ_PR 0xE0
#define GEST_UTF8_4BYTE_SEQ_PR 0xF0
#define GEST_CONTINUATION_BYTE 0x80

uint32_t gestUniNextU32Char(struct GestStr *iter, int lossy) {
    while (1) {
        gestErrno = GEST_ENONE;
        if (iter->length <= 0) return GEST_U32C_EOF;

        uint32_t codepoint;
        int bytes;

        uint8_t c0 = iter->chars[0];
        // Learn codepoint length
        if ((c0 & GEST_UTF8_2BYTE_SEQ_PR) == GEST_UTF8_1BYTE_SEQ_PR) {
            // It is ascii
            codepoint = c0;
            bytes = 1;
        } else if ((c0 & GEST_UTF8_3BYTE_SEQ_PR) == GEST_UTF8_4BYTE_SEQ_PR) {
            codepoint = c0 & 0x1F;
            bytes = 2;
        } else if ((c0 & GEST_UTF8_4BYTE_SEQ_PR) == GEST_UTF8_3BYTE_SEQ_PR) {
            codepoint = c0 & 0x0F;
            bytes = 3;
        } else if ((c0 & 0xF8) == GEST_UTF8_4BYTE_SEQ_PR) {
            codepoint = c0 & 0x07;
            bytes = 4;
        } else {
            iter->chars++;
            iter->length--;
            if (lossy) {
                continue;
            }
            gestErrno = GEST_EILSEQ;
            return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
        }

        if (iter->length < bytes) {
            iter->chars++;
            iter->length--;
            if (lossy) {
                continue;
            }
            gestErrno = GEST_ESMALL;
            return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
        }

        // Extract continuation bytes (10xxxxxx)
        for (size_t i = 1; i < bytes; i++) {
            if ((iter->chars[i] & 0xC0) != GEST_CONTINUATION_BYTE) {
                iter->chars += i;
                iter->length -= i;

                if (lossy) {
                    continue;
                }

                gestErrno = GEST_EINVAL;
                return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
            }
            codepoint = (codepoint << 6) | (iter->chars[i] & 0x3F);
        }

        int isOverlong = (bytes == 2 && codepoint < 0x80) || (bytes == 3 && codepoint < 0x800)
            || (bytes == 4 && codepoint < 0x10000);
        int isSurrogate = codepoint >= 0xD800 && codepoint < 0xE000;
        int isBeyondLimit = codepoint > 0x10FFFF;
        if (isOverlong || isSurrogate || isBeyondLimit) {
            iter->chars += bytes;
            iter->length -= bytes;

            if (lossy) {
                continue;
            }

            gestErrno = GEST_EINVAL;
            return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
        }

        iter->chars += bytes;
        iter->length -= bytes;

        return codepoint;
    }
}

struct GestUTF8EncodedChar__ gestUniC32EncodeUTF8(GestU32Char u32c) {
    struct GestUTF8EncodedChar__ result = {0};

    if (u32c < 0x80) {
        result.length = 1;
        result.enc[0] = u32c;
        return result;
    } else if (u32c < 0x800) {
        result.length = 2;
        result.enc[0] = GEST_UTF8_2BYTE_SEQ_PR | (u32c >> 6);
        result.enc[1] = GEST_CONTINUATION_BYTE | (u32c & 0x3F);
    } else if (u32c < 0x10000) {
        result.length = 3;
        result.enc[0] = GEST_UTF8_3BYTE_SEQ_PR | (u32c >> 12);
        result.enc[1] = GEST_CONTINUATION_BYTE | ((u32c >> 6) & 0x3F);
        result.enc[2] = GEST_CONTINUATION_BYTE | (u32c & 0x3F);
    } else if (u32c < 0x10FFFF) {
        result.length = 4;
        result.enc[0] = GEST_UTF8_4BYTE_SEQ_PR | (u32c >> 18);
        result.enc[1] = GEST_CONTINUATION_BYTE | ((u32c >> 12) & 0x3F);
        result.enc[2] = GEST_CONTINUATION_BYTE | ((u32c >> 6) & 0x3F);
        result.enc[3] = GEST_CONTINUATION_BYTE | (u32c & 0x3F);
    }

    return result;
}
