/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/avaxar/Kithare
 * Copyright (C) 2022 Kithare Organization
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <kithare/lib/array.h>
#include <kithare/lib/string.h>


typedef enum { khErrorType_LEXER, khErrorType_PARSER, khErrorType_UNSPECIFIED } khErrorType;


typedef struct {
    khErrorType type;
    khstring message;
    void* data;
} khError;

static inline khError khError_copy(khError* error) {
    return (khError){
        .type = error->type, .message = khstring_copy(&error->message), .data = error->data};
}

static inline void khError_delete(khError* error) {
    khstring_delete(&error->message);
}


void kh_raiseError(khError error);
size_t kh_hasErrors(void);
kharray(khError) * kh_getErrors(void);
void kh_flushErrors(void);


#ifdef __cplusplus
}
#endif
