/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once
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
    khOperatorToken_BIT_LSHIFT = khOperatorToken_BIT_OR + 1,
    khOperatorToken_BIT_RSHIFT,

    khOperatorToken_IBIT_AND,
    khOperatorToken_IBIT_OR,
    khOperatorToken_IBIT_XOR,
    khOperatorToken_IBIT_LSHIFT,
    khOperatorToken_IBIT_RSHIFT
} khOperatorToken;


typedef enum {
    khTokenType_NONE,
    khTokenType_COMMENT,

    khTokenType_IDENTIFIER,
    khTokenType_KEYWORD,
    khTokenType_DELIMITER,
    khTokenType_OPERATOR,

    khTokenType_CHAR,
    khTokenType_STRING,
    khTokenType_BUFFER,

    khTokenType_SBYTE,
    khTokenType_BYTE,
    khTokenType_SHORT,
    khTokenType_USHORT,
    khTokenType_INT,
    khTokenType_UINT,
    khTokenType_LONG,
    khTokenType_ULONG,

    khTokenType_FLOAT,
    khTokenType_IFLOAT,
    khTokenType_DOUBLE,
    khTokenType_IDOUBLE
} khTokenType;


typedef union {
    khArray_char identifier;
    khKeywordToken keyword;
    khDelimiterToken delimiter;
    khOperatorToken operator_v;

    char32_t char_v;
    khArray_char string;
    khArray_byte buffer;

    int8_t sbyte_v;
    uint8_t byte_v;
    int16_t short_v;
    uint16_t ushort_v;
    int32_t int_v;
    uint32_t uint_v;
    int64_t long_v;
    uint64_t ulong_v;

    float float_v;
    float ifloat_v;
    double double_v;
    double idouble_v;
} khTokenValue;


typedef struct {
    khTokenType type;
    khTokenValue value;
} khToken;


khToken khToken_copy(khToken* token);
void khToken_delete(khToken* token);

khArray_char khKeywordToken_string(khKeywordToken keyword);
khArray_char khDelimiterToken_string(khDelimiterToken delimiter);
khArray_char khOperatorToken_string(khOperatorToken operator_v);
khArray_char khTokenType_string(khTokenType type);
khArray_char khToken_string(khToken* token);


#define khArray_TYPE khToken
#define khArray_COPIER khToken_copy
#define khArray_DELETER khToken_delete
#include <kithare/t_array.h>


static inline khToken khToken_fromNone() {
    return (khToken){.type = khTokenType_NONE, .value = (khTokenValue){}};
}

static inline khToken khToken_fromComment() {
    return (khToken){.type = khTokenType_COMMENT, .value = (khTokenValue){}};
}

static inline khToken khToken_fromIdentifier(khArray_char identifier) {
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

static inline khToken khToken_fromString(khArray_char string) {
    return (khToken){.type = khTokenType_STRING, .value = (khTokenValue){.string = string}};
}

static inline khToken khToken_fromBuffer(khArray_byte buffer) {
    return (khToken){.type = khTokenType_BUFFER, .value = (khTokenValue){.buffer = buffer}};
}

static inline khToken khToken_fromSbyte(int8_t sbyte_v) {
    return (khToken){.type = khTokenType_SBYTE, .value = (khTokenValue){.sbyte_v = sbyte_v}};
}

static inline khToken khToken_fromByte(uint8_t byte_v) {
    return (khToken){.type = khTokenType_BYTE, .value = (khTokenValue){.byte_v = byte_v}};
}

static inline khToken khToken_fromShort(int16_t short_v) {
    return (khToken){.type = khTokenType_SHORT, .value = (khTokenValue){.short_v = short_v}};
}

static inline khToken khToken_fromUshort(uint16_t ushort_v) {
    return (khToken){.type = khTokenType_USHORT, .value = (khTokenValue){.ushort_v = ushort_v}};
}

static inline khToken khToken_fromInt(int32_t int_v) {
    return (khToken){.type = khTokenType_INT, .value = (khTokenValue){.int_v = int_v}};
}

static inline khToken khToken_fromUint(uint32_t uint_v) {
    return (khToken){.type = khTokenType_UINT, .value = (khTokenValue){.uint_v = uint_v}};
}

static inline khToken khToken_fromLong(int64_t long_v) {
    return (khToken){.type = khTokenType_LONG, .value = (khTokenValue){.long_v = long_v}};
}

static inline khToken khToken_fromUlong(uint64_t ulong_v) {
    return (khToken){.type = khTokenType_ULONG, .value = (khTokenValue){.ulong_v = ulong_v}};
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
