# Contract: `platform.wall-clock.v1`

Module: `platform.contracts` · Types: `platform::IWallClock`, `platform::WallTime`,
`platform::CivilTime`, `platform::CivilZone`
Header: `public/platform/contracts/wall_clock.h`
Shared suite: `unittests/platformtest/wall_clock/wall_clock_conformance.h`
Conformance: `unittests/platformtest/wall_clock/test_wall_clock.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/wall_clock/fake_wall_clock.h`
RFC: 0001 (foundation capability "Wall clock") · Migration: `PLAT-WALLCLOCK-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Civil/UTC time for things that show or record a date: save-game stamps, logs,
demo metadata. It can jump, so it never measures durations
(`platform.clock.v1` does). `Now()` and UTC breakdown are **required**; local
breakdown is **optional** (a provider without zone data refuses it).

## 2. Accepted inputs

- `ToCivil(time, zone, out)`: any `WallTime`. The representable range of
  signed 64-bit nanoseconds is 1677-09-21T00:12:43.145224192Z to
  2262-04-11T23:47:16.854775807Z.

## 3. Results and guarantees

- `WallTime` is nanoseconds since 1970-01-01T00:00:00Z without leap seconds.
- `ToCivil(kUtc)` always succeeds, proleptic Gregorian, floor semantics for
  instants before the epoch, `utcOffsetSeconds == 0`.
- `ToCivil(kLocal)` either succeeds with fields that denote the same instant
  through `utcOffsetSeconds` (within ±14 h), or returns false and leaves `out`
  unchanged.
- Every successful breakdown has fields in range (month 1–12, day valid for the
  month, second 0–59, nanosecond 0–999,999,999).

## 4. Ownership, threading, ordering

No owned resources; all methods are `const`. No thread-safety guarantee beyond
what a provider documents. `Now()` may decrease between calls.

## 5. Invariants and legal sequences

Breakdown is a pure function of the instant, the zone and the provider's zone
data at that instant.

## 6. Side effects and performance

No I/O, logging or global mutation; a native provider may read zone data once
at start.

## 7. Conformance suite and providers

- The shared predicate checks fixed UTC vectors (epoch, leap day 2000-02-29,
  one nanosecond before the epoch, the 2100 non-leap boundary, both ends of the
  range), `Now()`'s breakdown, local breakdowns around a year boundary and
  before the epoch, and a UTC sweep over the whole range. Its oracle is
  `days_from_civil`, the inverse of the fake's algorithm, compared in seconds
  so the range ends cannot overflow.
- `test_wall_clock.cpp`: the fake at UTC+2, UTC−9:30 with a pre-epoch `Now()`,
  and without a local zone. **Positive; certifies contract semantics only.**
- `test_wall_clock_negative.cpp` (`sensitivity`): zero-based months, Julian leap
  years, a flipped local offset, truncation toward zero before the epoch,
  dropped nanoseconds and a refusal that clobbers `out` are each caught.

### Native providers (installed 2026-10-08, R26)

- POSIX (`platform/posix/foundation_providers.h`, library `platform_posix`):
  `CreatePosixWallClock` (CLOCK_REALTIME; UTC by the calendar algorithm, local through `localtime_r`, refused outside `time_t`). Row `platform.foundation.posix` runs this suite and the native
  clauses on Linux with g++ and clang++, in release, under TSan and ASan/UBSan
  (`.tsan`, `.asan`) and as i386 (`.i386`). The same source cross-builds for
  Android arm64-v8a and x86_64 at API 29 (`tools/quality/android_foundation.py`)
  and compiles for iOS arm64.
- Win32 (`platform/win32/foundation_providers.h`): `CreateWin32WallClock` (GetSystemTimePreciseAsFileTime; local through the zone rules for that year, refused before 1601). Row
  `platform.foundation.win32` runs as a static PE under Wine
  (`tools/quality/parity_wine.py check --suite platform.foundation.win32`).
- Evidence and what is still unverified: `RFC/0001-foundation-providers-progress.md`.
