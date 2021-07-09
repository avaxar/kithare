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
    This assumes you have GCC (g++) installed. Also, you need to install SDL
    dependencies on your own, via your systems package manager.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and 'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared
    libraries.

    A recommended and easy way to do this on MacOS, is via homebrew. Just run
    `brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.

    And the build is really simple, just run 'python3 build.py'

If you are on a 64-bit system, and want to compile for 32-bit architecture, pass
'--arch=x86' as an argument to the build script (note that this might not work
in some cases)

To just run tests, pass '--run-tests'. Note that this command is only going to
run the tests, it does not do anything else.

Any other arguments passed to this builder will be forwarded to the compiler.
This feature might fall of use for advanced users, who know what they are doing.
"""


import io
import os
import platform
import shutil
import sys
import tarfile
import urllib.request as urllib
from pathlib import Path
from typing import Union

INCLUDE_DIRNAME = "include"
ICO_RES = "icon.res"
EXE = "kcr"

if platform.system() == "Windows":
    EXE += ".exe"

# While we recursively search for include files, we don't want to seach
# the whole file, because that would waste a lotta time. So, we just take
# an arbitrary line number limit, beyond which, we won't search
INC_FILE_LINE_LIMIT = 50

# SDL project-version pairs, remember to keep updated
SDL_DEPS = {
    "SDL2": "2.0.14",
    "SDL2_image": "2.0.5",
    "SDL2_mixer": "2.0.4",
    "SDL2_ttf": "2.0.15",
    "SDL2_net": "2.0.1",
}


def find_includes(file: Path, basedir: Path):
    """
    Recursively find include files for a given file
    Returns an iterator
    """
    for line in file.read_text().splitlines()[:INC_FILE_LINE_LIMIT]:
        words = line.split()
        if len(words) < 2 or words[0] != "#include":
            continue

        retfile = basedir / INCLUDE_DIRNAME / words[1].strip('"<>').strip()
        if retfile.is_file():
            yield retfile
            yield from find_includes(retfile, basedir)


def should_build(file: Path, ofile: Path, basedir: Path):
    """
    Determines whether a particular cpp file should be rebuilt
    """
    if not ofile.is_file():
        return True

    ofile_m = ofile.stat().st_mtime
    if file.stat().st_mtime > ofile_m:
        return True

    return any(
        incfile.stat().st_mtime > ofile_m for incfile in find_includes(file, basedir)
    )


class KithareBuilder:
    """
    Kithare builder class
    """

    def __init__(self, basepath: Path, *args: str):
        """
        Initialise kithare builder
        """
        self.args = list(args)
        self.basepath = basepath
        self.objfiles = []  # populated later by a call to self.build_sources
        self.set_machine()
        self.parse_args()
        self.init_cflags()

    def set_machine(self):
        """
        Set machine type
        """
        self.is_32_bit = "--arch=x86" in self.args
        self.machine = platform.machine()

        if self.machine.endswith("86"):
            self.machine = "x86"
            self.machine_alt = "i686"

        elif self.machine.lower() in ["x86_64", "amd64"]:
            self.machine = "x86" if self.is_32_bit else "x64"
            self.machine_alt = "i686" if self.is_32_bit else "x86_64"

        elif self.machine.lower() in ["armv8l", "arm64", "aarch64"]:
            self.machine = "ARM" if self.is_32_bit else "ARM64"

        elif self.machine.lower().startswith("arm"):
            self.machine = "ARM"

        elif not self.machine:
            self.machine = "None"

    def parse_args(self):
        """
        Parse any arguments into respective flags
        """
        self.compiler = "MinGW" if platform.system() == "Windows" else "GCC"

        self.download_dir = self.basepath / "deps" / "SDL"

        dirname = f"{self.compiler}-{self.machine}"
        self.builddir = self.basepath / "build" / dirname
        self.distdir = self.basepath / "dist" / dirname
        self.exepath = self.distdir / EXE

        self.run_tests = "--run-tests" in self.args

        # Prune unneeded args
        for i in list(self.args):
            if i.startswith("--arch="):
                self.args.remove(i)

    def init_cflags(self):
        """
        Initailise compiler flags into a list
        """
        self.cflags = [
            "-O3",
            "-std=c++14",
            f"-I {self.basepath/INCLUDE_DIRNAME}",
            "-lSDL2",
            "-lSDL2main",
            "-lSDL2_image",
            "-lSDL2_ttf",
            "-lSDL2_mixer",
            "-lSDL2_net",
        ]

        if self.compiler == "MinGW":
            self.cflags.append("-municode")

        if self.is_32_bit and "-m32" not in self.args:
            self.cflags.append("-m32")

        self.cflags.extend(self.args)

    def download_sdl_deps(self, name: str, version: str):
        """
        SDL Dependency download utility for windows
        """
        download_link = "https://www.libsdl.org/"
        if name != "SDL2":
            download_link += f"projects/{name}/".replace("2", "")

        download_link += f"release/{name}-devel-{version}-mingw.tar.gz"

        download_path = self.download_dir / f"{name}-{version}"
        if download_path.is_dir():
            print(f"Skipping {name} download because it already exists")

        else:
            print(f"Downloading {name} from {download_link}")
            request = urllib.Request(
                download_link,
                headers={"User-Agent": "Chrome/35.0.1916.47 Safari/537.36"},
            )
            with urllib.urlopen(request) as download:
                response = download.read()

            print("Extracting compressed files")
            with io.BytesIO(response) as fileobj:
                with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
                    tarred.extractall(self.download_dir)

            print(f"Finished downloading {name}")

        # Copy DLLs
        sdl_mingw = download_path / f"{self.machine_alt}-w64-mingw32"
        for dll in sdl_mingw.glob("bin/*.dll"):
            shutil.copyfile(dll, self.distdir / dll.name)

        inc_dir = sdl_mingw / "include" / "SDL2"
        lib_dir = sdl_mingw / "lib"
        self.cflags.extend([f"-I {inc_dir}", f"-L {lib_dir}"])

    def compile_gpp(self, src: Union[str, Path], output: Path, is_src: bool):
        """
        Used to execute g++ commands
        """
        srcflag = "-c " if is_src else ""
        cmd = f"g++ -o {output} {srcflag}{src} {' '.join(self.cflags)}"

        print("\nBuilding", f"file: {src}" if is_src else "executable")
        print(cmd)
        return os.system(cmd)

    def build_sources(self):
        """
        Generate obj files from source files
        """
        isfailed = False
        ecode = 0

        for file in self.basepath.glob(f"src/**/*.cpp"):
            ofile = self.builddir / f"{file.stem}.o"
            self.objfiles.append(ofile)
            if should_build(file, ofile, self.basepath):
                ecode = self.compile_gpp(file, ofile, is_src=True)
                if ecode:
                    isfailed = True
                    print("g++ command exited with an error code:", ecode)

        if isfailed:
            print("Skipped building executable, because all files didn't build")
            sys.exit(ecode)

    def build_exe(self):
        """
        Generate final exe.
        """
        self.build_sources()

        args = " ".join(map(str, self.objfiles))

        # Handle exe icon on MinGW
        ico_res = self.basepath / ICO_RES
        if self.compiler == "MinGW":
            assetfile = self.basepath / "assets" / "Kithare.rc"
            os.system(f"windres {assetfile} -O coff -o {ico_res}")
            args += f" {ico_res}"

        try:
            dist_m = None
            if self.exepath.is_file():
                dist_m = self.exepath.stat().st_mtime

            ofile: Path
            for ofile in self.objfiles:
                if dist_m is None or ofile.stat().st_mtime > dist_m:
                    ecode = self.compile_gpp(args, self.exepath, is_src=False)
                    if ecode:
                        sys.exit(ecode)
                    break
        finally:
            if ico_res.is_file():
                ico_res.unlink()

    def build(self):
        """
        Build Kithare
        """
        self.builddir.mkdir(parents=True, exist_ok=True)
        self.distdir.mkdir(parents=True, exist_ok=True)

        if self.run_tests:
            sys.exit(os.system(f"{self.exepath} --test"))

        # Prepare dependencies and cflags with SDL flags
        if platform.system() == "Windows":
            self.download_dir.mkdir(parents=True, exist_ok=True)
            for package, ver in SDL_DEPS.items():
                self.download_sdl_deps(package, ver)

        else:
            usr = Path("/usr")
            for inc_dir in {
                usr / "include" / "SDL2",
                usr / "local" / "include" / "SDL2",
            }:
                if inc_dir.is_dir():
                    self.cflags.append(f"-I {inc_dir}")
                    break

        self.build_exe()
        print("Done!")


if __name__ == "__main__":
    argv = sys.argv.copy()
    dname = Path(argv.pop(0)).parent

    kithare = KithareBuilder(dname, *argv)
    kithare.build()
