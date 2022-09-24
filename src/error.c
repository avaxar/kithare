/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/avaxar/Kithare
 * Copyright (C) 2022 Kithare Organization
 */

#include <stddef.h>

#include <kithare/core/error.h>
#include <kithare/lib/array.h>


static _Thread_local kharray(khError) error_stack = NULL;


void kh_raiseError(khError error) {
    if (error_stack == NULL) {
        error_stack = kharray_new(khError, khError_delete);
    }

    bool is_duplicate = false;
    for (size_t i = 0; i < kharray_size(&error_stack); i++) {
        khError* err = &error_stack[i];

        if (err->type == error.type && khstring_equal(&err->message, &error.message) &&
            err->data == error.data) {
            is_duplicate = true;
            break;
        }
    }

    if (is_duplicate) {
        khError_delete(&error);
    }
    else {
        kharray_append(&error_stack, error);
    }
}

size_t kh_hasErrors(void) {
    if (error_stack == NULL) {
        return 0;
    }
    else {
        return kharray_size(&error_stack);
    }
}

kharray(khError) * kh_getErrors(void) {
    if (error_stack == NULL) {
        error_stack = kharray_new(khError, khError_delete);
    }

    return &error_stack;
}

void kh_flushErrors(void) {
    if (error_stack != NULL) {
        kharray_delete(&error_stack);
        error_stack = NULL;
    }
}
