/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * src/utility/cmd.cpp
 * Implements argument parsing for kithare commandline interface
 */

#include <cwctype>
#include <memory>

#include "utility/cmd.hpp"
#include "utility/file.hpp"

#include "parser/ast.hpp"
#include "parser/json.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "parser/repr.hpp"

#include "info.hpp"


int kh::run(const std::vector<std::u32string>& args) {
    std::vector<std::u32string> excess_args;
    bool lex = false, ast = false, json = false, version = false, help = false, silent = false;

    for (const std::u32string& _arg : args) {
        std::u32string arg = _arg;
        bool is_flag = false;

        /* Indicates that it is a flag argument (which starts with `-`. `--`, or `/`) */
        if (arg.size() > 1 && arg[0] == '-' && arg[1] == '-') {
            arg = std::u32string(arg.begin() + 2, arg.end());
            is_flag = true;
        }
        else if (arg.size() > 0 && (arg[0] == '-' || arg[0] == '/')) {
            arg = std::u32string(arg.begin() + 1, arg.end());
            is_flag = true;
        }
        /* Excess arguments */
        else
            excess_args.push_back(arg);

        if (is_flag) {
            for (char32_t& chr : arg)
                chr = std::towlower(chr);

            /* Marks up the booleans of the indicated flag */
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

    bool error = false;

    /* Opening dictionary bracket of the JSON plus the argument(s) data */
    if (json) {
        std::cout << "{\"arguments\":[";

        for (const std::u32string& arg : args) {
            kprint(kh::quote(arg));
            if (&arg != &args.back())
                std::cout << ',';
        }

        std::cout << "]";
    }

    /* `kcr -h` */
    if ((help || args.empty()) && !silent)
        if (json) {
            std::cout << ",\"help\":";
            kprint(kh::quote(KH_HELP_STR));
        }
        else
            kprintln(KH_HELP_STR);

    /* `kcr -v` */
    if (version && !silent) {
        if (json) {
            std::cout << ",\"version\":\"" KH_VERSION_STR "\"";
            std::cout << ",\"os\":\"" KH_OS "\"";
            std::cout << ",\"compiler\":\"" KH_COMPILER "\"";
            std::cout << ",\"compile_date\":\"" __DATE__ "\"";
            std::cout << ",\"compile_time\":\"" __TIME__ "\"";
        }
        else {
            kprintln("Kithare " KH_VERSION_STR);
            kprintln("OS: " KH_OS);
            kprintln("Compiler: " KH_COMPILER);
            kprintln("Compiled on " __DATE__ " at " __TIME__);
        }
    }

    /* `kcr [...] some_file_name.kh` */
    if (excess_args.size() > 0) {
        std::u32string source;
        std::vector<kh::Token> tokens;
        std::shared_ptr<kh::Ast> ast_tree;

        /* Tries to open the file and read its content in UTF-8 */
        try {
            source = kh::readFile(excess_args[0]);
        }
        /* If the file was not found or something */
        catch (const kh::FileError& exc) {
            if (!silent) {
                if (json) {
                    std::cout << ",\"file_error\":";
                    kprint(kh::quote(exc.fname));
                    std::cout << "}";
                }
                else {
                    std::cerr << "FileError: ";
                    kprintln(exc.fname);
                }
            }

            return 1;
        }
        /* Failed to decode with a UTF-8 encoding */
        catch (const kh::UnicodeDecodeError& exc) {
            if (!silent) {
                if (json) {
                    std::cout << ",\"unicode_decode_error\":[";
                    kprint(kh::quote(exc.what));
                    std::cout << "," << exc.index << "]}";
                }
                else {
                    std::cerr << "UnicodeDecodeError: ";
                    kprint(exc.what);
                    std::cerr << " at index " << exc.index;
                }
            }

            return 1;
        }

        /* Lexicates the source */
        kh::LexExceptions lex_exceptions({});
        tokens = kh::lex(source, true, &lex_exceptions);

        /* Prints the tokens */
        if (lex && !silent) {
            if (json) {
                std::cout << ",\"tokens\":";
                kprint(kh::json(tokens));
            }
            else {
                for (const kh::Token& token : tokens) {
                    std::cout << token.index << ':' << token.length << ' ';
                    kprintln(token);
                }

                if (ast)
                    std::cout << '\n';
            }
        }

        /* If there's any exceptions from the lexicating process */
        if (!lex_exceptions.exceptions.empty()) {
            if (!silent) {
                if (json)
                    std::cout << ",\"lex_exceptions\":[";

                for (const kh::LexException& exc : lex_exceptions.exceptions) {
                    if (json) {
                        std::cout << "[";
                        kprint(kh::quote(exc.what));
                        std::cout << "," << exc.index << "]";

                        if (&exc != &lex_exceptions.exceptions.back())
                            std::cout << ",";
                    }
                    else {
                        std::cerr << "LexException: ";
                        kprint(exc.what);
                        std::cerr << " at index " << exc.index;

                        size_t column, line;
                        kh::strIndexPos(source, exc.index, column, line);

                        std::cerr << " of column " << column << " line " << line << '\n';
                    }
                }

                if (json)
                    std::cout << "]";
            }

            error = true;
        }

        if (!lex || (lex && ast)) {
            /* Tries to parse the tokens */
            try {
                ast_tree.reset(kh::parse(tokens));
            }
            catch (const kh::ParseExceptions& exc) {
                if (!silent) {
                    if (json)
                        std::cout << ",\"parse_exceptions\":[";

                    for (const kh::ParseException& ex : exc.exceptions) {
                        if (json) {
                            std::cout << "[";
                            kprint(kh::quote(ex.what));
                            std::cout << "," << ex.index << "]";

                            if (&ex != &exc.exceptions.back())
                                std::cout << ",";
                        }
                        else {
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
                    }

                    if (json)
                        std::cout << "]";
                }

                error = true;
            }

            /* Prints the AST tree */
            if (ast && ast_tree) {
                if (silent)
                    return 0;

                if (json) {
                    /* placeholder */
                    std::cout << ",\"ast\":null";
                }
                else
                    kprintln(*ast_tree);
            }
        }
    }

    if (json)
        std::cout << '}';

    return error;
}
