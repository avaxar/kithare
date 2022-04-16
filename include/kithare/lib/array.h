/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
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
    void (*deleter)(void*);
    size_t size;
    size_t reserved;
} _kharrayHeader;


// Getter macros from the header
#define kharray(TYPE) TYPE* // Type alias
#define _kharray_header(ARRAY) (((_kharrayHeader*)(*ARRAY))[-1])
#define _kharray_typeSize(ARRAY) (_kharray_header(ARRAY).type_size)
#define _kharray_deleter(ARRAY) (_kharray_header(ARRAY).deleter)
#define kharray_size(ARRAY) (_kharray_header(ARRAY).size)
#define kharray_reserved(ARRAY) (_kharray_header(ARRAY).reserved)

// Verifies that the argument given is a pointer to a pointer (Known as a pointer to an array; e.g:
// int**, char**), then casts it into a void**
#define _kharray_verify(PTR)                                               \
    ({                                                                     \
        typeof(PTR) __kh_ptr = PTR;                                        \
        typeof(**__kh_ptr) __kh_dptr __attribute__((unused)) = **__kh_ptr; \
        (void**)__kh_ptr;                                                  \
    })


#define kharray_new(TYPE, DELETER) (TYPE*)_kharray_new(sizeof(TYPE), (void (*)(void*))(DELETER))
static inline void* _kharray_new(size_t type_size, void (*deleter)(void*)) {
    // Don't forget to allocate an extra null-terminator space in case it's a string
    void* array = calloc(sizeof(_kharrayHeader) + type_size, 1);
    *(_kharrayHeader*)array =
        (_kharrayHeader){.type_size = type_size, .size = 0, .reserved = 0, .deleter = deleter};
    return array + sizeof(_kharrayHeader);
}

#define kharray_copy(ARRAY, COPIER)                                                                \
    ({                                                                                             \
        typeof(ARRAY) __kh_array_ptr = ARRAY;                                                      \
        typeof(*__kh_array_ptr) __kh_array = *__kh_array_ptr;                                      \
                                                                                                   \
        /* The copied array */                                                                     \
        typeof(__kh_array) __kh_copy =                                                             \
            calloc(sizeof(_kharrayHeader) +                                                        \
                       _kharray_typeSize(__kh_array_ptr) * (kharray_size(__kh_array_ptr) + 1),     \
                   1);                                                                             \
                                                                                                   \
        /* Placing the array header and fitting the reserve count, then offsetting the copy */     \
        *(_kharrayHeader*)__kh_copy = _kharray_header(__kh_array_ptr);                             \
        ((_kharrayHeader*)__kh_copy)->reserved = kharray_size(__kh_array_ptr);                     \
        __kh_copy = (typeof(__kh_array))((_kharrayHeader*)__kh_copy + 1);                          \
                                                                                                   \
        /* Call the copy constructor of each element, unless it's NULL */                          \
        if (COPIER == NULL) {                                                                      \
            memcpy(__kh_copy, __kh_array,                                                          \
                   _kharray_typeSize(__kh_array_ptr) * kharray_size(__kh_array_ptr));              \
        }                                                                                          \
        else {                                                                                     \
            for (size_t __kh_i = 0; __kh_i < kharray_size(__kh_array_ptr); __kh_i++) {             \
                __kh_copy[__kh_i] =                                                                \
                    ((typeof (*__kh_array) (*)(typeof(__kh_array)))(COPIER))(&__kh_array[__kh_i]); \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        __kh_copy;                                                                                 \
    })

#define kharray_delete(ARRAY) _kharray_delete(_kharray_verify(ARRAY))
#define kharray_arrayDeleter(TYPE) ((void (*)(TYPE**))_kharray_delete)
static inline void _kharray_delete(void** array) {
    if (_kharray_deleter(array) != NULL) {
        // Increment by type size
        for (size_t i = 0; i < _kharray_typeSize(array) * kharray_size(array);
             i += _kharray_typeSize(array)) {
            _kharray_deleter(array)(*array + i);
        }
    }

    // Don't forget to undo the header offset before freeing it
    free(*array - sizeof(_kharrayHeader));
    *array = NULL;
}

#define kharray_reserve(ARRAY, SIZE) _kharray_reserve(_kharray_verify(ARRAY), SIZE)
static inline void _kharray_reserve(void** array, size_t size) {
    if (kharray_reserved(array) >= size) {
        return;
    }

    // Also, don't forget the null-terminator space
    void* expanded_array = calloc(sizeof(_kharrayHeader) + _kharray_typeSize(array) * (size + 1), 1);
    *(_kharrayHeader*)expanded_array = _kharray_header(array);
    memcpy(expanded_array + sizeof(_kharrayHeader), *array,
           _kharray_typeSize(array) * kharray_size(array));
    // Undo offset, free!
    free(*array - sizeof(_kharrayHeader));

    ((_kharrayHeader*)expanded_array)->reserved = size;
    *array = expanded_array + sizeof(_kharrayHeader);
}

#define kharray_fit(ARRAY) _kharray_fit(_kharray_verify(ARRAY));
static inline void _kharray_fit(void** array) {
    // Pretty much the same implementation of `_kharray_reserve` but with this part different
    if (kharray_reserved(array) == kharray_size(array)) {
        return;
    }

    void* shrunk_array =
        calloc(sizeof(_kharrayHeader) + _kharray_typeSize(array) * (kharray_size(array) + 1), 1);
    *(_kharrayHeader*)shrunk_array = _kharray_header(array);
    memcpy(shrunk_array + sizeof(_kharrayHeader), *array,
           _kharray_typeSize(array) * kharray_size(array));
    free(*array - sizeof(_kharrayHeader));

    ((_kharrayHeader*)shrunk_array)->reserved = kharray_size(array);
    *array = shrunk_array + sizeof(_kharrayHeader);
}

#define kharray_pop(ARRAY, ITEMS) _kharray_pop(_kharray_verify(ARRAY), ITEMS)
static inline void _kharray_pop(void** array, size_t items) {
    // If it's trying to pop more items than the array actually has, cap it
    items = items > kharray_size(array) ? kharray_size(array) : items;

    if (_kharray_deleter(array) != NULL) {
        for (size_t i = items; i > 0; i--) {
            _kharray_deleter(array)(*array + _kharray_typeSize(array) * (kharray_size(array) - i));
        }
    }

    // Clean after yourself
    memset(*array + _kharray_typeSize(array) * (kharray_size(array) - items), 0,
           _kharray_typeSize(array) * items);
    kharray_size(array) -= items;
}

#define kharray_reverse(ARRAY) _kharray_reverse(_kharray_verify(ARRAY))
static inline void _kharray_reverse(void** array) {
    for (size_t i = 0; i < kharray_size(array) / 2; i++) {
        char tmp[_kharray_typeSize(array)];

        // front -> tmp
        memcpy(tmp, *array + i * _kharray_typeSize(array), _kharray_typeSize(array));

        // back -> front
        memcpy(*array + i * _kharray_typeSize(array),
               *array + (kharray_size(array) - i - 1) * _kharray_typeSize(array),
               _kharray_typeSize(array));

        // tmp -> back
        memcpy(*array + (kharray_size(array) - i - 1) * _kharray_typeSize(array), tmp,
               _kharray_typeSize(array));
    }
}

#define kharray_memory(ARRAY, PTR, SIZE, COPIER)                                                      \
    {                                                                                                 \
        typeof(ARRAY) __kh_array_ptr = ARRAY;                                                         \
        typeof(PTR) __kh_ptr = PTR;                                                                   \
        size_t __kh_size = SIZE;                                                                      \
                                                                                                      \
        /* If the reserved memory isn't enough, try to allocate more by *2 of the existing memory */  \
        if (kharray_size(__kh_array_ptr) + __kh_size > kharray_reserved(__kh_array_ptr)) {            \
            size_t __kh_exsize = kharray_size(__kh_array_ptr) ? kharray_size(__kh_array_ptr) * 2 : 2; \
            kharray_reserve(__kh_array_ptr, __kh_exsize < kharray_size(__kh_array_ptr) + __kh_size    \
                                                ? kharray_size(__kh_array_ptr) + __kh_size            \
                                                : __kh_exsize);                                       \
        }                                                                                             \
                                                                                                      \
        typeof(*__kh_array_ptr) __kh_array = *__kh_array_ptr;                                         \
                                                                                                      \
        /* Call the copy constructor of each element, unless it's NULL */                             \
        if (COPIER == NULL) {                                                                         \
            memcpy(__kh_array + kharray_size(__kh_array_ptr), __kh_ptr,                               \
                   _kharray_typeSize(__kh_array_ptr) * __kh_size);                                    \
        }                                                                                             \
        else {                                                                                        \
            for (size_t __kh_i = 0; __kh_i < __kh_size; __kh_i++) {                                   \
                __kh_array[kharray_size(__kh_array_ptr) + __kh_i] =                                   \
                    ((typeof (*__kh_array) (*)(typeof(__kh_array)))(COPIER))(__kh_ptr + __kh_i);      \
            }                                                                                         \
        }                                                                                             \
                                                                                                      \
        kharray_size(__kh_array_ptr) += __kh_size;                                                    \
    }

#define kharray_append(ARRAY, ITEM)                              \
    {                                                            \
        typeof(ARRAY) ___kh_array_ptr = ARRAY;                   \
        typeof(*___kh_array_ptr) ___kh_array = *___kh_array_ptr; \
        typeof(*___kh_array) ___kh_item = ITEM;                  \
                                                                 \
        kharray_memory(___kh_array_ptr, &___kh_item, 1, NULL);   \
    }

#define kharray_concatenate(ARRAY, OTHER, COPIER) \
    kharray_memory(ARRAY, *(OTHER), kharray_size(OTHER), COPIER)


#ifdef __cplusplus
}
#endif
