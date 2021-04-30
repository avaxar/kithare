/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/utility/cmd.cpp
 * Implements argument parsing for kithare commandline interface
 */

#include <cwctype>

#include "utility/cmd.hpp"
#include "utility/file.hpp"

#include "parser/json.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"

#include "info.hpp"


int kh::run(const std::vector<std::u32string>& args) {
    std::vector<std::u32string> excess_args;
    bool lex = false, ast = false, json = false, version = false, help = false, silent = false;

    for (const std::u32string& _arg : args) {
        std::u32string arg = _arg;
        bool is_flag = false;

        if (arg.size() > 1 && arg[0] == '-' && arg[1] == '-') {
            arg = std::u32string(arg.begin() + 2, arg.end());
            is_flag = true;
        }
        else if (arg.size() > 0 && (arg[0] == '-' || arg[0] == '/')) {
            arg = std::u32string(arg.begin() + 1, arg.end());
            is_flag = true;
        }
        else
            excess_args.push_back(arg);

        if (is_flag) {
            for (char32_t& chr : arg)
                chr = std::towlower(chr);

            if (arg == U"lex" || arg == U"l")
                lex = true;
            else if (arg == U"ast" || arg == U"a")
                ast = true;
            else if (arg == U"json" || arg == U"j")
                json = true;
            else if (arg == U"version" || arg == U"v")
                version = true;
            else if (arg == U"help" || arg == U"h")
                help = true;
            else if (arg == U"silent" || arg == U"s")
                silent = true;
            else {
                if (!silent) {
                    std::cerr << "Unrecognized flag argument: ";
                    kprintln(arg);
                }

                return 1;
            }
        }
        else
            excess_args.push_back(arg);
    }

    if ((help || args.empty()) && !silent)
        kprintln(KH_HELP_STR);

    if (version && !silent) {
        kprintln("Kithare " KH_VERSION_STR);
        kprintln("OS: " KH_OS);
        kprintln("Compiler: " KH_COMPILER);
        kprintln("Compiled on " __DATE__ " at " __TIME__);
    }

    if (excess_args.size() > 0) {
        std::u32string source;
        std::vector<kh::Token> tokens;
        kh::Ast* ast_tree = nullptr;

        try {
            source = kh::readFile(excess_args[0]);
        }
        catch (const kh::FileError& exc) {
            if (!silent) {
                std::cerr << "FileError: ";
                kprintln(exc.fname);
            }

            return 1;
        }
        catch (const kh::UnicodeDecodeError& exc) {
            if (!silent) {
                std::cerr << "UnicodeDecodeError: ";
                kprint(exc.what);
                std::cerr << " at index ";
                kprintln((uint64_t)exc.index);
            }

            return 1;
        }

        try {
            tokens = kh::lex(source, true);
        }
        catch (const kh::LexException& exc) {
            if (!silent) {
                std::cerr << "LexException: ";
                kprint(exc.what);
                std::cerr << " at index ";
                kprint((uint64_t)exc.index);

                size_t column, line;
                kh::strIndexPos(source, exc.index, column, line);

                std::cerr << " of column ";
                kprint((uint64_t)column);
                std::cerr << " line ";
                kprintln((uint64_t)line);
            }

            return 1;
        }

        if (lex && !ast) {
            if (silent)
                return 0;

            if (json)
                kprint(kh::json(tokens));
            else {
                for (const kh::Token& token : tokens) {
                    std::cout << token.index << ':' << token.length << ' ';
                    kprintln(token);
                }
            }

            return 0;
        }

        try {
            ast_tree = kh::parse(tokens);
        }
        catch (const kh::ParseExceptions& exc) {
            for (const kh::ParseException& ex : exc.exceptions) {
                std::cerr << "ParseException: ";
                kprint(ex.what);
                std::cerr << " at index ";
                kprint((uint64_t)ex.index);

                size_t column, line;
                kh::strIndexPos(source, ex.index, column, line);

                std::cerr << " of column ";
                kprint((uint64_t)column);
                std::cerr << " line ";
                kprintln((uint64_t)line);
            }

            return 1;
        }

        if (ast) {
            if (silent)
                return 0;

            if (lex) {
                for (const kh::Token& token : tokens) {
                    std::cout << token.index << ':' << token.length << ' ';
                    kprintln(token);
                }

                std::cout << '\n';
            }

            kprintln(*ast_tree);

            return 0;
        }
    }

    return 0;
}
