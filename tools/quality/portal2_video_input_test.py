#!/usr/bin/env python3
"""Compile the real Portal 2 list handlers against a parent input-routing fake.

The generated include is an exact source slice, not a copied implementation.
No display, game assets or VGUI singleton is needed. A seeded fallthrough
restores the reported defect and must fail the same behavioral checks.
"""

import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

import conformance

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "game/client/portal2/gameui/portal2/vhybridbutton.cpp"
FIXTURE = "unittests/vguitest/test_portal2_video_input.cpp"
METHODS = (
    "void BaseModHybridButton::OnKeyCodePressed( vgui::KeyCode code )",
    "void BaseModHybridButton::ChangeDialogListSelection( ListSelectionChange_t eNext )",
    "void BaseModHybridButton::SetCurrentSelection( const char *pText )",
    "const char *BaseModHybridButton::GetCurrentSelection()",
)


def source_methods(seed_fallthrough=False):
    source = (ROOT / SOURCE).read_text()
    methods = []
    for signature in METHODS:
        if source.count(signature + "\n{") != 1:
            raise ValueError("expected one production definition: " + signature)
        begin = source.index(signature + "\n{")
        end = source.index("\n}", begin) + len("\n}")
        method = source[begin:end]
        if seed_fallthrough and signature == METHODS[0]:
            # Only the three handled-selection branches; preserve early refusal
            # and IgnoreButtonA routing exactly as production implements them.
            for direction in ("SELECT_NEXT", "SELECT_PREV"):
                method = method.replace(
                    "ChangeDialogListSelection( " + direction + " );\n\t\t\treturn;",
                    "ChangeDialogListSelection( " + direction + " );\n\t\t\tbreak;")
        methods.append(method)
    return "\n\n".join(methods) + "\n"


def run(output, seed_fallthrough=False):
    output.mkdir(parents=True, exist_ok=True)
    (output / "portal2_dialog_input_methods.inc").write_text(source_methods(seed_fallthrough))
    profile = conformance.load_profile(str(ROOT / "quality/profiles"), "linux-headless-core")
    # This SDK-free fixture needs no generated shader headers.
    profile["generated_include_roots"] = []
    executable = output / "portal2_video_input_test"
    command = conformance.build_command(
        str(ROOT), os.environ.get("CONFORMANCE_CXX", "g++"), profile,
        # vhybridbutton.cpp's legacy if ( ( a == b ) ), which clang rejects.
        {"sources": [FIXTURE], "extra_flags": ["-I", str(output), "-Wno-parentheses-equality"]},
        str(executable), config="release")
    (output / "compile-command.json").write_text(json.dumps(command, indent=2) + "\n")
    compiled = subprocess.run(command, capture_output=True, text=True, timeout=60)
    (output / "compile.log").write_text(compiled.stdout + compiled.stderr)
    if compiled.returncode:
        print(compiled.stdout + compiled.stderr)
        return compiled.returncode
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    (output / "run.log").write_text(result.stdout + result.stderr)
    print(result.stdout, end="")
    print(result.stderr, end="", file=sys.stderr)
    return result.returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, default=os.environ.get("CONFORMANCE_OUT"))
    parser.add_argument("--seed-fallthrough", action="store_true")
    args = parser.parse_args()
    if args.out:
        return run(args.out.resolve(), args.seed_fallthrough)
    with tempfile.TemporaryDirectory(prefix="portal2-video-input-") as temporary:
        return run(Path(temporary), args.seed_fallthrough)


if __name__ == "__main__":
    raise SystemExit(main())
