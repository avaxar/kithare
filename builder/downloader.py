"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/downloader.py
Defines a ThreadedDownloader that helps in downloading dependencies in the
background
"""


import hashlib
import io
import queue
import ssl
import threading
import urllib.request as urllib
import zipfile
from pathlib import Path
from typing import Optional

from .utils import BuildError

# allow unverified SSL because armv7 CI errors at that for some reason while
# downloading deps
ssl._create_default_https_context = ssl._create_unverified_context

# Downloads timeout in seconds
DOWNLOAD_TIMEOUT = 600
TIMEOUT_PER_LOOP = 0.05

LINKS_AND_HASHES = {}

MINGW_DOWNLOAD_DIR = (
    "https://github.com/brechtsanders/winlibs_mingw/releases/download/9.4.0-9.0.0-r1/"
)
MINGW_ZIP = (
    "winlibs-x86_64-posix-seh-gcc-9.4.0-mingw-w64-9.0.0-r1.zip",
    "winlibs-i686-posix-dwarf-gcc-9.4.0-mingw-w64-9.0.0-r1.zip",
)


class ThreadedDownloader:
    """
    Install file(s) concurrently using threads in the background
    """

    def __init__(self):
        """
        Initialise ThreadedDownloader object
        """
        self.threads: set[threading.Thread] = set()
        self.downloaded: queue.Queue[tuple[str, Optional[bytes]]] = queue.Queue()

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
            self.downloaded.put((name, None))

        else:
            self.downloaded.put((name, download))

    def download(self, name: str, download_link: str):
        """
        Download a file
        """
        # Start thread, add it to a set of running threads
        thread = threading.Thread(
            target=self._download_thread,
            name=name,
            args=(name, download_link),
            daemon=True,
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
        Iterate over downloaded resources (name-data pairs). Blocks while
        waiting for all threads to complete. data being None indicates error in
        download
        """
        loops = 0
        while self.is_downloading() or not self.downloaded.empty():
            try:
                # Do timeout and loop because we need be able to handle any
                # potential KeyboardInterrupt errors
                name, data = self.downloaded.get(timeout=TIMEOUT_PER_LOOP)
            except queue.Empty:
                pass

            else:
                if data is None:
                    raise BuildError(
                        f"Failed to download {name} due to some networking error"
                    )

                datahash = hashlib.sha256(data).hexdigest()
                presethash = LINKS_AND_HASHES.get(name)
                if datahash != presethash:
                    if presethash is not None:
                        raise BuildError(f"Download hash for {name} mismatched")

                    print(f"BuildWarning: Download {name} has not been hash verified!")
                    print(datahash)

                print(f"Successfully downloaded {name}!")
                yield name, data

            loops += 1
            if loops * TIMEOUT_PER_LOOP > DOWNLOAD_TIMEOUT:
                raise BuildError(
                    f"Download(s) timed out! Took longer than {DOWNLOAD_TIMEOUT} s."
                )

    def get_one(self):
        """
        Get one downloaded resource (name-data pair). Function can block while
        waiting for resource. If download failed, raises error.
        """
        for name, data in self.get_finished():
            return name, data

        raise BuildError("Failed to fetch downloads as all were completed")


def install_mingw(basepath: Path, is_32_bit: bool):
    """
    Install MinGW into the deps folder, this is used as a fallback when MinGW
    is not pre-installed on the machine. Returns path object to MinGW bin dir
    """
    mingw_name = "MinGW32" if is_32_bit else "MinGW64"
    deps_dir = basepath / "deps"
    ret = deps_dir / mingw_name.lower() / "bin"
    if ret.is_dir():
        return ret

    print("MinGW is not pre-installed, installing it into deps dir.")
    downloader = ThreadedDownloader()
    downloader.download(mingw_name, MINGW_DOWNLOAD_DIR + MINGW_ZIP[is_32_bit])
    print("This can take a while, depending on your internet speeds")

    with zipfile.ZipFile(io.BytesIO(downloader.get_one()[1]), "r") as zipped:
        zipped.extractall(deps_dir)

    print()  # newline
    return ret
