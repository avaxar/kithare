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

C_STD_FLAG = "-std=c11"
CPP_STD_FLAG = "-std=c++14"

VERSION_PACKAGE_REV = "0"

COMPILER = "MinGW" if platform.system() == "Windows" else "GCC"
EXE = "kcr"
if COMPILER == "MinGW":
    EXE += ".exe"

_CPU_COUNT = os.cpu_count()
CPU_COUNT = 1 if _CPU_COUNT is None else _CPU_COUNT

INIT_TEXT = """Kithare Programming Language
----------------------------
An open source general purpose statically-typed cross-platform
interpreted/transpiled C++/Python like programming language.

The source code for Kithare programming language is distributed
under the MIT license.
Copyright (C) 2021 Kithare Organization

GitHub: https://github.com/Kithare/Kithare
Website: https://kithare.de/Kithare/
"""

EPILOG = """
For any bug reports or feature requests, check out the issue tracker on GitHub
https://github.com/Kithare/Kithare/issues
"""
