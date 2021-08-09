"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/constants.py
A place to store a few constants used throughout the builder
"""

import os
import platform

INCLUDE_DIRNAME = "include"
ICO_RES = "icon.res"

SUPPORTED_ARCHS = {"x86", "x64", "arm", "armv6", "armv7", "arm64"}

# Make this False, when building with C source
CPP = True

STD_FLAG = "c++14" if CPP else "c99"
COMPILER_NAME = "g++" if CPP else "gcc"
SOURCE_GLOB = "src/**/*.cpp" if CPP else "src/**/*.c"

KITHARE_VERSION = "0.0.1rc1"
VERSION_PACKAGE_REV = "1"

COMPILER = "MinGW" if platform.system() == "Windows" else "GCC"
EXE = "kcr"
if COMPILER == "MinGW":
    EXE += ".exe"

_CPU_COUNT = os.cpu_count()
CPU_COUNT = 1 if _CPU_COUNT is None else _CPU_COUNT
