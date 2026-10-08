#!/usr/bin/env python3
"""RFC 0027 L1d gate: sepipe and the play_embedded sample.

    sepipe_gate.py [--tree out/tools-linux/dev/build]

sepipe's results must be the documents `kiln --json` prints for the same
request (profiles, switches, launch plans), its failures must raise
sepipe.KilnError, and play_embedded, a play request from a catalog the
sample composes itself, must spawn exactly the planned argv. Seeded
divergences (another map, another switch) must be caught. Prints one
`CONFORMANCE <checks> <failures>` record (checks-v1).
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# (profile, map, switches, arguments), each also planned by `kiln play --dry-run`.
PLANS = [
    ("portal", None, [], []),
    ("portal", "testchmb_a_00", ["null"], ["-w", "1280"]),
    ("portal2", "sp_a1_intro1", [], []),
    ("portal2-fsr", None, [], ["-h", "720"]),
    ("fstop", None, [], []),
]


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


def kiln_json(*args):
    result = subprocess.run([str(ROOT / "kiln"), "--json", *args], cwd=ROOT, capture_output=True,
                            text=True, timeout=600)
    return json.loads(result.stdout) if result.returncode == 0 else None


def dry_run(profile, map_name, switches, arguments, display=None):
    argv = ["play", profile]
    if map_name:
        argv.append(map_name)
    for switch in switches:
        argv += ["--set", switch]
    if display:
        argv += ["--display", display]
    argv.append("--dry-run")
    if arguments:
        argv += ["--", *arguments]
    return kiln_json(*argv)


def sample_argv(tree, profile, map_name):
    program = tree / "tools" / "samples" / "play_embedded" / "play_embedded_sample"
    result = subprocess.run([str(program), str(ROOT), profile, *([map_name] if map_name else [])],
                            cwd=ROOT, capture_output=True, text=True, timeout=1800)
    lines = result.stdout.splitlines()
    if result.returncode != 0 or not lines or lines[0] != "spawn:":
        return None, result.stderr.strip()[-400:]
    return lines[1:], ""


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--tree", type=Path, default=ROOT / "out" / "tools-linux" / "dev" / "build")
    args = parser.parse_args(argv)
    record = Record()

    sys.path.insert(0, str(args.tree / "product" / "kiln" / "python"))
    try:
        import sepipe
    except ImportError as error:
        record.check(False, "sepipe imports from the tools tree", str(error))
        return record.report()
    record.check(sepipe.CONTRACT == "kiln.api.v1", "sepipe binds kiln.api.v1", sepipe.CONTRACT)
    session = sepipe.Session(str(ROOT))

    record.check(session.profiles() == kiln_json("profiles", "list"),
                 "profiles() equals kiln profiles list")
    switches = session.switches("portal2")
    record.check(switches == kiln_json("switches", "portal2") and len(switches) > 0,
                 "switches() equals kiln switches")
    for profile, map_name, sets, arguments in PLANS:
        name = f"{profile} map={map_name} switches={sets} args={arguments}"
        plan = session.plan(profile, map=map_name, switches=sets, arguments=arguments)
        record.check(plan == dry_run(profile, map_name, sets, arguments),
                     f"plan() equals kiln play --dry-run: {name}")

    for profile, bad in (("nonesuch", "an unknown profile"), ("portal2", "an unknown switch")):
        try:
            if bad == "an unknown switch":
                session.plan(profile, switches=["no-such-switch"])
            else:
                session.plan(profile)
            record.check(False, f"{bad} raises KilnError")
        except sepipe.KilnError as error:
            record.check(str(error).split(":")[0] in ("profile", "switch", "launch"),
                         f"{bad} raises KilnError with its code", str(error))

    # Seeded: the comparator sees a different map and a different switch.
    base = session.plan("portal", map="testchmb_a_00")
    record.check(base != session.plan("portal", map="testchmb_a_01"), "a different map is caught")
    record.check(base != session.plan("portal", map="testchmb_a_00", switches=["null"]),
                 "a different switch is caught")

    # play_embedded: its own catalog, a recording spawner, the planned argv.
    planned = session.plan("portal", map="testchmb_a_00", display="none")
    spawned, detail = sample_argv(args.tree, "portal", "testchmb_a_00")
    record.check(spawned is not None, "play_embedded launches through kiln.api", detail)
    if spawned is not None:
        record.check(spawned == planned["argv"], "play_embedded spawns the planned argv",
                     f"{spawned[:6]} vs {planned['argv'][:6]}")
        record.check(spawned != session.plan("portal", map="testchmb_a_01", display="none")["argv"],
                     "the sample comparison catches another map")
    return record.report()


if __name__ == "__main__":
    sys.exit(main())
