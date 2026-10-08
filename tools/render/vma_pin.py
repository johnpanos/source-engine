#!/usr/bin/env python3
"""The pinned Vulkan Memory Allocator (RFC 0016 decision 4, K1).

    python3 tools/render/vma_pin.py fetch   # download (or reuse) and unpack
    python3 tools/render/vma_pin.py check   # verify the unpacked files

quality/toolchain/vulkan-memory-allocator.json owns the pin. `fetch` takes the
archive from the cache when it is there, downloads it otherwise, rejects any
archive whose sha256 differs from the pin, and unpacks exactly the pinned files.
`check` (also run by Waf's configure step through verify()) requires every
unpacked file to match its recorded digest. Nothing here ever reads an
unpinned copy.
"""
import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import source_pin  # noqa: E402  (the one owner of pinned source archives)

PIN = ROOT / "quality" / "toolchain" / "vulkan-memory-allocator.json"
PinError = source_pin.PinError


def load_pin(path=PIN):
    return source_pin.load_pin(path)


def include_directory(pin=None, root=ROOT):
    return source_pin.directory(pin or load_pin(), root) / "include"


def verify(pin=None, root=ROOT):
    """Problems with the unpacked copy; empty when every file matches the pin."""
    return source_pin.verify(pin or load_pin(), root)


def fetch(pin=None, root=ROOT):
    return source_pin.fetch(pin or load_pin(), root)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=("fetch", "check"))
    args = parser.parse_args(argv)
    try:
        if args.command == "fetch":
            print("vma_pin: unpacked into %s" % fetch())
            return 0
        problems = verify()
    except PinError as error:
        print("vma_pin: " + str(error), file=sys.stderr)
        return 1
    for problem in problems:
        print("vma_pin: " + problem, file=sys.stderr)
    if not problems:
        print("vma_pin: %s matches the pin" % include_directory())
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
