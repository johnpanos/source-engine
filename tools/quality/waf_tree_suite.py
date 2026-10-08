#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run a conformance program that a configured Waf tree builds.

    python3 tools/quality/waf_tree_suite.py --tree build \
        --target legacy_render_provider_conformance \
        --program unittests/shaderextensiontest/legacy_render_provider_conformance

Some suites link the legacy module stack (the material system, a shader
backend, tier libraries) that only a product tree builds, so the runner cannot
compile them from sources. This command builds the target incrementally in
the tree (from inside it: each tree keeps its own Waf lock), then runs the
program with the tree's shared-library directories on LD_LIBRARY_PATH and
passes its output through. The program's own `CONFORMANCE <checks> <failures>`
record is the result; a failed build or a missing program prints a failing
record instead. Python 3 standard library only.
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def library_dirs(tree):
    dirs = sorted({str(path.parent) for path in tree.rglob("*.so") if path.is_file()})
    return ":".join(dirs)


def fail(reason):
    print("FAIL waf-tree-suite: %s" % reason)
    print("CONFORMANCE 1 1")
    return 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--tree", required=True, help="configured Waf output directory")
    parser.add_argument("--target", required=True, help="Waf target to build first")
    parser.add_argument("--program", required=True, help="the program, relative to the tree")
    parser.add_argument("--timeout", type=int, default=1800, help="build timeout in seconds")
    args = parser.parse_args(argv)

    tree = (ROOT / args.tree).resolve()
    if not (tree / "c4che").is_dir():
        return fail("%s is not a configured Waf tree" % args.tree)
    build = subprocess.run([sys.executable, str(ROOT / "waf"), "build", "--targets=" + args.target],
                           cwd=tree, capture_output=True, text=True, timeout=args.timeout)
    if build.returncode != 0:
        sys.stdout.write((build.stdout + build.stderr)[-4000:])
        return fail("building %s in %s failed" % (args.target, args.tree))
    program = tree / args.program
    if not program.is_file():
        return fail("%s was not built" % program)
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = library_dirs(tree) + (":" + env["LD_LIBRARY_PATH"]
                                                   if env.get("LD_LIBRARY_PATH") else "")
    run = subprocess.run([str(program)], cwd=tree, env=env)
    return run.returncode


if __name__ == "__main__":
    sys.exit(main())
