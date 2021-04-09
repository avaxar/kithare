/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/utility/file.hpp
* Declares file IO helper functions
*/

#pragma once

#include <string>


namespace kh {
    struct FileNotFound {
        FileNotFound(const std::u32string& _fname) :
            fname(_fname) {}

        std::u32string fname;
    };

    std::u32string readFile(const std::u32string& path);
    std::string readFileBinary(const std::u32string& path);
}
