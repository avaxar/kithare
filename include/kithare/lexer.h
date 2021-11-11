/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_LEXER_H
#define kh_HG_LEXER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <kithare/token.h>


khToken kh_lex(char** cursor);
khToken kh_lexWord(char** cursor);
khToken kh_lexSymbol(char** cursor);
khToken kh_lexNumber(char** cursor);

uint32_t kh_lexChar(char** cursor, bool with_quotes);
khList_char kh_lexString(char** cursor, bool with_quotes);
khList_byte kh_lexBuffer(char** cursor, bool with_quotes);

uint32_t kh_lexUtf8(char** cursor);
uint64_t kh_lexInt(char** cursor, uint8_t base, bool* had_overflowed);
double kh_lexFloat(char** cursor, uint8_t base);

static inline uint8_t kh_digitOf(char chr) {
    if (chr >= '0' && chr <= '9') {
        return chr - '0';
    }
    else if (chr >= 'A' && chr <= 'Z') {
        return chr - 'A' + 10;
    }
    else if (chr >= 'a' && chr <= 'z') {
        return chr - 'a' + 10;
    }
    else {
        return 0xFF;
    }
}


#ifdef __cplusplus
}
#endif
#endif
