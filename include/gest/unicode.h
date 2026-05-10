/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/_generated/_unicode.h>
#include <gest/export.h>
#include <stddef.h>
#include <stdint.h>

#define GEST_U32C_EOF ((uint32_t)-1)

typedef struct GestCodepointIterator GestCodepointIterator;
struct GestCodepointIterator {
    const uint8_t *source;
    size_t length;
};

#define GestCodepointIteratorZ(s) \
    ((GestCodepointIterator){ .source = (uint8_t *)(s), .length = strlen(s) })

uint32_t gestUniNextU32Char(GestCodepointIterator *iter, int lossy);
