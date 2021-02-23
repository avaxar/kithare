"""
Builder script to build Kithare.

On Windows and MINGW:
    You must have MINGW (AKA MinGW-w64) installed, and 'g++' command must
    be on PATH.

    By default, this compiles for 64-bit architectures. If you want to compile
    for 32-bit architecture, pass '-m32' as an argument to the build script.

    This builder automatically installs SDL dependencies. Just run this file
    with: 'py build.py'.

On Windows and MSVC:
    Because MSVC has its own setup for compilation, this builder has only one 
    job in this case, that is installing SDL dependencies. Just run:
    'py build.py --msvc'

On Other OS:
    This assumes you have GCC (and G++) installed. Also, you need to install SDL
    dependencies on your own, via your systems package manager.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and 'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared 
    libraries.

Note that any arguments passed to this builder will be forwarded to the 
compiler.

This feature might fall of use for advanced users, who know what they are
doing.
"""

import glob
import hashlib
import io
import os
import platform
import shutil
import sys
import tarfile
import urllib.request as urllib
import zipfile

WIN = platform.system() == "Windows"

DIST = "dist/kcr"
if WIN:
    DIST += ".exe"

cflags = "-O3 -I include"
cflags += " -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net"

if WIN:
    cflags += " -municode"

# SDL project-version pairs, remember to keep updated
SDL_DEPS = {
    "SDL2": "2.0.14",
    "SDL2_image": "2.0.5",
    "SDL2_mixer": "2.0.4",
    "SDL2_ttf": "2.0.15",
    "SDL2_net": "2.0.1",
}

# While we recursively search for include files, we don't want to seach
# the whole file, because that would waste a lotta time. So, we just take
# an arbitrary line number limit, beyond which, we won't search
INC_FILE_LINE_LIMIT = 26

IS_32_BIT = "-m32" in sys.argv
IS_MSVC = "--msvc" in sys.argv

if not IS_32_BIT:
    IS_32_BIT = sys.maxsize == 2 ** 32 - 1

ARCH = 'x86' if IS_32_BIT else 'x64'

cflags += " " + " ".join(sys.argv[1:])

DOWNLOAD_DIR = f"deps/SDL-{'MSVC' if IS_MSVC else 'mingw'}"

if IS_MSVC:
    INC_DIR = "include"
    LIB_DIR = f"lib/{ARCH}"
    DLL_DIR = LIB_DIR
    
else:
    _GPP_ARCH = "i686" if IS_32_BIT else "x86_64"
    DLL_DIR = f"{_GPP_ARCH}-w64-mingw32/bin"
    LIB_DIR = f"{_GPP_ARCH}-w64-mingw32/lib"
    INC_DIR = f"{_GPP_ARCH}-w64-mingw32/include/SDL2"


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
                retfile = os.path.join("include", 
                    os.path.normcase(retfile.strip()))

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
    download_link += "VC.zip" if IS_MSVC else "mingw.tar.gz"

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
        if IS_MSVC:
            with zipfile.ZipFile(io.BytesIO(response), 'r') as zipped:
                zipped.extractall(DOWNLOAD_DIR)

        else:
            # Tarfile does not support bytes IO, so use temp file
            try:
                with open("temp", "wb") as f:
                    f.write(response)

                with tarfile.open("temp", 'r:gz') as tarred:
                    tarred.extractall(DOWNLOAD_DIR)
            finally:
                if os.path.exists("temp"):
                    os.remove("temp")
        
        os.rename(f"{download_path}-{version}", download_path)

        # Copy all SDL DLLs to dist dir
        for dll in glob.iglob(f"{download_path}/{DLL_DIR}/*.dll"):
            shutil.copyfile(dll, os.path.join("dist", os.path.basename(dll)))

        print(f"Finished downloading {name}")

    return f" -I {download_path}/{INC_DIR} -L {download_path}/{LIB_DIR}"


def compile_gpp(src, output, srcflag=""):
    """
    Used to execute g++ commands
    """
    cmd = f"g++ -o {output} {srcflag}{src} {cflags}"
    print(cmd)
    return os.system(cmd)


def main():
    """
    Main code, that runs on startup
    """
    global cflags

    mkdir(os.path.dirname(DIST))
    if WIN:
        mkdir(DOWNLOAD_DIR)
        for package, ver in SDL_DEPS.items():
            cflags += download_sdl_deps(package, ver)

        if IS_MSVC:
            print("Done!")
            return
    else:
        for inc_dir in ["/usr/include/SDL2", "/usr/local/include/SDL2"]:
            if os.path.isdir(inc_dir):
                cflags += f" -I {inc_dir}"
                break
    
    print()
    mkdir("build")
    isfailed = False
    for file in glob.iglob("src/**/*.cpp", recursive=True):
        ofile = file.replace("src", "build", 1).rstrip(".cpp") + f"-{ARCH}.o"
        if should_build(file, ofile):
            mkdir(os.path.dirname(ofile))
            print("Building file:", file)
            if compile_gpp(file, ofile, "-c "):
                print("g++ command exited with an error")
                isfailed = True
            print()
    
    if isfailed:
        print("Skipped building final executable, because all files didn't build")
        return
    
    if not os.path.exists(DIST):
        print("Building exe")
        compile_gpp(
            " ".join(
                glob.iglob(f"build/**/*-{ARCH}.o", recursive=True)
            ),
            DIST
        )

    else:
        dist_m = os.stat(DIST).st_mtime
        obj_files = glob.glob(f"build/**/*-{ARCH}.o", recursive=True)

        for ofile in obj_files:
            if os.stat(ofile).st_mtime > dist_m:
                print("Building exe")
                compile_gpp(" ".join(obj_files), DIST)
                break
    print("Done!")


if __name__ == "__main__":
    main()
