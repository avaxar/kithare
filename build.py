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
    This assumes you have GCC installed. Also, you need to install SDL
    dependencies on your own, via your systems package manager.
    Kithare needs 'SDL2', 'SDL2_mixer', 'SDL2_image', 'SDL2_ttf' and
    'SDL2_net'.
    Make sure to install 'devel' releases of those, not just runtime shared
    libraries.

    A recommended and easy way to do this on MacOS, is via homebrew. Just run
    `brew install gcc sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf`.

    And the build is really simple, just run 'python3 build.py'

If you are on a 64-bit system, and want to compile for 32-bit architecture,
pass '--arch=x86' (or -m32) as an argument to the build script (note that this
might not work in some cases)

By default, the builder uses all cores on the machine to build Kithare. But if
you want the builder to consume less CPU power while compiling (at the cost of
longer compile times), you can use the '-j' flag to set the number of cores you
want the builder to use. '-j1' means that you want to use only one core, '-j4'
means that you want to use 4 cores, and so on.

To just run tests, do 'python3 build.py test'. Note that this command is only
going to run the tests, it does not do anything else.

'python3 build.py clean' deletes folders that contain generated executable(s)
and temporary build files that are cached for performance reasons. In normal
usage one need not run this command, but in cases like change in version of
compiler and/or deps, one needs to run this command before installation.

Additionally on Windows, one can run 'python3 build.py cleandep' to delete the
installed SDL dependencies. Note that this command is not required at all in
normal usage, and has been only provided for completeness sake, use only if
you know what you are doing.

Any other arguments passed to this builder will be forwarded to the compiler.
"""


import io
import json
import os
import platform
import queue
import shutil
import stat
import subprocess
import sys
import tarfile
import threading
import time
import urllib.request as urllib
from functools import lru_cache
from json.decoder import JSONDecodeError
from pathlib import Path
from typing import Optional, Sequence, Union

INCLUDE_DIRNAME = "include"
ICO_RES = "icon.res"

SUPPORTED_ARCHS = {"x86", "x64", "ARM", "ARM64"}

# Make this False, when building with C source
CPP = True

STD_FLAG = "c++14" if CPP else "c99"
COMPILER_NAME = "g++" if CPP else "gcc"
SOURCE_GLOB = "src/**/*.cpp" if CPP else "src/**/*.c"

INIT_TEXT = """Kithare Programming Language
----------------------------
An open source general purpose statically-typed cross-platform
interpreted/transpiled C++/Python like programming language.

The source code for Kithare programming language is distributed
under the MIT license.
Copyright (C) 2021 Kithare Organization

Github: https://github.com/Kithare/Kithare
Website: https://kithare.cf/Kithare/

Building Kithare...
"""

KITHARE_ISSUES_LINK = "https://github.com/Kithare/Kithare/issues"

COMPILER = "MinGW" if platform.system() == "Windows" else "GCC"
EXE = "kcr"
if COMPILER == "MinGW":
    EXE += ".exe"

    if sys.version_info < (3, 8):
        # Because there is a pathlib + subprocess bug on py < 3.8 on windows
        raise RuntimeError("Kithare builder needs atleast Python v3.8 on Windows")

# While we recursively search for include files, we don't want to seach
# the whole file, because that would waste a lotta time. So, we just take
# an arbitrary line number limit, beyond which, we won't search
INC_FILE_LINE_LIMIT = 75

# Downloads timeout in seconds
DOWNLOAD_TIMEOUT = 600
TIMEOUT_PER_LOOP = 0.05

# SDL project-version pairs, remember to keep updated
SDL_DEPS = {
    "SDL2": "2.0.14",
    "SDL2_image": "2.0.5",
    "SDL2_mixer": "2.0.4",
    "SDL2_ttf": "2.0.15",
    "SDL2_net": "2.0.1",
}


class BuildError(Exception):
    """
    Exception class for all build related exceptions
    """

    def __init__(self, emsg: str = "", ecode: int = 1):
        """
        Initialise exception object
        """
        super().__init__(self, emsg)
        self.emsg = emsg
        self.ecode = ecode


def run_cmd(*cmds: Union[str, Path], strict: bool = False, silent_cmds: bool = False):
    """
    Helper function to run command in subprocess.
    Prints the command, command output, and error exit code (if nonzero), and
    also returns the exit code. If strict arg is given True, BuildError is
    raised rather than printing the error.
    """
    if not silent_cmds:
        print(*cmds)

    # run with subprocess
    try:
        proc = subprocess.run(
            cmds,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            check=False,
        )

    except FileNotFoundError:
        emsg = f"The command '{cmds[0]}' was not found"
        if strict:
            raise BuildError(emsg) from None

        print("BuildError:", emsg)
        return 1

    print(proc.stdout, end="")
    if proc.returncode:
        emsg = f"'{cmds[0]}' command failed with exit code {proc.returncode}"
        if abs(proc.returncode) > 9:
            emsg += f" ({hex(proc.returncode)})"

        if strict:
            raise BuildError(emsg, proc.returncode)

        print("BuildError:", emsg)

    return proc.returncode


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
    Determines whether a particular source file should be rebuilt
    """
    try:
        # a file, or an included file was newer than the object file
        return find_includes_max_time(file, incdir) > ofile.stat().st_mtime
    except FileNotFoundError:
        return True


def rmtree(top: Path):
    """
    Reimplementation of shutil.rmtree, used to remove a directory. Returns a
    boolean on whether top was a directory or not (in the latter case this
    function does nothing). This function may raise BuildErrors on any internal
    OSErrors that happen.
    The reason shutil.rmtree itself is not used, is of a permission error on
    Windows.
    """
    if not top.is_dir():
        return False

    try:
        for newpath in top.iterdir():
            if not rmtree(newpath):
                # could not rmtree newpath because it is a file, hence unlink
                newpath.chmod(stat.S_IWUSR)
                newpath.unlink()

        top.rmdir()
        return True
    except OSError:
        raise BuildError(f"Could not delete directory '{top}'") from None


def copy(file: Path, dest: Path, overwrite: bool = True):
    """
    Thin wrapper around shutil.copy, raises BuildError if the copy failed.
    Also, overwrite arg is a bool that indicates whether the file is
    overwritten if it already exists
    """
    if not overwrite and (dest / file.name).exists():
        return

    try:
        shutil.copy(file, dest)
    except OSError:
        raise BuildError(
            f"Could not copy file '{file}' to '{dest}' directory"
        ) from None


def get_machine(is_32_bit: bool):
    """
    Utility to get string representation of the machine name
    """
    machine = platform.machine()
    if machine.endswith("86"):
        machine = "x86"

    elif machine.lower() in {"x86_64", "amd64"}:
        machine = "x86" if is_32_bit else "x64"

    elif machine.lower() in {"armv8l", "arm64", "aarch64"}:
        machine = "ARM" if is_32_bit else "ARM64"

    elif machine.lower().startswith("arm"):
        machine = "ARM"

    elif not machine:
        machine = "Unknown"

    return machine


def get_cpu_count():
    """
    Get number of logical cores on CPU, thin wrapper on os.cpu_count but does
    not return None (returns 1 instead)
    """
    ret = os.cpu_count()
    if ret is None:
        return 1

    return ret


class ThreadedDownloader:
    """
    Install file(s) concurrently using threads
    """

    def __init__(self):
        """
        Initialise ThreadedDownloader object
        """

        self.threads: set[threading.Thread] = set()
        self.downloaded: queue.Queue[tuple[str, bytes]] = queue.Queue()

    def _download_thread(self, name: str, download_link: str):
        """
        Download a file/resource on a seperate thread
        """
        print(f"Downloading {name} from {download_link}")
        request = urllib.Request(
            download_link,
            headers={"User-Agent": "Chrome/35.0.1916.47 Safari/537.36"},
        )
        try:
            with urllib.urlopen(request) as downloadobj:
                download: bytes = downloadobj.read()

        except OSError:
            # some networking error
            print(f"Failed to download {name} due to some networking error")

        else:
            self.downloaded.put((name, download))

    def download(self, name: str, download_link: str):
        """
        Download a file
        """
        thread = threading.Thread(
            target=self._download_thread, args=(name, download_link), daemon=True
        )
        thread.start()
        self.threads.add(thread)

    def is_downloading(self):
        """
        Check whether a file is being downloaded
        """
        return any(t.is_alive() for t in self.threads)

    def get_finished(self):
        """
        Iterate over downloaded resources (name-path pairs). Blocks while
        waiting for threads to complete.
        """
        loops = 0
        while self.is_downloading():
            try:
                # Do timeout and loop because we need be able to handle any
                # potential KeyboardInterrupt errors
                yield self.downloaded.get(timeout=TIMEOUT_PER_LOOP)
            except queue.Empty:
                pass

            loops += 1
            if loops * TIMEOUT_PER_LOOP > DOWNLOAD_TIMEOUT:
                print(
                    "Download(s) timed out!\n"
                    f"Took longer than {DOWNLOAD_TIMEOUT} seconds.\n"
                    "Skipping download(s), continuing with compilation..."
                )
                break


class DummySDLInstaller:
    """
    Dummy SDLInstaller class, with limited functionality. This class is kept
    for compatability with non-windows OSes
    """

    def __init__(self):
        """
        Initialise DummySDLInstaller class
        """
        self.lflags: list[str] = []

    def clean(self):  # pylint: disable=no-self-use
        """
        Clean install, this is not available on non-windows
        """
        raise BuildError("The flag 'cleandep' is not supported on your OS")

    def install_all(self):
        """
        Utility function to install all SDL deps. In this dummy installer, only
        updates lflags to link with already installed SDL
        """
        for name in SDL_DEPS:
            self.lflags.append(f"-l{name}")
            if name == "SDL2":
                self.lflags.append("-lSDL2main")

        return ""


class SDLInstaller(DummySDLInstaller):
    """
    Helper class to install SDL deps on MinGW
    """

    def __init__(self, basepath: Path, dist_dir: Path, machine: str):
        """
        Initialise SDLInstaller class
        """
        super().__init__()
        self.sdl_dir = basepath / "deps" / "SDL"
        self.sdl_include = self.sdl_dir / "include" / "SDL2"

        self.dist_dir = dist_dir
        self.downloader = ThreadedDownloader()

        if machine == "x64":
            self.sdl_type = "x86_64-w64-mingw32"
        elif machine == "x86":
            self.sdl_type = "i686-w64-mingw32"
        elif machine.startswith("ARM"):
            raise BuildError("Windows builds are not supported yet on ARM CPU")
        else:
            raise BuildError(
                "Your CPU type could not be determined by the builder. If you "
                "are seeing this message, report it to Kithare bug tracker."
            )

    def clean(self):
        """
        Clean (remove) SDL install directory
        """
        rmtree(self.sdl_dir)

    def _prepare_install(self):
        """
        Prepare for install, delete any outdated installs, prepare download
        path.
        """
        if self.sdl_dir.is_dir():
            # delete old SDL version installations, if any
            saved_dirs = {f"{n}-{v}" for n, v in SDL_DEPS.items()}
            saved_dirs.add("include")
            for subdir in self.sdl_dir.iterdir():
                if subdir.name not in saved_dirs:
                    print(f"Deleting old SDL install: '{subdir.name}'")
                    rmtree(subdir)

        # make SDL include dir
        self.sdl_include.mkdir(parents=True, exist_ok=True)

    def _download_dep(self, name: str, ver: str):
        """
        Download an SDL dep, uses ThreadedDownloader to download in background.
        Return a two element tuple, first one indicating whether download was
        skipped, second Path object to downloaded dir
        """
        sdl_mingw_dep = self.sdl_dir / f"{name}-{ver}" / self.sdl_type
        if sdl_mingw_dep.is_dir():
            print(f"Skipping {name} download because it already exists")
            return True, sdl_mingw_dep

        download_link = "https://www.libsdl.org/"
        if name != "SDL2":
            download_link += f"projects/{name}/".replace("2", "")

        download_link += f"release/{name}-devel-{ver}-mingw.tar.gz"
        self.downloader.download(name, download_link)
        return False, sdl_mingw_dep

    def _extract(self, name: str, downloaddata: bytes, downloaded_path: Path):
        """
        Extract downloaded dep into SDL deps folder, return bool on whether
        extraction succeeded or not
        """
        try:
            with io.BytesIO(downloaddata) as fileobj:
                with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
                    tarred.extractall(self.sdl_dir)

        except (tarfile.TarError, OSError):
            # some error while extracting
            print(f"Failed to extract tarfile of {name} while downloading")
            rmtree(downloaded_path.parent)  # clean failed download
            return False

        print(f"Finished downloading {name}")

        # Copy includes
        for header in downloaded_path.glob("include/SDL2/*.h"):
            copy(header, self.sdl_include)

        return True

    def _copy_dll(self, path: Path, overwrite: bool = True):
        """
        Copy DLLs from downloaded path and update lflags with libpath
        """
        # Copy DLLs that have not been copied already
        for dll in path.glob("bin/*.dll"):
            copy(dll, self.dist_dir, overwrite)

        self.lflags.append(f"-L{path / 'lib'}")

    def _install(self, *skipped_downloads: Path, **downloads: Path):
        """
        Install SDL dependencies. Returns whether the install was successful
        """
        for download in skipped_downloads:
            self._copy_dll(download, False)

        failed: bool = False
        for name, downloaddata in self.downloader.get_finished():
            if not self._extract(name, downloaddata, downloads[name]):
                # download failed, remove from downloads
                downloads.pop(name)
                failed = True

        for path in downloads.values():
            self._copy_dll(path)

        return not failed

    def install_all(self):
        """
        Utility function to install all SDL deps. Deletes any old SDL install,
        and downloads the deps concurrently, and returns a two element tuple,
        first being flag for SDL include, and second is a list of SDL linking
        linker flags.
        """
        print("Configuring SDL dependencies...")
        print("Any missing dependencies will be downloaded")
        print("This might take a while, depending on your internet speeds")

        self._prepare_install()
        super().install_all()

        downloads: dict[str, Path] = {}
        skipped_downloads: list[Path] = []

        # Download SDL deps if unavailable, use threading to download deps
        # concurrently
        for name, ver in SDL_DEPS.items():
            skipped, path = self._download_dep(name, ver)
            if skipped:
                skipped_downloads.append(path)
            else:
                downloads[name] = path

        if not self._install(*skipped_downloads, **downloads):
            print(
                "These error(s) will be ignored for now, but may cause build "
                "errors later"
            )

        print()  # newline
        return f"-I{self.sdl_include.parent}"


class CompilerPool:
    """
    A pool of C++ files to be compiled in multiple subprocesses
    """

    def __init__(self, maxpoolsize: Optional[int], *cflags: str):
        """
        Initialise CompilerPool instance. maxpoolsize is the limit on number of
        subprocesses that can be opened at a given point. If not specified
        (None), defaults to number of cores on the machine. The varargs cflags
        are compiler flags to be passed
        """
        self.cflags = cflags

        self._procs: dict[Path, subprocess.Popen[str]] = {}
        self._queued_procs: list[tuple[Path, Path]] = []
        self.failed: bool = False

        self.maxpoolsize = get_cpu_count() if maxpoolsize is None else maxpoolsize

    def _start_proc(self, cfile: Path, ofile: Path):
        """
        Internal function to start a compile subprocess
        """
        args: list[Union[str, Path]] = [COMPILER_NAME, "-o", ofile, "-c", cfile]
        args.extend(self.cflags)

        # pylint: disable=consider-using-with
        self._procs[cfile] = subprocess.Popen(
            args,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        )

    def _finish_proc(self, proc: subprocess.Popen):
        """
        Take a finished Popen subprocess, print command name, stdout and
        error code
        """
        # stderr is redirected to stdout here
        stdout, _ = proc.communicate()
        if isinstance(proc.args, bytes):
            print(proc.args.decode())
        elif isinstance(proc.args, Sequence):
            print(*proc.args)
        else:
            print(proc.args)

        if proc.returncode:
            print(stdout, end="")
            print(f"g++ exited with error code: {proc.returncode}\n")
            self.failed = True
        else:
            print(stdout)

    def update(self):
        """
        Runs an "update" operation. Any processes that have been finished are
        removed from the process pool after the subprocess output has been
        printed along with the return code (on error). Starts a new subprocess
        from the queued pending process pool.
        """
        for cfile, proc in tuple(self._procs.items()):
            if proc.poll() is None:
                # proc is still running
                continue

            print(f"Building file: {cfile}")
            self._finish_proc(proc)
            self._procs.pop(cfile)

            # start a new process from queued process
            if self._queued_procs:
                self._start_proc(*self._queued_procs.pop())

    def add(self, cfile: Path, ofile: Path):
        """
        Add a source file to be compiled into the compiler pool. cfile is the
        Path object to the source file, while ofile is the Path object to the
        output file
        """
        if len(self._procs) >= self.maxpoolsize:
            # pool is full, queue the command
            self._queued_procs.append((cfile, ofile))
        else:
            self._start_proc(cfile, ofile)

        # call an update
        self.update()

    def poll(self):
        """
        Returns False when all files in the pool finished compiling, True
        otherwise
        """
        return bool(self._queued_procs or self._procs)

    def wait(self):
        """
        Block until all queued files are compiled
        """
        while self.poll():
            self.update()
            time.sleep(0.005)


class KithareBuilder:
    """
    Kithare builder class
    """

    def __init__(self, basepath: Path, *args: str):
        """
        Initialise kithare builder
        """
        self.basepath = basepath

        is_32_bit = "--arch=x86" in args or "-m32" in args
        machine = get_machine(is_32_bit)

        # debug mode for the builder
        debug = False
        if args and args[0] == "debug":
            debug = True
            args = args[1:]

        dirname = f"{COMPILER}-Debug" if debug else f"{COMPILER}-{machine}"
        self.builddir = self.basepath / "build" / dirname
        self.exepath = self.basepath / "dist" / dirname / EXE

        self.sdl_installer = (
            SDLInstaller(basepath, self.exepath.parent, machine)
            if COMPILER == "MinGW"
            else DummySDLInstaller()
        )

        if args:
            self._handle_first_arg(args[0])

        print(INIT_TEXT)
        print("Platform:", platform.platform())
        print("Compiler:", COMPILER, STD_FLAG)
        print("Builder Python version:", platform.python_version())

        if machine in SUPPORTED_ARCHS:
            print("Machine:", machine)

        else:
            print(
                "BuildWarning: Your CPU arch has been determined to be "
                f"'{machine}'\nNote that this is not well supported. "
                "If you think this is a Kithare builder bug, report it on "
                "the bug tracker"
            )

        print("Additional compiler info:")
        run_cmd(COMPILER_NAME, "--version", strict=True, silent_cmds=True)

        # compiler flags
        self.cflags = [
            "-g" if debug else "-O3",  # no -O3 on debug mode
            f"-std={STD_FLAG}",
            f"-I{basepath / INCLUDE_DIRNAME}",
        ]

        if COMPILER == "MinGW":
            self.cflags.append("-municode")

        if is_32_bit and "-m32" not in args:
            self.cflags.append("-m32")

        self.j_flag: Optional[int] = None

        # update compiler flags with more args
        for i in args:
            if i.startswith("-j"):
                if self.j_flag is not None:
                    # -j specified again
                    raise BuildError("Argument '-j' can only be specified once")

                try:
                    self.j_flag = int(i[2:])
                    if self.j_flag <= 0:
                        raise ValueError()

                except ValueError:
                    raise BuildError(
                        "Argument '-j' must be a positive integer"
                    ) from None

            elif not i.startswith("--arch="):
                self.cflags.append(i)

    def _handle_first_arg(self, arg: str):
        """
        Utility method to handle the first argument
        """
        if arg == "test":
            sys.exit(run_cmd(self.exepath, "--test"))

        if arg == "clean":
            for dist in {self.builddir, self.exepath.parent}:
                rmtree(dist.parent)
            sys.exit(0)

        if arg == "cleandep":
            self.sdl_installer.clean()
            sys.exit(0)

    def build_sources(self, build_skippable: bool):
        """
        Generate obj files from source files, returns a list of generated
        objfiles. May also return None if all older objfiles are up date and
        dist exe already exists.
        """
        skipped_files: list[Path] = []
        objfiles: list[Path] = []

        print("Building Kithare sources...")
        compilerpool = CompilerPool(self.j_flag, *self.cflags)

        print(f"Building on {min(compilerpool.maxpoolsize, get_cpu_count())} core(s)")
        if compilerpool.maxpoolsize > get_cpu_count():
            print(f"Using {compilerpool.maxpoolsize} subprocess(es)")

        print()  # newline
        for file in self.basepath.glob(SOURCE_GLOB):
            ofile = self.builddir / f"{file.stem}.o"
            if ofile in objfiles:
                raise BuildError("Got duplicate filename in Kithare source")

            objfiles.append(ofile)
            if build_skippable and not should_build(
                file, ofile, self.basepath / INCLUDE_DIRNAME
            ):
                skipped_files.append(file)
                continue

            compilerpool.add(file, ofile)

        compilerpool.wait()
        if skipped_files:
            if len(skipped_files) == 1:
                print(f"Skipping file {skipped_files[0]}")
                print("Because the intermediate object file is already built\n")
            else:
                print("Skipping files:")
                print(*skipped_files, sep="\n")
                print("Because the intermediate object files are already built\n")

        if compilerpool.failed:
            raise BuildError(
                "Skipped building executable, because all files didn't build"
            )

        if len(objfiles) == len(skipped_files) and self.exepath.is_file():
            # exe is already up to date
            return None

        return objfiles

    def build_exe(self):
        """
        Generate final exe.
        """
        # load old cflags from the previous build
        build_conf = self.builddir / "build_conf.json"
        try:
            old_cflags = json.loads(build_conf.read_text())
        except (FileNotFoundError, JSONDecodeError):
            old_cflags = []

        new_cflags = self.cflags.copy()
        new_cflags.extend(self.sdl_installer.lflags)

        # because order of args should not matter here
        build_skippable = sorted(new_cflags) == sorted(old_cflags)
        if not build_skippable:
            # update conf file with latest cflags
            build_conf.write_text(json.dumps(new_cflags))

        objfiles = self.build_sources(build_skippable)
        if objfiles is None:
            print("Skipping final exe build, since it is already built")
            return

        # Handle exe icon on MinGW
        ico_res = self.basepath / ICO_RES
        if COMPILER == "MinGW":
            assetfile = self.basepath / "assets" / "Kithare.rc"

            print("Running windres command to set icon for exe")
            ret = run_cmd("windres", assetfile, "-O", "coff", "-o", ico_res)
            if ret:
                print("This means the final exe will not have the kithare logo")
            else:
                objfiles.append(ico_res)

            print()  # newline

        print("Building executable")
        try:
            run_cmd(
                COMPILER_NAME, "-o", self.exepath, *objfiles, *new_cflags, strict=True
            )
        finally:
            # delete icon file
            if ico_res.is_file():
                ico_res.unlink()

    def build(self):
        """
        Build Kithare
        """
        # prepare directories
        self.builddir.mkdir(parents=True, exist_ok=True)
        self.exepath.parent.mkdir(parents=True, exist_ok=True)

        t_1 = time.perf_counter()
        # Prepare dependencies and cflags with SDL flags
        incflag = self.sdl_installer.install_all()
        if incflag:
            self.cflags.append(incflag)

        t_2 = time.perf_counter()
        self.build_exe()
        print("Done!\nKithare has been built successfully!\n")

        t_3 = time.perf_counter()

        # display stats
        print("Some timing stats for peeps who like to 'optimise':")

        # print SDL install time stats only if it is large enough (haha majik number)
        if t_2 - t_1 > 0.42:
            print(f"SDL deps took {t_2 - t_1:.3f} seconds to configure and install")

        print(f"Kithare took {t_3 - t_2:.3f} seconds to compile")


def main():
    """
    Invoke Kithare builder class with arguments and build Kithare
    """
    err_code = 0
    try:
        argv = sys.argv.copy()
        dname = Path(argv.pop(0)).parent

        kithare = KithareBuilder(dname, *argv)
        kithare.build()

    except BuildError as err:
        err_code = err.ecode
        if err.emsg:
            print("BuildError:", err.emsg)

    except Exception as err:  # pylint: disable=broad-except
        print(
            "Unknown exception occured! This is probably a bug in the build "
            "script itself. Report this bug to Kithare devs, along with the "
            f"whole buildlog here ({KITHARE_ISSUES_LINK}).\n"
            "Reraising error:"
        )
        raise

    except KeyboardInterrupt:
        err_code = 1
        print("Compilation was terminated with Keyboard Interrupt")

    print(
        "\nFor any bug reports or feature requests, check out the issue "
        f"tracker at github\n{KITHARE_ISSUES_LINK}"
    )
    sys.exit(err_code)


if __name__ == "__main__":
    main()
