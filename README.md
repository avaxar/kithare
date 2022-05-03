![Kithare](misc/banner.png)

# Kithare

 A statically typed semi-safe programmming language which is inspired by [Python](https://www.python.org) and C++. Its syntax is designed to be easily-learnable by intermeddiate programmers and non-verbose. It offers functional style programming and object-oriented programming with polymorphism, function/method overloading, operator overloading, and templates. Join the [Discord](https://discord.com/invite/hXvY8CzS7A) server to keep track of the development or to ask for help.

## Supported OSes

- **Windows** is well supported by Kithare; both Windows 10 and Windows 11. Although not tested, Kithare should work well on Windows 7 and above too. Both 32-bit and 64-bit variants are supported on Windows, however, Windows on ARM architecture is not supported yet
- **GNU/Linux** is also well supported by Kithare; primarily Debian and Ubuntu based modern sufficient Linux distros. Kithare supports a range of architectures (x86, x64, armv6, armv7, arm64, s390x and ppc64le).
- **MacOS (Darwin)** machines whose version is 10.9 or above are supported. Kithare supports x86_64 (Intel) architecture and arm64 (Apple Silicon) architecture. In the future, Kithare will also support "universal" binaries, ones that works on both architectures. These univeral builds are "fat" builds, one that contains the binaries of both architectures in one binary. The usage of the "universal" builds are recommended over the usage of the architecture specific builds.

## Installation and Versioning

- **Important note**: The language is still unfinished and there are no stable releases yet.
- Kithare follows semantic versioning system for its releases.
- All releases will be live on the [GitHub releases tab](https://github.com/Kithare/Kithare/releases)
- For people who like to live on the edge, Kithare provides nighly builds, here are some direct download links

1. **Windows builds:**

    Each of these contain a portable ZIP-file based install, and an exe installer
    - [32-bit](https://nightly.link/Kithare/Kithare/workflows/windows/main/kithare-windows-x86-installers.zip)
    - [64-bit](https://nightly.link/Kithare/Kithare/workflows/windows/main/kithare-windows-x64-installers.zip)

2. **MacOS (Darwin) builds:**

    Each of these contain a portable ZIP-file based install (more installers are WIP)
    - [x86_64 (Intel)](https://nightly.link/Kithare/Kithare/workflows/darwin/main/kithare-darwin-x64-installers.zip)
    - [arm64 (Apple Silicon - not implemented yet)](https://nightly.link/Kithare/Kithare/workflows/darwin/main/kithare-darwin-arm64-installers.zip)
    - [universal2 (not implemented yet)](https://nightly.link/Kithare/Kithare/workflows/darwin/main/kithare-darwin-universal2-installers.zip)

3. **Linux builds:**

    Each of these contain a portable ZIP-file based install, a `deb` apt package, an `rpm` package and an `AppImage` installer.
    - [x86_64 (64 bit Intel/AMD)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-x64-installers.zip)
    - [x86 (32 bit Intel/AMD)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-x86-installers.zip)
    - [armv6 (armhf - Common in older Raspberry Pis)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-armv6-installers.zip)
    - [armv7 (armhf - Common in newer Raspberry Pis)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-armv7-installers.zip)
    - [arm64 (aarch64)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-arm64-installers.zip)
    - [s390x (not well supported/tested)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-s390x-installers.zip)
    - [ppc64le (not well supported/tested)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-ppc64le-installers.zip)

- For these builds, the version in the installer packages is usually given by `YYYY.MM.DD.HHmm-nightly`. So if the date during the build is 22nd August 2021 and the time is 09:10:36 UTC, then the nightly build version will look like `2021.08.22.0910-nightly`.

## Building

- Kithare uses a Python helper script to make building easier. So in order to build Kithare, you need Python v3.6 or above installed.
- For advanced usage, you may checkout the `build.py` file, which contains instructions on how to use the build script to achieve more things.
- A basic HOWTO to building Kithare is given below.

### Setup

#### Windows

- If you are building on Windows, the build script will automatically download and configure the deps. Kithare uses the MinGW compiler, if MinGW is not pre-installed, the buildscript will install it in a sub-directory. This means that on Windows, virtually no pre-build setup is required!

#### Others

- On other platforms, you would need the GCC compiler installed (On Mac, GCC is just a shim for clang).
- Install the development libraries for these: `SDL2`, `SDL2_mixer`, `SDL2_image`, `SDL2_ttf`, `SDL2_net`. You may use your distro's package manager to do this.
- A recommended way to do this on Mac is to use Homebrew. Just run `brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.
- On Ubuntu and other debian based systems, you can do `sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-net-dev -y`

### Build

- Run `python3 build.py` to build the project. If you are a `make` user, there is a stub `makefile` that calls the Python builder.
