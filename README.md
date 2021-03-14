![Kithare](assets/banner.png) <br/>
![State](https://img.shields.io/badge/state-unfinished-ff2222.svg)
![Version](https://img.shields.io/badge/version-0.0.0-00ffaa.svg)

# Kithare
 An open source easy-to-use and cross-platform C-like programmimg language.

## Community / Contact
- [Discord server](https://discord.gg/hXvY8CzS7A)

## Building
- Kithare uses a python helper script to make building easier. So in order to
build Kithare, you need Python 3.6 or above installed
- For advanced usage, you may checkout the "build.py" file, which contains instructions on how to
use the build script to achieve more things
- A basic HOWTO to building Kithare is given below

### Windows
#### MSVC / Visual Studio
- Make sure you have Visual Studio 2019 with C++ build tools and Windows 10 SDK installed.
- To build kithare from the command line, run 'py build.py --msvc'. This command will build kithare sources using the MSVC compiler.
- If you are familiar with Visual Studio C++ IDE, you can also use the graphical
interface in the IDE to build kithare. But in that case, you would first need to run 
'py build.py --msvc-dep', as this installs the required dependencies.

#### MinGW
- Make sure you have MinGW (aka MinGW-w64) installed, and it's `bin` directory be put in 
the PATH.
- Run `python3 build.py`. It will automatically download the dependencies such as SDL and 
compile Kithare.

### Other platforms
- Make sure you have the GCC compiler.
- Install development libraries for these: `SDL2`, `SDL2_mixer`, `SDL2_image`, `SDL2_ttf`, `SDL2_net`.
- You may use your disto's package manager to do this.
- A recommended way to do this on Mac, is to use Homebrew. Just run
`brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`
- Run `python3 build.py`.

## Contributing
- New Contributors are most welcome to come and help us in making Kithare better.
- If you want to contribute a major change, it's a good idea to discuss with us
first, either on our discord server, or on GitHub issues section.
- It is expected that contributors follow code formatting rules while contributing
to Kithare. If any *unclean* code is opened up in a PR, that PR will be closed
immediately.

