#!/usr/bin/env python3
"""RFC 0027 L1: no tracked file calls the retired launch and staging surface.

    retired_scan.py check       the repository
    retired_scan.py selftest    seeded callers and history lines

L1 deleted the launchers (./play*, run.sh, run.conf), their helpers
(render_flags.sh, launcher_ccache.sh, ensure_configured.py,
profile_extends.py), the staging scripts (stage_*.py, portal_boot's staging),
private_session.py, transcode_av1.py, the scripts/ CI wrappers and the
launch-equivalence gate that compared the launchers with kiln. A retired path
must not be tracked, and a tracked file that is not documentation (RFC/ and
Markdown describe history) may name one only on a provenance line: one that
says the thing is retired, or what was ported from it. Prints one
`CONFORMANCE <checks> <failures>` record (checks-v1).
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# The L1 retirement cohort (RFC 0027 "Migration and deletions").
RETIRED = (
    "play", "play_p2", "play_p2_fsr", "play_release", "play_p2_release", "play_p2_fsr_release",
    "play_fstop", "play_p2_coop", "run.sh", "run.conf", "run_portal_box3d.sh",
    "tools/quality/render_flags.sh", "tools/quality/launcher_ccache.sh",
    "tools/quality/ensure_configured.py", "tools/quality/profile_extends.py",
    "tools/quality/stage_runtime.py", "tools/quality/stage_portal2_runtime.py",
    "tools/quality/stage_fstop_runtime.py", "tools/quality/private_session.py",
    "tools/video/transcode_av1.py", "tools/kiln/launch_equivalence.py", "tools/kiln/exec_capture.c",
    "scripts/build-ubuntu-amd64.sh", "scripts/build-ubuntu-i386.sh",
    "scripts/tests-ubuntu-amd64.sh", "scripts/tests-ubuntu-i386.sh",
)

# How a caller names them: a launcher run from the root, or a helper's name.
NAMES = re.compile(
    r"(?<![\w/.-])\./(?:play(?:_p2(?:_fsr)?(?:_release)?|_release|_fstop|_p2_coop)?|run\.sh)\b"
    r"|[\"'](?:play_(?:p2(?:_fsr)?(?:_release)?|release|fstop|p2_coop))[\"']"
    r"|/\s*[\"']play[\"']"
    r"|\brun\.conf\b|\brun_portal_box3d\.sh\b"
    r"|\b(?:render_flags|launcher_ccache)\.sh\b"
    r"|\b(?:ensure_configured|profile_extends|stage_runtime|stage_portal2_runtime|"
    r"stage_fstop_runtime|private_session|transcode_av1|launch_equivalence|exec_capture)\b"
    r"|\bscripts/(?:build|tests)-ubuntu-(?:amd64|i386)\.sh\b")

# A provenance line: it records the retirement or what was ported.
PROVENANCE = re.compile(r"\bretired\b|\bported from\b|\bport of\b|\bformerly\b", re.IGNORECASE)

SELF = "tools/kiln/retired_scan.py"


class Record:
    def __init__(self):
        self.checks = 0
        self.failures = 0

    def check(self, condition, name, detail=""):
        self.checks += 1
        if not condition:
            self.failures += 1
            print("FAIL: %s%s" % (name, ": " + detail if detail else ""), file=sys.stderr)
        return condition

    def report(self):
        print("CONFORMANCE %d %d" % (self.checks, self.failures), flush=True)
        return 0 if self.checks and not self.failures else 1


def scanned(path):
    """Callers live in code, scripts, workflows and data, not in history."""
    return not (path.startswith("RFC/") or path.endswith(".md") or path == SELF)


def callers(files):
    """(path, line number, text) of every line that names the retired surface
    without recording its retirement. `files` maps path -> text."""
    found = []
    for path, text in sorted(files.items()):
        if not scanned(path):
            continue
        for number, line in enumerate(text.splitlines(), 1):
            if NAMES.search(line) and not PROVENANCE.search(line):
                found.append((path, number, line.strip()))
    return found


def tracked(root):
    output = subprocess.run(["git", "ls-files", "-z"], cwd=root, capture_output=True,
                            check=True).stdout
    return [name for name in output.decode("utf-8", "replace").split("\0") if name]


def read_texts(root, paths):
    texts = {}
    for path in paths:
        if not scanned(path):
            continue
        full = Path(root) / path
        if not full.is_file() or full.is_symlink():
            continue
        data = full.read_bytes()
        if b"\0" in data[:8192]:
            continue
        texts[path] = data.decode("utf-8", "replace")
    return texts


def check(root=ROOT):
    record = Record()
    paths = tracked(root)
    still = sorted(set(paths) & set(RETIRED))
    record.check(not still, "no retired path is tracked", ", ".join(still))
    found = callers(read_texts(root, paths))
    record.check(not found, "no tracked file calls the retired surface (%d files scanned)"
                 % sum(1 for p in paths if scanned(p)),
                 "; ".join("%s:%d: %s" % hit for hit in found[:20]))
    return record.report()


def selftest():
    record = Record()
    seeded = {
        "tools/quality/harness.py": 'subprocess.run([str(ROOT / "play_p2"), "+map", name])\n',
        "tools/quality/other.py": 'command = ["./play_p2", "-novid"]\n',
        "tools/quality/importer.py": "import stage_portal2_runtime\n",
        ".github/workflows/ci.yml": "      run: scripts/tests-ubuntu-amd64.sh\n",
        "hammer/gtk/app.cpp": '\tconst gchar *argv[] = { "./play", map.c_str(), nullptr };\n',
        "tools/quality/conf.py": "RUN_CONF = ROOT / 'run.conf'\n",
        "tools/quality/launcher.py": "launcher = ROOT / \"play\"\n",
    }
    found = {path for path, _, _ in callers(seeded)}
    for path in sorted(seeded):
        record.check(path in found, "seeded caller caught: " + path)
    history = {
        "product/stage/video_av1.cpp": "//\t\t\t  port of tools/video/transcode_av1.py.\n",
        "quality/x.json": '"description": "the retired ./play_p2 built it"\n',
        "AGENTS.md": "The user played `./play` on 2026-09-26.\n",
        "RFC/0027-progress.md": "`./play_p2` was deleted.\n",
        "tools/quality/unrelated.py": "player = playback_window(rows)  # ./kiln play portal2\n",
        "tools/render/d3d12_lane.py": '(bundle / "run.sh").write_text(script)\n',
        "tools/quality/x.py": 'kiln("play", "portal2", "--dry-run")\n',
    }
    record.check(not callers(history), "history, documentation and unrelated names pass",
                 str(callers(history)))
    return record.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("command", choices=("check", "selftest"))
    args = parser.parse_args(argv)
    return check() if args.command == "check" else selftest()


if __name__ == "__main__":
    sys.exit(main())
