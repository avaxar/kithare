/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
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


typedef kharray(uint8_t) khbuffer;
typedef kharray(char32_t) khstring;


static inline khstring khstring_new(const char32_t* cstring) {
    size_t size = 0;
    for (; cstring[size] != U'\0'; size++) {}

    khstring string = kharray_new(char32_t, NULL);
    kharray_memory(&string, (char32_t*)cstring, size, NULL);

    return string;
}

static inline khstring khstring_copy(khstring* string) {
    return kharray_copy(string, NULL);
}

static inline void khstring_delete(khstring* string) {
    kharray_delete(string);
}

static inline size_t khstring_size(khstring* string) {
    return kharray_size(string);
}

static inline size_t khstring_reserved(khstring* string) {
    return kharray_reserved(string);
}

static inline void khstring_reserve(khstring* string, size_t size) {
    kharray_reserve(string, size);
}

static inline void khstring_fit(khstring* string) {
    kharray_fit(string);
}

static inline void khstring_pop(khstring* string, size_t items) {
    kharray_pop(string, items);
}

static inline void khstring_reverse(khstring* string) {
    kharray_reverse(string);
}

static inline void khstring_append(khstring* string, char32_t chr) {
    kharray_append(string, chr);
}

static inline void khstring_concatenate(khstring* string, khstring* other) {
    kharray_concatenate(string, other, NULL);
}

static inline void khstring_concatenateCstring(khstring* string, const char32_t* cstring) {
    size_t size = 0;
    for (; cstring[size] != U'\0'; size++) {}
    kharray_memory(string, (char32_t*)cstring, size, NULL);
}

static inline bool khstring_equal(khstring* a, khstring* b) {
    if (khstring_size(a) != khstring_size(b)) {
        return false;
    }

    for (size_t i = 0; i < khstring_size(a); i++) {
        if ((*a)[i] != (*b)[i]) {
            return false;
        }
    }

    return true;
}

static inline bool khstring_equalCstring(khstring* a, const char32_t* b) {
    size_t length = 0;
    for (; b[length] != '\0'; length++) {}

    if (khstring_size(a) != length) {
        return false;
    }

    for (size_t i = 0; i < khstring_size(a); i++) {
        if ((*a)[i] != b[i]) {
            return false;
        }
    }

    return true;
}

static inline bool khstring_startswith(khstring* string, khstring* prefix) {
    if (khstring_size(string) < khstring_size(prefix)) {
        return false;
    }

    for (size_t i = 0; i < khstring_size(prefix); i++) {
        if ((*string)[i] != (*prefix)[i]) {
            return false;
        }
    }

    return true;
}

static inline bool khstring_startswithCstring(khstring* string, const char32_t* prefix) {
    size_t length = 0;
    for (; prefix[length] != '\0'; length++) {}

    if (khstring_size(string) < length) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        if ((*string)[i] != prefix[i]) {
            return false;
        }
    }

    return true;
}

static inline bool khstring_endswith(khstring* string, khstring* suffix) {
    if (khstring_size(string) < khstring_size(suffix)) {
        return false;
    }

    for (size_t i = 0; i < khstring_size(suffix); i++) {
        if ((*string)[khstring_size(string) - khstring_size(suffix) + i] != (*suffix)[i]) {
            return false;
        }
    }

    return true;
}

static inline bool khstring_endswithCstring(khstring* string, const char32_t* suffix) {
    size_t length = 0;
    for (; suffix[length] != '\0'; length++) {}

    if (khstring_size(string) < length) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        if ((*string)[khstring_size(string) - length + i] != suffix[i]) {
            return false;
        }
    }

    return true;
}

static inline khstring kh_uintToString(uint64_t uint_v, uint8_t base) {
    khstring string = khstring_new(U"");

    uint64_t value = uint_v;
    while (value > 0) {
        uint8_t digit = value % base;
        khstring_append(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
        value /= base;
    }

    if (khstring_size(&string) == 0) {
        khstring_append(&string, U'0');
    }

    kharray_reverse(&string);
    return string;
}

static inline khstring kh_intToString(int64_t int_v, uint8_t base) {
    khstring string = khstring_new(U"");

    if (int_v < 0) {
        khstring_append(&string, U'-');
        int_v *= -1;
    }

    khstring str = kh_uintToString(int_v, base);
    khstring_concatenate(&string, &str);
    khstring_delete(&str);

    return string;
}

static inline khstring kh_floatToString(double floating, uint8_t precision, uint8_t base) {
    khstring string = khstring_new(U"");

    if (floating < 0) {
        khstring_append(&string, U'-');
        floating *= -1;
    }

    if (isinf(floating)) {
        khstring_concatenateCstring(&string, U"inf");
    }
    else if (isnan(floating)) {
        khstring_concatenateCstring(&string, U"nan");
    }
    else {
        double value = floating;
        while (value >= 1) {
            uint8_t digit = (uint8_t)fmod(value, base);
            khstring_append(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
            value /= base;
        }

        kharray_reverse(&string);
        if (khstring_size(&string) == 0) {
            khstring_append(&string, U'0');
        }

        if (precision > 0) {
            khstring_append(&string, U'.');

            value = floating;
            for (uint8_t i = 0; i < precision; i++) {
                value *= base;
                uint8_t digit = (uint8_t)fmod(value, base);
                khstring_append(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
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

static inline khbuffer kh_encodeUtf8(khstring* string) {
    khbuffer buffer = kharray_new(uint8_t, NULL); // Can't use `khbuffer_new`
    kharray_reserve(&buffer, khstring_size(string));

    // Can't use `khbuffer_append`
    for (char32_t* chr = *string; chr < *string + khstring_size(string); chr++) {
        if (*chr > 0xFFFF) {
            kharray_append(&buffer, 0b11110000 | (uint8_t)(0b00000111 & (*chr >> 18)));
            kharray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 12)));
            kharray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 6)));
            kharray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else if (*chr > 0x7FF) {
            kharray_append(&buffer, 0b11100000 | (uint8_t)(0b00001111 & (*chr >> 12)));
            kharray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 6)));
            kharray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else if (*chr > 0x7F) {
            kharray_append(&buffer, 0b11000000 | (uint8_t)(0b00011111 & (*chr >> 6)));
            kharray_append(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else {
            kharray_append(&buffer, *chr);
        }
    }

    return buffer;
}

static inline khstring kh_decodeUtf8(khbuffer* buffer) {
    khstring string = khstring_new(U"");
    kharray_reserve(&string, kharray_size(buffer)); // Can't use khbuffer_size

    uint8_t* cursor = *buffer;
    while (cursor != *buffer + kharray_size(buffer)) {
        khstring_append(&string, kh_utf8(&cursor));
    }

    return string;
}

static inline khstring kh_escapeChar(char32_t chr) {
    switch (chr) {
        // Regular single character escapes
        case U'\0':
            return khstring_new(U"\\0");
        case U'\n':
            return khstring_new(U"\\n");
        case U'\r':
            return khstring_new(U"\\r");
        case U'\t':
            return khstring_new(U"\\t");
        case U'\v':
            return khstring_new(U"\\v");
        case U'\b':
            return khstring_new(U"\\b");
        case U'\a':
            return khstring_new(U"\\a");
        case U'\f':
            return khstring_new(U"\\f");
        case U'\\':
            return khstring_new(U"\\\\");
        case U'\'':
            return khstring_new(U"\\\'");
        case U'\"':
            return khstring_new(U"\\\"");

        default: {
            khstring string = khstring_new(U"");

            if (iswprint(chr)) {
                khstring_append(&string, chr);
                return string;
            }

            uint8_t chars;
            if (chr < 0x100) {
                khstring_concatenateCstring(&string, U"\\x");
                chars = 2;
            }
            else if (chr < 0x10000) {
                khstring_concatenateCstring(&string, U"\\u");
                chars = 4;
            }
            else {
                khstring_concatenateCstring(&string, U"\\U");
                chars = 8;
            }

            // Fills the placeholder zeroes \x0AAA
            khstring hex = kh_uintToString(chr, 16);
            for (uint8_t i = 0; i < (uint8_t)khstring_size(&hex) - chars; i++) {
                khstring_append(&string, U'0');
            }

            khstring_concatenate(&string, &hex);
            khstring_delete(&hex);

            return string;
        }
    }
}

static inline khstring khstring_quote(khstring* string) {
    khstring quoted_string = khstring_new(U"\"");

    for (char32_t* chr = *string; chr < *string + khstring_size(string); chr++) {
        khstring escaped = kh_escapeChar(*chr);
        khstring_concatenate(&quoted_string, &escaped);
        khstring_delete(&escaped);
    }

    khstring_append(&quoted_string, U'\"');
    return quoted_string;
}


#ifdef __cplusplus
}
#endif
