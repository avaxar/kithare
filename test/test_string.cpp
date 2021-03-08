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


bool testStringEncodeToUtf8() {
    
}

bool testStringDecodeFromUtf8() {

}

KH_TEST_BEGIN(String)
    KH_TEST_WITH_FUNC(testStringEncodeToUtf8, "'encodeUtf8'")
    KH_TEST_WITH_FUNC(testStringDecodeFromUtf8, "'decodeUtf8'")
KH_TEST_END
