"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2022 Kithare Organization

builder/__init__.py
Defines the main KithareBuilder class that builds Kithare
"""


import platform
import shutil
import sys
import time
from pathlib import Path
from typing import Optional

from .cflags import CompilerFlags
from .compilerpool import CompilerPool
from .constants import (
    C_STD_FLAG,
    COMPILER,
    CPP_STD_FLAG,
    CPU_COUNT,
    EPILOG,
    EXE,
    ICO_RES,
    INCLUDE_DIRNAME,
    INIT_TEXT,
    SUPPORTED_ARCHS,
)
from .downloader import install_mingw
from .packaging import get_packager
from .sdl_installer import get_installer
from .utils import (
    BuildError,
    ConvertType,
    convert_machine,
    copy,
    get_machine,
    get_rel_path,
    parse_args,
    rmtree,
    run_cmd,
    should_build,
)

__all__ = ["EPILOG", "KithareBuilder", "BuildError"]


class KithareBuilder:
    """
    Kithare builder class
    """

    def __init__(self):
        """
        Initialise kithare builder
        """
        self.basepath = get_rel_path(Path(__file__).resolve().parents[1])

        is_32_bit, args = parse_args()
        machine = get_machine(is_32_bit)
        dirname = (
            f"{COMPILER}-Debug" if args.make == "debug" else f"{COMPILER}-{machine}"
        )

        self.builddir = self.basepath / "build" / dirname
        self.exepath = self.basepath / "dist" / dirname / EXE

        self.sdl_installer = get_installer(self.basepath, self.exepath.parent, machine)
        self.installer = get_packager(
            self.basepath,
            self.exepath,
            machine,
            args.release,
            args.use_alien,
            args.make == "installer",
        )

        self._handle_make_and_clean(args.make, args.clean)
        self.j_flag: Optional[int] = args.j
        if self.j_flag is not None and self.j_flag <= 0:
            raise BuildError("The '-j' flag should be a positive integer")

        # compiler flags
        self.cflags = CompilerFlags(self.basepath)

        self.t_0 = time.perf_counter()
        self.configure_compiler_and_flags(machine, is_32_bit, args.make)

    def _handle_make_and_clean(self, arg: Optional[str], clean: Optional[str]):
        """
        Utility method to handle the first argument
        """
        if arg == "test":
            # # CKithare currently doesn't have unittests
            # sys.exit(run_cmd(self.exepath, "--test"))
            sys.exit(run_cmd(self.exepath))

        if clean is not None:
            clean_list = (
                ["dep", "build", "dist", "package"]
                if clean == "all"
                else clean.split("+")
            )
            cleaned = 0
            if "dep" in clean_list:
                cleaned += 1
                deps_dir = self.basepath / "deps"
                for dirname in deps_dir.iterdir():
                    rmtree(dirname)

            if "build" in clean_list:
                cleaned += 1
                rmtree(self.builddir.parent)

            dist_dir = self.exepath.parents[1]
            if "dist" in clean_list:
                cleaned += 1

                if (dist_dir / "packaging").is_dir():
                    for sub in dist_dir.iterdir():
                        if sub.is_dir() and sub.name != "packaging":
                            rmtree(sub)
                else:
                    rmtree(dist_dir)

            if "package" in clean_list:
                cleaned += 1
                rmtree(dist_dir / "packaging")

            # remove dist dir if it is empty
            try:
                dist_dir.rmdir()
            except OSError:
                pass

            if cleaned != len(clean_list):
                raise BuildError(
                    "Invalid 'clean' arg passed. Use --help to check the correct usage"
                )

            sys.exit(0)

    def configure_compiler_and_flags(self, machine: str, is_32_bit: bool, make: str):
        """
        Configure and initialise self.cflags object, setup the MinGW compiler
        if it is missing
        """
        if COMPILER == "MinGW":
            mingw_machine = convert_machine(machine, ConvertType.WINDOWS_MINGW)
            self.cflags.cc = f"{mingw_machine}-gcc.exe"
            self.cflags.cxx = f"{mingw_machine}-g++.exe"

        print(INIT_TEXT)
        if shutil.which(self.cflags.get_compiler()) is None:
            # Compiler is not installed and/or not on PATH
            retpath = install_mingw(self.basepath, is_32_bit)
            self.cflags.cc = retpath / self.cflags.cc
            self.cflags.cxx = retpath / self.cflags.cxx
            self.cflags.windres = retpath / "windres.exe"

        print("Building Kithare...")
        print("Platform:", platform.platform())
        print("Compiler:", COMPILER)
        print("Builder Python version:", platform.python_version())

        if machine in SUPPORTED_ARCHS:
            print("Machine:", machine)

        else:
            print(
                "BuildWarning: Your CPU arch has been determined to be "
                f"'{machine}'\nNote that this is not well supported."
            )

        print("Additional compiler info:")
        run_cmd(self.cflags.get_compiler(), "--version", strict=True, silent_cmds=True)

        self.cflags.ccflags.extend(
            (
                "-Wall",
                "-pthread",
                "-g" if make == "debug" else "-O3",  # no -O3 on debug mode
                self.basepath / INCLUDE_DIRNAME,
            )
        )

        if make != "debug":
            # don't pass -Werror in debug mode
            self.cflags.ccflags.append("-Werror")

        if COMPILER == "MinGW":
            self.cflags.add_m_flags("-municode", "-mthreads")

            # statically link C/C++ stdlib and winpthread on Windows MinGW
            self.cflags.ldflags.extend(
                (
                    "-static-libgcc",
                    "-static-libstdc++",
                    "-Wl,-Bstatic,--whole-archive",
                    "-lwinpthread",
                    "-Wl,--no-whole-archive",
                )
            )

        elif platform.system() == "Darwin":
            self.cflags.add_m_flags("-mmacosx-version-min=10.9")

        if is_32_bit:
            self.cflags.add_m_flags("-m32")

        self.cflags.cflags.append(C_STD_FLAG)
        self.cflags.cxxflags.append(CPP_STD_FLAG)

        self.cflags.load_from_env()

    def build_sources(self, build_skippable: bool):
        """
        Generate obj files from source files, returns a list of generated
        objfiles. May also return None if all older objfiles are up date and
        dist exe already exists.
        """
        skipped_files: list[Path] = []
        objfiles: list[Path] = []

        print("Building Kithare sources...")
        compilerpool = CompilerPool(self.j_flag, self.cflags)

        print(f"Building on {min(compilerpool.maxpoolsize, CPU_COUNT)} core(s)")
        if compilerpool.maxpoolsize > CPU_COUNT:
            print(f"Using {compilerpool.maxpoolsize} subprocess(es)")

        print()  # newline
        for file in self.basepath.glob("src/**/*.c*"):
            if file.suffix not in {".c", ".cpp"}:
                # not a C or CPP file
                continue

            ofile = self.builddir / f"{file.stem}.o"
            if ofile in objfiles:
                raise BuildError("Got duplicate filename in Kithare source")

            objfiles.append(ofile)
            if build_skippable and not should_build(
                file, ofile, self.basepath / INCLUDE_DIRNAME
            ):
                # file is already built, skip it
                skipped_files.append(file)
                continue

            compilerpool.add(file, ofile)

        # wait for all sources to compile
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

        if not objfiles:
            raise BuildError(
                "Failed to generate executable because no sources were found"
            )

        if (
            len(objfiles) == len(skipped_files)
            and self.exepath.is_file()
            and (
                platform.system() != "Linux"
                or self.exepath.with_name(f"{EXE}-static").is_file()
            )
        ):
            # exe(s) is(are) already up to date
            return None

        return objfiles

    def build_exe(self):
        """
        Generate final exe.
        """
        # load old cflags from the previous build
        build_conf = self.builddir / "build_conf.json"
        old_cflags = CompilerFlags.from_json(self.basepath, build_conf)

        # because order of args should not matter here
        build_skippable = self.cflags == old_cflags
        if not build_skippable:
            # update conf file with latest cflags
            self.cflags.to_json(build_conf)

        objfiles = self.build_sources(build_skippable)
        if objfiles is None:
            print("Skipping final exe(s) build, since it is already built")
            return

        # Handle exe icon on MinGW
        ico_res = self.basepath / ICO_RES
        if COMPILER == "MinGW":
            assetfile = self.basepath / "src" / "mingw.rc"

            print("Running windres command to set icon for exe")
            ret = run_cmd(self.cflags.windres, assetfile, "-O", "coff", "-o", ico_res)
            if ret:
                print("This means the final exe will not have the kithare logo")
            else:
                objfiles.append(ico_res)

            print()  # newline

        print("Building executable")
        try:
            run_cmd(
                self.cflags.get_compiler(),
                "-o",
                self.exepath,
                *objfiles,
                *self.cflags.flags_by_ext("o"),
                strict=True,
            )
        finally:
            # delete icon file
            if ico_res.is_file():
                ico_res.unlink()

        if platform.system() == "Linux":
            # make statically linked binaries on Linux (useful for appimage)
            self.cflags.ldflags.append("-static")
            print()
            run_cmd(
                self.cflags.get_compiler(),
                "-o",
                self.exepath.with_name(f"{EXE}-static"),
                *objfiles,
                *self.cflags.flags_by_ext("o"),
                strict=True,
            )

        # copy LICENSE and readme to dist
        for filename in ("LICENSE", "README.md"):
            copy(self.basepath / filename, self.exepath.parent)

        for dfile in self.exepath.parent.rglob("*"):
            # Make file permissions less strict on dist dir
            try:
                dfile.chmod(0o775)
            except OSError:
                raise BuildError(
                    "Failed to set file permissions of files in dist dir"
                ) from None

        print("Kithare has been built successfully!")

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
        if incflag is not None:
            self.cflags.ccflags.append(incflag)

        self.cflags.ldflags.extend(self.sdl_installer.ldflags)
        # do any pre-build setup for installer generation
        self.installer.setup()

        t_2 = time.perf_counter()
        self.build_exe()
        print(f"Path to executable: '{self.exepath}'\n")

        t_3 = time.perf_counter()
        # make installer if flag was passed already
        self.installer.package()

        t_4 = time.perf_counter()

        print("Done!\nSome timing stats for peeps who like to 'optimise':")

        # print MinGW install time only if it is large enough (haha majik number)
        if t_1 - self.t_0 > 0.69 and COMPILER == "MinGW":
            print(f"MinGW compiler took {t_1 - self.t_0:.3f} seconds to install")

        # print SDL install time stats only if it is large enough (haha majik number)
        if t_2 - t_1 > 0.69:
            print(f"SDL deps took {t_2 - t_1:.3f} seconds to configure and install")

        print(f"Kithare took {t_3 - t_2:.3f} seconds to compile")

        if t_4 - t_3 > 0.042:
            print(f"Generating the installer took {t_4 - t_3:.3f} seconds")
