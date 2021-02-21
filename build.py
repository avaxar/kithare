"""
Builder script to build Kithare.

On Windows and MINGW:
    You must have MINGW installed, and 'g++' command must be on PATH.

    By default, this compiles for 64-bit architectures. If you want to compile for
    32-bit architecture and have a 32-bit mingw, pass '--32' as an argument to 
    the build script.

    This builder automatically installs SDL dependencies. Just run:
    'py build.py'

On Windows and MSVC:
    Because MSVC has it's own setup for compilation, this builder has only one 
    job in this case, that is installing SDL dependencies. Just run:
    'py build.py --msvc'

On Other OS:
    This assumes you have GCC (and G++) installed. Also, you need to install SDL
    dependencies on your own, via your systems package manager.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and 'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared 
    libraries.
"""

import glob
import hashlib
import io
import os
import platform
import shutil
import sys
import urllib.request as urllib
import zipfile
import tarfile

WIN = platform.system() == "Windows"

cflags = "-I include -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net"

if WIN:
    cflags += " -municode"

# Project, version pairs
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

IS_32_BIT = "--32" in sys.argv
IS_MSVC = "--msvc" in sys.argv

DOWNLOAD_DIR = f"deps/SDL-{'MSVC' if IS_MSVC else 'mingw'}"

if IS_MSVC:
    INC_DIR = "include"
    LIB_DIR = f"lib/{'x86' if IS_32_BIT else 'x64'}"
    DLL_DIR = LIB_DIR
    
else:
    _BASE_DIR = "i686-w64-mingw32" if IS_32_BIT else "x86_64-w64-mingw32"
    DLL_DIR = f"{_BASE_DIR}/bin"
    LIB_DIR = f"{_BASE_DIR}/lib"
    INC_DIR = f"{_BASE_DIR}/include"


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
                retfile = os.path.join("include", os.path.normcase(retfile.strip()))

                if os.path.isfile(retfile):
                    yield retfile
                    yield from find_includes(retfile)

            if cnt >= INC_FILE_LINE_LIMIT:
                break


def should_build(file, ofile):
    """
    Determines whether a particular file should be rebuilt or not
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
    global cflags

    name_2 = name.replace("2", "")
    suffix = "VC.zip" if IS_MSVC else "mingw.tar.gz"

    download_link = "https://www.libsdl.org/"
    if name == "SDL2":
        download_link += f"release/SDL2-devel-{version}-{suffix}"
    else:
        download_link += f"projects/{name_2}/release/{name}-devel-{version}-{suffix}"

    renamed_path = f"{DOWNLOAD_DIR}/{name}"

    incdir = f"{renamed_path}/{INC_DIR}"
    libdir = f"{renamed_path}/{LIB_DIR}"
    dlldir = f"{renamed_path}/{DLL_DIR}"

    cflags += f" -I {incdir} -L {libdir}"

    if os.path.isdir(renamed_path):
        print(f"Skipping {name} download because it already exists")
        return

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
        
        os.rename(f"{renamed_path}-{version}", renamed_path)
        
        # The headers are not correctly arranged here, so move to desired
        # location
        mkdir(f"{incdir}/SDL2")
        for i in glob.iglob(f'{incdir}/*'):
            if os.path.isfile(i):
                shutil.move(i, f"{incdir}/SDL2/{os.path.basename(i)}")
    else:
        # Tarfile does not support bytes IO, so use temp file
        with open("temp", "wb") as f:
            f.write(response)

        with tarfile.open("temp", 'r:gz') as tarred:
            tarred.extractall(DOWNLOAD_DIR)
        os.remove("temp")
        os.rename(f"{renamed_path}-{version}", renamed_path)

    # Copy all SDL DLLs to dist dir
    for dll in glob.iglob(f"{dlldir}/*.dll"):
        shutil.copyfile(dll, os.path.join("dist", os.path.basename(dll)))

    print(f"Download of {name} complete")


def compile_obj(src, output):
    """
    Used to compile a cpp file to an object file
    """
    cmd = f"g++ -c {src} -o {output} {cflags}"
    print(cmd)
    os.system(cmd)


def compile_all(out):
    """
    Used to compile all object files to the final exe
    """
    joined = ' '.join(glob.iglob("build/**/*.o", recursive=True))
    cmd = f"g++ {joined} -o {out} {cflags}"
    print(cmd)
    os.system(cmd)


def main():
    """
    Main code, that runs on startup
    """
    mkdir("dist")
    mkdir("build")
    if WIN:
        mkdir(DOWNLOAD_DIR)
        for package, ver in SDL_DEPS.items():
            download_sdl_deps(package, ver)

        if IS_MSVC:
            return
    
    for file in glob.iglob("src/**/*.cpp", recursive=True):
        ofile = file.replace("src", "build", 1) + ".o"
        if should_build(file, ofile):
            mkdir(os.path.dirname(ofile))
            print("Building file:", file)
            compile_obj(file, ofile)
    
    print("Building exe")
    compile_all("dist/kcr")
    print("Done!")


if __name__ == "__main__":
    main()
