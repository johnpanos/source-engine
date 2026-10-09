#!/usr/bin/env python3
"""Play recorded Portal 2 demos back to back on single game instances.

The demo corpus is quality/fixtures/demos/portal2-board (top leaderboard runs,
one per chamber; see its README) indexed by
quality/workloads/portal2-demos-v1/demos.json. A demo carries its own map, so
playing it loads and runs that map through the client and the engine's
level-load, signon, entity and network paths.

Selection (default: every demo in the manifest):
  --map NAME[,NAME...]      maps, run in the order given
  --chapter NAME[,NAME...]  chapters by name or board id, in manifest order

One game instance plays a chunk of up to --chunk demos in sequence with the
engine's ``startdemos`` (a level change per demo, no restart). ``--workers N``
splits the selection over N instances that run at the same time, each in its
own private runtime and sandbox. A demo that fails to play ends its chain, so
the demos after it are re-run once in a fresh instance.

Checks per demo (one checks-v1 record at the end):
  * the engine reported ``Playing demo from <file>`` and then
    ``Demo playback finished`` for it, in order;
  * the log holds no demo-read error, Host_Error, script error or assert;
  * the instance did not die or stall before the chain finished.

Retail content comes from a local Steam installation (the portal2 kiln
profile stages it); the client renders offscreen.
"""

import argparse
import concurrent.futures
import datetime
import json
import os
from pathlib import Path
import shutil
import sys
import threading
import time

import conformance
import conformance_result
import launch_sandbox
import portal2_scenarios

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402


SCHEMA = "portal2-demos/v1"
EVIDENCE_SCHEMA = "portal2-demo-suite-evidence/v1"
DEFAULT_MANIFEST = Path("quality/workloads/portal2-demos-v1/demos.json")
PLAYING = "Playing demo from "
FINISHED = "Demo playback finished"
# Substrings that fail the instance that logs them.
FATAL = ("demo file protocol", "Failed to read demo header", "is not a valid demo file", "Host_Error",
         "CDemo::Play: failed", "StartupDemoFile:", "Assertion failed",
         portal2_scenarios.SCRIPT_ERROR)
RUNTIME_DEMOS = "demos"


class SuiteError(Exception):
    pass


def load_manifest(path, root):
    data = json.loads(Path(path).read_text())
    if data.get("schema") != SCHEMA:
        raise SuiteError("%s: schema is not %s" % (path, SCHEMA))
    directory = Path(root) / data["directory"]
    seen = set()
    for demo in data["demos"]:
        if demo["map"] in seen:
            raise SuiteError("%s: duplicate map %s" % (path, demo["map"]))
        seen.add(demo["map"])
        demo["path"] = directory / demo["file"]
        if not demo["path"].is_file():
            raise SuiteError("missing demo file %s" % demo["path"])
    return data["demos"]


def select(demos, maps=(), chapters=()):
    """The demos for --map/--chapter (maps keep the given order); all when neither."""
    if not maps and not chapters:
        return list(demos)
    by_map = {demo["map"]: demo for demo in demos}
    selected = []
    for name in maps:
        if name not in by_map:
            raise SuiteError("no demo for map %s" % name)
        selected.append(by_map[name])
    for chapter in chapters:
        matching = [demo for demo in demos
                    if chapter.lower() == demo["chapter"].lower() or chapter == str(demo["chapter_id"])]
        if not matching:
            raise SuiteError("no demos for chapter %s" % chapter)
        selected.extend(matching)
    unique = []
    for demo in selected:
        if demo not in unique:
            unique.append(demo)
    return unique


def partition(items, workers, chunk):
    """Contiguous chunks of at most `chunk`, dealt to `workers` jobs so the
    chunk count per worker differs by at most one."""
    workers = max(1, min(workers, len(items) or 1))
    size = max(1, min(chunk, -(-len(items) // workers)))
    chunks = [items[i:i + size] for i in range(0, len(items), size)]
    jobs = [[] for _ in range(workers)]
    for index, part in enumerate(chunks):
        jobs[index % workers].append(part)
    return [job for job in jobs if job]


def evaluate(demos, log, finished_early=None):
    """Per-demo results for one chain from its console log: a list of
    (demo, ok, detail). Demos after the first one the log did not reach have
    detail "not run"."""
    lines = log.splitlines()
    fatal = next((line.strip() for line in lines if any(mark in line for mark in FATAL)), None)
    events = []
    for line in lines:
        if PLAYING in line:
            events.append(("play", line.split(PLAYING, 1)[1].strip().rstrip(".")))
        elif FINISHED in line:
            events.append(("done", None))
    results = []
    cursor = 0
    for demo in demos:
        stem = Path(demo["file"]).stem
        if cursor >= len(events) or events[cursor][0] != "play" or \
                Path(events[cursor][1]).stem != stem:
            results.append((demo, False, "not run" if cursor >= len(events) else
                            "out of order: %s" % (events[cursor][1],)))
            if fatal and cursor >= len(events) and not any(r[1] for r in results):
                results[-1] = (demo, False, "not run: " + fatal)
            continue
        cursor += 1
        if cursor < len(events) and events[cursor][0] == "done":
            cursor += 1
            results.append((demo, True, "played"))
        else:
            results.append((demo, False, "did not finish" + (": " + fatal if fatal else "")))
    if fatal and all(ok for _, ok, _ in results):
        results[-1] = (results[-1][0], False, fatal)
    return results


class Worker:
    """One private runtime, sandbox and output directory, reused for its chains."""

    def __init__(self, index, args, output, tools):
        self.index = index
        self.args = args
        self.output = output / ("worker%d" % index)
        self.runtime = self.output / "runtime"
        self.tools = tools
        self.runs = 0

    def prepare(self, demos):
        portal2_scenarios.package_runtime((self.args.profile, self.args.flavor), self.runtime)
        target = self.runtime / "portal2" / RUNTIME_DEMOS
        target.mkdir(parents=True, exist_ok=True)
        for demo in demos:
            destination = target / demo["file"]
            if not destination.exists():
                try:
                    os.link(demo["path"], destination)
                except OSError:
                    shutil.copy2(demo["path"], destination)

    def run_chain(self, chain):
        """One game instance playing `chain` in order; (results, info)."""
        self.runs += 1
        out = self.output / ("chain%d" % self.runs)
        out.mkdir(parents=True, exist_ok=True)
        console = self.runtime / "portal2/console.log"
        console.unlink(missing_ok=True)
        sandbox = launch_sandbox.Sandbox(out / "sandbox", write_paths=[self.runtime])
        environment = sandbox.environment(os.environ)
        environment["PATH"] = str(self.tools) + os.pathsep + environment.get("PATH", "")
        names = ["%s/%s" % (RUNTIME_DEMOS, Path(demo["file"]).stem) for demo in chain]
        arguments = ["-game", "portal2", "-multirun", "-novid", "-insecure", "-windowed",
                     "-w", str(self.args.width), "-h", str(self.args.height), "-condebug",
                     "+volume", "0", "+developer", "1", "+wait", str(self.args.start_frames),
                     "+startdemos", *names]
        progress = {"count": 0, "at": time.monotonic()}

        def done():
            if not console.is_file():
                return False
            text = console.read_text(errors="replace")
            count = text.count(FINISHED) + text.count(PLAYING)
            if count != progress["count"]:
                progress["count"], progress["at"] = count, time.monotonic()
            if any(mark in text for mark in FATAL):
                return True
            if text.count(FINISHED) >= len(chain):
                return True
            return time.monotonic() - progress["at"] > self.args.stall_seconds

        timeout = self.args.start_timeout + self.args.demo_timeout * len(chain)
        returncode, timed_out, seconds, error = sepipe_loader.run_test(
            self.args.profile, self.args.flavor, self.runtime, arguments, out / "stdout.log",
            timeout, environment=environment, stop_when=done, poll_seconds=0.5)
        log = console.read_text(errors="replace") if console.is_file() else ""
        (out / "console.log").write_text(log)
        results = evaluate(chain, log)
        info = {"worker": self.index, "chain": [demo["map"] for demo in chain],
                "seconds": round(seconds, 1), "timed_out": timed_out, "returncode": returncode,
                "kiln_error": error, "arguments": arguments, "sandbox": sandbox.finish()}
        return results, info


def run_job(worker, job, retry, lock, log):
    """A worker's chunks in order. Demos a chain did not reach are re-run in a
    fresh instance, unless the chain ended before its first demo started."""
    outcomes = []
    for chunk in job:
        pending = list(chunk)
        attempts = 0
        while pending:
            attempts += 1
            results, info = worker.run_chain(pending)
            with lock:
                log("worker %d chain %d: %d demos, %.0fs" % (
                    worker.index, worker.runs, len(pending), info["seconds"]))
            reached = next((i for i, (_, _, detail) in enumerate(results)
                            if detail.startswith("not run")), len(results))
            outcomes.extend((demo, ok, detail, info) for demo, ok, detail in results[:reached])
            unrun = results[reached:]
            if not unrun:
                break
            if reached == 0 or attempts > retry:
                outcomes.extend((demo, False, detail, info) for demo, _, detail in unrun)
                break
            pending = [demo for demo, _, _ in unrun]
    return outcomes


def self_test():
    """Selection, partitioning and log evaluation against synthetic logs, with
    a negative case for each failure the suite must detect."""
    checks = conformance_result.Checks()
    demos = [{"map": "m%d" % i, "chapter": "C%d" % (i // 2), "chapter_id": i // 2,
              "file": "m%d_x.dem" % i} for i in range(6)]
    checks.equal([d["map"] for d in select(demos, ["m3", "m1"])], ["m3", "m1"], "select.map-order")
    checks.equal([d["map"] for d in select(demos, [], ["c1"])], ["m2", "m3"], "select.chapter")
    checks.equal(len(select(demos)), 6, "select.all")
    checks.equal([d["map"] for d in select(demos, ["m2"], ["1"])], ["m2", "m3"], "select.dedup")
    for bad in (dict(maps=["nope"]), dict(chapters=["nope"])):
        try:
            select(demos, **bad)
            checks.check(False, "select.unknown-rejected")
        except SuiteError:
            checks.check(True, "select.unknown-rejected")
    for workers, chunk in ((1, 30), (2, 30), (4, 2), (9, 30), (3, 1)):
        jobs = partition(list(range(10)), workers, chunk)
        flat = sorted(item for job in jobs for part in job for item in part)
        checks.equal(flat, list(range(10)), "partition.covers.%d.%d" % (workers, chunk))
        checks.check(len(jobs) <= workers and all(len(p) <= chunk for j in jobs for p in j),
                     "partition.bounds.%d.%d" % (workers, chunk))
    two = demos[:2]
    play = lambda d: "%s%s/%s.\n" % (PLAYING, RUNTIME_DEMOS, Path(d["file"]).stem + ".dem")
    good = play(two[0]) + FINISHED + "\n" + play(two[1]) + FINISHED + "\n"
    checks.check(all(ok for _, ok, _ in evaluate(two, good)), "evaluate.good")
    cut = evaluate(two, play(two[0]) + FINISHED + "\n")
    checks.check(cut[0][1] and not cut[1][1] and cut[1][2].startswith("not run"), "evaluate.not-reached")
    hung = evaluate(two, good.replace(FINISHED + "\n" + play(two[1]), play(two[1])))
    checks.check(not hung[0][1], "evaluate.unfinished")
    err = evaluate(two, "ERROR: demo file protocol 4 outdated\n")
    checks.check(not err[0][1] and "protocol" in err[0][2], "evaluate.names-protocol-error")
    checks.check(not evaluate(two, good + "Host_Error: x\n")[-1][1], "evaluate.fatal-after-success")
    swapped = play(two[1]) + FINISHED + "\n" + play(two[0]) + FINISHED + "\n"
    checks.check(not evaluate(two, swapped)[0][1], "evaluate.out-of-order")
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    root = Path(conformance.repo_root())
    parser.add_argument("--manifest", type=Path, default=root / DEFAULT_MANIFEST)
    parser.add_argument("--map", action="append", default=[], help="map(s), comma separated")
    parser.add_argument("--chapter", action="append", default=[], help="chapter(s), comma separated")
    sepipe_loader.add_arguments(parser, "portal2")
    parser.add_argument("--workers", type=int, default=1, help="concurrent game instances")
    parser.add_argument("--chunk", type=int, default=30,
                        help="most demos one instance plays in sequence (startdemos limit)")
    parser.add_argument("--retry", type=int, default=1,
                        help="re-runs of the demos after a chain-ending failure")
    parser.add_argument("--out", type=Path, help="new evidence directory")
    parser.add_argument("--start-frames", type=int, default=120)
    parser.add_argument("--start-timeout", type=float, default=120)
    parser.add_argument("--demo-timeout", type=float, default=180,
                        help="seconds allowed per demo, loading included")
    parser.add_argument("--stall-seconds", type=float, default=90,
                        help="stop an instance whose log shows no demo event for this long")
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--list", action="store_true", help="list the selection and exit")
    parser.add_argument("--self-test", action="store_true",
                        help="check selection, partitioning and log evaluation (no game)")
    args = parser.parse_args(argv)
    if args.self_test:
        return self_test()
    split = lambda values: [part for value in values for part in value.split(",") if part]

    try:
        demos = select(load_manifest(args.manifest, root), split(args.map), split(args.chapter))
    except (SuiteError, OSError, ValueError) as error:
        parser.exit(2, "portal2_demo_suite: %s\n" % error)
    if args.list:
        for demo in demos:
            print("%s\t%s\t%s" % (demo["map"], demo["chapter"], demo["file"]))
        return 0

    if args.out is None:
        parser.error("--out is required")
    output = args.out.resolve()
    if (output / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    output.mkdir(parents=True, exist_ok=True)
    jobs = partition(demos, args.workers, args.chunk)
    evidence = {"schema": EVIDENCE_SCHEMA, "status": "incomplete",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(str(root)), "profile": args.profile,
                "flavor": args.flavor, "workers": len(jobs), "demos": [d["map"] for d in demos],
                "results": []}
    evidence_path = output / "evidence.json"
    tools = output / "tools"
    portal2_scenarios.write_fake_zenity(tools)
    lock = threading.Lock()
    say = lambda message: print(message, flush=True)
    workers = [Worker(index, args, output, tools) for index in range(len(jobs))]
    try:
        # Packaging goes through one kiln session: do it before the instances start.
        for worker, job in zip(workers, jobs):
            worker.prepare([demo for chunk in job for demo in chunk])
    except (portal2_scenarios.ScenarioError, OSError, ValueError) as error:
        evidence.update(status="fail", error=str(error))
        evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
        parser.exit(1, "portal2_demo_suite: %s\n" % error)

    outcomes = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=len(jobs)) as pool:
        futures = [pool.submit(run_job, worker, job, args.retry, lock, say)
                   for worker, job in zip(workers, jobs)]
        for future in futures:
            outcomes.extend(future.result())

    order = {demo["map"]: index for index, demo in enumerate(demos)}
    outcomes.sort(key=lambda outcome: order[outcome[0]["map"]])
    checks = conformance_result.Checks()
    for demo, ok, detail, info in outcomes:
        checks.check(ok, "%s.plays" % demo["map"], detail)
        evidence["results"].append({"map": demo["map"], "chapter": demo["chapter"],
                                    "ok": ok, "detail": detail, "worker": info["worker"],
                                    "seconds": info["seconds"]})
        say("%-4s %-34s %s" % ("PASS" if ok else "FAIL", demo["map"], detail))
    missing = set(order) - {outcome[0]["map"] for outcome in outcomes}
    for name in sorted(missing):
        checks.check(False, "%s.plays" % name, "no result")
    evidence["status"] = "pass" if checks.failures == 0 and checks.checks else "fail"
    evidence["finished_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
    say("evidence: %s" % evidence_path)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
