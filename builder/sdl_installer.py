"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/sdl_installer.py
Defines classes to handle SDL installation and linker flags
"""


import io
import tarfile
from pathlib import Path
from typing import Union, Optional

from .constants import COMPILER
from .downloader import ThreadedDownloader
from .utils import BuildError, ConvertType, convert_machine, copy, rmtree

# SDL project-version pairs, remember to keep updated
SDL_DEPS = {
    "SDL2": "2.0.16",
    "SDL2_image": "2.0.5",
    "SDL2_mixer": "2.0.4",
    "SDL2_ttf": "2.0.15",
    "SDL2_net": "2.0.1",
}


class DummySDLInstaller:
    """
    Dummy SDLInstaller class, with limited functionality. This class is kept
    for compatability with non-windows OSes
    """

    def __init__(self):
        """
        Initialise DummySDLInstaller class
        """
        self.lflags: list[Union[str, Path]] = []

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
            self.lflags.append(name)
            if name == "SDL2":
                self.lflags.append("SDL2main")


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

        machine = convert_machine(machine, ConvertType.WINDOWS_MINGW)
        self.sdl_type = f"{machine}-w64-mingw32"

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

    def _extract(self, name: str, downloaddata: Optional[bytes], downloaded_path: Path):
        """
        Extract downloaded dep into SDL deps folder, return bool on whether
        extraction succeeded or not
        """
        if downloaddata is None:
            return False

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

        self.lflags.append(path / "lib")

    def _install(self, *skipped_downloads: Path, **downloads: Path):
        """
        Install SDL dependencies. Returns whether the install was successful
        """
        for download in skipped_downloads:
            self._copy_dll(download, False)

        failed = False
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
        skipped_downloads: set[Path] = set()

        # Download SDL deps if unavailable, use threading to download deps
        # concurrently
        for name, ver in SDL_DEPS.items():
            skipped, path = self._download_dep(name, ver)
            if skipped:
                skipped_downloads.add(path)
            else:
                downloads[name] = path

        if not self._install(*skipped_downloads, **downloads):
            print(
                "These error(s) will be ignored for now, but may cause build "
                "errors later"
            )

        print()  # newline
        return self.sdl_include.parent


def get_installer(basepath: Path, dist_dir: Path, machine: str):
    """
    Gets an instance of SDLInstaller or DummySDLInstaller depending on the OS
    """
    return (
        SDLInstaller(basepath, dist_dir, machine)
        if COMPILER == "MinGW"
        else DummySDLInstaller()
    )
