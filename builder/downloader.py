"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/downloader.py
Defines a ThreadedDownloader that helps in downloading dependencies in the
background
"""


from builder.utils import BuildError
import queue
import threading
import urllib.request as urllib
from typing import Optional


# Downloads timeout in seconds
DOWNLOAD_TIMEOUT = 600
TIMEOUT_PER_LOOP = 0.05


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
            print(f"Failed to download {name} due to some networking error")
            self.downloaded.put((name, None))

        else:
            self.downloaded.put((name, download))

    def download(self, name: str, download_link: str):
        """
        Download a file
        """
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
        waiting for threads to complete. data being None indicates error in
        download
        """
        loops = 0
        while self.is_downloading() or not self.downloaded.empty():
            try:
                # Do timeout and loop because we need be able to handle any
                # potential KeyboardInterrupt errors
                yield self.downloaded.get(timeout=TIMEOUT_PER_LOOP)
            except queue.Empty:
                pass

            loops += 1
            if loops * TIMEOUT_PER_LOOP > DOWNLOAD_TIMEOUT:
                ret = [(t.name, None) for t in self.threads if t.is_alive()]
                print(
                    "Download(s) timed out!\n"
                    f"Took longer than {DOWNLOAD_TIMEOUT} seconds.\n"
                    "Skipping download(s), continuing with compilation..."
                )
                yield from ret
                break

    def get_one(self):
        """
        Get one downloaded resource (name-data pair). Function can block while
        waiting for resource. If download failed, raises error.
        """
        for name, data in self.get_finished():
            if data is None:
                raise BuildError(f"Downloads failed")

            print(f"Successfully downloaded {name}!")
            return name, data

        raise BuildError(f"Failed to fetch downloads as all were completed")
