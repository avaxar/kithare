/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_ARRAYS_H
#define kh_HG_ARRAYS_H
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
#include <kithare/t_array.h>
#endif

#ifndef khArray_ubyte_DEFINED
#define khArray_ubyte_DEFINED
#define khArray_TYPE int8_t
#define khArray_NAME khArray_sbyte
#include <kithare/t_array.h>
#endif

#ifndef khArray_short_DEFINED
#define khArray_short_DEFINED
#define khArray_TYPE int16_t
#define khArray_NAME khArray_short
#include <kithare/t_array.h>
#endif

#ifndef khArray_ushort_DEFINED
#define khArray_ushort_DEFINED
#define khArray_TYPE uint16_t
#define khArray_NAME khArray_ushort
#include <kithare/t_array.h>
#endif

#ifndef khArray_int_DEFINED
#define khArray_int_DEFINED
#define khArray_TYPE int32_t
#define khArray_NAME khArray_int
#include <kithare/t_array.h>
#endif

#ifndef khArray_uint_DEFINED
#define khArray_uint_DEFINED
#define khArray_TYPE uint32_t
#define khArray_NAME khArray_uint
#include <kithare/t_array.h>
#endif

#ifndef khArray_long_DEFINED
#define khArray_long_DEFINED
#define khArray_TYPE int64_t
#define khArray_NAME khArray_long
#include <kithare/t_array.h>
#endif

#ifndef khArray_ulong_DEFINED
#define khArray_ulong_DEFINED
#define khArray_TYPE uint64_t
#define khArray_NAME khArray_ulong
#include <kithare/t_array.h>
#endif

#ifndef khArray_float_DEFINED
#define khArray_float_DEFINED
#define khArray_TYPE float
#include <kithare/t_array.h>
#endif

#ifndef khArray_double_DEFINED
#define khArray_double_DEFINED
#define khArray_TYPE double
#include <kithare/t_array.h>
#endif

#ifndef khArray_char_DEFINED
#define khArray_char_DEFINED
#define khArray_TYPE char32_t
#define khArray_NAME khArray_char
#include <kithare/t_array.h>
#endif


#ifdef __cplusplus
}
#endif
#endif
