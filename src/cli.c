/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
 */

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <kithare/core/ast.h>
#include <kithare/core/info.h>
#include <kithare/core/lexer.h>
#include <kithare/core/parser.h>

#include <kithare/lib/ansi.h>
#include <kithare/lib/array.h>
#include <kithare/lib/buffer.h>
#include <kithare/lib/io.h>
#include <kithare/lib/string.h>


static int argi = 1;
static kharray(khstring) args = NULL;


static int help(void) {
    puts(kh_ANSI_BOLD "Kithare programming language Compiler and Runtime (kcr) " kh_VERSION_STR);
    puts(kh_ANSI_RESET "Copyright (C) 2022 Kithare Organization at " kh_ANSI_FG_CYAN kh_ANSI_UNDERLINE
                       "https://www.kithare.de" kh_ANSI_RESET "; distributed under the "
                       "MIT license.\n");

    puts("    " kh_ANSI_BOLD "kcr [help]" kh_ANSI_RESET " : shows this help page.");
    puts("    " kh_ANSI_BOLD "kcr version" kh_ANSI_RESET " : shows the used Kithare version.");
    puts("    " kh_ANSI_BOLD "kcr [run] <file.kr> [... arguments]" kh_ANSI_RESET
         " : builds and runs source file.");
    puts("    " kh_ANSI_BOLD "kcr debug <file.kr> [... arguments]" kh_ANSI_RESET
         " : builds and runs source file on debug mode for debugging.");
    puts("    " kh_ANSI_BOLD "kcr build <file.kr> [executable.exe]" kh_ANSI_RESET
         " : builds source file.");
    puts("    " kh_ANSI_BOLD "kcr lexicate <file.kr>" kh_ANSI_RESET
         " : lexicates source file into tokens.");
    puts("    " kh_ANSI_BOLD "kcr parse <file.kr>" kh_ANSI_RESET
         " : parses source file into an AST tree.");
    puts("    " kh_ANSI_BOLD "kcr semantic <file.kr>" kh_ANSI_RESET
         " : semantically analyze source file into a semantic tree.");

    puts("\n`" kh_ANSI_BOLD "[...]" kh_ANSI_RESET "` arguments are optional. `" kh_ANSI_BOLD
         "<...>" kh_ANSI_RESET "` arguments are required input arguments.");

    return 0;
}

static int version(void) {
    puts(kh_VERSION_STR);
    return 0;
}

static int run(void) {
    fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "unimplemented command: " kh_ANSI_RESET "run\n", stderr);
    return 1;
}

static int debug(void) {
    fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "unimplemented command: " kh_ANSI_RESET "debug\n", stderr);
    return 1;
}

static int build(void) {
    fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "unimplemented command: " kh_ANSI_RESET "build\n", stderr);
    return 1;
}

static int lexicate(void) {
    if (argi >= kharray_size(&args)) {
        fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "missing required argument: " kh_ANSI_RESET "file\n", stderr);
        return 1;
    }
    else if (kharray_size(&args) > 3) {
        fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "too many arguments; " kh_ANSI_RESET kh_ANSI_BOLD
                                          "lexicate" kh_ANSI_RESET
                                          " only requires exactly 1 argument.\n",
              stderr);
        return 1;
    }

    // Read file provided by argument
    bool file_exists;
    khbuffer content_buf = kh_readFile(&args[argi], &file_exists);
    khstring content = kh_decodeUtf8(&content_buf);
    khbuffer_delete(&content_buf);
    argi++;

    if (!file_exists) {
        fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "file not found: " kh_ANSI_RESET, stderr);
        kh_putln(&args[argi - 1], stderr);

        khstring_delete(&content);
        return 1;
    }

    puts("{");
    puts("\"tokens\": [");

    // Print tokens
    kharray(khToken) tokens = kh_lexicate(&content);
    for (size_t i = 0; i < kharray_size(&tokens); i++) {
        khstring token_str = khToken_string(&(tokens[i]), content);
        kh_put(&token_str, stdout);
        khstring_delete(&token_str);

        if (i < kharray_size(&tokens) - 1) {
            printf(",\n");
        }
        else {
            printf("\n");
        }
    }

    puts("],");
    puts("\"errors\": [");

    // Print errors
    size_t errors = kh_hasErrors();
    for (size_t i = 0; i < errors; i++) {
        khError* error = &(*kh_getErrors())[i];
        khstring message = khstring_quote(&error->message);

        printf("{\"index\": %lu, \"message\": ", (unsigned long)((char32_t*)error->data - content));
        kh_put(&message, stdout);
        printf("}");

        khstring_delete(&message);

        if (i < errors - 1) {
            printf(",\n");
        }
        else {
            printf("\n");
        }
    }

    kh_flushErrors();
    puts("]");
    puts("}");

    khstring_delete(&content);
    kharray_delete(&tokens);

    return errors;
}

static int parse(void) {
    if (argi >= kharray_size(&args)) {
        fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "missing required argument: " kh_ANSI_RESET "file\n", stderr);
        return 1;
    }
    else if (kharray_size(&args) > 3) {
        fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "too many arguments; " kh_ANSI_RESET kh_ANSI_BOLD
                                          "parse" kh_ANSI_RESET " only requires exactly 1 argument.\n",
              stderr);
        return 1;
    }

    // Read file provided by argument
    bool file_exists;
    khbuffer content_buf = kh_readFile(&args[argi], &file_exists);
    khstring content = kh_decodeUtf8(&content_buf);
    khbuffer_delete(&content_buf);
    argi++;

    if (!file_exists) {
        fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "file not found: " kh_ANSI_RESET, stderr);
        kh_putln(&args[argi - 1], stderr);

        khstring_delete(&content);
        return 1;
    }

    puts("{");
    puts("\"ast\": [");

    // Print statements
    kharray(khAstStatement) ast = kh_parse(&content);
    for (size_t i = 0; i < kharray_size(&ast); i++) {
        khstring statement_str = khAstStatement_string(&ast[i], content);
        kh_put(&statement_str, stdout);
        khstring_delete(&statement_str);

        if (i < kharray_size(&ast) - 1) {
            printf(",\n");
        }
        else {
            printf("\n");
        }
    }

    puts("],");
    puts("\"errors\": [");

    // Print errors
    size_t errors = kh_hasErrors();
    for (size_t i = 0; i < errors; i++) {
        khError* error = &(*kh_getErrors())[i];
        khstring message = khstring_quote(&error->message);

        printf("{\"index\": %lu, \"message\": ", (unsigned long)((char32_t*)error->data - content));
        kh_put(&message, stdout);
        printf("}");

        khstring_delete(&message);

        if (i < errors - 1) {
            printf(",\n");
        }
        else {
            printf("\n");
        }
    }

    kh_flushErrors();
    puts("]");
    puts("}");

    khstring_delete(&content);
    kharray_delete(&ast);

    return errors;
}

static int semantic(void) {
    fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "unimplemented command: " kh_ANSI_RESET "semantic\n", stderr);
    return 1;
}


// Entry point of the Kithare CLI program
#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    setlocale(LC_ALL, "en_US.utf8");
    setlocale(LC_TIME, "en_GB.utf8");

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IOFBF, 256);
    system(" ");
#endif

    args = kharray_new(khstring, khstring_delete);

    kharray_reserve(&args, argc);
    for (int i = 0; i < argc; i++) {
#ifdef _WIN32
        kharray_append(&args, khstring_new(U""));

        size_t length = wcslen(argv[i]);
        kharray_reserve(&args[i], length);
        for (wchar_t* wchr = argv[i]; wchr < argv[i] + length; wchr++) {
            kharray_append(&args[i], *wchr);
        }
#else
        khbuffer arg = khbuffer_new(argv[i]);
        kharray_append(&args, kh_decodeUtf8(&arg));
        khbuffer_delete(&arg);
#endif
    }

    int code = 0;

    if (kharray_size(&args) == 1 || khstring_equalCstring(&args[1], U"help")) {
        argi++;
        code = help();
    }
    else if (khstring_equalCstring(&args[1], U"version")) {
        argi++;
        code = version();
    }
    else if (khstring_equalCstring(&args[1], U"run")) {
        argi++;
        code = run();
    }
    else if (khstring_equalCstring(&args[1], U"debug")) {
        argi++;
        code = debug();
    }
    else if (khstring_equalCstring(&args[1], U"build")) {
        argi++;
        code = build();
    }
    else if (khstring_equalCstring(&args[1], U"lexicate")) {
        argi++;
        code = lexicate();
    }
    else if (khstring_equalCstring(&args[1], U"parse")) {
        argi++;
        code = parse();
    }
    else if (khstring_equalCstring(&args[1], U"semantic")) {
        argi++;
        code = semantic();
    }
    else if (khstring_endswithCstring(&args[1], U".kh")) {
        code = run();
    }
    else {
        fputs(kh_ANSI_BOLD kh_ANSI_FG_RED "unknown command: " kh_ANSI_RESET, stderr);
        kh_putln(&args[argi], stderr);
        fputs(kh_ANSI_FG_YELLOW "Use `" kh_ANSI_RESET kh_ANSI_BOLD
                                "kcr help" kh_ANSI_RESET kh_ANSI_FG_YELLOW
                                "` for the available command arguments.\n" kh_ANSI_RESET,
              stderr);
        code = 1;
    }

    kharray_delete(&args);
    return code;
}
