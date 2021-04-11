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
    /* Use C style file handling, because it's "superior" (as @ankith26 would say it -.-), and also
     * handles UTF-8 file paths on MinGW correctly */
    std::string ret;
    FILE* file;
#if _WIN32
    std::wstring u16path;
    u16path.reserve(path.size());
    for (const char32_t ch : path)
        u16path += (wchar_t)ch;

    file = _wfopen(u16path.c_str(), L"rb");
#else
    file = fopen(kh::encodeUtf8(path).c_str(), "rb");
#endif

    if (!file)
        throw kh::FileNotFound(path);

    int c; /* Note: int, not char, required to handle EOF */
    while ((c = fgetc(file)) != EOF)
        ret += (char)c;

    if (ferror(file))
        throw kh::FileNotFound(path);

    fclose(file);
    return ret;
}
