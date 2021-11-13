/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifdef khList_TYPE

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <kithare/macros.h>


#ifndef khList_NAME
#define khList_NAME kh_CONCAT(khList_, khList_TYPE)
#endif
#define khList_CONCAT(X) kh_CONCAT(khList_NAME, _##X)


typedef struct {
    khList_TYPE* array;
    size_t size;
    size_t reserved;
} khList_NAME;


static inline khList_NAME khList_CONCAT(new)() {
    return (khList_NAME){.array = NULL, .size = 0, .reserved = 0};
}

static inline khList_NAME khList_CONCAT(copy)(const khList_NAME* list) {
    if (!list->array || !list->size) {
        return khList_CONCAT(new)();
    }

    khList_TYPE* array = (khList_TYPE*)malloc(list->size * sizeof(khList_TYPE));

#ifdef khList_COPIER
    for (size_t i = 0; i < list->size; i++) {
        array[i] = khList_COPIER(&list->array[i]);
    }
#else
    memcpy(array, list->array, list->size * sizeof(khList_TYPE));
#endif

    return (khList_NAME){.array = array, .size = list->size, .reserved = list->size};
}

static inline void khList_CONCAT(delete)(khList_NAME* list) {
    if (list->array) {
#ifdef khList_DELETER
        for (size_t i = 0; i < list->size; i++) {
            khList_DELETER(&list->array[i]);
        }
#endif

        free(list->array);
        list->array = NULL;
    }

    list->size = 0;
    list->reserved = 0;
}

static inline void khList_CONCAT(reserve)(khList_NAME* list, size_t size) {
    if (list->reserved >= size) {
        return;
    }

    if (list->array) {
        khList_TYPE* array = (khList_TYPE*)malloc(size * sizeof(khList_TYPE));
        memcpy(array, list->array, list->size * sizeof(khList_TYPE));
        free(list->array);
        list->array = array;
    }
    else {
        list->array = (khList_TYPE*)malloc(size * sizeof(khList_TYPE));
    }

    list->reserved = size;
}

static inline void khList_CONCAT(fit)(khList_NAME* list) {
    if (!list->array) {
        return;
    }

    if (list->size) {
        if (list->size == list->reserved) {
            return;
        }

        khList_TYPE* array = (khList_TYPE*)malloc(list->size * sizeof(khList_TYPE));
        memcpy(array, list->array, list->size * sizeof(khList_TYPE));
        free(list->array);
        list->array = array;
        list->reserved = list->size;
    }
    else {
        khList_CONCAT(delete)(list);
    }
}

static inline void khList_CONCAT(push)(khList_NAME* list, khList_TYPE item) {
    if (list->size == list->reserved) {
        khList_CONCAT(reserve)(list, list->size ? list->size * 2 : 2);
    }

    list->array[list->size] = item;
    list->size++;
}

static inline void khList_CONCAT(pushPtr)(khList_NAME* list, const khList_TYPE* item) {
    if (list->size == list->reserved) {
        khList_CONCAT(reserve)(list, list->size ? list->size * 2 : 2);
    }

#ifdef khList_COPIER
    list->array[list->size] = khList_COPIER(item);
#else
    list->array[list->size] = *item;
#endif

    list->size++;
}

static inline void khList_CONCAT(memory)(khList_NAME* list, const khList_TYPE* address, size_t size) {
    khList_CONCAT(reserve)(list, list->size + size);

#ifdef khList_COPIER
    for (size_t i = 0; i < size; i++) {
        list->array[list->size + i] = khList_COPIER(&address[i]);
    }
#else
    memcpy(&list->array[list->size], address, size);
#endif

    list->size += size;
}

static inline khList_NAME khList_CONCAT(fromMemory)(const khList_TYPE* address, size_t size) {
    khList_NAME list = khList_CONCAT(new)();
    khList_CONCAT(memory)(&list, address, size);
    return list;
}

static inline void khList_CONCAT(string)(khList_NAME* list, const khList_TYPE* string) {
    static char ZEROES[sizeof(khList_TYPE)];
    static bool first = true; // TODO: handle the case of static variables in static functions
    if (first) {
        memset(ZEROES, 0, sizeof(khList_TYPE));
        first = false;
    }

    size_t size = 0;
    for (;; size++) {
        if (!memcmp(&string[size], ZEROES, sizeof(khList_TYPE))) {
            break;
        }
    }

    khList_CONCAT(memory)(list, string, size);
}

static inline khList_NAME khList_CONCAT(fromString)(const khList_TYPE* string) {
    khList_NAME list = khList_CONCAT(new)();
    khList_CONCAT(string)(&list, string);
    return list;
}

static inline void khList_CONCAT(concatenate)(khList_NAME* list, const khList_NAME* other) {
    if (!other->array) {
        return;
    }

    khList_CONCAT(memory)(list, other->array, other->size);
}

static inline void khList_CONCAT(pop)(khList_NAME* list, size_t elements) {
    if (elements > list->size) {
        elements = list->size;
    }

#ifdef khList_DELETER
    for (size_t i = elements; i > 0; i--) {
        khList_DELETER(&list->array[list->size - i]);
    }
#endif

    list->size -= elements;
}


#ifdef khList_COPIER
#undef khList_COPIER
#endif

#ifdef khList_DELETER
#undef khList_DELETER
#endif

#undef khList_NAME
#undef khList_TYPE
#else
// Ignore this error from your IDE while editing this header
#error "`khList_TYPE` hasn't been defined"
#endif
#ifdef __cplusplus
}
#endif
