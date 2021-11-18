/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_STRING_H
#define kh_HG_STRING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <kithare/arrays.h>


bool kh_compareString(khArray_char* a, khArray_char* b);
bool kh_compareBuffer(khArray_byte* a, khArray_byte* b);
bool kh_compareCstring(khArray_char* a, const char* b);

khArray_byte kh_encodeUtf8(khArray_char* string);
khArray_char kh_decodeUtf8(khArray_byte* buffer);
uint32_t kh_utf8(uint8_t** cursor);

khArray_char kh_escapeChar(uint32_t chr);
khArray_char kh_quoteString(khArray_char* string);
khArray_char kh_quoteBuffer(khArray_byte* buffer);

int64_t kh_stringToInt(khArray_char* string, uint8_t base);
uint64_t kh_stringToUint(khArray_char* string, uint8_t base);
double kh_stringToFloat(khArray_char* string, uint8_t base);

khArray_char kh_intToString(int64_t int_v, uint8_t base);
khArray_char kh_uintToString(uint64_t uint_v, uint8_t base);
khArray_char kh_floatToString(double floating, uint8_t precision, uint8_t base);

static inline khArray_char kh_string(const char* cstring) {
    khArray_byte buffer = khArray_byte_fromString((uint8_t*)cstring);
    khArray_char string = kh_decodeUtf8(&buffer);
    khArray_byte_delete(&buffer);
    return string;
}


#ifdef __cplusplus
}
#endif
#endif
