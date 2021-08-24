"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

build.py
Builder script to build Kithare.

On Windows and MinGW:
    You must have MinGW (AKA MinGW-w64) installed, and the bin folder of MinGW
    must be on PATH.

    This builder automatically installs SDL dependencies. Just run this file
    with: 'py build.py'.

On other OS:
    This assumes you have GCC installed. Also, you need to install SDL
    dependencies on your own, via your systems package manager.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and
    'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared
    libraries.

    A recommended and easy way to do this on MacOS, is via homebrew. Just run
    `brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.

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

'python3 build.py --make clean' deletes folders that contain generated
executable(s) and temporary build files that are cached for performance
reasons. In normal usage one need not run this command, but in cases like 
change in version of compiler and/or deps, one needs to run this command before
installation.

To generate installers for Kithare, one can pass '--make installer' flag to
this build script. On Windows, this will use INNO Setup to make an exe
installer (INNO will be downloaded by the builder if not found). On Debian
linux (and derived distros), the builder makes a .deb installer using dpkg-deb.
Making packages for other package managers like rpm (Red Hat based distros like
RHEL, Fedora, CentOS, Oracle Linux and such), pacman (Arch based distos,
including Manjaro Linux), zypp (OpenSUSE) and Gentoo packages are in the to-do
list.
For now, there are no plans for a Mac installer, because none of the Kithare
core-devs have a mac setup for making, testing and debugging mac installers

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
