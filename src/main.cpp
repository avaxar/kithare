/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/main.cpp
* The main entry point of the Kithare programming language compiler and runtime.
*/

#include "utility/cmd.hpp"

#undef main
#undef wmain
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    std::vector<std::u32string> args;

    // Ignore first argument
    #ifdef _WIN32
    for (int arg = 1; arg < argc; arg++)
        args.push_back(kh::repr(std::wstring(argv[arg])));
    #else
    for (int arg = 1; arg < argc; arg++)
        args.push_back(kh::decodeUtf8(std::string(argv[arg])));
    #endif

    return kh::run(args);
}
