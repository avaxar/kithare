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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    size_t type_size;
    size_t size;
    size_t reserved;
    void (*deleter)(void*);
} _khArrayHeader;


// Getter macros from the header
#define khArray(TYPE) TYPE* // Type alias
#define khArray_header(ARRAY) (((_khArrayHeader*)(*ARRAY))[-1])
#define khArray_typeSize(ARRAY) (khArray_header(ARRAY).type_size)
#define khArray_size(ARRAY) (khArray_header(ARRAY).size)
#define khArray_reserved(ARRAY) (khArray_header(ARRAY).reserved)
#define khArray_deleter(ARRAY) (khArray_header(ARRAY).deleter)


#define khArray_new(TYPE, DELETER) (TYPE*)_khArray_new(sizeof(TYPE), (void (*)(void*))(DELETER))
static inline void* _khArray_new(size_t type_size, void (*deleter)(void*)) {
    // Don't forget to allocate an extra null-terminator space in case it's a string
    void* array = calloc(sizeof(_khArrayHeader) + type_size, 1);
    *(_khArrayHeader*)array =
        (_khArrayHeader){.type_size = type_size, .size = 0, .reserved = 0, .deleter = deleter};
    return array + sizeof(_khArrayHeader);
}

// The gnu11 standard must be used with some of the macros here. Since some utilize `typeof` and
// statement-expressions, which are not available in vanilla C
#define khArray_copy(ARRAY, COPIER)                                                                \
    ({                                                                                             \
        typeof(ARRAY) __kh_array_ptr = ARRAY;                                                      \
        typeof(*__kh_array_ptr) __kh_array = *__kh_array_ptr;                                      \
                                                                                                   \
        /* The copied array */                                                                     \
        typeof(__kh_array) __kh_copy =                                                             \
            calloc(sizeof(_khArrayHeader) +                                                        \
                       khArray_typeSize(__kh_array_ptr) * (khArray_size(__kh_array_ptr) + 1),      \
                   1);                                                                             \
                                                                                                   \
        /* Placing the array header and fitting the reserve count, then offsetting the copy */     \
        *(_khArrayHeader*)__kh_copy = khArray_header(__kh_array_ptr);                              \
        ((_khArrayHeader*)__kh_copy)->reserved = khArray_size(__kh_array_ptr);                     \
        __kh_copy = (typeof(__kh_array))((_khArrayHeader*)__kh_copy + 1);                          \
                                                                                                   \
        /* Call the copy constructor of each element, unless it's NULL */                          \
        if (COPIER == NULL) {                                                                      \
            memcpy(__kh_copy, __kh_array,                                                          \
                   khArray_typeSize(__kh_array_ptr) * khArray_size(__kh_array_ptr));               \
        }                                                                                          \
        else {                                                                                     \
            for (size_t __kh_i = 0; __kh_i < khArray_size(__kh_array_ptr); __kh_i++) {             \
                __kh_copy[__kh_i] =                                                                \
                    ((typeof (*__kh_array) (*)(typeof(__kh_array)))(COPIER))(&__kh_array[__kh_i]); \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        __kh_copy;                                                                                 \
    })


// Verifies that the argument given is a pointer to a pointer (e.g: int**, char**), then casts it into
// a void**
#define _kh_verifyPtrToPtr(PTR)                                            \
    ({                                                                     \
        typeof(PTR) __kh_ptr = PTR;                                        \
        typeof(**__kh_ptr) __kh_dptr __attribute__((unused)) = **__kh_ptr; \
        (void**)__kh_ptr;                                                  \
    })


#define khArray_delete(ARRAY) _khArray_delete(_kh_verifyPtrToPtr(ARRAY))
#define khArray_arrayDeleter(TYPE) ((void (*)(TYPE**))_khArray_delete)
static inline void _khArray_delete(void** array) {
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

#define khArray_reserve(ARRAY, SIZE) _khArray_reserve(_kh_verifyPtrToPtr(ARRAY), SIZE)
static inline void _khArray_reserve(void** array, size_t size) {
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

#define khArray_fit(ARRAY) _khArray_fit(_kh_verifyPtrToPtr(ARRAY));
static inline void _khArray_fit(void** array) {
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

#define khArray_pop(ARRAY, ITEMS) _khArray_pop(_kh_verifyPtrToPtr(ARRAY), ITEMS)
static inline void _khArray_pop(void** array, size_t items) {
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

#define khArray_reverse(ARRAY) _khArray_reverse(_kh_verifyPtrToPtr(ARRAY))
static inline void _khArray_reverse(void** array) {
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

#define khArray_memory(ARRAY, PTR, SIZE, COPIER)                                                      \
    {                                                                                                 \
        typeof(ARRAY) __kh_array_ptr = ARRAY;                                                         \
        typeof(PTR) __kh_ptr = PTR;                                                                   \
        size_t __kh_size = SIZE;                                                                      \
                                                                                                      \
        /* If the reserved memory isn't enough, try to allocate more by *2 of the existing memory */  \
        if (khArray_size(__kh_array_ptr) + __kh_size > khArray_reserved(__kh_array_ptr)) {            \
            size_t __kh_exsize = khArray_size(__kh_array_ptr) ? khArray_size(__kh_array_ptr) * 2 : 2; \
            khArray_reserve(__kh_array_ptr, __kh_exsize < khArray_size(__kh_array_ptr) + __kh_size    \
                                                ? khArray_size(__kh_array_ptr) + __kh_size            \
                                                : __kh_exsize);                                       \
        }                                                                                             \
                                                                                                      \
        typeof(*__kh_array_ptr) __kh_array = *__kh_array_ptr;                                         \
                                                                                                      \
        /* Call the copy constructor of each element, unless it's NULL */                             \
        if (COPIER == NULL) {                                                                         \
            memcpy(__kh_array + khArray_size(__kh_array_ptr), __kh_ptr,                               \
                   khArray_typeSize(__kh_array_ptr) * __kh_size);                                     \
        }                                                                                             \
        else {                                                                                        \
            for (size_t __kh_i = 0; __kh_i < __kh_size; __kh_i++) {                                   \
                __kh_array[khArray_size(__kh_array_ptr) + __kh_i] =                                   \
                    ((typeof (*__kh_array) (*)(typeof(__kh_array)))(COPIER))(__kh_ptr + __kh_i);      \
            }                                                                                         \
        }                                                                                             \
                                                                                                      \
        khArray_size(__kh_array_ptr) += __kh_size;                                                    \
    }

#define khArray_append(ARRAY, ITEM)                              \
    {                                                            \
        typeof(ARRAY) ___kh_array_ptr = ARRAY;                   \
        typeof(*___kh_array_ptr) ___kh_array = *___kh_array_ptr; \
        typeof(*___kh_array) ___kh_item = ITEM;                  \
                                                                 \
        khArray_memory(___kh_array_ptr, &___kh_item, 1, NULL);   \
    }

#define khArray_concatenate(ARRAY, OTHER, COPIER) \
    khArray_memory(ARRAY, *(OTHER), khArray_size(OTHER), COPIER)


#ifdef __cplusplus
}
#endif
