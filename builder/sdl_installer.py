"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/sdl_installer.py
Defines classes to handle SDL installation and linker flags
"""


import io
import platform
import tarfile
from pathlib import Path
from typing import Union

from .downloader import ThreadedDownloader
from .utils import BuildError, copy, rmtree

# SDL project-version pairs, remember to keep updated
SDL_DEPS = {
    "SDL2": "2.0.16",
    "SDL2_image": "2.0.5",
    "SDL2_mixer": "2.0.4",
    "SDL2_ttf": "2.0.15",
    "SDL2_net": "2.0.1",
}


class SDLInstaller:
    """
    SDLInstaller base class, with limited functionality. This class is kept
    for compatability with non-windows and non-mac OSes
    """

    def __init__(self):
        """
        Initialise SDLInstaller class
        """
        self.ldflags: list[Union[str, Path]] = []

    def clean(self):  # pylint: disable=no-self-use
        """
        Clean install, this is not available on non-windows
        """
        raise BuildError("The flag 'cleandep' is not supported on your OS")

    def install_all(self):
        """
        Utility function to install all SDL deps. In this dummy installer, only
        updates ldflags to link with already installed SDL
        """
        for name in SDL_DEPS:
            self.ldflags.append(f"-l{name}")
            if name == "SDL2":
                self.ldflags.append("-lSDL2main")


class WindowsSDLInstaller(SDLInstaller):
    """
    Helper class to install SDL deps on MinGW
    """

    def __init__(self, basepath: Path, dist_dir: Path, machine: str):
        """
        Initialise WindowsSDLInstaller class
        """
        super().__init__()
        self.sdl_dir = basepath / "deps" / "SDL"
        self.sdl_include = self.sdl_dir / "include" / "SDL2"

        self.dist_dir = dist_dir
        self.downloader = ThreadedDownloader()

        self.machine = machine

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
        sdl_mingw_dep = self.sdl_dir / f"{name}-{ver}" / self.machine
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
        Extract downloaded dep into SDL deps folder
        """
        try:
            with io.BytesIO(downloaddata) as fileobj:
                with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
                    tarred.extractall(self.sdl_dir)

        except (tarfile.TarError, OSError):
            # some error while extracting
            rmtree(downloaded_path.parent)  # clean failed download
            raise BuildError(
                f"Failed to extract tarfile of {name} while downloading"
            ) from None

        # Copy includes
        for header in downloaded_path.glob("include/SDL2/*.h"):
            copy(header, self.sdl_include)

    def _copy_dll(self, path: Path, overwrite: bool = True):
        """
        Copy DLLs from downloaded path and update ldflags with libpath
        """
        # Copy DLLs that have not been copied already
        for dll in path.glob("bin/*.dll"):
            copy(dll, self.dist_dir, overwrite)

        self.ldflags.append(path / "lib")

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

        # Download SDL deps if unavailable, use threading to download deps
        # concurrently
        for name, ver in SDL_DEPS.items():
            skipped, path = self._download_dep(name, ver)
            if skipped:
                # download was skipped, update lflags and copy any DLLs
                self._copy_dll(path, False)
            else:
                downloads[name] = path

        # extract dependencies
        for name, downloaddata in self.downloader.get_finished():
            self._extract(name, downloaddata, downloads[name])

        # copy DLLs from extracted dependencies
        for path in downloads.values():
            self._copy_dll(path)

        print()  # newline
        return self.sdl_include.parent


class MacSDLInstaller(SDLInstaller):
    """
    Helper class to install SDL deps on MacOS.
    TODO: Fully implement this class with better SDL and dep handling on Mac
    """


def get_installer(basepath: Path, dist_dir: Path, machine: str):
    """
    Gets an instance of the platform specific installer class, fallback to base
    class on other OSes
    """
    if platform.system() == "Windows":
        return WindowsSDLInstaller(basepath, dist_dir, machine)

    if platform.system() == "Darwin":
        return MacSDLInstaller()

    return SDLInstaller()
