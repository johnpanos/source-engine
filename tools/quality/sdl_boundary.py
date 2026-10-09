#!/usr/bin/env python3
"""Check that SDL stays private to its declared owners (RFC 0001 R18).

Every first-party C, C++ and Objective-C file that includes an SDL header or calls
an SDL function must be listed in architecture/sdl_boundary.json, by file or by
prefix. The list is exact and shrink-only: a new user fails, and a listed file
that no longer uses SDL fails until it is removed. Comments do not count; opaque
type names (SDL_Window, SDL_Cursor) are not uses.

  sdl_boundary.py check [--root DIR]
"""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
POLICY = "architecture/sdl_boundary.json"
SUFFIXES = (".c", ".cc", ".cpp", ".h", ".hpp", ".m", ".mm")
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]SDL[0-9]?[/A-Za-z0-9_.]*\.h[>"]', re.M)
CALL = re.compile(r"\bSDL_[A-Za-z0-9_]+\s*\(")
COMMENT = re.compile(r"//[^\n]*|/\*.*?\*/", re.S)
STRING = re.compile(r'"(?:\\.|[^"\\\n])*"')


def uses_sdl(text):
    if INCLUDE.search(text):
        return True
    code = COMMENT.sub(" ", STRING.sub('""', text))
    return CALL.search(code) is not None


def source_files(root):
    listed = subprocess.run(["git", "ls-files", "-co", "--exclude-standard"], cwd=root,
                            capture_output=True, text=True, check=True).stdout.split("\n")
    return sorted(p for p in listed if p.endswith(SUFFIXES) and (root / p).is_file())


def check(root):
    policy = json.loads((root / POLICY).read_text())
    excluded = tuple(policy["excluded_prefixes"])
    prefixes = tuple(policy["allowed_prefixes"])
    files = set(policy["allowed_files"])
    users = set()
    for path in source_files(root):
        if path.startswith(excluded):
            continue
        if uses_sdl((root / path).read_text(encoding="latin-1")):
            users.add(path)
    errors = []
    for path in sorted(users):
        if path not in files and not path.startswith(prefixes):
            errors.append("%s: uses SDL but is not an SDL owner in %s; ask the launcher "
                          "(ILauncherPlatformServices) instead" % (path, POLICY))
    for path in sorted(files - users):
        errors.append("%s: listed in %s but no longer uses SDL; remove it" % (path, POLICY))
    for line in errors:
        print("SDL001 " + line)
    print("sdl_boundary: %d SDL users, %d errors" % (len(users), len(errors)))
    return 1 if errors else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("command", choices=["check"])
    parser.add_argument("--root", type=Path, default=ROOT)
    args = parser.parse_args()
    return check(args.root.resolve())


if __name__ == "__main__":
    sys.exit(main())
