/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* include/utility/string.hpp
* Declares the string type and its conversion functions for Kithare to maintain consistency in handling unicode. (*cough*, Windows)
*/

#pragma once

#include "utility/std.hpp"


namespace kh {
    typedef std::basic_string<uint32> String;

    kh::String toString(const std::string& str);
    kh::String toString(const std::wstring& wstr);

    std::string fromString(const kh::String& str);
    std::wstring fromStringW(const kh::String& str);
}
