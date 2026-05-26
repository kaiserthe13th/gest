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

/// Returns an opaque token which can be used to seek back to the current state of the iterator.
/// @param iter The iterator to save the state of.
/// @return An opaque token denoting the position of the iterator.
const void *gestUniIterSaveState(struct GestStr *iter);
/// Takes an opaque token and sets the iterator position to the state it was back when the token was created.
/// @param iter The iterator to reset the state of.
/// @param token The opaque token
void gestUniIterSeek(struct GestStr *iter, const void *token);

/// Takes in a mutable iterator, advances it by one char and returns the new char decoded.
/// 
/// @param iter The iterator to advance
/// @param lossy If non-zero, the iterator skips over broken chars (ignoring errors)
/// @return The decoded `GestU32Char` character.
/// @retval `GEST_UNI_U32C_REPLACEMENT_CHARACTER` If a decoding error occurs and `lossy` is zero (`gestErrno` will be set).
/// @retval `GEST_U32C_EOF` If the end of the stream is reached.
GestU32Char gestUniNextU32Char(struct GestStr *iter, int lossy);

struct GestUTF8EncodedChar {
    uint8_t length;
    uint8_t enc[4];
};

/// Takes in a codepoint and outputs a UTF-8 encoded char, which specifies the length and the raw bytes in it.
///
/// @param u32c The codepoint to encode.
/// @return the encoded char, if the codepoint is invalid, a 0-length encoding will be returned with `gestErrno` set.
struct GestUTF8EncodedChar gestUniC32EncodeUTF8(GestU32Char u32c);

#endif // GEST_NOGEN_UNICODE_H