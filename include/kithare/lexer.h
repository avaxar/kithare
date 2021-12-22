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

#include "array.h"
#include "string.h"
#include "token.h"


typedef struct {
    char32_t* ptr;
    khArray(char32_t) error_str;
} khLexError;

static inline khLexError khLexError_copy(khLexError* error) {
    return (khLexError){.ptr = error->ptr, .error_str = khArray_copy(&error->error_str, NULL)};
}

static inline void khLexError_delete(khLexError* error) {
    khArray_delete(&error->error_str);
}


khToken kh_lex(char32_t** cursor, khArray(khLexError) * errors);
khToken kh_lexWord(char32_t** cursor, khArray(khLexError) * errors);
khToken kh_lexNumber(char32_t** cursor, khArray(khLexError) * errors);
khToken kh_lexSymbol(char32_t** cursor, khArray(khLexError) * errors);

char32_t kh_lexChar(char32_t** cursor, bool with_quotes, bool is_byte, khArray(khLexError) * errors);
khArray(char32_t) kh_lexString(char32_t** cursor, bool is_buffer, khArray(khLexError) * errors);

uint64_t kh_lexInt(char32_t** cursor, uint8_t base, size_t max_length, bool* had_overflowed);
double kh_lexFloat(char32_t** cursor, uint8_t base);


#ifdef __cplusplus
}
#endif
