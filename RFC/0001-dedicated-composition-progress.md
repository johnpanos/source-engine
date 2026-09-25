# Dedicated composition migration slice

Updated: 2026-09-25

The dedicated child application group now registers one `IAppSystem` compatibility
bridge. `ApplicationComposition` orders, connects, initializes, rolls back, and
tears down the product systems behind it. The bridge keeps the existing interface
names, `CreateInterface` lookup behavior during legacy `Connect` callbacks, and
the host's separate `PreInit`/`PostShutdown` phases. The child group's physics
module handle transfers to composition and unloads after provider teardown. New
composition capabilities stay unpublished until connection succeeds.

The dedicated exports are the first native service in this graph. Composition
owns `IDedicatedExportsService`; the preserved `IDedicatedExports` singleton
forwards mod-facing calls only while the service is bound. Other dedicated
services remain module-owned `IAppSystem` implementations reached through
explicit transition adapters. The bridge and the same authoritative composition
kernel source are compiled with the legacy target's libstdc++ ABI, so no STL
object crosses an ABI variant at this boundary.

## Evidence on Linux x86_64, Portal profile

- `WAFLOCK=.lock-waf-ded-composition-tests ./waf build
  --targets=dedicatedcompositiontest,appsystemgrouptest -j8`: passed. The tests
  cover mod-facing lookup, ordered lifecycle, rollback at connection and
  initialization, native service construction failure, restart, module unload
  order, and host cleanup.
- `python3 tools/quality/conformance.py check --domain Q-FOUNDATION`: 17/17
  suites matched, including 252 composition checks.
- `WAFLOCK=.lock-waf-ded-composition ./waf build
  --targets=dedicated,dedicated_launcher,server -j8`: passed.
- `python3 tools/quality/toolchain_boundary.py check
  build-ded-composition-tests/toolchain-invocations.json
  build-ded-composition/toolchain-invocations.json`: passed, zero errors.
- One Portal `testchmb_a_01` dedicated BSP/BSP2 installed-runtime probe passed;
  see `quality-results/ded-composition-smoke-final/evidence.json`.
- `python3 tools/stylelint/stylelint.py --changed`: no finding in this slice;
  the full current branch diff is red from a concurrent shader edit. `archlint
  check --all`, baseline verification, and loader inventory remain red from
  unrelated current-tree changes; no finding in the new bridge paths remains
  after the legacy ABI classification.

This does not close R06 or R12. The parent Steam application still owns its
bootstrap services, the module resolver is still legacy, and most dedicated
systems remain legacy implementations. The current dedicated
binary also links material/studio render support and has not met R12's absent
render/UI proof. Linux native runtime evidence does not certify Windows or the
other planned platforms.

State on 2026-09-25: the dedicated Waf target still links `materialsystem`,
`shaderapiempty` and `studiorender`, and `CSys::LoadModules` still loads the
physics provider by filename. The dedicated builds pass: the R01 re-audit
(2026-09-25) built `build.dedicated` (gcc) and `build.dedicated-clang` after
`97e298c6` replaced the protected `LoadModule` call. The clang build also
needed a `(void *)` cast of `CSys::GetProcAddress`'s `long` handle for
`dlsym` (`sys_linux.cpp:118`). [`quality/baseline.json`](../quality/baseline.json)
records both as `pass` (user decision). This is build evidence only; the
installed startup/shutdown and link gates of R12 remain open.
