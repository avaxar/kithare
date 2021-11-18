/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <kithare/io.h>
#include <kithare/lexer.h>
#include <kithare/string.h>


void cli(void) {
    khArray_char str = kh_string("\"Super Idol的笑容\"");
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

    cli();
    return 0;
}
