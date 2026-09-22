# Platform capability conformance (RFC 0001, Q-FOUNDATION)

Shared conformance suites for the RFC 0001 capability contracts under
`public/platform/contracts/`. Each capability has ONE shared predicate that every
provider claiming the contract must pass — the deterministic test backend here,
and the real Win32/POSIX backends as they land.

These run under the shared runner and are gated with every other domain:

```sh
python3 tools/quality/conformance.py check --domain Q-FOUNDATION
python3 tools/quality/conformance.py check --suite platform.dynamic_library
```

They build under the `linux-headless-core` profile: C++20, include root `public`
only, `-Wall -Wextra -Werror`, no tier0/tier1, no GPU, no display.

## Layout

| Capability | Contract | Shared suite | Test backend | Suites |
| --- | --- | --- | --- | --- |
| Dynamic library | `public/platform/contracts/dynamic_library.h` | `dynamic_library/dynamic_library_conformance.h` | `dynamic_library/fake_dynamic_library.h` | `platform.dynamic_library`, `.sensitivity` |
| Monotonic clock | `public/platform/contracts/clock.h` | `clock/clock_conformance.h` | `clock/fake_clock.h` | `platform.clock`, `.sensitivity` |
| Platform paths | `public/platform/contracts/paths.h` | `paths/paths_conformance.h` | `paths/fake_paths.h` | `platform.paths`, `.sensitivity` |

Contract docs: `unittests/platformtest/contracts/platform.<cap>.v1.md`.

## The pattern (add a new capability the same way)

1. **Contract** (`public/platform/contracts/<cap>.h`): pure abstract interface +
   structured error/value types. Standard library only — no native SDK, no
   OS-selection macros (the dir is archlint-strict from first commit).
2. **Shared predicate** (`<cap>/<cap>_conformance.h`): a `Run…Conformance(iface&,
   fixture)` that drives the interface through every RFC clause and returns a
   report of `checks`/`failures` with a first-divergence line. Provider-agnostic.
3. **Test backend** (`<cap>/fake_<cap>.h`): a deterministic conforming provider
   (in-memory, virtual time). It is the positive subject.
4. **Positive suite** (`<cap>/test_<cap>.cpp`): run the predicate against the fake
   in the configurations that matter; exit non-zero on any failure.
5. **Sensitivity suite** (`<cap>/test_<cap>_negative.cpp`): feed the SAME predicate
   deliberately-broken providers (one clause each) and assert every one is caught
   while the conforming backend passes. Proves the suite is not vacuous.
6. **Contract doc** + two **manifest rows** (`positive`, `sensitivity`) in
   `quality/conformance.manifest.json`, `domain: Q-FOUNDATION`, `rfc: 0001`.

## Native providers

Fake-provider success certifies **contract semantics only** — it is not evidence
of OS behavior (RFC 0001, "Backend conformance tests"). When a real Win32/POSIX
provider lands, it adds one manifest row that compiles the same shared predicate
against the real provider (plus a real fixture library for dynamic-library), so
Windows and POSIX pass the identical suite. Keep native SDK includes inside the
provider target, never in the contract or shared-suite headers.
