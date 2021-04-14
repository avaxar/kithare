/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
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

    for (const std::u32string& arg : args) {
        if (exargs.empty() && !arg.compare(0, 1, U"-")) {
            /* A Kithare command line option, handle all those here */
            if (arg == U"-h" || arg == U"--help") {
                kprintln(KH_HELP_STR);
                return 0;
            }

            if (arg == U"-v") {
                kprintln("Kithare " KH_VERSION_STR);
                return 0;
            }

            if (arg == U"-V") {
                kprintln("Kithare " KH_VERSION_STR);
                kprintln("OS: " KH_OS);
                kprintln("Compiler: " KH_COMPILER);
                kprintln("Compiled on " __DATE__ " at " __TIME__);
                return 0;
            }

            lex = arg == U"--lex";
        }
        else {
            exargs.emplace_back(arg);
        }
    }

    if (exargs.empty()) {
        kprintln(U"Path to file to be executed was not passed");
        return 1;
    }

    try {
        source = kh::readFile(exargs[0]);
        tokens = kh::lex(source);
        ast = kh::parse(tokens);
    }
    catch (const kh::FileError& exc) {
        std::string estr = "FileError in file ";
        estr += kh::encodeUtf8(exc.fname);
        perror(estr.c_str());
        return 1;
    }
    catch (const kh::UnicodeDecodeError& exc) {
        kprint(U"UnicodeDecodeError: ");
        kprint(exc.what);
        kprint(U" at ");
        kprintln((uint64_t)exc.index);
        return 1;
    }
    catch (const kh::LexException& exc) {
        size_t column, line;
        kh::strIndexPos(source, exc.index, column, line);

        kprint(U"SyntaxError: ");
        kprint(exc.what);
        kprint(U" at ");
        kprint((uint64_t)column);
        kprint(U", ");
        kprintln((uint64_t)line);
        return 1;
    }
    catch (const kh::ParseExceptions& exc) {
        kprintln(U"SyntaxError(s)");
        for (const kh::ParseException& ex : exc.exceptions) {
            size_t column, line;
            kh::strIndexPos(source, ex.index, column, line);

            kprint(ex.what);
            kprint(U" at ");
            kprint((uint64_t)column);
            kprint(U", ");
            kprintln((uint64_t)line);
        }
        return 1;
    }

    if (lex) {
        kprintln(U"Lexicated the source and generated tokens...");
        for (auto token : tokens)
            kprintln(token);
        kprintln(U"\n\nParsed the tokens and generated an AST tree...");
        kprintln(*ast);
    }

    delete ast;
    return 0;
}
