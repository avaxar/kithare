"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/packaging.py
Defines classes to handle Kithare packaging into platform-specific installers
"""

import platform
from pathlib import Path
from typing import Optional
from zipfile import ZipFile

from .constants import KITHARE_VERSION, VERSION_PACKAGE_REV
from .downloader import ThreadedDownloader
from .utils import BuildError, ConvertType, convert_machine, copy, rmtree, run_cmd

# Windows INNO installer related constants, remember to keep updated
INNO_SETUP_DOWNLOAD = "https://files.jrsoftware.org/is/6/innosetup-6.2.0.exe"
INNO_FLAGS = "/SP- /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /NOCANCEL".split()

INNO_COMPILER_PATH = Path("C:\\", "Program Files (x86)", "Inno Setup 6", "ISCC.exe")


class Packager:
    """
    Packager is a base class for all other platform-specific Packager classes,
    that help packaging Kithare.
    """

    def __init__(self, basepath: Path, exepath: Path, machine: str):
        """
        Initialise Packager class
        """
        self.basepath = basepath
        self.packaging_dir = basepath / "builder" / "packaging"
        self.exepath = exepath
        self.machine = machine

    def setup(self):
        """
        This function sets up any dependencies and such needed to make the
        package. If there is no setup to do, this function does nothing.
        """

    def package(self):
        """
        Make portable zip package
        """
        print("Making portable binary ZIP")
        zipname = f"kithare-{KITHARE_VERSION}-{platform.system()}-{self.machine}.zip"
        portable_zip = self.packaging_dir / "dist" / zipname

        portable_zip.parent.mkdir(exist_ok=True)
        with ZipFile(portable_zip, mode="w") as myzip:
            # copy executable and other files
            for dfile in self.exepath.parent.rglob("*"):
                zipped_file = Path("Kithare") / dfile.relative_to(self.exepath.parent)
                myzip.write(dfile, arcname=zipped_file)

        print(f"Finished making zipfile in '{portable_zip}'\n")


class WindowsPackager(Packager):
    """
    Subclass of Packager that handles Windows packaging
    """

    def __init__(self, basepath: Path, exepath: Path, machine: str):
        """
        Initialise WindowsPackager class
        """
        super().__init__(basepath, exepath, machine)

        self.machine = convert_machine(machine, ConvertType.WINDOWS)
        self.downloader: Optional[ThreadedDownloader] = None

    def setup(self):
        """
        This prepares INNO installer
        """
        if INNO_COMPILER_PATH.is_file():
            return

        print("Could not find pre-installed INNO Setup, after looking in dir")
        print(INNO_COMPILER_PATH.parent)
        print(
            "Downloading and installing INNO Setup in the background while "
            "compilation continues\n"
        )

        # Download INNO Setup installer in background
        self.downloader = ThreadedDownloader()
        self.downloader.download("INNO Setup Installer", INNO_SETUP_DOWNLOAD)

    def package(self):
        """
        Make installer for Windows
        """
        super().package()

        print("Using Windows installer configuration")

        installer_build_dir = self.packaging_dir / "build"
        rmtree(installer_build_dir)  # clean old build dir
        installer_build_dir.mkdir()

        default_iss_file = self.packaging_dir / "kithare_windows.iss"
        iss_file = self.packaging_dir / "build" / "kithare_windows.iss"

        # Rewrite iss file, with some macros defined
        iss_file.write_text(
            f'#define MyAppVersion "{KITHARE_VERSION}"\n'
            + f'#define MyAppArch "{self.machine}"\n'
            + f'#define BasePath "{self.basepath.resolve()}"\n'
            + default_iss_file.read_text()
        )

        if self.downloader is not None:
            if self.downloader.is_downloading():
                print("Waiting for INNO Setup download to finish")

            inno_installer = installer_build_dir / "innosetup_installer.exe"
            inno_installer.write_bytes(self.downloader.get_one()[1])

            print("Installing Inno setup")
            try:
                run_cmd(inno_installer, *INNO_FLAGS, strict=True)
            finally:
                inno_installer.unlink()

            print()  # newline

        print("Making Kithare installer")
        run_cmd(INNO_COMPILER_PATH, iss_file, strict=True)
        print("Successfully made Kithare installer!\n")


class LinuxPackager(Packager):
    """
    Subclass of Packager that handles Linux packaging
    """

    def __init__(self, basepath: Path, exepath: Path, machine: str, use_alien: bool):
        """
        Initialise LinuxPackager class
        """
        super().__init__(basepath, exepath, machine)
        self.use_alien = use_alien

    def debian_package(self):
        """
        Make deb installer file for Debian
        """
        print("\nMaking Debian .deb installer")

        version = f"{KITHARE_VERSION}-{VERSION_PACKAGE_REV}"
        machine = convert_machine(self.machine, ConvertType.LINUX_DEB)

        installer_dir = self.packaging_dir / "build" / f"kithare_{version}"
        rmtree(installer_dir)  # clean old dir

        bin_dir = installer_dir / "usr" / "local" / "bin"
        bin_dir.mkdir(parents=True)

        # copy dist exe
        copy(self.exepath, bin_dir)

        # write a control file
        write_control_file = installer_dir / "DEBIAN" / "control"
        write_control_file.parent.mkdir()

        control_file = self.packaging_dir / "debian_control.txt"
        write_control_file.write_text(
            f"Version: {version}\n"
            + f"Architecture: {machine}\n"
            + control_file.read_text()
        )

        # write license file in the doc dir
        doc_dir = installer_dir / "usr" / "share" / "doc" / "kithare"
        doc_dir.mkdir(parents=True)
        license_file = copy(self.packaging_dir / "debian_license.txt", doc_dir)
        license_file.rename(license_file.with_name("copyright"))

        dist_dir = self.packaging_dir / "dist"
        dist_dir.mkdir(exist_ok=True)

        run_cmd("dpkg-deb", "--build", installer_dir, dist_dir, strict=True)
        print(".deb file was made successfully\n")

        if self.use_alien:
            print("Using 'alien' package to make rpm packages from debian packages")
            rpm_machine = convert_machine(self.machine, ConvertType.LINUX_RPM)
            run_cmd(
                "alien",
                "--to-rpm",
                "--keep-version",
                f"--target={rpm_machine}",
                dist_dir / f"kithare_{version}_{machine}.deb",
                strict=True,
            )

            gen_rpm = self.basepath / f"kithare-{version}.{rpm_machine}.rpm"
            try:
                rpm_file = copy(gen_rpm, dist_dir)
            finally:
                gen_rpm.unlink()

            print(f"Generated rpm package in '{rpm_file}'!\n")

    def package(self):
        """
        Make installer for Linux
        """
        super().package()

        print("Using Linux installer configuration")
        print("Testing for Debian")
        try:
            run_cmd("dpkg-deb", "--version", strict=True)
        except BuildError:
            print("The platform is not Debian-based")
        else:
            return self.debian_package()

        raise BuildError(
            "Your linux distro is unsupported by Kithare for now. "
            "However, adding support for more distros is in our TODO!"
        )


class MacPackager(Packager):
    """
    Subclass of Packager that handles MacOS packaging
    """


def get_packager(basepath: Path, exepath: Path, machine: str, use_alien: bool):
    """
    Get appropriate packager class for handling packaging
    """
    system = platform.system()
    if system == "Windows":
        if use_alien:
            raise BuildError("'--use-alien' is not a supported flag on this OS")

        return WindowsPackager(basepath, exepath, machine)

    if system == "Linux":
        return LinuxPackager(basepath, exepath, machine, use_alien)

    if system == "Darwin":
        if use_alien:
            raise BuildError("'--use-alien' is not a supported flag on this OS")

        return MacPackager(basepath, exepath, machine)

    raise BuildError(
        "Cannot generate installer as your platform could not be determined!"
    )
