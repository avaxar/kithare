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

#include <kithare/arrays.h>
#include <kithare/token.h>


typedef struct {
    uint32_t* ptr;
    khArray_char error;
} khLexError;


static inline khLexError khLexError_copy(khLexError* error) {
    return (khLexError){.ptr = error->ptr, .error = khArray_char_copy(&error->error)};
}

static inline void khLexError_delete(khLexError* error) {
    khArray_char_delete(&error->error);
}


#define khArray_TYPE khLexError
#define khArray_COPIER khLexError_copy
#define khArray_DELETER khLexError_delete
#include <kithare/t_array.h>


khToken kh_lex(uint32_t** cursor, khArray_khLexError* errors);
khToken kh_lexWord(uint32_t** cursor, khArray_khLexError* errors);
khToken kh_lexNumber(uint32_t** cursor, khArray_khLexError* errors);
khToken kh_lexSymbol(uint32_t** cursor, khArray_khLexError* errors);

uint32_t kh_lexChar(uint32_t** cursor, bool with_quotes, bool is_byte, khArray_khLexError* errors);
khArray_char kh_lexString(uint32_t** cursor, bool is_buffer, khArray_khLexError* errors);

uint64_t kh_lexInt(uint32_t** cursor, uint8_t base, size_t max_length, bool* had_overflowed);
double kh_lexFloat(uint32_t** cursor, uint8_t base);


#ifdef __cplusplus
}
#endif
#endif
