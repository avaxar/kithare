"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/__init__.py
Defines the main KithareBuilder class that builds Kithare
"""


import json
import platform
import sys
import time
from json.decoder import JSONDecodeError
from pathlib import Path
from typing import Optional, Union

from .compilerpool import CompilerPool
from .constants import (
    COMPILER,
    COMPILER_NAME,
    CPU_COUNT,
    EXE,
    ICO_RES,
    INCLUDE_DIRNAME,
    KITHARE_FLAGS,
    STD_FLAG,
    SUPPORTED_ARCHS,
)
from .packaging import get_packager
from .sdl_installer import get_installer
from .utils import (
    BuildError,
    get_machine,
    get_rel_path,
    rmtree,
    copy,
    run_cmd,
    should_build,
)

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

        self.sdl_installer = get_installer(basepath, self.exepath.parent, machine)
        if args:
            self._handle_first_arg(args[0])

        use_alien = "--use-alien" in args
        self.installer = (
            get_packager(self.basepath, self.exepath, machine, use_alien)
            if "--make-installer" in args
            else None
        )

        if self.installer is None and use_alien:
            raise BuildError(
                "The '--use-alien' flag cannot be passed without '--make-installer'"
            )

        if self.installer is not None and debug:
            raise BuildError(
                "Cannot generate installer with Kithare compiled in debug mode"
            )

        print(INIT_TEXT)
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
        run_cmd(COMPILER_NAME[".c"], "--version", strict=True, silent_cmds=True)

        # compiler flags
        self.cflags: list[Union[str, Path]] = [
            "-g" if debug else "-O3",  # no -O3 on debug mode
            basepath / INCLUDE_DIRNAME,
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

            elif not i.startswith("--arch=") and i not in KITHARE_FLAGS:
                self.cflags.append(i)

    def get_flags(self, rel_base: bool = False, with_lflag: bool = False):
        """
        Get cflags/lflags, with any path objects resolved correctly. rel_base
        arg specifies whether the paths should be relative to base dir, or
        current working dir. If with_lflag is specified, linker flags are also
        outputted.
        """
        new_flags: list[str] = []

        flags = self.sdl_installer.lflags if with_lflag else self.cflags
        for flag in flags:
            if isinstance(flag, str):
                new_flags.append(f"-l{flag}" if with_lflag else flag)
                continue

            if rel_base:
                flag = flag.relative_to(self.basepath)

            new_flags.append(f"-L{flag}" if with_lflag else f"-I{flag}")

        if with_lflag:
            new_flags.extend(self.get_flags(rel_base))
            if rel_base:
                new_flags.extend(STD_FLAG.values())

        return new_flags

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
        compilerpool = CompilerPool(self.j_flag, *self.get_flags())

        print(f"Building on {min(compilerpool.maxpoolsize, CPU_COUNT)} core(s)")
        if compilerpool.maxpoolsize > CPU_COUNT:
            print(f"Using {compilerpool.maxpoolsize} subprocess(es)")

        print()  # newline
        for file in self.basepath.glob("src/**/*.c*"):
            if file.suffix not in STD_FLAG:
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
            old_cflags: list[str] = json.loads(build_conf.read_text())
        except (FileNotFoundError, JSONDecodeError):
            old_cflags = []

        new_store_cflags = self.get_flags(rel_base=True, with_lflag=True)

        # because order of args should not matter here
        build_skippable = sorted(new_store_cflags) == sorted(old_cflags)
        if not build_skippable:
            # update conf file with latest cflags
            build_conf.write_text(json.dumps(new_store_cflags))

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
            new_cflags = self.get_flags(with_lflag=True)
            run_cmd(
                COMPILER_NAME[".cpp"],
                "-o",
                self.exepath,
                *objfiles,
                *new_cflags,
                strict=True,
            )
        finally:
            # delete icon file
            if ico_res.is_file():
                ico_res.unlink()

        # copy LICENSE and readme to dist
        for filename in {"LICENSE", "README.md"}:
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
            self.cflags.append(incflag)

        if self.installer is not None:
            # do any pre-build setup for installer generation
            self.installer.setup()

        t_2 = time.perf_counter()
        self.build_exe()
        print(f"Path to executable: {self.exepath}\n")

        t_3 = time.perf_counter()
        if self.installer is not None:
            # make installer if flag was passed already
            self.installer.package()

        t_4 = time.perf_counter()

        print("Done!\nSome timing stats for peeps who like to 'optimise':")

        # print SDL install time stats only if it is large enough (haha majik number)
        if t_2 - t_1 > 0.42:
            print(f"SDL deps took {t_2 - t_1:.3f} seconds to configure and install")

        print(f"Kithare took {t_3 - t_2:.3f} seconds to compile")

        if self.installer is not None:
            print(f"Generating the installer took {t_4 - t_3:.3f} seconds")
