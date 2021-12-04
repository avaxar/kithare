/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifdef khArray_TYPE

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define _kh_CONCAT(A, B) A##B
#define kh_CONCAT(A, B) _kh_CONCAT(A, B)

#ifndef khArray_NAME
#define khArray_NAME kh_CONCAT(khArray_, khArray_TYPE)
#endif
#define khArray_CONCAT(X) kh_CONCAT(khArray_NAME, _##X)


typedef struct {
    khArray_TYPE* array;
    size_t size;
    size_t reserved;
} khArray_NAME;


static inline khArray_NAME khArray_CONCAT(new)() {
    return (khArray_NAME){.array = calloc(sizeof(khArray_TYPE), 1), .size = 0, .reserved = 0};
}

static inline khArray_NAME khArray_CONCAT(copy)(khArray_NAME* array) {
    khArray_TYPE* ptr = (khArray_TYPE*)calloc(sizeof(khArray_TYPE), array->size + 1);

#ifdef khArray_COPIER
    for (size_t i = 0; i < array->size; i++) {
        ptr[i] = (khArray_TYPE)khArray_COPIER((khArray_TYPE*)&array->array[i]);
    }
#else
    memcpy(ptr, array->array, array->size * sizeof(khArray_TYPE));
#endif

    return (khArray_NAME){.array = ptr, .size = array->size, .reserved = array->size};
}

static inline void khArray_CONCAT(delete)(khArray_NAME* array) {
#ifdef khArray_DELETER
    for (size_t i = 0; i < array->size; i++) {
        khArray_DELETER((khArray_TYPE*)&array->array[i]);
    }
#endif

    free(array->array);
}

static inline void khArray_CONCAT(reserve)(khArray_NAME* array, size_t size) {
    if (array->reserved >= size) {
        return;
    }

    khArray_TYPE* ptr = (khArray_TYPE*)calloc(sizeof(khArray_TYPE), size + 1);
    memcpy(ptr, array->array, array->size * sizeof(khArray_TYPE));
    free(array->array);
    array->array = ptr;
    array->reserved = size;
}

static inline void khArray_CONCAT(fit)(khArray_NAME* array) {
    if (array->size == array->reserved) {
        return;
    }

    khArray_TYPE* ptr = (khArray_TYPE*)calloc(sizeof(khArray_TYPE), array->size + 1);
    memcpy(ptr, array->array, array->size * sizeof(khArray_TYPE));
    free(array->array);
    array->array = ptr;
    array->reserved = array->size;
}

static inline void khArray_CONCAT(push)(khArray_NAME* array, khArray_TYPE item) {
    if (array->size == array->reserved) {
        khArray_CONCAT(reserve)(array, array->size ? array->size * 2 : 2);
    }

    array->array[array->size] = item;
    array->size++;
}

static inline void khArray_CONCAT(pushPtr)(khArray_NAME* array, khArray_TYPE* item) {
    if (array->size == array->reserved) {
        khArray_CONCAT(reserve)(array, array->size ? array->size * 2 : 2);
    }

#ifdef khArray_COPIER
    array->array[array->size] = (khArray_TYPE)khArray_COPIER((khArray_TYPE*)item);
#else
    array->array[array->size] = *item;
#endif

    array->size++;
}

static inline void khArray_CONCAT(memory)(khArray_NAME* array, khArray_TYPE* address, size_t size) {
    khArray_CONCAT(reserve)(array, array->size + size);

#ifdef khArray_COPIER
    for (size_t i = 0; i < size; i++) {
        array->array[array->size + i] = (khArray_TYPE)khArray_COPIER((khArray_TYPE*)&address[i]);
    }
#else
    memcpy(&array->array[array->size], address, size * sizeof(khArray_TYPE));
#endif

    array->size += size;
}

static inline khArray_NAME khArray_CONCAT(fromMemory)(khArray_TYPE* address, size_t size) {
    khArray_NAME array = khArray_CONCAT(new)();
    khArray_CONCAT(memory)(&array, address, size);
    return array;
}

#ifndef kh_HG_T_ARRAY_H
#define kh_HG_T_ARRAY_H
// This is the sane way of declaring and zero-initialising an array in C
static int64_t kh_ZEROES[240] = {0};
#endif

static inline void khArray_CONCAT(string)(khArray_NAME* array, const khArray_TYPE* string) {
    size_t size = 0;
    for (; memcmp(&string[size], kh_ZEROES, sizeof(khArray_TYPE)); size++) {}
    khArray_CONCAT(memory)(array, (khArray_TYPE*)string, size);
}

static inline khArray_NAME khArray_CONCAT(fromString)(const khArray_TYPE* string) {
    khArray_NAME array = khArray_CONCAT(new)();
    khArray_CONCAT(string)(&array, string);
    return array;
}

static inline void khArray_CONCAT(concatenate)(khArray_NAME* array, khArray_NAME* other) {
    if (!other->array) {
        return;
    }

    khArray_CONCAT(memory)(array, other->array, other->size);
}

static inline void khArray_CONCAT(pop)(khArray_NAME* array, size_t elements) {
    if (elements > array->size) {
        elements = array->size;
    }

    for (size_t i = elements; i > 0; i--) {
#ifdef khArray_DELETER
        khArray_DELETER((khArray_TYPE*)&array->array[array->size - i]);
#endif
        memset(&array->array[array->size - i], 0, sizeof(khArray_TYPE));
    }

    array->size -= elements;
}

static inline void khArray_CONCAT(reverse)(khArray_NAME* array) {
    for (size_t i = 0; i < array->size / 2; i++) {
        khArray_TYPE tmp = array->array[i];
        array->array[i] = array->array[array->size - i - 1];
        array->array[array->size - i - 1] = tmp;
    }
}


#ifdef khArray_COPIER
#undef khArray_COPIER
#endif

#ifdef khArray_DELETER
#undef khArray_DELETER
#endif

#undef khArray_NAME
#undef khArray_TYPE
#else
// Ignore this error from your IDE while editing this header
#error "`khArray_TYPE` hasn't been defined"
#endif
#ifdef __cplusplus
}
#endif
