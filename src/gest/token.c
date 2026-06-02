/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/token.h>

struct GestIdToKw {
    GestStr id;
    size_t tokenKind;
};

struct GestIdToKw gestIdToKw[] = {
    {
        .id = gstrlit("and"),
        .tokenKind = GEST_TOK_KW_AND,
    },
    {
        .id = gstrlit("or"),
        .tokenKind = GEST_TOK_KW_OR,
    },
    {
        .id = gstrlit("nil"),
        .tokenKind = GEST_TOK_KW_NIL,
    },
    {
        .id = gstrlit("xor"),
        .tokenKind = GEST_TOK_KW_XOR,
    },
    {
        .id = gstrlit("true"),
        .tokenKind = GEST_TOK_KW_TRUE,
    },
    {
        .id = gstrlit("false"),
        .tokenKind = GEST_TOK_KW_FALSE,
    },
    {
        .id = gstrlit("let"),
        .tokenKind = GEST_TOK_KW_LET,
    },
    {
        .id = gstrlit("global"),
        .tokenKind = GEST_TOK_KW_GLOBAL,
    },
    {
        .id = gstrlit("nonlocal"),
        .tokenKind = GEST_TOK_KW_NONLOCAL,
    },
    {
        .id = gstrlit("pragma"),
        .tokenKind = GEST_TOK_KW_PRAGMA,
    },
};

/// Skips all whitespace characters and returns the first non-whitespace character
/// @param tokenizer The tokenizer
/// @return The first non whitespace character
GestU32Char gestSkipWhitespace(GestTokenizer *tokenizer) {
    GestU32Char lastChar;
    while (1) {
        lastChar = gestUniPeekU32Char(0, tokenizer->iterator, 0);
        int isError = lastChar == GEST_UNI_U32C_REPLACEMENT_CHARACTER && gestErrno != GEST_ENONE;
        if (lastChar == GEST_U32C_EOF || isError) return lastChar;
        if (gestUniC32IsUniWhiteSpaceFast(lastChar)) gestUniNextU32Char(&tokenizer->iterator, 0);
    }
    return lastChar;
}

GestToken gestNextToken(GestTokenizer *tokenizer) {
    GestToken tok = { GEST_TOK_EOF,
                      (size_t)tokenizer->source.chars - (size_t)tokenizer->iterator.chars,
                      tokenizer->iterator };
    GestU32Char ch = gestSkipWhitespace(tokenizer);
    if (ch == GEST_U32C_EOF) return tok;
    if (ch == GEST_UNI_U32C_REPLACEMENT_CHARACTER && gestErrno != GEST_ENONE) {
        tok.lexeme = (GestStr)gstrlit("invalid encoding");
        tokenizer->error = gestErrno;
        return tok;
    }
    tok.lexeme.chars = tokenizer->iterator.chars;
    tok.byteOffset = (size_t)tokenizer->source.chars - (size_t)tokenizer->iterator.chars;

    if (gestUniC32IsUniXidStartFast(ch) || ch == '_') {
        tok.kind = GEST_TOK_ID;
        while (1) {
            ch = gestUniPeekU32Char(0, tokenizer->iterator, 0);
            if (!gestUniC32IsUniXidContinueFast(ch) && ch != '_') break;
            gestUniNextU32Char(&tokenizer->iterator, 0);
        }
        if (ch == '?') {
            gestUniNextU32Char(&tokenizer->iterator, 0);
        }

        tok.lexeme.length = (size_t)tokenizer->iterator.chars - (size_t)tok.lexeme.chars;
        return tok;
    }

    if (ch == '$') {
        tok.kind = GEST_TOK_CHARACTER;
        gestUniNextU32Char(&tokenizer->iterator, 0);
        char peek = gestUniPeekU32Char(0, tokenizer->iterator, 0);
        if (gestUniC32IsUniXidStartFast(peek) || peek == '_') {
            gestUniNextU32Char(&tokenizer->iterator, 0);
            tok.lexeme.length = (size_t)tokenizer->iterator.chars - (size_t)tok.lexeme.chars;
            return tok;
        }
        if (peek == '(') {
            ch = gestUniNextU32Char(&tokenizer->iterator, 0);
            while (ch != ')') {
                ch = gestUniNextU32Char(&tokenizer->iterator, 0);
                if (!gestIsAsciiHexDigit(ch) && ch != ')') {
                    tok.kind = GEST_TOK_EOF;
                    tok.lexeme = (GestStr)gstrlit("unicode codepoint character literal $(...) cannot contain non hex digits");
                    tokenizer->error = GEST_EINVAL;
                    return tok;
                }
            }
            tok.lexeme.length = (size_t)tokenizer->iterator.chars - (size_t)tok.lexeme.chars;
            return tok;
        }
        if (peek == '\'') {
            peek = gestUniPeekU32Char(2, tokenizer->iterator, 0);
            if (peek != '\'') {
                tok.kind = GEST_TOK_ID;
                tok.lexeme.length = 1;
                return tok;
            }
            // Skip the first '
            gestUniNextU32Char(&tokenizer->iterator, 0);
            // Get the codepoint
            gestUniNextU32Char(&tokenizer->iterator, 0);
            // Skip the second '
            gestUniNextU32Char(&tokenizer->iterator, 0);
            tok.lexeme.length = (size_t)tokenizer->iterator.chars - (size_t)tok.lexeme.chars;
            return tok;
        }
        tok.kind = GEST_TOK_ID;
        tok.lexeme.length = 1;
        return tok;
    }

    return tok;
}
