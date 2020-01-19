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
    parser = argparse.ArgumentParser(description="Builds 2019 vision program")
    parser.add_argument(
        "target",
        choices=["build", "clean"],
        help="'build' compiles the vision program and downloads missing dependencies. 'clean' removes all build artifacts from the build folder.",
    )
    args = parser.parse_args()

    download_file(
        "https://github.com/wpilibsuite/FRCVision-pi-gen/releases/download/v2019.3.1/example-cpp-2019.3.1.zip",
        "build",
    )
    if sys.platform.startswith("linux"):
        download_file(
            "https://github.com/wpilibsuite/raspbian-toolchain/releases/download/v1.3.0/Raspbian9-Linux-Toolchain-6.3.0.tar.gz",
            "build",
        )
    elif sys.platform.startswith("msys"):
        download_file(
            "https://github.com/wpilibsuite/raspbian-toolchain/releases/download/v1.3.0/Raspbian9-Windows-Toolchain-6.3.0.zip",
            "build",
        )
    elif sys.platform.startswith("darwin"):
        download_file(
            "https://github.com/wpilibsuite/raspbian-toolchain/releases/download/v1.3.0/Raspbian9-Max-Toolchain-6.3.0.tar.gz",
            "build",
        )

    nproc = subprocess.check_output("nproc", encoding="utf-8").strip()
    if args.target == "build":
        subprocess.run(["make", "build", f"-j{nproc}"])
    elif args.target == "clean":
        subprocess.run(["make", "clean"])


if __name__ == "__main__":
    main()
