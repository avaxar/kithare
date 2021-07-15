/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/file.hpp>
#include <kithare/string.hpp>
#include <kithare/utf8.hpp>


using namespace std;

string kh::FileError::format() const {
    return "unable to read file";
}

u32string kh::readFile(const u32string& path) {
    return kh::decodeUtf8(kh::readFileBinary(path));
}

string kh::readFileBinary(const u32string& path) {
    /* Use C style file handling, because it's "superior" (as @ankith26 would say it -.-), and also
     * handles UTF-8 file paths on MinGW correctly */
    string ret;
    FILE* file;
#if _WIN32
    wstring u16path;
    u16path.reserve(path.size());
    for (char32_t ch : path) {
        u16path += (wchar_t)ch;
    }

    file = _wfopen(u16path.c_str(), L"rb");
#else
    file = fopen(kh::encodeUtf8(path).c_str(), "rb");
#endif

    if (!file) {
        throw kh::FileError();
    }

    int c; /* Note: int, not char, required to handle EOF */
    while ((c = fgetc(file)) != EOF) {
        ret += (char)c;
    }

    if (ferror(file)) {
        throw kh::FileError();
    }

    fclose(file);
    return ret;
}
