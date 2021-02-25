/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* test/test_string.cpp
* Source for string file tests
*/

#include "test/test_string.hpp"

int testFromStringToSting(void) {
    std::string teststr = "¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹2je fe [32*# ";
    kh::String temp = kh::toString(teststr);
    return kh::fromString(temp) != teststr;
}

int testString(void) {
    int fail = 0;

    std::cout << "Testing 'FromString' and 'ToString': ";
    if (testFromStringToSting()) {
        fail++;
        std::cout << "Failed\n";
    }
    else 
        std::cout << "Passed\n";

    return fail;
}