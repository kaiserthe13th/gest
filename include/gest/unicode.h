/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/_generated/_unicode.h>
#include <gest/export.h>
#include <stddef.h>
#include <stdint.h>
#include <gest/str.h>

#define GEST_U32C_EOF ((uint32_t)-1)

typedef uint32_t GestU32Char;

uint32_t gestUniNextU32Char(GestBaseStrIter *iter, int lossy);
struct GestUTF8EncodedChar__ {
    uint8_t length;
    GestU32Char enc[4];
} gestUniC32EncodeUTF8(GestU32Char u32c);
