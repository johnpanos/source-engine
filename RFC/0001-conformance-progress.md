# RFC 0001 capability conformance progress (Q-FOUNDATION)

Updated: 2026-09-25

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
| Composition kernel (rank 3 / R06) | `public/platform/composition.h`, `platform.composition.v1.md` | `platform.composition`, `platform.test-runner` | test providers green |
| Tool process (Phase E) | `public/platform/contracts/tool_process.h` | `platform.tool_process` (+ `.sensitivity`) | test backend green |
| Gyro aim policy (Android input) | `inputsystem/gyro_math.cpp` | `input.gyro` (+ `.sensitivity`) | pure policy green |
| Single-vibrator rumble policy (Android input) | `inputsystem/vibrator_policy.cpp` | `input.vibrator` (+ `.sensitivity`) | pure policy green |

The last four rows were added after the first three (2026-09-22 to
2026-09-24). The composition and tool-process rows follow the contract pattern
below. The two input rows test pure policy code; they have no contract header
and no provider. Their record is the
[Android profile README](../quality/product_profiles/README.md).

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

Update (2026-09-25): the manifest now declares 21 Q-FOUNDATION suites. The
latest local full-domain evidence,
`quality-results/conformance.20260923T152310Z.json` (g++, dirty tree over
`c289e206`), matched all 17 suites then declared, with 1,303 checks. That
includes `platform.composition` (252 checks), `platform.test-runner` (273) and
`platform.tool_process` (49). The four `input.*` suites were added later. The
Android profile README records `input.gyro` at 68 checks and `input.vibrator`
at 43. The architecture check and loader inventory are no longer current
tree-wide; see the [Phase A record](0001-phase-a-progress.md). No finding is in
`public/platform/` or `unittests/platformtest/`.

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
Render-backend conformance is delivered separately in Q-PRESENTATION:
`render.backend.null` and `render.presentation.headless`, each with a
sensitivity row, plus `render.profile` (R15 and R16; see the
[render seam](0001-render-seam-progress.md) and
[presentation bridge](0001-presentation-bridge-progress.md) records).

Window and input contracts (`public/platform/window/`, `platform/window/`, and
an SDL2 window system under `platform/sdl2/window_system/`) have fake and SDL2
tests in `unittests/platformtest/window*`. No Waf target or manifest row builds
or runs them yet, so they are not evidence for R14.
