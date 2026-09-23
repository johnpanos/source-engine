# RFC 0005 progress: Quality and correctness harnesses

Updated: 2026-09-22
Source revision at assessment: `95798d15` (working tree; AGENTS.md portfolio row: R02)

This file is the human-readable, durable gate-decision record for RFC 0005. The
machine facts live in the versioned artifacts (`quality/conformance.manifest.json`,
`quality/profiles/`) and the generated evidence under `quality-results/`; those
artifacts are authoritative when they disagree with prose here.

RFC 0005 is Proposed. This record opens the shared-infrastructure increments. It
marks **no** domain gate complete: installing a runner does not certify any
domain's correctness, and this document makes no such claim.

## Increment status (RFC 0005 §"Delivery increments")

| Increment | Deliverable | State | Evidence |
| --- | --- | --- | --- |
| Q0 | Baseline profiles and current-check audit | **done (R01, Linux x86_64 host)** | [Q0 section below](#q0--r01-baseline-and-profile-inventory); `quality/baseline.json` |
| Q1 | Shared runner, fixtures, result schema | **done (R02)** | [Q1 section below](#q1--r02-runner-shared-conformance-runner); `tools/quality/conformance.py` |
| Q2 | Q-ARCH and Q-FOUNDATION reference suites | planned | `tools/archlint`, `tools/stylelint` (R04-STYLE only) |
| Q3 | Domain oracles | partial | Q-EDITOR + Q-JOBS suites registered (below) |
| Q4 | Content and native integration | planned | — |
| Q5 | Promotion and maintenance | planned | — |

## Q0 — R01: baseline and profile inventory

`done` (2026-09-22) for the R01 scope: current checks and their failures are
recorded; build, content and tool availability and the support matrix are
established; baseline captures and budgets are identified per domain. Observed at
source revision `2b2ee370` plus a dirty tree (107 files, digest
`8e0a762a…`, recorded in the evidence) on one host: Fedora 44, Linux 7.2.5,
x86_64, 32 threads, 125 GiB, AMD Radeon 8060S (RADV, Mesa 26.2.2), Python 3.14.7.

### What is installed

- **Declaration:** [`quality/baseline.json`](../quality/baseline.json)
  (`source-baseline/v1`). It declares 37 tool probes, 6 content corpora,
  17 profiles in the support matrix, 33 checks with their recorded outcomes, and
  17 baseline and budget entries. It links profile files and doesn't copy their facts.
- **Audit:** [`tools/quality/baseline.py`](../tools/quality/baseline.py)
  `validate | probe | audit`. The audit writes `baseline-evidence/v1` and
  per-check logs. It fails on any deviation from the recorded outcome, including
  a known failure that now passes. Missing requirements report as `unavailable`,
  which never counts as a pass; `--strict` turns them into failures.
- **Self-tests:** [`test_baseline.py`](../tools/quality/tests/test_baseline.py)
  has 21 tests: malformed declarations, dangling references, missing records,
  regression, stale known failure, crash/signal and timeout classification,
  unavailable versus `--strict`, and empty or unmatched selection. Three seeded
  mutants of the comparator and classifier were each caught.
- **Command:** `python3 tools/quality/baseline.py audit`. Evidence for this record:
  `quality-results/baseline/r01/evidence.json` (git-ignored, per run). All 32
  non-package checks matched their recorded outcome: 24 pass, 6 known fail,
  2 known crash, 0 deviations, 0 unavailable. The run took about 15 minutes.
  `package.android-arm64` passed separately: `build-android-apk.sh` plus the
  APK verifier (`quality-results/baseline/r01-package/`).

### Pre-existing failures (recorded, not fixed here)

| Check | Outcome | Owner | Finding |
| --- | --- | --- | --- |
| `arch.check`, `arch.baseline` | fail | R04 | ARCH105 `CreateInterfaceFn` drift: 7 new (in-flight `shaderapivulkan.cpp`, `vphysics_box3d`, `physicstest`), 3 stale, 0 relocated |
| `arch.inventory` | fail | R07 | 10 uninstrumented native loader sites: vendored `box3d/extern/sokol` (7), `box3d/samples` (2), `test_vphysics_conformance.cpp:379` (1) |
| `roadmap.check` | fail | R15/R16 | Hard-gate invariant: R15 `done` with R06 `planned`; R16 `done` with R14 `planned` (R16 records a partial-R14 note) |
| `build.dedicated`, `build.dedicated-clang` | fail | R12 | `dedicated/sys_linux.cpp:277` calls protected `CAppSystemGroup::LoadModule` |
| `legacy.unittest-legacy` (gcc) | crash | R20 | SIGSEGV in tier0 `TSListTests::PushThreadFunc` (CTSQueue multithread pop), 4 of 4 runs; this is the `scripts/tests-ubuntu-amd64.sh` command. R02 triage: a lock-free `CTSQueue` defect (NULL tail via an unvalidated help path and untagged `pNext` CAS), not a runner defect; see [Q1](#legacy-host-and-ctsqueue-triage) |
| `legacy.unittest-legacy-clang` | crash | R07 | `moduleloadtelemetrytest.cpp:289/291` cannot `dlopen("./libmoduleloadfixture.so")` from the installed layout, then crashes |

Passing at this revision: the archlint, stylelint, roadmap and quality
self-tests; `archlint hammer`; the conformance runner (93 suites, g++ and
clang++); the Wine PE parity run (60 suites); the DXVK profile dependency check;
incremental builds of the tests, tools, HL2 and Portal native/DXVK trees (gcc and
clang where configured); the toolchain boundary (10 trees, 18/18 probes); both
`unittest` hosts (258 checks); the GTK Hammer viewport smoke; the BSP2 corpus
(54/54 maps); and physics conformance (IVP reference plus Box3D, every seeded
fault detected).

Drift fixed while recording the baseline:

- `roadmap.py` rejected every State cell that carried a progress link, so 13
  valid rows were errors and the real hard-gate violations above were hidden.
  It now parses `state (links)` and verifies each linked record exists
  (3 new tests).
- The R39 row linked a non-existent `RFC/0001-phase-d-progress.md`. It now links
  the Phase B record section that tracks Phase D.

### Host availability

Available: g++ 16.2.1, clang++ 22.1.8, clang ASan/UBSan and TSan, MinGW g++
16.1.1, Wine 11.0 staging, GE-Proton 11-5, clang-format 22.1.8 (the pin),
glslc/SPIR-V tools, SDL3 3.4.16, SDL2, Vulkan 1.4.341, GTK 4.22.5,
libadwaita 1.9.3, a hardware Vulkan GPU, mutter (for isolated compositors),
the pinned DXVK Native 2.7.1, NDK r30 and SDK platform 36, Java, adb with one
authorized device, the Android emulator, and Blender 5.2.1 (Cycles).

Unavailable: **g++ sanitizer runtimes** (`libasan`/`libtsan` missing, so the
gcc sanitizer lanes can't run here), 32-bit multilib (`linux-i386`),
MSVC (native or msvc-wine; the legacy Hammer parity toolchain is provisioned
per session), Xcode and MoltenVK, OpenUSD tools, and KTX tools.

Content: the Portal runtime and its private staging, Portal 2, and chamber-sdk
(319 VMF, 28 BSP) are present. A standalone HL2 runtime is absent. The live probe
is `baseline.py probe`.

### Support matrix (what is established, not certified)

The authoritative list is `profiles` in `quality/baseline.json`. North-star Linux
x86_64 roles are built and checked here: the headless core, the Portal DXVK and
native-Vulkan clients, tools, and the test composition. The dedicated server is
declared but does not build. macOS, iOS device and iOS simulator have **no
profile file, runner or SDK**. Android has a profile, and its arm64 APK builds and verifies here. Device
lifecycle remains unverified. Preserved profiles
are Windows PE (the Wine parity run), legacy MFC Hammer parity (toolchain absent),
Windows MSVC client/dedicated, linux-i386, android-armv7a and FreeBSD. None of the
last four can run here or has a profile file. Missing profile files for the
dedicated server, tools, test composition and GTK Hammer are recorded as gaps with
their owning rows.

### Baseline captures and budgets identified

Per domain, from `baselines` in the declaration:

- **Recorded captures:** the loader ratchet, the toolchain invocations, scheduler
  microbenchmarks, the IVP physics oracle, the BSP2 corpus and the D3D9
  material-pixel references.
- **Partial captures:** the Hammer inventory, legacy Hammer parity, legacy job
  captures, the VMF corpus, DXVK frames and Portal startup.
- **Missing, with an owner:** every budget. That covers editor latency/memory
  (R25), frame/p95/p99/low-core/mobile power (R21), IVP memory/load/step
  measurements (R09), render frame/GPU/power (R32), and product
  startup/load/memory/package/power (R36).

No performance budget exists for any profile. R01 identifies them; the owning
rows must set absolute budgets before measurement.

### Non-claims

- R01 certifies no domain, platform or product gate. A check passing here is
  a recorded baseline, not acceptance.
- The evidence covers one Linux x86_64 host. macOS, iOS, Windows MSVC,
  linux-i386 and FreeBSD are recorded as unavailable, not verified.
- Native window/GPU product boots (`portal_boot.py`, SDL3 presentation) were not
  re-run. They require an isolated compositor and remain in their own records.
- Known failures are recorded for review by their owners. No baseline or
  inventory was regenerated to make a check green.
- The tree is shared by concurrent sessions. Re-run `baseline.py audit` before
  relying on these outcomes, since a deviation is the signal that the record
  needs review.

## Q1 — R02-RUNNER: shared conformance runner

`done` (2026-09-22) for the R02 scope. Zero or missing tests, skips, crashes,
timeouts and incomplete output fail correctly. Test composition is explicit.
Per-test evidence and logs are reproducible. All of this was observed at source
revision `2b2ee370` plus a dirty tree shared with concurrent sessions (each
run's digest is in its evidence). R02 certifies the **runner**, not any domain
gate; see the non-claims below.

### What is installed

- **Runner:** [`tools/quality/conformance.py`](../tools/quality/conformance.py)
  runs the pipeline `list` / `plan` / `check`. It enumerates the selected
  suites and their profiles and providers before execution, builds each suite
  with its profile's toolchain, and runs it in its own process session under a
  timeout and an address-space cap. It classifies the outcome, reconciles
  enumerated against executed ids, and writes `conformance-evidence/v2` before,
  during and after the run, so an interrupted run reads `incomplete`.
- **Result protocol `checks-v1`:**
  [`public/testing/conformance_result.h`](../public/testing/conformance_result.h)
  provides `testing::ReportConformance( checks, failures )`. The manifest loader
  requires the protocol on every suite that expects `pass`, and the runner
  requires exactly one record with `checks > 0` and zero failures. A missing
  record (early return or incomplete output), a duplicate record, zero checks
  or a failure count therefore fail even with exit status 0. All 82 such suites
  (Q-EDITOR, Q-FOUNDATION, Q-JOBS, Q-PRESENTATION) were converted. Each counts
  its evaluated checks, and each conversion was proven sensitive by flipping one
  check. `parity_wine.py` classifies Wine runs through the same
  `classify_run()`.
- **Authorities:**
  [`quality/conformance.manifest.json`](../quality/conformance.manifest.json)
  owns which suites exist, their sources, `expect`, `requires`/`optional` and
  `min_checks`. [`quality/profiles/`](../quality/profiles/) owns the toolchain,
  flags, timeout and memory cap.
- **Providers and skips:** `requires` names `executable:`, `env:` and `path:`
  providers.
  - A required suite with a missing provider fails as `unavailable-provider`.
  - An `optional` suite is `skipped` with its reason recorded, and is counted as
    skipped, never certified.
  - An unavailable compiler is fatal.
- **Bounds:** each suite gets the profile timeout and a `memory_limit_mb`
  address-space cap (sanitizer builds excepted), with no core files. A timeout
  kills the whole process group and keeps the output written so far.
- **Determinism and repeats:** `--seed` is exported as `CONFORMANCE_SEED`.
  `--repeat N` retains every attempt, and the suite matches only if all attempts
  match; there is no retry-until-green.
- **Optimized builds:** `--config release` adds `-O2 -DNDEBUG` after the suite
  flags. Suites whose `unittests/` sources call `assert()` are rejected as
  `invalid-oracle` before building.
- **Evidence identity:**
  - Source: revision, dirty digest and submodule revisions.
  - Toolchain and host: compiler path, version and target triple; host OS,
    architecture and Python version.
  - Run: exact invocation, selectors, config, seed and repeat, plus the manifest
    digest.
  - Per suite: an input digest of its declared sources and contract, the
    rebuild command, every attempt, and full build/run logs in
    `<evidence>.logs/`.
- **Self-tests:** [`test_conformance.py`](../tools/quality/tests/test_conformance.py)
  has 44 tests over the negative fixtures in
  [`tests/fixtures/cpp`](../tools/quality/tests/fixtures/cpp):
  - Process outcomes: pass, fail, crash, a hang, and a hang whose forked child
    must not survive.
  - Resource and record handling: a memory hog stopped by a 64 MiB cap (the
    same fixture passes at 2 GiB); missing, duplicate and zero-check records;
    `min_checks`.
  - Build and oracle: `assert()`-only suites, a counted failure in both
    configurations, compile errors and missing sources.
  - Providers and selection: required and optional unavailable providers, an
    unavailable compiler, zero discovery and partially unmatched selectors.
  - Runner behaviour: seed and attempt export, retained intermittent failures,
    interrupted runs, and evidence completeness.
- **Legacy hosts:**
  - `utils/unittest/unittest.cpp` (`unittest_legacy`) now fails on zero
    discovered test modules or tests, and on a `tests/*.so` that fails to load
    or export the unit-test interface. Previously an empty `tests/` directory
    exited 0.
  - The linked `unittest` host (`capability_runner.cpp`) already rejected zero,
    duplicate, skipped and zero-check suites.
  - `unittests/hammertest/run_headless.sh` now fails when a listed compiler is
    missing or none is listed, instead of passing with zero coverage.
  - `parity_wine.py` now fails on zero selection.
- **CI:** [`conformance.yml`](../.github/workflows/conformance.yml) runs the
  runner self-tests, then the full manifest over {g++, clang++} × {default,
  release} on `ubuntu-24.04`. It uploads evidence and logs even on failure.
  [`tests.yml`](../.github/workflows/tests.yml) legacy jobs now have a 60-minute
  bound, so a hung legacy test fails instead of occupying the runner.

### Installed commands (verified working here)

```sh
python3 -m unittest tools/quality/tests/test_conformance.py -v   # 44 tests
python3 tools/quality/conformance.py plan
python3 tools/quality/conformance.py check [--cxx clang++] [--config release]
python3 tools/quality/conformance.py check --suite <id> --repeat 5 --seed 7
unittests/hammertest/run_headless.sh                              # g++ and clang++, Q-EDITOR
python3 tools/quality/parity_wine.py check --cxx x86_64-w64-mingw32-g++ --wrapper wine
```

### Evidence (2026-09-22)

| Run | Toolchain | Result |
| --- | --- | --- |
| full manifest, default | g++ 16.2.1 | 93/93 matched, 186,452 checks |
| full manifest, default | clang++ 22.1.8 | 93/93 matched, 187,849 checks |
| full manifest, release (`-O2 -DNDEBUG`) | g++ 16.2.1 | 93/93 matched |
| full manifest, release | clang++ 22.1.8 | 93/93 matched |
| full manifest in an `ubuntu:24.04` container (the CI image's default compilers) | g++ 13.3.0, clang++ 18.1.3 | 93/93 matched with each |
| Windows PE under Wine (`parity_wine.py`, shared classifier) | MinGW g++ 16.1.1, Wine 11.0 | 60/60 matched |
| runner self-tests | Python 3.14.7 | 44/44 |
| legacy host, private `--tests` tree | g++ 16.2.1 | empty `tests/` → exit 1; missing `tests/` → exit 1; one good module → exit 0; one good module plus a corrupt `.so` → exit 1 |

The `executorstress` check count varies between runs (for example 166,135 and
167,434) because some of its checks run inside timing-dependent loops. That is
existing behaviour, not a protocol gap. The count is recorded per run.

### Legacy host and CTSQueue triage

The legacy `unittest_legacy` run crashes or hangs in the tier0 `CTSQueue`
tests. The Q0 table records this. It is now attributed to **R20**, not the
runner, based on this triage:
- **Where it fails:** gdb shows `lock cmpxchg` on a NULL destination. `Push`
  read `m_Tail.pNode == NULL`.
- **Why:** `CTSQueue::Push` reads the tail with plain loads, and GCC 16 loads
  `pNode` before `sequence`. Unlike `Pop`, its help path installs
  `oldTail.pNode->pNext` without re-checking the sequence. Popped nodes return
  to a free list whose link aliases `pNext` and can be NULL. In addition, the
  `pNext` CAS has no ABA tag.
- **What was tried:** a `Push`-only fix (sequence first plus a barrier, and
  re-validating before helping) cleared the first failing test, but then
  crashed in multi-push/multi-pop and hung in single-push/multi-pop. It was
  reverted: the queue needs a redesign with native stress and TSan evidence,
  which belongs to RFC 0003 C / RFC 0006 M2.
- **Runner behaviour:** the host correctly reports the crash (exit 139). The
  hosted job is bounded.

### Honest non-claims

- R02 certifies the shared runner and its outcome handling, not any domain gate.
  RFC 0002, 0003 and other gates keep their own acceptance criteria.
- No hosted CI run has executed `conformance.yml` yet. The `ubuntu:24.04`
  container run shows the image's default compilers build and pass every suite.
  Making the check required is repository-administrator policy.
- Only the `linux-headless-core` runner profile executes here. Native
  GPU/window, device (Android/Apple), sanitizer and TSan lanes are separate
  profiles and gates. The g++ sanitizer runtimes are absent on this host (Q0).
- The runner bounds processes it starts. It does not prove safe production
  shutdown (RFC 0005).
- Evidence in `quality-results/` is per run and git-ignored. A linked old run
  cannot certify new code.

## Q-PRESENTATION: render backend contract (RFC 0001, contract-first)

Registered two suites (`render.backend.null`, `render.backend.sensitivity`,
migration `REND-BACKEND-001`) that pin the backend-agnostic obligations of RFC
0001's render-device capability family **before** any real backend is ported.
This is the RFC-sanctioned order: RFC 0001's render migration sequence makes
"add contract tests and a fixture" step 1. The material system consumes these
render services; the contract is what will let it be tested against a null
provider without process-global shader interfaces.

- Contract API: `public/render/render_backend.h` (`render::IRenderBackendProvider`
  and family) — self-contained, standard fixed-width types only. Building it under
  `-I public` alone mechanically proves the RFC obligation "absence of native
  window and graphics API types from public portable headers".
- Shared suite: `unittests/rendertest/conformance/render_backend_conformance.{h,cpp}`
  runs against **any** provider claiming the contract. Reference provider:
  `unittests/rendertest/fakes/null_render_backend.{h,cpp}` (deterministic,
  in-memory, no GPU). Contract record:
  `unittests/rendertest/contracts/render.backend.v1.md`.
- Obligations covered (33 checks): stable adapter descriptions; structured
  creation failure (invalid adapter, unsatisfied required feature); immutable
  capabilities; child-resource lifetime gated on **provider-owned completion
  tokens** (a frame index is not proof of completion, RFC 0006); ordered
  monotonic submission completion; presentation resize/orientation without device
  recreation; zero-size suspend (non-fatal) and resume; multiple surfaces up to
  `maxPresentations`; device-loss state machine (recover vs fatal); complete
  destruction with zero leaked devices.
  (2026-09-22, R16: the presentation obligations moved to the separate
  `render.presentation.v1` suite, `render.presentation.{headless,sensitivity}`,
  46 checks and 14 defects. The backend suite now has 24 checks and 7 defects,
  including `lifetime.owns_device`. See
  [presentation bridges](0001-presentation-bridge-progress.md).)
- Sensitivity: eight deliberately broken providers (one injected
  `NullBackendDefect` each) are each **detected** by the specific named check —
  proving the suite's teeth. `render.backend.null` and `render.backend.sensitivity`
  pass under g++ 16.2.1 and clang++ on `linux-headless-core` via the shared runner.

### Honest non-claims (render backend)

- This certifies **no GPU backend**. A null/recording provider certifies only
  command and lifetime behavior; image fidelity, performance, cross-backend pixel
  tolerances, native window/render interop, and concurrent submission are out of
  v1 scope and need real render-provider evidence (Q-PRESENTATION/Q-PRODUCT).
- It does not close R15 or any RFC 0001 render-migration gate; it is the
  contract-and-fixture-first artifact those gates build on. When a Vulkan/D3D9/GL
  provider lands it runs **this same suite**; a green run here means the suite is
  ready for that port, nothing more.

## Next steps

- Observe the first hosted `conformance.yml` run. Administrators decide whether
  to make it a required check.
- Grow the manifest as each RFC lands a suite (Q-FOUNDATION lifecycle/loader/ABI,
  Q-CONTENT corpora, Q-PHYSICS IVP/Box3D comparisons, Q-PRESENTATION command
  suites). Each addition is one manifest row plus a domain contract and a
  `checks-v1` suite.
- Add native/device profiles (and their `conformance-profile/v1` files) as the
  R03 toolchain boundary and R14–R18 native seams provide runnable targets.
