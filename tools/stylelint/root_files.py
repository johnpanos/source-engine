#!/usr/bin/env python3
"""The repository root's tracked files (RFC 0027 "Repository root").

    python3 tools/stylelint/root_files.py

After the L1 cutover the tracked root files are limited to an allowlist: the
build entry points, the documentation and licence files, configuration and
directories (submodules count as directories). Files a later phase deletes
are named in PENDING with that phase; the list only shrinks: an entry whose
file is gone fails until it is removed here. Prints the violations and exits
1, or exits 0 with nothing to report.
"""

import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

ALLOWED = frozenset({
    # Build entry points.
    "wscript", "waf", "waf.bat", "kiln",
    # Documentation and licences.
    "README.md", "AGENTS.md", "LIMITS.md", "LICENSE", "thirdpartylegalnotices.txt",
    # Configuration.
    ".clang-format", ".gitignore", ".gitmodules",
})

# Root files a later RFC 0027 phase replaces with kiln, by phase. Shrink-only.
PENDING = {
    "build-android-apk.sh": "L7 (kiln with android-apk and adb)",
    "build-android-portal2-apk.sh": "L7 (kiln with android-apk and adb)",
    "build-apple-app.sh": "L7 (kiln with apple-app and apple-relay)",
    "build-ios-app.sh": "L7 (kiln with apple-app and apple-relay)",
    "build-ios-portal2-app.sh": "L7 (kiln with apple-app and apple-relay)",
    "build-macos-app.sh": "L7 (kiln with apple-app and apple-relay)",
    "build-tvos-app.sh": "L7 (kiln with apple-app and apple-relay)",
    "build-tvos-portal2-app.sh": "L7 (kiln with apple-app and apple-relay)",
    "ios-deploy.sh": "L7 (kiln with apple-app and apple-relay)",
}


def tracked_root_entries(root):
    """{name: is_directory} for every tracked path at the root (a submodule's
    gitlink, mode 160000, is a directory)."""
    output = subprocess.run(["git", "ls-files", "-s", "-z"], cwd=root, capture_output=True,
                            check=True).stdout
    entries = {}
    for record in output.split(b"\0"):
        if not record:
            continue
        meta, path = record.split(b"\t", 1)
        name = os.fsdecode(path)
        top, _, rest = name.partition("/")
        mode = meta.split(b" ", 1)[0]
        entries[top] = entries.get(top, False) or bool(rest) or mode == b"160000"
    return entries


def violations(entries, allowed=ALLOWED, pending=PENDING):
    problems = []
    for name, is_directory in sorted(entries.items()):
        if is_directory or name in allowed or name in pending:
            continue
        problems.append("%s: not an allowed root file (RFC 0027 repository root allowlist)" % name)
    for name, phase in sorted(pending.items()):
        if name not in entries:
            problems.append("%s: gone; remove its PENDING entry (%s); the list only shrinks"
                            % (name, phase))
    return problems


def main(argv=None):
    root = Path(argv[0]) if argv else ROOT
    problems = violations(tracked_root_entries(root))
    for problem in problems:
        print("root_files: " + problem)
    if not problems:
        print("root_files: every tracked root file is allowed (%d pending later phases)"
              % len(PENDING))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
