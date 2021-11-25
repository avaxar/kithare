/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <kithare/io.h>
#include <kithare/lexer.h>
#include <kithare/string.h>


#define khArray_TYPE khArray_char
#define khArray_COPIER khArray_char_copy
#define khArray_DELETER khArray_char_delete
#include <kithare/t_array.h>


void cli(khArray_khArray_char* args) {
    khArray_char str = kh_string("->");
    uint32_t* str_ptr = str.array;

    khArray_khLexError errors = khArray_khLexError_new();
    khToken token = kh_lex(&str_ptr, &errors);

    khArray_char string = khToken_string(&token);
    khPrintln(&string);
    khArray_char_delete(&string);

    printf("BREAKPOINT\n");

    khToken_delete(&token);
    khArray_khLexError_delete(&errors);
    khArray_char_delete(&str);
}


// Entry point of the Kithare CLI program
#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    setlocale(LC_ALL, "en_US.utf8");
    setlocale(LC_TIME, "en_GB.utf8");

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IOFBF, 256);
    system(" ");
#endif

    khArray_khArray_char args = khArray_khArray_char_new();
    khArray_khArray_char_reserve(&args, argc);

    for (int i = 0; i < argc; i++) {
#ifdef _WIN32
        khArray_khArray_char_push(&args, khArray_char_new());

        size_t length = wcslen(argv[i]);
        khArray_char_reserve(&args.array[i], length);
        for (wchar_t* wchr = argv[i]; wchr < argv[i] + length; wchr++) {
            khArray_char_push(&args.array[i], *wchr);
        }
#else
        // No need to delete
        khArray_byte arg = (khArray_byte){
            .array = (uint8_t*)argv[i], .size = strlen(argv[i]), .reserved = strlen(argv[i])};
        khArray_khArray_char_push(&args, kh_decodeUtf8(&arg));
#endif
    }

    cli(&args);

    khArray_khArray_char_delete(&args);
    return 0;
}
