/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Avaxar (AvaxarXapaxa)
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

#define KH_HELP_STR                                                                                 \
    U"This is the Kithare programming language.\n\n"                                                \
    U"Kithare is a general purpose statically-typed cross-platform interpreted Python and C like "  \
    U"programming language.\n"                                                                      \
    U"The source code for Kithare programming language is open sourced, and distributed under the " \
    U"MIT license.\n"                                                                               \
    U"Copyright(C) 2021 Avaxar(AvaxarXapaxa)\n\n"                                                   \
    U"The 'kcr' command has the following command signature:\n"                                     \
    U"kcr [-hvV] [--lex] filename\n\n"                                                              \
    U"'kcr -h' or 'kcr --help' displays this message.\n"                                            \
    U"'kcr -v' and 'kcr -V' display version info, the latter prints more info.\n"                   \
    U"'--lex' is an optional (temporary) argument, that prints the lexed and parsed output of a "   \
    U"Kithare source file.\n\n"                                                                     \
    U"'filename' is the path to the kithare source file, and any arguments after this will be "     \
    U"forwarded to the Kithare app\n"
