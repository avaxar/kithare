/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_LISTS_H
#define kh_HG_LISTS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#ifndef khList_byte_DEFINED
#define khList_byte_DEFINED
#define khList_TYPE int8_t
#define khList_NAME khList_byte
#include <kithare/t_list.h>
#endif

#ifndef khList_ubyte_DEFINED
#define khList_ubyte_DEFINED
#define khList_TYPE uint8_t
#define khList_NAME khList_ubyte
#include <kithare/t_list.h>
#endif

#ifndef khList_short_DEFINED
#define khList_short_DEFINED
#define khList_TYPE int16_t
#define khList_NAME khList_short
#include <kithare/t_list.h>
#endif

#ifndef khList_ushort_DEFINED
#define khList_ushort_DEFINED
#define khList_TYPE uint16_t
#define khList_NAME khList_ushort
#include <kithare/t_list.h>
#endif

#ifndef khList_int_DEFINED
#define khList_int_DEFINED
#define khList_TYPE int32_t
#define khList_NAME khList_int
#include <kithare/t_list.h>
#endif

#ifndef khList_uint_DEFINED
#define khList_uint_DEFINED
#define khList_TYPE uint32_t
#define khList_NAME khList_uint
#include <kithare/t_list.h>
#endif

#ifndef khList_long_DEFINED
#define khList_long_DEFINED
#define khList_TYPE int64_t
#define khList_NAME khList_long
#include <kithare/t_list.h>
#endif

#ifndef khList_ulong_DEFINED
#define khList_ulong_DEFINED
#define khList_TYPE uint64_t
#define khList_NAME khList_ulong
#include <kithare/t_list.h>
#endif

#ifndef khList_float_DEFINED
#define khList_float_DEFINED
#define khList_TYPE float
#include <kithare/t_list.h>
#endif

#ifndef khList_double_DEFINED
#define khList_double_DEFINED
#define khList_TYPE double
#include <kithare/t_list.h>
#endif

#ifndef khList_char_DEFINED
#define khList_char_DEFINED
#define khList_TYPE uint32_t
#define khList_NAME khList_char
#include <kithare/t_list.h>
#endif


#ifdef __cplusplus
}
#endif
#endif
