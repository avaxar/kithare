/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 *
 * test/test_main.cpp
 * Main file for tests
 */

#include "test.hpp"
#include "utility/string.hpp"

/* wmain is needed for Windows in this case */
#undef main
#undef wmain
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    int failures = 0;
    kprintln("Running Kithare Unit tests!");

    failures += testString();
    failures += testLexer();

    kprint("Total number of failures are ");
    kprintln((uint64_t)failures);

    return failures;
}
