# Kithare

![Kithare](assets/banner.png) <br/>
![State](https://img.shields.io/badge/state-unfinished-ff2222.svg)
![Version](https://img.shields.io/badge/version-0.0.0-00ffaa.svg)

 An open source general purpose statically-typed cross-platform interpreted/transpiled C++/Python like programming language.

## Links and Contact

- [Website](https://kithare.cf/Kithare)
- [Contributing guide](https://kithare.cf/Kithare/contributing)
- [Code of conduct](https://kithare.cf/Kithare/code_of_conduct)
- [Discord server](https://discord.gg/hXvY8CzS7A)
- [Channel on /r/ProgrammingLanguages discord](https://discord.gg/sggx9T9xsz)

## Installation and Versioning

- **Important note:** The language is still unfinished and there are no stable releases yet
- Kithare follows semantic versioning system for its releases.
- All releases will be live on the [Github releases tab](https://github.com/Kithare/Kithare/releases)
- For people who like to live on the edge, Kithare provides nighly builds, here are some direct download links
    1. [Windows builds (x86/x64)](https://nightly.link/Kithare/Kithare/workflows/windows/main/kithare-windows-installers.zip)
    2. [Linux builds (x86/x64)](https://nightly.link/Kithare/Kithare/workflows/linux/main/kithare-linux-installers.zip)
    3. [Linux multiarch builds (armv6/armv7/arm64/s390x/ppc64le)](https://nightly.link/Kithare/Kithare/workflows/linux-multiarch/main/kithare-linux-multiarch-installers.zip)
    4. [MacOS (Darwin) builds (x64)](https://nightly.link/Kithare/Kithare/workflows/darwin/main/kithare-darwin-installers.zip)
- For these builds, the version in the installer packages is usually given by `YYYY.MM.DD.HHmm-nightly`, so if the date during the build is 22nd August 2021 and the time is 09:10:36 UTC, then the nightly build version will look like `2021.08.22.0910-nightly`

## Building

- Kithare uses a python helper script to make building easier. So in order to build Kithare, you need Python v3.6 or above installed.
- For advanced usage, you may checkout the "build.py" file, which contains instructions on how to use the build script to achieve more things
- A basic HOWTO to building Kithare is given below

### Setup

#### Windows

- If you are building on Windows, make sure you have MinGW (aka MinGW-w64) installed, and its `bin` directory be put in the PATH.

#### Others

- On other platforms, you would need the GCC compiler installed (on mac, gcc is just a shim for clang).
- Install the development libraries for these: `SDL2`, `SDL2_mixer`, `SDL2_image`, `SDL2_ttf`, `SDL2_net`. You may use your disto's package manager to do this.
- A recommended way to do this on Mac is to use Homebrew. Just run
`brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.
- On Ubuntu and other debian based systems you can do `sudo apt-get install libsdl2-dev libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libmikmod-dev libfishsound1-dev libsmpeg-dev liboggz2-dev libflac-dev libfluidsynth-dev libsdl2-mixer-dev libfreetype6-dev libsdl2-ttf-dev libsdl2-net-dev -y` (these are mostly SDL deps)

### Build

- Run `python3 build.py` to build the project. If you are a `make` user, there is a stub `makefile` that calls the python build helper. On Windows it will automatically download the dependencies such as SDL.
