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

### Native providers (added as they land)

A real monotonic-clock provider (POSIX `clock_gettime(CLOCK_MONOTONIC)`, Win32
`QueryPerformanceCounter`) adds one manifest row running the same shared suite to
certify OS timer behavior; fake-provider success is not evidence of it.
