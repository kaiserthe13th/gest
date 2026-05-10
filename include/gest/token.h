/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_TOKEN_H
#define GEST_TOKEN_H

#include <gest/common.h>

enum {
#define GESTX_TOKEN_KIND_PAIRS(X) \
    X(GEST_TOK_PAREN_OPEN, '(') \
    X(GEST_TOK_PAREN_CLOSE, ')') \
    X(GEST_TOK_BRACK_OPEN, '[') \
    X(GEST_TOK_BRACK_CLOSE, ']') \
    X(GEST_TOK_CURLY_OPEN, '{') \
    X(GEST_TOK_CURLY_CLOSE, '}') \
    X(GEST_TOK_OP_LRT, '<') \
    X(GEST_TOK_OP_GRT, '>') \
    X(GEST_TOK_OP_LRE, 'z') /* <= */ \
    X(GEST_TOK_OP_GRE, 'Z') /* >= */ \
    X(GEST_TOK_OP_EQU, '=') /* == */ \
    X(GEST_TOK_OP_NEQ, '?') /* != */ \
    X(GEST_TOK_OP_BNOT, '~') \
    X(GEST_TOK_OP_BANG, '!') \
    X(GEST_TOK_OP_BXOR, '^') \
    X(GEST_TOK_OP_BAND, '&') \
    X(GEST_TOK_OP_LAND, 'n') /* && */ \
    X(GEST_TOK_OP_BOR, '|') \
    X(GEST_TOK_OP_LOR, 166) /* || */ \
    X(GEST_TOK_OP_PLUS, '+') \
    X(GEST_TOK_OP_MINUS, '-') \
    X(GEST_TOK_OP_STAR, '*') \
    X(GEST_TOK_OP_DIV, '/') \
    X(GEST_TOK_OP_MOD, '%') \
    X(GEST_TOK_OP_ASSIGN, ':') /* := */ \
    X(GEST_TOK_OP_SEMI, ';') \
    X(GEST_TOK_OP_DOT, '.') \
    X(GEST_TOK_OP_COMMA, ',') \
    X(GEST_TOK_ID, 'i') /* identifier */ \
    X(GEST_TOK_LABEL, 'l') /* label: */ \
    X(GEST_TOK_STRING, '"') /* "...", '...' */ \
    X(GEST_TOK_INTEGER, '0') /* 193, 0x134 */ \
    X(GEST_TOK_FLOAT, '1') /* 1.0, 3.14159, 1e10 */

    GESTX_TOKEN_KIND_PAIRS(GESTX_ENUM_PAIR)
};

inline static int gestIsDigit(int ch) {
    return '0' <= ch && ch <= '9';
}

inline static int gestIsAlphaLower(int ch) {
    return 'a' <= ch && ch <= 'z';
}

inline static int gestIsAlphaUpper(int ch) {
    return 'A' <= ch && ch <= 'Z';
}

inline static int gestIsAlpha(int ch) {
    return gestIsAlphaLower(ch) || gestIsAlphaUpper(ch);
}

inline static int gestIsAlphanumeric(int ch) {
    return gestIsAlpha(ch) || gestIsDigit(ch);
}

#endif // GEST_TOKEN_H
