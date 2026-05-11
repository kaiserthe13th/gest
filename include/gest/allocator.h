/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_ALLOCATOR_H
#define GEST_ALLOCATOR_H

#include <gest/common.h>
#include <gest/error.h>

#include <errno.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct GestAllocator GestAllocator;
typedef struct GestBumpAllocator GestBumpAllocator;
typedef struct GestBumpAllocatorHeader GestBumpAllocatorHeader;

//// Zeroes out the allocated buffer
#define GEST_ALLOC_ZEROED (1 << 0)
/// Remember the values attached to the pointer, such that when freeing, you don't need to heed the
/// size and alignment arguments (0, 0) is accepted.
#define GEST_ALLOC_REMEMBER (1 << 1)

/// The unified allocator function for all memory operations.
///
/// This function serves as the single entry point for allocation, reallocation,
/// deallocation, and feature querying.
///
/// @param self         The allocator instance.
/// @param ptr          The pointer to operate on. Set to NULL for new allocations or queries.
/// @param oldSize      The current size of the buffer at `ptr`. Must be 0 if `ptr` is NULL.
/// @param newSize      The requested size. Set to 0 to free the memory at `ptr`.
/// @param alignment    The alignment of the buffer. Set to 0 for allocator default. Must be a power
/// of two.
/// @param flags        Behavioral flags (e.g., GEST_ALLOC_ZEROED, GEST_ALLOC_REMEMBER).
///
/// @return             Returns a pointer to the memory on success, or NULL on failure/failed query.
///
/// NOTES:
///
/// - To Allocate:      Set `ptr == NULL`, `oldSize = 0`, and `newSize > 0`.
///
/// - To Free:          Set `ptr != NULL`, `oldSize = actualSize`, and `newSize = 0`.
///
/// - To Reallocate:    Set `ptr != NULL`, `oldSize = actualSize`, and `newSize = requestedSize`.
///
/// - To Query:         Set `ptr == NULL`, `oldSize = 0`, and `newSize = 0`. Use `flags` to check
/// support.
///
/// - Implementors must set `gestErrno = GEST_EUNSUP` for unsupported flags or alignment.
///
/// - Implementors must set `gestErrno = GEST_EINVALIGN` if size/alignment constraints are violated.
///
/// EXAMPLES:
///
/// - Allocation & Sized Free:
///   ```c
///   ... // Allocate 64 bytes aligned to 16
///   void *p = gestAlloc(&alloc, NULL, 0, 64, 16, GEST_ALLOC_ZEROED);
///   ...
///   ... // Free requires the known size and alignment
///   gestAlloc(&alloc, p, 64, 0, 16, 0);
///   ```
///
/// - Reallocation:
///   ```c
///   ... // Grow from 64 to 128 bytes
///   p = gestAlloc(&alloc, p, 64, 128, 16, 0);
///   ```
///
/// - Feature Query:
///   ```c
///   ... // Check if the allocator supports remembering sizes (returns non-NULL if supported)
///   if (gestAlloc(&alloc, NULL, 0, 0, 0, GEST_ALLOC_REMEMBER)) {
///       ... // Supports GEST_ALLOC_REMEMBER
///   }
///   ```
typedef void *(*GestAllocFunc)(
    void *self,
    void *ptr,
    size_t oldSize,
    size_t newSize,
    size_t alignment,
    uint32_t flags
);

struct GestAllocator {
    void *instance;
    GestAllocFunc alloc;
};

static inline void *gestAlloc(
    GestAllocator *allocator,
    void *ptr,
    size_t oldSize,
    size_t newSize,
    size_t alignment,
    uint32_t flags
) {
    return allocator->alloc(allocator->instance, ptr, oldSize, newSize, alignment, flags);
}

static inline void *
gestQuickAlloc(GestAllocator *allocator, size_t size, size_t alignment, uint32_t flags) {
    return gestAlloc(allocator, NULL, 0, size, alignment, flags);
}

static inline void *
gestDupe__(GestAllocator *allocator, void *data, size_t size, size_t alignment, uint32_t flags) {
    void *p = gestQuickAlloc(allocator, size, alignment, flags);
    if (p) return memcpy(p, data, size);
    return NULL;
}

#define gestDupe(allocator, data, flags) \
    gestDupe__((allocator), (void *)(data), sizeof(*(data)), alignof(*(data)), (flags))
#define gestAllocOne(allocator, ty, flags) \
    gestQuickAlloc((allocator), sizeof(ty), alignof(ty), (flags))
#define gestAllocMany(allocator, ty, n, flags) \
    gestQuickAlloc((allocator), sizeof(ty) * (n), alignof(ty), (flags))
#define gestFreeOne(allocator, data) \
    gestAlloc((allocator), (data), sizeof(*(data)), 0, alignof(*(data)), 0)
#define gestFreeMany(allocator, data, n) \
    gestAlloc((allocator), (data), sizeof(*(data)) * (n), 0, alignof(*(data)), 0)

static inline void *gestQuery(GestAllocator *allocator, uint32_t flags) {
    return gestAlloc(allocator, NULL, 0, 0, 0, flags);
}

GEST_API void *gestAllocSystemImpl(
    void *unused,
    void *ptr,
    size_t oldSize,
    size_t newSize,
    size_t alignment,
    uint32_t flags
);
GEST_API GestAllocator gestAllocSystem();

#define GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY 4096

struct GestBumpAllocatorHeader {
    size_t capacity;
    size_t bump;
    size_t paddedHeaderSize;
    GestBumpAllocatorHeader *previous;
    // uint8_t bytes[(capacity + alignof(GestBumpAllocatorHeader) - 1) &
    // ~(alignof(GestBumpAllocatorHeader) - 1)];
};

struct GestBumpAllocator {
    GestAllocator *parentAllocator;
    size_t stdChunkCapacity;
    size_t reserved0__;
    GestBumpAllocatorHeader *last;
};

GEST_API void *gestAllocBumpImpl(
    GestBumpAllocator *bump,
    void *ptr,
    size_t oldSize,
    size_t newSize,
    size_t alignment,
    uint32_t flags
);
GEST_API GestAllocator gestAllocBump(GestBumpAllocator *bump);
GEST_API void gestAllocBumpFreeAll(GestBumpAllocator *bump);

#endif // GEST_ALLOCATOR_H
