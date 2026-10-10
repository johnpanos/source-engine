#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The queued TSan lane for RFC 0016 K3 ("Threading").

    python3 tools/render/tsan_triage.py run [--profile portal-tsan-linux] \\
        --out DIR [--mode 0 --mode 2] [--map M]
        [--content-root DIR]
    python3 tools/render/tsan_triage.py check --logs DIR [--logs DIR ...]
    python3 tools/render/tsan_triage.py selftest

  run        boots the ThreadSanitizer profile (portal-tsan-linux: clang,
             --sanitize=thread; kiln builds and packages it) headless through
             portal_boot.py, once per material-system queue mode, with TSan
             writing its reports under DIR/mode<N>/tsan, then checks them.
  check      parses every TSan report in the given directories into
             signatures (the kind plus the innermost first-party frame of
             each access stack) and matches each against the reviewed
             families in tools/render/tsan_triage.json. It fails when a
             signature matches no family, or when any frame of a report lies
             in the render core (CORE_PATH): those are
             never triaged away. It also fails when a directory holds no
             TSan log header at all, so a run without the sanitizer cannot
             pass silently.
  selftest   synthetic reports: a report in a known family is accepted; an
             unknown one, a render-core one inside a family and an empty
             log directory are each rejected.

The families are the queued-mode triage of RFC 0001's R32-QUEUED record
(RFC/0001-native-vulkan-queued-rendering-progress.md, "TSan product tree"):
engine-pool debt present in mode 0, and Source's queued design seen only in
mode 2. RFC 0016 calls it the K0 triage list. A new family is a reviewed
decision recorded with its reason. Ends with one checks-v1 record
(CONFORMANCE n f). Python 3 standard library only.
"""

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "kiln"))
import sepipe_loader  # noqa: E402
from conformance_result import Checks  # noqa: E402

SCHEMA = "render-tsan-triage/v1"
TRIAGE = ROOT / "tools" / "render" / "tsan_triage.json"

# Code K3 owns, relative to the repository root: no report touching it is
# triaged away. The public contract headers the engine already shared
# (public/render/*.h) are not the core.
CORE_PATH = re.compile(
    r"^(render/|public/render/(composition|legacy|device|graph|frame|scene|renderer|pass|"
    r"material|shaderlib|resources|math)/|engine/render_core_host\.cpp)")


# The core's namespaces (render::...), not the legacy backend's render_vulkan::.
CORE_FUNCTION = re.compile(r"^(non-virtual thunk to )?render::")

REPORT_START = re.compile(r"^WARNING: ThreadSanitizer: (?P<kind>.+?)(?: \(pid=\d+\))?\s*$")
FRAME = re.compile(r"^\s+#\d+\s+(?P<func>.+?)\s+(?P<loc>\S+?)(?::\d+)*(?::\d+)?\s+\((?P<obj>[^)]*)\)\s*$")
STACK_HEAD = re.compile(r"^\s{2}(?:Previous |Atomic )?(?:[Ww]rite|[Rr]ead|Mutex|Thread|Location)")


def parse_reports(text):
    """Each report: {'kind', 'stacks': [[(func, loc)]], 'text'}."""
    reports, current, stack = [], None, None
    for line in text.splitlines():
        start = REPORT_START.match(line)
        if start:
            current = {"kind": start.group("kind"), "stacks": [], "text": [line]}
            reports.append(current)
            stack = None
            continue
        if current is None:
            continue
        if line.startswith("SUMMARY: ThreadSanitizer") or line.startswith("=================="):
            current = None
            continue
        current["text"].append(line)
        if STACK_HEAD.match(line):
            stack = []
            current["stacks"].append(stack)
            continue
        frame = FRAME.match(line)
        if frame and stack is not None:
            stack.append((frame.group("func"), frame.group("loc")))
    return reports


def first_party(frame):
    func, loc = frame
    return not (loc.startswith("/usr/") or "sanitizer_common" in loc or "tsan" in loc or
                func.startswith("__tsan") or func.startswith("std::"))


def signature(report):
    tops = []
    for stack in report["stacks"][:2]:
        own = [f for f in stack if first_party(f)]
        tops.append(re.sub(r"\(.*", "", (own or stack or [("?", "")])[0][0]))
    return report["kind"] + " | " + " / ".join(sorted(tops))


def repo_relative(loc):
    root = str(ROOT) + "/"
    return loc[len(root):] if loc.startswith(root) else None


def in_core(report):
    for stack in report["stacks"]:
        for _, loc in stack:
            rel = repo_relative(loc)
            if rel and CORE_PATH.match(rel):
                return rel
    # A release TSan build leaves many frames without a source path (<null>,
    # or a bare file name), so the two access stacks are also checked by
    # function: a core function racing is a core report wherever its source
    # line went. Thread-creation stacks are not accesses and don't count.
    for stack in report["stacks"][:2]:
        for func, _ in stack:
            if CORE_FUNCTION.match(func):
                return func
    return None


def load_families(path=TRIAGE):
    data = json.loads(Path(path).read_text())
    if data.get("schema") != SCHEMA:
        raise ValueError("%s: schema is not %s" % (path, SCHEMA))
    families = []
    for family in data["families"]:
        families.append((family["id"], [re.compile(p) for p in family["frames"]]))
    return families


def family_of(report, families):
    frames = [func + " " + loc for stack in report["stacks"] for func, loc in stack]
    for fid, patterns in families:
        if any(p.search(frame) for p in patterns for frame in frames):
            return fid
    return None


def log_files(directory):
    return sorted(p for p in Path(directory).rglob("*") if p.is_file() and
                  p.name.startswith("tsan"))


def check(dirs, families, checks, stream=sys.stdout):
    unknown, core, counts, total = {}, {}, {}, 0
    for directory in dirs:
        files = log_files(directory)
        checks.check(bool(files), "tsan.ran-under-the-sanitizer",
                     "%s holds no TSan log (tsan.<pid>)" % directory)
        for path in files:
            for report in parse_reports(path.read_text(errors="replace")):
                total += 1
                sig = signature(report)
                where = in_core(report)
                if where:
                    core.setdefault(sig, (where, path, report))
                    continue
                fid = family_of(report, families)
                if fid:
                    counts[fid] = counts.get(fid, 0) + 1
                else:
                    unknown.setdefault(sig, (path, report))
    print("INFO tsan: %d report(s); by family: %s" % (
        total, ", ".join("%s %d" % kv for kv in sorted(counts.items())) or "none"), file=stream)
    for sig, (where, path, report) in sorted(core.items()):
        print("CORE %s (%s, %s)" % (sig, where, path), file=stream)
        print("\n".join(report["text"][:30]), file=stream)
    for sig, (path, report) in sorted(unknown.items()):
        print("NEW %s (%s)" % (sig, path), file=stream)
        print("\n".join(report["text"][:30]), file=stream)
    checks.equal(len(core), 0, "tsan.no-report-in-the-render-core")
    checks.equal(len(unknown), 0, "tsan.every-signature-is-in-a-triaged-family")
    return total


def run(args):
    checks = Checks()
    dirs = []
    for mode in args.mode or [0, 2]:
        out = Path(args.out) / ("mode%d" % mode)
        tsan_dir = out / "tsan"
        tsan_dir.mkdir(parents=True, exist_ok=True)
        env = dict(os.environ)
        env["TSAN_OPTIONS"] = " ".join([
            "log_path=%s" % (tsan_dir / "tsan"), "halt_on_error=0", "report_signal_unsafe=0",
            "history_size=4", "second_deadlock_stack=1", "exitcode=0"])
        command = [sys.executable, str(ROOT / "tools" / "quality" / "portal_boot.py"),
                   *sepipe_loader.boot_arguments(args),
                   "--renderer", "core", "--headless", "--map", args.map,
                   "--timeout", str(args.timeout),
                   "--engine-arg=+mat_queue_mode", "--engine-arg=%d" % mode,
                   "--out", str(out / "boot")]
        if args.content_root:
            command += ["--content-root", args.content_root]
        command += ["--engine-arg=" + argument for argument in args.engine_arg or []]
        result = subprocess.run(command, env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True)
        tail = result.stdout.strip().splitlines()[-1:] or [""]
        checks.check(result.returncode == 0, "tsan.boot-mode-%d-passes" % mode, tail[0])
        dirs.append(tsan_dir)
    check(dirs, load_families(), checks)
    return checks.report()


SAMPLE = """==================
WARNING: ThreadSanitizer: data race (pid=42)
  Write of size 4 at 0x7b0400000000 by thread T3:
    #0 %(a)s %(afile)s:10:3 (libx.so+0x1)
    #1 caller /src/x.cpp:20:3 (libx.so+0x2)

  Previous read of size 4 at 0x7b0400000000 by main thread:
    #0 %(b)s /src/engine/b.cpp:30:3 (libengine.so+0x3)

SUMMARY: ThreadSanitizer: data race /src/x.cpp:10:3 in %(a)s
==================
"""


def selftest():
    checks = Checks()
    families = [("proxies", [re.compile(r"CMaterialVar::Set")])]
    cases = {
        "known": ({"a": "CMaterialVar::SetFloatValue(float)", "afile": str(ROOT / "materialsystem/cmaterialvar.cpp"),
                   "b": "CProxy::OnBind()"}, True),
        "unknown": ({"a": "CNewThing::Poke()", "afile": str(ROOT / "engine/new.cpp"), "b": "CNewThing::Peek()"}, False),
        "core-in-family": ({"a": "CMaterialVar::SetFloatValue(float)",
                            "afile": str(ROOT / "render/legacy/frame_executor.cpp"), "b": "CProxy::OnBind()"}, False),
        # A core function with no source path (a release TSan build), racing
        # a triaged frame: still a core report.
        "core-by-function": ({"a": "render::pass::world::WorldPass::Record(unsigned int)",
                              "afile": "world_pass.cpp", "b": "CMaterialVar::SetFloatValue(float)"}, False),
    }
    with tempfile.TemporaryDirectory() as tmp:
        for name, (fields, accept) in cases.items():
            directory = Path(tmp) / name
            directory.mkdir()
            (directory / "tsan.42").write_text(SAMPLE % fields)
            quiet = open(os.devnull, "w")
            inner = Checks(stream=quiet)
            check([directory], families, inner, stream=quiet)
            checks.check((inner.failures == 0) == accept, "selftest.%s-%s" % (
                name, "accepted" if accept else "rejected"))
            if name == "known":
                parsed = parse_reports(SAMPLE % fields)
                checks.equal(signature(parsed[0]), "data race | CMaterialVar::SetFloatValue / CProxy::OnBind",
                             "selftest.signature-shape")
        empty = Path(tmp) / "empty"
        empty.mkdir()
        quiet = open(os.devnull, "w")
        inner = Checks(stream=quiet)
        check([empty], families, inner, stream=quiet)
        checks.check(inner.failures > 0, "selftest.a-directory-without-tsan-logs-is-rejected")
    checks.check(bool(load_families()), "selftest.the-triage-list-loads")
    return checks.report()


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    run_parser = sub.add_parser("run")
    sepipe_loader.add_arguments(run_parser, "portal-tsan-linux")
    run_parser.add_argument("--out", required=True)
    run_parser.add_argument("--mode", type=int, action="append", choices=(0, 2))
    run_parser.add_argument("--map", default="testchmb_a_01")
    run_parser.add_argument("--timeout", type=int, default=1200)
    run_parser.add_argument("--engine-arg", action="append",
                            help="an extra engine argument for both boots, e.g. +r_core_world")
    run_parser.add_argument("--content-root",
                            help="a compiled map's content (portal_boot --content-root)")
    check_parser = sub.add_parser("check")
    check_parser.add_argument("--logs", action="append", required=True)
    sub.add_parser("selftest")
    args = parser.parse_args()
    if args.command == "run":
        return run(args)
    if args.command == "check":
        checks = Checks()
        check(args.logs, load_families(), checks)
        return checks.report()
    return selftest()


if __name__ == "__main__":
    sys.exit(main())
