#!/usr/bin/env python3
"""Render frame budgets (R32-RENDER-BUDGETS, RFC 0016 K0).

    python3 tools/quality/render_budgets.py check
    python3 tools/quality/render_budgets.py record --row ROW --evidence EVIDENCE [--update]
    python3 tools/quality/render_budgets.py report --row ROW --evidence EVIDENCE

`quality/budgets/render-v1.json` owns the numbers. Each row names a product
profile, a workload, its hardware and the limits of its presentation modes,
set before the row is measured (RFC 0005 "Performance and promotion").

`check` validates declarations, not achieved frame performance. Every required row needs numeric limits for p50,
p95, p99, GPU render time and main-thread submission time, the profile and
workload it names exist, and it carries a k0_record (the measurement later
render-core gates compare against). A k0_record that misses a limit must say
so in the row's over_budget (the exact fields, an owning roadmap row and a
reason); an undeclared miss, or a declared miss the record no longer has,
fails. Hard rows also require an every-frame floor, conditions and bounded
CPU/GPU metrics. A new hard row may explicitly have no baseline: it remains
unverified until its route, quality receipt and complete-image evidence pass.

`record` stores a frame_pacing evidence file's warm pass as a row's
k0_record; it never replaces one without --update.

`report` judges a later run: the row's limits, and RFC 0016's frame
allowance (warm median and p99 at most the allowance times the k0_record).
The hard High row uses frame_floor.py; a warm-pass report cannot certify High.
"""

import argparse
import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUDGETS = ROOT / "quality/budgets/render-v1.json"
REQUIRED_ROWS = ("linux-wayland-portal-frame-pacing", "linux-desktop-high-120")
# Rows of optional runners (the Fold7: user decision, 2026-10-08; AGENTS.md) are
# validated like required rows when present, but may be absent.
OPTIONAL_ROWS = ("android-fold7-portal-frame-pacing",)
LIMITS = ("max_p50_ms", "max_p95_ms", "max_p99_ms", "max_gpu_render_p99_ms", "max_submission_p99_ms",
          "max_hitches")
# k0_record field <- frame_pacing warm-pass summary field
RECORD_FIELDS = {"p50_ms": "median_ms", "p95_ms": "p95_ms", "p99_ms": "p99_ms",
                 "gpu_render_median_ms": "gpu_render_median_ms", "gpu_render_p99_ms": "gpu_render_p99_ms",
                 "submission_median_ms": "submission_median_ms", "submission_p99_ms": "submission_p99_ms"}
# limit -> k0_record field it bounds
LIMIT_FIELDS = {"max_p50_ms": "p50_ms", "max_p95_ms": "p95_ms", "max_p99_ms": "p99_ms",
                "max_gpu_render_p99_ms": "gpu_render_p99_ms", "max_submission_p99_ms": "submission_p99_ms",
                "max_hitches": "hitches", "max_frame_ms": "max_frame_ms", "max_cpu_p99_ms": "cpu_p99_ms"}


def valid_number(value):
    return (isinstance(value, (int, float)) and not isinstance(value, bool)
            and math.isfinite(value) and value >= 0)


def load(path=BUDGETS):
    budgets = json.loads(Path(path).read_text())
    if budgets.get("schema") != "source-render-budgets/v1":
        raise ValueError("budget schema must be source-render-budgets/v1")
    return budgets


def find(budgets, row_id):
    for row in budgets["rows"]:
        if row["id"] == row_id:
            return row
    raise ValueError("no budget row %s" % row_id)


def check(budgets, root=ROOT, required=REQUIRED_ROWS, optional=OPTIONAL_ROWS):
    problems = []
    allowance = budgets.get("allowance", {})
    for key in ("median", "p99"):
        if not isinstance(allowance.get(key), (int, float)) or allowance[key] < 1.0:
            problems.append("allowance.%s must be a number of at least 1" % key)
    ids = [row.get("id") for row in budgets["rows"]]
    for row_id in required:
        if row_id not in ids:
            problems.append("required row %s is missing" % row_id)
    for row in budgets["rows"]:
        if row.get("id") not in required and row.get("id") not in optional:
            continue
        name = row["id"]
        for key in ("profile", "workload"):
            if not row.get(key) or not (Path(root) / row[key]).is_file():
                problems.append("%s: %s %s does not exist" % (name, key, row.get(key)))
        for key in ("hardware", "set", "presentation"):
            if not row.get(key):
                problems.append("%s: needs %s" % (name, key))
        limits = row.get("modes", {}).get("headroom", {})
        for key in LIMITS:
            if not valid_number(limits.get(key)):
                problems.append("%s: headroom limit %s must be a number" % (name, key))
        if row.get("acceptance") == "hard":
            fps = row.get("minimum_fps")
            if not valid_number(fps) or fps == 0:
                problems.append("%s: minimum_fps must be positive" % name)
            else:
                floor_ms = 1000.0 / fps
                for key in ("max_frame_ms", "max_p50_ms", "max_p95_ms", "max_p99_ms",
                            "max_gpu_render_p99_ms", "max_cpu_p99_ms", "max_submission_p99_ms"):
                    if not valid_number(limits.get(key)) or limits[key] > floor_ms:
                        problems.append("%s: %s must meet the hard frame floor" % (name, key))
            conditions = row.get("conditions", {})
            if not all(conditions.get(key) for key in ("width", "height", "quality", "device")):
                problems.append("%s: hard budget needs resolution, quality and device conditions" % name)
        record = row.get("k0_record")
        if not record:
            if row.get("acceptance") == "hard" and row.get("baseline_required") is False:
                # This only validates the declaration. No measurement means no acceptance.
                continue
            problems.append("%s: no k0_record; measure the row and run record" % name)
            continue
        for field in list(RECORD_FIELDS) + ["hitches", "revision", "measured", "evidence_command"]:
            if field not in record:
                problems.append("%s: k0_record lacks %s" % (name, field))
        # A measured baseline may miss its target; the miss is then declared,
        # owned and exact, never silent and never stale.
        over = set()
        for key, field in LIMIT_FIELDS.items():
            value, limit = record.get(field), limits.get(key)
            if isinstance(value, (int, float)) and isinstance(limit, (int, float)) and value > limit:
                over.add(field)
        declared = row.get("over_budget") or {}
        fields = set(declared.get("fields", []))
        if declared and (not re_row(declared.get("owner", "")) or not declared.get("reason")):
            problems.append("%s: over_budget needs a roadmap-row owner and a reason" % name)
        for field in sorted(over - fields):
            problems.append("%s: k0_record %s %.3f is over its limit and not declared in over_budget"
                            % (name, field, record[field]))
        for field in sorted(fields - over):
            problems.append("%s: over_budget declares %s, which the k0_record no longer exceeds" % (name, field))
    return problems


def re_row(owner):
    import re
    return re.match(r"^R[0-9]{2}$", owner) is not None


def warm_summary(evidence):
    passes = evidence.get("analysis", {}).get("passes", [])
    if not passes or not passes[-1]["summary"].get("frames"):
        raise ValueError("the evidence has no measured warm pass")
    return passes[-1]


def record_from(evidence):
    warm = warm_summary(evidence)
    summary = warm["summary"]
    record = {}
    for field, source in RECORD_FIELDS.items():
        if source not in summary:
            raise ValueError("the warm pass has no %s" % source)
        record[field] = summary[source]
    record["hitches"] = warm["hitch_count"]
    record["frames"] = summary["frames"]
    record["revision"] = evidence.get("source", {}).get("revision")
    record["dirty"] = evidence.get("source", {}).get("dirty")
    record["measured"] = evidence.get("started_utc")
    record["device"] = (evidence.get("device") or {}).get("device")
    record["evidence_command"] = evidence.get("command") or evidence.get("arguments")
    for field, source in (("max_frame_ms", "max_ms"), ("cpu_p99_ms", "cpu_p99_ms")):
        if source in summary:
            record[field] = summary[source]
    return record


def report(row, evidence, allowance):
    failures = []
    warm = warm_summary(evidence)
    summary = warm["summary"]
    limits = row["modes"]["headroom"]
    measured = {"p50_ms": summary.get("median_ms"), "p95_ms": summary.get("p95_ms"),
                "p99_ms": summary.get("p99_ms"), "gpu_render_p99_ms": summary.get("gpu_render_p99_ms"),
                "submission_p99_ms": summary.get("submission_p99_ms"), "hitches": warm["hitch_count"],
                "max_frame_ms": summary.get("max_ms"), "cpu_p99_ms": summary.get("cpu_p99_ms")}
    for key, field in LIMIT_FIELDS.items():
        if key not in limits:
            continue
        if not valid_number(measured[field]):
            failures.append("the warm pass has no valid %s" % field)
        elif measured[field] > limits[key]:
            failures.append("%s %.3f is over the limit %.3f" % (field, measured[field], limits[key]))
    if evidence.get("status") in ("fail", "error", "incomplete"):
        failures.append("the evidence did not complete successfully")
    if row.get("acceptance") == "hard":
        failures.append("High qualification requires the row-linked frame_floor.py route and quality receipt; "
                        "a warm-pass timing report alone cannot certify it")
    record = row.get("k0_record")
    if record:
        for field, factor in (("p50_ms", allowance["median"]), ("p99_ms", allowance["p99"])):
            if measured[field] is not None and measured[field] > factor * record[field]:
                failures.append("%s %.3f exceeds the frame allowance %.2f x the K0 record %.3f"
                                % (field, measured[field], factor, record[field]))
    return measured, failures


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("check")
    for name in ("record", "report"):
        command = sub.add_parser(name)
        command.add_argument("--row", required=True)
        command.add_argument("--evidence", type=Path, required=True)
        if name == "record":
            command.add_argument("--update", action="store_true", help="replace an existing k0_record")
    args = parser.parse_args(argv)
    budgets = load()
    if args.command == "check":
        problems = check(budgets)
        for problem in problems:
            print("render_budgets:", problem)
        print("render_budgets: %d row(s), %d problem(s)" % (len(budgets["rows"]), len(problems)))
        return 1 if problems else 0
    row = find(budgets, args.row)
    evidence = json.loads(args.evidence.read_text())
    if args.command == "record":
        if row.get("k0_record") and not args.update:
            print("render_budgets: %s already has a k0_record; pass --update to replace it" % args.row)
            return 1
        row["k0_record"] = record_from(evidence)
        BUDGETS.write_text(json.dumps(budgets, indent=2, ensure_ascii=False) + "\n")
        print("render_budgets: recorded %s: %s" % (args.row, json.dumps(row["k0_record"])))
        return 0
    measured, failures = report(row, evidence, budgets["allowance"])
    print("render_budgets: %s measured %s" % (args.row, json.dumps(measured)))
    for failure in failures:
        print("render_budgets:", failure)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
