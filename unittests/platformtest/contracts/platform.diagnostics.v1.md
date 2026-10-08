# Contract: `platform.diagnostics.v1`

Module: `platform.contracts` · Types: `platform::IDebugOutput`,
`platform::ICrashReporter`, `DiagnosticSeverity`, `CrashReportResult`
Header: `public/platform/contracts/diagnostics.h`
Shared suite: `unittests/platformtest/diagnostics/diagnostics_conformance.h`
Conformance: `unittests/platformtest/diagnostics/test_diagnostics.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/diagnostics/fake_diagnostics.h`
RFC: 0001 (foundation capability "Diagnostics") · Migration: `PLAT-DIAG-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

`IDebugOutput` is the native sink a logging owner writes through (stderr,
`OutputDebugString`, `os_log`, logcat). It applies no filtering and is not a
logging framework; it is **required**. `ICrashReporter` is **optional**: a
provider without crash capture returns false from `IsAvailable()` and refuses
every call. Installing the native crash handler is the provider's start step,
owned by the composition root; consumers only annotate and request reports.

## 2. Accepted inputs

- `Write`: any severity and UTF-8 message; null is ignored.
- Annotation keys: 1–64 bytes of `[A-Za-z0-9_.-]`. Values: UTF-8, at most 1024
  bytes; null removes the key. At most `kCrashMaxAnnotations` (32) keys are held
  at once, so a crash handler reads them from fixed storage (amended
  2026-10-08, R26); one more is `kInvalidArgument`.
- `WriteReport`: a non-null reason and an id buffer large enough for the id.

## 3. Results and guarantees

- Each `Write` arrives whole, byte for byte (embedded newlines kept) with its
  severity, and per thread in call order. A text sink frames each message with
  a severity tag (`[info] `, `[warn] `, `[error] `) and a line terminator; a
  reader that removes the framing recovers message and severity exactly
  (amended 2026-10-08, R26).
- A record sink whose channel has no severity (the Windows debugger channel)
  delivers each message as one unframed record and drops the severity; it says
  so where it is declared (amended 2026-10-08, R103, so Tier 0's
  `Plat_DebugString` output stays byte-identical). Logcat carries severity as
  its priority.
- Annotations set, replace, remove (removing an absent key is `kOk`) and read
  back by the paths convention. A refused set changes nothing.
- `WriteReport` captures the process without terminating it and returns a
  non-empty id unique per report. Anything but `kOk` leaves the id buffer
  unchanged. Annotations apply to every later report, including a crash.
- Unavailable: `kUnsupported` or −1 everywhere, no change.

## 4. Ownership, threading, ordering

`Write` is safe from any thread. The crash reporter's calls are serialized by
the caller, except that a crash may occur at any time; a provider keeps the
annotation store readable from its crash handler.

## 5. Invariants and legal sequences

The annotations attached to a report are those set before `WriteReport`, or
before the crash.

## 6. Side effects and performance

`Write` performs native I/O. `WriteReport` writes a report file to a
platform-approved location.

## 7. Conformance suite and providers

- The debug-output predicate reads messages back through a test-supplied
  `IDebugOutputCapture`: severities, an empty message, newlines, UTF-8, an
  8 KiB message, ignored nulls, and two concurrent writers whose messages must
  arrive whole and in per-thread order.
- The crash-reporter predicate runs the refusal branch when unavailable;
  otherwise it covers set/replace/remove, tight buffers, key and value limits,
  malformed keys, an oversized value, unique report ids and refused reports.
- `test_diagnostics.cpp`: the fake output, and the fake reporter available and
  unavailable. **Positive; contract semantics only.**
- `test_diagnostics_negative.cpp` (`sensitivity`): an appended newline, split
  lines, truncation, dropped severity, a write delivered in two halves, an
  unavailable reporter accepting annotations, malformed keys accepted, an
  oversized value truncated, null removal refused, a reused report id and no
  annotation capacity are each caught.

### Native providers (installed 2026-10-08, R26)

- POSIX (`platform/posix/foundation_providers.h`, library `platform_posix`):
  `CreateFdDebugOutput`, `CreateAndroidLogDebugOutput`, `CreatePosixCrashReporter` (text reports on demand and from a fatal-signal handler on an alternate stack) and `CreateUnavailableCrashReporter`. Row `platform.foundation.posix` runs this suite and the native
  clauses on Linux with g++ and clang++, in release, under TSan and ASan/UBSan
  (`.tsan`, `.asan`) and as i386 (`.i386`). The same source cross-builds for
  Android arm64-v8a and x86_64 at API 29 (`tools/quality/android_foundation.py`)
  and compiles for iOS arm64.
- Win32 (`platform/win32/foundation_providers.h`): `CreateWin32HandleDebugOutput`, `CreateWin32DebuggerOutput`, `CreateWin32CrashReporter` (unhandled-exception filter). Row
  `platform.foundation.win32` runs as a static PE under Wine
  (`tools/quality/parity_wine.py check --suite platform.foundation.win32`).
- Evidence and what is still unverified: `RFC/0001-foundation-providers-progress.md`.
