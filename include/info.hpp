/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * include/info.hpp
 * Header that contains info about the Kithare build. Such as version, compiler, and OS.
 */

#pragma once


#define KH_VERSION_MAJOR 0    // Can be from 0-9
#define KH_VERSION_MINOR 0    // Can be from 0-99
#define KH_VERSION_PATCH 1    // Can be from 0-99
#define KH_VERSION_TAGS "rc1" // Can be a[1-6], b[1-6], rc[1-2]

#define KH_VERSION_STR "v0.0.1rc1"

#ifdef _MSC_VER
#define KH_COMPILER "MSVC"
#elif __MINGW64__
#define KH_COMPILER "MinGW-x64"
#elif __MINGW32__
#define KH_COMPILER "MinGW"
#elif __GNUC__
#define KH_COMPILER "GCC"
#else
#define KH_COMPILER "Unknown"
#endif

#ifdef _WIN32
#define KH_OS "Windows"
#elif __APPLE__
#define KH_OS "Mac OS"
#elif __linux__
#define KH_OS "Linux"
#else
#define KH_OS "Unknown"
#endif

const char32_t KH_HELP_STR[] =
    U"This is the Kithare compiler-runtime executable (`kcr`).\n"
    U"The Kithare programming language with this `kcr` is distributed "
    U"under the MIT license. Meaning that you can use it for commercial and "
    U"non-commercial projects freely! A credit to Kithare would be nice.\n\n"
    U"`kcr -h` or `kcr --help` shows this help message!\n"
    U"`kcr -v` or `kcr --version` shows the version of this `kcr` build.\n"
    U"`kcr -l [-j] filename` or `kcr --lex [--json] filename` shows the "
    U"lexicated output of the source file, with an option to print it as JSON.\n"
    U"`kcr -a filename` or `kcr --ast filename` shows the generated AST tree from the "
    U"parsed source file.\n"
    U"`kcr -s [...]` silents any print output or error messages.\n";
