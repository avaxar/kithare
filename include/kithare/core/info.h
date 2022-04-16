/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif


#define kh_VERSION_MAJOR 0    // Can be from 0-9.
#define kh_VERSION_MINOR 0    // Can be from 0-99.
#define kh_VERSION_PATCH 1    // Can be from 0-99.
#define kh_VERSION_TAGS "rc1" // Can be a[1-6], b[1-6], rc[1-2].

// Manually keep this macro updated, and don't use nested macros here
#define kh_VERSION_STR "v0.0.1rc1"


// These are done as macros so that it could be used in preprocessor if statements.
#define khCOMPILER_EMSCRIPTEN 0
#define khCOMPILER_MINGW64 1
#define khCOMPILER_MINGW 2
#define khCOMPILER_GCC_ARM64 3
#define khCOMPILER_GCC_ARM 4
#define khCOMPILER_GCC_POWPC 5
#define khCOMPILER_GCC64 6
#define khCOMPILER_GCC 7
#define khCOMPILER_CLANG 8
#define khCOMPILER_UNKNOWN 9

#ifdef __EMSCRIPTEN__
#define kh_COMPILER khCOMPILER_EMSCRIPTEN
#define kh_COMPILER_STR "Emscripten"

#elif defined(__MINGW64__)
#define kh_COMPILER khCOMPILER_MINGW64
#define kh_COMPILER_STR "MinGW-x64"

#elif defined(__MINGW32__)
#define kh_COMPILER khCOMPILER_MINGW
#define kh_COMPILER_STR "MinGW"

#elif defined(__GNUC__) && __aarch64__
#define kh_COMPILER khCOMPILER_GCC_ARM64
#define kh_COMPILER_STR "GCC-ARM64"

#elif defined(__GNUC__) && __arm__
#define kh_COMPILER khCOMPILER_GCC_ARM
#define kh_COMPILER_STR "GCC-ARM"

#elif defined(__GNUC__) && __powerpc64__
#define kh_COMPILER khCOMPILER_GCC_POWPC
#define kh_COMPILER_STR "GCC-PowerPC"

#elif defined(__GNUC__) && __x86_64__
#define kh_COMPILER khCOMPILER_GCC64
#define kh_COMPILER_STR "GCC-x64"

#elif defined(__GNUC__) && __i386__
#define kh_COMPILER khCOMPILER_GCC
#define kh_COMPILER_STR "GCC"

#elif defined(__clang__)
#define kh_COMPILER khCOMPILER_CLANG
#define kh_COMPILER_STR "Clang"

#else
#define kh_COMPILER khCOMPILER_UNKNOWN
#define kh_COMPILER_STR "Unknown"
#endif


// These are also done as macros.
#define khPLATFORM_WEB 0
#define khPLATFORM_WINDOWS64 1
#define khPLATFORM_WINDOWS 2
#define khPLATFORM_IOS 3
#define khPLATFORM_MACOS 4
#define khPLATFORM_APPLE 5
#define khPLATFORM_ANDROID 6
#define khPLATFORM_LINUX 7
#define khPLATFORM_FREEBSD 8
#define khPLATFORM_UNIX 9
#define khPLATFORM_POSIX 10
#define khPLATFORM_UNKNOWN 11

#ifdef __EMSCRIPTEN__
#define kh_PLATFORM khPLATFORM_WEB
#define kh_PLATFORM_STR "Web"

#elif defined(_WIN64)
#define kh_PLATFORM khPLATFORM_WINDOWS64
#define kh_PLATFORM_STR "Windows (64 bit)"

#elif defined(_WIN32)
#define kh_PLATFORM khPLATFORM_WINDOWS
#define kh_PLATFORM_STR "Windows"

#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define kh_PLATFORM khPLATFORM_IOS
#define kh_PLATFORM_STR "iOS"

#elif TARGET_OS_MAC
#define kh_PLATFORM khPLATFORM_MACOS
#define kh_PLATFORM_STR "MacOS"

#else
#define kh_PLATFORM khPLATFORM_APPLE
#define kh_PLATFORM_STR "Unknown (Apple)"
#endif

#elif defined(__ANDROID__)
#define kh_PLATFORM khPLATFORM_ANDROID
#define kh_PLATFORM_STR "Android"

#elif defined(__linux__)
#define kh_PLATFORM khPLATFORM_LINUX
#define kh_PLATFORM_STR "Linux"

#elif defined(__FreeBSD__)
#define kh_PLATFORM khPLATFORM_FREEBSD
#define kh_PLATFORM_STR "FreeBSD"

#elif defined(__unix__)
#define kh_PLATFORM khPLATFORM_UNIX
#define kh_PLATFORM_STR "Unix"

#elif defined(__posix__)
#define kh_PLATFORM khPLATFORM_POSIX
#define kh_PLATFORM_STR "POSIX"

#else
#define kh_PLATFORM khPLATFORM_UNKNOWN
#define kh_PLATFORM_STR "Unknown"
#endif


#ifdef __cplusplus
}
#endif
