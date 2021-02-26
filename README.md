![Kithare](assets/logo.png) <br/>
![State](https://img.shields.io/badge/state-unfinished-ff2222.svg)
![Version](https://img.shields.io/badge/version-0.0.0-00ffaa.svg)

# Kithare
 An open source easy-to-use and cross-platform C-like programmimg language.

## Contributing
- New Contributors are most welcome to come and help us in making Kithare better.
- If you want to contribute a major change, it's a good idea to discuss with us
first, either on our discord channel, or on github issues section.
- It is expected that contributors follow code formatting rules while contributing
to Kithare. If any *unclean* code is opened up in a PR, that PR will be closed
immediately.

## Building
- Have Python 3.6 or above installed
- To create 32-bit binaries, pass in `-m32` to the arguments. (An exception for MSVC)

### Windows
#### MSVC / Visual Studio
- Make sure you have Visual Studio 2019 with C/C++ build tools and Windows 10 SDK installed.
- Run `python3 build.py --msvc`. It'll download the dependencies such as SDL and create the build destination directory with the dependencies' DLLs copied.
- Open the solution `Kithare.sln`.
- See `Kithare` in the Solution Explorer. If the name's not bolded, right click and click "Set as Startup Project".
- You can now build it by clicking "Local Windows Debugger".

#### MinGW
- Make sure you have MinGW's bin directory be put in the PATH. (In order to call the compiler commands, especially `g++`)
- Run `python3 build.py`. It'll download the dependencies such as SDL and compile Kithare and put the executable in the build destination directory with the dependencies' DLLs copied.

### Other platforms
- Make sure you have the GCC compiler installed for linux, or Clang for MacOS.
- Install development libraries for these: `SDL2`, `SDL2_mixer`, `SDL2_image`, `SDL2_ttf`, `SDL2_net`.
- Run `python3 build.py`. It'll compile Kithare and put the executable in the build destination directory.

### Running Tests
- To run the tests, use the command `python3 build.py --run-tests`, note that 
this command only runs the tests, and does nothing else. Pass `--msvc` flag too, if
needed. For this command to work, the tests need to be built fisrt.

#### MSVC / Visual Studio
- Make sure you've successfully done the compilation above.
- Open the solution `Kithare.sln`.
- See `KithareTest` in the Solution Explorer. If the name's not bolded, right click and click "Set as Startup Project".
- You can now build it by clicking "Local Windows Debugger".

#### Others
- Run `python3 build.py --build-tests` to build the Kithare source AND unit test 
sources.

## Community / Contact
- [Discord server](https://discord.gg/hXvY8CzS7A)
