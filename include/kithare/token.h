/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "array.h"
#include "string.h"


typedef enum {
    khTokenType_NONE,
    khTokenType_COMMENT,
    khTokenType_NEWLINE,

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


typedef enum {
    khKeywordToken_IMPORT,
    khKeywordToken_INCLUDE,
    khKeywordToken_AS,
    khKeywordToken_INCASE,
    khKeywordToken_DEF,
    khKeywordToken_CLASS,
    khKeywordToken_STRUCT,
    khKeywordToken_ENUM,
    khKeywordToken_ALIAS,

    khKeywordToken_REF,
    khKeywordToken_STATIC,

    khKeywordToken_IF,
    khKeywordToken_ELIF,
    khKeywordToken_ELSE,
    khKeywordToken_FOR,
    khKeywordToken_WHILE,
    khKeywordToken_DO,
    khKeywordToken_BREAK,
    khKeywordToken_CONTINUE,
    khKeywordToken_RETURN
} khKeywordToken;


typedef enum {
    khDelimiterToken_DOT,
    khDelimiterToken_COMMA,
    khDelimiterToken_COLON,
    khDelimiterToken_SEMICOLON,
    khDelimiterToken_EXCLAMATION,

    khDelimiterToken_PARENTHESES_OPEN,
    khDelimiterToken_PARENTHESES_CLOSE,
    khDelimiterToken_CURLY_BRACKET_OPEN,
    khDelimiterToken_CURLY_BRACKET_CLOSE,
    khDelimiterToken_SQUARE_BRACKET_OPEN,
    khDelimiterToken_SQUARE_BRACKET_CLOSE,

    khDelimiterToken_ARROW,
    khDelimiterToken_ELLIPSIS
} khDelimiterToken;


typedef enum {
    khOperatorToken_ADD,
    khOperatorToken_SUB,
    khOperatorToken_MUL,
    khOperatorToken_DIV,
    khOperatorToken_MODULO,
    khOperatorToken_POWER,
    khOperatorToken_DOT,

    khOperatorToken_IADD,
    khOperatorToken_ISUB,
    khOperatorToken_IMUL,
    khOperatorToken_IDIV,
    khOperatorToken_IMODULO,
    khOperatorToken_IPOWER,
    khOperatorToken_IDOT,

    khOperatorToken_INCREMENT,
    khOperatorToken_DECREMENT,

    khOperatorToken_ASSIGN,

    khOperatorToken_EQUAL,
    khOperatorToken_NOT_EQUAL,
    khOperatorToken_LESS,
    khOperatorToken_MORE,
    khOperatorToken_ELESS,
    khOperatorToken_EMORE,

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

    khOperatorToken_IBIT_AND,
    khOperatorToken_IBIT_OR,
    khOperatorToken_IBIT_XOR,
    khOperatorToken_IBIT_LSHIFT,
    khOperatorToken_IBIT_RSHIFT
} khOperatorToken;


typedef union {
    khArray(char32_t) identifier;
    khKeywordToken keyword;
    khDelimiterToken delimiter;
    khOperatorToken operator_v;

    char32_t char_v;
    khArray(char32_t) string;
    khArray(uint8_t) buffer;

    uint8_t byte;
    int64_t integer;
    uint64_t uinteger;
    float float_v;
    double double_v;
    float ifloat;
    double idouble;
} khTokenValue;


typedef struct {
    khTokenType type;
    khTokenValue value;
} khToken;

khToken khToken_copy(khToken* token);
void khToken_delete(khToken* token);

khArray(char32_t) khKeywordToken_string(khKeywordToken keyword);
khArray(char32_t) khDelimiterToken_string(khDelimiterToken delimiter);
khArray(char32_t) khOperatorToken_string(khOperatorToken operator_v);
khArray(char32_t) khTokenType_string(khTokenType type);
khArray(char32_t) khToken_string(khToken* token);

static inline khToken khToken_fromNone() {
    return (khToken){.type = khTokenType_NONE, .value = (khTokenValue){}};
}

static inline khToken khToken_fromComment() {
    return (khToken){.type = khTokenType_COMMENT, .value = (khTokenValue){}};
}

static inline khToken khToken_fromNewline() {
    return (khToken){.type = khTokenType_NEWLINE, .value = (khTokenValue){}};
}

static inline khToken khToken_fromIdentifier(khArray(char32_t) identifier) {
    return (khToken){.type = khTokenType_IDENTIFIER, .value = (khTokenValue){.identifier = identifier}};
}

static inline khToken khToken_fromKeyword(khKeywordToken keyword) {
    return (khToken){.type = khTokenType_KEYWORD, .value = (khTokenValue){.keyword = keyword}};
}

static inline khToken khToken_fromDelimiter(khDelimiterToken delimiter) {
    return (khToken){.type = khTokenType_DELIMITER, .value = (khTokenValue){.delimiter = delimiter}};
}

static inline khToken khToken_fromOperator(khOperatorToken operator_v) {
    return (khToken){.type = khTokenType_OPERATOR, .value = (khTokenValue){.operator_v = operator_v}};
}

static inline khToken khToken_fromChar(char32_t char_v) {
    return (khToken){.type = khTokenType_CHAR, .value = (khTokenValue){.char_v = char_v}};
}

static inline khToken khToken_fromString(khArray(char32_t) string) {
    return (khToken){.type = khTokenType_STRING, .value = (khTokenValue){.string = string}};
}

static inline khToken khToken_fromBuffer(khArray(uint8_t) buffer) {
    return (khToken){.type = khTokenType_BUFFER, .value = (khTokenValue){.buffer = buffer}};
}

static inline khToken khToken_fromByte(uint8_t byte) {
    return (khToken){.type = khTokenType_BYTE, .value = (khTokenValue){.byte = byte}};
}

static inline khToken khToken_fromInteger(int64_t integer) {
    return (khToken){.type = khTokenType_INTEGER, .value = (khTokenValue){.integer = integer}};
}

static inline khToken khToken_fromUinteger(uint64_t uinteger) {
    return (khToken){.type = khTokenType_UINTEGER, .value = (khTokenValue){.uinteger = uinteger}};
}

static inline khToken khToken_fromFloat(float float_v) {
    return (khToken){.type = khTokenType_FLOAT, .value = (khTokenValue){.float_v = float_v}};
}

static inline khToken khToken_fromDouble(double double_v) {
    return (khToken){.type = khTokenType_DOUBLE, .value = (khTokenValue){.double_v = double_v}};
}

static inline khToken khToken_fromIfloat(float ifloat) {
    return (khToken){.type = khTokenType_IFLOAT, .value = (khTokenValue){.ifloat = ifloat}};
}

static inline khToken khToken_fromIdouble(double idouble) {
    return (khToken){.type = khTokenType_IDOUBLE, .value = (khTokenValue){.idouble = idouble}};
}


#ifdef __cplusplus
}
#endif
