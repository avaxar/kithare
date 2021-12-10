/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2021 Kithare Organization at https://www.kithare.de
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Avaxar: Apple sucks
#ifdef __APPLE__
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#else
#include <uchar.h>
#endif


#ifndef khArray_byte_DEFINED
#define khArray_byte_DEFINED
#define khArray_TYPE uint8_t
#define khArray_NAME khArray_byte
#include "t_array.h"
#endif

#ifndef khArray_ubyte_DEFINED
#define khArray_ubyte_DEFINED
#define khArray_TYPE int8_t
#define khArray_NAME khArray_sbyte
#include "t_array.h"
#endif

#ifndef khArray_short_DEFINED
#define khArray_short_DEFINED
#define khArray_TYPE int16_t
#define khArray_NAME khArray_short
#include "t_array.h"
#endif

#ifndef khArray_ushort_DEFINED
#define khArray_ushort_DEFINED
#define khArray_TYPE uint16_t
#define khArray_NAME khArray_ushort
#include "t_array.h"
#endif

#ifndef khArray_int_DEFINED
#define khArray_int_DEFINED
#define khArray_TYPE int32_t
#define khArray_NAME khArray_int
#include "t_array.h"
#endif

#ifndef khArray_uint_DEFINED
#define khArray_uint_DEFINED
#define khArray_TYPE uint32_t
#define khArray_NAME khArray_uint
#include "t_array.h"
#endif

#ifndef khArray_long_DEFINED
#define khArray_long_DEFINED
#define khArray_TYPE int64_t
#define khArray_NAME khArray_long
#include "t_array.h"
#endif

#ifndef khArray_ulong_DEFINED
#define khArray_ulong_DEFINED
#define khArray_TYPE uint64_t
#define khArray_NAME khArray_ulong
#include "t_array.h"
#endif

#ifndef khArray_float_DEFINED
#define khArray_float_DEFINED
#define khArray_TYPE float
#include "t_array.h"
#endif

#ifndef khArray_double_DEFINED
#define khArray_double_DEFINED
#define khArray_TYPE double
#include "t_array.h"
#endif

#ifndef khArray_char_DEFINED
#define khArray_char_DEFINED
#define khArray_TYPE char32_t
#define khArray_NAME khArray_char
#include "t_array.h"
#endif


#ifdef __cplusplus
}
#endif
