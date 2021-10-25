/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_OBJECT_H
#define kh_HG_OBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>


typedef struct {
    void (*copier)(void*);
    void (*destructor)(void*);
} khType;


typedef struct {
    khType* type;
    size_t references;
} khObject;


#ifdef __cplusplus
}
#endif
#endif
