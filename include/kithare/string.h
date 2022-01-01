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

#include <stdbool.h>
#include <stdint.h>

// Avaxar: Apple sucks
#ifdef __APPLE__
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#else
#include <uchar.h>
#endif

#include "array.h"


khArray(char32_t) kh_string(const char32_t* cstring);
khArray(uint8_t) kh_buffer(const char* cstring);

bool kh_compareString(khArray(char32_t) * a, khArray(char32_t) * b);
bool kh_compareBuffer(khArray(uint8_t) * a, khArray(uint8_t) * b);

khArray(uint8_t) kh_encodeUtf8(khArray(char32_t) * string);
khArray(char32_t) kh_decodeUtf8(khArray(uint8_t) * buffer);
char32_t kh_utf8(uint8_t** cursor);

khArray(char32_t) kh_escapeChar(char32_t chr);
khArray(char32_t) kh_quoteString(khArray(char32_t) * string);
khArray(char32_t) kh_quoteBuffer(khArray(uint8_t) * buffer);

int64_t kh_stringToInt(khArray(char32_t) * string, uint8_t base);
uint64_t kh_stringToUint(khArray(char32_t) * string, uint8_t base);
double kh_stringToFloat(khArray(char32_t) * string, uint8_t base);

khArray(char32_t) kh_intToString(int64_t int_v, uint8_t base);
khArray(char32_t) kh_uintToString(uint64_t uint_v, uint8_t base);
khArray(char32_t) kh_floatToString(double floating, uint8_t precision, uint8_t base);

static inline bool kh_compareCstring(khArray(char32_t) * a, const char32_t* b) {
    size_t length = 0;
    for (; b[length] != '\0'; length++) {}

    if (khArray_size(a) != length) {
        return false;
    }

    for (size_t i = 0; i < khArray_size(a); i++) {
        if ((*a)[i] != b[i]) {
            return false;
        }
    }

    return true;
}

static inline void kh_appendCstring(khArray(char32_t) * string, const char32_t* cstring) {
    size_t size = 0;
    for (; cstring[size] != U'\0'; size++) {}
    khArray_memory(string, (char32_t*)cstring, size, NULL);
}


#ifdef __cplusplus
}
#endif
