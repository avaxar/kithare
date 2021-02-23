"""
Builder script to build Kithare.

On Windows and MinGW:
    You must have MinGW (AKA MinGW-w64) installed, and 'g++' command must
    be on PATH.

    If you are on a 64-bit system, and want to compile for 32-bit architecture,
    pass '-m32' as an argument to the build script.

    This builder automatically installs SDL dependencies. Just run this file
    with: 'py build.py'.

On Windows and MSVC:
    Because MSVC has its own setup for compilation, this builder has only one 
    job in this case, that is installing SDL dependencies. Just run:
    'py build.py --msvc'

On Mac:
    (Unfinished as of now)
    This assumes you have clang (clang++) installed. Also, you need to install 
    SDL dependencies on your own.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and 'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared 
    libraries.

On other OS:
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
import io
import os
import platform
import shutil
import sys
import tarfile
import urllib.request as urllib
import zipfile

EXE = "kcr.exe" if platform.system() == "Windows" else "kcr"

# While we recursively search for include files, we don't want to seach
# the whole file, because that would waste a lotta time. So, we just take
# an arbitrary line number limit, beyond which, we won't search
INC_FILE_LINE_LIMIT = 26

# SDL project-version pairs, remember to keep updated
SDL_DEPS = {
    "SDL2": "2.0.14",
    "SDL2_image": "2.0.5",
    "SDL2_mixer": "2.0.4",
    "SDL2_ttf": "2.0.15",
    "SDL2_net": "2.0.1",
}

is_32_bit_req = "-m32" in sys.argv

_machine = platform.machine()
if _machine in ["i686", "i386"]:
    machine = "x86"
    machine_alt = "i686"

elif _machine.lower() in ["x86_64", "amd64"]:
    machine = "x86" if is_32_bit_req else "x64"
    machine_alt = "i686" if is_32_bit_req else "x86_64"

elif _machine.lower() in ["arm", "arm32", "armv6l", "armv7l"]:
    machine = "ARM"

elif _machine.lower() in ["armv8l", "arm64", "aarch64"]:
    machine = "ARM" if is_32_bit_req else "ARM64"

else:
    machine = _machine if _machine else "None"

if platform.system() == "Windows":
    compiler = "MSVC" if "--msvc" in sys.argv else "MinGW"

else:
    compiler = "Clang" if platform.system() == "Darwin" else "GCC"

cc = "clang++" if compiler == "Clang" else "g++"

cflags = "-O3 -I include"
cflags += " -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net"

if compiler == "MinGW":
    cflags += " -municode"

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
                mkdir(f"build/MSVC-{i}-{j}")
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


def compile_gpp(src, output, srcflag=""):
    """
    Used to execute g++ commands
    """
    cmd = f"{cc} -o {output} {srcflag}{src} {cflags}"
    print(cmd)
    return os.system(cmd)


def main():
    """
    Main code, that runs on startup
    """
    global cflags

    builddir = f"build/{compiler}-{machine}"
    distdir = f"dist/{compiler}-{machine}"
    if compiler != "MSVC":
        mkdir(builddir)
        mkdir(distdir)

    # Prepare dependencies
    if platform.system() == "Windows":
        mkdir(download_dir)

        for package, ver in SDL_DEPS.items():
            cflags += download_sdl_deps(package, ver)

        if compiler == "MSVC":
            print("Done!")
            return

    elif platform.system() == "Darwin":
        # TODO: Include SDL directories properly on this one
        pass

    else:
        for inc_dir in ["/usr/include/SDL2", "/usr/local/include/SDL2"]:
            if os.path.isdir(inc_dir):
                cflags += f" -I {inc_dir}"
                break
    
    print()
    isfailed = False
    for file in glob.iglob("src/**/*.cpp", recursive=True):
        ofile = f"{builddir}/{os.path.basename(file)}".replace(".cpp", ".o")
        if should_build(file, ofile):
            print("Building file:", file)
            if compile_gpp(file, ofile, "-c "):
                print("g++ command exited with an error")
                isfailed = True
            print()
    
    if isfailed:
        print("Skipped building executable, because all files didn't build")
        return
    
    if not os.path.exists(f"{distdir}/{EXE}"):
        print("Building exe")
        compile_gpp(" ".join(glob.iglob(f"{builddir}/*.o")), f"{distdir}/{EXE}")

    else:
        dist_m = os.stat(f"{distdir}/{EXE}").st_mtime
        obj_files = glob.glob(f"{builddir}/*.o")

        for ofile in obj_files:
            if os.stat(ofile).st_mtime > dist_m:
                print("Building exe")
                compile_gpp(" ".join(obj_files), f"{distdir}/{EXE}")
                break
    print("Done!")


if __name__ == "__main__":
    main()
