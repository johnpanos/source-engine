#!/usr/bin/env python3
"""Pinned source archives unpacked into dependencies/ (one owner for every pin).

    python3 tools/quality/source_pin.py fetch PIN.json   # download (or reuse) and unpack
    python3 tools/quality/source_pin.py check PIN.json   # verify the unpacked files

A pin file names one archive (url, sha256, size) and the exact files to take
from it, each with its own sha256. `fetch` takes the archive from the cache
when it is there, downloads it otherwise (HTTPS only, bounded by the pinned
size), rejects any archive whose digest differs, and unpacks exactly the
listed files. `check` (also run by the Waf configure steps of the pin's
consumers through verify()) requires every unpacked file to match. Nothing
here reads an unpinned copy.
"""
import argparse
import hashlib
import json
import shutil
import sys
import tarfile
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCHEMAS = ("source-archive-pin/v1", "vulkan-memory-allocator-pin/v1")


class PinError(Exception):
    pass


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def load_pin(path):
    pin = json.loads(Path(path).read_text())
    if pin.get("schema") not in SCHEMAS:
        raise PinError("%s: schema %r is not one of %s" % (path, pin.get("schema"), ", ".join(SCHEMAS)))
    for key in ("directory", "archive_cache", "component", "files"):
        if not pin.get(key):
            raise PinError("%s: no %r" % (path, key))
    for key in ("url", "sha256", "archive_bytes", "cache_archive", "extracted_directory"):
        if not pin["component"].get(key):
            raise PinError("%s: component has no %r" % (path, key))
    pin["_path"] = str(path)
    return pin


def directory(pin, root=ROOT):
    return Path(root) / pin["directory"]


def verify(pin, root=ROOT):
    """Problems with the unpacked copy; empty when every file matches the pin."""
    problems = []
    for relative, expected in sorted(pin["files"].items()):
        path = directory(pin, root) / relative
        if not path.is_file():
            problems.append("%s is missing; run 'python3 tools/quality/source_pin.py fetch %s'"
                            % (path, pin["_path"]))
        elif sha256(path) != expected:
            problems.append("%s: sha256 %s does not match the pin %s" % (path, sha256(path), expected))
    return problems


def _download(url, archive, limit):
    if urllib.parse.urlparse(url).scheme != "https":
        raise PinError("%s is not an HTTPS url" % url)
    print("source_pin: downloading " + url, file=sys.stderr)
    partial = archive.with_suffix(archive.suffix + ".partial")
    with urllib.request.urlopen(url, timeout=60) as response, open(partial, "wb") as out:
        if urllib.parse.urlparse(response.geturl()).scheme != "https":
            raise PinError("%s redirected off HTTPS" % url)
        count = 0
        while block := response.read(1 << 20):
            count += len(block)
            if count > limit:
                raise PinError("%s: download exceeded the pinned %d bytes" % (url, limit))
            out.write(block)
    partial.rename(archive)


def fetch(pin, root=ROOT):
    component = pin["component"]
    archives = Path(root) / pin["archive_cache"]
    archives.mkdir(parents=True, exist_ok=True)
    archive = archives / component["cache_archive"]
    if not archive.is_file():
        _download(component["url"], archive, component["archive_bytes"])
    actual = sha256(archive)
    if actual != component["sha256"]:
        raise PinError("%s: sha256 %s does not match the pin %s" % (archive, actual, component["sha256"]))
    target = directory(pin, root)
    with tarfile.open(archive, "r:gz") as tar:
        for relative in sorted(pin["files"]):
            if relative.startswith("/") or ".." in Path(relative).parts:
                raise PinError("%s: unsafe pinned path %s" % (pin["_path"], relative))
            member = tar.getmember(component["extracted_directory"] + "/" + relative)
            source = tar.extractfile(member) if member.isfile() else None
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
    parser.add_argument("pin", type=Path)
    args = parser.parse_args(argv)
    try:
        pin = load_pin(args.pin)
        if args.command == "fetch":
            print("source_pin: unpacked into %s" % fetch(pin))
            return 0
        problems = verify(pin)
    except PinError as error:
        print("source_pin: " + str(error), file=sys.stderr)
        return 1
    for problem in problems:
        print("source_pin: " + problem, file=sys.stderr)
    if not problems:
        print("source_pin: %s matches the pin" % directory(pin))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
