/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* src/main.cpp
* The main entry point of the Kithare programming language compiler and runtime.
*/

#include <iostream>
#include <vector>

#include "utility/string.hpp"
#include "parser/lexer.hpp"


void run(const std::vector<std::u32string>& args) {
    std::u32string source =
        U"0 1 2 8 9  "  /* Single digit decimal integers */
        U"00 10 29U  "  /* Multi-digit + Unsigned */
        U"0.1 0.2    "  /* Floating point */
        U"11.1 .123  "  /* Several other cases */
        U"0xFFF 0x1  "  /* Hexadecimal */
        U"0o77 0o11  "  /* Octal */
        U"0b111 0b01 "  /* Binary */
        U"4i 2i 5.6i "; /* Imaginary */
    
    auto tokens = kh::lex(source);
    for (auto token : tokens) {
        println(token);
    }
}

#undef main
#undef wmain
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    std::vector<std::u32string> args;
    
    #ifdef _WIN32
    for (int arg = 0; arg < argc; arg++)
        args.push_back(kh::repr(std::wstring(argv[arg])));
    #else
    for (int arg = 0; arg < argc; arg++)
        args.push_back(kh::decodeUtf8(std::string(argv[arg])));
    #endif

    run(args);

    return 0;
}
