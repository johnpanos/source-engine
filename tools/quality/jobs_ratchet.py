#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""RFC 0003 phase I ratchet: one task system, and the work still outside it.

    python3 tools/quality/jobs_ratchet.py check [--root DIR] [--write]
    python3 tools/quality/jobs_ratchet.py sensitivity

RFC/0003-dependency-aware-job-system.md#frame-wide-scheduling-goals-amended-2026-09-28
(row R94). Two kinds of rule.

Invariants (zero, never recordable; --write refuses while any holds):

  wave-executor     the retired wave executor or a fork/join backend hook
                    (PooledExecutor, ParallelForWithCaller) in first-party
                    code: J1 says no product caller of a wave loop, so none
                    may exist to be called;
  backend-fork-join a ParallelFor anywhere in the job system or its pool
                    bridge (jobsystem/, public/jobsystem/,
                    vstdlib/jobgraph_*): the backend runs tasks only;
  backend-surface   public/jobsystem/worker_backend.h declares exactly the
                    virtual functions WorkerCount, PostTask, SettleTask and
                    ShouldRunInline (and the destructor): no barrier can be
                    added to the boundary under another name;
  foreign-executor  a class implementing IGraphExecutor or IWorkerBackend
                    outside the job system and its pool bridge: a second
                    scheduler under any name (structural; added 2026-10-08
                    because the wave-executor invariant matches names);
  thread-executor   ParallelExecutor, DynamicScope or ThreadWorkerBackend
                    (executors that own threads) constructed outside the job
                    system, its tests and tools: products run graphs on
                    TaskExecutor over the root's pool (J1, J3).

Ratchets (exact per file and category, shrink-only, like the RFC 0015
lookup ratchet):

  thread-create     a first-party thread started outside a declared owner
                    (J3, static half): std::thread, pthread_create,
                    ThreadCreate/CreateSimpleThread/CreateThread/
                    SDL_CreateThread/_beginthreadex, a CThread or
                    CWorkerThread subclass, CreateThreadPool. Files of a
                    declared owner (RFC 0003 J3: the compute pool, MatQueue,
                    filesystem I/O, audio, device/driver threads, declared
                    blocking lanes, the thread primitives themselves) are
                    listed under "declared" with a reason and reported, not
                    counted. J3 needs this category at 0;
  fork-join         a host-side blocking fork/join outside the task graph
                    (J2, J6): ParallelProcess, ParallelLoopProcess,
                    CParallelProcessor, RunThreadPoolJobBatch,
                    ExecuteParallelBatch, other executors' ParallelFor. Each
                    call blocks its caller until a batch ends; a frame-wide
                    graph replaces them with nodes;
  pool-wait         a blocking wait on pool work: WaitForFinish,
                    WaitForFinishAndRelease, YieldWait;
  legacy-job-api    the second task API (direction audit 2026-10-08): the
                    legacy pool's jobs and calls (CJob, CFunctorJob,
                    CJobSet, IThreadPool/CThreadPool, ThreadExecute*,
                    QueueCall*, CreateFunctorJob, g_pThreadPool) outside the
                    job system. One task system means one task API; the
                    pool's implementation, its frozen header and the bridge
                    are declared owners;
  unaudited-node    a host frame node in legacy order (J6 host-graph census):
                    a HOST_FRAME_PHASE/HostFrame_AddPhase call (a serial
                    host-frame phase) or a FRAME_DOMAIN_ALL declaration
                    outside the job system's own definitions.

The counts equal tools/quality/jobs_ratchet.json exactly: a new file, a
higher count, or a lower count not yet recorded fails, so the list only
shrinks and stays exact. --write records the current counts after review; a
migration commit records its own decrease in the same change. A declared
file must exist, carry a reason, and still contain a site of its category
(a stale declaration fails). The totals must agree with the files.

Scope: first-party C/C++ (tools/render/retirement_scans.py's listing),
skipping tests (unittests/, */tests/), reference and vendored trees (games/,
external/, devtools/), toolchain probes (quality/) and the offline tool
programs under utils/, which run outside the product frame. Comments, string literals and #define bodies are
stripped before matching. Ends with one checks-v1 record. Python 3 standard
library only.
"""

import argparse
import json
import re
import shutil
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "render"))
from conformance_result import Checks  # noqa: E402
# One owner for first-party source listing and comment stripping.
from retirement_scans import sources, strip_code  # noqa: E402

SCHEMA = "jobs-ratchet/v1"
RATCHET = "tools/quality/jobs_ratchet.json"
SKIPPED = ("unittests/", "games/", "external/", "devtools/", "utils/", "quality/")
SKIPPED_PARTS = ("/tests/",)
JOB_SYSTEM = ("jobsystem/", "public/jobsystem/")
POOL_BRIDGE = ("vstdlib/jobgraph_", "public/vstdlib/jobgraph_")
WORKER_BACKEND = "public/jobsystem/worker_backend.h"
BACKEND_VIRTUALS = {"WorkerCount", "PostTask", "SettleTask", "ShouldRunInline"}

INVARIANTS = ("wave-executor", "backend-fork-join", "backend-surface", "thread-executor",
              "foreign-executor")
RATCHETS = ("thread-create", "fork-join", "pool-wait", "unaudited-node", "legacy-job-api")
# The Game Coordinator SDK's gcsdk::CJob is an unrelated, unbuilt job system.
LEGACY_JOB_API_SKIPPED = ("gcsdk/", "public/gcsdk/")

WAVE_EXECUTOR = re.compile(r"\b(?:PooledExecutor|ParallelForWithCaller)\b")
PARALLEL_FOR = re.compile(r"\bParallelFor\b")
THREAD_EXECUTOR = re.compile(
    r"\b(?:ParallelExecutor|DynamicScope|ThreadWorkerBackend)\b\s*(?:[\w:<>]*\s*)?[({]"
    r"|\bmake_unique\s*<\s*(?:jobsystem::)?(?:ParallelExecutor|DynamicScope|ThreadWorkerBackend)\b"
    r"|\bnew\s+(?:jobsystem::)?(?:ParallelExecutor|DynamicScope|ThreadWorkerBackend)\b"
    r"|\b(?:jobsystem::)?(?:ParallelExecutor|DynamicScope|ThreadWorkerBackend)\s+\w+\s*[({;]")
THREAD_CREATE = re.compile(
    r"\bstd::thread\b(?!\s*::)"
    r"|\bpthread_create\s*\("
    r"|\b(?:ThreadCreate|CreateSimpleThread|CreateThread|SDL_CreateThread|_beginthreadex)\s*\("
    r"|\bpublic\s+(?:CThread|CWorkerThread)\b"
    r"|\bCreateThreadPool\s*\(")
FORK_JOIN = re.compile(
    r"\b(?:ParallelProcess|ParallelLoopProcess|RunThreadPoolJobBatch|ExecuteParallelBatch"
    r"|ParallelFor)\s*\("
    r"|\bCParallelProcessor\b")
POOL_WAIT = re.compile(r"\b(?:WaitForFinish|WaitForFinishAndRelease|YieldWait)\s*\(")
UNAUDITED_NODE = re.compile(
    r"\b(?:HOST_FRAME_PHASE|HostFrame_AddPhase)\s*\(\s*phases\b|\bFRAME_DOMAIN_ALL\b")
# The second task API: the legacy pool's jobs and calls (direction audit
# 2026-10-08). One task system means one task API in first-party code; the
# legacy pool's own implementation, its frozen mod-facing header and the
# bridge that turns it into an IWorkerBackend are declared owners. Calls the
# fork-join and pool-wait categories already count are not counted again.
LEGACY_JOB_API = re.compile(
    r"\b(?:CJob|CFunctorJob|CJobSet|IThreadPool|CThreadPool)\b"
    r"|\b(?:ThreadExecute\w*|QueueCall\w*|CreateFunctorJob)\s*\("
    r"|\bg_pThreadPool\b")
# An executor or worker backend implemented outside the job system and its
# pool bridge: a second scheduler under any name (structural, unlike the
# name-based wave-executor invariant).
FOREIGN_EXECUTOR = re.compile(
    r"\b(?:class|struct)\s+\w+(?:\s+final)?\s*:\s*(?:public\s+)?(?:jobsystem::)?"
    r"(?:IGraphExecutor|IWorkerBackend)\b")
VIRTUAL = re.compile(r"\bvirtual\b[^;{(]*?(~?\w+)\s*\(")


def strip_defines(code):
    """#define lines and their continuations blanked; lines kept."""
    out, in_define = [], False
    for line in code.split("\n"):
        starts = line.lstrip().startswith("#") and re.match(r"\s*#\s*define\b", line)
        if starts or in_define:
            in_define = line.rstrip().endswith("\\")
            out.append("")
        else:
            out.append(line)
    return "\n".join(out)


def scope(path):
    return not path.startswith(SKIPPED) and not any(part in "/" + path for part in SKIPPED_PARTS)


def count_file(path, text):
    """{category: count} for one file, invariants included."""
    code = strip_defines(strip_code(text))
    found = {}
    in_job_system = path.startswith(JOB_SYSTEM)

    def add(category, n):
        if n:
            found[category] = found.get(category, 0) + n

    add("wave-executor", len(WAVE_EXECUTOR.findall(code)))
    if path.startswith(JOB_SYSTEM + POOL_BRIDGE):
        add("backend-fork-join", len(PARALLEL_FOR.findall(code)))
    if path == WORKER_BACKEND:
        names = set(VIRTUAL.findall(code))
        names.discard("~IWorkerBackend")
        add("backend-surface", len(names ^ BACKEND_VIRTUALS))
    if not in_job_system:
        add("thread-executor", len(THREAD_EXECUTOR.findall(code)))
    add("thread-create", len(THREAD_CREATE.findall(code)))
    if not in_job_system and not path.startswith(POOL_BRIDGE):
        add("fork-join", len(FORK_JOIN.findall(code)))
    add("pool-wait", len(POOL_WAIT.findall(code)))
    if not in_job_system:
        add("unaudited-node", len(UNAUDITED_NODE.findall(code)))
    if not in_job_system and not path.startswith(POOL_BRIDGE):
        add("foreign-executor", len(FOREIGN_EXECUTOR.findall(code)))
    if not in_job_system and not path.startswith(LEGACY_JOB_API_SKIPPED):
        add("legacy-job-api", len(LEGACY_JOB_API.findall(code)))
    return found


def scan(root):
    counts = {}
    for path in sources(root):
        if not scope(path):
            continue
        found = count_file(path, (Path(root) / path).read_text(encoding="utf-8", errors="replace"))
        if found:
            counts[path] = found
    if (Path(root) / WORKER_BACKEND).is_file() is False and scope(WORKER_BACKEND):
        counts.setdefault(WORKER_BACKEND, {})["backend-surface"] = 1  # the boundary is gone
    return counts


def load_ratchet(root):
    path = Path(root) / RATCHET
    if not path.exists():
        return None
    data = json.loads(path.read_text())
    if data.get("schema") != SCHEMA:
        raise ValueError("%s: schema must be %s" % (RATCHET, SCHEMA))
    return data


def split(counts, declared):
    """Ratcheted counts per file (declared sites removed), declared sites,
    and invariant violations."""
    ratcheted, owned, violations = {}, {}, []
    for path, found in counts.items():
        for category, n in found.items():
            if category in INVARIANTS:
                violations.append("%s %s: %d" % (path, category, n))
            elif path in declared.get(category, {}):
                owned.setdefault(category, {})[path] = n
            else:
                ratcheted.setdefault(path, {})[category] = n
    return ratcheted, owned, violations


def totals(files):
    out = {}
    for found in files.values():
        for category, n in found.items():
            out[category] = out.get(category, 0) + n
    return dict(sorted(out.items()))


def check(root, checks, write=False):
    counts = scan(root)
    previous = load_ratchet(root)
    declared = previous.get("declared", {}) if previous else {}
    ratcheted, owned, violations = split(counts, declared)
    for line in violations:
        print("jobs-ratchet: invariant:", line)
    if write:
        if violations:
            print("jobs-ratchet: refusing --write: %d invariant violation(s)" % len(violations))
        else:
            data = {"schema": SCHEMA,
                    "note": "RFC 0003 phase I (R94): work outside the one task system, per file "
                            "and category. Only shrinks; rewrite with `jobs_ratchet.py check "
                            "--write` after review. J3 needs thread-create at 0; J6 shrinks "
                            "unaudited-node. Declared owners are listed with a reason.",
                    "declared": declared,
                    "totals": totals(ratcheted),
                    "files": ratcheted}
            (Path(root) / RATCHET).write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")
            print("jobs-ratchet: recorded %d site(s) in %d file(s)"
                  % (sum(totals(ratcheted).values()), len(ratcheted)))
    checks.check(not violations, "jobs-ratchet.invariants",
                 "%d invariant violation(s): the task system admits no wave executor, fork/join "
                 "backend or thread-owning executor in products" % len(violations))
    ratchet = load_ratchet(root)
    checks.check(ratchet is not None, "jobs-ratchet.exists", "no %s" % RATCHET)
    if ratchet is None:
        return
    # Declarations: known category, a reason, an existing file, a live site.
    stale = []
    for category, files in ratchet.get("declared", {}).items():
        checks.check(category in RATCHETS, "jobs-ratchet.declared-category",
                     "declared category %r is not a ratchet" % category)
        for path, reason in files.items():
            checks.check(bool(str(reason).strip()), "jobs-ratchet.declared-has-reason",
                         "%s %s is declared without a reason" % (path, category))
            if counts.get(path, {}).get(category, 0) == 0:
                stale.append("%s %s" % (path, category))
    for line in stale:
        print("jobs-ratchet: stale declaration:", line)
    checks.check(not stale, "jobs-ratchet.declared-live",
                 "%d declaration(s) name a file without a site; remove them" % len(stale))
    recorded = ratchet.get("files", {})
    grew, shrank = [], []
    for path in sorted(set(ratcheted) | set(recorded)):
        cur, rec = ratcheted.get(path, {}), recorded.get(path, {})
        for category in sorted(set(cur) | set(rec)):
            c, r = cur.get(category, 0), rec.get(category, 0)
            if c > r:
                grew.append("%s %s: %d, the ratchet allows %d" % (path, category, c, r))
            elif c < r:
                shrank.append("%s %s: down to %d from %d; record it with --write"
                              % (path, category, c, r))
    for line in grew:
        print("jobs-ratchet: grew:", line)
    for line in shrank:
        print("jobs-ratchet: stale:", line)
    checks.check(not grew, "jobs-ratchet.no-growth", "%d site count(s) grew" % len(grew))
    checks.check(not shrank, "jobs-ratchet.exact",
                 "%d site count(s) shrank without a ratchet update" % len(shrank))
    checks.check(ratchet.get("totals") == totals(recorded), "jobs-ratchet.totals-consistent",
                 "the recorded totals disagree with the recorded files")
    checks.check(all(c in RATCHETS for f in recorded.values() for c in f),
                 "jobs-ratchet.recorded-categories", "the ratchet records a non-ratchet category")
    for category, n in totals(ratcheted).items():
        print("INFO jobs-ratchet: %s %d site(s)%s" % (
            category, n, "; J3 needs 0" if category == "thread-create" else ""))
    for category, files in sorted(owned.items()):
        print("INFO jobs-ratchet: %s %d site(s) in %d declared owner file(s)"
              % (category, sum(files.values()), len(files)))


def sensitivity():
    checks = Checks()
    with tempfile.TemporaryDirectory() as tmp:
        base = Path(tmp) / "clean"
        for d in ("public/jobsystem", "jobsystem", "engine", "vstdlib", "tier0", "game/client",
                  "unittests/jobsystemtest", "utils/vrad", "tools/quality"):
            (base / d).mkdir(parents=True)
        (base / "public/jobsystem/worker_backend.h").write_text(
            "class IWorkerBackend {\npublic:\n\tvirtual ~IWorkerBackend() {}\n"
            "\tvirtual int WorkerCount() const = 0;\n"
            "\tvirtual void *PostTask( WorkerTaskFn task, void *context ) = 0;\n"
            "\tvirtual bool SettleTask( void *ticket ) = 0;\n"
            "\tvirtual bool ShouldRunInline() = 0;\n};\n")
        (base / "jobsystem/parallel_executor.cpp").write_text(
            "void W() { std::thread t( [] {} ); }\n")
        (base / "public/jobsystem/declared_frame_graph.h").write_text(
            "enum { FRAME_DOMAIN_ALL = 0xFFFFFFFFu };\n")
        (base / "tier0/threadtools.cpp").write_text(
            "void T() { pthread_create( 0, 0, 0, 0 ); }\n")
        (base / "engine/host_frame_phases.h").write_text(
            "#define HOST_FRAME_PHASE( phases, name, s, arg ) \\\n"
            "\tHostFrame_AddPhase( phases, #name, 0, s, arg )\n"
            "static void HostFrame_AddPhase( CUtlVector<int> &phases ) {}\n"
            "void B() { HOST_FRAME_PHASE( phases, CmdExecute, s, 0 ); "
            "HOST_FRAME_PHASE( phases, FrameEnd, s, 0 ); }\n")
        (base / "game/client/render_start.cpp").write_text(
            "// ParallelProcess( a ) in a comment is not a site\n"
            "void R() { RunThreadPoolJobBatch( pool, desc, mode ); job->WaitForFinish(); }\n")
        (base / "unittests/jobsystemtest/wave.h").write_text("class PooledExecutor {};\n")
        (base / "utils/vrad/threads.cpp").write_text("void V() { CreateThread( 0 ); }\n")
        seed = {"schema": SCHEMA, "declared": {"thread-create": {
            "tier0/threadtools.cpp": "the thread primitives"}}, "files": {}, "totals": {}}
        (base / RATCHET).write_text(json.dumps(seed))
        clean = Checks()
        check(base, clean, write=True)
        checks.check(clean.failures == 0, "control.write-passes", "%d failure(s)" % clean.failures)
        recorded = json.loads((base / RATCHET).read_text())["files"]
        checks.check(recorded.get("engine/host_frame_phases.h") == {"unaudited-node": 2},
                     "control.define-body-not-counted", str(recorded))
        checks.check(recorded.get("game/client/render_start.cpp") == {"fork-join": 1,
                                                                       "pool-wait": 1},
                     "control.comment-not-counted", str(recorded))
        checks.check("tier0/threadtools.cpp" not in recorded, "control.declared-not-counted", "")
        checks.check("jobsystem/parallel_executor.cpp" in recorded
                     and "public/jobsystem/declared_frame_graph.h" not in recorded,
                     "control.job-system-definitions", str(recorded))
        checks.check(not any(p.startswith(("unittests/", "utils/")) for p in recorded),
                     "control.scope", str(recorded))
        again = Checks()
        check(base, again)
        checks.check(again.failures == 0, "control.clean-tree-passes", "")

        def seeded(label, mutate, expect_write_refused=False):
            tree = Path(tmp) / label
            shutil.copytree(base, tree)
            mutate(tree)
            result = Checks()
            check(tree, result)
            checks.check(result.failures > 0, "detects.%s" % label, "the scan passed")
            if expect_write_refused:
                before = (tree / RATCHET).read_text()
                check(tree, Checks(), write=True)
                checks.check((tree / RATCHET).read_text() == before,
                             "refuses-write.%s" % label, "--write recorded an invariant")

        def write(path, text, append=False):
            def mutate(t):
                target = t / path
                target.parent.mkdir(parents=True, exist_ok=True)
                old = target.read_text() if append and target.exists() else ""
                target.write_text(old + text)
            return mutate

        seeded("wave-executor", write("render/composition/core.cpp",
                                      "PooledExecutor cull( backend );\n"), True)
        seeded("fork-join-hook", write("engine/frame.cpp",
                                       "void F() { b->ParallelForWithCaller( 1, f, g ); }\n"), True)
        seeded("backend-parallel-for", write("jobsystem/pooled.cpp",
                                             "void P( IWorkerBackend *b ) { b->ParallelFor( 4 ); }\n"),
               True)
        seeded("backend-extra-virtual", write(
            "public/jobsystem/worker_backend.h",
            "class Extra { virtual void Barrier( int n ) = 0; };\n", append=True), True)
        seeded("backend-missing-virtual", lambda t: (t / WORKER_BACKEND).write_text(
            (t / WORKER_BACKEND).read_text().replace(
                "\tvirtual bool SettleTask( void *ticket ) = 0;\n", "")), True)
        seeded("backend-deleted", lambda t: (t / WORKER_BACKEND).unlink(), True)
        seeded("thread-executor", write("render/composition/lab.cpp",
                                        "jobsystem::ParallelExecutor workers( 4 );\n"), True)
        seeded("thread-executor-heap", write(
            "engine/cull.cpp", "auto e = std::make_unique<jobsystem::ThreadWorkerBackend>( 2 );\n"),
            True)
        seeded("new-thread", write("game/client/streamer.cpp",
                                   "void S() { std::thread loader( Load ); }\n"))
        seeded("more-thread", write("jobsystem/parallel_executor.cpp",
                                    "void X() { std::thread u( [] {} ); }\n", append=True))
        seeded("cthread-subclass", write("engine/audio_mix.cpp",
                                         "class CMixer : public CThread {};\n"))
        seeded("new-fork-join", write("engine/bones.cpp",
                                      "void B() { ParallelProcess( items, n, fn ); }\n"))
        seeded("new-pool-wait", write("engine/load.cpp",
                                      "void L( CJob *j ) { j->WaitForFinishAndRelease(); }\n"))
        seeded("new-host-phase", write("engine/host_frame_phases.h",
                                       "void C() { HOST_FRAME_PHASE( phases, Extra, s, 0 ); }\n",
                                       append=True))
        seeded("new-domain-all", write("game/client/steps.h",
                                       "FrameAccess a = { jobsystem::FRAME_DOMAIN_ALL, true };\n"))
        seeded("foreign-executor", write(
            "engine/my_scheduler.cpp",
            "class CWaveRunner final : public jobsystem::IGraphExecutor {};\n"), True)
        seeded("new-legacy-job", write("game/client/loader.cpp",
                                       "void L() { g_pThreadPool->QueueCall( Load ); }\n"))
        seeded("stale-ratchet", write("game/client/render_start.cpp", "void R() {}\n"))
        seeded("stale-declaration", lambda t: (t / "tier0/threadtools.cpp").write_text("\n"))

        def tamper(**fields):
            def mutate(t):
                data = json.loads((t / RATCHET).read_text())
                for key, value in fields.items():
                    data[key] = value(data) if callable(value) else value
                (t / RATCHET).write_text(json.dumps(data))
            return mutate

        seeded("declared-without-reason", tamper(declared={"thread-create": {
            "tier0/threadtools.cpp": " "}}))
        seeded("declared-unknown-category", tamper(declared=lambda d: dict(
            d["declared"], **{"pool-hope": {"tier0/threadtools.cpp": "x"}})))
        seeded("totals-tampered", tamper(totals={}))
        seeded("invariant-recorded", tamper(files=lambda d: dict(
            d["files"], **{"engine/x.cpp": {"wave-executor": 1}}),
            totals=lambda d: dict(d["totals"], **{"wave-executor": 1})))
        seeded("ratchet-missing", lambda t: (t / RATCHET).unlink())
    return checks


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check", "sensitivity"])
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--write", action="store_true", help="check: record the current counts")
    args = parser.parse_args(argv)
    if args.command == "sensitivity":
        if args.write:
            parser.error("--write applies to check only")
        checks = sensitivity()
    else:
        checks = Checks()
        check(args.root, checks, write=args.write)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
