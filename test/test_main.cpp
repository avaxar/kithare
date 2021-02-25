/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* test/test_main.cpp
* Main file for tests
*/

#include "test/test_string.hpp"

#undef main
#undef wmain
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    int tot_fails = 0;
    srand((unsigned int)time(NULL));
    std::setlocale(LC_ALL, "en_US.utf8");

    #ifdef _WIN32
    std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    std::wcout.imbue(utf8);
    #endif

    tot_fails += testString();

    std::cout << "Total number of failures are " << tot_fails << "\n";
    return tot_fails;
}