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

#include <kithare/core/token.h>
#include <kithare/lib/array.h>
#include <kithare/lib/string.h>


typedef struct {
    char32_t* ptr;
    khstring error_str;
} khLexError;

static inline khLexError khLexError_copy(khLexError* error) {
    return (khLexError){.ptr = error->ptr, .error_str = kharray_copy(&error->error_str, NULL)};
}

static inline void khLexError_delete(khLexError* error) {
    khstring_delete(&error->error_str);
}


khToken kh_lex(char32_t** cursor, kharray(khLexError) * errors);
khToken kh_lexWord(char32_t** cursor, kharray(khLexError) * errors);
khToken kh_lexNumber(char32_t** cursor, kharray(khLexError) * errors);
khToken kh_lexSymbol(char32_t** cursor, kharray(khLexError) * errors);

char32_t kh_lexChar(char32_t** cursor, bool with_quotes, bool is_byte, kharray(khLexError) * errors);
khstring kh_lexString(char32_t** cursor, bool is_buffer, kharray(khLexError) * errors);

uint64_t kh_lexInt(char32_t** cursor, uint8_t base, size_t max_length, bool* had_overflowed);
double kh_lexFloat(char32_t** cursor, uint8_t base);


#ifdef __cplusplus
}
#endif
