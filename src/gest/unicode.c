/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/error.h>
#include <gest/unicode.h>

uint32_t gestUniNextU32Char(GestCodepointIterator *iter, int lossy) {
    while (1) {
        gestErrno = GEST_ENONE;
        if (iter->length <= 0) return GEST_U32C_EOF;

        uint32_t codepoint;
        int bytes;

        uint8_t c0 = iter->source[0];
        // Learn codepoint length
        if ((c0 & 0x80) == 0) {
            // It is ascii
            codepoint = c0;
            bytes = 1;
        } else if ((c0 & 0xE0) == 0xC0) {
            codepoint = c0 & 0x1F;
            bytes = 2;
        } else if ((c0 & 0xF0) == 0xE0) {
            codepoint = c0 & 0x0F;
            bytes = 3;
        } else if ((c0 & 0xF8) == 0xF0) {
            codepoint = c0 & 0x07;
            bytes = 4;
        } else {
            iter->source++;
            iter->length--;
            if (lossy) {
                continue;
            }
            gestErrno = GEST_EILSEQ;
            return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
        }

        if (iter->length < bytes) {
            iter->source++;
            iter->length--;
            if (lossy) {
                continue;
            }
            gestErrno = GEST_ESMALL;
            return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
        }

        // Extract continuation bytes (10xxxxxx)
        for (size_t i = 1; i < bytes; i++) {
            if ((iter->source[i] & 0xC0) != 0x80) {
                iter->source += i;
                iter->length -= i;

                if (lossy) {
                    continue;
                }

                gestErrno = GEST_EINVAL;
                return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
            }
            codepoint = (codepoint << 6) | (iter->source[i] & 0x3F);
        }

        int isOverlong = (bytes == 2 && codepoint < 0x80) || (bytes == 3 && codepoint < 0x800)
            || (bytes == 4 && codepoint < 0x10000);
        int isSurrogate = codepoint >= 0xD800 && codepoint < 0xE000;
        int isBeyondLimit = codepoint > 0x10FFFF;
        if (isOverlong || isSurrogate || isBeyondLimit) {
            iter->source += bytes;
            iter->length -= bytes;

            if (lossy) {
                continue;
            }

            gestErrno = GEST_EINVAL;
            return GEST_UNI_U32C_REPLACEMENT_CHARACTER;
        }

        iter->source += bytes;
        iter->length -= bytes;

        return codepoint;
    }
}
