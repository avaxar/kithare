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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wctype.h>

// Avaxar: Apple sucks
#ifdef __APPLE__
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#else
#include <uchar.h>
#endif

#include "array.h"


static inline khArray(char32_t) kh_string(const char32_t* cstring) {
    size_t size = 0;
    for (; cstring[size] != U'\0'; size++) {}

    khArray(char32_t) string = khArray_new(char32_t, NULL);
    khArray_memory(&string, (char32_t*)cstring, size, NULL);

    return string;
}

static inline khArray(uint8_t) kh_buffer(const char* cstring) {
    khArray(uint8_t) buffer = khArray_new(uint8_t, NULL);
    khArray_memory(&buffer, (uint8_t*)cstring, strlen(cstring), NULL);
    return buffer;
}

static inline void kh_appendCstring(khArray(char32_t) * string, const char32_t* cstring) {
    size_t size = 0;
    for (; cstring[size] != U'\0'; size++) {}
    khArray_memory(string, (char32_t*)cstring, size, NULL);
}

static inline bool kh_compareString(khArray(char32_t) * a, khArray(char32_t) * b) {
    if (khArray_size(a) != khArray_size(b)) {
        return false;
    }

    for (size_t i = 0; i < khArray_size(a); i++) {
        if ((*a)[i] != (*b)[i]) {
            return false;
        }
    }

    return true;
}

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

static inline bool kh_compareBuffer(khArray(uint8_t) * a, khArray(uint8_t) * b) {
    if (khArray_size(a) != khArray_size(b)) {
        return false;
    }

    for (size_t i = 0; i < khArray_size(a); i++) {
        if ((*a)[i] != (*b)[i]) {
            return false;
        }
    }

    return true;
}

static inline khArray(char32_t) kh_uintToString(uint64_t uint_v, uint8_t base) {
    khArray(char32_t) string = khArray_new(char32_t, NULL);

    uint64_t value = uint_v;
    while (value > 0) {
        uint8_t digit = value % base;
        khArray_append(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
        value /= base;
    }

    if (khArray_size(&string) == 0) {
        khArray_append(&string, U'0');
    }

    khArray_reverse(&string);
    return string;
}

static inline khArray(char32_t) kh_intToString(int64_t int_v, uint8_t base) {
    khArray(char32_t) string = khArray_new(char32_t, NULL);

    if (int_v < 0) {
        khArray_append(&string, U'-');
        int_v *= -1;
    }

    khArray(char32_t) str = kh_uintToString(int_v, base);
    khArray_concatenate(&string, &str, NULL);
    khArray_delete(&str);

    return string;
}

static inline khArray(char32_t) kh_floatToString(double floating, uint8_t precision, uint8_t base) {
    khArray(char32_t) string = khArray_new(char32_t, NULL);

    if (floating < 0) {
        khArray_append(&string, U'-');
        floating *= -1;
    }

    if (isinf(floating)) {
        kh_appendCstring(&string, U"inf");
    }
    else if (isnan(floating)) {
        kh_appendCstring(&string, U"nan");
    }
    else {
        double value = floating;
        while (value >= 1) {
            uint8_t digit = (uint8_t)fmod(value, base);
            khArray_append(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
            value /= base;
        }

        khArray_reverse(&string);
        if (khArray_size(&string) == 0) {
            khArray_append(&string, U'0');
        }

        if (precision > 0) {
            khArray_append(&string, U'.');

            value = floating;
            for (uint8_t i = 0; i < precision; i++) {
                value *= base;
                uint8_t digit = (uint8_t)fmod(value, base);
                khArray_append(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
            }
        }
    }

    return string;
}

static inline char32_t kh_utf8(uint8_t** cursor) {
    // Pass ASCII characters
    if ((uint8_t)(**cursor) < 128) {
        return *(*cursor)++;
    }

    char32_t chr = 0;
    uint8_t continuation = 0;

    if ((**cursor & 0b11100000) == 0b11000000) {
        chr = **cursor & 0b00011111;
        continuation = 1;
    }
    else if ((**cursor & 0b11110000) == 0b11100000) {
        chr = **cursor & 0b00001111;
        continuation = 2;
    }
    else if ((**cursor & 0b11111000) == 0b11110000) {
        chr = **cursor & 0b00000111;
        continuation = 3;
    }
    else if ((**cursor & 0b11111100) == 0b11111000) {
        chr = **cursor & 0b00000011;
        continuation = 4;
    }
    else if ((**cursor & 0b11111110) == 0b11111100) {
        chr = **cursor & 0b00000001;
        continuation = 5;
    }

    for ((*cursor)++; continuation > 0; continuation--, (*cursor)++) {
        if ((**cursor & 0b11000000) != 0b10000000) {
            return -1;
        }

        chr = (chr << 6) | (**cursor & 0b00111111);
    }

    return chr;
}

static inline khArray(uint8_t) kh_encodeUtf8(khArray(char32_t) * string) {
    khArray(uint8_t) buffer = khArray_new(uint8_t, NULL);
    khArray_reserve(&buffer, khArray_size(string));

    for (char32_t* chr = *string; chr < *string + khArray_size(string); chr++) {
        if (*chr > 0xFFFF) {
            khArray_append(&buffer, 0b11110000 | (uint8_t)(0b00000111 & (*chr >> 18)));
            khArray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 12)));
            khArray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 6)));
            khArray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else if (*chr > 0x7FF) {
            khArray_append(&buffer, 0b11100000 | (uint8_t)(0b00001111 & (*chr >> 12)));
            khArray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 6)));
            khArray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else if (*chr > 0x7F) {
            khArray_append(&buffer, 0b11000000 | (uint8_t)(0b00011111 & (*chr >> 6)));
            khArray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else {
            khArray_append(&buffer, *chr);
        }
    }

    return buffer;
}

static inline khArray(char32_t) kh_decodeUtf8(khArray(uint8_t) * buffer) {
    khArray(char32_t) string = khArray_new(char32_t, NULL);
    khArray_reserve(&string, khArray_size(buffer));

    uint8_t* cursor = *buffer;
    while (cursor != *buffer + khArray_size(buffer)) {
        khArray_append(&string, kh_utf8(&cursor));
    }

    return string;
}

static inline khArray(char32_t) kh_escapeChar(char32_t chr) {
    switch (chr) {
        // Regular single character escapes
        case U'\0':
            return kh_string(U"\\0");
        case U'\n':
            return kh_string(U"\\n");
        case U'\r':
            return kh_string(U"\\r");
        case U'\t':
            return kh_string(U"\\t");
        case U'\v':
            return kh_string(U"\\v");
        case U'\b':
            return kh_string(U"\\b");
        case U'\a':
            return kh_string(U"\\a");
        case U'\f':
            return kh_string(U"\\f");
        case U'\\':
            return kh_string(U"\\\\");
        case U'\'':
            return kh_string(U"\\\'");
        case U'\"':
            return kh_string(U"\\\"");

        default: {
            khArray(char32_t) string = khArray_new(char32_t, NULL);

            if (iswprint(chr)) {
                khArray_append(&string, chr);
                return string;
            }

            uint8_t chars;
            if (chr < 0x100) {
                kh_appendCstring(&string, U"\\x");
                chars = 2;
            }
            else if (chr < 0x10000) {
                kh_appendCstring(&string, U"\\u");
                chars = 4;
            }
            else {
                kh_appendCstring(&string, U"\\U");
                chars = 8;
            }

            // Fills the placeholder zeroes \x0AAA
            khArray(char32_t) hex = kh_uintToString(chr, 16);
            for (uint8_t i = 0; i < (uint8_t)khArray_size(&hex) - chars; i++) {
                khArray_append(&string, U'0');
            }

            khArray_concatenate(&string, &hex, NULL);
            khArray_delete(&hex);

            return string;
        }
    }
}

static inline khArray(char32_t) kh_quoteString(khArray(char32_t) * string) {
    khArray(char32_t) quoted_string = khArray_new(char32_t, NULL);

    khArray_append(&quoted_string, U'\"');
    for (char32_t* chr = *string; chr < *string + khArray_size(string); chr++) {
        khArray(char32_t) escaped = kh_escapeChar(*chr);
        khArray_concatenate(&quoted_string, &escaped, NULL);
        khArray_delete(&escaped);
    }
    khArray_append(&quoted_string, U'\"');

    return quoted_string;
}

static inline khArray(char32_t) kh_quoteBuffer(khArray(uint8_t) * buffer) {
    khArray(char32_t) quoted_buffer = khArray_new(char32_t, NULL);

    khArray_append(&quoted_buffer, U'\"');
    for (uint8_t* byte = *buffer; byte < *buffer + khArray_size(buffer); byte++) {
        khArray(char32_t) escaped = kh_escapeChar(*byte);
        khArray_concatenate(&quoted_buffer, &escaped, NULL);
        khArray_delete(&escaped);
    }
    khArray_append(&quoted_buffer, U'\"');

    return quoted_buffer;
}


#ifdef __cplusplus
}
#endif
