# Contract: `platform.process-environment.v1`

Module: `platform.contracts` · Types: `platform::IProcessEnvironment`,
`platform::DebuggerState`
Header: `public/platform/contracts/process_environment.h`
Shared suite: `unittests/platformtest/process_environment/process_environment_conformance.h`
Conformance: `unittests/platformtest/process_environment/test_process_environment.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/process_environment/fake_process_environment.h`
RFC: 0001 (foundation capability "Process environment") · Migration: `PLAT-ENV-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Arguments, environment variables, the process id and debugger state, read from
a snapshot the provider takes when the root composes it. **Read-only by design**:
no consumer mutates process-global state (`setenv` is not thread-safe on POSIX).
What arguments mean stays with the engine's command-line owner. All clauses are
**required**; a platform that cannot tell debugger state reports `kUnknown`, and
a mobile container may supply no arguments.

## 2. Accepted inputs

- Argument indices of any value; out-of-range ones are absent.
- Variable names of any value; null, empty or containing `=` are absent.

## 3. Results and guarantees

- `ArgumentCount() >= 0`, with the program name at index 0 when supplied.
- Get/Length pairs follow `platform.paths.v1`: exact copy, the length excluding
  the NUL, or −1 with no write when absent, null, non-positive or too small.
  Length equals what Get writes.
- Variable names are **case-sensitive on every provider**, including Windows,
  so portable code gets one answer. An empty value is present with length 0.
- `ProcessId()` is nonzero and stable. `GetDebuggerState()` may change.

## 4. Ownership, threading, ordering

Snapshot values are immutable; every method is `const` and safe from any thread.

## 5. Invariants and legal sequences

Repeated calls return the same strings.

## 6. Side effects and performance

None after the snapshot. Debugger state may cost a system call (reading
`/proc/self/status`, `IsDebuggerPresent`, `sysctl`).

## 7. Conformance suite and providers

- The shared predicate takes a fixture describing how the process was started
  (arguments, a present variable, an empty one, an absent one, a case variant)
  and checks every item exactly, its length, stability, exact-fit and one-short
  buffers, null and zero buffers, out-of-range indices and malformed names.
- `test_process_environment.cpp`: the fake with a full argv (including an empty
  argument and a UTF-8 value containing `=`), and with no arguments. **Positive;
  contract semantics only.**
- `test_process_environment_negative.cpp` (`sensitivity`): a dropped program
  name, partial writes, case-insensitive lookup, an empty value reported absent,
  a name with `=` accepted, a length counting the NUL, a clamped index and a
  zero pid are each caught.

### Native providers (installed 2026-10-08, R26)

- POSIX (`platform/posix/foundation_providers.h`, library `platform_posix`):
  `CreatePosixProcessEnvironment(argc, argv)` (environ snapshot; TracerPid on Linux, P_TRACED on Apple). Row `platform.foundation.posix` runs this suite and the native
  clauses on Linux with g++ and clang++, in release, under TSan and ASan/UBSan
  (`.tsan`, `.asan`) and as i386 (`.i386`). The same source cross-builds for
  Android arm64-v8a and x86_64 at API 29 (`tools/quality/android_foundation.py`)
  and compiles for iOS arm64.
- Win32 (`platform/win32/foundation_providers.h`): `CreateWin32ProcessEnvironment` (CommandLineToArgvW and the environment block as UTF-8, case-sensitive names). Row
  `platform.foundation.win32` runs as a static PE under Wine
  (`tools/quality/parity_wine.py check --suite platform.foundation.win32`).
- Evidence and what is still unverified: `RFC/0001-foundation-providers-progress.md`.
