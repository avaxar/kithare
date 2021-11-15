/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_TOKEN_H
#define kh_HG_TOKEN_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <kithare/arrays.h>


typedef enum {
    khKeywordToken_IMPORT,
    khKeywordToken_INCLUDE,
    khKeywordToken_AS,
    khKeywordToken_TRY,
    khKeywordToken_DEF,
    khKeywordToken_CLASS,
    khKeywordToken_STRUCT,
    khKeywordToken_ENUM,
    khKeywordToken_ALIAS,

    khKeywordToken_REF,
    khKeywordToken_PUBLIC,
    khKeywordToken_PRIVATE,
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
    khDelimiterToken_SQUARE_BRACKET_CLOSE
} khDelimiterToken;


typedef enum {
    khOperatorToken_ADD,
    khOperatorToken_SUB,
    khOperatorToken_MUL,
    khOperatorToken_DIV,
    khOperatorToken_MODULO,
    khOperatorToken_POWER,

    khOperatorToken_IADD,
    khOperatorToken_ISUB,
    khOperatorToken_IMUL,
    khOperatorToken_IDIV,
    khOperatorToken_IMODULO,
    khOperatorToken_IPOWER,

    khOperatorToken_ASSIGN,
    khOperatorToken_ID,

    khOperatorToken_INCREMENT,
    khOperatorToken_DECREMENT,

    khOperatorToken_EQUALS,
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
    khOperatorToken_BIT_LSHIFT,
    khOperatorToken_BIT_RSHIFT,

    khOperatorToken_IBIT_AND,
    khOperatorToken_IBIT_OR,
    khOperatorToken_IBIT_XOR,
    khOperatorToken_IBIT_LSHIFT,
    khOperatorToken_IBIT_RSHIFT
} khOperatorToken;


typedef enum {
    khTokenType_NONE,

    khTokenType_IDENTIFIER,
    khTokenType_KEYWORD,
    khTokenType_DELIMITER,
    khTokenType_OPERATOR,

    khTokenType_CHAR,
    khTokenType_STRING,
    khTokenType_BUFFER,

    khTokenType_INT8,
    khTokenType_UINT8,
    khTokenType_INT16,
    khTokenType_UINT16,
    khTokenType_INT32,
    khTokenType_UINT32,
    khTokenType_INT64,
    khTokenType_UINT64,

    khTokenType_FLOAT,
    khTokenType_IFLOAT,
    khTokenType_DOUBLE,
    khTokenType_IDOUBLE
} khTokenType;


typedef union {
    khArray_byte identifier;
    khKeywordToken keyword;
    khDelimiterToken delimiter;
    khOperatorToken operator_v;

    uint32_t char_v;
    khArray_char string;
    khArray_byte buffer;

    int8_t int8_v;
    uint8_t uint8_v;
    int16_t int16_v;
    uint16_t uint16_v;
    int32_t int32_v;
    uint32_t uint32_v;
    int64_t int64_v;
    uint64_t uint64_v;

    float float_v;
    float ifloat_v;
    double double_v;
    double idouble_v;
} khTokenValue;


typedef struct {
    khTokenType type;
    khTokenValue value;
} khToken;


static inline khToken khToken_new(khTokenType type, khTokenValue value) {
    return (khToken){.type = type, .value = value};
}

khToken khToken_copy(const khToken* token);
void khToken_delete(khToken* token);


#define khArray_TYPE khToken
#define khArray_COPIER khToken_copy
#define khArray_DELETER khToken_delete
#include <kithare/t_array.h>


static inline khToken khToken_fromNone() {
    return (khToken){.type = khTokenType_NONE, .value = (khTokenValue){}};
}

static inline khToken khToken_fromIdentifier(khArray_byte identifier) {
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

static inline khToken khToken_fromChar(int32_t char_v) {
    return (khToken){.type = khTokenType_CHAR, .value = (khTokenValue){.char_v = char_v}};
}

static inline khToken khToken_fromString(khArray_char string) {
    return (khToken){.type = khTokenType_STRING, .value = (khTokenValue){.string = string}};
}

static inline khToken khToken_fromBuffer(khArray_byte buffer) {
    return (khToken){.type = khTokenType_BUFFER, .value = (khTokenValue){.buffer = buffer}};
}

static inline khToken khToken_fromInt8(int8_t int8_v) {
    return (khToken){.type = khTokenType_INT8, .value = (khTokenValue){.int8_v = int8_v}};
}

static inline khToken khToken_fromUint8(uint8_t uint8_v) {
    return (khToken){.type = khTokenType_UINT8, .value = (khTokenValue){.uint8_v = uint8_v}};
}

static inline khToken khToken_fromInt16(int16_t int16_v) {
    return (khToken){.type = khTokenType_INT16, .value = (khTokenValue){.int16_v = int16_v}};
}

static inline khToken khToken_fromUint16(uint16_t uint16_v) {
    return (khToken){.type = khTokenType_UINT16, .value = (khTokenValue){.uint16_v = uint16_v}};
}

static inline khToken khToken_fromInt32(int32_t int32_v) {
    return (khToken){.type = khTokenType_INT32, .value = (khTokenValue){.int32_v = int32_v}};
}

static inline khToken khToken_fromUint32(uint32_t uint32_v) {
    return (khToken){.type = khTokenType_UINT32, .value = (khTokenValue){.uint32_v = uint32_v}};
}

static inline khToken khToken_fromInt64(int64_t int64_v) {
    return (khToken){.type = khTokenType_INT64, .value = (khTokenValue){.int64_v = int64_v}};
}

static inline khToken khToken_fromUint64(uint64_t uint64_v) {
    return (khToken){.type = khTokenType_UINT64, .value = (khTokenValue){.uint64_v = uint64_v}};
}

static inline khToken khToken_fromFloat(float float_v) {
    return (khToken){.type = khTokenType_FLOAT, .value = (khTokenValue){.float_v = float_v}};
}

static inline khToken khToken_fromIfloat(float ifloat_v) {
    return (khToken){.type = khTokenType_IFLOAT, .value = (khTokenValue){.ifloat_v = ifloat_v}};
}

static inline khToken khToken_fromDouble(double double_v) {
    return (khToken){.type = khTokenType_DOUBLE, .value = (khTokenValue){.double_v = double_v}};
}

static inline khToken khToken_fromIdouble(double idouble_v) {
    return (khToken){.type = khTokenType_IDOUBLE, .value = (khTokenValue){.idouble_v = idouble_v}};
}


#ifdef __cplusplus
}
#endif
#endif
