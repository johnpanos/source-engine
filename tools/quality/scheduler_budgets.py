#!/usr/bin/env python3
"""RFC 0003 R20 scheduler budgets: declaration check, measurement and census.

    python3 tools/quality/scheduler_budgets.py check
    python3 tools/quality/scheduler_budgets.py run --build build-sched-tests \\
        --profile linux-x86_64-desktop [--samples 15] [--census census.json]
    python3 tools/quality/scheduler_budgets.py census --pid PID [--out census.json]

`quality/budgets/scheduler-v1.json` owns the numbers. `check` requires, for
every declared profile, a capacity row (worker threads per pool and in total)
and overhead rows (pool microbenchmarks at the declared worker counts, host
graph cost per node) with an owner and positive limits; a profile may inherit
another's rows or be declared unverified. `run` measures the overhead rows with
the real pool benchmark and the host frame oracle's overhead probe and fails on
any value over its limit or any missing measurement. `census` counts a running
process's threads by name (numbers stripped) so capacity can be compared.
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUDGETS = ROOT / "quality/budgets/scheduler-v1.json"
SCHEMA = "scheduler-budget/v1"

BENCH_LINE = re.compile(r"^bench\s+(\S+)\s+median\s+([\d.]+)\s+us\b.*workers=(\d+)\)")
OVERHEAD_LINE = re.compile(r"^overhead: (\d+) phases: graph (\d+) ns/frame, direct loop (\d+) ns/frame")


def load(path=BUDGETS):
    budgets = json.loads(Path(path).read_text())
    if budgets.get("schema") != SCHEMA:
        raise ValueError("budget schema must be " + SCHEMA)
    return budgets


def resolve(budgets, profile):
    """The profile's own entry, following `inherits` for its rows."""
    seen = set()
    entry = budgets["profiles"][profile]
    while "inherits" in entry:
        if profile in seen:
            raise ValueError("budget inheritance cycle at " + profile)
        seen.add(profile)
        profile = entry["inherits"]
        entry = budgets["profiles"][profile]
    return entry


def check(budgets):
    problems = []
    for profile, own in budgets["profiles"].items():
        if own.get("status") == "unverified" and "inherits" not in own:
            if not own.get("owner"):
                problems.append("%s: unverified profile has no owner" % profile)
            continue
        try:
            entry = resolve(budgets, profile)
        except (KeyError, ValueError) as error:
            problems.append("%s: %s" % (profile, error))
            continue
        capacity = entry.get("capacity")
        if not capacity or not capacity.get("owner"):
            problems.append("%s: no capacity row with an owner" % profile)
        else:
            pools = capacity.get("pools", {})
            if not pools:
                problems.append("%s: capacity declares no pools" % profile)
            for pool, limit in pools.items():
                if not isinstance(limit, int) or limit < 0:
                    problems.append("%s: pool %s limit must be a count" % (profile, pool))
            total = capacity.get("total_pool_workers")
            if not isinstance(total, int) or total <= 0:
                problems.append("%s: total_pool_workers must be positive" % profile)
            elif sum(pools.values()) > total:
                problems.append("%s: pool limits exceed total_pool_workers" % profile)
        overhead = entry.get("overhead")
        if not overhead:
            problems.append("%s: no overhead rows" % profile)
            continue
        for name, row in overhead.items():
            if not row.get("owner"):
                problems.append("%s/%s: no owner" % (profile, name))
            limit = row.get("limit")
            if not isinstance(limit, (int, float)) or limit <= 0:
                problems.append("%s/%s: limit must be positive" % (profile, name))
            if row.get("kind") not in ("pool_bench", "graph_node_ns"):
                problems.append("%s/%s: unknown kind %r" % (profile, name, row.get("kind")))
    return problems


def parse_bench(text):
    """{(workload, workers): median_us} from threadpoolbench output."""
    values = {}
    for line in text.splitlines():
        match = BENCH_LINE.match(line.strip())
        if match:
            values[(match.group(1), int(match.group(3)))] = float(match.group(2))
    return values


def parse_graph_overhead(text):
    """{phases: ns added per node} from the host frame oracle's overhead probe."""
    values = {}
    for line in text.splitlines():
        match = OVERHEAD_LINE.match(line.strip())
        if match:
            phases, graph, loop = (int(g) for g in match.groups())
            values[phases] = (graph - loop) / float(phases)
    return values


def evaluate(overhead_rows, bench, graph):
    results = {}
    for name, row in overhead_rows.items():
        if row["kind"] == "pool_bench":
            measured = bench.get((row["workload"], row["workers"]))
        else:
            measured = graph.get(row["phases"])
        if measured is None:
            results[name] = {"status": "missing", "limit": row["limit"]}
        else:
            results[name] = {"measured": round(measured, 2), "limit": row["limit"],
                             "status": "pass" if measured <= row["limit"] else "fail"}
    return results


def evaluate_census(capacity, census):
    """Compare a thread census ({name: count}) with the capacity row."""
    results = {}
    total = 0
    for pool, limit in capacity["pools"].items():
        count = sum(n for name, n in census.items() if name == pool)
        total += count
        results[pool] = {"measured": count, "limit": limit,
                         "status": "pass" if count <= limit else "fail"}
    results["total_pool_workers"] = {"measured": total, "limit": capacity["total_pool_workers"],
                                     "status": "pass" if total <= capacity["total_pool_workers"]
                                     else "fail"}
    return results


def thread_census(pid):
    counts = {}
    for task in Path("/proc/%d/task" % pid).iterdir():
        try:
            name = (task / "comm").read_text().strip()
        except OSError:
            continue
        name = re.sub(r"\d+$", "", name)
        counts[name] = counts.get(name, 0) + 1
    return counts


def run_program(build, relative, args):
    env = dict(os.environ)
    libs = [str(Path(build) / d) for d in ("tier0", "tier1", "vstdlib")]
    env["LD_LIBRARY_PATH"] = ":".join(libs + [env.get("LD_LIBRARY_PATH", "")])
    result = subprocess.run([str(Path(build) / relative)] + args, capture_output=True, text=True,
                            env=env, timeout=1200)
    if result.returncode != 0:
        raise RuntimeError("%s failed (%d):\n%s" % (relative, result.returncode, result.stdout[-2000:]))
    return result.stdout


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("check")
    run = commands.add_parser("run")
    run.add_argument("--build", required=True, help="waf --tests output tree")
    run.add_argument("--profile", required=True)
    run.add_argument("--samples", type=int, default=15)
    run.add_argument("--census", help="thread census JSON from `census` for the capacity row")
    run.add_argument("--json", help="write the report here")
    census = commands.add_parser("census")
    census.add_argument("--pid", type=int, required=True)
    census.add_argument("--out")
    args = parser.parse_args(argv)

    if args.command == "census":
        counts = thread_census(args.pid)
        text = json.dumps(counts, indent=2, sort_keys=True)
        if args.out:
            Path(args.out).write_text(text + "\n")
        print(text)
        return 0

    budgets = load()
    if args.command == "check":
        problems = check(budgets)
        for problem in problems:
            print("FAIL " + problem)
        print("scheduler budgets: %s" % ("FAIL" if problems else "pass"))
        return 1 if problems else 0

    entry = resolve(budgets, args.profile)
    rows = entry["overhead"]
    bench = {}
    for workers in sorted({row["workers"] for row in rows.values() if row["kind"] == "pool_bench"}):
        bench.update(parse_bench(run_program(
            args.build, "unittests/jobsystemtest/jobsystemthreadpoolbench",
            ["--samples", str(args.samples), "--workers", str(workers)])))
    graph = parse_graph_overhead(run_program(
        args.build, "unittests/jobsystemtest/jobsystemhostframetest", ["--scenarios", "50"]))
    report = {"profile": args.profile, "overhead": evaluate(rows, bench, graph),
              "host_load": list(os.getloadavg())}
    if args.census:
        report["capacity"] = evaluate_census(entry["capacity"],
                                             json.loads(Path(args.census).read_text()))
    statuses = [r["status"] for section in ("overhead", "capacity") if section in report
                for r in report[section].values()]
    report["status"] = "pass" if statuses and all(s == "pass" for s in statuses) else "fail"
    text = json.dumps(report, indent=2, sort_keys=True)
    if args.json:
        Path(args.json).write_text(text + "\n")
    print(text)
    return 0 if report["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
