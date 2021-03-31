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
#include "parser/parser.hpp"


void run(const std::vector<std::u32string>& args) {
    std::u32string source =
        U"import stuff;                             \n"
        U"import stuff.with.path;                   \n"
        U"import stuff.with.path as something;      \n"
        U"include this.too;                         \n"

        U"def main() {                              \n"
        U"    std.println(\"Hello, world!\");       \n"
        U"}                                         \n";

    try {
        println(U"Lexicating the source and generate tokens...");
        std::vector<kh::Token> tokens = kh::lex(source);
        for (auto token : tokens)
            println(token);

        println(U"\n\nParsing the tokens and generate an AST tree...");
        // kh::Ast* ast = kh::parse(tokens);
        // println(*ast);
        // delete ast;
    }
    catch (const kh::LexException& exc) {
        println(exc.what);
        std::exit(1);
    }
    catch (const kh::ParseExceptions& exc) {
        for (const kh::ParseException& ex : exc.exceptions)
            println(ex.what);
        std::exit(1);
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
