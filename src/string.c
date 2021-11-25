/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <math.h>
#include <string.h>
#include <wctype.h>

#include <kithare/lexer.h>
#include <kithare/string.h>


bool kh_compareString(khArray_char* a, khArray_char* b) {
    if (a->size != b->size) {
        return false;
    }

    for (size_t i = 0; i < a->size; i++) {
        if (a->array[i] != b->array[i]) {
            return false;
        }
    }

    return true;
}

bool kh_compareBuffer(khArray_byte* a, khArray_byte* b) {
    if (a->size != b->size) {
        return false;
    }

    for (size_t i = 0; i < a->size; i++) {
        if (a->array[i] != b->array[i]) {
            return false;
        }
    }

    return true;
}

bool kh_compareCstring(khArray_char* a, const char32_t* b) {
    size_t length = 0;
    for (; b[length] != '\0'; length++) {}

    if (a->size != length) {
        return false;
    }

    for (size_t i = 0; i < a->size; i++) {
        if (a->array[i] != b[i]) {
            return false;
        }
    }

    return true;
}

khArray_byte kh_encodeUtf8(khArray_char* string) {
    khArray_byte buffer = khArray_byte_new();
    khArray_byte_reserve(&buffer, string->size);

    for (char32_t* chr = string->array; chr < string->array + string->size; chr++) {
        if (*chr > 0xFFFF) {
            khArray_byte_push(&buffer, 0b11110000 | (uint8_t)(0b00000111 & (*chr >> 18)));
            khArray_byte_push(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 12)));
            khArray_byte_push(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 6)));
            khArray_byte_push(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else if (*chr > 0x7FF) {
            khArray_byte_push(&buffer, 0b11100000 | (uint8_t)(0b00001111 & (*chr >> 12)));
            khArray_byte_push(&buffer, 0b10000000 | (uint8_t)(0b00111111 & (*chr >> 6)));
            khArray_byte_push(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else if (*chr > 0x7F) {
            khArray_byte_push(&buffer, 0b11000000 | (uint8_t)(0b00011111 & (*chr >> 6)));
            khArray_byte_push(&buffer, 0b10000000 | (uint8_t)(0b00111111 & *chr));
        }
        else {
            khArray_byte_push(&buffer, *chr);
        }
    }

    return buffer;
}

khArray_char kh_decodeUtf8(khArray_byte* buffer) {
    khArray_char string = khArray_char_new();
    khArray_char_reserve(&string, buffer->size);

    uint8_t* cursor = buffer->array;
    while (cursor != &buffer->array[buffer->size]) {
        khArray_char_push(&string, kh_utf8(&cursor));
    }

    return string;
}

char32_t kh_utf8(uint8_t** cursor) {
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

khArray_char kh_escapeChar(char32_t chr) {
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
            khArray_char string = khArray_char_new();

            if (iswalnum(chr) || iswspace(chr)) {
                khArray_char_push(&string, chr);
                return string;
            }

            uint8_t chars;
            if (chr < 0x100) {
                khArray_char_string(&string, U"\\x");
                chars = 2;
            }
            else if (chr < 0x10000) {
                khArray_char_string(&string, U"\\u");
                chars = 4;
            }
            else {
                khArray_char_string(&string, U"\\U");
                chars = 8;
            }

            // Fills the placeholder zeroes \x0AAA
            khArray_char hex = kh_uintToString(chr, 16);
            for (uint8_t i = 0; i < hex.size - chars; i++) {
                khArray_char_push(&string, U'0');
            }

            khArray_char_concatenate(&string, &hex);
            khArray_char_delete(&hex);

            return string;
        }
    }
}

khArray_char kh_quoteString(khArray_char* string) {
    khArray_char quoted_string = khArray_char_new();

    khArray_char_push(&quoted_string, U'\"');
    for (char32_t* chr = string->array; chr < string->array + string->size; chr++) {
        khArray_char escaped = kh_escapeChar(*chr);
        khArray_char_concatenate(&quoted_string, &escaped);
        khArray_char_delete(&escaped);
    }
    khArray_char_push(&quoted_string, U'\"');

    return quoted_string;
}

khArray_char kh_quoteBuffer(khArray_byte* buffer) {
    khArray_char quoted_buffer = khArray_char_new();

    khArray_char_string(&quoted_buffer, U"b\"");
    for (uint8_t* byte = buffer->array; byte < buffer->array + buffer->size; byte++) {
        khArray_char escaped = kh_escapeChar(*byte);
        khArray_char_concatenate(&quoted_buffer, &escaped);
        khArray_char_delete(&escaped);
    }
    khArray_char_push(&quoted_buffer, U'\"');

    return quoted_buffer;
}

int64_t kh_stringToInt(khArray_char* string, uint8_t base) {
    bool is_negative = false;
    if (*string->array) {
        is_negative = true;
    }

    char32_t* ptr = string->array;
    int64_t int_v = kh_lexInt(&ptr, base, -1, NULL) * -(int64_t)is_negative;

    if (ptr != string->array + string->size) {
        // TODO: handle error
        return 0;
    }

    return int_v;
}

uint64_t kh_stringToUint(khArray_char* string, uint8_t base) {
    char32_t* ptr = string->array;
    uint64_t int_v = kh_lexInt(&ptr, base, -1, NULL);

    if (ptr != string->array + string->size) {
        // TODO: handle error
        return 0;
    }

    return int_v;
}

double kh_stringToFloat(khArray_char* string, uint8_t base) {
    bool is_negative = false;
    if (*string->array) {
        is_negative = true;
    }

    char32_t* ptr = string->array;
    double floating = kh_lexFloat(&ptr, base) * -(int64_t)is_negative;

    if (ptr != string->array + string->size) {
        // TODO: handle error
        return 0;
    }

    return floating;
}

khArray_char kh_intToString(int64_t int_v, uint8_t base) {
    khArray_char string = khArray_char_new();

    if (int_v < 0) {
        khArray_char_push(&string, U'-');
        int_v *= -1;
    }

    khArray_char str = kh_uintToString(int_v, base);
    khArray_char_concatenate(&string, &str);
    khArray_char_delete(&str);

    return string;
}

khArray_char kh_uintToString(uint64_t uint_v, uint8_t base) {
    khArray_char string = khArray_char_new();

    uint64_t value = uint_v;
    while (value > 0) {
        uint8_t digit = value % base;
        khArray_char_push(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
        value /= base;
    }

    if (string.size == 0) {
        khArray_char_push(&string, U'0');
    }

    khArray_char_reverse(&string);
    return string;
}

khArray_char kh_floatToString(double floating, uint8_t precision, uint8_t base) {
    khArray_char string = khArray_char_new();

    if (floating < 0) {
        khArray_char_push(&string, U'-');
        floating *= -1;
    }

    double value = floating;
    while (value >= 1) {
        uint8_t digit = (uint8_t)fmod(value, base);
        khArray_char_push(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
        value /= base;
    }

    khArray_char_reverse(&string);

    if (precision > 0) {
        khArray_char_push(&string, U'.');

        value = floating;
        for (uint8_t i = 0; i < precision; i++) {
            value *= base;
            uint8_t digit = (uint8_t)fmod(value, base);
            khArray_char_push(&string, (digit < 10 ? U'0' + digit : U'A' + digit - 10));
        }
    }

    return string;
}
