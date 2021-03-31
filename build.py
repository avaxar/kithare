"""
Builder script to build Kithare.

On Windows and MinGW:
    You must have MinGW (AKA MinGW-w64) installed, and the bin folder of
    MinGW must be on PATH.

    This builder automatically installs SDL dependencies. Just run this file
    with: 'py build.py'.

On Windows and MSVC:
    Make sure you have Visual Studio 2019 with C/C++ build tools and Windows 10
    SDK installed.

    To build kithare from the command line, you first need to have the 'msbuild'
    command in your PATH. Then run 'py build.py --msvc'. This will build kithare
    sources using the MSVC compiler. By default, this command will build in
    "Release" mode, if you want to build in "Debug" mode, pass '--debug' too.

    If you are familiar with Visual Studio C++ IDE, you can also use the
    graphical interface in the IDE to build kithare. But in that case, you
    would first need to run 'py build.py --msvc-deps', as this installs the
    required dependencies.

On other OS:
    This assumes you have GCC (g++) installed. Also, you need to install SDL
    dependencies on your own, via your systems package manager.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and 'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared
    libraries.

    A recommended and easy way to do this on MacOS, is via homebrew. Just run
    `brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.

    And the build is really simple, just run 'python3 build.py'

If you are on a 64-bit system, and want to compile for 32-bit architecture,
pass '--arch=x86' as an argument to the build script (note that this might not
work in some cases)

If you want to compile tests too, pass '--build-tests' to this builder.

To just run tests, pass '--run-tests'. Note that this command is
only going to run the tests, it does not do anything else.

Any other arguments passed to this builder will be forwarded to the
compiler (but not on MSVC).

This feature might fall of use for advanced users, who know what they are
doing.
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

EXE = "kcr.exe" if platform.system() == "Windows" else "kcr"
TEST_EXE = "test_kcr.exe" if platform.system() == "Windows" else "test_kcr"

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

build_tests = "--build-tests" in sys.argv
if build_tests:
    sys.argv.remove("--build-tests")

run_tests = "--run-tests" in sys.argv
if run_tests:
    sys.argv.remove("--run-tests")

is_32_bit = "--arch=x86" in sys.argv

for i in ["--arch=x86", "--arch=x64"]:
    if i in sys.argv:
        sys.argv.remove(i)

_machine = platform.machine()
if _machine.endswith("86"):
    machine = "x86"
    machine_alt = "i686"

elif _machine.lower() in ["x86_64", "amd64"]:
    machine = "x86" if is_32_bit else "x64"
    machine_alt = "i686" if is_32_bit else "x86_64"

elif _machine.lower() in ["armv8l", "arm64", "aarch64"]:
    machine = "ARM" if is_32_bit else "ARM64"

elif _machine.lower().startswith("arm"):
    machine = "ARM"

else:
    machine = _machine if _machine else "None"

if platform.system() == "Windows":
    msvc_no_compile = "--msvc-deps" in sys.argv
    msvc_config = "Debug" if "--debug" in sys.argv else "Release"
    compiler = "MSVC" if "--msvc" in sys.argv or msvc_no_compile else "MinGW"

else:
    compiler = "GCC"


cflags = "-O3 -std=c++17 -I include"
cflags += " -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net"

if compiler == "MinGW":
    cflags += " -municode"

if is_32_bit and "-m32" not in sys.argv:
    cflags += " -m32"

cflags += " " + " ".join(sys.argv[1:])

download_dir = f"deps/SDL-{compiler}"


def mkdir(file):
    """
    Make a directory, don't error if it exists
    """
    os.makedirs(file, exist_ok=True)


def find_includes(file):
    """
    Recursively find include files for a given file
    """
    with open(file, "r") as f:
        for cnt, line in enumerate(f.read().splitlines()):
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

    for f in find_includes(file):
        if os.stat(f).st_mtime > ofile_m:
            return True
    return False


def download_sdl_deps(name, version):
    """
    SDL Dependency download utility for windows
    """
    download_link = "https://www.libsdl.org/"
    if name != "SDL2":
        download_link += f"projects/{name}/".replace("2", "")

    download_link += f"release/{name}-devel-{version}-"
    download_link += "VC.zip" if compiler == "MSVC" else "mingw.tar.gz"

    download_path = f"{download_dir}/{name}"

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
        if compiler == "MSVC":
            with zipfile.ZipFile(io.BytesIO(response), 'r') as zipped:
                zipped.extractall(download_dir)

        else:
            # Tarfile does not support bytes IO, so use temp file
            try:
                with open("temp", "wb") as f:
                    f.write(response)

                with tarfile.open("temp", 'r:gz') as tarred:
                    tarred.extractall(download_dir)
            finally:
                if os.path.exists("temp"):
                    os.remove("temp")

        os.rename(f"{download_path}-{version}", download_path)
        print(f"Finished downloading {name}")

    # Copy all SDL DLLs to dist dir
    if compiler == "MSVC":
        for i in ["x86", "x64"]:
            for j in ["Debug", "Release"]:
                d = f"dist/MSVC-{i}-{j}"
                mkdir(d)
                for dll in glob.iglob(f"{download_path}/lib/{i}/*.dll"):
                    shutil.copyfile(
                        dll,
                        os.path.join(d, os.path.basename(dll))
                    )
        return ""

    else:
        for dll in glob.iglob(
            f"{download_path}/{machine_alt}-w64-mingw32/bin/*.dll"
        ):
            shutil.copyfile(
                dll,
                os.path.join(f"dist/MinGW-{machine}", os.path.basename(dll))
            )
        return f" -I {download_path}/{machine_alt}-w64-mingw32/include/SDL2" + \
            f" -L {download_path}/{machine_alt}-w64-mingw32/lib"


def compile_gpp(src, output, srcflag="-c "):
    """
    Used to execute g++ commands
    """
    cmd = f"g++ -o {output} {srcflag}{src} {cflags}"
    print(cmd)
    return os.system(cmd)


def _build_exe(files, exepath):
    """
    Helper to generate final exe
    """
    print("Building exe")

    ecode = compile_gpp(" ".join(files).replace("\\", "/"), exepath, "")
    print()
    if ecode:
        sys.exit(ecode)


def build_exe(builddir, distdir, testmode=False, icoflag=""):
    """
    Generate final exe
    """
    exepath = f"{distdir}/{TEST_EXE if testmode else EXE}"
    objfiles = glob.glob(f"{builddir}/*.o")

    if testmode:
        # remove main in testmode, to remove 'main' redefination
        objfiles.remove(os.path.join(builddir, "main.o"))
    else:
        # prune away test files if we are not in testmode
        for i in list(objfiles):
            if os.path.basename(i).startswith("test_"):
                objfiles.remove(i)

    if icoflag:
        objfiles.append(icoflag)

    if not os.path.exists(exepath):
        _build_exe(objfiles, exepath)

    else:
        dist_m = os.stat(exepath).st_mtime
        for ofile in objfiles:
            if os.stat(ofile).st_mtime > dist_m:
                _build_exe(objfiles, exepath)
                break


def main():
    """
    Main code, that runs on startup
    """
    global cflags

    if compiler == "MSVC":
        distdir = f"dist/MSVC-{machine}-Release"
    else:
        builddir = f"build/{compiler}-{machine}"
        distdir = f"dist/{compiler}-{machine}"
        mkdir(builddir)
        mkdir(distdir)

    if run_tests:
        sys.exit(os.system(os.path.normpath(f"{distdir}/{TEST_EXE}")))

    # Prepare dependencies
    if platform.system() == "Windows":
        mkdir(download_dir)

        for package, ver in SDL_DEPS.items():
            cflags += download_sdl_deps(package, ver)

    else:
        for inc_dir in ["/usr/include/SDL2", "/usr/local/include/SDL2"]:
            if os.path.isdir(inc_dir):
                cflags += f" -I {inc_dir}"
                break

    if compiler == "MSVC":
        if msvc_no_compile:
            return

        bfile = "KithareTest.vcxproj" if build_tests else "Kithare.vcxproj"
        sys.exit(
            os.system(
                f"msbuild /m /p:Configuration={msvc_config} " + \
                f"/p:Platform={machine} {bfile}"
            )
        )

    print()
    isfailed = False
    for file in glob.iglob("src/**/*.cpp", recursive=True):
        ofile = f"{builddir}/{os.path.basename(file)}".replace(".cpp", ".o")
        if should_build(file, ofile):
            print("Building file:", file)
            if compile_gpp(file, ofile):
                print("g++ command exited with an error")
                isfailed = True
            print()

    if isfailed:
        print("Skipped building executable, because all files didn't build")
        sys.exit(1)

    icoflag = ""
    if compiler == "MinGW":
        icoflag = "icon.res"
        os.system(f"windres assets/Kithare.rc -O coff -o {icoflag}")

    build_exe(builddir, distdir, icoflag=icoflag)
    os.remove(icoflag)

    # Below section is for tests
    if build_tests:
        isfailed = False
        for file in glob.iglob("test/**/test_*.cpp", recursive=True):
            ofile = f"{builddir}/{os.path.basename(file)}".replace(".cpp", ".o")
            if should_build(file, ofile):
                print("Building test file:", file)
                if compile_gpp(file, ofile):
                    print("g++ command exited with an error")
                    isfailed = True
                print()

        if isfailed:
            print("Skipped building test exe, because all tests didn't build")
            sys.exit(1)

        build_exe(builddir, distdir, testmode=True)

    print("Done!")


if __name__ == "__main__":
    main()
