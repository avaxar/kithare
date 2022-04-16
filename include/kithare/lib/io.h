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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "array.h"
#include "buffer.h"
#include "string.h"


static inline void kh_put(khstring* string, FILE* stream) {
    khbuffer buffer = kh_encodeUtf8(string);
    fputs((char*)buffer, stream);
    khbuffer_delete(&buffer);
}

static inline void kh_putln(khstring* string, FILE* stream) {
    khbuffer buffer = kh_encodeUtf8(string);
    fprintf(stream, "%s\n", (char*)buffer);
    khbuffer_delete(&buffer);
}

static inline khbuffer kh_readFile(khstring* file_name, bool* success) {
    khbuffer buffer = khbuffer_new("");

#ifdef _WIN32
    char16_t* char16_file_name = (char16_t*)calloc(khstring_size(file_name) + 1, sizeof(char16_t));
    for (size_t i = 0; i < khstring_size(file_name); i++) {
        char16_file_name[i] = (char16_t)(*file_name)[i];
    }

    FILE* file = _wfopen(char16_file_name, L"rb");
    free(char16_file_name);
#else
    khbuffer utf8_file_name = kh_encodeUtf8(file_name);
    FILE* file = fopen((char*)utf8_file_name, "rb");
    khbuffer_delete(&utf8_file_name);
#endif

    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        khbuffer_reserve(&buffer, ftell(file));
        rewind(file);

        int chr;
        while ((chr = fgetc(file)) != EOF) {
            khbuffer_append(&buffer, chr);
        }

        *success = true;
        fclose(file);
    }
    else {
        *success = false;
    }

    return buffer;
}


#ifdef __cplusplus
}
#endif
