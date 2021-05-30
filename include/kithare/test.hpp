/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <string>
#include <vector>

#define KH_TEST_ASSERT(c) \
    if (!(c))             \
        goto error;


namespace kh_test {
    void utf8Test(std::vector<std::u32string>& errors);
    void lexerTest(std::vector<std::u32string>& errors);
    void parserTest(std::vector<std::u32string>& errors);
}
