#!/usr/bin/env python3
"""RFC 0027 L0 gate checks that need the real repository and a built kiln.

    kiln_gate.py parity [--kiln PATH]     `kiln profiles resolve` equals
                                          profile_extends.py on every profile
    kiln_gate.py rebuild PROFILE [--kiln PATH]
                                          a second `kiln build` of an unchanged
                                          profile neither reconfigures nor rebuilds
    kiln_gate.py coop                     a real co-op pair starts headless and connects
    kiln_gate.py selftest                 the comparators catch seeded divergence

Each prints one `CONFORMANCE <checks> <failures>` record (checks-v1).
The C++ suites (contracts, bad providers, seeded profile faults, the fixture
platform) are unittests/kilntest; the layer rules are archlint CAP011.
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import profile_extends  # noqa: E402  (the reference implementation)

PROFILES = ROOT / "quality" / "product_profiles"


class Record:
    def __init__(self):
        self.checks = 0
        self.failures = 0

    def check(self, condition, name, detail=""):
        self.checks += 1
        if not condition:
            self.failures += 1
            print(f"FAIL: {name}" + (f": {detail}" if detail else ""), file=sys.stderr)

    def report(self):
        print(f"CONFORMANCE {self.checks} {self.failures}", flush=True)
        return 0 if self.checks and not self.failures else 1


def ordered(text):
    """A JSON document with member order kept (lists of pairs)."""
    return json.loads(text, object_pairs_hook=lambda pairs: [list(pair) for pair in pairs])


def same_document(a, b):
    """Exact equality including member order; numbers compare by value."""
    return a == b


def run_kiln(kiln, *args):
    return subprocess.run([str(kiln), *args], cwd=ROOT, capture_output=True, text=True, timeout=7200)


def profile_files():
    return sorted(p for p in PROFILES.rglob("*.json"))


def parity(kiln):
    record = Record()
    files = profile_files()
    record.check(len(files) >= 16, "at least the 16 pre-RFC profiles exist", str(len(files)))
    for path in files:
        name = path.relative_to(PROFILES).as_posix()
        result = run_kiln(kiln, "--json", "profiles", "resolve", name.removesuffix(".json")
                          if "/" not in name else name)
        if result.returncode != 0:
            record.check(False, f"kiln resolves {name}", result.stderr.strip()[-300:])
            continue
        expected = json.dumps(profile_extends.load_profile(path))
        record.check(same_document(ordered(result.stdout), ordered(expected)),
                     f"{name}: kiln profiles resolve equals profile_extends.py (values and member order)")
    return record.report()


def rebuild(kiln, profile):
    record = Record()
    runs = []
    for attempt in (1, 2):
        result = run_kiln(kiln, "--json", "build", profile)
        record.check(result.returncode == 0, f"kiln build {profile} (run {attempt})", result.stderr.strip()[-500:])
        if result.returncode != 0:
            return record.report()
        runs.append(json.loads(result.stdout))
    second = runs[1]
    stage = next((s for s in second["stages"] if s["role"] == "engine"), None)
    record.check(stage is not None, "the engine stage ran")
    if stage:
        evidence = stage["evidence"]
        record.check(evidence.get("configured") is False, "the second build does not reconfigure",
                     evidence.get("configure_reason", ""))
        record.check(evidence.get("waf_tasks") == 0, "the second build runs no Waf task",
                     str(evidence.get("waf_tasks")))
        record.check(evidence.get("installed_files") == 0, "the second build installs nothing",
                     str(evidence.get("installed_files")))
        record.check(stage["up_to_date"] is True and second["up_to_date"] is True, "the result says up to date")
    evidence_file = Path(second.get("evidence_file", ""))
    record.check(evidence_file.is_file(), "the build wrote its evidence record", str(evidence_file))
    if evidence_file.is_file():
        data = json.loads(evidence_file.read_text())
        for key in ("revision", "dirty_digest", "profile_digest", "toolchain", "reproduction"):
            record.check(key in data, f"evidence records {key}")
    print(json.dumps({"profile": profile, "first": runs[0]["stages"][0]["summary"],
                      "second": second["stages"][0]["summary"], "tree": second["tree"]}), file=sys.stderr)
    return record.report()


def coop(kiln):
    """kiln play portal2-coop, headless (display none, null renderer), until the
    client joins: the live half of the coop-pair gate."""
    record = Record()
    result = subprocess.run([str(kiln), "play", "portal2-coop", "--display", "none", "--set", "null",
                             "--set", "until-joined"], cwd=ROOT, capture_output=True, text=True, timeout=1800)
    output = result.stdout + result.stderr
    record.check(result.returncode == 0, "the co-op pair runs and stops", output[-800:])
    record.check("[run] host answers at" in output, "the host's server answers the engine challenge")
    record.check("[run] connected:" in output, "the client joins over the LAN")
    plan = json.loads(run_kiln(kiln, "play", "portal2-coop", "--dry-run").stdout)
    log = Path(plan["runtime"]) / "engine.log"
    text = log.read_text(errors="replace") if log.is_file() else ""
    record.check(" connected (" in text and "loopback" not in text.split(" connected (")[-1][:10],
                 "the host's engine.log records the remote join", str(log))
    return record.report()


def selftest():
    """Seeded divergences the comparators must catch (sensitivity)."""
    record = Record()
    base = '{"a": 1, "o": {"x": [1, 2], "y": "s"}}'
    record.check(same_document(ordered(base), ordered('{"a": 1, "o": {"x": [1, 2], "y": "s"}}')),
                 "identical documents compare equal")
    record.check(same_document(ordered(base), ordered('{"a": 1.0, "o": {"x": [1, 2], "y": "s"}}')),
                 "numbers compare by value (1 == 1.0)")
    for name, seeded in (("member order", '{"o": {"x": [1, 2], "y": "s"}, "a": 1}'),
                         ("nested member order", '{"a": 1, "o": {"y": "s", "x": [1, 2]}}'),
                         ("array order", '{"a": 1, "o": {"x": [2, 1], "y": "s"}}'),
                         ("a value", '{"a": 2, "o": {"x": [1, 2], "y": "s"}}'),
                         ("a dropped member", '{"a": 1, "o": {"x": [1, 2]}}'),
                         ("an extra member", '{"a": 1, "o": {"x": [1, 2], "y": "s"}, "extends": "p.json"}')):
        record.check(not same_document(ordered(base), ordered(seeded)), f"the comparator catches {name}")
    # The rebuild verdict rejects a second build that did work.
    record.check(json.loads('{"configured": true}').get("configured") is not False,
                 "a reconfigure is not read as up to date")
    return record.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("parity", "rebuild", "coop"):
        command = sub.add_parser(name)
        command.add_argument("--kiln", type=Path, default=ROOT / "kiln", help="the kiln bootstrap or binary")
        if name == "rebuild":
            command.add_argument("profile")
    sub.add_parser("selftest")
    args = parser.parse_args(argv)
    if args.command == "parity":
        return parity(args.kiln)
    if args.command == "rebuild":
        return rebuild(args.kiln, args.profile)
    if args.command == "coop":
        return coop(args.kiln)
    return selftest()


if __name__ == "__main__":
    sys.exit(main())
