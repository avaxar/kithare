/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* test/test_main.cpp
* Main file for tests
*/

#include "test/test_string.hpp"


int runTests() {
    int failures = 0;

    /* String tests */
    failures += testString();

    return failures;
}

#undef main
int main() {
    std::setlocale(LC_ALL, "en_US.utf8");
    #ifdef _WIN32
    /* Sets up std::wcout and std::wcin on Windows */
    std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    std::wcout.imbue(utf8);
    #endif

    return runTests();
}
