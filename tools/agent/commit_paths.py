#!/usr/bin/env python3
"""Commit only your own changes from a checkout that several agents share.

A plain `git commit` records the whole shared index, including anything
another agent has staged. This tool builds the commit in a private index
that starts from HEAD and holds only the given paths (whole files) and
patches (your hunks of a shared file). It checks that the commit touches
exactly those paths, commits, and then points the shared index at the new
HEAD for those paths alone. Other agents' staged and unstaged work is left
untouched.

  commit_paths.py -m "Portal 2 audio: ..." engine/audio/snd_mixgroups.cpp \\
      --patch my-manifest-hunks.patch

Paths may be new, modified or deleted files; a directory expands to its
changed files. Each patch is `git diff` output limited to your hunks and must
apply cleanly to HEAD.
"""

import argparse
import os
import subprocess
import sys
import tempfile


def git(*args, env=None, check=True, capture=True):
    result = subprocess.run(["git", *args], env=env, text=True,
                            capture_output=capture)
    if check and result.returncode != 0:
        raise SystemExit("git %s failed:\n%s" % (" ".join(args), result.stderr.strip()))
    return result.stdout if capture else ""


def expand(paths):
    """Changed files under each path (tracked changes, deletions and new files)."""
    files = set()
    for path in paths:
        listed = git("status", "--porcelain=v1", "-uall", "--no-renames", "--", path)
        for line in listed.splitlines():
            files.add(line[3:])
        if not listed and os.path.isfile(path):
            files.add(path)  # unchanged file: nothing to commit, reported below
    return sorted(files)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-m", "--message", required=True)
    parser.add_argument("--patch", action="append", default=[],
                        help="a patch of your hunks in a shared file (repeatable)")
    parser.add_argument("paths", nargs="*", help="your files or directories")
    args = parser.parse_args()
    if not args.paths and not args.patch:
        parser.error("give at least one path or --patch")

    root = git("rev-parse", "--show-toplevel").strip()
    os.chdir(root)
    files = expand(args.paths)
    head = git("rev-parse", "HEAD").strip()

    fd, index = tempfile.mkstemp(prefix="commit-paths-index-")
    os.close(fd)
    os.unlink(index)
    env = dict(os.environ, GIT_INDEX_FILE=index)
    try:
        git("read-tree", head, env=env)
        for path in files:
            if os.path.lexists(path):
                git("add", "--", path, env=env)
            else:
                git("rm", "--cached", "--quiet", "--ignore-unmatch", "--", path, env=env)
        for patch in args.patch:
            git("apply", "--cached", "--unidiff-zero", "--", patch, env=env)

        staged = git("diff", "--cached", "--name-only", head, env=env).split()
        if not staged:
            print("commit_paths: nothing to commit (already in HEAD)")
            return 0
        patched = set()
        for patch in args.patch:
            patched.update(git("apply", "--numstat", "--unidiff-zero", "--", patch).split()[2::3])
        unexpected = sorted(set(staged) - set(files) - patched)
        if unexpected:
            raise SystemExit("commit_paths: refusing, the commit would also touch: " +
                             ", ".join(unexpected))
        if git("rev-parse", "HEAD").strip() != head:
            raise SystemExit("commit_paths: HEAD moved while staging; run again")
        print("commit_paths: committing %d file(s):\n  %s" % (len(staged), "\n  ".join(staged)))
        git("commit", "--quiet", "-m", args.message, env=env)
    finally:
        if os.path.exists(index):
            os.unlink(index)

    # The shared index still holds the old HEAD's entries for these files;
    # align just those entries with the new commit.
    git("restore", "--staged", "--", *staged)
    print("commit_paths: " + git("log", "--oneline", "-1").strip())
    return 0


if __name__ == "__main__":
    sys.exit(main())
