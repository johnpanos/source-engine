#!/usr/bin/env python3
"""Shrink-only ratchet of native OS calls in Tier 0, per R103 cohort.

R103 moves Tier 0's platform internals onto the RFC 0001 foundation providers.
This counts the native calls each cohort still makes directly in tier0/ and in
the inline code of public/tier0/. A count may only fall; `check` fails when one
grows or a call appears in a new file. A cohort is closed at zero.

CPU instructions (rdtsc, pause) and compile-time atomics are not OS calls and
stay compile-time (RFC 0001 "Initial foundation capabilities").

  tools/quality/tier0_ratchet.py check
  tools/quality/tier0_ratchet.py write      # record lower counts after a slice
  tools/quality/tier0_ratchet.py redefine   # after a reviewed change to COHORTS or SCANNED
  tools/quality/tier0_ratchet.py report
  tools/quality/tier0_ratchet.py selftest
"""

from __future__ import annotations

import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BASELINE = os.path.join(ROOT, "quality", "fixtures", "tier0-abi", "ratchet.json")
SCHEMA = "tier0-ratchet/v1"
SCANNED = ("tier0", "public/tier0")

COHORTS = {
    "T1-time": [r"\bclock_gettime\s*\(", r"\bgettimeofday\s*\(", r"\bQueryPerformanceCounter\s*\(",
                r"\bQueryPerformanceFrequency\s*\(", r"\bmach_absolute_time\s*\(", r"\btimeGetTime\s*\("],
    "T2-threads": [r"\bpthread_create\s*\(", r"\bpthread_join\s*\(", r"\bpthread_setname_np\s*\(",
                   r"\bpthread_getname_np\s*\(", r"\busleep\s*\(", r"\bnanosleep\s*\(",
                   r"\bsched_yield\s*\(", r"\bsetpriority\s*\(", r"\bSYS_gettid\b", r"\bgettid\s*\(",
                   r"\b_beginthreadex\s*\(", r"\bCreateThread\s*\(", r"\bSetThreadPriority\s*\(",
                   r"\bGetCurrentThreadId\s*\(", r"\bSwitchToThread\s*\(", r"(?<![\w.>])(?<!\w::)(?<!__stdcall )(?<!void )Sleep\s*\(",
                   # Handle and identity operations (added 2026-10-08 with .inl scanning).
                   # WaitForSingleObject is not counted: Tier 0 also waits on events and
                   # mutexes with it, which are synchronization, not thread mechanism.
                   r"\bpthread_kill\s*\(", r"\bpthread_self\s*\(", r"\bpthread_attr_\w+\s*\(",
                   r"\bpthread_detach\s*\(", r"\bpthread_cancel\s*\(", r"\bpthread_setaffinity_np\s*\(",
                   r"\bOpenThread\s*\(", r"\bGetCurrentThread\s*\(",
                   r"\bSetThreadAffinityMask\s*\(",
                   r"\bGetThreadPriority\s*\(", r"\bResumeThread\s*\(", r"\bTerminateThread\s*\(",
                   r"\bGetExitCodeThread\s*\(", r"\bSetThreadDescription\s*\("],
    "T3-process-environment": [r"/proc/self/cmdline", r"\bGetCommandLine[AW]?\s*\(", r"TracerPid",
                               r"\bIsDebuggerPresent\s*\(", r"\bgetenv\s*\(", r"\benviron\b",
                               r"\bP_TRACED\b"],
    # Native debug sinks. stdio to stdout or stderr is the portable C library and
    # stays compile-time, like std::chrono (reviewed 2026-10-08).
    "T4-debug-output": [r"\bOutputDebugString[AW]?\s*\(", r"\b__android_log_(write|print|vprint)\s*\(",
                        r"\bos_log\w*\s*\(", r"\bwrite\s*\(\s*(2|STDERR_FILENO)\b"],
    "T5-crash-reporting": [r"\bsigaction\s*\(", r"(?<![\w.])signal\s*\(", r"\bSetUnhandledExceptionFilter\s*\(",
                           r"\bMiniDumpWriteDump\s*\(", r"\bbacktrace\s*\(", r"\b_Unwind_Backtrace\s*\("],
    "T6-memory-paths": [r"\bmmap\s*\(", r"\bmunmap\s*\(", r"\bmprotect\s*\(", r"\bVirtualAlloc\s*\(",
                        r"\bVirtualFree\s*\(", r"\bVirtualProtect\s*\(", r"_SC_PAGESIZE", r"\bgetpagesize\s*\(",
                        r"/proc/self/exe", r"\bGetModuleFileName[AW]?\s*\(", r"\breadlink\s*\("],
}


def strip_only_comments(text):
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def strip_comments(text):
    """Comments and string literals removed (a call named in a message is not a
    call), line structure kept."""
    return re.sub(r'"(?:[^"\\\n]|\\.)*"', '""', strip_only_comments(text))


# Patterns that are themselves literals (a /proc path, the status field name)
# match the text with only comments removed.
def literal_pattern(pattern):
    return "/proc/" in pattern or "TracerPid" in pattern


def scan(root=ROOT):
    counts = {cohort: {} for cohort in COHORTS}
    for directory in SCANNED:
        base = os.path.join(root, directory)
        for name in sorted(os.listdir(base)):
            if not name.endswith((".cpp", ".h", ".inl", ".c", ".mm")):
                continue
            path = os.path.join(base, name)
            with open(path, encoding="latin-1") as handle:
                raw = handle.read()
            code, literal = strip_comments(raw), strip_only_comments(raw)
            relative = "%s/%s" % (directory, name)
            for cohort, patterns in COHORTS.items():
                n = sum(len(re.findall(p, literal if literal_pattern(p) else code)) for p in patterns)
                if n:
                    counts[cohort][relative] = n
    return counts


def compare(baseline, current):
    problems, lower = [], []
    for cohort, files in current.items():
        recorded = baseline.get(cohort, {})
        for path, n in files.items():
            was = recorded.get(path, 0)
            if n > was:
                problems.append("%s: %s grew %d -> %d" % (cohort, path, was, n))
            elif n < was:
                lower.append((cohort, path, was, n))
        for path, was in recorded.items():
            if path not in files:
                lower.append((cohort, path, was, 0))
    return problems, lower


def totals(counts):
    return {cohort: sum(files.values()) for cohort, files in counts.items()}


def main(argv):
    command = argv[1] if len(argv) > 1 else "check"
    current = scan()
    if command == "redefine":
        # A reviewed change to what the ratchet counts (patterns or scanned
        # files) re-records everything; the commit says why.
        with open(BASELINE, "w", encoding="utf-8") as handle:
            json.dump({"schema": SCHEMA, "counts": current, "totals": totals(current)}, handle,
                      indent=1, sort_keys=True)
            handle.write("\n")
        print("redefined: %s" % totals(current))
        return 0
    if command == "write":
        if os.path.exists(BASELINE):
            with open(BASELINE, encoding="utf-8") as handle:
                problems, _ = compare(json.load(handle)["counts"], current)
            if problems:
                for p in problems:
                    print("FAIL " + p)
                print("refusing to record a higher count")
                return 1
        with open(BASELINE, "w", encoding="utf-8") as handle:
            json.dump({"schema": SCHEMA, "counts": current, "totals": totals(current)}, handle,
                      indent=1, sort_keys=True)
            handle.write("\n")
        print("recorded: %s" % totals(current))
        return 0
    if command == "report":
        for cohort, n in totals(current).items():
            print("%-26s %4d  %s" % (cohort, n, ", ".join("%s=%d" % kv for kv in sorted(current[cohort].items()))))
        return 0
    with open(BASELINE, encoding="utf-8") as handle:
        baseline = json.load(handle)["counts"]
    if command == "selftest":
        checks = failures = 0

        def expect(ok, what):
            nonlocal checks, failures
            checks += 1
            if not ok:
                failures += 1
                print("FAIL selftest: " + what)

        expect(not compare(baseline, baseline)[0], "the baseline matches itself")
        grown = json.loads(json.dumps(baseline))
        grown["T1-time"]["tier0/new_file.cpp"] = 1
        expect(bool(compare(baseline, grown)[0]), "a call in a new file is caught")
        for cohort, files in baseline.items():
            if files:
                more = json.loads(json.dumps(baseline))
                path = next(iter(files))
                more[cohort][path] += 1
                expect(bool(compare(baseline, more)[0]), "%s growth is caught" % cohort)
        sleep = next(p for p in COHORTS["T2-threads"] if "Sleep" in p and "Switch" not in p)
        expect(len(re.findall(sleep, "x.Sleep( 1 ); Sleep( 2 ); ThreadSleep( 3 ); void __stdcall Sleep( long ); static void Sleep( unsigned ); CThread::Sleep( 4 ); ::Sleep( 5 );")) == 2,
               "Sleep( matches the Win32 call (bare or ::), not a declaration or a method")
        expect(strip_comments('Error( "CreateThread() failed" ); x(); // Sleep( 1 )').count("CreateThread") == 0,
               "string literals and comments are not counted")
        print("CONFORMANCE %d %d" % (checks, failures))
        return 1 if failures else 0
    problems, lower = compare(baseline, current)
    for p in problems:
        print("FAIL " + p)
    for cohort, path, was, n in lower:
        print("note: %s %s fell %d -> %d (run write to record)" % (cohort, path, was, n))
    print("totals: %s" % totals(current))
    print("CONFORMANCE %d %d" % (sum(totals(current).values()) + len(problems), len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
