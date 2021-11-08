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

#include <kithare/lexer.h>


void cli() {
    char str[] = "12.3456e2";
    char* str_ptr = &str[0];
    printf("%f\n", kh_lexFloat(&str_ptr, 10));

    char utf8[] = "\x42\x63\xc3\xb6\xc3\xb3\xe4\x89\x82\xe6\x8d\xa3\xf0\x90\x80\x80";
    // int32_t utf32[] = {0x42, 0x63, 0xf6, 0xf3, 0x4242, 0x6363, 0x10000, 0x0};
    str_ptr = &utf8[0];

    int32_t c;
    while ((c = kh_lexUtf8(&str_ptr)) != 0) {
        printf("%d\n", c);
    }
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
