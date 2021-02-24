# Kithare
 An open source high performant interpreted statically typed programming language.

## Contributing
- TODO: Someone fill this pls

## Building
- Have Python 3.7 or above installed

### Windows
#### MSVC / Visual Studio
- Make sure you have Visual Studio 2019 with C/C++ build tools and Windows 10 SDK installed
- Run `python3 build.py --msvc`. It'll download the dependencies such as SDL and create the build destination directory with the dependencies' DLLs copied.
- Open the `Kithare.sln` and you can build it on Visual Studio right away!

#### MinGW
- Make sure you have MinGW's bin directory be put in the PATH. (In order to call the compiler commands, especially `g++`)
- Run `python3 build.py`. It'll download the dependencies such as SDL and compile Kithare and put the executable in the build destination directory with the dependencies' DLLs copied.
- To create 32-bit binaries, pass in `-m32` to the arguments.

### Other platforms
- Make sure you have the GCC compiler installed for linux, or Clang for MacOS and Darwin.
- Install development libraries for these: `'SDL2`, `SDL2_mixer`, `SDL2_image`, `SDL2_ttf`, `SDL2_net`.
- Run `python3 build.py`. It'll compile Kithare and put the executable in the build destination directory.

## Community / Contact
- [Discord server](https://discord.gg/hXvY8CzS7A)
