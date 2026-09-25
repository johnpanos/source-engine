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
