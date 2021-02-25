/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* src/utility/string.cpp
* Defines the declarations in include/string.hpp
*/

#include "utility/string.hpp"
#include "utility/utf8.hpp"


kh::String kh::toString(const std::string& str) {
    return kh::decodeUtf8(str);
}

kh::String kh::toString(const std::wstring& wstr) {
    kh::String string;
    
    for (const wchar_t wchr : wstr)
        string += wchr;

    return string;
}

std::string kh::fromString(const kh::String& str) {
    return kh::encodeUtf8(str);
}

std::wstring kh::fromStringW(const kh::String& str) {
    std::wstring wstr;
    for (const uint32 chr : str)
        wstr += (wchar_t)chr;

    return wstr;
}

std::ostream& kh::operator<<(std::ostream& out_stream, const kh::String& string) {
    out_stream << kh::encodeUtf8(string);
    return out_stream;
}

std::wostream& kh::operator<<(std::wostream& wout_stream, const kh::String& string) {
    std::wstring wstr;
    for (const uint32 chr : string)
        wstr += (wchar_t)chr;

    wout_stream << wstr;
    return wout_stream;
}
