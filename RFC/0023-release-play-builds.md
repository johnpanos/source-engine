# RFC 0023: Release builds of `play`, `play_p2` and `play_p2_fsr`

- Status: proposed (user direction, 2026-10-06: "release builds ... should
  greatly reduce logging, counting, and tracing and build at an optimized
  level, ie. the fastest we can be"; "plan this out before doing it")
- Owners: build profiles (RFC 0001/0005), render core (RFC 0016), jobs (RFC 0003)
- Related: RFC 0005 (evidence, performance and promotion), RFC 0016 binding
  rules (hard render budgets, in-game pipeline performance, resolution sweep),
  RFC 0014 (debug controls), RFC 0006 (toolchain policy)

## Summary

Add a **release flavor** of the three desktop launchers — `./play_release`,
`./play_p2_release`, `./play_p2_fsr_release` (or `--release` on the existing
scripts; see decision D1) — each with its own build tree, Waf lock and staged
runtime. A release tree is built with the fastest optimization profile the
toolchain policy allows and compiles out, or leaves dormant, development-only
logging, counting, tracing and verification. The game image and gameplay are
unchanged: a release build is the same product with less instrumentation,
never a lower-quality one.

## Motivation

The current trees are tuned for development:

- `play_p2` configures `-T release`, which on gcc/clang means
  `-O2 -funsafe-math-optimizations -ftree-vectorize -ffast-math`, no LTO and
  no `-march` tuning (`scripts/waifulib/compiler_optimizations.py`). `./play`
  uses whatever `build/` was configured with.
- Engine and render code carry development instrumentation that runs in every
  frame: frame statistics streams (`-vkframestats`, `cl_render_debug_*`
  counters), census/ownership checks, shadow verifiers
  (`cl_bone_setup_verify`, within-frame geometry verifiers), VProf/telemetry
  zones, sequence checkers, spew to console and log files, and assertion
  paths that stay effective in release (RFC 0005 requires test assertions to
  stay effective; product hot paths are a separate question).
- RFC 0016's [in-game pipeline performance priority](0016-render-core.md#in-game-pipeline-performance-user-decision-2026-10-03)
  judges complete gameplay frames. Measuring them with development overhead
  mixed in blurs the real bottlenecks.

## Goals

1. G1 — **Fastest supported code generation** for the three products on the
   Linux desktop profile, chosen by measurement, not assumption.
2. G2 — **Instrumentation off by construction**: per-frame logging, counters,
   tracing and shadow verification are compiled out or reduced to one
   predictable branch; nothing allocates, formats strings or takes locks for
   diagnostics on the hot path.
3. G3 — **Same image, same gameplay**: release frames match the development
   tree's frames within run-to-run noise on the matched game cameras, and
   gameplay/physics captures match their recorded tolerances.
4. G4 — **Separate trees**: release never reconfigures or overwrites `build/`,
   `build-p2` or `build-p2-fsr` (user build profiles are preserved).
5. G5 — **Diagnosable**: errors, warnings that indicate wrong output, crash
   handling and symbolized crash diagnostics remain (symbols split out, not
   stripped away).

## Non-goals

- No change to shipped quality, effects, resolution or sample counts (RFC 0016
  rule 7). Release is not a "low" preset.
- No new packaging, signing or distribution (AGENTS.md platform rules).
  Mobile/Apple release profiles are follow-ups using the same switch.
- Not a replacement for the development trees; conformance suites keep running
  on them, plus a release lane (Phase R5).

## Design

### Build profile (one owner)

A new versioned product profile per launcher under `quality/product_profiles/`
(e.g. `portal-linux-native-vulkan-release.json`,
`portal2-linux-native-vulkan-release.json`, `...-fsr-release.json`) owns the
facts: build type, LTO, target ISA, defines, launch arguments. Launchers and
reports read the profile; they do not copy flags.

Waf gets a single configure option, `--product-flavor=release` (name to be
settled in R0), which:

- selects the optimization set below;
- defines `SOURCE_RELEASE_BUILD` (one macro; no ad-hoc per-module switches);
- keeps `NDEBUG`, and keeps C++20 dialect selection in
  `quality/toolchain/policy.json` (never target-local `-std`).

Candidate optimization set (each item kept only if R1 measures a gain and R2
shows identical images/gameplay):

| Item | Candidate | Risk / check |
| --- | --- | --- |
| Level | `-O3` vs current `-O2` | code size; measure |
| ISA | `-march=x86-64-v3` (portable AVX2) vs `-march=native` | `native` is host-only: allowed for a local profile, never for a distributed one |
| LTO | `--enable-lto` (`-flto=auto`), ThinLTO on clang | link time, static-composition `-r` interaction, visibility |
| PGO | `-fprofile-generate/-use` from the frame-floor route | phase R4 only; profile data versioned |
| Frame pointers | `-fomit-frame-pointer` | keep `-g` split DWARF for crash symbols |
| Semantic interposition | `-fno-semantic-interposition`, `-Wl,-O1,--as-needed`, `-Bsymbolic-functions` | shared-module ABI and loader telemetry |
| Floating point | **unchanged** | AGENTS.md: no tree-wide FP flag changes |

### Instrumentation policy

R0 inventories every per-frame diagnostic and classifies it:

| Class | Release behavior | Examples (to be confirmed in R0) |
| --- | --- | --- |
| Correctness verifier / shadow oracle | compiled out | `cl_bone_setup_verify`, geometry-reuse verifier, sequence checkers (`PLATFORM_CHECK_SEQUENCE` is already diagnostic-only) |
| Counters and frame statistics | compiled out, or gated by one cold branch when the user enables them | `-vkframestats`, `cl_render_debug_stats`, cross-thread census, K5 submission metrics |
| GPU timers | off by default, still available on request | `cl_render_debug_gpu_timers` (RFC 0014 D4) |
| Tracing / profiling zones | compiled out | VProf (`VPROF_LEVEL` already 0 by default), telemetry `tmZone`, debug-utils labels |
| Logging | developer/verbose spew compiled out; `Warning`/`Error` kept; console log file off unless requested | `DevMsg`/`ConDMsg`, per-frame `Msg` |
| Validation | off unless requested | `-vkvalidate` already opt-in |

Mechanism: one header (e.g. `public/tier0/release_build.h`) with
`SOURCE_RELEASE_BUILD` and small macros (`DIAG_COUNTER`, `DIAG_ONLY(...)`).
Frozen legacy paths (RFC 0016 binding rules) take such edits only as core
plumbing with a `Frozen-path:` line; frozen ABI headers are not changed.
Convars stay registered so configs and RFC 0014 controls keep parsing; in
release a compiled-out control reports "unavailable in release" by name rather
than silently doing nothing.

### Launch arguments

Release launchers pass the same quality arguments as the development
launchers (`JOB_ARGS`, `MAT_ARGS`, `QUEUE_ARGS`, physics, FSR) taken from the
profile, plus `-nodev`-style flags only where they remove diagnostics, never
where they change the image.

### Trees

| Launcher | Build dir | Waf lock | Runtime |
| --- | --- | --- | --- |
| `play` release | `build-release` | `.lock-waf-build-release` | via `run.sh` with `BUILD_DIR` |
| `play_p2` release | `build-p2-release` | `.lock-waf-p2-release` | `run/runtime-p2-release` |
| `play_p2_fsr` release | `build-p2-fsr-release` | `.lock-waf-p2-fsr-release` | `run/runtime-p2-fsr-release` |

`play_p2_fsr` already parameterizes `P2_BUILD_DIR`/`P2_RUNTIME`/`P2_WAFLOCK`;
the release launchers follow that pattern and add `P2_FLAVOR=release`.

## Phases and acceptance

| Phase | Work | Done looks like |
| --- | --- | --- |
| R0 | Inventory per-frame logging/counters/tracing/verifiers in engine, client, render core, native backend, jobs; classify; baseline cost | Checked-in inventory with owner and class per item; `perf` share of each in a gameplay route |
| R1 | Waf flavor, profiles, three trees and launchers; compiler set only (no source changes) | Trees build; interleaved A/B (frame-floor route, `portal-frame-pacing-v1`) per candidate flag, full resolution sweep 1024×768→4K; each kept flag shows a gain |
| R2 | Image/gameplay equivalence | Matched game cameras within run-to-run noise vs the dev tree; physics parity and host-frame captures within recorded tolerances; FSR on and off (strict coverage rule) |
| R3 | `SOURCE_RELEASE_BUILD` instrumentation removal, by class | Each class removed with a measurement; R2 rerun; diagnostics still available in dev trees |
| R4 | Optional PGO/BOLT | Only if R1–R3 leave a measured CPU-bound gap |
| R5 | Release conformance lane | Selected suites + boots run on release trees; a seeded instrumentation leak (counter left on) is detected |

Performance claims follow RFC 0005 and RFC 0016: interleaved runs, every
resolution reported, CPU/GPU limits identified per resolution. A release build
does not by itself close `linux-desktop-high-120` or any render gate.

## Decisions to settle in R0 (recommended defaults)

- D1: separate `*_release` scripts (recommended: clearer, no flag parsing in
  three places) vs a `--release` argument.
- D2: `-march=native` for the local release profile, `x86-64-v3` declared as
  the portable variant (recommended).
- D3: LTO on by default for release if static-composition and loader checks
  pass (recommended).
- D4: compiled-out convars report "unavailable in release" (recommended) vs
  silently ignored.

## Risks

- `-ffast-math` is already on; LTO can expose ODR/UB in legacy code
  (P1/P2 header ODR hazards are known). R2 is the guard.
- Removing verifiers hides regressions; dev trees and CI keep them.
- Separate trees cost disk and build time; ccache is shared via
  `launcher_ccache.sh`.

## Progress

### R1 first slice: release flavor and launchers (2026-10-06, in progress)

Installed:

- `--product-flavor=release` and `--release-march` (default `native`) in
  `scripts/waifulib/compiler_optimizations.py`. Release appends
  `-O3 -fno-semantic-interposition -fno-lifetime-dse` and `-Wl,-O1`, replaces the dev trees'
  x86-64 `-march=core2` with `-march=<release-march>`, and defines
  `SOURCE_RELEASE_BUILD=1`. `--enable-lto` now uses `-flto=auto` on gcc.
  Dev trees are unchanged (no option given → `dev`).
- `./play_release` (`build-release`, `run/runtime-release`; `./play` takes
  `PLAY_BUILD_DIR`), `./play_p2_release` (`build-p2-release`) and
  `./play_p2_fsr_release` (`build-p2-fsr-release`). `./play_p2` takes
  `P2_FLAVOR=dev|release` and refuses a tree of the other flavor when it is
  set; unset accepts the tree's own flavor (harnesses pass only `P2_BUILD_DIR`).
- Decisions D1–D4 taken as recommended (user, 2026-10-06: "get started").

Not yet: the per-product profile JSON owning these flags (the launchers carry
them for now), the A/B measurements that must justify each flag, and R2.

### R0 findings so far (2026-10-06)

| Item | State in current trees | Release action |
| --- | --- | --- |
| VProf (`VPROF_*`) | `VPROF_ENABLED` is never defined: already compiled out | none |
| RAD Telemetry (`tmZone`) | dummy macros: already compiled out | none |
| Vulkan debug labels / object names | loaded only with validation (`-vkvalidate`) | none |
| `-vkframestats`, `cl_render_debug_stats`, GPU timers | opt-in | keep opt-in; measure the off-path cost |
| Shadow verifiers (`cl_bone_setup_verify`, indirect-light, dynamic-occlusion `_verify`) | convar, default off | measure; compile out if the branch shows |
| Native backend route census | **every draw**: `std::map<std::string>` lookup plus a per-snapshot `std::string` copy (`shaderapivulkan.cpp`, frozen path) | candidate for R3 (`Frozen-path:` user request) |
| Unimplemented-entry and cross-thread ownership census | per call, pointer scan / relaxed atomic | measure |
| Draw-state fixture / view oracle | one `Enabled()` branch per draw | likely none |

### LTO exposes ODR violations in the Portal 2 game modules (2026-10-06)

`build-p2-release` builds (3,110 steps, 5 min 26 s on 32 threads), but gcc's
LTO reports 28 `-Wodr` diagnostics. Most are `CPortal_Player`/`C_Portal_Player`
defined with different bases in different translation units (Portal 1 and
Portal 2 headers in one module), plus `CPortalRender`, `g_pPortalRender`,
`C_BaseHLPlayer`, `LadderMove_t`, `BeamInfo_t`, `ZIPENTRY` and `CLZMAStream`.
The violations exist in dev trees too, where nothing reports them; under LTO
they can change devirtualization and alias analysis. Release keeps LTO
pending R2, and R2's image and gameplay comparison on Portal 2 is the gate.
If R2 shows a divergence, the game modules build without LTO until the ODR
violations are fixed. Warnings are not silenced.

### First release boots crashed: lifetime DSE and a header case collision (2026-10-06)

The first `build-p2-release` crashed after the map loaded, three times:

1. `C_FuncTrackTrain::GetSoundSpatialization` indexed `vecDir` with a garbage
   `m_nLongAxis`;
2. then `C_Prop_Portal::OnActiveStateChanged` wrote through a garbage
   `TransformedLighting.m_pEntityLight`.

Cause: `C_BaseEntity::operator new` zeroes the object with `memset`, and the
entity code relies on those zeroes. With LTO, gcc sees the allocator and
deletes that memset as a dead store before the object's lifetime starts
(`-flifetime-dse`, as in the TF2 bring-up's `CHudElement`). The release flavor
now passes `-fno-lifetime-dse`. Dev trees don't hit it today because nothing
inlines `operator new` across files there.

Fixed on the way, in every flavor:

- `C_FuncTrackTrain` initializes its spatialization members.
- Portal 1's `game/client/portal/PortalRender.h` forwards to Portal 2's
  `portalrender.h` in Portal 2 builds. Eight Portal 2 client files
  (`viewrender.cpp`, `viewdebug.cpp`, `cdll_client_int.cpp`,
  `entity_client_tools.cpp` and four Portal 1 sources) spelled
  `"PortalRender.h"`. On Linux that resolved to Portal 1's `CPortalRender`,
  which has a different layout. This was a live ODR violation in dev builds
  too. Portal 2's header gains the `CRenderStartSteps` friend that Portal 1's
  already had.

The other `-Wodr` reports remain: `C_Portal_Player` and
`PreDataChanged_Backup_t` (Portal 1 and Portal 2 `c_portal_player.h`),
`C_BaseHLPlayer`, `C_HL2PlayerLocalData`, `LadderMove_t`, `BeamInfo_t`, the
server's `CPortal_Player`, `ZIPENTRY` and `CLZMAStream`. They are open R2
items.

### First A/B: Portal 2 dev vs release (2026-10-06)

Method: `frame_floor.py --no-stop` with a private copy of
`portal2-frame-floor-v1` pointed at retail `sp_a1_intro4`, because the
published `sp_a1_intro4_probe64` is no longer in `run/maps`. Settings: High
pinned by the `linux-desktop-high-120` row, 1920x1080 in a private compositor,
three interleaved pairs, and the same source revision for `build-p2` and
`build-p2-release`.

| Run | median ms | p99 ms | 1% low fps | cpu_p99 ms | gpu_render_p99 ms |
| --- | --- | --- | --- | --- | --- |
| dev1 | 57.71 | 95.12 | 10.3 | 32.46 | 94.34 |
| rel1 | 56.12 | 87.46 | 11.2 | 26.08 | 85.26 |
| dev2 | 54.79 | 86.32 | 11.4 | 31.31 | 82.08 |
| rel2 | 56.48 | 88.81 | 10.7 | 26.04 | 86.56 |
| dev3 | 56.71 | 90.87 | 10.9 | 32.84 | 87.22 |
| rel3 | 54.87 | 85.32 | 11.3 | 25.67 | 83.39 |

Release cuts CPU p99 by about 19% (31.3–32.8 → 25.7–26.1 ms) in every pair.
Present-to-present frame time does not change: this route is GPU-bound at
about 55 ms on this host, with the GPU shared with other sessions. The
CPU-side gain only shows up in frame time where the CPU is the limit. Flags
are not yet A/B'd one at a time, and the 1024x768→4K resolution sweep is not
done. This is no frame-floor evidence (`map.loaded` fails on the substituted
map in both flavors).

### All three release trees build (2026-10-06)

- `build-release` (Portal 1): builds in 3 min 9 s and passes
  `portal_boot.py --headless --renderer native-vulkan --map testchmb_a_01`.
  `play_release` configures with `--no-lock-in-run --no-lock-in-top`, so the
  Waf lock lives only in the output tree, as for `build/`. Without that,
  `run.sh`'s in-tree `waf build` built `build/` instead.
- `build-p2-release`: route above.
- `build-p2-fsr-release`: builds. Not booted yet: `frame_floor.py` doesn't
  set `P2_FSR=1`, so `play_p2` refuses an FSR tree from that harness.
