/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* include/utility/utils.hpp
* Includes all of the utility headers; Provide some header-only overloads
*/

#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"
#include "utility/utf8.hpp"


/* These can't be put in separate source files */

inline std::ostream& operator<<(std::ostream& out_stream, const kh::String& string) {
    out_stream << kh::encodeUtf8(string);
    return out_stream;
}

inline std::wostream& operator<<(std::wostream& wout_stream, const kh::String& string) {
    std::wstring wstr;
    for (const uint32 chr : string)
        wstr += (wchar_t)chr;

    wout_stream << wstr;
    return wout_stream;
}
