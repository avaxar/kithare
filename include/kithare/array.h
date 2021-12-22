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


#define khArray(TYPE) TYPE*
#define khArray_header(ARRAY) (((_khArrayHeader*)(*ARRAY))[-1])
#define khArray_typeSize(ARRAY) (khArray_header(ARRAY).type_size)
#define khArray_size(ARRAY) (khArray_header(ARRAY).size)
#define khArray_reserved(ARRAY) (khArray_header(ARRAY).reserved)
#define khArray_deleter(ARRAY) (khArray_header(ARRAY).deleter)

void* _khArray_new(size_t type_size, void (*deleter)(void*));
#define khArray_new(TYPE, DELETER) (TYPE*)_khArray_new(sizeof(TYPE), (void (*)(void*))(DELETER))

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

void _khArray_delete(void** array);
#define khArray_delete(ARRAY) _khArray_delete(_kh_verifyPtrToPtr(ARRAY))
#define khArray_arrayDeleter(TYPE) ((void (*)(TYPE**))_khArray_delete)

void _khArray_reserve(void** array, size_t size);
#define khArray_reserve(ARRAY, SIZE) _khArray_reserve(_kh_verifyPtrToPtr(ARRAY), SIZE)

void _khArray_fit(void** array);
#define khArray_fit(ARRAY) _khArray_fit(_kh_verifyPtrToPtr(ARRAY));

void _khArray_pop(void** array, size_t items);
#define khArray_pop(ARRAY, ITEMS) _khArray_pop(_kh_verifyPtrToPtr(ARRAY), ITEMS)

void _khArray_reverse(void** array);
#define khArray_reverse(ARRAY) _khArray_reverse(_kh_verifyPtrToPtr(ARRAY))

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
