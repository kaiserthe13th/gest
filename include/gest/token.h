/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_TOKEN_H
#define GEST_TOKEN_H

#include <gest/common.h>
#include <gest/str.h>
#include <gest/unicode.h>

typedef struct GestSourceLineMap GestSourceLineMap;
typedef struct GestTokenizer GestTokenizer;
typedef struct GestToken GestToken;

struct GestSourceLineMap {
    size_t *lineTo;
};

struct GestTokenizer {
    GestStr source;
    GestStr iterator;
    size_t line;
    int error;
};

struct GestToken {
    size_t kind;
    size_t byteOffset;
    GestStr lexeme;
};

enum {
#define GESTX_TOKEN_KINDS(X) \
    X(GEST_TOK_EOF) \
    X(GEST_TOK_PAREN_OPEN) /* ( */ \
    X(GEST_TOK_BRACK_OPEN) /* [ */ \
    X(GEST_TOK_CURLY_OPEN) /* { */ \
    X(GEST_TOK_PAREN_CLOSE) /* ) */ \
    X(GEST_TOK_BRACK_CLOSE) /* ] */ \
    X(GEST_TOK_CURLY_CLOSE) /* } */ \
    X(GEST_TOK_HASH_PAREN_OPEN) /* #( */ \
    X(GEST_TOK_HASH_BRACK_OPEN) /* #[ */ \
    X(GEST_TOK_HASH_CURLY_OPEN) /* #{ */ \
    X(GEST_TOK_CARET_BRACK_OPEN) /* ^[ */ \
    X(GEST_TOK_OP_LRT) /* < */ \
    X(GEST_TOK_OP_GRT) /* > */ \
    X(GEST_TOK_OP_LRE) /* <= */ \
    X(GEST_TOK_OP_GRE) /* >= */ \
    X(GEST_TOK_OP_EQU) /* = */ \
    X(GEST_TOK_OP_NEQ) /* ~= */ \
    X(GEST_TOK_OP_IDETL) /* == */ \
    X(GEST_TOK_OP_NIDTL) /* ~~ */ \
    X(GEST_TOK_OP_BAND) /* & */ \
    X(GEST_TOK_OP_BOR) /* | */ \
    X(GEST_TOK_OP_ASSOC) /* => */ \
    X(GEST_TOK_OP_POINT) /* @ */ \
    X(GEST_TOK_OP_SHR) /* >> */ \
    X(GEST_TOK_OP_SHL) /* << */ \
    X(GEST_TOK_OP_PLUS) /* + */ \
    X(GEST_TOK_OP_CONCAT) /* ++ */ \
    X(GEST_TOK_OP_MINUS) /* - */ \
    X(GEST_TOK_OP_STAR) /* * */ \
    X(GEST_TOK_OP_DIV) /* / */ \
    X(GEST_TOK_OP_DIV_INT) /* // */ \
    X(GEST_TOK_OP_MOD) /* \\ */ \
    X(GEST_TOK_OP_ASSIGN) /* := */ \
    X(GEST_TOK_OP_COLON) /* : */ \
    X(GEST_TOK_OP_SEMI) /* ; */ \
    X(GEST_TOK_OP_DOT) /* . */ \
    X(GEST_TOK_OP_COMMA) /* , */ \
    X(GEST_TOK_OP_CARET) /* ^ */ \
    X(GEST_TOK_OP_FEED) /* |> */ \
    X(GEST_TOK_KW_XOR) /* xor */ \
    X(GEST_TOK_KW_AND) /* and */ \
    X(GEST_TOK_KW_OR) /* or */ \
    X(GEST_TOK_KW_LET) /* let */ \
    X(GEST_TOK_KW_PRAGMA) /* pragma */ \
    X(GEST_TOK_KW_GLOBAL) /* global */ \
    X(GEST_TOK_KW_NONLOCAL) /* nonlocal */ \
    X(GEST_TOK_KW_TRUE) /* true */ \
    X(GEST_TOK_KW_FALSE) /* false */ \
    X(GEST_TOK_KW_NIL) /* nil */ \
    X(GEST_TOK_ID) /* identifier, $ */ \
    X(GEST_TOK_LABEL) /* label: */ \
    X(GEST_TOK_STRING) /* '...' */ \
    X(GEST_TOK_CHARACTER) /* $a, $'🔥', $(1F4DC) */ \
    X(GEST_TOK_INTEGER) /* 193, 0x134 */ \
    X(GEST_TOK_FLOAT) /* 1.0, 3.14159, 1e10 */ \
    X(GEST_TOK_COMMENT) /* "..." */

    GESTX_TOKEN_KINDS(GESTX_ENUM)
};

inline static int gestIsAsciiDigit(int ch) {
    return '0' <= ch && ch <= '9';
}

inline static int gestIsAsciiHexDigit(int ch) {
    return ('0' <= ch && ch <= '9') || ('A' <= ch && 'F') || ('a' <= ch && 'f');
}

/// Advances through the source and returns the next token in the stream
/// @param tokenizer The tokenizer state
/// @return the token if exists, a token of kind `GEST_TOK_EOF` on error or end (check `tokenizer->error` to see if it is an error).
GEST_API GestToken gestNextToken(GestTokenizer *tokenizer);

#endif // GEST_TOKEN_H
