"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/utils.py
Defines common builder utility functions
"""

import argparse
import platform
import shutil
import stat
import subprocess

from datetime import datetime
from enum import Enum, auto
from functools import lru_cache
from pathlib import Path
from typing import Union

from .constants import EPILOG, INIT_TEXT

# While we recursively search for include files, we don't want to seach
# the whole file, because that would waste a lotta time. So, we just take
# an arbitrary line number limit, beyond which, we won't search
INC_FILE_LINE_LIMIT = 75

TIMESTAMP = (
    datetime.utcnow().isoformat(".", "minutes").replace("-", ".").replace(":", "")
)
NIGHLY_BUILD_VER = f"{TIMESTAMP}-nightly"


class ConvertType(Enum):
    """
    Enum for input for convert_machine
    """

    WINDOWS = auto()
    WINDOWS_MINGW = auto()
    MAC = auto()
    LINUX_DEB = auto()
    LINUX_ARCH = auto()
    LINUX_RPM = auto()
    APP_IMAGE = auto()


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


def get_rel_path(dirpath: Path, basepath: Path = Path().resolve()):
    """
    Get dirpath as relative to basepath. This handles corner cases better than
    Path.relative_to, uses ".." path notation so that the relative path is
    always obtained no matter where the two paths are located. Here, dirpath
    and basepath must be fully resolved absolute paths to directories. By
    default, basepath is current working dir
    """
    ret_parts: list[str] = []
    back_parts = 0
    has_seperated = False

    cnt = -1
    for cnt, part in enumerate(dirpath.parts):
        if cnt >= len(basepath.parts):
            ret_parts.append(part)
            continue

        # got a mismatching parent dir
        if basepath.parts[cnt] != part:
            has_seperated = True

        if has_seperated:
            ret_parts.append(part)
            back_parts += 1

    cnt += 1
    back_parts += max(len(basepath.parts) - cnt, 0)

    ret = Path(*([".."] * back_parts))
    for part in ret_parts:
        ret /= part

    return ret


def run_cmd(*cmds: Union[str, Path], strict: bool = False, silent_cmds: bool = False):
    """
    Helper function to run command in subprocess.
    Prints the command, command output, and error exit code (if nonzero), and
    also returns the exit code. If strict arg is given True, BuildError is
    raised rather than printing the error.
    """
    if not silent_cmds:
        print(">", *cmds)

    # run with subprocess
    try:
        proc = subprocess.run(
            list(map(str, cmds)),
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
    overwritten if it already exists.
    Returns Path object to the created file.
    """
    if not overwrite and (dest / file.name).exists():
        return dest / file.name

    try:
        return Path(shutil.copy(file, dest))
    except OSError:
        raise BuildError(
            f"Could not copy file '{file}' to '{dest}' directory"
        ) from None


def get_machine(is_32_bit: bool):
    """
    Utility to get string representation of the machine name. Possible return
    values:
    name    | Description           | Aliases
    ----------------------------------------
    x86     | Intel/AMD 32-bit      | i386, i686
    x64     | Intel/AMD 64-bit      | x86_64, amd64
    arm     | ARM 32-bit (very old) | armel, armv5 (or older)
    armv6   | ARM 32-bit (old)      | armhf, armv6l, armv6h
    armv7   | ARM 32-bit            | armhf, armv7l, armv7h
    arm64   | ARM 64-bit            | aarch64, armv8l, armv8 (or newer)
    ppc64le | PowerPC achitecture   | ppc64el (debian terminology)
    s390x   | IBM (big endian)      | None
    Unknown | Architecture could not be determined

    The function can also return other values platform.machine returns, without
    any modifications
    """
    machine = platform.machine()
    machine_lowered = machine.lower()
    if machine.endswith("86"):
        return "x86"

    if machine_lowered in {"x86_64", "amd64", "x64"}:
        return "x86" if is_32_bit else "x64"

    if machine_lowered in {"armv8", "armv8l", "arm64", "aarch64"}:
        return "armv7" if is_32_bit else "arm64"

    if machine_lowered.startswith("arm"):
        if "v7" in machine_lowered or "hf" in machine_lowered:
            return "armv7"

        if "v6" in machine_lowered:
            return "armv6"

        return "arm"

    if machine == "ppc64el":
        return "ppc64le"

    if not machine:
        return "Unknown"

    return machine


def convert_machine(machine: str, mode: ConvertType):
    """
    Convert machine returned by get_machine to another format

    Here is a table of what this function does

    name    | Windows | MinGW  | Mac    | Debian  | Arch    | RPM      | AppImage
    ------------------------------------------------------------------------------
    x86     | x86     | i686   | i686   | i386    | i686    | i686     | i686
    x64     | x64     | x86_64 | x86_64 | amd64   | x86_64  | x86_64   | x86_64
    arm     | Errors  | Errors | Errors | armel   | arm     | armv5tel | Errors
    armv6   | Errors  | Errors | Errors | armhf   | armv6h  | armv6l   | Errors
    armv7   | Errors  | Errors | Errors | armhf   | armv7h  | armv7l   | armhf
    arm64   | Errors  | Errors | arm64  | arm64   | aarch64 | aarch64  | aarch64
    ppc64le | Errors  | Errors | Errors | ppc64el | Errors  | ppc64le  | Errors
    Others  | Errors  | Errors | Errors | Returns | Errors  | Returns  | Errors
    Unknown | Errors  | Errors | Errors | Errors  | Errors  | Errors   | Errors
    """

    if mode not in ConvertType:
        raise ValueError("Bug in builder, received invalid mode arg")

    if machine.lower() == "unknown":
        raise BuildError(
            "Your CPU Architecture could not be determined, an installer "
            "cannot be made"
        )

    if machine == "x86":
        if mode == ConvertType.LINUX_DEB:
            return "i386"

        if mode == ConvertType.WINDOWS:
            return "x86"

        return "i686"

    if machine == "x64":
        if mode == ConvertType.LINUX_DEB:
            return "amd64"

        if mode == ConvertType.WINDOWS:
            return "x64"

        return "x86_64"

    if machine == "arm":
        if mode == ConvertType.LINUX_DEB:
            return "armel"

        if mode == ConvertType.LINUX_ARCH:
            return "arm"

        if mode == ConvertType.LINUX_RPM:
            return "armv5tel"

    if machine in {"armv6", "armv7"}:
        if mode == ConvertType.LINUX_DEB or (
            mode == ConvertType.APP_IMAGE and machine == "armv7"
        ):
            return "armhf"

        if mode == ConvertType.LINUX_ARCH:
            return machine + "h"

        if mode == ConvertType.LINUX_RPM:
            return machine + "l"

    if machine == "arm64":
        if mode in {ConvertType.MAC, ConvertType.LINUX_DEB}:
            return "arm64"

        if mode in {
            ConvertType.LINUX_ARCH,
            ConvertType.LINUX_RPM,
            ConvertType.APP_IMAGE,
        }:
            return "aarch64"

    if machine == "ppc64le":
        if mode == ConvertType.LINUX_DEB:
            return "ppc64el"

        if mode == ConvertType.LINUX_RPM:
            return "ppc64le"

    if mode in {ConvertType.LINUX_DEB, ConvertType.LINUX_RPM}:
        return machine

    if mode == ConvertType.APP_IMAGE:
        raise BuildError(
            f"installers for {machine} CPU are not supported with AppImage"
        )

    raise BuildError(f"Installers for {machine} CPU are not supported on this platform")


def parse_args():
    """
    Parse commandline args using the argparse module
    """
    parser = argparse.ArgumentParser(
        description=INIT_TEXT,
        epilog=EPILOG,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    parser.add_argument(
        "--make",
        choices=("clean", "debug", "test", "installer"),
        help="Specifies the action that the build script should take",
    )

    parser.add_argument(
        "-j",
        metavar="cores",
        type=int,
        help="Specifies the number of cores to use during compilation",
    )

    parser.add_argument(
        "--arch",
        metavar="architecture",
        choices=("x86", "x64"),
        default="x64",
        help="A flag that can be x86 for 32-bit, and x64 for 64-bit",
    )

    parser.add_argument(
        "--release",
        metavar="version",
        default=NIGHLY_BUILD_VER,
        help="Used to set the version on the installer (PROVISIONAL FLAG, DO NOT USE)",
    )

    parser.add_argument(
        "--use-alien",
        action="store_true",
        help="A flag that indicates whether to use 'alien' command on installers",
    )

    ret = parser.parse_args()
    return ret.arch == "x86", ret
