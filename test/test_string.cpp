/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* test/test_string.cpp
* Source for string file tests
*/

#include "test/test_string.hpp"


bool testFromStringToString() {
    std::string teststr = u8"¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ";
    kh::String temp = kh::toString(teststr);
    KH_TEST_TRUE(kh::fromString(temp) == teststr);
    return false;
}

bool testFromStringWToString() {
    std::wstring teststr = L"¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ";
    kh::String temp = kh::toString(teststr);
    KH_TEST_TRUE(kh::fromStringW(temp) == teststr);
    return false;
}

bool testUtf8Encode() {
    kh::String temp = kh::toString(L"¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ");
    std::string teststr = kh::encodeUtf8(temp);
    KH_TEST_TRUE(teststr == kh::encodeUtf8(temp));
    return false;
}

bool testUtf8Decode() {
    std::string teststr = u8"¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ";
    kh::String temp = kh::decodeUtf8(teststr);
    KH_TEST_TRUE(temp == kh::decodeUtf8(teststr));
    return false;
}

KH_TEST_BEGIN(String)
    KH_TEST_WITH_FUNC(testFromStringToString, "'fromString' and 'toString'")
    KH_TEST_WITH_FUNC(testFromStringWToString, "'fromStringW' and 'toString'")
    KH_TEST_WITH_FUNC(testUtf8Encode, "'encodeUtf8'")
    KH_TEST_WITH_FUNC(testUtf8Decode, "'decodeUtf8'")
KH_TEST_END
