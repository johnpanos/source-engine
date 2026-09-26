# RFC 0001 Phase A progress

Updated: 2026-09-25

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

Current state (2026-09-25): the freeze is still installed and in CI, but the
tree no longer matches it. The Phase A work items remain complete; the ratchet
now needs reviewed classification (owned by R04 and R07), never a blind rewrite.

- `check --all` and `baseline --verify` fail with 64 new occurrences and 1
  stale one. They are 50 `ARCH105` and 14 `ARCH101`–`ARCH103`. Most are in the
  Portal 2 client GameUI, VScript and matchmaking code added from 2026-09-22
  (`game/client/portal2/gameui/`, `game/client/gameui/`,
  `game/shared/portal2/`, `matchmaking/`). The rest are in `vphysics_box3d/`,
  `unittests/physicstest/` and `engine/vgui_baseui_interface.cpp`.
- `inventory --verify` rejects 13 native loader sites without telemetry:
  vendored `box3d/extern/sokol` (7) and `box3d/samples` (2),
  `external/portal2_steam2_xsi/src/SMDExport/expvalidate.cpp` (2),
  `unittests/physicstest/test_vphysics_conformance.cpp` (1), and the RenderDoc
  `RTLD_NOLOAD` probe in `materialsystem/shaderapivulkan/shaderapivulkan.cpp`
  (1). `nativeTelemetryCoverage` is therefore no longer complete for the tree.
- R01's [baseline](../quality/baseline.json) records the older drift (7 new and
  3 stale; 10 uninstrumented sites).

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

Update (2026-09-25): the `CTSQueue` crash is fixed (R20-POOL-TRUST; see the
[scheduler trust record](0003-scheduler-trust-progress.md)). R01 now records
the remaining `unittest_legacy` crash in this suite itself: in the installed
layout, `moduleloadtelemetrytest.cpp` cannot `dlopen` `./libmoduleloadfixture.so`
relative to the working directory. That fixture path is owned by R07.

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

## R04-DRIFT: loader-ratchet reconciliation (slice done, 2026-09-25)

Scope: review each of the 64 new and 1 stale `check --all` occurrences
(ARCH101–ARCH105). Classify each as one of:

- legacy-ABI package membership;
- relocated debt;
- a test host;
- a genuine new boundary expansion.

Following RFC 0001's baseline rules, a genuine new expansion is not absorbed
into `architecture/baseline.json`. It is fixed in code, or it becomes a
manifest exception naming an owner and a removal condition. The ratchet's own
negative fixtures must keep failing. The 13 uninstrumented native sites in
`inventory --verify` are R07's and are not in this slice.

Result: `archlint baseline --verify` passes, with 0 new and 0 stale entries.
The review classified 65 new and 2 stale occurrences (the count had grown
since the scope was written). User decisions of 2026-09-25 applied:

- **Deleted dead code.** `game/client/gameui/` and
  `game/client/gameui.{cpp,h}` were deleted. Their only build reference is
  the `$IFDEF_GAMEUI_UISYSTEM2_ENABLED` VPC folder, which nothing defines.
  No object in `build`, `build-p2` or `build-r03-portal-native` depended on
  them; the check read 4,301 `.d` files in `build-p2`. This removed 18
  occurrences.
- **Legacy-ABI package.** Six reviewed files joined it:
  - `matchmaking/main.cpp` and `mm_framework.h` (`IAppSystem::Connect`);
  - the Portal 2 GameUI `gameui_interface.{h,cpp}` (`IGameUI`);
  - `vphysics_box3d/main.cpp` (`IPhysics::QueryInterface`);
  - the VPhysics conformance host.

  The GameUI's duplicate declaration of `Portal2_ConnectMatchFramework` moved
  into one owning header, `game/shared/portal2/portal2_matchframework_connect.h`,
  which `portal2_shared_compat.h` also includes. The affected client and
  server TUs compile with `build-p2`'s recorded commands.
- **Baseline edits.** Two reformatted lines were replaced one for one
  (`IAppSystem.h:61` and `physics_environment.cpp:204`). One entry was
  removed as obsolete: `vphysics_box3d/main.cpp:9`, now covered by the
  package.
- **Exceptions.** `archlint` now enforces RFC 0001's rule for them.

### Loader exceptions (2026-09-25)

`architecture/modules.json` has a `loaderExceptions` section. Each entry
names:

- one existing file (never a glob or directory);
- one ARCH101–ARCH105 rule;
- the exact number of unbaselined occurrences it covers;
- a reason, an owning roadmap row, a tracking record and a removal
  condition.

`check` and `baseline --verify` fail on a malformed entry, a count mismatch
in either direction, or a stale entry. `baseline --write` never absorbs an
excepted occurrence. The fixtures cover each rule: 5 new tests, 80 archlint
tests in total. Three seeded mutants (count, stale, glob) are each detected.

The 19 entries cover the Portal 2 reconstruction glue.

- R39, which a typed composition replaces:
  - `Portal2_ConnectEngineInterfaces`, `Portal2_ConnectMatchFramework` and
    `Portal2_InitMatchFramework`;
  - the engine's `g_ClientFactory` probe for an embedded GameUI;
  - the `vscript` connection and its `Sys_LoadModule` fallback.
- R41, which a named extension host replaces:
  - the `vtex_dll` loads in the multiplayer options and add-ons dialogs;
  - the GameUI platform-module loader.

`check --all` still fails, only on 23 pre-existing CAP002 strict-capability
include findings (rendertest, shaderapivulkan, platform/window,
platform/sdl2, platformtest, `indirect_light.h`). That is the next R04 slice.
`inventory --verify` (13 sites) belongs to R07. `quality/baseline.json`
records `arch.baseline` as `pass` (user decision); `arch.check` stays
`fail` with its reason updated.

## R04-CAP: strict capability registrations (slice done, 2026-09-25)

Scope: the 23 CAP002 findings left in `check --all`. They came from headers
added after their strict modules were declared, and from the R14 SDL2 window
provider (`2134571b`), whose files fell under `platform.composition`'s
`platform/` prefix. Manifest-only changes; no source changed:

- **`standardHeaders`:** `ctime`, `deque`, `fstream`, `iterator` and
  `sstream` added.
- **`render.vulkan.core`:** `vulkan_compute.h`, `vulkan_debug_utils.h`,
  `vulkan_frame_stats.h` and `vulkan_shader_library.h` registered.
- **`render.contracts`:** `gpu_compute.h` registered.
- **`render.indirect-light`:** the RFC 0011 producer headers
  (`direct_occlusion.h`, `indirect_sdf.h`, `indirect_radiosity.h`)
  registered.
- **New modules:**
  - `platform.window-contracts` (`public/platform/window/`);
  - `platform.window-support` (the portable `platform/window/` helpers
    shared by providers and fakes);
  - `platform.sdl2.window` (backend, `SDL.h`);
  - `platform.sdl2.window.native-tests`.
- **Edges:** the SDL2 provider depends on the composition kernel for
  `IProviderLifecycle`, as the `platform.tests` fakes already do.
  `platform.composition` and `platform.tests` gain edges to the window
  contracts.

Evidence:

- `archlint check --all` passes, as do `baseline --verify`, `hammer
  --verify` and the 80 archlint tests.
- Four manifest mutants run against the real tree each fail:
  - removing the provider's window-support edge;
  - removing its `SDL.h` grant;
  - unregistering `vulkan_compute.h`;
  - granting `SDL.h` to the portable window-support module (CAP004).
- `quality/baseline.json` records `arch.check` as `pass` (user decision).
  The static audit group shows 0 deviations.

Still open for R04: transitive include and Waf/link graph enforcement
(`--compile-deps` is proposed, not installed), hermetic-build evidence, and
the Hammer include-graph cycle. `inventory --verify` (13 sites) is R07's.

## R04-DEPS: compiler-grounded transitive include check (slice done, 2026-09-25)

`archlint check --all --compile-deps TREE` (repeatable) implements the RFC's
"full mode" for the strict capability modules (CAP005). It reads every `-MMD`
dependency file in TREE. For each translation unit owned by a strict module
M, it checks:

- every reached first-party header belongs to M or to a module in M's
  transitive allowlist;
- a portable (non-native) M reaches no external, vendored (`thirdparty/`,
  `dependencies/`, `external/`, `box3d/`, `ivp/`) or unowned first-party
  header;
- a portable M reaches no native SDK. `-MMD` omits system-directory headers,
  where SDL3 and Vulkan live, so the check scans the include lines of every
  first-party file the compiler really reached for native families (SDL,
  Vulkan, X11, xcb, Wayland, EGL/GL, Win32/D3D, Metal/UIKit/AppKit, Android,
  GTK), ignoring commented-out includes.

A tree without dependency files, or with no strict units, fails.

Evidence:

- The real trees are clean: `build-r03-portal-native` (2,506 dependency
  files, 109 strict units), `build-r03-tests` (395 files, 24 units) and
  `build-r03-dedicated` (2,679 files, 15 units). Units cover 19 modules,
  46 of them portable (`world.map-container`, `platform.composition`,
  `platform.tests`, `testing.runner` and others).
- 8 fixtures (88 archlint tests in total) cover:
  - the allowed closure;
  - an out-of-closure header;
  - unowned, vendored and external headers;
  - a system Vulkan include reached through a first-party header;
  - commented-out includes;
  - native grants;
  - non-strict units;
  - the parser.
- Four mutants (closure, native scan, unowned, vendored) are each detected.

Blind spots:

- Only configurations that were built are seen (the trees above, Linux).
- `.d` files record what the preprocessor included under that tree's
  defines, so another profile's `#ifdef` branch needs its own tree.
- Hermetic one-line header compiles: see R04-HERMETIC below.
- The link graph (CAP006, added the same day, below) judges only portable
  strict targets.

**Link graph (CAP006).** Each `--compile-deps TREE` also reads
`TREE/toolchain-invocations.json` (each compile's target, sources and `use`).

- A target whose strict sources are all portable may not `use` a native SDK
  library (SDL, Vulkan, X11/XCB, Wayland, EGL/GL/GLES, GTK/GDK/libadwaita,
  epoxy, D3D/DXVK, Metal/MoltenVK).
- It may not link a first-party target whose strict code lies outside its
  modules' allowed closure.
- Targets that mix native and portable modules, or have no strict source,
  are counted but not judged. That needs the RFC's per-target `arch_module`
  ownership, which is not implemented.

Results: 10 portable strict targets judged and clean. 19 mixed or native
targets are not judged (18 in the Portal tree, 1 in dedicated). There are
4 fixtures (92 archlint tests in total), and three mutants (native library,
closure, mixed-target skip) are each detected. `arch.compile-deps` includes
it.
- It is declared as `arch.compile-deps` in `quality/baseline.json` over the three trees (user decision), and passes in 16 s.

## R04-HERMETIC: hermetic contract-header compiles (slice done, 2026-09-25)

`archlint hermetic [--cxx g++] [--cxx clang++]` implements CAP007. Every
public header of a portable (non-native) strict module is compiled alone,
from a one-line translation unit, with only the `public/` include root,
`-std=c++20 -Wall -Wextra -Werror`, and `-M`.

- `-M` lists system headers too, unlike the build's `-MMD`. So the check
  rejects any native SDK path (SDL, Vulkan, X11/XCB, Wayland, EGL/GL, GTK,
  libadwaita, D3D, Metal/MoltenVK, Android) anywhere in the resolved closure,
  and any repository header outside `public/`.
- On Linux the SDKs live in `/usr/include`, so omitting their `-I` paths
  alone would prove nothing. The resolved closure is the evidence.

Results: 49 headers, 0 errors with g++ 16.2.1 and with clang++ 22.1.8, in
2.4 s. The 5 fixtures inject the compiler, so they need no host SDK; one
uses the real g++. There are 97 archlint tests in total. Three mutants
(native path, non-public header, compile failure) are each detected.

Blind spots:

- Headers of native (backend/interop) modules are out of scope by design.
- Contract headers outside `public/` are not compiled.
- Only the Linux host compilers are covered.

Declared as `arch.hermetic` in `quality/baseline.json` (static group, user decision).

## R04-TARGETS: link graph for every strict target (slice done, 2026-09-25)

CAP006 now judges every target whose sources are all strict, native ones
included, not only portable targets. RFC 0001's "external libraries are
allowlisted" rule has an owner: a per-module `uselib` list.

- **Rule.**
  - A target's allowed modules are the union of its strict modules' closures;
    every first-party target it links must stay inside that union.
  - Every native SDK library it links must be granted by one of its modules'
    `uselib`.
  - Only native kinds may declare `uselib` (CAP004), so a portable target can
    link none.
  - Mixed targets, with strict plus legacy sources, are counted but not
    judged. The link from `materialsystem` to `shaderapivulkan`, for example,
    comes from legacy provider-catalog code, not from its strict modules.
    Judging those needs the RFC's explicit per-target `arch_module` owner,
    which is not installed.
- **Grants.**
  - `render.vulkan.core`: VULKAN only, never SDL3, preserving the rule that
    only the SDL3↔Vulkan bridge sees both.
  - The Vulkan bridges and native tests: VULKAN, and SDL3 where they own
    SDL.
  - The SDL3 launcher, render surface and tests: SDL3. The SDL2 window
    provider and its test: SDL2.
  - `content.ktx2-reader`: VULKAN, matching its declared `vulkan_core.h`
    header grant.
  - Edge: `platform.sdl3.native-tests` → `platform.sdl3.launcher`, the module
    it tests.
- **Evidence.**
  - `arch.compile-deps` now also runs `build-r03-portal-features` (debug API
    and KTX2) and passes in 22 s. Strict targets judged: 4 in
    `portal-native`, 5 in `tests`, 4 in `dedicated`, 8 in `portal-features`.
  - 110 archlint tests pass. Three mutants (dropping the grant check,
    judging mixed targets, allowing portable grants) are each detected.

Still open for R04:

- per-target `arch_module` ownership for the mixed targets (80 in the Portal
  trees, mostly legacy);
- the Waf-time checker the RFC describes, beyond reading recorded
  invocations.

## R07-INVENTORY: loader inventory reconciled (slice done, 2026-09-25)

`archlint inventory --verify` passes again; it had rejected 13 uninstrumented
native loader sites.

- **Two first-party sites now go through the Phase A telemetry adapter**
  (`tier0/native_module_load_telemetry.h`):
  - the RenderDoc in-application API probe in `shaderapivulkan.cpp`;
  - the VPhysics conformance host's provider `dlopen`.

  Both binaries were rebuilt and import `ModuleLoadTelemetry_dl*`. The
  physics host still runs: IVP boot 150/150 and gameplay 453/453; Box3D
  shows only its recorded 3 failures (R19).
- **Eleven sites are in vendored trees no product builds**: Box3D's sokol
  headers and samples (9) and the XSI SMD exporter (2). They are covered by a
  reviewed `loaderInventory.unbuiltVendorPaths` exclusion.
  - Each entry names one directory with a reason and the evidence reviewed.
    At review, none of the 47 recorded `build*/toolchain-invocations.json`
    files compiled a source under these prefixes, and no wscript names them.
  - `inventory --verify` rechecks the recorded trees on every run, and fails
    if any compiles a source under an excluded prefix.
  - The sites are classified `unbuilt-vendor` rather than defaulting to
    `first-party-composition`, and are left out of telemetry coverage.
- **Reviewed inventory drift, then rewritten:**
  - 36 entries only shifted line numbers;
  - 10 new `retained-extension` sites are the Portal 2 GameUI/VScript glue
    already owned in R04-DRIFT's exceptions (R39/R41);
  - 1 new `optional-provider` and 1 `tool-indirection` site are the two
    instrumented above;
  - native telemetry coverage is complete at 92/92.
- **Tests.** 4 fixtures (108 archlint tests). Two mutants (skipping the
  built-vendor check; dropping the classification) are each detected.
- **Recorded outcome.** `arch.inventory` is `pass` in `quality/baseline.json`
  (agent decision under the user's standing instruction, 2026-09-25).

**Fixture path and a tier0 logger race (same day).** `unittest_legacy`, the
legacy test host recorded as crashing on both compilers, now passes.

- *Fixture path.* `moduleloadtelemetrytest.cpp` `dlopen`ed
  `./libmoduleloadfixture.so` and `./liblegacymoduleclientfixture.so`
  relative to the working directory. The installed layout keeps them in
  `bin/`. The test now resolves `./` then `./bin/` with `access()` before
  loading. That makes no loader call, so the telemetry stream still holds
  exactly the events the tests assert (for example 3 for the native
  lifecycle).
- *Logger race.* With the path fixed, clang crashed in 2 of 6 runs. The core
  dump showed the `ConcurrentFailedLoad` threads calling `Warning()` together
  and dying in `CDbgLogger::Write`'s `memmove`.
  - `tier0/dbg.cpp`'s logger checked and incremented its message count with
    no lock, so concurrent writers could take one slot or run past
    `MAX_MSGS`.
  - `Disable()` and the destructor also freed slots `iMsg..1`: one
    uninitialized pointer past the end, and they leaked slot 0.
  - Now a `CThreadFastMutex` guards the buffer and the file for
    `Write`/`Init`/`Disable`. `Init` flushes buffered messages without
    re-entering `Write`, and exactly `0..iMsg-1` are freed.

  This logger is in every tier0 product, not only the test host.
- *Evidence.*
  - 20 of 20 runs pass with each compiler. At the observed pre-fix rate of
    about 1 crash in 3 runs, 20 clean runs by chance would have probability
    about 0.0003.
  - The build group passes: 15 checks, the only `fail` being the recorded
    R54 `toolchain.coverage`.
  - `legacy.unittest`/`-clang` are unchanged. `legacy.unittest-legacy` and
    `-clang` are recorded as `pass` (agent decision under the user's
    standing instruction), with a 25 s budget for a complete run.

**Line-only drift (2026-09-25).** Site fingerprints already exclude line
numbers, yet `inventory --verify` failed whenever an edit above a loader
site moved it. That happened three times in one day, and each time forced a
rewrite that reviewed nothing. `--verify` now compares sites without their
line numbers and reports how many only moved; any added, removed or changed
site still fails. `--write` still records current lines. There are 2 new
tests (112 archlint tests).

Still open for R07: the telemetry cases for failed, duplicate and nested
requests (the concurrent and nested tests now run to completion), the legacy
bridge, and fake/native suites. No TSan run of tier0 was made; the logger
race is fixed by construction and shown by the repeated runs.

## Next increment

Phase A is closed. Later RFC 0001 phases begin removing the frozen loader
surface: rename the `LoadModule( CreateInterfaceFn )` pseudo-module overload,
give `CAppSystemGroup` explicit `IAppSystem` instances, link mandatory
first-party systems through typed factories, and replace backend loading with
typed provider catalogs. Source presence alone does not establish ABI or
behavioral compatibility; each removal carries its own gate.

Phase B has since renamed the pseudo-module overload and added explicit
`IAppSystem` instances; see the [Phase B record](0001-phase-b-progress.md). The
typed-factory and provider-catalog work is tracked under R39, and tool
cleanup under R40 ([Phase E](0001-phase-e-progress.md)).
