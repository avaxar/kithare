/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/utility/file.cpp
* Defines include/utility/file.hpp
*/

#include "utility/file.hpp"


std::u32string kh::readFile(const std::u32string& path) {
    return kh::decodeUtf8(kh::readFileBinary(path));
}

std::string kh::readFileBinary(const std::u32string& path) {
    std::string content;

    /* Windows doesn't accept UTF-8 paths. In order to provide unicode path,
     * MSVC provide a std::wstring argument for std::fstream with its subclasses (std::ifstream, std::ofstream).
     * But MinGW's STL doesn't provide that overload, thus Kithare MinGW builds won't be able to open unicode named files.
     * GL fixing that @ankith26, I'ma add that to why MinGW sucks even more - Avaxar */
    #ifdef _MSC_VER
    std::wstring u16path;
    u16path.reserve(path.size());
    for (const char32_t ch : path)
        u16path += (wchar_t)ch;

    std::ifstream fin(u16path.c_str(), std::ios::in | std::ios::binary);
    #else
    std::ifstream fin(kh::encodeUtf8(path), std::ios::in | std::ios::binary);
    #endif

    char byte;

    if (fin.fail())
        throw kh::FileNotFound(path);

    while (!fin.eof()) {
        fin.read(&byte, 1);
        content += byte;
    }

    return content;
}
