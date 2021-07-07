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


import glob
import os
import platform
import shutil
import sys
import tarfile
import urllib.request as urllib

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


def mkdir(file: str):
    """
    Make a directory, don't error if it exists
    """
    os.makedirs(file, exist_ok=True)


def find_includes(file: str, basedir: str):
    """
    Recursively find include files for a given file
    Returns an iterator
    """
    with open(file, "r") as fobj:
        for cnt, line in enumerate(fobj.read().splitlines()):
            words = line.split()
            if len(words) < 2:
                continue

            if words[0] == "#include":
                retfile = words[1]
                for char in ['"', "<", ">"]:
                    retfile = retfile.replace(char, "")

                retfile = os.path.join(
                    basedir, "include", os.path.normcase(retfile.strip())
                )

                if os.path.isfile(retfile):
                    yield retfile
                    yield from find_includes(retfile, basedir)

            if cnt >= INC_FILE_LINE_LIMIT:
                break


def should_build(file: str, ofile: str, basedir: str):
    """
    Determines whether a particular cpp file should be rebuilt
    """
    if not os.path.isfile(ofile):
        return True

    ofile_m = os.stat(ofile).st_mtime
    if os.stat(file).st_mtime > ofile_m:
        return True

    for incfile in find_includes(file, basedir):
        if os.stat(incfile).st_mtime > ofile_m:
            return True
    return False


class KithareBuilder:
    """
    Kithare builder class
    """

    def __init__(self, basepath: str, *args: str):
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

        self.download_dir = f"{self.basepath}/deps/SDL-{self.compiler}"

        dirname = f"{self.compiler}-{self.machine}"

        self.builddir = f"{self.basepath}/build/{dirname}"
        self.distdir = f"{self.basepath}/dist/{dirname}"

        self.run_tests = "--run-tests" in self.args

        # Prune unneeded args
        for i in list(self.args):
            if i.startswith("--arch="):
                self.args.remove(i)

    def init_cflags(self):
        """
        Initailise compiler flags into a list
        """
        self.cflags = ["-O3", "-std=c++14", f"-I {self.basepath}/include"]
        self.cflags.extend(
            [
                "-lSDL2",
                "-lSDL2main",
                "-lSDL2_image",
                "-lSDL2_ttf",
                "-lSDL2_mixer",
                "-lSDL2_net",
            ]
        )

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

        download_path = f"{self.download_dir}/{name}"

        if os.path.isdir(download_path):
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
            # Tarfile does not support bytes IO, so use temp file
            try:
                with open("temp", "wb") as tar:
                    tar.write(response)

                with tarfile.open("temp", "r:gz") as tarred:
                    tarred.extractall(self.download_dir)
            finally:
                if os.path.exists("temp"):
                    os.remove("temp")

            os.rename(f"{download_path}-{version}", download_path)
            print(f"Finished downloading {name}")

        # Copy DLLs
        sdl_mingw = f"{download_path}/{self.machine_alt}-w64-mingw32"
        for dll in glob.iglob(f"{sdl_mingw}/bin/*.dll"):
            shutil.copyfile(
                dll,
                os.path.join(self.distdir, os.path.basename(dll)),
            )

        self.cflags.extend([f"-I {sdl_mingw}/include/SDL2", f"-L {sdl_mingw}/lib"])

    def compile_gpp(self, src: str, output: str, is_src: bool):
        """
        Used to execute g++ commands
        """
        srcflag = "-c " if is_src else ""
        cmd = f"g++ -o {output} {srcflag}{src} {' '.join(self.cflags)}"
        src_repr = src.replace("\\", "/")

        print("\nBuilding", f"file: {src_repr}" if is_src else "executable")
        print(cmd.replace("\\", "/"))
        return os.system(cmd)

    def build_sources(self):
        """
        Generate obj files from source files
        """
        isfailed = 0

        for file in glob.iglob(f"{self.basepath}/src/**/*.cpp", recursive=True):
            ofile = f"{self.builddir}/{os.path.basename(file)}".replace(".cpp", ".o")
            self.objfiles.append(ofile)
            if should_build(file, ofile, self.basepath):
                isfailed = self.compile_gpp(file, ofile, is_src=True)
                if isfailed:
                    print("g++ command exited with an error code:", isfailed)

        if isfailed:
            print("Skipped building executable, because all files didn't build")
            sys.exit(isfailed)

    def build_exe(self):
        """
        Generate final exe.
        """
        exepath = f"{self.distdir}/{EXE}"
        self.build_sources()

        args = " ".join(self.objfiles).replace("\\", "/")

        # Handle exe icon
        try:
            if self.compiler == "MinGW":
                assetfile = f"{self.basepath}/assets/Kithare.rc"
                os.system(f"windres {assetfile} -O coff -o {ICO_RES}")
                args += f" {ICO_RES}"

            dist_m = None
            if os.path.exists(exepath):
                dist_m = os.stat(exepath).st_mtime

            for ofile in self.objfiles:
                if dist_m is None or os.stat(ofile).st_mtime > dist_m:
                    ecode = self.compile_gpp(args, exepath, is_src=False)
                    if ecode:
                        sys.exit(ecode)
                    break
        finally:
            if os.path.exists(ICO_RES):
                os.remove(ICO_RES)

    def build(self):
        """
        Build Kithare
        """
        mkdir(self.builddir)
        mkdir(self.distdir)

        if self.run_tests:
            sys.exit(os.system(os.path.normpath(f"{self.distdir}/{EXE} --test")))

        # Prepare dependencies and cflags with SDL flags
        if platform.system() == "Windows":
            mkdir(self.download_dir)

            for package, ver in SDL_DEPS.items():
                self.download_sdl_deps(package, ver)

        else:
            for inc_dir in ["/usr/include/SDL2", "/usr/local/include/SDL2"]:
                if os.path.isdir(inc_dir):
                    self.cflags.append(f"-I {inc_dir}")
                    break

        self.build_exe()
        print("Done!")


if __name__ == "__main__":
    argv = sys.argv.copy()
    dname = os.path.dirname(argv.pop(0))
    if not dname:
        dname = "."

    kithare = KithareBuilder(dname, *argv)
    kithare.build()
