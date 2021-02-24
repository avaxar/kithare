/*
* This file is a part of the Kithare programming language source code.
* The Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
* 
* include/utility/std.hpp
* Includes the necessary C++ STL libraries and typedefs several built-in types.
*/

#pragma once

#include <stdio.h>
#include <ctype.h>
#include <cwctype>
#include <stdint.h>

#include <iostream>
#include <fstream>

#include <algorithm>
#include <cmath>
#include <clocale>
#include <functional>
#include <time.h>

#include <complex>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <codecvt>
#undef max
#undef min
#else
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <unistd.h>
#include <sys/ioctl.h>
#endif


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef std::complex<double> complex;
