#!/usr/bin/env python3
"""RFC 0011 indirect-light budgets: declaration check and measurement report.

    python3 tools/quality/gi_budgets.py check
    python3 tools/quality/gi_budgets.py report --profile P --producer radiosity \\
        --measured cpu_ms_per_update=0.4 --measured memory_mb=10

`quality/budgets/indirect-light-v1.json` owns the numbers. `check` requires a
row, with an owner, for every producer on every declared profile (a profile
may inherit another's rows until it is measured), and numeric limits. `report`
compares measured values with a row: a value over its limit fails, and a row
that declares the producer unsupported fails any claim of support.
"""

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUDGETS = ROOT / "quality/budgets/indirect-light-v1.json"
PRODUCERS = ("baked", "radiosity", "sdf", "rayquery")
LIMITS = ("cpu_ms_per_update", "gpu_ms", "memory_mb", "load_ms")


def load(path=BUDGETS):
    budgets = json.loads(Path(path).read_text())
    if budgets.get("schema") != "gi-budget/v1":
        raise ValueError("budget schema must be gi-budget/v1")
    return budgets


def rows(budgets, profile):
    entry = budgets["profiles"][profile]
    seen = set()
    while "inherits" in entry:
        if profile in seen:
            raise ValueError("budget inheritance cycle at " + profile)
        seen.add(profile)
        profile = entry["inherits"]
        entry = budgets["profiles"][profile]
    return entry["budgets"]


def check(budgets):
    problems = []
    for profile in budgets["profiles"]:
        try:
            table = rows(budgets, profile)
        except (KeyError, ValueError) as error:
            problems.append("%s: %s" % (profile, error))
            continue
        for producer in PRODUCERS + ("switching",):
            row = table.get(producer)
            if not row:
                problems.append("%s: no %s budget" % (profile, producer))
                continue
            if not row.get("owner"):
                problems.append("%s/%s: budget has no owner" % (profile, producer))
            if producer in PRODUCERS and "status" not in row:
                limits = [key for key in LIMITS if key in row]
                if not limits:
                    problems.append("%s/%s: no numeric limit" % (profile, producer))
                for key in limits:
                    if not isinstance(row[key], (int, float)) or row[key] <= 0:
                        problems.append("%s/%s: %s must be positive" % (profile, producer, key))
    return problems


def report(budgets, profile, producer, measured):
    row = rows(budgets, profile)[producer]
    if "status" in row:
        return {"status": "fail", "reason": "producer declared unsupported: " + row["status"]}
    results = {}
    for key, value in measured.items():
        if key not in row:
            results[key] = {"measured": value, "status": "no-limit"}
            continue
        results[key] = {"measured": value, "limit": row[key],
                        "status": "pass" if value <= row[key] else "fail"}
    status = "pass" if results and all(r["status"] != "fail" for r in results.values()) else "fail"
    return {"status": status, "profile": profile, "producer": producer, "row": row,
            "results": results}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("check")
    measured = commands.add_parser("report")
    measured.add_argument("--profile", required=True)
    measured.add_argument("--producer", required=True, choices=PRODUCERS)
    measured.add_argument("--measured", action="append", default=[], metavar="KEY=VALUE")
    args = parser.parse_args()
    budgets = load()
    if args.command == "check":
        problems = check(budgets)
        for problem in problems:
            print("FAIL " + problem)
        print("indirect-light budgets: %s" % ("FAIL" if problems else "pass"))
        return 1 if problems else 0
    values = {key: float(value) for key, _, value in
              (item.partition("=") for item in args.measured)}
    result = report(budgets, args.profile, args.producer, values)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
