/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <math.h>

#include <kithare/lexer.h>
#include <kithare/lists.h>


uint32_t kh_lexUtf8(char** cursor) {
    if ((uint8_t)(**cursor) < 128) {
        return *(*cursor)++;
    }

    uint32_t chr = 0;
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

    for ((*cursor)++; continuation > 0; continuation--, (*cursor)++) {
        if ((**cursor & 0b11000000) != 0b10000000) {
            return -1;
        }

        chr = (chr << 6) | (**cursor & 0b00111111);
    }

    return chr;
}

uint64_t kh_lexInt(char** cursor, uint8_t base, bool* had_overflowed) {
    uint64_t result = 0;
    *had_overflowed = false;

    while (kh_digitOf(**cursor) < base) {
        uint64_t previous = result;
        result *= base;
        result += kh_digitOf(**cursor);

        if (result < previous) {
            *had_overflowed = true;
        }

        (*cursor)++;
    }

    return result;
}

double kh_lexFloat(char** cursor, uint8_t base) {
    double result = 0;

    // The same implementation of kh_lexInt is used here. The reason of not using kh_lexInt directly is
    // to avoid any integer overflows.
    while (kh_digitOf(**cursor) < base) {
        result *= base;
        result += kh_digitOf(**cursor);
        (*cursor)++;
    }

    if (**cursor == '.') {
        (*cursor)++;
        double exponent = 1.l / base;

        while (kh_digitOf(**cursor) < base) {
            result += kh_digitOf(**cursor) * exponent;
            exponent /= base;
            (*cursor)++;
        }
    }

    bool had_overflowed;
    switch (**cursor) {
        case 'e':
        case 'E':
            (*cursor)++;
            if (**cursor == '-') {
                (*cursor)++;

                result *= pow(10, -kh_lexInt(cursor, 10, &had_overflowed));
                if (had_overflowed) {
                    result = 0.0;
                }
            }
            else {
                if (**cursor == '+') {
                    (*cursor)++;
                }

                result *= pow(10, kh_lexInt(cursor, 10, &had_overflowed));
                if (had_overflowed) {
                    result = INFINITY;
                }
            }

            break;

        case 'p':
        case 'P':
            (*cursor)++;
            if (**cursor == '-') {
                (*cursor)++;

                result *= pow(2, -kh_lexInt(cursor, 10, &had_overflowed));
                if (had_overflowed) {
                    result = 0.0;
                }
            }
            else {
                if (**cursor == '+') {
                    (*cursor)++;
                }

                result *= pow(2, kh_lexInt(cursor, 10, &had_overflowed));
                if (had_overflowed) {
                    result = INFINITY;
                }
            }

            break;
    }

    return result;
}
