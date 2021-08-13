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

# A set of architectures well supported by Kithare (have CI running tests on these)
SUPPORTED_ARCHS = {"x86", "x64", "armv6", "armv7", "arm64", "ppc64le", "s390x"}


STD_FLAG = {
    ".cpp": "--std=c++14",
    ".c": "--std=c99",
}

COMPILER_NAME = {
    ".cpp": "g++",
    ".c": "gcc",
}

# Kithare version and rev, remember to keep updated
KITHARE_VERSION = "0.0.1rc1"
VERSION_PACKAGE_REV = "1"

COMPILER = "MinGW" if platform.system() == "Windows" else "GCC"
EXE = "kcr"
if COMPILER == "MinGW":
    EXE += ".exe"

_CPU_COUNT = os.cpu_count()
CPU_COUNT = 1 if _CPU_COUNT is None else _CPU_COUNT

# flags that Kithare uses
KITHARE_FLAGS = {"--make-installer", "--use-alien"}
