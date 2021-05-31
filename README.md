![Kithare](assets/banner.png) <br/>
![State](https://img.shields.io/badge/state-unfinished-ff2222.svg)
![Version](https://img.shields.io/badge/version-0.0.0-00ffaa.svg)

# Kithare
 An open source general purpose statically-typed cross-platform interpreted/transpiled C++/Python like programming language.

## Links and Contact
- [Website](https://kithare.cf/Kithare)
- [Contributing guide](https://kithare.cf/Kithare/contributing)
- [Code of conduct](https://kithare.cf/Kithare/code_of_conduct)
- [Discord server](https://discord.gg/hXvY8CzS7A)
- [Channel on /r/ProgrammingLanguages discord](https://discord.gg/sggx9T9xsz)

## Building
- Kithare uses a python helper script to make building easier. So in order to
build Kithare, you need Python 3.6 or above installed
- For advanced usage, you may checkout the "build.py" file, which contains instructions on how to
use the build script to achieve more things
- A basic HOWTO to building Kithare is given below

### Windows
#### MSVC / Visual Studio
- Make sure you have Visual Studio 2019 with C++ build tools and Windows 10 SDK installed.
- Run `python3 build.py --msvc-deps`. It'll download the dependencies such as SDL and create the build destination directory with the dependencies' DLLs copied.
- Open the solution file, `Kithare.sln`.
- See `Kithare` in the Solution Explorer. If the name's not bolded, right click and click "Set as Startup Project".
- You can now build it by clicking "Local Windows Debugger".

#### MinGW
- Make sure you have MinGW (aka MinGW-w64) installed, and its `bin` directory be put in 
the PATH.
- Run `python3 build.py`. It will automatically download the dependencies such as SDL and 
compile Kithare.

### Other platforms
- Make sure you have the GCC compiler.
- Install the development libraries for these: `SDL2`, `SDL2_mixer`, `SDL2_image`, `SDL2_ttf`, `SDL2_net`. You may use your disto's package manager to do this.
- A recommended way to do this on Mac is to use Homebrew. Just run
`brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.
- Run `python3 build.py`.
