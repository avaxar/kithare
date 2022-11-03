/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/avaxar/Kithare
 * Copyright (C) 2022 Kithare Organization
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <kithare/lib/array.h>
#include <kithare/lib/buffer.h>
#include <kithare/lib/string.h>


typedef enum {
    khTokenType_INVALID,
    khTokenType_EOF,
    khTokenType_NEWLINE,
    khTokenType_COMMENT,

    khTokenType_IDENTIFIER,
    khTokenType_KEYWORD,
    khTokenType_DELIMITER,
    khTokenType_OPERATOR,

    khTokenType_CHAR,
    khTokenType_STRING,
    khTokenType_BUFFER,

    khTokenType_BYTE,
    khTokenType_INTEGER,
    khTokenType_UINTEGER,
    khTokenType_FLOAT,
    khTokenType_DOUBLE,
    khTokenType_IFLOAT,
    khTokenType_IDOUBLE
} khTokenType;

khstring khTokenType_string(khTokenType type);


typedef enum {
    khKeywordToken_IMPORT,
    khKeywordToken_INCLUDE,
    khKeywordToken_AS,
    khKeywordToken_DEF,
    khKeywordToken_CLASS,
    khKeywordToken_INHERITS,
    khKeywordToken_STRUCT,
    khKeywordToken_ENUM,
    khKeywordToken_ALIAS,

    khKeywordToken_REF,
    khKeywordToken_WILD,
    khKeywordToken_INCASE,
    khKeywordToken_STATIC,

    khKeywordToken_IF,
    khKeywordToken_ELIF,
    khKeywordToken_ELSE,
    khKeywordToken_FOR,
    khKeywordToken_IN,
    khKeywordToken_WHILE,
    khKeywordToken_DO,
    khKeywordToken_BREAK,
    khKeywordToken_CONTINUE,
    khKeywordToken_RETURN
} khKeywordToken;

khstring khKeywordToken_string(khKeywordToken keyword);


typedef enum {
    khDelimiterToken_DOT,
    khDelimiterToken_COMMA,
    khDelimiterToken_COLON,
    khDelimiterToken_SEMICOLON,
    khDelimiterToken_EXCLAMATION,

    khDelimiterToken_PARENTHESIS_OPEN,
    khDelimiterToken_PARENTHESIS_CLOSE,
    khDelimiterToken_CURLY_BRACKET_OPEN,
    khDelimiterToken_CURLY_BRACKET_CLOSE,
    khDelimiterToken_SQUARE_BRACKET_OPEN,
    khDelimiterToken_SQUARE_BRACKET_CLOSE,

    khDelimiterToken_ARROW,
    khDelimiterToken_ELLIPSIS
} khDelimiterToken;

khstring khDelimiterToken_string(khDelimiterToken delimiter);


typedef enum {
    khOperatorToken_ASSIGN,
    khOperatorToken_RANGE,

    khOperatorToken_ADD,
    khOperatorToken_SUB,
    khOperatorToken_MUL,
    khOperatorToken_DIV,
    khOperatorToken_MOD,
    khOperatorToken_DOT,
    khOperatorToken_POW,

    khOperatorToken_IP_ADD,
    khOperatorToken_IP_SUB,
    khOperatorToken_IP_MUL,
    khOperatorToken_IP_DIV,
    khOperatorToken_IP_MOD,
    khOperatorToken_IP_DOT,
    khOperatorToken_IP_POW,

    khOperatorToken_EQUAL,
    khOperatorToken_UNEQUAL,
    khOperatorToken_LESS,
    khOperatorToken_GREATER,
    khOperatorToken_LESS_EQUAL,
    khOperatorToken_GREATER_EQUAL,

    khOperatorToken_NOT,
    khOperatorToken_AND,
    khOperatorToken_OR,
    khOperatorToken_XOR,

    khOperatorToken_BIT_NOT,
    khOperatorToken_BIT_AND,
    khOperatorToken_BIT_OR,
    khOperatorToken_BIT_XOR = khOperatorToken_BIT_NOT, // Both uses `~`.
    khOperatorToken_BIT_LSHIFT = khOperatorToken_BIT_OR + 1,
    khOperatorToken_BIT_RSHIFT,

    khOperatorToken_IP_BIT_AND,
    khOperatorToken_IP_BIT_OR,
    khOperatorToken_IP_BIT_XOR,
    khOperatorToken_IP_BIT_LSHIFT,
    khOperatorToken_IP_BIT_RSHIFT
} khOperatorToken;

khstring khOperatorToken_string(khOperatorToken operator_v);


typedef struct {
    char32_t* begin;
    char32_t* end;

    khTokenType type;
    union {
        khstring identifier;
        khKeywordToken keyword;
        khDelimiterToken delimiter;
        khOperatorToken operator_v;

        char32_t char_v;
        khstring string;
        khbuffer buffer;

        uint8_t byte;
        __int128_t integer;
        __uint128_t uinteger;
        float float_v;
        double double_v;
        float ifloat;
        double idouble;
    };
} khToken;

khToken khToken_copy(khToken* token);
void khToken_delete(khToken* token);
khstring khToken_string(khToken* token, char32_t* origin);

static inline khToken khToken_fromInvalid(char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_INVALID};
}

static inline khToken khToken_fromEof(char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_EOF};
}

static inline khToken khToken_fromNewline(char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_NEWLINE};
}

static inline khToken khToken_fromComment(char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_COMMENT};
}

static inline khToken khToken_fromIdentifier(khstring identifier, char32_t* begin, char32_t* end) {
    return (khToken){
        .begin = begin, .end = end, .type = khTokenType_IDENTIFIER, .identifier = identifier};
}

static inline khToken khToken_fromKeyword(khKeywordToken keyword, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_KEYWORD, .keyword = keyword};
}

static inline khToken khToken_fromDelimiter(khDelimiterToken delimiter, char32_t* begin,
                                            char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_DELIMITER, .delimiter = delimiter};
}

static inline khToken khToken_fromOperator(khOperatorToken operator_v, char32_t* begin, char32_t* end) {
    return (khToken){
        .begin = begin, .end = end, .type = khTokenType_OPERATOR, .operator_v = operator_v};
}

static inline khToken khToken_fromChar(char32_t char_v, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_CHAR, .char_v = char_v};
}

static inline khToken khToken_fromString(khstring string, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_STRING, .string = string};
}

static inline khToken khToken_fromBuffer(khbuffer buffer, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_BUFFER, .buffer = buffer};
}

static inline khToken khToken_fromByte(uint8_t byte, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_BYTE, .byte = byte};
}

static inline khToken khToken_fromInteger(__int128_t integer, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_INTEGER, .integer = integer};
}

static inline khToken khToken_fromUinteger(__uint128_t uinteger, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_UINTEGER, .uinteger = uinteger};
}

static inline khToken khToken_fromFloat(float float_v, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_FLOAT, .float_v = float_v};
}

static inline khToken khToken_fromDouble(double double_v, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_DOUBLE, .double_v = double_v};
}

static inline khToken khToken_fromIfloat(float ifloat, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_IFLOAT, .ifloat = ifloat};
}

static inline khToken khToken_fromIdouble(double idouble, char32_t* begin, char32_t* end) {
    return (khToken){.begin = begin, .end = end, .type = khTokenType_IDOUBLE, .idouble = idouble};
}


#ifdef __cplusplus
}
#endif
