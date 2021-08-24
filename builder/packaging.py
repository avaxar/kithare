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

from .constants import VERSION_PACKAGE_REV
from .downloader import ThreadedDownloader
from .utils import BuildError, ConvertType, convert_machine, copy, rmtree, run_cmd

# Windows INNO installer related constants, remember to keep updated
INNO_SETUP_DOWNLOAD = "https://files.jrsoftware.org/is/6/innosetup-6.2.0.exe"
INNO_FLAGS = "/SP- /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /NOCANCEL".split()

INNO_COMPILER_PATH = Path("C:\\", "Program Files (x86)", "Inno Setup 6", "ISCC.exe")

APPIMAGE_DOWNLOAD = "https://github.com/AppImage/AppImageKit/releases/latest/download"


class Packager:
    """
    Packager is a base class for all other platform-specific Packager classes,
    that help packaging Kithare.
    """

    def __init__(self, basepath: Path, exepath: Path, machine: str, version: str):
        """
        Initialise Packager class
        """
        self.exepath = exepath
        self.machine = machine
        self.version = version

        self.packaging_dir = basepath / "builder" / "packaging"
        self.downloader: Optional[ThreadedDownloader] = None

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
        zipname = (
            f"kithare-{self.version}-{platform.system().lower()}-{self.machine}.zip"
        )
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

    def __init__(self, basepath: Path, exepath: Path, machine: str, version: str):
        """
        Initialise WindowsPackager class
        """
        super().__init__(basepath, exepath, machine, version)

        self.machine = convert_machine(machine, ConvertType.WINDOWS)

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
            f'#define MyAppVersion "{self.version}"\n'
            + f'#define MyAppArch "{self.machine}"\n'
            + f'#define BasePath "{self.packaging_dir.parents[1].resolve()}"\n'
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

    def __init__(
        self,
        basepath: Path,
        exepath: Path,
        machine: str,
        version: str,
        use_alien: bool,
    ):
        """
        Initialise LinuxPackager class
        """
        super().__init__(basepath, exepath, machine, version)
        self.version += f"-{VERSION_PACKAGE_REV}"

        self.use_alien = use_alien

        self.appimagekitdir: Optional[Path] = None

    def setup(self):
        """
        This prepares AppImageKit
        """
        try:
            appimage_type = convert_machine(self.machine, ConvertType.APP_IMAGE)
        except BuildError as exc:
            print(f"Skipping AppImage generation, because {exc.emsg}\n")
            return

        self.appimagekitdir = (
            self.packaging_dir.parents[1] / "deps" / "AppImage" / appimage_type
        )

        if self.appimagekitdir.is_dir():
            # AppImageKit already exists, no installation required
            return

        print(
            "Downloading and installing AppImageKit (for making AppImages) in "
            "the background while compilation continues"
        )

        # Download INNO Setup installer in background
        self.downloader = ThreadedDownloader()
        self.downloader.download(
            "AppImageTool",
            f"{APPIMAGE_DOWNLOAD}/appimagetool-{appimage_type}.AppImage",
        )
        self.downloader.download(
            "AppImageRun",
            f"{APPIMAGE_DOWNLOAD}/AppRun-{appimage_type}",
        )
        self.downloader.download(
            "AppImageRuntime",
            f"{APPIMAGE_DOWNLOAD}/runtime-{appimage_type}",
        )

        print()  # newline

    def make_appimage(self):
        """
        Make AppImage installers
        """
        if self.appimagekitdir is None:
            return

        print("Making Linux universal packages with AppImage")

        installer_build_dir = self.packaging_dir / "build" / "kithare.AppDir"
        rmtree(installer_build_dir)  # clean old build dir

        bin_dir = installer_build_dir / "usr" / "bin"
        bin_dir.mkdir(parents=True)

        # copy dist exe
        copy(self.exepath, bin_dir)

        # copy desktop file
        copy(self.packaging_dir / "kithare.desktop", installer_build_dir)

        # copy icon file
        copy(
            self.packaging_dir.parents[1] / "assets" / "kithare.png",
            installer_build_dir,
        )

        self.appimagekitdir.mkdir(parents=True, exist_ok=True)

        appimagekit = {
            "AppImageTool": self.appimagekitdir / "appimagetool.AppImage",
            "AppImageRun": self.appimagekitdir / "AppRun",
            "AppImageRuntime": self.appimagekitdir / "runtime",
        }

        if self.downloader is not None:
            if self.downloader.is_downloading():
                print("Waiting for AppImageKit downloads to finish")

            for name, data in self.downloader.get_finished():
                if data is None:
                    raise BuildError(f"Failed to fetch downloads of {name}")

                # save download in file
                appimagekit[name].write_bytes(data)
                appimagekit[name].chmod(0o775)
                print(f"Successfully downloaded {name}!")

        if self.machine not in {"x86", "x64"}:
            # A workaround for AppImage bug on arm docker
            # https://github.com/AppImage/AppImageKit/issues/1056
            run_cmd(
                "sed",
                "-i",
                r"s|AI\x02|\x00\x00\x00|",
                appimagekit["AppImageTool"],
                strict=True,
            )

        # copy main runfile to AppDir
        copy(appimagekit["AppImageRun"], installer_build_dir)

        dist_image = (
            self.packaging_dir
            / "dist"
            / f"kithare-{self.version}-{self.appimagekitdir.name}.AppImage"
        )
        dist_image.parent.mkdir(exist_ok=True)

        run_cmd(
            appimagekit["AppImageTool"],
            "--appimage-extract-and-run",
            "--runtime-file",
            appimagekit["AppImageRuntime"],
            installer_build_dir,
            dist_image,
            strict=True,
        )
        dist_image.chmod(0o775)
        print(f"Successfully made AppImage installer in '{dist_image}'!\n")

    def debian_package(self):
        """
        Make deb installer file for Debian
        """
        print("\nMaking Debian .deb installer")

        machine = convert_machine(self.machine, ConvertType.LINUX_DEB)

        installer_dir = (
            self.packaging_dir / "build" / "Debian" / f"kithare_{self.version}"
        )
        rmtree(installer_dir.parent)  # clean old dir

        bin_dir = installer_dir / "usr" / "bin"
        bin_dir.mkdir(parents=True)

        # copy dist exe
        copy(self.exepath, bin_dir)

        # write a control file
        write_control_file = installer_dir / "DEBIAN" / "control"
        write_control_file.parent.mkdir()

        control_file = self.packaging_dir / "debian_control.txt"
        write_control_file.write_text(
            f"Version: {self.version}\n"
            + f"Architecture: {machine}\n"
            + control_file.read_text()
        )

        # write license file in the doc dir
        doc_dir = installer_dir / "usr" / "share" / "doc" / "kithare"
        doc_dir.mkdir(parents=True)
        license_file = copy(self.packaging_dir / "debian_license.txt", doc_dir)
        license_file.rename(license_file.with_name("copyright"))

        dist_dir = self.packaging_dir / "dist"

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
                dist_dir / f"kithare_{self.version}_{machine}.deb",
                strict=True,
            )

            gen_rpm = (
                self.packaging_dir.parents[1]
                / f"kithare-{self.version.replace('-', '_', 1)}.{rpm_machine}.rpm"
            )
            try:
                rpm_file = copy(gen_rpm, dist_dir)
            finally:
                if gen_rpm.is_file():
                    gen_rpm.unlink()

            print(f"Generated rpm package in '{rpm_file}'!\n")

    def package(self):
        """
        Make installer for Linux
        """
        super().package()

        self.make_appimage()

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


def get_packager(
    basepath: Path, exepath: Path, machine: str, version: str, use_alien: bool
):
    """
    Get appropriate packager class for handling packaging
    """
    system = platform.system()
    if system == "Windows":
        if use_alien:
            raise BuildError("'--use-alien' is not a supported flag on this OS")

        return WindowsPackager(basepath, exepath, machine, version)

    if system == "Linux":
        return LinuxPackager(basepath, exepath, machine, version, use_alien)

    if system == "Darwin":
        if use_alien:
            raise BuildError("'--use-alien' is not a supported flag on this OS")

        return MacPackager(basepath, exepath, machine, version)

    raise BuildError(
        "Cannot generate installer as your platform could not be determined!"
    )
