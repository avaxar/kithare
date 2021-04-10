"""
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

IS_32_BIT = "--arch=x86" in sys.argv

for arch in ["--arch=x86", "--arch=x64"]:
    if arch in sys.argv:
        sys.argv.remove(arch)

MACHINE = platform.machine()
if MACHINE.endswith("86"):
    MACHINE = "x86"
    MACHINE_ALT = "i686"

elif MACHINE.lower() in ["x86_64", "amd64"]:
    MACHINE = "x86" if IS_32_BIT else "x64"
    MACHINE_ALT = "i686" if IS_32_BIT else "x86_64"

elif MACHINE.lower() in ["armv8l", "arm64", "aarch64"]:
    MACHINE = "ARM" if IS_32_BIT else "ARM64"

elif MACHINE.lower().startswith("arm"):
    MACHINE = "ARM"

elif not MACHINE:
    MACHINE = "None"

MSVC_NO_COMPILE = False
if platform.system() == "Windows":
    MSVC_NO_COMPILE = "--msvc-deps" in sys.argv
    MSVC_CONFIG = "Debug" if "--debug" in sys.argv else "Release"
    COMPILER = "MSVC" if "--msvc" in sys.argv or MSVC_NO_COMPILE else "MinGW"

else:
    COMPILER = "GCC"

DOWNLOAD_DIR = f"deps/SDL-{COMPILER}"


def mkdir(file):
    """
    Make a directory, don't error if it exists
    """
    os.makedirs(file, exist_ok=True)


def find_includes(file):
    """
    Recursively find include files for a given file
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


def copy_sdl_dll(download_path):
    """
    Copy SDL dll's into the dist folder, and also return the cflags to include
    the SDL library
    """
    if COMPILER == "MSVC":
        for i in ["x86", "x64"]:
            for j in ["Debug", "Release"]:
                distdir = f"dist/MSVC-{i}-{j}"
                mkdir(distdir)
                for dll in glob.iglob(f"{download_path}/lib/{i}/*.dll"):
                    shutil.copyfile(
                        dll,
                        os.path.join(distdir, os.path.basename(dll))
                    )
        return tuple()

    for dll in glob.iglob(
        f"{download_path}/{MACHINE_ALT}-w64-mingw32/bin/*.dll"
    ):
        shutil.copyfile(
            dll,
            os.path.join(f"dist/MinGW-{MACHINE}", os.path.basename(dll))
        )
    return (
        f"-I {download_path}/{MACHINE_ALT}-w64-mingw32/include/SDL2",
        f"-L {download_path}/{MACHINE_ALT}-w64-mingw32/lib"
    )


def download_sdl_deps(name, version):
    """
    SDL Dependency download utility for windows
    """
    download_link = "https://www.libsdl.org/"
    if name != "SDL2":
        download_link += f"projects/{name}/".replace("2", "")

    download_link += f"release/{name}-devel-{version}-"
    download_link += "VC.zip" if COMPILER == "MSVC" else "mingw.tar.gz"

    download_path = f"{DOWNLOAD_DIR}/{name}"

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
        if COMPILER == "MSVC":
            with zipfile.ZipFile(io.BytesIO(response), 'r') as zipped:
                zipped.extractall(DOWNLOAD_DIR)

        else:
            # Tarfile does not support bytes IO, so use temp file
            try:
                with open("temp", "wb") as tar:
                    tar.write(response)

                with tarfile.open("temp", 'r:gz') as tarred:
                    tarred.extractall(DOWNLOAD_DIR)
            finally:
                if os.path.exists("temp"):
                    os.remove("temp")

        os.rename(f"{download_path}-{version}", download_path)
        print(f"Finished downloading {name}")

    return copy_sdl_dll(download_path)


def compile_gpp(src, output, srcflag="-c ", cflags=()):
    """
    Used to execute g++ commands
    """
    cmd = f"g++ -o {output} {srcflag}{src} {' '.join(cflags)}"
    print(cmd)
    return os.system(cmd)


def build_sources(builddir, testmode, cflags):
    """
    Generate obj files from source files
    """
    isfailed = 0
    globpattern = "test/**/test_*.cpp" if testmode else "src/**/*.cpp"

    for file in glob.iglob(globpattern, recursive=True):
        ofile = f"{builddir}/{os.path.basename(file)}".replace(".cpp", ".o")
        if should_build(file, ofile):
            print("\nBuilding file:", file)
            isfailed = compile_gpp(file, ofile, cflags=cflags)
            if isfailed:
                print("g++ command exited with an error code:", isfailed)

    if isfailed:
        print("Skipped building executable, because all files didn't build")
        sys.exit(isfailed)


def build_exe(builddir, distdir, cflags, testmode=False):
    """
    Generate final exe.
    """
    if COMPILER == "MSVC":
        build_file = "KithareTest.vcxproj" if testmode else "Kithare.vcxproj"
        ecode = os.system(
            f"msbuild /m /p:Configuration={MSVC_CONFIG} " + \
            f"/p:Platform={MACHINE} {build_file}"
        )

        if ecode:
            sys.exit(ecode)
        return

    build_sources(builddir, testmode, cflags)

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

    args = " ".join(objfiles).replace("\\", "/")

    icores = "icon.res"
    if not testmode and COMPILER == "MinGW":
        os.system(f"windres assets/Kithare.rc -O coff -o {icores}")
        args += f" {icores}"

    if os.path.exists(exepath):
        dist_m = os.stat(exepath).st_mtime

    for ofile in objfiles:
        if not os.path.exists(exepath) or os.stat(ofile).st_mtime > dist_m:
            print("\nBuilding exe")

            ecode = compile_gpp(args, exepath, "", cflags=cflags)
            if ecode:
                sys.exit(ecode)
            break

    if os.path.exists(icores):
        os.remove(icores)


def init_cflags():
    """
    Return an initialized cflags instance
    """
    cflags = ["-O3", "-std=c++14", "-I include"]
    cflags.extend(
        [
            "-lSDL2", "-lSDL2main", "-lSDL2_image", "-lSDL2_ttf",
            "-lSDL2_mixer", "-lSDL2_net"
        ]
    )

    if COMPILER == "MinGW":
        cflags.append("-municode")

    if IS_32_BIT and "-m32" not in sys.argv:
        cflags.append("-m32")

    cflags.extend(sys.argv[1:])
    return cflags


def main():
    """
    Main code, that runs on startup
    """
    build_tests = "--build-tests" in sys.argv
    if build_tests:
        sys.argv.remove("--build-tests")

    run_tests = "--run-tests" in sys.argv
    if run_tests:
        sys.argv.remove("--run-tests")

    cflags = init_cflags()

    if COMPILER == "MSVC":
        distdir = f"dist/MSVC-{MACHINE}-{MSVC_CONFIG}"
        builddir = ""  # For a dummy argument, that goes unused
    else:
        builddir = f"build/{COMPILER}-{MACHINE}"
        distdir = f"dist/{COMPILER}-{MACHINE}"
        mkdir(builddir)
        mkdir(distdir)

    if run_tests:
        sys.exit(os.system(os.path.normpath(f"{distdir}/{TEST_EXE}")))

    # Prepare dependencies and cflags with SDL flags
    if platform.system() == "Windows":
        mkdir(DOWNLOAD_DIR)

        for package, ver in SDL_DEPS.items():
            cflags.extend(download_sdl_deps(package, ver))

    else:
        for inc_dir in ["/usr/include/SDL2", "/usr/local/include/SDL2"]:
            if os.path.isdir(inc_dir):
                cflags.append(f"-I {inc_dir}")
                break

    # Building in only dependency install mode
    if MSVC_NO_COMPILE:
        return

    build_exe(builddir, distdir, cflags)
    if build_tests:
        build_exe(builddir, distdir, cflags, testmode=True)

    print("Done!")


if __name__ == "__main__":
    main()
