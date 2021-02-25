/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* test/test_string.cpp
* Source for string file tests
*/

#include "test/test_string.hpp"

int testFromStringToSting() {
    std::string teststr = "¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ";
    kh::String temp = kh::toString(teststr);
    return kh::fromString(temp) != teststr;
}

KH_TEST_BEGIN(String)
    KH_TEST_WITH_FUNC(testFromStringToSting, "'FromString' and 'ToString'")
KH_TEST_END