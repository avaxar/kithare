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
#include <stdlib.h>


typedef struct {
    char* name;
    void (*deleter)(void**);
} khType;


typedef struct {
    khType* type;
    size_t ref_count;
} khObject;
typedef khObject* khObjectPtr;


static inline khObjectPtr khObject_new() {
    khObjectPtr object = (khObjectPtr)malloc(sizeof(khObject));
    object->type = NULL;
    object->ref_count = 1;
    return object;
}

static inline khObjectPtr khObject_copy(khObjectPtr* object) {
    (*object)->ref_count++;
    return *object;
}

static inline void khObject_delete(khObjectPtr* object) {
    (*object)->ref_count--;

    if ((*object)->ref_count == 0) {
        (*object)->type->deleter((void**)object);
        free(*object);
        *object = NULL;
    }
}


#ifdef __cplusplus
}
#endif
#endif
