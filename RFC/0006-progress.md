# RFC 0006 progress: per-target toolchain boundary

Updated: 2026-09-25
Portfolio row: R03 (M0), active.

## Bounded scope

Make the declared C++20 dialect the normal in-tree Waf build setting. Keep the
strict first-party targets separately selected, preserve Box3D's C17 flags and
the C++11 frozen external-consumer fixtures, and validate the final commands and
mixed-target ABI calls. The root build no longer accepts a private environment
override that silently puts unmigrated targets back on C++11.

## Current evidence and remaining gate

The installed Waf dialect tool and `quality/toolchain/policy.json` declare and
check the target settings. R01 captured ten Linux build trees and 18 successful
compiler probes under the C++20 opt-in; this established feasibility but did
not make normal builds use the declared policy. R03 remains active until fresh
normal-build evidence covers the installed product matrix, the final flag and
frozen-consumer checks pass, and supported non-Linux toolchains are assessed
against their declared profiles. Unavailable Apple runners remain explicit
verification gaps, not inferred support.

## State on 2026-09-25

- By user decision (2026-09-22, recorded in `quality/toolchain/policy.json`),
  the root wscript applies the policy default `cxx20-permissive` to every
  in-tree C++ target. The policy declares 38 target entries: 32 strict `cxx20`,
  5 `cxx20-vendored-ivp` and Box3D's private target on `box3d-c17`.
  `legacy-cxx11` is used only by frozen-consumer fixtures in the conformance
  manifest.
- R01's [baseline](../quality/baseline.json) records `toolchain.boundary`
  passing over the recorded Linux trees. It also records passing `tests`,
  `tools` and native-Vulkan Portal builds with gcc and clang, and an `hl2`
  build with the default compiler.
  `build.dedicated` and `build.dedicated-clang` pass since the R01 re-audit
  (2026-09-25); see the [dedicated record](0001-dedicated-composition-progress.md).
- `build-android-apk.sh` runs `toolchain_boundary.py check` on each ABI's
  final commands and stops the package build on failure. R01 records
  `package.android-arm64` passing, so the arm64 Android final commands passed
  that check. The x86_64 ABI has no baseline check.
- `.github/workflows/toolchain.yml` and `apple-toolchain.yml` declare the
  Linux and Apple probe lanes. Neither has hosted-CI results recorded here.
  The only local Apple evidence, `quality-results/r03-apple-local.json`, is a
  `fail` decision because the probe requires an arm64 macOS runner.
- Later stages have started outside this record, and none of them is closed.
  For M1 (R05), the project `Expected` is `public/foundation/expected.h`, with
  the `foundation.expected` suite and its crash and compile-error negative rows.
  It is used by the composition kernel, map and texture containers, the debug
  API and indirect lighting. M2 and M3 have bounded-queue and completion-token
  work in the [scheduler](0003-scheduler-nodes-progress.md) and
  [presentation bridge](0001-presentation-bridge-progress.md) records.

## R03-A: frozen-consumer ABI contract (slice done, 2026-09-25)

Scope: the four `toolchain.abi.*` manifest suites (two positive, two
sensitivity) already run in the R01 baseline's `conformance.gcc` and
`conformance.clang` checks, and both negative controls fail as expected there.
All four name `unittests/toolchaintest/contracts/toolchain.abi.v1.md`, which
does not exist and has no history; it is the only missing contract among the
manifest's 140 contract references, and the runner hashes it as absent without
complaint. This slice writes that contract from the fixture sources and makes
manifest validation reject a suite whose declared contract file is missing,
with a negative self-test. It changes no dialect selection and does not touch
the whole-tree C++20 policy question.

Result:

- [`toolchain.abi.v1.md`](../unittests/toolchaintest/contracts/toolchain.abi.v1.md)
  states what the fixtures assert: layout and dialect identity, calls in both
  directions, the rejection paths, one libstdc++ ABI, and what each
  sensitivity suite must detect. It also records the Linux/libstdc++-only
  scope. It claims no check the code does not make: the invalid-batch case
  checks rejection, not the absence of callbacks.
- `conformance.py`'s `load_manifest` rejects a suite whose `contract` is not a
  non-empty string or does not name an existing file under the root. Before
  the contract was written, `conformance.py plan` on the real manifest failed
  with `suite toolchain.abi.jobsystem-batch names missing contract …`; that
  was the live negative control. It now plans 157 suites. There are 3 new
  runner self-tests (missing, present, non-string); `test_conformance` has
  62/62 passing.
- The four ABI suites pass as expected with g++ and clang++: 35 and 58
  checks positive; the dialect-layout (37) and dual-ABI (35) sensitivity
  suites fail as required.

R03 stays `active`. Still open:

- fresh evidence for the normal build under the current policy digest, with
  `--require-all-targets` coverage (slice B);
- Android x86_64;
- Apple and MSVC toolchains (no runners here);
- hosted CI lanes;
- the user decision on the whole-tree C++20 wording conflict between
  AGENTS.md / RFC 0006 and `policy.json`.


## R03-B: complete Linux target coverage and Android x86_64 (slice done, 2026-09-25)

Scope (user decision, 2026-09-25):

- Turn on `--require-all-targets` for the baseline `toolchain.boundary` check.
  With the 10 R03 trees it fails with TOOLCHAIN010 for 8 policy targets that
  only feature-gated configurations build: `debugapi_core` (`--debug-api`),
  the KTX2/VTF texture readers and their conformance tests (KTX roots), and
  `sourceWorld`/`vbspworldstage` (OpenUSD).
- Add two declared baseline trees built only from pinned inputs
  (`quality/product_profiles/pbrt-map-linux-tools.json` layout;
  `build-deps/debugapi/prefix`):
  - `build-r03-portal-features`: Portal native Vulkan with `--debug-api` and
    the KTX reader;
  - `build-r03-tools-worldstage`: tools with OpenUSD and oneTBB.
- Add a `package.android-x86_64` check (cross-build, APK verifier and the
  toolchain boundary; no device).
- Oracle: `toolchain_boundary.py check --require-all-targets` over 12 trees.
  Negative control: a new self-test in which an uncovered policy target
  produces TOOLCHAIN010 and full coverage produces none (11/11 pass).

Result:

- New declared checks in `quality/baseline.json` (45 checks):
  - `build.portal-features` and `build.tools-worldstage` (both `pass`);
  - `package.android-x86_64` (`pass`: 29 libraries, the APK verifier, and
    `toolchain_boundary` over the x86_64 commands with 0 errors);
  - `toolchain.coverage`.
- `toolchain.boundary` now covers 11 trees (the 10 plus the features tree)
  and still passes.
- `toolchain.coverage` runs all 12 trees with `--require-all-targets`.
  - Every policy target is covered: 0 TOOLCHAIN010.
  - It fails only on TOOLCHAIN007, because `vbsp2` links the ABI=1 OpenUSD
    island (`vbspworldstage`, `sourceWorld`) into an ABI=0 closure.
    `policy.json` declares that combination unsupported.
  - A symbol review found only ABI-neutral symbols crossing today.
    However, the ABI=1 object emits inline `FileByteSink` COMDATs (a class
    holding `std::string`) that any future ABI=0 definition in `vbsp2` would
    collide with silently.
  - By user decision it is recorded as a known `fail` owned by R54. It is a
    separate check so that `toolchain.boundary` keeps catching regressions in
    the other trees.
- Audits: `r03b-2026-09-25` and `r03b-android-2026-09-25` both had 0
  deviations.

R03 stays `active`. Still open:

- Apple and MSVC toolchains (no runners here);
- hosted CI lanes;
- the R54 ABI island;
- the user decision on the whole-tree C++20 wording conflict.

