/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <string>
#include <vector>

#define KH_TEST_ASSERT(c)                                        \
    if (!(c)) {                                                  \
        errors_ptr->push_back("Assertion failed at " #c " in "); \
        goto error;                                              \
    }


namespace kh_test {
    void utf8Test(std::vector<std::string>& errors);
    void lexerTest(std::vector<std::string>& errors);
    void parserTest(std::vector<std::string>& errors);
}
