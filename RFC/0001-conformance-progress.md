# RFC 0001 capability conformance progress (Q-FOUNDATION)

Updated: 2026-09-22

Bounded scope: shared conformance suites for the RFC 0001 capability contracts,
wired into the shared runner (`tools/quality/conformance.py`,
`quality/conformance.manifest.json`) so every capability provider is gated the
same way as the other RFC domains. This record tracks the conformance harness for
RFC 0001; contract *implementation* (native providers, loader retirement) is
tracked in `0001-phase-a-progress.md` / `0001-phase-b-progress.md`.

This is deliberately built in parallel with the providers: the suite defines the
behavior a provider must satisfy, so a native Win32/POSIX backend has a green
target to implement against and adds one manifest row when it lands.

## Delivered

| Capability | Contract | Suites (Q-FOUNDATION, rfc 0001) | State |
| --- | --- | --- | --- |
| Dynamic library (RFC 0001 reference migration) | `public/platform/contracts/dynamic_library.h` | `platform.dynamic_library` (+ `.sensitivity`) | test backend green |
| Monotonic clock | `public/platform/contracts/clock.h` | `platform.clock` (+ `.sensitivity`) | test backend green |
| Platform paths | `public/platform/contracts/paths.h` | `platform.paths` (+ `.sensitivity`) | test backend green |

Each capability ships: a pure-abstract contract header (no tier0, no native SDK),
a single shared conformance predicate, a deterministic test backend (positive
subject), a sensitivity suite that feeds the same predicate deliberately-broken
providers and asserts each is caught (proving the suite is not vacuous), a
contract doc, and two manifest rows. See `unittests/platformtest/README.md`.

RFC 0001 clauses currently exercised:
- dynamic library: valid load + single ownership, known-symbol resolution
  (exact address when known), missing-library and missing-symbol **structured**
  failures (operation + status + echoed request), null-argument rejection,
  explicit optional-capability (no-load) reporting, unload releases the handle,
  clean second load/unload in one process, and end-state emptiness (loader never
  destroyed with live handles).
- monotonic clock: positive resolution, never-backward samples, zero self-elapsed,
  monotonic-in-end elapsed, and additive conversion across an intermediate point.
- platform paths: per-location availability reported explicitly, normalized
  UTF-8 `/`-separated paths (no `\`, no `//`, no trailing `/`), stable results,
  and safe buffer handling (null/zero-size/one-byte -> -1, no overflow).

## Verification

```sh
python3 tools/quality/conformance.py check --domain Q-FOUNDATION --out -
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
```

At this revision: 6 Q-FOUNDATION suites pass under g++ and clang++ on
`linux-headless-core`; the architecture baseline/inventory remain current (the new
`public/platform/` and `unittests/platformtest/` trees are archlint-strict from
first commit and carry no native includes).

## Not claimed here

- **No native evidence.** The test backends certify contract *semantics* only.
  Fake-provider success is not evidence of OS loader/timer behavior (RFC 0001,
  "Backend conformance tests"). Native Win32/POSIX providers must run the same
  shared suites (with a real fixture shared library for dynamic-library loading)
  before any platform support claim.
- The real native loader (`Sys_LoadModule`/`Sys_GetFactory` in
  `tier1/interface.{h,cpp}`) is not yet migrated behind `IDynamicLibraryLoader`;
  that migration is the RFC 0001 reference migration and is separate work.

## Next capabilities (same pattern)

Threading, virtual memory, process environment, and diagnostics — each is a
self-contained contract + shared suite + test backend + sensitivity suite.
Render-backend conformance (`render.backend.*`) is being built separately by
another session.
