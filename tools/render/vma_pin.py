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
import hashlib
import json
import shutil
import sys
import tarfile
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PIN = ROOT / "quality" / "toolchain" / "vulkan-memory-allocator.json"
SCHEMA = "vulkan-memory-allocator-pin/v1"


class PinError(Exception):
    pass


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def load_pin(path=PIN):
    pin = json.loads(Path(path).read_text())
    if pin.get("schema") != SCHEMA:
        raise PinError("%s: schema %r is not %s" % (path, pin.get("schema"), SCHEMA))
    for key in ("directory", "archive_cache", "component", "files"):
        if not pin.get(key):
            raise PinError("%s: no %r" % (path, key))
    for key in ("commit", "url", "sha256", "cache_archive", "extracted_directory"):
        if not pin["component"].get(key):
            raise PinError("%s: component has no %r" % (path, key))
    return pin


def include_directory(pin=None, root=ROOT):
    pin = pin or load_pin()
    return Path(root) / pin["directory"] / "include"


def verify(pin=None, root=ROOT):
    """Problems with the unpacked copy; empty when every file matches the pin."""
    pin = pin or load_pin()
    problems = []
    for relative, expected in sorted(pin["files"].items()):
        path = Path(root) / pin["directory"] / relative
        if not path.is_file():
            problems.append("%s is missing; run 'python3 tools/render/vma_pin.py fetch'" % path)
        elif sha256(path) != expected:
            problems.append("%s: sha256 %s does not match the pin %s" % (path, sha256(path), expected))
    return problems


def fetch(pin=None, root=ROOT):
    pin = pin or load_pin()
    component = pin["component"]
    archives = Path(root) / pin["archive_cache"]
    archives.mkdir(parents=True, exist_ok=True)
    archive = archives / component["cache_archive"]
    if not archive.is_file():
        print("vma_pin: downloading " + component["url"], file=sys.stderr)
        partial = archive.with_suffix(archive.suffix + ".partial")
        with urllib.request.urlopen(component["url"]) as response, open(partial, "wb") as out:
            shutil.copyfileobj(response, out)
        partial.rename(archive)
    actual = sha256(archive)
    if actual != component["sha256"]:
        raise PinError("%s: sha256 %s does not match the pin %s" % (archive, actual, component["sha256"]))
    target = Path(root) / pin["directory"]
    with tarfile.open(archive, "r:gz") as tar:
        for relative in sorted(pin["files"]):
            member = tar.getmember(component["extracted_directory"] + "/" + relative)
            source = tar.extractfile(member)
            if source is None:
                raise PinError("%s: %s is not a file" % (archive, relative))
            destination = target / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            with open(destination, "wb") as out:
                shutil.copyfileobj(source, out)
    problems = verify(pin, root)
    if problems:
        raise PinError("; ".join(problems))
    return target


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
