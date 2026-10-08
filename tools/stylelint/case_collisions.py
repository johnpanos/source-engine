#!/usr/bin/env python3
"""No two tracked paths may differ only in letter case.

    python3 tools/stylelint/case_collisions.py

Windows and macOS check out onto case-insensitive file systems, where such a
pair is one file: the last one written wins. A lowercase forwarding header
beside its real header (iappsystem.h including IAppSystem.h) therefore
becomes a header that includes itself, and the Windows build breaks while
Linux passes. Spell includes with the real file's case instead. Paths under
external/ and games/ (imported trees with their own history) are not
checked. Prints each collision and exits 1, or exits 0.
"""

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXEMPT_PREFIXES = ("external/", "games/")


def tracked(root):
    output = subprocess.run(["git", "ls-files", "-z"], cwd=root, capture_output=True,
                            check=True).stdout
    return [name for name in output.decode("utf-8", "replace").split("\0") if name]


def collisions(paths):
    """Groups of two or more paths equal ignoring case, sorted."""
    groups = {}
    for path in paths:
        if path.startswith(EXEMPT_PREFIXES):
            continue
        groups.setdefault(path.lower(), []).append(path)
    return sorted(sorted(group) for group in groups.values() if len(group) > 1)


def main():
    found = collisions(tracked(ROOT))
    for group in found:
        print("case collision: " + " and ".join(group))
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
