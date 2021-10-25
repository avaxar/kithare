/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifdef khMap_TYPE

/*
#include <stddef.h>
#include <stdint.h>

#include <kithare/macros.h>


#define khMap_NAME kh_CONCAT(khMap_, khMap_TYPE)
#define khMap_CONCAT(X) kh_CONCAT(khMap_NAME, _##X)


typedef struct {
    size_t key_size;
    void* nodes;
} khMap_NAME;
*/


#undef khMap_TYPE
#else
/* Ignore this error from your IDE while editing this header */
#error "`khMap_TYPE` hasn't been defined"
#endif
#ifdef __cplusplus
}
#endif
