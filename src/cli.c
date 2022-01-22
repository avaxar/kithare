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

#include <kithare/core/ast.h>
#include <kithare/core/lexer.h>
#include <kithare/core/parser.h>
#include <kithare/lib/array.h>
#include <kithare/lib/buffer.h>
#include <kithare/lib/io.h>
#include <kithare/lib/string.h>


static void cli(kharray(khstring) args) {
    khstring str = khstring_new(U"def main() {\n"
                                U"    callback := def(a: int, b: int) -> int {\n"
                                U"        return a + b\n"
                                U"    }\n"
                                U"    std.println(callback(62, 42))\n"
                                U"}\n");
    char32_t* str_ptr = str;

    khAst ast = kh_parse(&str_ptr);

    khstring string = khAst_string(&ast);
    kh_println(&string);
    khstring_delete(&string);

    khAst_delete(&ast);
    khstring_delete(&str);
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

    kharray(khstring) args = kharray_new(khstring, khstring_delete);
    kharray_reserve(&args, argc);

    for (int i = 0; i < argc; i++) {
#ifdef _WIN32
        kharray_append(&args, khstring_new(U""));

        size_t length = wcslen(argv[i]);
        kharray_reserve(&args[i], length);
        for (wchar_t* wchr = argv[i]; wchr < argv[i] + length; wchr++) {
            kharray_append(&args[i], *wchr);
        }
#else
        khbuffer arg = khbuffer_new(argv[i]);
        kharray_append(&args, kh_decodeUtf8(&arg));
        khbuffer_delete(&arg);
#endif
    }

    cli(args);

    kharray_delete(&args);

    return 0;
}
