/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_INFO_H
#define kh_HG_INFO_H
#ifdef __cplusplus
extern "C" {
#endif


#define kh_VERSION_MAJOR 0    /* Can be from 0-9                */
#define kh_VERSION_MINOR 0    /* Can be from 0-99               */
#define kh_VERSION_PATCH 1    /* Can be from 0-99               */
#define kh_VERSION_TAGS "rc1" /* Can be a[1-6], b[1-6], rc[1-2] */

#define kh_VERSION_STR "v0.0.1rc1"


#ifdef __EMSCRIPTEN__
#define kh_COMPILER "Emscripten"
#elif defined(__MINGW64__)
#define kh_COMPILER "MinGW-x64"
#elif defined(__MINGW32__)
#define kh_COMPILER "MinGW"
#elif defined(__GNUC__) && __aarch64__
#define kh_COMPILER "GCC-ARM64"
#elif defined(__GNUC__) && __arm__
#define kh_COMPILER "GCC-ARM"
#elif defined(__GNUC__) && __powerpc64__
#define kh_COMPILER "GCC-PowerPC"
#elif defined(__GNUC__) && __x86_64__
#define kh_COMPILER "GCC-x64"
#elif defined(__GNUC__) && __i386__
#define kh_COMPILER "GCC"
#elif defined(__clang__)
#define kh_COMPILER "Clang"
#else
#define kh_COMPILER "unknown"
#endif


#ifdef __EMSCRIPTEN__
#define kh_PLATFORM "Web"
#elif defined(_WIN64)
#define kh_PLATFORM "Windows (64 bit)"
#elif defined(_WIN32)
#define kh_PLATFORM "Windows"
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define kh_PLATFORM "iOS"
#elif TARGET_OS_MAC
#define kh_PLATFORM "MacOS"
#else
#define kh_PLATFORM "Unknown (Apple)"
#endif
#define kh_PLATFORM "MacOS"
#elif defined(__ANDROID__)
#define kh_PLATFORM "Android"
#elif defined(__linux__)
#define kh_PLATFORM "Linux"
#elif defined(__FreeBSD__)
#define kh_PLATFORM "FreeBSD"
#elif defined(__unix__)
#define kh_PLATFORM "Unix"
#elif defined(__posix__)
#define kh_PLATFORM "POSIX"
#else
#define kh_OS "Unknown"
#endif


#ifdef __cplusplus
}
#endif
#endif
