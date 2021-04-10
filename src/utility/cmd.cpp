/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/utility/cmd.cpp
* Implements argument parsing for kithare commandline interface
*/

#include "utility/cmd.hpp"


int kh::run(const std::vector<std::u32string>& args) {
    std::vector<std::u32string> exargs;
    std::u32string source;
    std::vector<kh::Token> tokens;
    kh::Ast* ast;
    bool lex = false;

    for (const std::u32string& arg: args) {
        if (exargs.empty() && !arg.compare(0, 1, U"-")) {
            // A kithare command line option, handle all those here
            if (arg == U"-v") {
                khPrintln("Kithare " KH_VERSION_STR);
                return 0;
            }

            if (arg == U"-V") {
                khPrintln("Kithare " KH_VERSION_STR);
                khPrintln("OS: " KH_OS);
                khPrintln("Compiler: " KH_COMPILER);
                khPrintln("Compiled on " __DATE__ " at " __TIME__);
                return 0;
            }

            lex = arg == U"--lex";
        }
        else {
            exargs.emplace_back(arg);
        }

    }

    if (exargs.empty()) {
        khPrintln(U"Path to file to be executed was not passed");
        return 1;
    }

    try {
        source = kh::readFile(exargs[0]);
        tokens = kh::lex(source);
        ast = kh::parse(tokens);
    }
    catch (const kh::FileNotFound& exc) {
        khPrint(U"FileNotFoundError: ");
        khPrintln(exc.fname);
        return 1;
    }
    catch (const kh::UnicodeDecodeError& exc) {
        khPrint(exc.what);
        khPrint(U" at ");
        khPrintln((uint64_t)exc.index);
        return 1;
    }
    catch (const kh::LexException& exc) {
        khPrint(exc.what);
        khPrint(U" at ");
        khPrintln((uint64_t)exc.index);
        return 1;
    }
    catch (const kh::ParseExceptions& exc) {
        for (const kh::ParseException& ex : exc.exceptions) {
            size_t column, line;
            kh::strIndexPos(source, ex.index, column, line);

            khPrint(ex.what);
            khPrint(U" at ");
            khPrint((uint64_t)column);
            khPrint(U", ");
            khPrintln((uint64_t)line);
        }
        return 1;
    }

    if (lex) {
        khPrintln(U"Lexicated the source and generated tokens...");
        for (auto token : tokens)
            khPrintln(token);
        khPrintln(U"\n\nParsed the tokens and generated an AST tree...");
        khPrintln(*ast);
    }

    delete ast;
    return 0;
}
