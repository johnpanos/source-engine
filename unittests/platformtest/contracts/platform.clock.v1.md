# Contract: `platform.clock.v1`

Module: `platform.contracts` · Types: `platform::IMonotonicClock`,
`platform::MonotonicTimestamp`
Header: `public/platform/contracts/clock.h`
Shared suite: `unittests/platformtest/clock/clock_conformance.h`
Conformance: `unittests/platformtest/clock/test_clock.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/clock/fake_clock.h`
RFC: 0001 (foundation capability "Monotonic clock") · Migration: `PLAT-CLOCK-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

A monotonic clock supplies elapsed-time measurement that never runs backward.
Portable code that needs durations depends on this contract rather than a native
timer or an OS check. A native provider is backed by the platform's monotonic
timer; the deterministic test provider is backed by virtual time so subsystem
tests are reproducible without sleeping (RFC 0001: clocks are injectable so tests
use virtual time).

Wall-clock / civil time is a **separate** capability and is deliberately excluded
here. All clauses are **required**.

## 2. Accepted inputs

- `Now()` takes no input.
- `ElapsedNanoseconds(begin, end)`: `begin` and `end` are timestamps produced by
  **this** clock, with `begin` produced no later than `end`. Mixing clocks or
  passing `end` before `begin` is a programmer error, not a defined result.

## 3. Results and guarantees

- `Now()` returns a `MonotonicTimestamp`. Successive calls are non-decreasing:
  within one clock, `ticks` is a non-decreasing counter (two calls closer than
  the resolution may be equal, but never decrease).
- `ResolutionNanoseconds()` returns the smallest distinguishable positive
  increment in nanoseconds and is strictly greater than zero.
- `ElapsedNanoseconds(begin, end)`:
  - `ElapsedNanoseconds(t, t) == 0`;
  - is non-negative for `begin <= end` (hence unsigned);
  - is monotonic in `end`: for `a <= b <= c`, `elapsed(a,b) <= elapsed(a,c)`;
  - is additive across an intermediate point: `elapsed(a,c) == elapsed(a,b) +
    elapsed(b,c)` (the conversion is linear in the tick delta).

The tick SCALE is opaque; only the producing clock turns a timestamp pair into
nanoseconds.

## 4. Ownership, threading, ordering

- `IMonotonicClock` owns no external resource. `MonotonicTimestamp` is a plain
  value that borrows nothing. `Now()` and `ElapsedNanoseconds` are `const`.
- The contract states no thread-safety guarantee; a provider that is safe to call
  concurrently documents that separately. Timestamps from different clock
  instances are not comparable or convertible.

## 5. Invariants and legal sequences

- Time never moves backward: for any sequence of `Now()` calls, each result is
  `>=` the previous.
- A stopped clock (equal successive timestamps) is degenerate but conforming; it
  is not a violation, so the suite does not reject it.

## 6. Side effects and performance

- No I/O, logging, or global mutation. `Now()`, `ResolutionNanoseconds()`, and
  `ElapsedNanoseconds()` are O(1). The test backend touches no real timer.

## 7. Conformance suite and providers

- `clock_conformance.h` is the single shared predicate: positive resolution,
  non-decreasing samples (never backward), zero self-elapsed, monotonic-in-end
  elapsed, and additivity across an intermediate point.
- `test_clock.cpp` runs it against the deterministic virtual-time backend
  (`CFakeMonotonicClock`) at two resolutions/steps, including an odd step to
  exercise additivity with non-power-of-two deltas. **Positive; certifies contract
  semantics, not native timer behavior.**
- `test_clock_negative.cpp` (`sensitivity`) feeds the same predicate three broken
  clocks (backward-time, zero-resolution, non-additive/clamped-elapsed) and
  asserts each is caught while the conforming backend passes.

### Native providers (installed 2026-10-08, R26)

- POSIX (`platform/posix/foundation_providers.h`, library `platform_posix`):
  `CreatePosixMonotonicClock` (CLOCK_MONOTONIC). Row `platform.foundation.posix` runs this suite and the native
  clauses on Linux with g++ and clang++, in release, under TSan and ASan/UBSan
  (`.tsan`, `.asan`) and as i386 (`.i386`). The same source cross-builds for
  Android arm64-v8a and x86_64 at API 29 (`tools/quality/android_foundation.py`)
  and compiles for iOS arm64.
- Win32 (`platform/win32/foundation_providers.h`): `CreateWin32MonotonicClock` (QueryPerformanceCounter, converted to nanoseconds once per sample so elapsed time stays exactly additive). Row
  `platform.foundation.win32` runs as a static PE under Wine
  (`tools/quality/parity_wine.py check --suite platform.foundation.win32`).
- Evidence and what is still unverified: `RFC/0001-foundation-providers-progress.md`.
