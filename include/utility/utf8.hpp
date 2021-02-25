/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* Extra credit:
* Ankith (ankith26)
* 
* include/utility/utf8.hpp
* Declares UTF-8 conversion functions which was some written in C by Ankith.
*/

#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"

#define _SAFE_INC_I                                             \
    i++;                                                        \
    if ((i >= size && size != -1) || (size == -1 && !src[i])) { \
        free(ret);                                              \
        success = false;                                        \
        return NULL; /* Unicode error */                        \
    }                                                           \
    if (src[i] < 0x80 || src[i] >= 0xC0) {                      \
        free(ret);                                              \
        success = false;                                        \
        return NULL; /* Unicode error */                        \
    }


namespace kh {
    std::string encodeUtf8(const kh::String& str);
    kh::String decodeUtf8(const std::string& utf8_str, bool& success);
    kh::String decodeUtf8(const std::string& utf8_str);

    int64 _getRealSizeUtf8(const uint8* src, const int64 size);
    int64 _getUint32SizeInUtf8(const uint32* src, const int64 size);
    uint32* _decodeFromUtf8(const uint8* csrc, const int64 size, bool& success);
    uint8* _encodeToUTF8(const uint32* src, const int64 size);
}
