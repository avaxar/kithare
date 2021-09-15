"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

build.py
Builder script to build Kithare.

On Windows and MinGW:
    If you are building on Windows, the build script will automatically
    download and configure the deps. Kithare uses the MinGW compiler,
    if MinGW is not pre-installed, the buildscript will install it in a local
    directory. This means that on Windows, virtually no pre-build setup is
    required, just run this file with 'py build.py'.

On other OS:
    This assumes you have GCC installed. Also, you need to install SDL
    dependencies on your own, via your systems package manager.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and
    'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared
    libraries.

    A recommended and easy way to do this on MacOS, is via homebrew. Just run
    `brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.

    And the build is really simple, just run 'python3 build.py'

If you are on a 64-bit system, and want to compile for 32-bit architecture,
pass '--arch x86' as an argument to the build script (note that this might not
work in some cases)

By default, the builder uses all cores on the machine to build Kithare. But if
you want the builder to consume less CPU power while compiling (at the cost of
longer compile times), you can use the '-j' flag to set the number of cores you
want the builder to use. '-j1' means that you want to use only one core, '-j4'
means that you want to use 4 cores, and so on.

To just run tests, do 'python3 build.py --make test'. Note that this command is 
only going to run the tests, it does not do anything else.

'python3 build.py --clean dep' deletes any deps that the build script would
have installed automatically in the 'deps' dir

'python3 build.py --clean build' deletes folders that contain generated
executable(s) and temporary build files that are cached for performance
reasons. 

'python3 build.py --clean installers' deletes any generated Kithare installers
and any temporary build files associated with it.

'python3 build.py --clean all' is a shorthand for calling all the above clean
commands in a single command. In normal usage one need not run clean commands,
but these are provided by the script anyways if you know what you are doing.

To generate installers for Kithare, one can pass '--make installer' flag to
this build script. On Windows, this will use INNO Setup to make an exe
installer (INNO will be downloaded by the builder if not found). On Debian
linux (and derived distros), the builder makes a .deb installer using dpkg-deb.

Additionally on linux distros, one can pass a '--use-alien' flag, this will
make the builder use the 'alien' package to generate a package for another
distro from the package generated for the host distro. This feature is still
considered an alpha-quality feature though.

To pass any additional compiler flags, one can use CFLAGS, CPPFLAGS, CXXFLAGS,
LDFLAGS and LDLIBS (makefile terminology) and additionally CCFLAGS (for unified
C and C++ compilation flags). These can be set into env variables which the
builder script will load from
"""

import sys

from builder.constants import EPILOG
from builder import BuildError, KithareBuilder


def main():
    """
    Invoke Kithare builder class with arguments and build Kithare
    """
    err_code = 0
    try:
        kithare = KithareBuilder()
        kithare.build()

    except BuildError as err:
        err_code = err.ecode
        if err.emsg:
            print("BuildError:", err.emsg)

    except Exception:  # pylint: disable=broad-except
        print(
            "Unknown exception occured! This is probably a bug in the build "
            "script itself. Report this bug to Kithare devs, along with the "
            f"whole buildlog.{EPILOG}"
        )
        raise

    except KeyboardInterrupt:
        err_code = 1
        print("Compilation was terminated with Keyboard Interrupt")

    print(EPILOG)
    sys.exit(err_code)


if __name__ == "__main__":
    main()
