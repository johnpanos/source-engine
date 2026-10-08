#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""RFC 0003 J3 runtime thread census, with a shrink-only ratchet.

    python3 tools/quality/thread_census.py run [--profile portal] \\
        --out <dir> [--compute-workers N] [--map testchmb_a_00] [--write]
    python3 tools/quality/thread_census.py check --log <console.log> [--compute-workers N]
    python3 tools/quality/thread_census.py sensitivity

RFC/0003-dependency-aware-job-system.md#frame-wide-scheduling-goals-amended-2026-09-28,
goal J3 "Providers borrow the root's pool": every live thread of a running
product belongs to a declared owner (main, the compute pool, MatQueue,
filesystem I/O, audio, a device or driver thread, or a declared blocking
lane). `run` boots the Portal product headless through portal_boot.py and runs
the engine's `thread_census` console command after the map loads; `check`
reads its "threadcensus <count> <name>" lines (OS names, pool indices
stripped) from a console log.

quality/budgets/thread-census-v1.json owns the classification: each declared
owner is a name pattern with an owner and a reason; the executable's own
name is the main thread, exactly once (more threads with that name were
started without one and are undeclared). Every other thread is undeclared and
must equal the recorded "undeclared" counts exactly: a new name or a higher
count fails, and so does a lower count not yet recorded (--write records it
after review), so the list only shrinks. J3 needs it empty. With
--compute-workers N (the engine's -compute_workers, RFC 0003 J5) the census
must show exactly N CmpJob workers. A census without its total line, or whose
counts do not add up to it, is incomplete and fails. Ends with one checks-v1
record. Python 3 standard library only.
"""

import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
from conformance_result import Checks  # noqa: E402

SCHEMA = "thread-census/v1"
DECLARATION = "quality/budgets/thread-census-v1.json"
LINE = re.compile(r"^threadcensus (\d+) (.+?)\s*$")
TOTAL = re.compile(r"^threadcensus total (\d+)\s*$")
COMPUTE_POOL = "CmpJob"


def parse(text):
    """{name: count} and the reported total (None when missing)."""
    counts, total = {}, None
    for line in text.splitlines():
        m = TOTAL.match(line)
        if m:
            total = int(m.group(1))
            continue
        m = LINE.match(line)
        if m:
            counts[m.group(2)] = counts.get(m.group(2), 0) + int(m.group(1))
    return counts, total


def load(root):
    data = json.loads((Path(root) / DECLARATION).read_text())
    if data.get("schema") != SCHEMA:
        raise ValueError("%s: schema must be %s" % (DECLARATION, SCHEMA))
    return data


def classify(counts, declaration):
    """Declared {owner: count}, undeclared {name: count}."""
    main = declaration["main_thread"]
    owners = [(re.compile(o["pattern"]), o) for o in declaration["owners"]]
    declared, undeclared = {}, {}
    for name, n in sorted(counts.items()):
        if name == main:
            declared["main"] = declared.get("main", 0) + 1
            if n > 1:
                undeclared[name + " (unnamed)"] = n - 1
            continue
        for pattern, owner in owners:
            if pattern.search(name):
                declared[owner["owner"]] = declared.get(owner["owner"], 0) + n
                break
        else:
            undeclared[name] = n
    return declared, undeclared


def check(text, declaration, checks, compute_workers=None, root=None, write=False):
    counts, total = parse(text)
    checks.check(total is not None and total > 0, "thread-census.complete",
                 "no 'threadcensus total' line: the census did not run or was cut short")
    checks.check(total is None or sum(counts.values()) == total, "thread-census.adds-up",
                 "%d thread(s) listed, total says %s" % (sum(counts.values()), total))
    for owner in declaration["owners"]:
        checks.check(bool(str(owner.get("reason", "")).strip()) and bool(owner.get("owner")),
                     "thread-census.owner-declared",
                     "pattern %r has no owner or reason" % owner.get("pattern"))
    declared, undeclared = classify(counts, declaration)
    checks.check(declared.get("main", 0) == 1, "thread-census.main",
                 "the main thread %r was not seen" % declaration["main_thread"])
    if compute_workers is not None:
        seen = counts.get(COMPUTE_POOL, 0)
        checks.check(seen == compute_workers, "thread-census.compute-workers",
                     "-compute_workers %d but %d %s worker(s)" % (compute_workers, seen,
                                                                   COMPUTE_POOL))
    if write and root is not None:
        data = dict(declaration, undeclared=dict(sorted(undeclared.items())))
        (Path(root) / DECLARATION).write_text(json.dumps(data, indent=2) + "\n")
        print("thread-census: recorded %d undeclared thread(s)" % sum(undeclared.values()))
        declaration = data
    recorded = declaration.get("undeclared", {})
    grew, shrank = [], []
    for name in sorted(set(undeclared) | set(recorded)):
        c, r = undeclared.get(name, 0), recorded.get(name, 0)
        if c > r:
            grew.append("%s: %d, the ratchet allows %d" % (name, c, r))
        elif c < r:
            shrank.append("%s: down to %d from %d; record it with --write" % (name, c, r))
    for line in grew:
        print("thread-census: undeclared:", line)
    for line in shrank:
        print("thread-census: stale:", line)
    checks.check(not grew, "thread-census.no-new-threads",
                 "%d undeclared thread name(s) grew or appeared" % len(grew))
    checks.check(not shrank, "thread-census.exact",
                 "%d undeclared count(s) shrank without a ratchet update" % len(shrank))
    for owner, n in sorted(declared.items()):
        print("INFO thread-census: %s %d" % (owner, n))
    print("INFO thread-census: undeclared %d; J3 needs 0" % sum(undeclared.values()))


def run(args, checks):
    out = Path(args.out)
    command = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"),
               "--profile", args.profile, "--flavor", args.flavor, "--out", str(out),
               "--headless", "--renderer", "native-vulkan", "--map", args.map,
               "--console-command", "thread_census"]
    if args.compute_workers is not None:
        command += ["--engine-arg=-compute_workers", "--engine-arg=%d" % args.compute_workers]
    boot = subprocess.run(command, capture_output=True, text=True, timeout=args.timeout)
    checks.check(boot.returncode == 0, "thread-census.boot",
                 "portal_boot.py exited %d: %s" % (boot.returncode, boot.stdout[-400:]))
    log = out / "runtime/portal/console.log"
    text = log.read_text(errors="replace") if log.exists() else ""
    check(text, load(ROOT), checks, args.compute_workers, ROOT, args.write)


SAMPLE = """threadcensus 1 AchievementSave
threadcensus 3 CmpJob
threadcensus 4 IOJob
threadcensus 1 SDLAudioP
threadcensus 2 hl2_lau:disk$
threadcensus 2 hl2_launcher
threadcensus total 13
"""


def sensitivity():
    checks = Checks()
    declaration = {
        "schema": SCHEMA, "main_thread": "hl2_launcher",
        "owners": [
            {"pattern": "^CmpJob$", "owner": "compute pool", "reason": "the root's pool"},
            {"pattern": "^IOJob$", "owner": "filesystem I/O", "reason": "async file I/O"},
            {"pattern": "^SDL.*Audio", "owner": "audio", "reason": "SDL audio device"},
            {"pattern": ":disk\\$$", "owner": "driver", "reason": "Mesa shader disk cache"}],
        "undeclared": {"AchievementSave": 1, "hl2_launcher (unnamed)": 1}}
    clean = Checks()
    check(SAMPLE, declaration, clean, compute_workers=3)
    checks.check(clean.failures == 0, "control.clean-census-passes",
                 "%d failure(s)" % clean.failures)
    _, undeclared = classify(parse(SAMPLE)[0], declaration)
    checks.check(undeclared == {"AchievementSave": 1, "hl2_launcher (unnamed)": 1},
                 "control.classification", str(undeclared))

    def seeded(label, text, decl=None, workers=3):
        result = Checks()
        check(text, decl or declaration, result, compute_workers=workers)
        checks.check(result.failures > 0, "detects.%s" % label, "the census passed")

    seeded("new-provider-thread", SAMPLE.replace("total 13", "total 14") +
           "threadcensus 1 CullWorker\n")
    seeded("extra-unnamed-thread", SAMPLE.replace("2 hl2_launcher", "3 hl2_launcher")
           .replace("total 13", "total 14"))
    seeded("more-undeclared", SAMPLE.replace("1 AchievementSave", "2 AchievementSave")
           .replace("total 13", "total 14"))
    seeded("stale-ratchet", SAMPLE.replace("threadcensus 1 AchievementSave\n", "")
           .replace("total 13", "total 12"))
    seeded("compute-workers-mismatch", SAMPLE, workers=1)
    seeded("no-total", SAMPLE.replace("threadcensus total 13\n", ""))
    seeded("total-mismatch", SAMPLE.replace("total 13", "total 15"))
    seeded("no-main", SAMPLE.replace("2 hl2_launcher", "2 other_exe"))
    seeded("empty-log", "")
    seeded("owner-without-reason", SAMPLE, dict(declaration, owners=declaration["owners"] + [
        {"pattern": "^MatQueue$", "owner": "MatQueue", "reason": ""}]))
    with tempfile.TemporaryDirectory() as tmp:
        (Path(tmp) / "quality/budgets").mkdir(parents=True)
        (Path(tmp) / DECLARATION).write_text(json.dumps(declaration))
        shrunk = SAMPLE.replace("threadcensus 1 AchievementSave\n", "").replace("total 13",
                                                                               "total 12")
        written = Checks()
        check(shrunk, load(tmp), written, 3, tmp, write=True)
        checks.check(written.failures == 0 and "AchievementSave" not in load(tmp)["undeclared"],
                     "control.write-records-shrink", "%d failure(s)" % written.failures)
    return checks


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    r = sub.add_parser("run")
    r.add_argument("--profile", default="portal", help="kiln profile (default portal)")
    r.add_argument("--flavor", default="dev", help="the profile's build flavor")
    r.add_argument("--out", required=True)
    r.add_argument("--map", default="testchmb_a_00")
    r.add_argument("--compute-workers", type=int)
    r.add_argument("--timeout", type=int, default=600)
    r.add_argument("--write", action="store_true", help="record the undeclared counts")
    c = sub.add_parser("check")
    c.add_argument("--log", required=True)
    c.add_argument("--compute-workers", type=int)
    sub.add_parser("sensitivity")
    args = parser.parse_args(argv)
    checks = Checks()
    if args.command == "run":
        run(args, checks)
    elif args.command == "check":
        check(Path(args.log).read_text(errors="replace"), load(ROOT), checks,
              args.compute_workers)
    else:
        checks = sensitivity()
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
