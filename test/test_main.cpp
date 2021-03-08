/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* test/test_main.cpp
* Main file for tests
*/

#include <clocale>
#include <ctime>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <codecvt>
#endif

#include "utility/string.hpp"

#include "test.hpp"


/* wmain is needed for Windows in this case */
#undef main
#undef wmain
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    srand((unsigned int)time(NULL));
    std::setlocale(LC_ALL, "en_US.utf8");
    
    int failures = 0;
    std::cout << "Running Kithare Unit tests!\n";

    failures += testString();
    failures += testLexer();
    failures += testToken();

    std::cout << "Total number of failures are " << failures << "\n";
    return failures;
}
