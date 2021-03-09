/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* test/test_string.cpp
* String utility functions unittest.
*/

#include "utility/string.hpp"

#include "test.hpp"

#define KH_TEST_U32STRING U"\x42\x63\xf6\xf3\u4242\u6363\U00010000"
#define KH_TEST_U8STRING  "\x42\x63\xc3\xb6\xc3\xb3\xe4\x89\x82\xe6\x8d\xa3\xf0\x90\x80\x80"


bool testStringEncodeToUtf8() {
    return kh::encodeUtf8(KH_TEST_U32STRING) != KH_TEST_U8STRING;
}

bool testStringDecodeFromUtf8() {
    return kh::decodeUtf8(KH_TEST_U8STRING) != KH_TEST_U32STRING;
}

KH_TEST_BEGIN(String)
    KH_TEST_WITH_FUNC(testStringEncodeToUtf8, "'encodeUtf8'")
    KH_TEST_WITH_FUNC(testStringDecodeFromUtf8, "'decodeUtf8'")
KH_TEST_END
