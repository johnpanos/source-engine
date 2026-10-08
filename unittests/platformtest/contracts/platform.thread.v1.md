# Contract: `platform.thread.v1`

Module: `platform.contracts` · Types: `platform::IThreads`, `ThreadId`,
`ThreadHandle`, `ThreadOptions`, `ThreadPriority`, `ThreadResult`
Header: `public/platform/contracts/thread.h`
Shared suite: `unittests/platformtest/thread/thread_conformance.h`
Conformance: `unittests/platformtest/thread/test_thread.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/thread/fake_threads.h`
RFC: 0001 (foundation capability "Threading") · Migration: `PLAT-THREAD-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Physical threads beneath `platform.task-runner.v1`. Feature code posts to
runners; only providers that own a pool or an API with real thread affinity
(executor workers, an audio callback thread, the render sequence) start threads.
Start, join, ids, sleep and hardware concurrency are **required**. Names and
priority are **optional**: a provider reports `kUnsupported` consistently.
Mutexes, condition variables and atomics stay compile-time `std::` types.

## 2. Accepted inputs

- `Start`: a non-null entry; any context; options with an optional name, a
  priority and a stack size (0 = default, otherwise a minimum).
- `Join`: a handle `Start` returned and nobody has joined, from a thread other
  than the one it names.
- `SetCurrentName`: a non-null UTF-8 name.

## 3. Results and guarantees

- `Start` → `kOk` with a nonzero handle; the entry runs exactly once. On
  failure `out` is unchanged and the entry never runs. An unsupported priority
  is not a failure.
- `Join` → `kOk` once the entry has returned. Everything the entry wrote
  happens-before `Join` returns. A second join, an unknown handle or a
  self-join → `kInvalidArgument`.
- `CurrentId()` is nonzero, stable for the thread and distinct among live
  threads; `IdOf(handle)` equals what that thread sees, and is 0 for an unknown
  or released handle.
- A name from the options is the thread's name when its entry starts. Names keep
  their first `kThreadNameMaxBytes` (15) bytes. `GetCurrentName` follows the
  paths convention: no partial write.
- `SleepFor(ns)` lasts at least `ns` on the provider's monotonic clock;
  `SleepFor(0)` yields. `HardwareConcurrency() >= 1`.

## 4. Ownership, threading, ordering

Every started handle is joined exactly once before the provider is destroyed;
an unjoined handle at destruction is a programmer error. All methods may be
called from any thread. The entry may start at any time after `Start` and has
returned before `Join` does (the fake runs it inside `Join`).

## 5. Invariants and legal sequences

`Start` → (`IdOf`)* → `Join`. A handle value is never reused while live.

## 6. Side effects and performance

Native providers create OS threads; no logging. Priority changes never need
elevated privileges.

## 7. Conformance suite and providers

- The shared predicate is safe under real concurrency (atomics, reads after
  `Join`, a bounded yielding poll). It covers null-entry refusal, exactly-once
  execution with visible writes, ids, self-join, option names, rename,
  truncation, small buffers, priority, double and unknown joins, four live
  threads with distinct ids, and sleep duration against the supplied clock.
- `test_thread.cpp`: the fake with names and priority, without names, and
  without priority. **Positive; certifies contract semantics only.**
- `test_thread_negative.cpp` (`sensitivity`): an entry that never runs or runs
  twice, a double join that succeeds, an accepted null entry, `IdOf` differing
  from `CurrentId`, an ignored option name, a partial name write, a short sleep
  and zero concurrency are each caught.

### Native providers (installed 2026-10-08, R26)

- POSIX (`platform/posix/foundation_providers.h`, library `platform_posix`):
  `CreatePosixThreads` (pthreads; names through pthread_setname_np, priority as a per-thread nice value or Apple QoS class). Row `platform.foundation.posix` runs this suite and the native
  clauses on Linux with g++ and clang++, in release, under TSan and ASan/UBSan
  (`.tsan`, `.asan`) and as i386 (`.i386`). The same source cross-builds for
  Android arm64-v8a and x86_64 at API 29 (`tools/quality/android_foundation.py`)
  and compiles for iOS arm64.
- Win32 (`platform/win32/foundation_providers.h`): `CreateWin32Threads` (_beginthreadex; SetThreadDescription when present, SetThreadPriority). Row
  `platform.foundation.win32` runs as a static PE under Wine
  (`tools/quality/parity_wine.py check --suite platform.foundation.win32`).
- Evidence and what is still unverified: `RFC/0001-foundation-providers-progress.md`.
