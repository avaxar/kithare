/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif


// The reason for this extra macro step is to expand any macro arguments rather than concatenating the
// macro name identifier.
#define _kh_CONCAT(A, B) A##B
#define kh_CONCAT(A, B) _kh_CONCAT(A, B)

// This too. Without this extra step, the macro argument name itself would be the string.
#define _kh_STRING(X) #X
#define kh_STRING(X) _kh_STRING(X)


#ifdef __cplusplus
}
#endif
