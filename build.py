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

On Windows and MSVC:
    Make sure you have Visual Studio 2019 with C/C++ build tools and Windows 10
    SDK installed.

    If you are familiar with Visual Studio C++ IDE, you can use the graphical
    interface in the IDE to build kithare. But in that case, you would first
    need to run 'py build.py --msvc-deps', as this installs the required
    dependencies.

    To build kithare from the command line, you first need to have the 'msbuild'
    command in your PATH. Then run 'py build.py --msvc'. This will build kithare
    sources using the MSVC compiler. By default, this command will build in
    "Release" mode, if you want to build in "Debug" mode, pass '--debug' too.

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

If you want to compile tests too, pass '--build-tests' to this builder.

To just run tests, pass '--run-tests'. Note that this command is only going to
run the tests, it does not do anything else.

Any other arguments passed to this builder will be forwarded to the compiler,
but this is not true in the case of MSVC.
This feature might fall of use for advanced users, who know what they are doing.
"""

import glob
import io
import os
import platform
import shutil
import sys
import tarfile
import urllib.request as urllib
import zipfile

EXE = "kcr"
TEST_EXE = "test_kcr"

if platform.system() == "Windows":
    EXE += ".exe"
    TEST_EXE += ".exe"

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


def mkdir(file):
    """
    Make a directory, don't error if it exists
    """
    os.makedirs(file, exist_ok=True)


def find_includes(file):
    """
    Recursively find include files for a given file
    Returns an iterator
    """
    with open(file, "r") as fobj:
        for cnt, line in enumerate(fobj.read().splitlines()):
            if line.startswith("#include "):
                retfile = line[len("#include "):]
                for char in ['"', '<', '>']:
                    retfile = retfile.replace(char, "")

                retfile = os.path.join(
                    "include",
                    os.path.normcase(retfile.strip())
                )

                if os.path.isfile(retfile):
                    yield retfile
                    yield from find_includes(retfile)

            if cnt >= INC_FILE_LINE_LIMIT:
                break


def should_build(file, ofile):
    """
    Determines whether a particular cpp file should be rebuilt
    """
    if not os.path.isfile(ofile):
        return True

    ofile_m = os.stat(ofile).st_mtime
    if os.stat(file).st_mtime > ofile_m:
        return True

    for incfile in find_includes(file):
        if os.stat(incfile).st_mtime > ofile_m:
            return True
    return False


class KithareBuilder:
    """
    Kithare builder class
    """

    def __init__(self, args, basepath="."):
        """
        Initialise kithare builder
        """
        self.args = args
        self.basepath = basepath
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
        self.msvc_no_compile = False
        self.compiler = "GCC"
        if platform.system() == "Windows":
            self.msvc_no_compile = "--msvc-deps" in self.args
            self.msvc_config = "Debug" if "--debug" in self.args else "Release"
            if "--msvc" in self.args or self.msvc_no_compile:
                self.compiler = "MSVC"
            else:
                self.compiler = "MinGW"

        self.download_dir = f"{self.basepath}/deps/SDL-{self.compiler}"

        msvc_tags = ""
        if self.compiler == "MSVC":
            msvc_tags += "-" + self.msvc_config
        dirname = f"{self.compiler}-{self.machine}{msvc_tags}"

        self.builddir = f"{self.basepath}/build/{dirname}"
        self.distdir = f"{self.basepath}/dist/{dirname}"

        self.build_tests = "--build-tests" in self.args
        self.run_tests = "--run-tests" in self.args

        # Prune unneeded args
        for i in list(self.args):
            if i.startswith("--arch=") or i == "--build-tests":
                self.args.remove(i)

    def init_cflags(self):
        """
        Initailise compiler flags into a list
        """
        self.cflags = ["-O3", "-std=c++14", f"-I {self.basepath}/include"]
        self.cflags.extend(
            [
                "-lSDL2", "-lSDL2main", "-lSDL2_image", "-lSDL2_ttf",
                "-lSDL2_mixer", "-lSDL2_net"
            ]
        )

        if self.compiler == "MinGW":
            self.cflags.append("-municode")

        if self.is_32_bit and "-m32" not in self.args:
            self.cflags.append("-m32")

        self.cflags.extend(self.args)

    def copy_sdl_dll(self, download_path):
        """
        Copy SDL dll's into the dist folder, and also update the cflags to
        include the SDL library
        """
        if self.compiler == "MSVC":
            for i in ["x86", "x64"]:
                for j in ["Debug", "Release"]:
                    distdir = f"{self.basepath}/dist/MSVC-{i}-{j}"
                    mkdir(distdir)
                    for dll in glob.iglob(f"{download_path}/lib/{i}/*.dll"):
                        shutil.copyfile(
                            dll,
                            os.path.join(distdir, os.path.basename(dll))
                        )
            return

        for dll in glob.iglob(
            f"{download_path}/{self.machine_alt}-w64-mingw32/bin/*.dll"
        ):
            shutil.copyfile(
                dll,
                os.path.join(
                    f"{self.basepath}/dist/MinGW-{self.machine}",
                    os.path.basename(dll)
                )
            )
        self.cflags.extend([
            f"-I {download_path}/{self.machine_alt}-w64-mingw32/include/SDL2",
            f"-L {download_path}/{self.machine_alt}-w64-mingw32/lib"
        ])

    def download_sdl_deps(self, name, version):
        """
        SDL Dependency download utility for windows
        """
        download_link = "https://www.libsdl.org/"
        if name != "SDL2":
            download_link += f"projects/{name}/".replace("2", "")

        download_link += f"release/{name}-devel-{version}-"
        download_link += "VC.zip" if self.compiler == "MSVC" else "mingw.tar.gz"

        download_path = f"{self.download_dir}/{name}"

        if os.path.isdir(download_path):
            print(f"Skipping {name} download because it already exists")

        else:
            print(f"Downloading {name} from {download_link}")
            request = urllib.Request(
                download_link, headers={
                    'User-Agent': 'Chrome/35.0.1916.47 Safari/537.36'
                }
            )
            with urllib.urlopen(request) as download:
                response = download.read()

            print("Extracting compressed files")
            if self.compiler == "MSVC":
                with zipfile.ZipFile(io.BytesIO(response), 'r') as zipped:
                    zipped.extractall(self.download_dir)

            else:
                # Tarfile does not support bytes IO, so use temp file
                try:
                    with open("temp", "wb") as tar:
                        tar.write(response)

                    with tarfile.open("temp", 'r:gz') as tarred:
                        tarred.extractall(self.download_dir)
                finally:
                    if os.path.exists("temp"):
                        os.remove("temp")

            os.rename(f"{download_path}-{version}", download_path)
            print(f"Finished downloading {name}")

        self.copy_sdl_dll(download_path)

    def compile_gpp(self, src, output, srcflag="-c "):
        """
        Used to execute g++ commands
        """
        cmd = f"g++ -o {output} {srcflag}{src} {' '.join(self.cflags)}"
        print(cmd.replace("\\", "/"))
        return os.system(cmd)

    def build_sources(self, testmode):
        """
        Generate obj files from source files
        """
        isfailed = 0
        globpattern = self.basepath + "/"
        globpattern += "test/**/test_*.cpp" if testmode else "src/**/*.cpp"

        for file in glob.iglob(globpattern, recursive=True):
            cfile = f"{self.builddir}/{os.path.basename(file)}"
            ofile = cfile.replace(".cpp", ".o")
            if should_build(file, ofile):
                print("\nBuilding file:", file.replace("\\", "/"))
                isfailed = self.compile_gpp(file, ofile)
                if isfailed:
                    print("g++ command exited with an error code:", isfailed)

        if isfailed:
            print("Skipped building executable, because all files didn't build")
            sys.exit(isfailed)

    def build_exe(self, testmode=False):
        """
        Generate final exe.
        """
        if self.compiler == "MSVC":
            build_file = f"{self.basepath}\\msvc\\"
            build_file += "KithareTest" if testmode else "Kithare"
            build_file += ".vcxproj"
            ecode = os.system(
                f"msbuild /m /p:Configuration={self.msvc_config} " +
                f"/p:Platform={self.machine} {build_file}"
            )

            if ecode:
                sys.exit(ecode)
            return

        self.build_sources(testmode)

        exepath = f"{self.distdir}/{TEST_EXE if testmode else EXE}"
        objfiles = glob.glob(f"{self.builddir}/*.o")

        if testmode:
            # remove main in testmode, to remove 'main' redefination
            objfiles.remove(os.path.join(self.builddir, "main.o"))
        else:
            # prune away test files if we are not in testmode
            for i in list(objfiles):
                if os.path.basename(i).startswith("test_"):
                    objfiles.remove(i)

        args = " ".join(objfiles).replace("\\", "/")

        # Handle exe icon
        icores = "icon.res"
        if not testmode and self.compiler == "MinGW":
            assetfile = f"{self.basepath}/assets/Kithare.rc"
            os.system(f"windres {assetfile} -O coff -o {icores}")
            args += f" {icores}"

        if os.path.exists(exepath):
            dist_m = os.stat(exepath).st_mtime

        for ofile in objfiles:
            if not os.path.exists(exepath) or os.stat(ofile).st_mtime > dist_m:
                print("\nBuilding exe")
                ecode = self.compile_gpp(args, exepath, "")
                if ecode:
                    sys.exit(ecode)
                break

        if os.path.exists(icores):
            os.remove(icores)

    def build(self):
        """
        Build Kithare
        """
        mkdir(self.builddir)
        mkdir(self.distdir)

        if self.run_tests:
            sys.exit(os.system(os.path.normpath(f"{self.distdir}/{TEST_EXE}")))

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

        # Building in only dependency install mode
        if self.msvc_no_compile:
            return

        self.build_exe()
        if self.build_tests:
            self.build_exe(testmode=True)

        print("Done!")


if __name__ == "__main__":
    argv = sys.argv.copy()
    argv.pop(0)
    kithare = KithareBuilder(argv)
    kithare.build()
