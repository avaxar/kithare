/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* include/test.hpp
* Base utilities for unit tests
*/

/* Pass a test function to this macro, that returns 0 on a successful test. Pass
 * a message argument to breifly describe the test */

#pragma once

#include <iostream>

#define KH_TEST_WITH_FUNC(funcname, msg)      \
    {                                         \
        std::cout << "  - Testing " msg ": "; \
        if (funcname()) {                     \
            fail++;                           \
            std::cout << "Failed\n";          \
        }                                     \
        else {                                \
            std::cout << "Passed\n";          \
        }                                     \
    }

/* Begin a test function declare block */
#define KH_TEST_BEGIN(modname)                  \
    int test##modname() {                       \
        int fail = 0;                           \
        std::cout << "> " #modname " module\n";

/* End a test function declare block */
#define KH_TEST_END                                             \
    std::cout << "  Failures in this module: " << fail << "\n"; \
    return fail; }

/* Test Equal macro */
#define KH_TEST_TRUE(exp)                                  \
    {                                                      \
        if (!exp) {                                      \
            std::cout << "\nFailed Assertion: " #exp "\n"; \
            return true;                                   \
        }                                                  \
    }

int testString();
int testLexer();
int testToken();
