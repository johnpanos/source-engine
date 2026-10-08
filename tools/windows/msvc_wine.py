#!/usr/bin/env python3
"""MSVC under Wine: the pinned Windows toolchain on Linux (RFC 0027).

    msvc_wine.py provision [--fragment F]   fetch, verify and install
    msvc_wine.py check [--fragment F]       the install's compiler identity
    msvc_wine.py path [--fragment F]        print the install directory

The pins live in quality/product_profiles/fragments/windows-x86_64-msvc-wine.json
(toolchain.msvc_wine): the mstorsjo/msvc-wine commit and its archive sha256,
the Visual Studio installer manifest by its versioned URL and sha256, the
MSVC and Windows SDK versions and the architectures. vsdownload.py checks
every downloaded payload against that manifest's sha256, so the chain from
the pins to each installed file is verified. The install is published under
dependencies/windows-msvc-wine/<toolset>-<sdk>/ (staged, then renamed) with
a stamp of its pins; provisioning the same pins again does nothing.

Installing accepts the Visual Studio Build Tools license (vsdownload.py
--accept-license); the toolchain is downloaded from Microsoft to this
machine and never redistributed by this repository.
"""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tarfile
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FRAGMENT = ROOT / "quality/product_profiles/fragments/windows-x86_64-msvc-wine.json"
DEPENDENCIES = ROOT / "dependencies" / "windows-msvc-wine"


class ToolchainError(Exception):
    pass


def log(message):
    print("msvc-wine: " + message, file=sys.stderr, flush=True)


def load_pins(fragment=FRAGMENT):
    document = json.loads(Path(fragment).read_text())
    toolchain = document.get("toolchain", {})
    pins = toolchain.get("msvc_wine")
    if not pins:
        raise ToolchainError("%s has no toolchain.msvc_wine" % fragment)
    for key in ("repository", "commit", "archive_sha256", "installer_manifest", "msvc_version",
                "msvc_toolset", "sdk_version", "architectures"):
        if key not in pins:
            raise ToolchainError("toolchain.msvc_wine.%s is required" % key)
    return dict(pins, compiler_version=toolchain.get("version"))


def install_dir(pins):
    return DEPENDENCIES / ("%s-%s" % (pins["msvc_toolset"], pins["sdk_version"]))


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def fetch(url, destination, expected):
    """Downloads url to destination once and checks its sha256."""
    if destination.is_file() and sha256(destination) == expected:
        return destination
    destination.parent.mkdir(parents=True, exist_ok=True)
    partial = destination.with_suffix(destination.suffix + ".partial")
    log("fetching " + url)
    with urllib.request.urlopen(url) as response, open(partial, "wb") as stream:
        shutil.copyfileobj(response, stream)
    actual = sha256(partial)
    if actual != expected:
        partial.unlink()
        raise ToolchainError("%s: sha256 %s is not the pin %s" % (url, actual, expected))
    partial.rename(destination)
    return destination


def stamp_of(pins):
    keys = ("commit", "archive_sha256", "installer_manifest", "msvc_version", "msvc_toolset",
            "sdk_version", "architectures")
    return {key: pins[key] for key in keys}


def installed(pins):
    """The install directory when it holds exactly these pins, else None."""
    directory = install_dir(pins)
    stamp = directory / "msvc-wine-stamp.json"
    if stamp.is_file() and json.loads(stamp.read_text()) == stamp_of(pins):
        return directory
    return None


def provision(pins):
    done = installed(pins)
    if done:
        log("up to date: %s" % done)
        return done
    for tool in ("wine", "msiextract"):
        if not shutil.which(tool):
            raise ToolchainError("%s is required (install wine and msitools)" % tool)
    cache = DEPENDENCIES / "cache"
    archive = fetch("%s/archive/%s.tar.gz" % (pins["repository"], pins["commit"]),
                    cache / ("msvc-wine-%s.tar.gz" % pins["commit"]), pins["archive_sha256"])
    manifest = fetch(pins["installer_manifest"]["url"], cache / "VisualStudio.vsman",
                     pins["installer_manifest"]["sha256"])
    source = DEPENDENCIES / ("msvc-wine-%s" % pins["commit"])
    if not (source / "vsdownload.py").is_file():
        shutil.rmtree(source, ignore_errors=True)
        with tarfile.open(archive) as tar:
            tar.extractall(DEPENDENCIES, filter="data")
        extracted = DEPENDENCIES / ("msvc-wine-%s" % pins["commit"])
        if not (extracted / "vsdownload.py").is_file():
            raise ToolchainError("%s has no vsdownload.py" % archive)
    target = install_dir(pins)
    staging = target.with_name(target.name + ".staging")
    shutil.rmtree(staging, ignore_errors=True)
    command = [sys.executable, str(source / "vsdownload.py"), "--accept-license",
               "--manifest", str(manifest), "--msvc-version", pins["msvc_version"],
               "--sdk-version", pins["sdk_version"], "--cache", str(cache / "payloads"),
               "--dest", str(staging), "--architecture", *pins["architectures"]]
    log("installing MSVC %s and SDK %s (this downloads several GB once)"
        % (pins["msvc_toolset"], pins["sdk_version"]))
    subprocess.run(command, check=True)
    subprocess.run([str(source / "install.sh"), str(staging)], check=True, cwd=source)
    (staging / "msvc-wine-stamp.json").write_text(json.dumps(stamp_of(pins), indent=2) + "\n")
    shutil.rmtree(target, ignore_errors=True)
    staging.rename(target)
    log("installed: %s" % target)
    return target


def identity(directory, architecture="x64"):
    """The `cl` banner of an install (compiles nothing)."""
    cl = directory / "bin" / architecture / "cl"
    if not cl.is_file():
        raise ToolchainError("%s is missing; run msvc_wine.py provision" % cl)
    result = subprocess.run([str(cl)], capture_output=True, text=True, timeout=300,
                            env=dict(os.environ, WINEDEBUG="-all"))
    banner = (result.stderr + result.stdout).strip().splitlines()
    return banner[0] if banner else ""


def check(pins):
    directory = installed(pins)
    if not directory:
        raise ToolchainError("not provisioned for these pins; run msvc_wine.py provision")
    banner = identity(directory)
    expected = pins.get("compiler_version") or ""
    print(banner)
    if expected and ("Version %s." % expected) not in banner:
        raise ToolchainError("cl reports %r; the profile pins %s" % (banner, expected))
    return banner


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("command", choices=("provision", "check", "path"))
    parser.add_argument("--fragment", type=Path, default=FRAGMENT)
    args = parser.parse_args(argv)
    try:
        pins = load_pins(args.fragment)
        if args.command == "provision":
            provision(pins)
        elif args.command == "check":
            check(pins)
        else:
            print(install_dir(pins))
    except (ToolchainError, subprocess.CalledProcessError) as error:
        log(str(error))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
