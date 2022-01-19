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

#include <stdio.h>
#include <wchar.h>

#include "array.h"
#include "string.h"


static inline void kh_print(khArray(char32_t) * string) {
    khArray(uint8_t) buffer = kh_encodeUtf8(string);
    printf("%s", (char*)buffer);
    khArray_delete(&buffer);
}

static inline void kh_println(khArray(char32_t) * string) {
    khArray(uint8_t) buffer = kh_encodeUtf8(string);
    printf("%s\n", (char*)buffer);
    khArray_delete(&buffer);
}

#ifdef __cplusplus
}
#endif
