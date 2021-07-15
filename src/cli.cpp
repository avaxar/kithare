/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifdef _WIN32
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <codecvt>
#endif

#include <chrono>
#include <clocale>
#include <iostream>
#include <vector>

#include <kithare/ansi.hpp>
#include <kithare/file.hpp>
#include <kithare/info.hpp>
#include <kithare/lexer.hpp>
#include <kithare/parser.hpp>
#include <kithare/string.hpp>
#include <kithare/test.hpp>
#include <kithare/utf8.hpp>


using namespace std;

#define CLI_ERROR_BEGIN() \
    if (!nocolor)         \
        cerr << KH_ANSI_FG_RED;

#define CLI_ERROR_END() \
    if (!nocolor)       \
        cerr << KH_ANSI_RESET;

vector<u32string> args;
bool nocolor = false, help = false, show_tokens = false, show_ast = false, show_timer = false,
     silent = false, test_mode = false, version = false;
vector<u32string> excess_args;

void handleArgs() {
    for (u32string& _arg : args) {
        u32string arg;

        /* Indicates that it is a flag argument (which starts with `-`. `--`, or `/`) */
        if (_arg.size() > 1 && _arg[0] == '-' && _arg[1] == '-') {
            arg = u32string(_arg.begin() + 2, _arg.end());
        }
        else if (_arg.size() > 0 && (_arg[0] == '-' || _arg[0] == '/')) {
            arg = u32string(_arg.begin() + 1, _arg.end());
        }
        /* Excess arguments */
        else {
            excess_args.push_back(_arg);
            continue;
        }

        /* Sets the booleans of the specified flags */
        if (arg == U"nocolor" || arg == U"nocolour" || arg == U"colorless" || arg == U"colourless") {
            nocolor = true;
        }
        else if (arg == U"h" || arg == U"help") {
            help = true;
        }
        else if (arg == U"tokens") {
            show_tokens = true;
        }
        else if (arg == U"ast") {
            show_ast = true;
        }
        else if (arg == U"t" || arg == U"timer") {
            show_timer = true;
        }
        else if (arg == U"s" || arg == U"silent") {
            silent = true;
        }
        else if (arg == U"test") {
            test_mode = true;
        }
        else if (arg == U"v" || arg == U"version") {
            version = true;
        }
        else {
            if (!silent) {
                CLI_ERROR_BEGIN();
                cout << "Unrecognized flag argument: " << kh::encodeUtf8(arg) << '\n';
                CLI_ERROR_END();
            }
            exit(1);
        }
    }
}

int execute() {
    int code = 0;

    if (version && !silent) {
        cout << "Kithare " KH_VERSION_STR << "\nOS: " KH_OS << "\nCompiler: " KH_COMPILER
             << "\nCompiled on " __DATE__ " at " __TIME__ << '\n';
    }

    if (help && !silent) {
        cout << "TODO\n";
    }

    /* Unittest */
    if (test_mode) {
        vector<string> errors;
        kh_test::utf8Test(errors);
        kh_test::lexerTest(errors);
        kh_test::parserTest(errors);

        if (!silent) {
            cout << "Unittest: " << errors.size() << " error(s)\n";

            CLI_ERROR_BEGIN();
            for (const string& error : errors) {
                cerr << error << '\n';
            }
            CLI_ERROR_END();
        }

        exit(errors.size());
    }

    /* Compilation */
    if (!excess_args.empty()) {
        u32string source;

        try {
            source = kh::readFile(excess_args[0]);
        }
        catch (kh::Exception& exc) {
            if (!silent) {
                CLI_ERROR_BEGIN();
                cerr << exc.format() << '\n';
                CLI_ERROR_END();
            }
            exit(1);
        }

        auto lex_start = chrono::high_resolution_clock::now();
        vector<kh::LexException> lex_exceptions;
        kh::LexerContext lexer_context{source, lex_exceptions};
        vector<kh::Token> tokens = kh::lex(lexer_context);
        auto lex_end = chrono::high_resolution_clock::now();
        chrono::duration<double> lex_elapsed = lex_end - lex_start;

        if (show_timer && !silent) {
            cout << "Finished lexing in " << lex_elapsed.count() << "s\n";
        }
        if (!lex_exceptions.empty()) {
            if (!silent) {
                CLI_ERROR_BEGIN();
                for (kh::LexException& exc : lex_exceptions) {
                    cerr << "LexException: " << exc.format() << '\n';
                }
                CLI_ERROR_END();
            }

            code += lex_exceptions.size();
        }
        if (show_tokens && !silent) {
            cout << "tokens:\n";
            for (kh::Token& token : tokens) {
                cout << '\t' << kh::encodeUtf8(kh::str(token, true)) << '\n';
            }
        }

        auto parse_start = chrono::high_resolution_clock::now();
        vector<kh::ParseException> parse_exceptions;
        kh::ParserContext parser_context{tokens, parse_exceptions};
        kh::AstModule ast = parseWhole(parser_context);
        auto parse_end = chrono::high_resolution_clock::now();
        chrono::duration<double> parse_elapsed = parse_end - parse_start;

        if (show_timer && !silent) {
            cout << "Finished parsing in " << parse_elapsed.count() << "s\n";
        }
        if (!parse_exceptions.empty()) {
            if (!silent) {
                CLI_ERROR_BEGIN();
                for (kh::ParseException& exc : parse_exceptions) {
                    cerr << "ParseException: " << exc.format() << '\n';
                }
                CLI_ERROR_END();
            }

            code += parse_exceptions.size();
        }
        if (show_ast && !code && !silent) {
            cout << kh::encodeUtf8(kh::str(ast)) << '\n';
        }
    }

    return code;
}

/* Entry point of the Kithare CLI program */
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    /* Sets the locale to using UTF-8 */
    setlocale(LC_ALL, "en_US.utf8");
#ifdef _WIN32
    /* Sets up wcout and wcin on Windows */
    locale utf8_locale(locale(), new codecvt_utf8_utf16<wchar_t>);
    wcout.imbue(utf8_locale);
    wcin.imbue(utf8_locale);

    /* A weird bug, I guess. It enables ANSI escape codes. */
    system(" ");
#endif

    args.reserve(argc - 1);

    /* Ignore the first argument */
    for (int arg = 1; arg < argc; arg++)
#ifdef _WIN32
        args.push_back(kh::str(wstring(argv[arg])));
#else
        args.push_back(kh::decodeUtf8(string(argv[arg])));
#endif

    handleArgs();
    return execute();
}
