/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/file.hpp>


using namespace kh;

std::string kh::FileError::format() const {
    return "unable to read file";
}

std::u32string kh::fileRead(const std::string& path) {
    return utf8Decode(fileReadBinary(path));
}

std::string kh::fileReadBinary(const std::string& path) {
    /* Use C style file handling, because it's "superior" (as @ankith26 would say it -.-), and also
     * handles UTF-8 file paths on MinGW correctly */
    std::string ret;
    FILE* file;
#ifdef _WIN32
    std::u32string u32path = utf8Decode(path);
    std::wstring u16path;
    u16path.reserve(u32path.size());
    for (char32_t ch : u32path) {
        if (ch > 0xFFFF) {
            throw FileError();
        }

        u16path += (wchar_t)ch;
    }

    file = _wfopen(u16path.c_str(), L"rb");
#else
    file = fopen(path.c_str(), "rb");
#endif

    if (!file) {
        throw FileError();
    }

    int c; /* Note: int, not char, required to handle EOF */
    while ((c = fgetc(file)) != EOF) {
        ret += (char)c;
    }

    if (ferror(file)) {
        throw FileError();
    }

    fclose(file);
    return ret;
}
