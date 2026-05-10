/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/allocator.h>

static GestAllocator gestSystemAllocatorInstance = {
    .instance = NULL,
    .alloc = gestAllocSystemImpl,
};

GestAllocator gestAllocSystem() {
    return gestSystemAllocatorInstance;
}

void *gestAllocSystemImpl(
    void *unused,
    void *ptr,
    size_t oldSize,
    size_t newSize,
    size_t alignment,
    uint32_t flags
) {
    (void)unused;

    if (alignment == 0) alignment = alignof(max_align_t);

    if (ptr == NULL && newSize == 0) {
        // Supports both zeroing and remembering
        return (void *)(uintptr_t)(flags == (flags & (GEST_ALLOC_ZEROED | GEST_ALLOC_REMEMBER)));
    }

    if (newSize == 0) {
#if defined(_MSC_VER) || defined(__MINGW32__)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
        return NULL;
    }

    if (alignment & (alignment - 1)) {
        gestErrno = GEST_EINVALIGN;
        return NULL;
    }

    size_t paddedSize = (newSize + alignment - 1) & ~(alignment - 1);

    if (ptr == NULL) {
#if defined(_MSC_VER) || defined(__MINGW32__)
        ptr = _aligned_malloc(paddedSize, alignment);
#else
        ptr = aligned_alloc(alignment, paddedSize);
#endif
        goto verify_ptr_and_return;
    }

#if defined(_MSC_VER) || defined(__MINGW32__)
    ptr = _aligned_realloc(ptr, paddedSize, alignment);
#else
    if (alignment <= alignof(max_align_t)) {
        ptr = realloc(ptr, paddedSize);
        goto verify_ptr_and_return;
    }
    void *newPtr = aligned_alloc(alignment, paddedSize);
    if (newPtr) {
        memcpy(newPtr, ptr, GEST_MIN(oldSize, newSize));
        free(ptr);
        ptr = newPtr;
    }
#endif

verify_ptr_and_return:
    if (!ptr) {
        gestErrno = GEST_ENOSPACE;
    }

    if ((flags & GEST_ALLOC_ZEROED) && newSize > oldSize) {
        memset((uint8_t *)ptr + oldSize, 0, newSize - oldSize);
    }

    return ptr;
}

void *gestAllocBumpImpl(
    GestBumpAllocator *bump,
    void *ptr,
    size_t oldSize,
    size_t newSize,
    size_t alignment,
    uint32_t flags
) {
    assert(
        bump->parentAllocator != NULL
        && "To use a bump allocator, you must specify a parent allocator."
    );

    if (alignment == 0) alignment = alignof(max_align_t);

    if (!bump->stdChunkCapacity) {
        bump->stdChunkCapacity = GEST_ALLOC_BUMP_DEFAULT_STD_CHUNK_CAPACITY;
    }

    // Query:
    if (ptr == NULL && oldSize == 0 && newSize == 0) {
        // Supports both zeroing and remembering (in that it does not need the previous size because
        // it always uses more)
        return (void *)(uintptr_t)(flags == (flags & (GEST_ALLOC_ZEROED | GEST_ALLOC_REMEMBER)));
    }

    int lastAllocEliminated = 0;
    // Steal some stack allocator behaviour
    if (ptr != NULL && bump->last
        && (uint8_t *)ptr + oldSize
            == (uint8_t *)bump->last + bump->last->paddedHeaderSize + bump->last->bump) {
        bump->last->bump -= oldSize;
        lastAllocEliminated = 1;
    }

    // Free is a no-op (except the stack thing which we already do above)
    if (ptr != NULL && newSize == 0) {
        return NULL;
    }

    if (alignment & (alignment - 1)) {
        gestErrno = GEST_EINVALIGN;
        return NULL;
    }

    // For a bump allocator, there is nearly no difference between an alloc and realloc (except what
    // we did above) And alloc and realloc are the only things left
    size_t allocAlign = GEST_MAX(alignof(GestBumpAllocatorHeader), alignment);
    size_t paddedHeaderSize
        = (sizeof(GestBumpAllocatorHeader) + allocAlign - 1) & ~(allocAlign - 1);

    // If there is no chunk, place one
    if (bump->last == NULL) {
        bump->last = gestAlloc(
            bump->parentAllocator,
            bump->last,
            0,
            paddedHeaderSize + bump->stdChunkCapacity,
            allocAlign,
            0
        );
        if (!bump->last) return NULL; // Pass the error on to the caller
        *bump->last = (GestBumpAllocatorHeader){ .capacity = bump->stdChunkCapacity,
                                                 .paddedHeaderSize = paddedHeaderSize };
    }

    void *result;
    size_t alignedBump = (bump->last->bump + alignment - 1) & ~(alignment - 1);
    size_t paddedSize = (newSize + alignment - 1) & ~(alignment - 1);

    // If we need a chunk bigger than the standart
    if (paddedSize > bump->stdChunkCapacity) {
        // We are going to do the following injection:
        // - Create a new chunk with the big size called I
        // - then if the list is: ..., bump->last->previous, bump->last
        // - we do: ..., bump->last->previous, I, bump->last
        GestBumpAllocatorHeader *injectedHeader = gestAlloc(
            bump->parentAllocator,
            NULL,
            0,
            paddedHeaderSize + paddedSize,
            allocAlign,
            0
        );
        if (!injectedHeader) return NULL; // Pass the error on to the caller
        *injectedHeader = (GestBumpAllocatorHeader){
            .capacity = paddedSize,
            .bump = paddedSize,
            .previous = bump->last->previous,
            .paddedHeaderSize = paddedHeaderSize,
        };
        bump->last->previous = injectedHeader;
        result = (uint8_t *)injectedHeader + paddedHeaderSize;
        goto finalise;
    }

    // If the last chunk ran out of space
    if (alignedBump + paddedSize > bump->last->capacity) {
        GestBumpAllocatorHeader *newLast = gestAlloc(
            bump->parentAllocator,
            NULL,
            0,
            paddedHeaderSize + bump->stdChunkCapacity,
            allocAlign,
            0
        );
        if (!newLast) return NULL; // Pass the error on to the caller
        *newLast = (GestBumpAllocatorHeader){
            .capacity = bump->stdChunkCapacity,
            .paddedHeaderSize = paddedHeaderSize,
            .previous = bump->last,
        };
        bump->last = newLast;
        result = (uint8_t *)bump->last + paddedHeaderSize;
        goto finalise;
    }

    bump->last->bump = alignedBump;
    result = (uint8_t *)bump->last + bump->last->paddedHeaderSize + bump->last->bump;
    bump->last->bump += paddedSize;

finalise:
    if (result != ptr && ptr != NULL) { // if realloc
        memcpy(result, ptr, GEST_MIN(oldSize, newSize));
    }
    if ((flags & GEST_ALLOC_ZEROED) && newSize > oldSize) {
        memset((uint8_t *)result + oldSize, 0, paddedSize - oldSize);
    }

    return result;
}

GestAllocator gestAllocBump(GestBumpAllocator *bump) {
    return (GestAllocator){
        .instance = (void *)bump,
        .alloc = (GestAllocFunc)gestAllocBumpImpl,
    };
}

void gestAllocBumpFreeAll(GestBumpAllocator *bump) {
    GestBumpAllocatorHeader *current = bump->last;
    while (current) {
        GestBumpAllocatorHeader *prev = current->previous;
        // Free the entire chunk (Header + Capacity)
        gestAlloc(
            bump->parentAllocator,
            current,
            current->paddedHeaderSize + current->capacity,
            0,
            0,
            0
        );
        current = prev;
    }
    bump->last = NULL;
}
