# RFC 0006 progress: per-target toolchain boundary

Updated: 2026-09-25
Portfolio row: R03 (M0), done (2026-09-25; see R03 closure).

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


## R05-STRONGID: strong identifier vocabulary (2026-09-25)

Scope: RFC 0006 "Results and value types" asks for strong IDs instead of
interchangeable integers. Before this slice, each contract hand-rolled its own
ID with different semantics: `platform::window::WindowId` had `IsValid` with
0 invalid but no ordering; `debugapi::ConnectionId` had ordering but no
validity. This slice gives the rule one owner and migrates those two
consumers.

- `public/foundation/strong_id.h`: `foundation::StrongId<Tag, Rep, Invalid>`.
  - It is an aggregate with a public `value`, so existing `Id{ n }`, `.value`
    and `==` uses compile unchanged.
  - `Invalid` is the default and the reserved "names nothing" value, and
    `IsValid()` tests against it.
  - It has defaulted `==` and `<=>` and a `std::hash` specialization.
  - There is no implicit conversion from or to `Rep`, and different tags
    neither convert nor compare.
  - Its layout is exactly `Rep`'s, and it is trivially copyable and
    standard-layout (static assertions).
- Consumers: `WindowId` and `ConnectionId` are now aliases of the shared
  type. The transports number connections from 1, and the transport suite
  already used `ConnectionId{}` as "none", so 0 as invalid matches existing
  semantics.
- Manifest: `standardHeaders` gains `compare`, and
  `platform.window-contracts` gains its `foundation` edge.
- Oracle:
  - `test_strong_id.cpp`: 9 runtime checks plus 10 static assertions.
  - Sensitivity sources `test_strong_id_mixed_tags.cpp` and
    `test_strong_id_implicit.cpp`; each fails to compile, as required.
- Evidence:
  - The whole manifest (`--runner all`, 220 suites): 193 matched,
    0 mismatched, 27 optional skips, with both g++ and clang++.
  - `build.tests`, `build.portal-native` and `build.portal-features` (debug
    API) pass.
  - `arch.check` and `arch.hermetic` (50 headers) pass.

**Scoped native resources (same slice).**

- `public/foundation/scoped_resource.h`: `foundation::ScopedResource<Traits>`.
  - It owns a non-pointer handle (a file descriptor, or an integer or 64-bit
    API handle). Pointer resources keep `std::unique_ptr` with a deleter, as
    RFC 0006 prescribes.
  - A `ResourceTraits` concept names `Handle`, a `noexcept Invalid()` and a
    `noexcept Close()`.
  - It is move-only and `[[nodiscard]]`, closes exactly once, and is safe on
    self-move. `Reset(h)` closes the previous handle but keeps an identical
    one open; `Release()` is `[[nodiscard]]`. Its layout is the handle's.
- Consumer: `debugapi`'s hand-rolled `ScopedFd` (26 lines) is now `FdTraits`
  plus an alias. Syscalls return -1 on failure, so `Invalid() == -1`
  preserves the old `fd >= 0` ownership rule.
- Oracle:
  - `test_scoped_resource.cpp`: 15 checks and 6 static assertions (closing
    exactly once, move and self-move, `Reset` and `Release`, a zero-invalid
    64-bit handle).
  - Sensitivity sources: a copy (deleted copy constructor) and an ignored
    `Release()` (`-Werror=unused-result`). Each fails to compile for exactly
    that reason.
- Evidence:
  - The debug API transport conformance (73 checks) and unix-socket endpoint
    test (18 checks) pass with g++ and clang++, the same counts as the
    pre-migration source.
  - `arch.check`, and `arch.hermetic` with 51 headers, pass.
- Finding: the `unittests/debugapitest` programs were referenced by no
  manifest row and no wscript, so nothing ran them. Transport (73 checks),
  unix-socket endpoint (18) and framing (51) are now headless manifest suites
  (`debugapi.*`). JSON-RPC and server need the generated protobuf headers of
  a `--debug-api` tree, so they stay unregistered for now.
- Manifest rows: `foundation.strong-id` and `foundation.scoped-resource`,
  plus four compile-error sensitivity rows with `expected_diagnostic`
  (`StrongId<BetaTag>`, `unsigned int`, `deleted`, `nodiscard`), and the
  three `debugapi.*` suites. All 9 match with g++ and clang++. The headless
  plan is now 166 suites, and `conformance.gcc`/`.clang` pass.

**Common test matchers (same slice).** RFC 0001 rank 2 names "common test
matchers". Before this, 14 test files re-declared the same `check( bool )`
lambda, with no location and no observed values.

- `public/testing/checks.h`: `testing::Checks`.
  - `That`, `Equal` (prints actual and expected when streamable, otherwise
    `<unprintable>`) and `Near` (tolerance; NaN never matches).
  - A failure prints `FAIL <file>:<line>: <what> (…)` using
    `std::source_location`, and never aborts.
  - The output stream is injectable. `Result()` feeds the linked runner, and
    `Report()` prints the checks-v1 record.
- Migrated: `test_expected`, `test_strong_id` and `test_scoped_resource`.
  They keep their `check(...)` calls through a thin adapter whose defaulted
  `source_location` names each call site. Counts are identical before and
  after (12, 9, 15), and the linked `unittest` host still reports
  `foundation.expected: 12`.
- Oracle:
  - `testing.checks` has 11 self-checks against a captured stream: counting,
    location, values, the unprintable fallback, tolerance and NaN, and
    zero-checks-never-passes.
  - `testing.checks.failing` is a sensitivity suite that fails one matcher
    check and must fail through the runner.
  - All 5 related rows match with g++ and clang++; `arch.hermetic` covers
    52 headers.
- Remaining cohort, with a deletion condition: the other 11 files with a
  private `check` lambda move to `testing::Checks` when next edited. They
  are `hammertest/adapters/adapter_checks.h`,
  `hammertest/formats/test_ktx2_preview.cpp`,
  `hammertest/ports/file_store_contract.h`,
  `mapcontainertest/test_world_lightmap.cpp`, `platformtest/composition/test_runner.cpp`
  (a runner self-test, kept independent on purpose), five
  `shaderapivulkantest` pixel suites, and two `texturecontainertest`
  readers.

**Quantities (same slice).** `public/foundation/units.h` is the one owner
of "one Source unit is one inch, 0.0254 m", with degree/radian conversions.
It is header-only, `constexpr` and dialect-neutral (it compiles as C++11).

- Its float forms use the same expressions as the literals they replace, and
  are bit-identical to them.
- Box3D's `kBox3DInchesPerMeter` and `kInertiaToBox3D` now use it. A
  `static_assert` ties the frozen `METERS_PER_INCH` ABI macro to the same
  value.
- Evidence:
  - `foundation.units`: 9 checks, including bit-identity, on g++ and clang++.
  - The recompiled Box3D object was byte-identical, so Waf did not relink
    the module.
  - `physics.conformance` observations are unchanged.
- `utils/vbsp/worldstage.cpp` (World Stage `metersPerUnit`) also uses it
  since the R59 U1 merge; `build.tools-worldstage` passes. No C++ literal
  copy remains in first-party code. Python tools keep their own constants,
  which is out of scope here.

R05 is `partial`. Every vocabulary item named in the row and in RFC 0001
rank 2 is installed with a real consumer and its own suite: results, strong
IDs, scoped ownership, matchers and quantities. The row cannot close while
its hard-gate prerequisites R03 (Apple/MSVC/CI) and R04 are open.

## R03 wording reconciliation (2026-09-25)

AGENTS.md ("C++ and code style") and RFC 0006's build-policy paragraph still
said not to change the whole tree's standard. That predated the user decision
of 2026-09-22 that every in-tree C++ target compiles as C++20. The decision is
recorded in `policy.json`, in this record and in the build. Both texts now
describe the installed policy:

- a per-target dialect owned by `policy.json`;
- `cxx20-permissive` for unmigrated code and strict `cxx20` for migrated code;
- `legacy-cxx11` only for frozen-consumer fixtures;
- Box3D on C17;
- no tree-wide floating-point change.

This removes the conflict listed as a user decision, as the recommended
option under the user's standing instruction (agent decision). No build flag
changed.

R03 stays `active`. The locally doable slices (A: the ABI contract; B: full
target coverage and Android x86_64) are done. What remains needs resources
this host lacks: Apple and MSVC toolchains, and hosted CI runs.

**Apple and MSVC runners optional (user decision, 2026-09-25).** The macOS,
iOS device, iOS simulator and both MSVC Windows profiles now carry
`runner_requirement: optional` in `quality/baseline.json`. It is validated by
`baseline.py`, with a new test (39 baseline tests). They stay declared and
reported when unavailable, but no gate waits on them.

R03's remaining blocker is the recorded R54 dual-ABI island in `vbsp2`
(TOOLCHAIN007 in `toolchain.coverage`), a declared-unsupported combination in
a built tool. The recommended fix moves the World Stage writer behind the
`sourceWorld` shared library so that only a C function crosses. It is not
started, because the R59 U2 work is editing `utils/vbsp` in a worktree.

**`GIT_COMMIT_HASH` is tier0-only (user request, 2026-09-25).** It was a
global `conf.define`, so a per-commit value sat on every compile command, and
ccache's direct-mode keys differed across trees and after every reconfigure.
Only `tier0/dbg.cpp` reads it (the engine log header).

- `tier0/wscript` now defines it for tier0 alone from `env.GIT_VERSION`; the
  root `wscript` no longer defines it.
- The corpus tool `compile_equivalence.py` falls back to `GIT_VERSION` when
  it records a tree's identity.
- Verified on `build-r03-tests` after replaying its configure line: 30 of 209
  compiles carry the define, all in tier0, and `unittest_legacy` passes.
- Every tree reconfigures once and rebuilds once. `./play` and `./play_p2` do
  it through `ensure_configured.py`.

## R03 closure (2026-09-25)

R03 is `done` for the required profiles, Linux x86_64 (gcc and clang) and
Android (arm64 and x86_64). Apple and MSVC runners are optional by user
decision (`runner_requirement: optional`), so no gate waits on them.

| Done clause | Evidence |
| --- | --- |
| Compile/link/run proof | `toolchain.boundary`: 18/18 dialect probes compile, link and run on the native host |
| Final flags verified | `toolchain.boundary` over 11 trees and `toolchain.coverage` over 12 trees with `--require-all-targets`: every policy target is covered, 0 errors. Every C/C++ task records its dialect, including C sources in cxx-only targets. |
| Legacy/C17 settings preserved | `box3d-c17` is verified on Box3D's target. `legacy-cxx11` is used only by the frozen-consumer fixtures. |
| Frozen-consumer ABI combinations | `toolchain.abi.*` (35 and 58 checks, with two sensitivity rows failing as required) on g++ and clang++, plus 2/2 frozen consumers linked against the Waf-built archives |
| Android final commands | `package.android-arm64` and `package.android-x86_64` run `toolchain_boundary` on each ABI's commands and pass |

**The last blocker: the `vbsp2` dual-ABI island.** It is now an enforced
policy exception, not a recorded gap.

- `quality/toolchain/policy.json` declares `abi.islands[vbsp2-openusd]`: host
  `vbsp2`, members `vbspworldstage` and `sourceWorld`, value 1, owner R54.
  A new supported combination, `declared-abi-island`, accepts only that
  exact split.
- `toolchain_boundary.py` TOOLCHAIN011 checks the island's edge with `nm` on
  the built objects:
  - no symbol that crosses it may carry a dual-ABI type (`St7__cxx11` or
    `B5cxx11` mangling);
  - no first-party inline (weak) symbol may be defined on both sides.
    Typeinfo objects and names (`_ZTI`/`_ZTS`) are ABI-neutral and exempt;
    vtables are not.
  - Missing objects fail.
- Today the edge is clean. The first run flagged only the two pure
  interfaces' typeinfo, which led to the typeinfo exemption above.
- Tests: 5 new fixtures, 16 toolchain tests in total. Four fixtures compile
  real objects at each ABI value: a C edge passes, a `std::string` crossing
  fails, shared inline code fails, and shared-interface typeinfo passes. The
  fifth checks that only the exact declared split is sanctioned. Three
  mutants (dual-ABI, weak, typeinfo) are each detected.
- `toolchain.coverage` is recorded as `pass` (agent decision under the
  user's standing instruction). This replaces the earlier "record as R54
  gap" choice with an enforced check.

Not covered: Apple and MSVC toolchains (optional), and hosted CI runs of the
toolchain lanes.

