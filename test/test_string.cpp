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
    return kh::fromString(temp) != teststr;
}

bool testFromStringWToString() {
    std::wstring teststr = L"¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ";
    kh::String temp = kh::toString(teststr);
    return kh::fromStringW(temp) != teststr;
}

bool testUtf8Encode() {
    kh::String temp = kh::toString(L"¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ");
    std::string teststr = kh::encodeUtf8(temp);
    return teststr != kh::encodeUtf8(temp);
}

bool testUtf8Decode() {
    std::string teststr = u8"¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ";
    kh::String temp = kh::decodeUtf8(teststr);
    return kh::decodeUtf8(teststr) != temp;
}

KH_TEST_BEGIN(String)
    KH_TEST_WITH_FUNC(testFromStringToString, "'fromString' and 'toString'")
    KH_TEST_WITH_FUNC(testFromStringWToString, "'fromStringW' and 'toString'")
    KH_TEST_WITH_FUNC(testUtf8Encode, "'encodeUtf8'")
    KH_TEST_WITH_FUNC(testUtf8Decode, "'decodeUtf8'")
KH_TEST_END
