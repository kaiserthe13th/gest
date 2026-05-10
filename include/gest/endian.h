/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_ENDIAN_H
#define GEST_ENDIAN_H

#include <assert.h>
#include <float.h>
#include <stdint.h>
#include <string.h>

static_assert(((uint16_t)0x1234) == 0x1234, "conversion sanity check: literal must match itself");

/// Byte swap operation
/// @param i Input value
/// @return Returns reversed byte order of the input.
static inline uint8_t gestByteSwap8(uint8_t i) {
    return i;
}

/// Byte swap operation
/// @param i Input value
/// @return Returns reversed byte order of the input.
static inline uint16_t gestByteSwap16(uint16_t i) {
    union {
        uint16_t i;
        uint8_t p[2];
    } p = { .i = i };
    uint8_t b0 = gestByteSwap8(p.p[0]), b1 = gestByteSwap8(p.p[1]);
    p.p[0] = b1, p.p[1] = b0;
    return p.i;
}

/// Byte swap operation
/// @param i Input value
/// @return Returns reversed byte order of the input.
static inline uint32_t gestByteSwap32(uint32_t i) {
    union {
        uint32_t i;
        uint16_t p[2];
    } p = { .i = i };
    uint32_t w0 = gestByteSwap16(p.p[0]), w1 = gestByteSwap16(p.p[1]);
    p.p[0] = w1, p.p[1] = w0;
    return p.i;
}

/// Byte swap operation
/// @param i Input value
/// @return Returns reversed byte order of the input.
static inline uint64_t gestByteSwap64(uint64_t i) {
    union {
        uint64_t i;
        uint32_t p[2];
    } p = { .i = i };
    uint32_t dw0 = gestByteSwap32(p.p[0]), dw1 = gestByteSwap32(p.p[1]);
    p.p[0] = dw1, p.p[1] = dw0;
    return p.i;
}

#define GEST_BIG_ENDIAN    0
#define GEST_LITTLE_ENDIAN 1

/// Checks the endianness of the integer architecture.
///
/// This function determines whether the system uses Little Endian or Big Endian
/// byte ordering for integers. It does this by inspecting the byte representation
/// of the 16-bit integer `0x1234`.
///
/// @return Returns `GEST_BIG_ENDIAN` if the system is Little Endian, or `GEST_LITTLE_ENDIAN` if it
/// is Big Endian.
static inline int gestGetEndian(void) {
    union {
        uint16_t i;
        uint8_t p[2];
    } e = { .i = 0x1234 };
    return e.p[0] != 0x12;
}

/// Checks the endianness of the floating-point architecture.
///
/// This function attempts to determine the endianness specific to float types (IEEE 754 single
/// precision). While float endianness often matches integer endianness, some historic (like ARM
/// FPA) or exotic architectures may differ. It compares the byte representation of `1.0f` against
/// known Big Endian and Little Endian patterns.
///
/// @return Returns `GEST_BIG_ENDIAN` if Big Endian, `GEST_LITTLE_ENDIAN` if Little Endian,
///         or `-1` if the format is unsupported or mixed-endian.
static inline int gestGetEndianFloat(void) {
    union {
        float d;
        uint8_t p[4];
    } e = { .d = 1.0f }; // should be 0x3f800000

    const uint8_t be[4] = { 0x3f, 0x80, 0x00, 0x00 };
    const uint8_t le[4] = { 0x00, 0x00, 0x80, 0x3f };

    if (memcmp(e.p, be, 4) == 0) return GEST_BIG_ENDIAN;
    if (memcmp(e.p, le, 4) == 0) return GEST_LITTLE_ENDIAN;

    return -1; // Unsupported/Mixed Endian
}

#define GESTX_GEN_ENDIAN_CONVERSIONS(ty) \
    static inline uint##ty##_t gestEnSwapHBe##ty(uint##ty##_t i) { \
        return (gestGetEndian() == GEST_LITTLE_ENDIAN) ? gestByteSwap##ty(i) : i; \
    } \
    static inline uint##ty##_t gestEnSwapHLe##ty(uint##ty##_t i) { \
        return (gestGetEndian() == GEST_LITTLE_ENDIAN) ? i : gestByteSwap##ty(i); \
    }

GESTX_GEN_ENDIAN_CONVERSIONS(8)
GESTX_GEN_ENDIAN_CONVERSIONS(16)
GESTX_GEN_ENDIAN_CONVERSIONS(32)
GESTX_GEN_ENDIAN_CONVERSIONS(64)

#undef GESTX_GEN_ENDIAN_CONVERSIONS

// Check if IEEE 754
#ifndef GEST_IS_IEEE754
    #if defined(__STDC_IEC_559__) || defined(__STDC_IEC_60559_BFP__)
        #define GEST_IS_IEEE754 1
    #elif (DBL_MANT_DIG != 53) || (DBL_MAX_EXP != 1024) || (FLT_RADIX != 2)
        #define GEST_IS_IEEE754 0
    #else
        #if defined(__GNUC__) || defined(__clang__)
            #warning "Non-strict IEEE754/IEC60559 support."
        #else
            #pragma message("WARNING: Non-strict IEE754/IEC60559 support.")
        #endif
        #define GEST_IS_IEEE754 1
    #endif
#endif

static_assert(
    GEST_IS_IEEE754,
    "IEEE754 support not able to be asserted. Either force compilation by defining "
    "GEST_IS_IEEE754, report if on a platform that should have support, or give up if the target "
    "doesn't support them."
);
static_assert(sizeof(float) == 4 && sizeof(double) == 8, "IEEE754 float/double size mismatch.");

#define GESTX_GEN_ENDIAN_CONVERSIONS(ty, nm) \
    static inline ty gestEnSwapHBeFlt##nm(ty i) { \
        uint##nm##_t v; \
        memcpy(&v, &i, sizeof(ty)); \
        v = (gestGetEndianFloat() == GEST_LITTLE_ENDIAN) ? gestByteSwap##nm(v) : v; \
        memcpy(&i, &v, sizeof(ty)); \
        return i; \
    } \
    static inline ty gestEnSwapHLeFlt##nm(ty i) { \
        uint##nm##_t v; \
        memcpy(&v, &i, sizeof(ty)); \
        v = (gestGetEndianFloat() == GEST_LITTLE_ENDIAN) ? v : gestByteSwap##nm(v); \
        memcpy(&i, &v, sizeof(ty)); \
        return i; \
    }

GESTX_GEN_ENDIAN_CONVERSIONS(float, 32)
GESTX_GEN_ENDIAN_CONVERSIONS(double, 64)

#undef GESTX_GEN_ENDIAN_CONVERSIONS

#define GESTX_GEN_ENDIAN_CONVERSIONS(ty) \
    static inline uint##ty##_t gestEnSwapIFlt##ty(uint##ty##_t v) { \
        return (gestGetEndianFloat() != gestGetEndian()) ? gestByteSwap##ty(v) : v; \
    }

GESTX_GEN_ENDIAN_CONVERSIONS(32)
GESTX_GEN_ENDIAN_CONVERSIONS(64)

#undef GESTX_GEN_ENDIAN_CONVERSIONS

static inline int gestIsNaNCanonicalised(void) {
    uint32_t originalBits32 = gestEnSwapIFlt32(0x7FAAAAAA); // A specific NaN payload
    float f;
    uint32_t checkBits32;

    memcpy(&f, &originalBits32, sizeof(f));

    volatile float volf = f;
    float f2 = volf;

    memcpy(&checkBits32, &f2, sizeof(checkBits32));

    uint64_t originalBits64 = gestEnSwapIFlt64(0xFFFEDEADBEEFCAFE); // A specific NaN payload
    double d;
    uint64_t checkBits64;

    memcpy(&d, &originalBits64, sizeof(d));

    volatile double vold = d;
    double d2 = vold;

    memcpy(&checkBits64, &d2, sizeof(checkBits64));

    return originalBits32 != checkBits32 || originalBits64 != checkBits64;
}

#endif // GEST_ENDIAN_H
