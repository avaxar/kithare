/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#include <kithare/array.h>


void* _khArray_new(size_t type_size, void (*deleter)(void*)) {
    // Don't forget to allocate an extra null-terminator space in case it's a string
    void* array = calloc(sizeof(_khArrayHeader) + type_size, 1);
    *(_khArrayHeader*)array =
        (_khArrayHeader){.type_size = type_size, .size = 0, .reserved = 0, .deleter = deleter};
    return array + sizeof(_khArrayHeader);
}

void _khArray_delete(void** array) {
    if (khArray_deleter(array) != NULL) {
        // Increment by type size
        for (size_t i = 0; i < khArray_typeSize(array) * khArray_size(array);
             i += khArray_typeSize(array)) {
            khArray_deleter(array)(*array + i);
        }
    }

    // Don't forget to undo the header offset before freeing it
    free(*array - sizeof(_khArrayHeader));
    *array = NULL;
}

void _khArray_reserve(void** array, size_t size) {
    if (khArray_reserved(array) >= size) {
        return;
    }

    // Also, don't forget the null-terminator space
    void* expanded_array = calloc(sizeof(_khArrayHeader) + khArray_typeSize(array) * (size + 1), 1);
    *(_khArrayHeader*)expanded_array = khArray_header(array);
    memcpy(expanded_array + sizeof(_khArrayHeader), *array,
           khArray_typeSize(array) * khArray_size(array));
    // Undo offset, free!
    free(*array - sizeof(_khArrayHeader));

    ((_khArrayHeader*)expanded_array)->reserved = size;
    *array = expanded_array + sizeof(_khArrayHeader);
}

void _khArray_fit(void** array) {
    // Pretty much the same implementation of `_khArray_reserve` but with this part different
    if (khArray_reserved(array) == khArray_size(array)) {
        return;
    }

    void* shrunk_array =
        calloc(sizeof(_khArrayHeader) + khArray_typeSize(array) * (khArray_size(array) + 1), 1);
    *(_khArrayHeader*)shrunk_array = khArray_header(array);
    memcpy(shrunk_array + sizeof(_khArrayHeader), *array,
           khArray_typeSize(array) * khArray_size(array));
    free(*array - sizeof(_khArrayHeader));

    ((_khArrayHeader*)shrunk_array)->reserved = khArray_size(array);
    *array = shrunk_array + sizeof(_khArrayHeader);
}

void _khArray_pop(void** array, size_t items) {
    // If it's trying to pop more items than the array actually has, cap it
    items = items > khArray_size(array) ? khArray_size(array) : items;

    if (khArray_deleter(array) != NULL) {
        for (size_t i = items; i > 0; i--) {
            khArray_deleter(array)(*array + khArray_typeSize(array) * (khArray_size(array) - i));
        }
    }

    // Clean after yourself
    memset(*array + khArray_typeSize(array) * (khArray_size(array) - items), 0,
           khArray_typeSize(array) * items);
    khArray_size(array) -= items;
}

void _khArray_reverse(void** array) {
    for (size_t i = 0; i < khArray_size(array) / 2; i++) {
        char tmp[khArray_typeSize(array)];

        // front -> tmp
        memcpy(tmp, *array + i * khArray_typeSize(array), khArray_typeSize(array));

        // back -> front
        memcpy(*array + i * khArray_typeSize(array),
               *array + (khArray_size(array) - i - 1) * khArray_typeSize(array),
               khArray_typeSize(array));

        // tmp -> back
        memcpy(*array + (khArray_size(array) - i - 1) * khArray_typeSize(array), tmp,
               khArray_typeSize(array));
    }
}
