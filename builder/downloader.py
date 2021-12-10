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

from utils import BuildError

# allow unverified SSL because armv7 CI errors at that for some reason while
# downloading deps
ssl._create_default_https_context = ssl._create_unverified_context

# Downloads timeout in seconds
DOWNLOAD_TIMEOUT = 600
TIMEOUT_PER_LOOP = 0.05

LINKS_AND_HASHES = {
    "SDL2": "2bfe48628aa9635c12eac7d421907e291525de1d0b04b3bca4a5bd6e6c881a6f",
    "SDL2_image": "41d9e5ff98aa84cf66e6c63c78e7c346746982fa53d3f36633423cc9177f986c",
    "SDL2_mixer": "14250b2ade20866c7b17cf1a5a5e2c6f3920c443fa3744f45658c8af405c09f1",
    "SDL2_ttf": "78009f19c1145b1adf86816f0cc12c407ada2b77cf06ee7042d55d231b726d3b",
    "SDL2_net": "fe0652ab1bdbeae277d7550f2ed686a37a5752f7a624f54f19cf1bd6ba5cb9ff",
    "MinGW32": "56159f3752d31c28e0a7ed480d5e6b6860ce21ce2e640eba6c4a847504626bef",
    "MinGW64": "695dbea4159066b7b02d22a8e7ac241e397ed5665171546526f5818c4b8b3a19",
    "INNO Setup Installer": "2459da3c0a67346bc43a9732d96929877d04f53b1d4c56e61be64e3b5f34d5cf",
    "AppImageTool": {
        "x86_64": "df3baf5ca5facbecfc2f3fa6713c29ab9cefa8fd8c1eac5d283b79cab33e4acb",
        "i686": "104978205c888cb2ad42d1799e03d4621cb9a6027cfb375d069b394a82ff15d1",
        "armhf": "36bb718f32002357375d77b082c264baba2a2dcf44ed1a27d51dbb528fbb60f6",
        "aarch64": "334e77beb67fc1e71856c29d5f3f324ca77b0fde7a840fdd14bd3b88c25c341f",
    },
    "AppImageRun": {
        "x86_64": "fd0e2c14a135e7741ef82649558150f141a04c280ed77a5c6f9ec733627e520e",
        "i686": "ec319f2ed657729c160d492330b617709047195bf55507ff6e7264cb6761c66c",
        "armhf": "cf3c197b8ef4faf1a3a14f71f5d05a49be0cca0a4a8973d58e34c78e26d4ff80",
        "aarch64": "9214c4c1f7a3cdd77f8d558c2039230a322469f8aaf7c71453eeaf1f2f33d204",
    },
    "AppImageRuntime": {
        "x86_64": "328e0d745c5c6817048c27bc3e8314871703f8f47ffa81a37cb06cd95a94b323",
        "i686": "5cbfd3c7e78d9ebb16b9620b28affcaa172f2166f1ef5fe7ef878699507bcd7f",
        "armhf": "c143d8981702b91cc693e5d31ddd91e8424fec5911fa2dda72082183b2523f47",
        "aarch64": "d2624ce8cc2c64ef76ba986166ad67f07110cdbf85112ace4f91611bc634c96a",
    },
}

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
        self.downloaded: queue.Queue[tuple[str, str, bytes]] = queue.Queue()

    def _download_thread(self, name: str, download_link: str, flavour: str):
        """
        Download a file/resource on a seperate thread
        """
        print(f"Downloading {name} from {download_link}")
        request = urllib.Request(
            download_link,
            headers={"User-Agent": "Chrome/35.0.1916.47 Safari/537.36"},
        )

        download: bytes = b""
        try:
            with urllib.urlopen(request) as downloadobj:
                download = downloadobj.read()

        except OSError:
            # some networking error
            pass

        self.downloaded.put((name, flavour, download))

    def download(self, name: str, download_link: str, flavour: str = ""):
        """
        Download a file
        """
        # Start thread, add it to a set of running threads
        thread = threading.Thread(
            target=self._download_thread,
            name=f"Installer Thread for {name} ({flavour})",
            args=(name, download_link, flavour),
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
                name, flavour, data = self.downloaded.get(timeout=TIMEOUT_PER_LOOP)
            except queue.Empty:
                pass

            else:
                if not data:
                    raise BuildError(
                        f"Failed to download {name} due to some networking error"
                    )

                datahash = hashlib.sha256(data).hexdigest()
                presethash = LINKS_AND_HASHES.get(name)
                if isinstance(presethash, dict):
                    presethash = presethash.get(flavour)

                if datahash != presethash:
                    print("Got hash:", datahash)
                    if presethash is not None:
                        raise BuildError(
                            f"Download hash for {name} ({flavour}) mismatched"
                        )

                    print(
                        f"BuildWarning: Download {name} ({flavour}) has not "
                        "been hash verified!"
                    )

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
    print("This can take a while, depending on your internet speeds...")

    with zipfile.ZipFile(io.BytesIO(downloader.get_one()[1]), "r") as zipped:
        zipped.extractall(deps_dir)

    print()  # newline
    return ret
