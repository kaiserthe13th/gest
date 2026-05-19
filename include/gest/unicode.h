/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_NOGEN_UNICODE_H
#define GEST_NOGEN_UNICODE_H

#include <gest/_generated/_unicode.h>
#include <gest/export.h>
#include <stddef.h>
#include <stdint.h>

#define GEST_U32C_EOF ((uint32_t)-1)

typedef uint32_t GestU32Char;

struct GestStr;

uint32_t gestUniNextU32Char(struct GestStr *iter, int lossy);

struct GestUTF8EncodedChar__ {
    uint8_t length;
    uint8_t enc[4];
};

struct GestUTF8EncodedChar__ gestUniC32EncodeUTF8(GestU32Char u32c);

#endif // GEST_NOGEN_UNICODE_H