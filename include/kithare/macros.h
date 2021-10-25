/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#ifndef kh_HG_MACROS_H
#define kh_HG_MACROS_H
#ifdef __cplusplus
extern "C" {
#endif


#define _kh_CONCAT(A, B) A##B
#define kh_CONCAT(A, B) _kh_CONCAT(A, B)


#ifdef __cplusplus
}
#endif
#endif
