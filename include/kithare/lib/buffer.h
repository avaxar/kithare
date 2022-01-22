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

#include <stdint.h>

#include "array.h"
#include "string.h"


typedef kharray(uint8_t) khbuffer;


static inline khbuffer khbuffer_new(const char* cstring) {
    khbuffer buffer = kharray_new(uint8_t, NULL);
    kharray_memory(&buffer, (uint8_t*)cstring, strlen(cstring), NULL);
    return buffer;
}

static inline khbuffer khbuffer_copy(khbuffer* buffer) {
    return kharray_copy(buffer, NULL);
}

static inline void khbuffer_delete(khbuffer* buffer) {
    kharray_delete(buffer);
}

static inline size_t khbuffer_size(khbuffer* buffer) {
    return kharray_size(buffer);
}

static inline size_t khbuffer_reserved(khbuffer* buffer) {
    return kharray_reserved(buffer);
}

static inline void khbuffer_reserve(khbuffer* buffer, size_t size) {
    kharray_reserve(buffer, size);
}

static inline void khbuffer_fit(khbuffer* buffer) {
    kharray_fit(buffer);
}

static inline void khbuffer_pop(khbuffer* buffer, size_t items) {
    kharray_pop(buffer, items);
}

static inline void khbuffer_reverse(khbuffer* buffer) {
    kharray_reverse(buffer);
}

static inline void khbuffer_append(khbuffer* buffer, uint8_t chr) {
    kharray_append(buffer, chr);
}

static inline void khbuffer_concatenate(khbuffer* buffer, khbuffer* other) {
    kharray_concatenate(buffer, other, NULL);
}

static inline void khbuffer_concatenateCstring(khbuffer* buffer, const char* cstring) {
    kharray_memory(buffer, (uint8_t*)cstring, strlen(cstring), NULL);
}

static inline bool khbuffer_compare(khbuffer* a, khbuffer* b) {
    if (khbuffer_size(a) != khbuffer_size(b)) {
        return false;
    }

    for (size_t i = 0; i < khbuffer_size(a); i++) {
        if ((*a)[i] != (*b)[i]) {
            return false;
        }
    }

    return true;
}

static inline bool khbuffer_compareCstring(khbuffer* buffer, const char* cstring) {
    size_t length = strlen(cstring);
    if (khbuffer_size(buffer) != length) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        if ((*buffer)[i] != cstring[i]) {
            return false;
        }
    }

    return true;
}

static inline khstring khbuffer_quote(khbuffer* buffer) {
    khstring quoted_buffer = khstring_new(U"");

    khstring_append(&quoted_buffer, U'\"');
    for (uint8_t* byte = *buffer; byte < *buffer + khbuffer_size(buffer); byte++) {
        khstring escaped = kh_escapeChar(*byte);
        khstring_concatenate(&quoted_buffer, &escaped);
        khstring_delete(&escaped);
    }
    khstring_append(&quoted_buffer, U'\"');

    return quoted_buffer;
}


#ifdef __cplusplus
}
#endif
