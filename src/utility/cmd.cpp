/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/utility/cmd.cpp
* Implements argument parsing for kithare commandline interface
*/
#include "utility/cmd.hpp"
#include "kithareinfo.hpp"

std::u32string kh::readFile(std::u32string scriptname) {
    std::string u8source;
    std::ifstream fin(kh::encodeUtf8(scriptname), std::ios::in | std::ios::binary);
    char byte;

    if (fin.fail())
        throw kh::FileNotFound(scriptname);

    while (!fin.eof()) {
        fin.read(&byte, 1);
        u8source += byte;
    }

    return kh::decodeUtf8(u8source);
}

int kh::run(const std::vector<std::u32string>& args) {
    std::vector<std::u32string> exargs;
    std::u32string source;
    std::vector<kh::Token> tokens;
    kh::Ast* ast;
    bool lex = false;

    for (auto arg: args) {
        if (exargs.empty() && !arg.compare(0, 1, U"-")) {
            // A kithare command line option, handle all those here
            if (arg == U"-v") {
                println("Kithare " KH_VERSION_STR);
                return 0;
            }

            if (arg == U"-V") {
                println("Kithare " KH_VERSION_STR);
                println("OS: " KH_OS);
                println("Compiler: " KH_COMPILER);
                println("Compiled on " __DATE__ " at " __TIME__);
                return 0;
            }

            lex = arg == U"--lex";
        }
        else {
            exargs.emplace_back(arg);
        }

    }

    if (exargs.empty()) {
        println(U"Path to file to be executed was not passed");
        return 1;
    }

    try {
        source = kh::readFile(exargs[0]);
        tokens = kh::lex(source);
        ast = kh::parse(tokens);
    }
    catch (const kh::FileNotFound& exc) {
        print(U"FileNotFoundError: ");
        println(exc.fname);
        return 1;
    }
    catch (const kh::UnicodeDecodeError& exc) {
        print(exc.what);
        print(U" at ");
        println((uint64_t)exc.index);
        return 1;
    }
    catch (const kh::LexException& exc) {
        print(exc.what);
        print(U" at ");
        println((uint64_t)exc.index);
        return 1;
    }
    catch (const kh::ParseExceptions& exc) {
        for (const kh::ParseException& ex : exc.exceptions) {
            size_t column, line;
            kh::strIndexPos(source, ex.index, column, line);

            print(ex.what);
            print(U" at ");
            print((uint64_t)column);
            print(U", ");
            println((uint64_t)line);
        }
        return 1;
    }

    if (lex) {
        println(U"Lexicated the source and generated tokens...");
        for (auto token : tokens)
            println(token);
        println(U"\n\nParsed the tokens and generated an AST tree...");
        println(*ast);
    }

    delete ast;
    return 0;
}
