/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/utility/cmd.hpp
* Implements argument parsing for kithare commandline interface
*/

#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "utility/string.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"

namespace kh {
    struct FileNotFound {
        FileNotFound (const std::u32string& _fname) :
            fname(_fname) {}

        std::u32string fname;
    };

    std::u32string readFile(std::u32string scriptname);
    int run(const std::vector<std::u32string>& args);
}
