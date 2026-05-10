/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_QNAN_H
#define GEST_QNAN_H

#include <gest/common.h>
#include <math.h>

/// The bit that determines the sign of the double.
#define GEST_SIGN_BIT 63ULL
/// The bit that determines the quiet/signaling state of the NaN.
#define GEST_QUIET_BIT 51ULL
/// The bit that is set to be 1 for the nonzero conditional of the IEEE 754 format.
#define GEST_SAFETY_BIT 50ULL
/// The bit that determines the type of the boxed value.
#define GEST_TYPE_BIT 48ULL

/// The bits that need to be compared with GEST_QNAN to check for quietness.
#define GEST_QNAN_MASK \
    (0x7FF0000000000000ULL | (1ULL << GEST_QUIET_BIT) | (1ULL << GEST_SAFETY_BIT))
/// The bits that need to be compared with GEST_NANBOX to check for whether a value is NaN-boxed.
#define GEST_NANBOX_MASK (GEST_QNAN_MASK | (1ULL << GEST_SIGN_BIT))


/// The bit pattern that if exactly on the double, means that is a quiet NaN.
extern uint64_t GEST_QNAN;
/// The bit pattern that if exactly on the double, means that is a NaN-boxed value.
extern uint64_t GEST_NANBOX;

/// Initializes the GEST_QNAN and GEST_NANBOX values for use.
GEST_API void gestInitialiseNanBoxPattern(void);

/// Returns whether a value is NaN-boxed.
inline static int gestNanIsBoxed(uint64_t val) {
    return (val & GEST_NANBOX_MASK) == GEST_NANBOX;
}

#define GEST_NAN_BOX_INT        0
#define GEST_NAN_BOX_PTR        1
#define GEST_NAN_BOX_TYPE_COUNT 2

/// Returns the kind of a NaN-boxed value.
inline static int gestNanBoxKind(uint64_t boxed) {
    return ((boxed & ~GEST_NANBOX_MASK) >> GEST_TYPE_BIT) & 1;
}

/// Unboxes a boxed payload.
/// @param boxed The boxed NaN-format
/// @return The payload extracted as a 48-bit signed integer
inline static uint64_t gestUnbox(uint64_t boxed) {
    return (uint64_t)((int64_t)(boxed << 16) >> 16); // sign extend
}

/// Boxes a 48-bit integer or pointer into a NaN.
/// @param unboxed The unboxed 48-bit integer to box.
/// @return A boxed-NaN containing `unboxed`.
inline static uint64_t gestBox(uint64_t unboxed, uint64_t type) {
#ifdef _DEBUG
    assert(unboxed < (1ULL << GEST_SAFETY_BIT));
    assert(type < GEST_NAN_BOX_TYPE_COUNT);
#endif // _DEBUG

    return unboxed | (type << GEST_TYPE_BIT) | GEST_NANBOX;
}

#endif // GEST_QNAN_H
