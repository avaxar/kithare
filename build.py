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

To just run tests, do 'python3 build.py test'. Note that this command is only
going to run the tests, it does not do anything else.

'python3 build.py clean' deletes folders that contain generated executable(s)
and temporary build files that are cached for performance reasons. In normal
usage one need not run this command, but in cases like change in version of
compiler and/or deps, one needs to run this command before installation.

Additionally on windows, one can run 'python3 build.py cleandep' to delete the
installed SDL dependencies. Note that this command is not required at all in
normal usage, and has been only provided for completeness sake, use only if
you know what you are doing.

Any other arguments passed to this builder will be forwarded to the compiler.
"""


import io
import json
import os
import platform
import shutil
import stat
import sys
import tarfile
import threading
import time
import urllib.request as urllib
from functools import lru_cache
from json.decoder import JSONDecodeError
from pathlib import Path
from typing import Union

INCLUDE_DIRNAME = "include"
ICO_RES = "icon.res"
EXE = "kcr"

COMPILER = "MinGW" if platform.system() == "Windows" else "GCC"
if COMPILER == "MinGW":
    EXE += ".exe"

# While we recursively search for include files, we don't want to seach
# the whole file, because that would waste a lotta time. So, we just take
# an arbitrary line number limit, beyond which, we won't search
INC_FILE_LINE_LIMIT = 75

# SDL project-version pairs, remember to keep updated
SDL_DEPS = {
    "SDL2": "2.0.14",
    "SDL2_image": "2.0.5",
    "SDL2_mixer": "2.0.4",
    "SDL2_ttf": "2.0.15",
    "SDL2_net": "2.0.1",
}


def run_cmd(cmd: str):
    """
    Helper function to run command in shell
    """
    print(cmd)
    return os.system(cmd)


@lru_cache(maxsize=256)
def find_includes_max_time(file: Path, incdir: Path) -> float:
    """
    Recursively find include files for a given file. Returns the latest time a
    file was modified
    """
    if not file.suffix:
        # no suffix for filename, C++ stdlib header
        return -1

    try:
        ret = file.stat().st_mtime
    except FileNotFoundError:
        return -1

    for line in file.read_text().splitlines()[:INC_FILE_LINE_LIMIT]:
        words = line.split()
        if len(words) < 2 or words[0] != "#include":
            # not an include line
            continue

        ret = max(ret, find_includes_max_time(incdir / words[1][1:-1], incdir))

    return ret


def should_build(file: Path, ofile: Path, incdir: Path):
    """
    Determines whether a particular cpp file should be rebuilt
    """
    try:
        # a file, or an included file was newer than the object file
        return find_includes_max_time(file, incdir) > ofile.stat().st_mtime
    except FileNotFoundError:
        return True


def rmtree(top: Path):
    """
    Reimplementation of shutil.rmtree. The reason shutil.rmtree itself is not
    used, is of a permission error in windows.
    """
    for newpath in top.iterdir():
        if newpath.is_dir():
            rmtree(newpath)
            continue

        newpath.chmod(stat.S_IWUSR)
        newpath.unlink()

    top.rmdir()


class KithareBuilder:
    """
    Kithare builder class
    """

    def __init__(self, basepath: Path, *args: str):
        """
        Initialise kithare builder
        """
        self.basepath = basepath
        self.baseinc = basepath / INCLUDE_DIRNAME

        is_32_bit = "--arch=x86" in args
        self.machine = platform.machine()
        if self.machine.endswith("86"):
            self.machine = "x86"
            self.machine_alt = "i686"

        elif self.machine.lower() in ["x86_64", "amd64"]:
            self.machine = "x86" if is_32_bit else "x64"
            self.machine_alt = "i686" if is_32_bit else "x86_64"

        elif self.machine.lower() in ["armv8l", "arm64", "aarch64"]:
            self.machine = "ARM" if is_32_bit else "ARM64"

        elif self.machine.lower().startswith("arm"):
            self.machine = "ARM"

        elif not self.machine:
            self.machine = "None"

        self.sdl_dir = self.basepath / "deps" / "SDL"
        self.sdl_mingw_include = self.sdl_dir / "include" / "SDL2"

        # debug mode for the builder
        debug = False
        if args and args[0] == "debug":
            debug = True
            args = args[1:]

        dirname = f"{COMPILER}-Debug" if debug else f"{COMPILER}-{self.machine}"
        self.builddir = self.basepath / "build" / dirname
        self.exepath = self.basepath / "dist" / dirname / EXE

        # store a build conf file that contains cflags, so that all the source
        # files are rebuilt if the cflags change
        self.build_conf = self.builddir / "build_conf.json"

        if args:
            if args[0] == "test":
                sys.exit(os.system(f"{self.exepath} --test"))

            if args[0] == "clean":
                for dist in {self.builddir, self.exepath.parent}:
                    if dist.parent.is_dir():
                        rmtree(dist.parent)
                sys.exit(0)

            if args[0] == "cleandep" and COMPILER == "MinGW":
                if self.sdl_dir.is_dir():
                    rmtree(self.sdl_dir)
                sys.exit(0)

        # compiler flags
        self.cflags = [
            "-g" if debug else "-O3",  # no -O3 on debug mode
            "-std=c++14",
            "-lSDL2",
            "-lSDL2main",
            "-lSDL2_image",
            "-lSDL2_ttf",
            "-lSDL2_mixer",
            "-lSDL2_net",
            f"-I {self.baseinc}",
        ]

        if COMPILER == "MinGW":
            self.cflags.append("-municode")

        if is_32_bit and "-m32" not in args:
            self.cflags.append("-m32")

        # update compiler flags with more args
        for i in args:
            if not i.startswith("--arch="):
                self.cflags.append(i)

    def download_sdl_deps(self, name: str, version: str):
        """
        SDL Dependency download utility for windows.
        """
        download_link = "https://www.libsdl.org/"
        if name != "SDL2":
            download_link += f"projects/{name}/".replace("2", "")

        download_link += f"release/{name}-devel-{version}-mingw.tar.gz"

        download_path = self.sdl_dir / f"{name}-{version}"
        sdl_mingw = download_path / f"{self.machine_alt}-w64-mingw32"
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

            with io.BytesIO(response) as fileobj:
                with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
                    tarred.extractall(self.sdl_dir)

            print(f"Finished downloading {name}")

            # Copy includes
            for header in sdl_mingw.glob("include/SDL2/*.h"):
                shutil.copyfile(header, self.sdl_mingw_include / header.name)

        # Copy DLLs that have not been copied already
        for dll in sdl_mingw.glob("bin/*.dll"):
            if dll.name not in (x.name for x in self.exepath.parent.glob("*.dll")):
                shutil.copyfile(dll, self.exepath.parent / dll.name)

        self.cflags.extend(("-L", str(sdl_mingw / "lib")))

    def compile_gpp(self, src: Union[str, Path], output: Path, is_src: bool):
        """
        Used to execute g++ commands
        """
        srcflag = "-c " if is_src else ""
        cmd = f"g++ -o {output} {srcflag}{src} " + " ".join(self.cflags)

        print("\nBuilding", f"file: {src}" if is_src else "executable")
        return run_cmd(cmd)

    def build_sources(self, build_skippable: bool):
        """
        Generate obj files from source files
        """
        isfailed = False
        ecode = 0

        skipped_files: list[str] = []
        objfiles: list[Path] = []
        for file in self.basepath.glob("src/**/*.cpp"):
            ofile = self.builddir / f"{file.stem}.o"
            if ofile in objfiles:
                print("BuildError: Got duplicate filename in Kithare source")
                sys.exit(1)

            objfiles.append(ofile)
            if build_skippable and not should_build(file, ofile, self.baseinc):
                skipped_files.append(str(file))
                continue

            ecode = self.compile_gpp(file, ofile, is_src=True)
            if ecode:
                isfailed = True
                print("g++ command exited with an error code:", ecode)

        if skipped_files:
            if len(skipped_files) == 1:
                print(f"\nSkipping file {skipped_files[0]}")
                print("Because the intermediate object file is already built")
            else:
                print("\nSkipping files:")
                print("\n".join(skipped_files))
                print("Because the intermediate object files are already built")

        if isfailed:
            print("Skipped building executable, because all files didn't build")
            sys.exit(1)

        if len(objfiles) == len(skipped_files) and self.exepath.is_file():
            # exe is already up to date
            return None

        return objfiles

    def build_exe(self):
        """
        Generate final exe.
        """
        # load old cflags from the previous build
        try:
            old_cflags = json.loads(self.build_conf.read_text())
        except (FileNotFoundError, JSONDecodeError):
            old_cflags = []

        # because order of args should not matter here
        build_skippable = sorted(self.cflags) == sorted(old_cflags)
        if not build_skippable:
            # update conf file with latest cflags
            self.build_conf.write_text(json.dumps(self.cflags))

        objfiles = self.build_sources(build_skippable)
        if objfiles is None:
            print("\nSkipping final exe build, since it is already built")
            return

        args = " ".join(map(str, objfiles))

        # Handle exe icon on MinGW
        ico_res = self.basepath / ICO_RES
        if COMPILER == "MinGW":
            assetfile = self.basepath / "assets" / "Kithare.rc"

            print("\nRunning windres command to set icon for exe")
            ret = run_cmd(f"windres {assetfile} -O coff -o {ico_res}")
            if ret:
                print(f"windres command failed with exit code {ret}")
                print("This means the final exe will not have the kithare logo")
            else:
                args += f" {ico_res}"

        ecode = self.compile_gpp(args, self.exepath, is_src=False)

        # delete icon file
        if ico_res.is_file():
            ico_res.unlink()

        if ecode:
            sys.exit(ecode)

    def build(self):
        """
        Build Kithare
        """

        # prepare directories
        self.builddir.mkdir(parents=True, exist_ok=True)
        self.exepath.parent.mkdir(parents=True, exist_ok=True)

        t1 = time.perf_counter()
        # Prepare dependencies and cflags with SDL flags
        if COMPILER == "MinGW":
            if self.sdl_dir.is_dir():
                # delete old SDL version installations, if any
                saved_dirs = [f"{n}-{v}" for n, v in SDL_DEPS.items()]
                saved_dirs.append("include")
                for subdir in self.sdl_dir.iterdir():
                    if subdir.name not in saved_dirs:
                        rmtree(subdir)

            # make SDL include dir
            self.sdl_mingw_include.mkdir(parents=True, exist_ok=True)

            # Download SDL deps if unavailable, use threading to download deps
            # concurrently
            threads: set[threading.Thread] = set()
            for package_and_ver in SDL_DEPS.items():
                thread = threading.Thread(
                    target=self.download_sdl_deps, args=package_and_ver, daemon=True
                )
                thread.start()
                threads.add(thread)

            for thread in threads:
                thread.join()

            # update cflags with SDL include
            self.cflags.extend(("-I", str(self.sdl_mingw_include.parent)))

        t2 = time.perf_counter()
        self.build_exe()
        print("Done!")

        t3 = time.perf_counter()

        # display stats
        print("\nSome timing stats for peeps who like to 'optimise':")
        if COMPILER == "MinGW":
            print(f"SDL deps took {t2 - t1:.3f} seconds to configure and install")
        print(f"Kithare took {t3 - t2:.3f} seconds to compile")


if __name__ == "__main__":
    argv = sys.argv.copy()
    dname = Path(argv.pop(0)).parent

    kithare = KithareBuilder(dname, *argv)
    kithare.build()
