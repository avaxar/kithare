/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* src/main.cpp
* The main entry point of the Kithare programming language compiler and runtime.
*/

#include "utility/utils.hpp"
#include "parser/parse.hpp"


void executeArgs(const std::vector<kh::String>& args) {
    kh::String test_source = kh::toString(
        "int main() {                       \n"
        "    str var = \"\\wrong_escape\";  \n"
        "}                                  \n"
    );
    auto tokens = kh::lex(test_source, kh::toString("<TESTSOURCE>"));
    for (auto& token : tokens)
        std::wcout << token << '\n';
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

    std::vector<kh::String> args;

    #ifdef _WIN32
    /* Sets up std::wcout and std::wcin on Windows */
    std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    std::wcout.imbue(utf8);

    for (int arg = 0; arg < argc; arg++)
        args.emplace_back((uint32*)argv[arg]);
    #else
    for (int arg = 0; arg < argc; arg++)
        args.push_back(kh::decodeUtf8(argv[arg]));
    #endif

    try {
        executeArgs(args);
    }
    catch (const kh::LexException& exc) {
        std::wcout
            << "Caught a LexException!"
            << "\nWhat: " << exc.what
            << "\nFile name: " << exc.file_name
            << "\nIndex: " << exc.index
            << "\nAt: " << exc.character_line << ", " << exc.line << " '" << (wchar_t)exc.character << "\'\n";
        std::exit(1);
    }

    std::cin.get();
    return 0;
}
