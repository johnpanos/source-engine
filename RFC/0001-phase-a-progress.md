# RFC 0001 Phase A progress

Updated: 2026-09-22

Phase A covers the dynamic-module-loader retirement inventory and freeze. This
file is the durable progress record; detailed current sites live in
`architecture/loader_inventory.json` and the enforcement snapshot lives in
`architecture/baseline.json`.

## Status

| Work item | Status | Evidence |
| --- | --- | --- |
| Classify every static load site | Complete | `architecture/loader_inventory.json` classifies every recorded loader-surface site into first-party composition, retained extension, optional provider, tool indirection, and native symbol probe; `inventory --verify` passes |
| Reject new `Sys_LoadModule`, `Sys_GetFactory`, filesystem `LoadModule`, and `CDllDemandLoader` dependencies | Complete | `ARCH101`–`ARCH104`; `check --all` and `baseline --verify` pass against the reviewed exact fingerprints |
| Prohibit new `CreateInterfaceFn` boundaries outside the legacy ABI package | Complete | `ARCH105`; legacy paths declared in `architecture/modules.json` |
| Run the freeze in CI | Complete | Architecture job in `.github/workflows/tests.yml` |
| Instrument load attempts and unloads with requester, resolved path, entry/interface, result, and lifetime | Complete | `nativeTelemetryCoverage` reports complete in `architecture/loader_inventory.json`: every recorded native-loader site is routed. Tier 1 / filesystem / appframework paths use `CScopedModuleLoadRequest` + the recording hooks; direct native `dlopen`/`dlsym`/`dlclose` and `LoadLibrary*`/`GetProcAddress`/`FreeLibrary` sites route through the `tier0/native_module_load_telemetry.h` adapters; standalone launchers that bootstrap Tier 0 itself use the self-contained `public/tier0/bootstrap_module_load_telemetry.h` |
| Process-shutdown reporting for intentionally retained handles | Complete | In-process `CModuleLoadTelemetryShutdownReporter` (`tier0/module_load_telemetry.cpp`) and the bootstrap `atexit` reporter (`public/tier0/bootstrap_module_load_telemetry.h`) emit an unload record for every handle still loaded at process teardown |
| Frozen-header ABI fixture | Complete | `unittests/legacymoduleclientfixture` is compiled against frozen pre-Phase-A loader declarations and loaded by the current host in `ModuleLoadTelemetryFrozenLegacyAbi`; the fixture's own loader calls surface on the shared stream as `<legacy ABI caller>` records with correlated load IDs |

Assessment at source revision `91c47351` (the shared worktree also carries
uncommitted, behavior-preserving refinements to the telemetry sources): all
architecture-checker tests pass; `check --all`, `baseline --verify`, and
`inventory --verify` all pass.

The isolated Tier 1 loader-telemetry composition
(`unittests/tier1test/moduleloadtelemetrytest.cpp`, the `moduleloadfixture`
interface fixture, and the `legacymoduleclientfixture` frozen-ABI fixture) was
built and executed with `-moduleloadtelemetry`; the `unittest` host returned
exit code 0 with zero assertions and the ten Phase A telemetry cases passed. The
emitted stream demonstrated correlated `LOAD` / `ENTRY_POINT` / `INTERFACE` /
`UNLOAD` records with resolved paths, structured provider errors, non-zero
lifetimes, and stable per-load IDs across every routed path class:

- filesystem-mediated (`basefilesystem.cpp`),
- application composition (`appframework/AppSystemGroup.cpp`),
- Tier 1 `Sys_LoadModule` / `Sys_LoadInterface`,
- direct native `dlopen`/`dlsym`/`dlclose` via the native adapters, and
- a frozen legacy-ABI client whose loads appear as `<legacy ABI caller>`.

Negative behaviors are exercised in the same run: failed loads record a
structured provider result and error, partial-startup interface failures roll
back with a matching `UNLOAD`, duplicate native handles receive distinct load
IDs, nested and concurrent request contexts attribute the correct requester and
source line, and a sink that unregisters itself from within its own callback is
not re-entered.

A release game composition (`build-phase-a-game`) was built in prior Phase A
work as product-build evidence; it was not re-run in this session and the full
`unittests`-with-`unitlib` legacy composition still cannot be used as a single
end-to-end run because its pre-existing `TSList`/`TSQueue` stress tests crash
before completion — the loader-telemetry suite is therefore built and run as the
isolated composition above.

Phase A (dynamic-module-loader retirement, inventory and freeze) is complete:
every load site is classified and frozen behind the architecture linter, every
recorded native-loader site is instrumented, intentionally retained handles are
reported at shutdown, and a frozen-header ABI consumer is verified against the
current host. Removing loader sites and decomposing the loader into typed
provider catalogs / extension hosts is later-phase work (RFC 0001 Phases B–G and
roadmap rows R39–R46), not part of Phase A.

Shared runner/evidence and loader tests are specified by
[RFC 0005](0005-quality-and-correctness-harnesses.md); implementation work is
tracked as R01/R07 in [AGENTS.md](../AGENTS.md).

## Commands

```text
python3 tools/archlint/archlint.py check --changed
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
python3 -m unittest discover -s tools/archlint/tests -v
```

Baseline or inventory rewrites are intentional review actions. After reviewing
the diff and its classification, use the corresponding `--write` command. When
`inventory --verify` reports staleness that is only line-number drift from an
in-place edit to an already-recorded telemetry site (same file, same call), the
refresh is a reviewed `inventory --write` with no classification change.

To reproduce the isolated loader-telemetry run, build the `tests` group and,
from a directory containing the `unittest` host, `tests/libtier1test.so`, the
shared libraries (`libtier0.so`, `libunitlib.so`, `libvstdlib.so`), the two
fixture libraries in the working directory, and `bin/filesystem_stdio.so`, run
`./unittest -moduleloadtelemetry`.

## Next increment

Phase A is closed. Later RFC 0001 phases begin removing the frozen loader
surface: rename the `LoadModule( CreateInterfaceFn )` pseudo-module overload,
give `CAppSystemGroup` explicit `IAppSystem` instances, link mandatory
first-party systems through typed factories, and replace backend loading with
typed provider catalogs. Source presence alone does not establish ABI or
behavioral compatibility; each removal carries its own gate.
