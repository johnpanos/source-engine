# Contract: `platform.task-runner.v1`

Module: `platform.contracts` · Types: `platform::Task`, `platform::PostResult`,
`platform::ITaskRunner`, `platform::ISequencedTaskRunner`,
`platform::ISingleThreadTaskRunner`
Header: `public/platform/contracts/task_runner.h`
Shared suite: `unittests/platformtest/task_runner/task_runner_conformance.h`
Conformance: `unittests/platformtest/task_runner/test_task_runner.cpp`
(+ `_negative`, + the optional `platform.task_runner.tsan` lane)
Providers: `platform/runners/` (`platform.runners`): `ManualTaskRunner` on a
`VirtualClock`, `ThreadTaskRunner`, `SequencedTaskRunner`
RFC: 0001 ("Threads, sequences, and injectable scheduling"), 0003 ("Sequences
and physical affinity") · Roadmap: R10 (R10-RUNNERS) · Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Features depend on how their work runs, not on physical threads. A consumer
receives the narrowest runner it needs:

- `ITaskRunner` for independent posting;
- `ISequencedTaskRunner` when its tasks must run one at a time, in order;
- `ISingleThreadTaskRunner` only for an API with real physical-thread
  affinity.

Delayed posting is part of every runner, so a consumer's tests can use virtual
time (`ManualTaskRunner`) instead of sleeping. All clauses are **required** for
the interfaces a provider claims.

## 2. Accepted inputs

- `PostTask(task)` and `PostDelayedTask(task, delayNanoseconds)` take any
  non-empty `Task` (a move-only callable) from any thread, including from a
  task of the same runner. A delay of 0 is a plain post.
- Posting an empty `Task` is a programmer error.

## 3. Results and guarantees

1. **Exactly once or destroyed.** An accepted task runs exactly once, unless
   its runner is shut down first; then it is destroyed without running. A
   refused task (`PostResult::kShutDown`) is destroyed before the post
   returns and never runs.
2. **Never inline.** A post never runs the task before it returns, and never
   blocks on running tasks.
3. **Not early.** A delayed task runs no earlier than its delay after the post,
   on the runner's clock (`VirtualClock` for `ManualTaskRunner`, the steady
   clock otherwise).
4. **Sequences.** A sequenced runner runs one task at a time. Everything a task
   did happens-before the next task of the sequence starts. Tasks posted
   without delay run in post order; posts from different threads are ordered
   by when each post returned.
5. **Identity.** `RunsTasksInCurrentSequence()` is true exactly while the
   calling thread runs a task of that sequence.
   - For a single-thread runner, `BelongsToCurrentThread()` is true exactly
     on the runner's thread, inside or outside its tasks; that thread may be
     a main loop's thread that the owner also runs (`GlibTaskRunner`).
     `RunsTasksInCurrentSequence()` equals it.
   - Every task of a single-thread runner runs on that one thread.
   - Once the runner has shut down and its thread has left it, no thread
     belongs to it, even one that reuses the thread's id.

## 4. Ownership, threading, ordering

- A runner owns its pending tasks. A task is destroyed on the runner's
  execution thread after it runs, or on the shutting-down owner's thread when
  dropped.
- **Shutdown** is the owner's operation and not part of the consumer
  interfaces. It refuses later posts, destroys pending tasks (delayed ones
  included) without running them, and returns only when no task of the
  runner is running or will run. A provider cannot be shut down from one of
  its own tasks; its providers abort on that.
- `SequencedTaskRunner` borrows its base runner, which must outlive it. The
  sequence's state is shared with its in-flight base tasks, so the sequence can
  be destroyed while base tasks are queued; they then run nothing. If the base
  refuses a post, the sequence shuts down.
- Delays of the sequence adapter are kept by the base runner, so the base
  runner's clock applies.

## 5. Evidence

- The shared suite runs against six configurations here, and against the GTK
  shell's `GlibTaskRunner` in `corpus.hammer.glib-runner`:
  - manual (virtual time) and thread;
  - a sequence over a thread, over a 4-thread test pool, and over a manual
    runner;
  - the test pool as a plain runner.
- It also checks provider-specific clauses: virtual time moves only when
  advanced, and each delayed task runs at its own due time. A destroyed
  sequence with a queued base task runs nothing, a base refusal shuts the
  sequence down, and concurrent thread-runner shutdowns both complete.
- The sensitivity suite must catch seven broken providers on the clause each
  breaks, while a conforming control passes. The broken providers:
  - a LIFO sequence;
  - an overlapping sequence;
  - one that ignores delays;
  - one that accepts after shutdown;
  - one that runs after shutdown;
  - one that runs inline;
  - one that claims every thread.
