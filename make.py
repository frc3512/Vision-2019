#!/usr/bin/env python3

import argparse
import os
import subprocess
import sys
import tarfile
from urllib.request import urlretrieve


def dl_progress(count, block_size, total_size):
    percent = min(int(count * block_size * 100 / total_size), 100)
    sys.stdout.write(f"\r-> {percent}%")
    sys.stdout.flush()


def download_file(file_url, dest_dir):
    """Download file from maven server.
    Keyword arguments:
    file_url -- file to download
    dest_dir -- destination directory for file
    """
    filename = os.path.split(file_url)[1]
    dest_file = f"{dest_dir}/{filename}"
    if os.path.splitext(filename)[1] == ".gz":
        folder_name = f"{dest_dir}/{os.path.splitext(os.path.splitext(filename)[0])[0]}"
    else:
        folder_name = f"{dest_dir}/{os.path.splitext(filename)[0]}"
    if not os.path.exists(folder_name):
        os.makedirs(folder_name)
        print(f"Downloading {filename}...")
        print(f"Querying {file_url}")
        urlretrieve(url=file_url, filename=dest_file, reporthook=dl_progress)
        print(" done.")
        print(f"Unzipping {dest_file}...", end="")
        sys.stdout.flush()
        if os.path.splitext(filename)[1] == ".gz":
            tar = tarfile.open(dest_file)
            tar.extractall(path=folder_name)
            tar.close()
        else:
            subprocess.run(["unzip", "-q", "-d", folder_name, f"{dest_file}"])
        print(" done.")


def main():
    download_file(
        "https://github.com/wpilibsuite/FRCVision-pi-gen/releases/download/v2019.3.1/example-cpp-2019.3.1.zip",
        "build",
    )
    download_file(
        "https://github.com/wpilibsuite/raspbian-toolchain/releases/download/v1.3.0/Raspbian9-Linux-Toolchain-6.3.0.tar.gz",
        "build")
    subprocess.run(["make"])


if __name__ == "__main__":
    main()
