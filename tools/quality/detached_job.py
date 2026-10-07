#!/usr/bin/env python3
"""Run a long command detached, and wait for it by its recorded exit, not by name.

    python3 tools/quality/detached_job.py start --name intro4-fast -- \\
        python3 tools/quality/map_lighting.py --bsp ... --map ...
    python3 tools/quality/detached_job.py wait intro4-fast     # blocks; exits with the job's code
    python3 tools/quality/detached_job.py status intro4-fast   # one look, never blocks

`start` launches a supervisor in its own session (setsid: closing the shell or
killing a process group does not reach it), which runs the command as its child
with stdout and stderr in the job's log, and records in the job's state file
(build/jobs/<name>.json): the command, both PIDs, the start time and, when the
child ends, its exit code (a negative code is the signal that ended it) and the
end time. Nothing here finds a process by matching its command line: a pattern
(pgrep -f, pkill -f) also matches the shell that runs it, so such a watcher can
see a dead job as alive or kill itself (2026-10-06: a crashed map bake went
unnoticed for 12 minutes that way).

`wait` returns as soon as the exit is recorded, or as soon as the supervisor is
gone without recording one (then it reports "supervisor lost", code 125). It
prints the outcome, the last lines of the log and, for a map lighting build
(`[step] running...` lines), the last step started and the last finished, and
exits with the job's code, so a caller (or a background task's notification)
sees a crash at once. `--timeout` bounds the wait (code 124 when it expires;
the job keeps running).
"""

import argparse
import json
import os
import re
import signal
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
JOBS = ROOT / "build" / "jobs"
POLL_SECONDS = 2.0
LOST = 125
TIMED_OUT = 124
STEP = re.compile(r"^\[([a-z0-9-]+)\] (running|done|cached)")
ERROR = re.compile(r"traceback|error|failed|exception", re.IGNORECASE)


def paths(name, jobs):
    if not re.fullmatch(r"[A-Za-z0-9._-]+", name):
        raise SystemExit("job names are letters, digits, '.', '_' and '-'")
    return jobs / (name + ".json"), jobs / (name + ".log")


def read_state(state):
    try:
        return json.loads(state.read_text())
    except (OSError, ValueError):
        return None


def write_state(state, record):
    temporary = state.with_name(state.name + ".tmp")
    temporary.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    os.replace(temporary, state)


def alive(pid):
    try:
        os.kill(pid, 0)
    except ProcessLookupError:
        return False
    except PermissionError:
        return True
    return True


def cmd_start(args):
    state, log = paths(args.name, args.jobs)
    args.jobs.mkdir(parents=True, exist_ok=True)
    previous = read_state(state)
    if previous and "exit_code" not in previous and alive(previous.get("supervisor_pid", -1)):
        raise SystemExit("job %s is still running (supervisor %d)" %
                         (args.name, previous["supervisor_pid"]))
    if not args.command:
        raise SystemExit("start needs a command after --")
    command = args.command[1:] if args.command[0] == "--" else args.command
    write_state(state, {"name": args.name, "command": command, "cwd": os.getcwd(),
                        "log": str(log), "started": time.time()})
    supervisor = subprocess.Popen(
        [sys.executable, str(Path(__file__).resolve()), "_supervise", str(state), str(log)],
        stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        start_new_session=True)
    # The supervisor records the child's PID; wait until it has (or died).
    deadline = time.time() + 30
    while time.time() < deadline:
        record = read_state(state) or {}
        if "pid" in record or "exit_code" in record:
            break
        if supervisor.poll() is not None:
            break
        time.sleep(0.1)
    record = read_state(state) or {}
    print(json.dumps({"job": args.name, "supervisor_pid": supervisor.pid,
                      "pid": record.get("pid"), "log": str(log), "state": str(state)}))


def cmd_supervise(args):
    state, log = Path(args.state), Path(args.log)
    record = read_state(state)
    with open(log, "w") as out:
        try:
            child = subprocess.Popen(record["command"], cwd=record["cwd"], stdin=subprocess.DEVNULL,
                                     stdout=out, stderr=subprocess.STDOUT)
        except OSError as error:
            record.update(supervisor_pid=os.getpid(), exit_code=127, ended=time.time(),
                          error=str(error))
            write_state(state, record)
            return
        record.update(supervisor_pid=os.getpid(), pid=child.pid)
        write_state(state, record)
        # Forward a termination request to the job, then record how it ended.
        signal.signal(signal.SIGTERM, lambda *_: child.terminate())
        code = child.wait()
    record.update(exit_code=code, ended=time.time())
    write_state(state, record)


def summary(record, log_path, lines):
    text = []
    try:
        log = Path(log_path).read_text(errors="replace").splitlines()
    except OSError:
        log = []
    steps = [m.groups() for m in map(STEP.match, log) if m]
    if steps:
        started = [s for s, kind in steps if kind == "running"]
        finished = [s for s, kind in steps if kind != "running"]
        text.append("last step started: %s; last finished: %s" %
                    (started[-1] if started else "-", finished[-1] if finished else "-"))
    errors = [line for line in log if ERROR.search(line)]
    if errors:
        text.append("error lines:")
        text += ["  " + line for line in errors[-6:]]
    text.append("log tail (%s):" % log_path)
    text += ["  " + line for line in log[-lines:]]
    return "\n".join(text)


def outcome(record):
    code = record["exit_code"]
    elapsed = record.get("ended", time.time()) - record.get("started", time.time())
    how = ("succeeded" if code == 0 else
           "killed by signal %d" % -code if code < 0 else "failed with exit code %d" % code)
    return "job %s %s after %.0f s" % (record["name"], how, elapsed)


def cmd_wait(args):
    state, log = paths(args.name, args.jobs)
    record = read_state(state)
    if record is None:
        raise SystemExit("no job named %s (%s)" % (args.name, state))
    deadline = time.time() + args.timeout if args.timeout else None
    while True:
        record = read_state(state) or record
        if "exit_code" in record:
            print(outcome(record))
            print(summary(record, log, args.lines))
            code = record["exit_code"]
            sys.exit(code if 0 <= code < 124 else 1 if code > 0 else 128 - code)
        supervisor = record.get("supervisor_pid")
        if supervisor is not None and not alive(supervisor):
            record = read_state(state) or record  # it may have written just now
            if "exit_code" not in record:
                print("job %s: supervisor lost (pid %d) without recording an exit" %
                      (args.name, supervisor))
                print(summary(record, log, args.lines))
                sys.exit(LOST)
            continue
        if deadline is not None and time.time() > deadline:
            print("job %s still running after %.0f s" % (args.name, args.timeout))
            print(summary(record, log, args.lines))
            sys.exit(TIMED_OUT)
        time.sleep(POLL_SECONDS)


def cmd_status(args):
    state, log = paths(args.name, args.jobs)
    record = read_state(state)
    if record is None:
        raise SystemExit("no job named %s" % args.name)
    if "exit_code" in record:
        print(outcome(record))
    elif record.get("supervisor_pid") and alive(record["supervisor_pid"]):
        print("job %s running for %.0f s (pid %s)" %
              (args.name, time.time() - record["started"], record.get("pid")))
    else:
        print("job %s: supervisor lost without recording an exit" % args.name)
    print(summary(record, log, args.lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--jobs", type=Path, default=JOBS, help="state directory")
    sub = parser.add_subparsers(dest="action", required=True)
    start = sub.add_parser("start")
    start.add_argument("--name", required=True)
    start.add_argument("command", nargs=argparse.REMAINDER)
    for action in ("wait", "status"):
        each = sub.add_parser(action)
        each.add_argument("name")
        each.add_argument("--lines", type=int, default=8)
        if action == "wait":
            each.add_argument("--timeout", type=float, default=0.0)
    supervise = sub.add_parser("_supervise")
    supervise.add_argument("state")
    supervise.add_argument("log")
    args = parser.parse_args()
    {"start": cmd_start, "wait": cmd_wait, "status": cmd_status,
     "_supervise": cmd_supervise}[args.action](args)


if __name__ == "__main__":
    main()
