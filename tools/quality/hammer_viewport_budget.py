#!/usr/bin/env python3
"""Judge the Hammer editor's edit-to-pixels budgets (quality/budgets/hammer-viewport-v1.json).

For each budget row this runs `hammer_gtk --viewport-budget` on the row's map:
the editor opens the map, selects one solid and, for every edit, moves it
through the command layer and times until all four views' new frames are back
on the host sequence from the render sequence (RFC 0016 decision "threading";
RFC 0002 R17). The samples are judged against the row's p95 and max limits.

A control judges the same samples against a limit no real run meets
(0.001 ms), which must be rejected, so a judge that passes everything fails
this suite.

hammer_gtk is built with hammer/gtk/build.sh unless --gtk names a binary. The
run needs a Vulkan device and opens no window.

Results are reported as checks-v1 (tools/quality/conformance_result.py).
"""

import argparse
import json
import math
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/quality"))
from conformance_result import Checks  # noqa: E402

BUDGETS = ROOT / "quality/budgets/hammer-viewport-v1.json"
SAMPLES_SCHEMA = "hammer-viewport-samples/v1"


def percentile(samples, fraction):
    """Nearest-rank percentile."""
    ordered = sorted(samples)
    rank = max(1, math.ceil(fraction * len(ordered)))
    return ordered[rank - 1]


def judge(samples, limits):
    """Returns (ok, detail) for samples against a row's limits."""
    if not samples:
        return False, "no samples"
    p95 = percentile(samples, 0.95)
    worst = max(samples)
    ok = p95 <= limits["edit_to_pixels_p95_ms"] and worst <= limits["edit_to_pixels_max_ms"]
    return ok, "p95 %.2f ms (limit %.2f), max %.2f ms (limit %.2f)" % (
        p95, limits["edit_to_pixels_p95_ms"], worst, limits["edit_to_pixels_max_ms"])


def run_row(gtk, row, out, timeout):
    workload = row["workload"]
    samples_path = out / (row["id"] + ".json")
    command = [str(gtk), "--viewport-budget", str(samples_path), str(ROOT / workload["map"]),
               "--width", str(workload["width"]), "--height", str(workload["height"]),
               "--warmup", str(workload["warmup_edits"]), "--edits", str(workload["edits"])]
    ran = subprocess.run(command, capture_output=True, text=True, timeout=timeout)
    (out / (row["id"] + ".log")).write_text(" ".join(command) + "\n" + ran.stdout + ran.stderr)
    if ran.returncode != 0 or not samples_path.exists():
        return ran.returncode, None
    return ran.returncode, json.loads(samples_path.read_text())


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--gtk", type=Path, help="a built hammer_gtk (default: build one)")
    parser.add_argument("--budgets", type=Path, default=BUDGETS)
    parser.add_argument("--row", action="append", help="only these row ids")
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/hammer-viewport-budget")
    parser.add_argument("--timeout", type=float, default=300)
    args = parser.parse_args()

    checks = Checks()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    if args.gtk is None:
        args.gtk = out / "hammer_gtk"
        built = subprocess.run([str(ROOT / "hammer/gtk/build.sh"), str(args.gtk)],
                               capture_output=True, text=True)
        (out / "build-gtk.log").write_text(built.stdout + built.stderr)
        checks.equal(built.returncode, 0, "shell.built")
        if built.returncode:
            return checks.report()

    budgets = json.loads(args.budgets.read_text())
    checks.equal(budgets.get("schema"), "hammer-viewport-budget/v1", "budgets.schema")
    rows = [r for r in budgets["rows"] if not args.row or r["id"] in args.row]
    checks.check(bool(rows), "budgets.rows", "no rows selected")
    summary = {"schema": "hammer-viewport-budget-results/v1", "rows": {}}
    for row in rows:
        name = row["id"]
        code, result = run_row(args.gtk, row, out, args.timeout)
        checks.check(code == 0 and result is not None, name + ".ran", "exit %s" % code)
        if result is None:
            continue
        samples = result.get("samples_ms", [])
        checks.equal(result.get("schema"), SAMPLES_SCHEMA, name + ".schema")
        checks.equal(len(samples), row["workload"]["edits"], name + ".samples")
        checks.check(all(isinstance(s, (int, float)) and s > 0 for s in samples),
                     name + ".samples-positive")
        ok, detail = judge(samples, row["limits"])
        checks.check(ok, name + ".within-budget", detail)
        impossible = {"edit_to_pixels_p95_ms": 0.001, "edit_to_pixels_max_ms": 0.001}
        rejected, _ = judge(samples, impossible)
        checks.check(not rejected, name + ".impossible-limit-rejected",
                     "a 0.001 ms limit passed")
        summary["rows"][name] = {"within": ok, "detail": detail,
                                 "p50_ms": percentile(samples, 0.5) if samples else None,
                                 "p95_ms": percentile(samples, 0.95) if samples else None,
                                 "max_ms": max(samples) if samples else None}
        print("%s: %s" % (name, detail))
    (out / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
