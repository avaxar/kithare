/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* src/main.cpp
* The main entry point of the Kithare programming language compiler and runtime.
*/

#include <clocale>
#include <ctime>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <codecvt>
#endif

#include "utility/string.hpp"
#include "parser/lexer.hpp"


void run(const std::vector<std::u32string>& args) {
    kh::Token te(U"Cough", 0, 0);
    println(te);
}

#undef main
#undef wmain
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    srand((unsigned int)time(NULL));
    std::setlocale(LC_ALL, "en_US.utf8");

    std::vector<std::u32string> args;

    #ifdef _WIN32
    /* Sets up std::wcout and std::wcin on Windows */
    std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    std::wcout.imbue(utf8);

    for (int arg = 0; arg < argc; arg++)
        args.push_back(kh::repr(std::wstring(argv[arg])));
    #else
    for (int arg = 0; arg < argc; arg++)
        args.push_back(kh::decodeUtf8(std::string(argv[arg])));
    #endif

    run(args);

    return 0;
}
