/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <kithare/array.h>
#include <kithare/ast.h>
#include <kithare/io.h>
#include <kithare/lexer.h>
#include <kithare/string.h>


static void cli(khArray(khArray(char32_t)) args) {
    khArray(char32_t) str = kh_string(U"\"\\x69\\x99\\u1020\\U99999999dkasdkas hello\\0\"");
    char32_t* str_ptr = str;

    khArray(khLexError) errors = khArray_new(khLexError, khLexError_delete);
    khToken token = kh_lex(&str_ptr, &errors);

    khArray(char32_t) string = khToken_string(&token);
    khPrintln(&string);
    khArray_delete(&string);

    printf("BREAKPOINT\n");

    khToken_delete(&token);
    khArray_delete(&errors);
    khArray_delete(&str);
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

    khArray(khArray(char32_t)) args = khArray_new(khArray(char32_t), NULL);
    khArray_reserve(&args, argc);

    for (int i = 0; i < argc; i++) {
#ifdef _WIN32
        khArray_append(&args, khArray_new(char32_t, NULL));

        size_t length = wcslen(argv[i]);
        khArray_reserve(&args[i], length);
        for (wchar_t* wchr = argv[i]; wchr < argv[i] + length; wchr++) {
            khArray_append(&args[i], *wchr);
        }
#else
        khArray(uint8_t) arg = khArray_new(uint8_t, NULL);
        khArray_append(&args, kh_decodeUtf8(&arg));
        khArray_delete(&arg);
#endif
    }

    cli(args);

    // Have to manually destruct the items
    for (int i = 0; i < argc; i++) {
        khArray_delete(&args[i]);
    }
    khArray_delete(&args);

    return 0;
}
