#!/usr/bin/env python3
"""Reconfigure a Waf output tree when its configuration inputs changed.

A tree configured before a wscript, Waf tool, toolchain policy or profile change
keeps the old flags and then fails the build (for example TOOLCHAIN002 after a
target moved to C++20). This replays the tree's own stored configure options
(<out>/configuration.py, written by the `reconfigure` Waf tool) with
`waf configure --rebuild-cache` whenever an input is newer than <out>/c4che.

    python3 tools/quality/ensure_configured.py --build build-portal-vulkan

Lock files are written only into the output tree, so shared lock files in the
repository root (used by other trees) are never replaced.
"""

import argparse
import ast
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
# Configure-time inputs besides the wscripts Waf records in the tree's lock file.
INPUT_GLOBS = (
    "waf",
    "scripts/waifulib/**/*.py",
    "quality/toolchain/**/*",
    "quality/profiles/*.json",
    "quality/product_profiles/*.json",
    "architecture/modules.json",
    "tools/quality/toolchain_policy.py",
    "tools/quality/product_profile.py",
)
LOCK = ".lock-waf_%s_build" % sys.platform


def recorded_wscripts(build):
    """Wscripts the last configure of this tree read (Waf's `files`)."""
    lock = build / LOCK
    if not lock.is_file():
        return None
    for line in lock.read_text().splitlines():
        key, separator, literal = line.partition(" = ")
        if separator and key == "files":
            return [Path(item) for item in ast.literal_eval(literal)]
    return None


def stale_inputs(build, root=ROOT):
    """Return the inputs newer than the tree's configuration (empty when current)."""
    build, root = Path(build).resolve(), Path(root).resolve()
    stamp = build / "c4che" / "_cache.py"
    if not (build / "configuration.py").is_file():
        raise ValueError("%s has no stored configuration; configure it with ./waf configure -o %s"
                         % (build, build))
    wscripts = recorded_wscripts(build)
    if not stamp.is_file() or wscripts is None:
        return [stamp]
    configured = stamp.stat().st_mtime
    inputs = set(wscripts)
    for pattern in INPUT_GLOBS:
        inputs.update(path for path in root.glob(pattern) if path.is_file())
    return sorted(path for path in inputs
                  if not path.is_file() or path.stat().st_mtime > configured)


def reconfigure(build, root=ROOT):
    build, root = Path(build).resolve(), Path(root).resolve()
    command = [sys.executable, str(root / "waf"), "configure", "--rebuild-cache",
               "--no-lock-in-run", "--no-lock-in-top", "-o", str(build)]
    subprocess.run(command, cwd=root, check=True)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build", required=True, help="Waf output tree")
    parser.add_argument("--check", action="store_true",
                        help="only report; exit 1 when the tree is stale")
    args = parser.parse_args(argv)
    try:
        stale = stale_inputs(args.build)
    except ValueError as error:
        print("ensure_configured: " + str(error), file=sys.stderr)
        return 2
    if not stale:
        return 0
    root = ROOT.resolve()
    shown = ", ".join(os.path.relpath(path, root) for path in stale[:3])
    more = " (+%d more)" % (len(stale) - 3) if len(stale) > 3 else ""
    print("ensure_configured: %s is older than %s%s" % (args.build, shown, more))
    if args.check:
        return 1
    print("ensure_configured: reconfiguring %s from its stored options" % args.build)
    try:
        reconfigure(args.build)
    except subprocess.CalledProcessError as error:
        print("ensure_configured: reconfigure failed (%d)" % error.returncode, file=sys.stderr)
        return error.returncode or 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
