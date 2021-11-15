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

#include <kithare/error.h>
#include <kithare/token.h>


khToken kh_lex(char** cursor);
khToken kh_lexWord(char** cursor);
khToken kh_lexNumber(char** cursor);
khToken kh_lexSymbol(char** cursor);

uint32_t kh_lexChar(char** cursor, bool with_quotes);
khArray_char kh_lexString(char** cursor);

uint32_t kh_lexUtf8(char** cursor, bool skip);
uint64_t kh_lexInt(char** cursor, uint8_t base, size_t max_length, bool* had_overflowed);
double kh_lexFloat(char** cursor, uint8_t base);


#ifdef __cplusplus
}
#endif
#endif
