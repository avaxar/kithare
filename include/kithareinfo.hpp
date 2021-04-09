/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/kithareinfo.hpp
* Header that contains info about Kithare, like version, compiler used to compile
* kithare, etc etc
*/

#pragma once


#define KH_VERSION_MAJOR 0  // Can be from 0-9
#define KH_VERSION_MINOR 0  // Can be from 0-99
#define KH_VERSION_PATCH 1  // Can be from 0-99
#define KH_VERSION_TAGS "rc1"  // Can be a[1-6], b[1-6], rc[1-2]

#define KH_VERSION_STR "v0.0.1rc1"


#ifdef _MSC_VER
#define KH_COMPILER "MSVC"

#elif defined(__MINGW64__)
#define KH_COMPILER "MinGW-x64"

#elif defined(__MINGW32__)
#define KH_COMPILER "MinGW"

#elif defined(__GNUC__)
#define KH_COMPILER "GCC"

#endif

#ifdef _WIN32
#define KH_OS "Windows"
#endif

#ifdef __APPLE__
#define KH_OS "Mac OS"
#endif

#ifdef __linux__
#define KH_OS "Linux"
#endif

#ifndef KH_OS
#define KH_OS "Unknown"
#endif

#ifndef KH_COMPILER
#define KH_COMPILER "Unknown"
#endif
