# RFC 0027 progress: One Product Pipeline and `kiln`

Design: [RFC 0027](0027-product-pipeline-lowering-streaming-kiln.md).
Roadmap: row R102, rank 75 (L0, L1, L7). The RFC first named it R98, an id
RFC 0026 also took for physics on 2026-10-07; R102 and the rank are an agent
placement (2026-10-07), movable by the user.

## L0: the pipeline library, profile schema v2 and `kiln build` (2026-10-07)

User goal: "complete L0: the pipeline library, profile schema v2 and kiln
build on the desktop profiles. Nothing gets deleted until L1's equivalence
check passes. Start on a worktree." Worked on branch `kiln-l0` in the
worktree `../source-engine-kiln`, by this session alone.

Nothing in the launch or build workflow is deleted or changed: `./play*`,
`run.sh`, `run.conf`, the platform scripts, `render_flags.sh`,
`ensure_configured.py` and `profile_extends.py` all stay and work as before
(L1 deletes them once its launch-equivalence check passes). The one moved
file is Hammer's private JSON value, which became `foundation.json`, as the
RFC's layer table requires (the "second consumer"); the Hammer MCP adapter
now uses it, and its suite passes unchanged.

### What is installed

| Module (`architecture/modules.json`) | Paths | Owns |
| --- | --- | --- |
| `foundation.json` | `public/foundation/json.h`, `foundation/` | The JSON value, reader, compact and pretty writers, structural equality. Extracted from `hammer/adapters/mcp/json_value.*` (history kept) |
| `product.profile` | `public/product/profile.h`, `product/profile/` | Schema v2, the `extends` merge (string or list), provenance, validation, aliases, flavors, derived Waf arguments, the workspace file's refusal of build facts |
| `product.contracts` | `public/product/contracts.h`, `product/contracts/` | `ITargetToolchain`, `IRecipeBuilder`, `IProductStage`, `IPackager`, `IDeployTransport` (split into `IInstall`, `IContentSync`, `ILaunch`, `IDeviceFacts`, ...), `IDisplaySession`, and the typed `ProviderCatalog` |
| `product.toolchain.linux` | `public/product/toolchain_linux.h`, `product/toolchain/` | The `linux-gcc` and `linux-clang` providers: pins verified against the host compiler, identity (compiler versions, target triple, SDK) |
| `product.stage.waf` | `public/product/stage_waf.h`, `product/stage/` | The `waf-engine` stage: configure only when the configure digest (arguments plus toolchain identity) changed or a configuration input is newer than the tree, then `waf install`; the lock stays in the tree |
| `kiln.core` (public as `kiln.api`) | `public/kiln/api.h`, `product/kiln/core/` | `kiln::Session`: profiles, doctor, and `Run` (the stage graph on the injected `jobs.graph` executor; package, deploy and run steps over the named contract providers; evidence) |
| `kiln.composition` | `public/kiln/composition.h`, `product/kiln/composition/` | `ComposeDefaultCatalog()` and `ComposeDefault()` |
| `kiln.app` | `product/kiln/app/` | The thin `kiln` application: `profiles list\|resolve\|explain`, `doctor`, `build [--flavor]`, `--json`, `--root` |
| `kiln.fixture-platform` | `unittests/kilntest/fixture_platform/` | One provider per contract, composed only by the test root |
| `kiln.tests` | `unittests/kilntest/` | The shared suites, the bad providers and the gate checks |

Also:

- `./kiln`, the POSIX bootstrap (RFC 0027 "The commands"): configures the
  host tree `out/host-<os>-<arch>/` (the tools product, render core without
  its Vulkan adapter) on first use, rebuilds the `kiln` target only when a
  source under `foundation`, `product`, `jobsystem`, `platform` or their
  public headers is newer than the binary, then execs it. A warm call takes
  about 30 ms.
- `quality/product_profiles/`: three fragments and ten schema v2 desktop
  profiles (below).
- `tools/kiln/kiln_gate.py`: the gate checks that need the real repository
  (`parity`, `rebuild <profile>`, `selftest`).
- archlint CAP011 rules 6 and 7 and two layer contracts, `product` and
  `kiln` (below).
- `.gitignore`: `/out/` and `/.kiln/`.

### Profiles, schema v2

Fragments (`source-product-fragment/v2`, no target):

- `fragments/linux-x86_64-desktop.json`: the target, the pinned host GCC
  and pkg-config dependencies (copied exactly from
  `portal-linux-wayland-native-vulkan.json`), `build.toolchain: linux-gcc`,
  the `dev` and `release` flavors (RFC 0023: `product_flavor=release`,
  `enable_lto`), `pipeline.stages: ["waf-engine"]`.
- `fragments/render-switches.json`: the 16 switches of `render_flags.sh`
  as data (`--core-probe=MODE` became two switches; `--native` is the
  default; `--dxvk` went with DXVK Native, deleted on 2026-10-07), each with a description, arguments and
  conflicts.
- `fragments/desktop-launch.json`: `ivp`, `serial-jobs`, `sync-queue` and
  `bink`, the environment variables of `run.conf` and `play_p2`.

Profiles (`source-product-profile/v2`), each with a host alias:

| File | Alias | Tree |
| --- | --- | --- |
| `portal-linux-native-vulkan.json` (extends the v1 slice) | `portal` | `out/portal-linux-native-vulkan/<flavor>` |
| `portal2-linux-native-vulkan.json` | `portal2` | `out/portal2-linux-native-vulkan/<flavor>` |
| `portal2-fsr.json` | `portal2-fsr` | `out/portal2-fsr/<flavor>` |
| `portal2-high.json` (Portal 2 plus the v1 High profile) | `portal2-high` | `out/portal2-high/<flavor>` |
| `portal2-coop.json` | `portal2-coop` | `out/portal2-coop/<flavor>` |
| `portal2-retail.json` (nothing to build) | `portal2-retail` | none |
| `fstop-linux-native-vulkan.json` | `fstop` | `out/fstop-linux-native-vulkan/<flavor>` |
| `dedicated-linux.json` | `dedicated` | `out/dedicated-linux/<flavor>` |
| `tools-linux.json` | `tools` | `out/tools-linux/<flavor>` |
| `hammer.json` (extends the v1 GTK Hammer profile) | `hammer` | `out/hammer/<flavor>` |

Decisions made here (agent, under the user's standing instruction to pick
the long-term option and record it):

- **Schema strings.** v2 profiles say `source-product-profile/v2`,
  fragments `source-product-fragment/v2`. The v1 files keep their schema
  and are not edited: `tools/quality/product_profile.py`, the Android and
  Apple scripts read them as v1. A v2 profile `extends` a v1 file where one
  exists, so pins are not copied twice; v1 families resolve for
  inspection (`kiln profiles resolve`) but are never buildable.
- **`extends` is a string or a list**, parents merged left to right, then
  the child. `profile_extends.py` learned the list form too, with cycle
  detection, so it stays an independent oracle for v2 files until L1
  deletes it.
- **Validation.** Unknown top-level members and unknown keys in `target`,
  `toolchain`, `build`, `pipeline`, `package`, `deploy`, `launch` and
  `content` are refused, naming the file that set them. Configure options
  are lower_snake_case Waf options (`true` is the bare flag, `false` omits
  it). Conflicts refused: options kiln owns (`out`, `prefix`, ...), more
  than one of `dedicated`/`tests`/`tools`, and a flavor that changes a base
  option (a flavor only adds). A buildable profile pins its toolchain
  family, version and compiler, and every pkg-config dependency.
- **The workspace file** may set `default_profile`, `default_map`,
  `resolution`, `windowed`, `frame_cap`, `content_locations`, `devices`
  and per-profile `launch` values (not `switches`); anything else is
  refused by name as `workspace-build-fact`.
- **Content sections** (mount sets, locators) are left to L1, which owns
  the `linux-dir` packager and content locators. Launch sections carry the
  game, default map and arguments of today's launchers as data; L1's
  launch-equivalence check compares them. `launch.display_session` and
  `launch.run` are not set yet, because their providers arrive in L1 and
  an unknown provider name is refused.
- **Trees and locks.** `out/<profile>/<flavor>/`, with
  `WAFLOCK=.lock-waf-kiln` and `NO_LOCK_IN_TOP`/`NO_LOCK_IN_RUN`, so the
  lock exists only inside the tree; builds run Waf from the tree.
- **Sources live in `product/kiln/`**, not `kiln/`: the repository root
  holds the `./kiln` bootstrap file.
- **Change detection** uses FNV-1a 64 digests (profile, toolchain identity,
  manifests, recipe keys): local identity, not a security hash.
- **Host tag** (`linux-x86_64`) comes from `uname -s -m` through the
  process contract, so `kiln.composition` needs no native header.

### The gate

| L0 gate item | Evidence | Result |
| --- | --- | --- |
| `kiln profiles resolve` equals `profile_extends.py` on every existing profile | `kiln.profiles.parity`: every file under `quality/product_profiles` (16 pre-RFC profiles, 11 v2 profiles, 3 fragments), values and member order | pass, 31 checks |
| Seeded faults fail: an `extends` cycle, an unknown key, an unknown provider name, a missing pin, a conflicting configure option, a workspace value that would change a build fact | `kiln.l0` (`fault.*`): 2 cycles, 2 unknown keys, 3 unknown providers, 2 missing pins, 3 conflicting options, 3 workspace build facts, plus an unknown switch conflict | each refused by its code and key |
| Every contract's bad providers are rejected | `kiln.l0`: 25 bad providers (toolchain 4, recipe builder 2, stage 4, packager 5, transport 6, display session 4), each by the clause it breaks; the real and fake providers pass the same suites | pass |
| The fixture platform runs end to end through an unchanged `kiln.core` | `kiln.l0` (`fixture.*`): build -> content -> package -> deploy -> run, the compiled program runs on the fake device with the display session's environment; an unchanged second run rebuilds and resends nothing | pass |
| The dependency rules for the core and the thin application, and the platform-literal scan, catch their seeded violations | archlint `KilnContractTest` over the real rows: core -> provider (CAP011 rule 3, and rule 6), application -> `product.profile` and -> a provider (rule 6, rule 3), `"Android"` in `kiln.core` (rule 7); comments, identifiers and providers are not flagged | pass, 10 tests |
| A second `kiln build` of an unchanged profile neither reconfigures nor rebuilds | `kiln.rebuild.portal2` and `kiln_gate.py rebuild` for every desktop profile (table below) | see below |

Other core checks in `kiln.l0`: a missing transport capability is refused
by name before the device is touched; deploy without a device is refused;
a stage that reads an undeclared artifact fails the run and publishes
nothing; an unknown provider fails by name; a fragment is not buildable;
cancellation stops the run; staging is cleared; the evidence record carries
the revision, dirty digest, profile digest, toolchain identity and
reproduction command.

`kiln.l0`: 108 checks, 0 failures, with g++ 16.2.1 (`out/host-tests`) and
clang++ 22.1.8 (`out/host-tests-clang`).

### Desktop builds

The first real build found a pre-existing defect: `shaderapivulkantest` and
`shaderextensiontest` both compile `render_backend_conformance.cpp`, and
Waf's per-directory indices gave both objects the same name, so every build
of a native Vulkan client tree recompiled it twice. The
`legacy_render_provider_conformance` target now takes a unique `idx`.

`kiln_gate.py rebuild <profile>` on every buildable desktop profile, after
the final source and pin changes (each first run reconfigured once because
a wscript or pin changed, then the second run was a complete no-op: no
configure, 0 Waf tasks, 0 installed files, evidence written; 13 checks
each):

| Profile | First full build through kiln | Second build |
| --- | --- | --- |
| `portal2` | 3,128 Waf tasks | no-op |
| `portal` | built (212 s) | no-op |
| `portal2-fsr` | 3,130 Waf tasks | no-op |
| `portal2-high` | built | no-op |
| `portal2-coop` | built | no-op |
| `fstop` | built after fix 2 below | no-op |
| `dedicated` | built after fix 3 below | no-op |
| `tools` | built | no-op |
| `hammer` | built | no-op |

`portal2-retail` declares nothing to build. No game was launched: launching
is L1.

Defects the real builds found, fixed in this change:

1. The duplicate object name above (`unittests/shaderextensiontest/wscript`).
2. `fstop` did not compile at HEAD: `game/server/gameinterface.cpp` still
   passed the factory to `BlobNetworkBypass_Connect`, which now takes the
   `ISPSharedMemoryManager` (the client was already updated); the server
   now asks the factory as the client does.
3. `dedicated` did not link at HEAD: `CStaticPropMgr::DrawStaticProps`
   called the render core's capture hooks, which the dedicated product
   does not build; the call is under `#ifndef SWDS`, as the file's other
   client-only code is.
4. The SDL3 pkg-config pin moved from 3.4.16 to the host's 3.4.18 (user
   decision, 2026-10-07: "you can change the pin") in the Linux desktop
   profiles, the fragment, the profile README and `test_product_profile.py`.
   The mobile profiles' SDL3 source-archive pins are separate and unchanged.
5. The root wscript installed the DXVK library by absolute path string,
   which Waf's `install_files` resolves relative to the source tree. Fixed
   on `kiln-l0`, then superseded when merged onto `subsystem-refactor`,
   which deleted DXVK Native (`0f9484583`, user decision 2026-10-07); the
   `portal-dxvk` profile was built and gated on `kiln-l0` and removed in
   the merge with its v1 parent. Fix 3 is the same change as the branch's
   `c138bb967`.

### Not done or unavailable

- **Providers that L1 and later add:** recipe builders (only the fake
  builder exists), packagers and transports (only the fixture platform's),
  display sessions and run providers. Their contracts and shared suites are
  in place.
- **`waf-engine` in the shared stage suite** runs only the descriptor and
  cancellation clauses; its success path is a full engine build, judged by
  the rebuild gate on real profiles.
- **Host prerequisites of the bootstrap:** the host tree is the tools
  product, whose configure needs the pinned shader toolchain
  (`python3 tools/render/shader_toolchain.py build`). In this worktree
  `dependencies/shader-toolchain`, `dependencies/vulkan-memory-allocator`
  are links to the main checkout's 
  and the submodules are links marked `assume-unchanged`.
- **No hosted CI run.** The manifest rows are local evidence.
- **No `kiln deps`, `content`, `package`, `deploy`, `play`, `run`,
  `switches` or `test` commands** in the CLI yet: the library runs those
  roles (the fixture platform proves it), and L1 adds the commands with the
  real providers.

### Reproduce

```sh
./kiln profiles list
./kiln profiles explain portal2
./kiln doctor portal2
./kiln build portal2                       # out/portal2-linux-native-vulkan/dev
python3 tools/kiln/kiln_gate.py parity
python3 tools/kiln/kiln_gate.py rebuild portal2
python3 tools/kiln/kiln_gate.py selftest
WAFLOCK=.lock-waf-kiln NO_LOCK_IN_TOP=1 NO_LOCK_IN_RUN=1 \
    python3 ./waf configure --tests -T release -o out/host-tests --disable-warns
(cd out/host-tests && WAFLOCK=.lock-waf-kiln python3 ../../waf build --targets=kilntest)
python3 tools/quality/conformance.py check --suite kiln.l0 --suite kiln.profiles.parity \
    --suite kiln.profiles.parity.sensitivity --suite kiln.rebuild.portal2 --suite hammer.adapters.mcp
python3 -m unittest discover -s tools/archlint/tests
```

## The 3DS client on kiln (2026-10-07)

User goal: "hook your build system into kiln" (the RFC 0026 3DS client,
built until now by an untracked `build-3ds.sh`).

- `product.toolchain.n3ds` (`public/product/toolchain_n3ds.h`,
  `product/toolchain/n3ds.cpp`): the `n3ds-devkitarm` ITargetToolchain,
  composed by `kiln.composition`. It extracts devkitPro once from the
  container image the profile pins by digest
  (`dependencies.container_image`; a tag alone is refused as
  `missing-pin`), verifies `toolchain.version` against
  `arm-none-eabi-g++`, cross-builds each pinned archive of
  `dependencies.archives` (sha256-checked, CMake with devkitPro's 3DS
  toolchain file, DESTDIR staging and one rename) under
  `dependencies/n3ds-devkitarm/<name>-<key>`, the key covering the archive,
  its options and the toolchain identity, and returns `--n3ds` plus the
  pkg-config environment. Until kiln.core runs recipe builders these
  archives are the toolchain's sysroot (agent decision).
- `waf-engine` now puts the toolchain's `wafOptions` (the cross target)
  first in the configure arguments and its digest; host toolchains return
  none, so desktop trees are unchanged.
- `quality/product_profiles/portal2-3ds.json` (`portal2-3ds`,
  `portal2@3ds`): the facts `build-3ds.sh` hard-coded, as data.
- `platform_posix` (loader and process providers) is not declared for the
  3DS, which has neither; it was the only target failing a full
  `waf install` of the 3DS tree.
- The harness reads the tree from one owner, `tools/n3ds/n3ds_tree.py`
  (`out/portal2-3ds/<flavor>`); `n3ds.py build` runs
  `./kiln build portal2-3ds`; `package_cia.sh` takes the ELF as its
  argument.

Evidence: `./kiln doctor portal2-3ds` all present; the first
`./kiln build portal2-3ds` extracted devkitARM, cross-built SDL3 3.4.16,
configured and built the tree; the second build was a complete no-op
(`waf-engine: up to date`); the kiln-built ELF, packaged as the CXI, boots
`sp_a1_intro4` headless on Azahar (verdict ok, 0 refused draws, 0 corrupted
textures); `kiln_gate.py parity` 0 failures. Open: an `IPackager` for the
3DSX/CXI/CIA forms and a transport for Azahar and the console (L1's
contracts), and the harness's other tools still read the old
`dependencies/3ds/devkitpro` copy.

## L1: desktop cutover (done 2026-10-08)

User direction: "Continue on L1." Worked in slices on `kiln-l0` in
`../source-engine-kiln`; nothing was deleted until the whole launch-
equivalence gate (argv, environment and staged-runtime manifest) passed
(76/0, 2026-10-08), and L1e then deleted the retired surface.

| Slice | Scope | State |
| --- | --- | --- |
| L1a | launch model, `.kiln/local.json` bindings, `kiln play`/`run --dry-run`, `kiln switches`, argv/environment equivalence | done (below) |
| L1b | `linux-dir` packager, mount sets and content locators, `video.av1`, manifest equivalence, real `kiln play` | done (below) |
| L1c | `coop-pair` and `external-install` run providers, `user`/`private`/`none` display sessions | done (below) |
| L1d | `sepipe` and `play_embedded`; the 35 harness modules and the CI workflows | done (below) |
| L1e | AGENTS.md and memory notes, the deletions, the root allowlist, the no-callers scan | done (below) |

### L1a: launch model and argv/environment equivalence

- **Launch as data.** `launch.variables` are named argument lists;
  `launch.arguments` and `launch.map_arguments` are templates where an
  element `{name}` splices a variable, `{switches}` the selected switches'
  arguments in request order, `{args}` the `--` tail and `{map_arguments}`
  the map arguments when a map is given; `{root}`, `{runtime}` and `{game}`
  are single values, and `{inherit}` in an environment value is the
  inherited value. Switches append `arguments` and `set` variables; a
  switch setting an undeclared variable, an unknown or conflicting switch,
  a repeated switch and an unknown template variable are refused by name.
  The workspace binds `resolution`, `windowed`, `frame_cap`, `default_map`
  and per-profile `launch.variables`.
- **Why variables:** `render_flags.sh` keeps state (core world, the
  indirect producer, runtime direct light, area lights) and emits it at a
  fixed place after the switches' own arguments, and each launcher puts the
  user's arguments and `--null` somewhere else. Appending alone could not
  reproduce them.
- **Profiles:** the render-switches and desktop-launch fragments are now
  variables and switches; Portal, Portal 2, Portal 2 FSR and F-Stop carry
  their launchers' exact templates and environments (F-Stop takes no render
  switches, as `./play_fstop`). `null` and `bink` are per game, because the
  launchers place them differently.
- **Commands:** `kiln play <profile> [map] [--set <switch>]... [--flavor f]
  [--dry-run] [-- args]`, `kiln run` (the same), `kiln switches <profile>`.
  Without `--dry-run`, play and run refuse until L1b's packager exists.
- **Tree layout** (changed for L1b): the Waf tree and its lock are now
  `out/<profile>/<flavor>/build/`, installing into `.../install/`, beside
  `runtime/`, `artifacts/` and `kiln-evidence/`. The old staging scans a
  whole build tree for the launcher and found Waf's install copy too.

Evidence (`kiln.launch-equivalence`, `tools/kiln/launch_equivalence.py`):
12 modes, each run through the real old launcher (staging included) until
it exec()s the game, captured by an `LD_PRELOAD` shim
(`tools/kiln/exec_capture.c`) and compared with `kiln play --dry-run`;
argv, launcher-set environment and working directory are equal in every
mode. Recorded differences: `BUILD_DIR`, `RENDERER`, `RUNTIME`,
`BASE_RUNTIME` and `MAP` (`./play` -> `run.sh` plumbing) and `CCACHE_DIR`
(exported by `launcher_ccache.sh` for the build; the compiler cache moves
to the engine stage). Seeded: a dropped switch, a swapped order of two
argument switches and an extra map are caught. 39 checks, 0 failures.
`kiln.l0` grows to 119 checks (11 launch-plan checks). The old launchers
build through Waf lock aliases into kiln's trees and stage into scratch
runtimes under `out/launch-equivalence/`; nothing in `run/` changes.

Not yet covered: `./play_p2 --release` (the release trees are not built
yet), `--workshop`, `--retail`, `./play_p2_coop`, and every manifest
comparison (L1b, L1c).

### L1b (in progress): the `linux-dir` packager

- **`product.package.linux-dir`** (`public/product/package_linux_dir.h`):
  the runtime is `out/<profile>/<flavor>/runtime`, a persistent directory
  the game also writes into. The packager owns only the entries it places
  (recorded in `.kiln-package.json`), replacing each through a temporary
  name and a rename; the game's own files are never touched. Steps are
  profile data: `seed` (once, from a located base: immutable asset suffixes
  linked, the rest private copies), `overlay` (an artifact's files by
  glob), `remove-elf32`, `mount-each` (published maps), `link` (located
  directories; optional links are removed when their content is absent),
  `search-paths` (the gameinfo block, written with LF newlines as the
  Python staging did) and `extract` (one VPK entry).
- **Content locators** (`content.locators`): named directories with
  defaults over `{root}` and `{home}`, overridden by `.kiln/local.json`'s
  `content_locations`. A missing optional locator removes what it mounted.
- **`content.vpk`**: the dependency-free VPK reader moved out of
  `hammer.formats` (history kept) behind a small `IByteSource`; Hammer's
  `hammer::formats::VpkArchive` is now an adapter over it with the same API.
  `hammer.formats.vpk_archive` (30 checks), its sensitivity row (12) and
  `hammer.adapters.mcp` (90) pass unchanged.
- **Commands:** `kiln package <profile>` (build, then lay out the runtime)
  and a real `kiln play` (package, then exec the plan through the new POSIX
  process entry `platform::ExecReplacingProcess`); `kiln run` execs what is
  staged.
- **Evidence:** `kiln.launch-equivalence` now also compares the old and the
  kiln runtimes entry by entry (files by SHA-256, links by resolved target):
  Portal (1,686 entries), Portal 2 and Portal 2 FSR are equal, and a changed
  byte in a runtime file is caught. 10 modes, 39 checks, 0 failures.

- **F-Stop:** `product.stage.fstop` (`fstop-content`, role `content`) is
  a C++ port of `stage_fstop_runtime.py`'s content assembly: the gameinfo
  derived from Portal's, the lower-case `fstop_valve*` mirrors of depot 852
  (Python's component-wise path order decides case duplicates), the merged
  sound-script, particle, HUD-layout and UTF-16 localization files, the
  authored scripts and the Portal 2 shop-door model with its materials and
  textures, read through a small search-path resolver over `content.vpk`.
  Its patterns are hand-written matchers (strict modules take no regex
  header). The packager overlays its artifact, links kept as links. The
  runtime equals a freshly staged Python runtime: 14,000 entries.
  Recorded defect not carried over: the Python stager appends its F-Stop
  lines to the sound and particle manifests again on every re-staging (its
  resolver finds its own previous output first); kiln always produces the
  fresh result, and the gate compares against a fresh Python staging.
- The packager now keeps its ownership record when a later step fails, so
  a seed that finished is not lost to the next run.

Evidence: `kiln.launch-equivalence`, 12 modes, 47 checks, 0 failures:
argv, environment and working directory in every mode; four runtimes
(Portal 1,686 entries, Portal 2, Portal 2 FSR, F-Stop 14,000) equal; seeded
faults (a changed runtime byte per runtime, a dropped switch, a swapped
switch order, an extra map) caught.

- **Mount sets:** `content.mount_sets` names package-time variants; `kiln
  play|package --mounts <set>` selects them, and steps or gameinfo lines
  tagged `mount_set` run only when selected. An undeclared set is refused
  by name. The P2:CE Workshop set is the `extract-packs` op, a port of
  `stage_workshop`: include/exclude selection, whole-model ownership, the
  VPHY 0x0100 refusal, dropped VMT keys, namespace moves of VMT texture
  references and MDL `$cdmaterials`, per-pack stamps (sizes and
  nanosecond times of the archives) and `mounts.json`, written as Python's
  `json.dumps(indent=1)`. Its runtime equals `./play_p2 --workshop`'s:
  2,972 entries.
- **`video.av1`:** `product.stage.video-av1` (`video-av1`, role `content`)
  ports `transcode_av1.py`: the CRF ladder, the frame/size/audio/SSIM
  gates, and `manifest.json` (sorted keys, the same per-clip key), driving
  ffmpeg and ffprobe through the process provider one clip at a time (the
  provider runs one process at a time; SVT-AV1 threads each encode). It
  agrees that all 59 retail clips are current in the shared cache, and on a
  scratch clip both implementations write identical manifests; the WebM
  bytes differ in 16 bytes exactly as two Python runs do (the muxer's random
  segment UID), so the stage declares statistical determinism. A locator
  may be `"create": true`, a writable cache that need not exist yet.
- **`kiln content <profile>`** runs the pipeline up to the content stages.
- **Fixed on the way:**
  - `kiln.core`'s Waf stage watches `quality/product_profiles` as
    `ensure_configured.py` does, and its configure stamp records a digest
    of Waf's `c4che`, so a tree reconfigured outside kiln (the old
    launchers' `ensure_configured.py` did that to kiln's trees and dropped
    their install prefix) is reconfigured.
  - A dangling reference in the Waf stage (an argument list held across
    later `Value::Set` calls) wrote garbage into the configure stamp's
    informational `arguments` and could throw `std::bad_alloc`; the same
    pattern in `extract-packs` is fixed too, and `foundation/json.h` now
    states that `Set`/`Push` references end at the next `Set`/`Push`.
  - A dangling temporary in the AV1 stage's range-for (C++20 does not
    extend it) crashed the first encode.

Evidence at the end of L1b: `kiln.launch-equivalence` 13 modes, 52/0
(argv, environment, working directory; five runtimes equal: Portal,
Portal 2, Portal 2 + Workshop, FSR, F-Stop; seeded faults caught);
`kiln.l0` 122/0; archlint and its 176 tests pass; style clean.

### L1c: run providers and display sessions

- **Contracts.** `IDisplaySession::Open` takes a `DisplayRequest`
  (a scratch directory it owns, the virtual monitor) and returns, beside
  environment changes, a command prefix that wraps the launch. A new
  `IRunProvider` contract (`product.contracts`) takes the launches, the
  display environment, `launch.facts` and a spawner, and returns the run's
  exit status. Decision (agent, recorded here): the RFC calls coop pairs
  and the retail launch run-role stage providers that need no new
  contract, but a stage has no way to receive the launch plans or start
  long-running programs, so a narrow run contract is the smaller seam.
  `platform.process-spawn.v1` (`public/platform/contracts/process_spawn.h`)
  starts long-running programs in their own process group, with a POSIX
  provider in `platform.posix`; the synchronous tool-process contract stays
  for tools whose output the caller collects.
- **Display sessions** (`product.display.desktop`): `user` (nothing
  changes), `none` (SDL offscreen, displays unset), `private` (a headless
  mutter on a private D-Bus that shadows the Flatpak document portal, as
  `private_session.py` does). `kiln play|run --display <session>`
  overrides the profile's session.
- **Run providers** (`product.run.desktop`, a native backend module):
  `single`, `external-install`, and `coop-pair`: the host first, its server
  probed with the engine's challenge, `{lan_address}` filled from the
  route's source address, the client next, the host's `engine.log` watched
  for a remote join, then both waited on (or stopped after the join with
  the `until-joined` switch). `kiln play` still execs a single launch in
  the user's own session in place, as the old launchers did; anything else
  goes through `Session::Launch`.
- **Profiles:** `launch.peers` (per-peer argument templates in the base
  template's `{args}` place), `launch.facts`, `launch.working_directory`
  and executables over `{locator:<name>}`. `portal2-coop` carries
  `play_p2_coop`'s peers; `portal2-retail` runs Steam's `portal2.sh` from
  the repository root.
- **Fixed on the way:** the `--` tail was spliced through placeholder
  substitution (a game argument containing braces would have been
  rewritten), and the app's global option loop read past `--`, so a game's
  `-h 720` printed the help.

Evidence:
- `kiln.launch-equivalence`: `./play_p2 --retail` equal; `./play_p2_coop`
  against a stub `play_p2` (which answers the challenge and writes the
  join line): each peer's arguments, through Portal 2's proven launch,
  equal kiln's peers, with start order and the client's `P2_NO_BUILD`;
  a seeded difference is caught.
- `kiln.coop-live`: a real pair, headless (`--display none --set null
  --set until-joined`): the host answered at its LAN address, the client
  joined over the LAN (`Client "…" connected (10.0.0.203:27006)` in the
  host's `engine.log`), and both stopped. No window reached the desktop.
- `kiln.l0`: 132 checks, including the display suites for `user`, `none`
  and `private`, a run-provider suite (exit status, waiting, prefix and
  environment, cancellation, refusal) for `single` and `external-install`,
  and two bad run providers (returns before its program ends; ignores
  cancellation) rejected by their clauses.

### L1d (first slice): `sepipe` and the `play_embedded` sample

- **Pin.** nanobind 2.9.2 is pinned by source archive in
  `quality/toolchain/nanobind.json`: the PyPI sdist, sha256 `e7608472…`,
  which bundles `robin_map`. There are 84 files, each with its own digest.
  - `tools/quality/source_pin.py` is now the one owner of pinned source
    archives (fetch, verify, HTTPS only, bounded by the pinned size).
  - `tools/render/vma_pin.py` delegates to it, and its `check` still passes.
- **Build.**
  - The tools profile pins the host Python with `sepipe_python: "3.14"`, which
    becomes `--sepipe-python=3.14`.
  - `product/kiln/python/wscript` finds `python3.14`, checks its version and
    `Python.h`, and verifies the pin. It fails configure by name otherwise.
  - It builds `nanobind` (`nb_combined.cpp`) and
    `sepipe.cpython-314-x86_64-linux-gnu.so` in the tools tree only.
  - Only this one link drops `--no-undefined`, because the Python C API
    resolves against the importing interpreter.
- **Module.** `kiln.python` is a backend-kind module, so it may include the
  Python and nanobind headers.
  - `sepipe.Session(root, diagnostics=None)` offers `profiles`, `resolve`,
    `switches`, `doctor`, `build(up_to=engine|content|package)`, `plan`, `play`
    and `run`.
  - Results are the `kiln --json` documents.
  - Failures raise `sepipe.KilnError("<code>: <detail>")`.
  - The GIL is released during builds and launches.
- **Policy lifted out of the app, so `kiln` and `sepipe` share one owner each:**
  - `kiln::DefaultSessionConfig` (in `kiln.core`; it names no provider);
  - `Session::BuildForPlay` (play builds through the package stage);
  - `ToJson(std::vector<ProfileSummary>)` (the `profiles list` document).
- **Sample.** `tools/samples/play_embedded` (`kiln.samples`, an adapter
  consumer of the product and kiln contracts) composes its own catalog:
  - the gcc toolchain, the Waf stage, the linux-dir packager, user and
    headless displays, and the single run provider;
  - no kiln app and no default composition;
  - it launches through `kiln.api` with a recording spawner (`--start` runs
    the program).
  - Its catalog is validated against the profile: Portal 2, which needs the
    AV1 stage, is refused by name.
- **Evidence.** `kiln.sepipe` (`tools/kiln/sepipe_gate.py`) passes 15/0
  through the runner. It checks:
  - `profiles`, `switches` and five plans equal to `kiln --json`;
  - `KilnError` for an unknown profile and an unknown switch;
  - seeded map and switch changes caught;
  - `play_embedded` spawning exactly the planned argv.
- **Rerun.** `kiln.coop-live` 4/0 through the runner (the L1c live pair).
- **Other checks.**
  - `kilntest` 132/0 on gcc and clang.
  - archlint self-tests 176 OK; no archlint finding from this change.
  - stylelint clean.
- **Pre-existing, not from L1.** `archlint check --all` reports 211 ARCH105
  findings, all in the tracked `games/csgo` and
  `external/portal2_steam2_decompiled` trees.
- **Release trees.** The release-flavor trees of `portal`, `portal2` and
  `portal2-fsr` build. Their equivalence modes are next.

### L1d (second slice): release equivalence and chosen runtimes

- **Release modes.** `launch_equivalence.py` adds `play_release`,
  `play_p2_release` and `play_p2_fsr_release`, each against kiln's release
  tree (`--flavor release`).
  - `./play_release` hard-codes `build-release`, so the check links that
    name to kiln's tree for the run.
  - One recorded environment difference: `PLAY_BUILD_DIR`, the wrapper's
    tree name. No engine source reads it.
- **Evidence.** The full suite passes 76/0: every launcher mode, runtime
  manifests (including release), the live co-op pair and the seeded faults
  (`kiln.launch-equivalence` minimum raised from 61 to 76).
- **Chosen runtime.** `PipelineRequest::runtime` and `PlayRequest::runtime`
  (CLI `--runtime <dir>`, `sepipe` `runtime=`) package into and launch from
  a caller's directory with the same packager and steps. This is how
  harnesses get a private runtime.
  - `./kiln package portal --runtime <dir>` takes 2.3 s.
  - `ResolvedProfile::PackageDirectoryName()` is now the one owner of the
    package directory. The plan had hard-coded `<tree>/runtime` while the
    package stage read `package.directory`.
  - `kilntest` 134/0 on gcc and clang, with `fixture.package-into-a-chosen-runtime`
    and `launch.chosen-runtime-is-the-working-directory-and-{runtime}`.
- **Harness survey.** 34 modules import scripts that L1 deletes:
  `private_session` 11, `stage_portal2_runtime` 15, `profile_extends` 9,
  `stage_runtime` 3, `stage_fstop_runtime` 3, `ensure_configured` 1 and
  `transcode_av1` 1.
  - Three tools exec a launcher directly: `frame_floor.py`, `demo_frames.py`
    and `term_sweep.py`.
  - The rest of the 40 files that mention `./play*` print it as a usage hint.
- **Open.** The tree's `kiln-evidence/package.json` is overwritten by a
  chosen-runtime package. It should record the output directory or use its
  own name.

### L1d (third slice): harness runs through `kiln.api`; `frame_floor.py` migrated

- **Contract additions.**
  - `SpawnRequest::outputFile` and `LaunchSpec::outputFile`: the program's
    output and errors go to a file. The POSIX spawner opens it before fork
    and `dup2`s it.
  - `PlayRequest::log`: a co-op peer's log is `<log>.<peer>`.
  - `PlayRequest::displayMode`: `Launch` never set the private compositor's
    mode before.
  - A chosen runtime keeps its display-session scratch beside it.
- **Shared run suite.** New clause N6: "the program's output and errors go
  to the launch's log". Every real run provider passes it, and a new bad
  provider (`kDropsLog`) is rejected on N6.
- **kilntest.** 135/0 on gcc and clang.
- **sepipe.** `plan`, `play` and `run` take keyword options (`map`,
  `switches`, `arguments`, `flavor`, `display`, `mounts`, `device`,
  `runtime`, `log`, `display_mode=(w, h, hz)`, `cancel`). An unknown option
  is refused by name. `sepipe.Cancellation` stops a run from any thread.
  - End to end: headless Portal in a private runtime, cancelled after 15 s,
    returned 130 in 15.3 s. The log held the engine output and no process
    was left.
- **Loader.** `tools/kiln/sepipe_loader.py` gives harnesses one way to import
  `sepipe` (it builds the tools tree once when the module is missing).
  `sepipe_loader.Run` is a run request on a thread with `poll()` and
  `stop()`.
- **`frame_floor.py`** (an L1 gate harness) no longer calls
  `stage_portal2_runtime.py`, `./play_p2` or `private_session`.
  - It packages its private runtime with `build(up_to="package",
    runtime=…)`.
  - It runs through `run(display="private"|"none", display_mode=…, log=…)` on
    a thread and stops the run by cancellation.
  - `--build` and `--steam-root` become `--kiln-profile`/`--flavor` (the
    workspace owns content locations), and `--render-switch=--x` becomes
    `--set x`. The evidence records the run's options and its plan.
  - Process-group cleanup is now the run provider's (clause N4). The
    harness's test checks that stopping cancels and waits.
  - 22 of 23 self-tests pass. `test_disabled_effect_missing_query_and_wrong_gpu_fail`
    fails identically on HEAD's unmodified copy, so it predates this change.
- **Real run on this host.** A scratch copy of the workload on
  `sp_a1_intro4_relit` packaged 2,803 files. It ran in a private 1920×1080
  compositor and measured 2,934 frames over 20.1 s, and the run stopped
  cleanly.
  - The verdict fails on performance. That is expected here: verdicts belong
    on the bazzite box.
- **Content gap, not from L1.** The workload's map `sp_a1_intro4_probe64` is
  no longer in the published map store (`run/maps` holds `_fast`, `_relit`
  and `_relit_v7`). The selected High workload cannot run until it is
  republished, with the old harness as well as the new one.

### L1d (fourth slice): display precedence, harness launch options, `portal_boot.py` on kiln

- **Fixed: the display session now owns its variables.** Run providers
  applied the display session's environment before the launch's, so the
  Portal profile's player default `SDL_VIDEODRIVER=wayland` overrode a
  headless (`none`) session's offscreen driver. The display's variables are
  now applied last.
  - New run-suite clause N7: "the display session's environment wins over
    the launch's".
  - Bad provider `kLaunchOverridesDisplay` is rejected on N7.
- **Harness launch options on `PlayRequest`** (`sepipe` keyword options in
  brackets):
  - `exactArguments` (`exact_arguments`): a harness's test command follows
    the profile's executable in place of `launch.arguments`. Switches, a map,
    request arguments and peer profiles are refused by name.
  - `environment` (`environment={name: value | None}`): variables applied
    after the profile's.
  - `wrapper`: a capture tool inside the display session.
  - `started` (`started=callable(name, pid)`): each program's start with its
    process. The run request carries it, under new run-suite clause N8.
    Bad provider `kSilentStart` is rejected on N8.
  - The profile still owns the program, working directory, environment,
    display session and run provider.
- **kilntest.** 139/0 on gcc and clang.
- **`portal_boot.py --profile <p> [--flavor f]`.** kiln packages the profile
  into the boot's private runtime and the game runs through `kiln.api`.
  - The boot's own test command, sandbox variables and RenderDoc wrapper are
    passed as the options above.
  - Mapped-library checks (`--require-vulkan|sdl3|wayland`) watch the
    reported process.
  - The legacy `--runtime`/`--build` path stays until its callers move (about
    40 tools and 28 manifest rows), then is deleted with the L1 deletions.
- **Equivalence.** The kiln path and the legacy path with the same kiln-built
  binaries give the same command, with `testchmb_a_00` headless on native
  Vulkan.
- **Found, not from L1.** Both paths currently fail the boot oracle the same
  way. The capture is an almost uniform grey (RGB 211, 15 colours), and the
  engine logs "Window presentation change failed." That points at the
  presentation work of the legacy device facade slices (F2–F4) on this
  branch. It is left to that work's owner, and the `portal_boot` gate item
  stays open until a boot renders.
- **User decision, 2026-10-07: the grey `portal_boot` capture is unrelated
  to L1.** The L1 `portal_boot` gate item is judged on equivalence: the kiln
  path and the legacy path give the same command and the same outcome with
  the same binaries. The rendering failure stays with the device facade
  work.

### L1d (fifth slice): mount sets leave the runtime; the first `portal_boot` callers

- **Fixed: unselected mount sets now leave the runtime.** A `linux-dir`
  package run without a mount set used to skip that set's steps and keep
  what they had placed. After the equivalence check's
  `--mounts p2ce-workshop` mode, the tree's Portal 2 runtime kept 46 Workshop
  entries.
  - Every placed entry is now owned through one `Own()` with the running
    step's mount set, persisted in the record as `mount_set`.
  - Skipping a set's step removes its entries and the directories they
    emptied.
  - `extract-packs` re-owns unchanged packs and owns each pack's stamp.
  - Result: 2,852 entries with the set and 2,803 without, with nothing left
    over.
- **Oracle.** `package.linux-dir-unselected-mount-set-entries-removed` is the
  first kilntest check that runs the `linux-dir` packager itself. A seeded
  `RemoveSet` defect fails it. kilntest is 140/0 on gcc and clang.
- **Equivalence check.** It had passed only because the old
  `stage_portal2_runtime.py` keeps the same leftovers: without `--workshop`
  it drops the search paths but leaves the packs on disk. The Portal 2
  reference runtimes are now one per mount selection, and that old defect is
  recorded rather than carried over. The Portal 2 modes pass 16/0.
- **Callers.** `sepipe_loader` gains `session()`, `packaged_runtime(profile,
  flavor)` (a profile's package used as a content source),
  `add_arguments()` and `boot_arguments()`.
  - `fizzler_light.py` and `fizzler_door_light.py` take `--profile`/`--flavor`
    and boot through `portal_boot.py --profile`. Their capture fails only on
    the approved, unrelated grey boot frame.
  - Their map compile still reads the map pipeline's
    `build/toolchains/pbrt-map-toolchain.json` (Blender, OpenUSD, KTX, xatlas
    and the compile tools). Moving that pipeline is later RFC 0027 work, not
    L1.

### L1d (sixth slice): the map pipeline boots its kiln client profile

- **Toolchain record.** `pbrt-map-linux-tools.json` names the boot client as
  a kiln profile (`runtime.client_profile` `portal`, `client_flavor` `dev`),
  in place of `run/runtime-native` and a legacy client tree.
  - `pbrt_map_toolchain.load()` resolves `runtime` (the profile's package, a
    content source) and `model_tool` (the engine install's
    `mdl_mesh_export`) through `sepipe_loader`. `client_build` is gone.
  - `add_client_arguments()` and `boot_target()` own the default and the
    `--profile`/`--flavor` override.
- **Callers on `portal_boot.py --profile`.** `pbrt_map_build.py` (boot,
  camera-boot and traversal-boot; the cache keys name the client profile),
  `gi_runtime.py` (its unused `--game` goes), `gi_soak.py`, `gi_swing.py`,
  `gi_temporal.py`, `gi_probes.py` and `reflection_runtime.py`.
- **Tests.** `load()` resolves on this host. `test_gi_tools` 48, the
  `pbrt_map_build_run` tests 14 and the toolchain boundary tests 16 pass.
- **Pre-existing failures, files not touched.** Three
  `test_reflection_probe_set` placement tests, and four lighting back-end
  plan tests (the `light-masks` operation).

### L1d (seventh slice): the remaining `portal_boot` callers and `corpus.hammer.ui` on the kiln tree

- **Callers on kiln profiles.** Booting through `portal_boot.py --profile`:
  host-frame baseline, legacy-ports and release views, spark scene, pedestal
  carousel, sign light, video frame cache, world-light route, game/lab
  matrix, dlight lab, the Portal 2 scenario family (scenarios, material
  shots, physics, storybeats, map views, audio), sign panel, monitors, mover
  shadow, view oracle, culling capture, core-world smoke, the render tools
  (skin corpus, layer dump, debug views, proxy corpus, map swipe, TSan
  triage, `rdc.py`), `vmf_map_build --boot`, the Hammer loop, intro4 strict
  game and particle census.
- **Shared owners in `sepipe_loader`:** `run_test` (a harness's test command
  through kiln, with timeout, early stop and process watch),
  `packaged_runtime`, `installed`, `game_of`, `add_arguments` and
  `boot_arguments`. `portal_boot`, `portal2_scenarios` and `core_world_smoke`
  dropped their own `Popen` loops.
- **New profile.** `portal-tsan-linux` (clang 22.1.8, `sanitize=thread`). The
  TSan tree is now profile data. Profile parity passes 30/0.
- **Hammer.** `hammer_gtk` and `hammer_cli` install into the `hammer`
  profile's install. `hammer_ui_test.py` takes the editor from
  `kiln build hammer` instead of `hammer/gtk/build.sh`. The Hammer rows use
  the kiln tools tree's `hammer_cli` and require kiln instead of
  `run/runtime`.
- **Driver fix.** The UI driver accepts "radio button" where it asks for a
  check box, because GTK reports a grouped check button that way. This is
  why the replace-textures cases failed before the change, whatever built
  the editor.
- **Evidence through kiln.**
  - `corpus.hammer.ui` 59/0 on the kiln-built editor, through the runner.
  - Hammer loop 14/0.
  - `sp_a1_intro5` scenario 9/9.
  - Pedestal carousel 14/0.
  - Core-world smoke 2/0 (self-test 21/0).
  - Host-frame captures equal the fixture (395 frames, 8,312 calls) in both
    modes.
  - View oracle: 16 identical failures on the kiln and legacy paths with the
    same binaries.
  - Sign panel: the same 10 failures as HEAD's tool on the legacy path.
- **Pre-existing, not from L1.**
  - The view-oracle `decals/rendershadow` drift against the frozen captures.
  - Ten sign-panel checks.
  - Three reflection-probe placement tests.
  - Four lighting back-end plan tests (`light-masks`).
  - One frame-floor test.
- **Not run in this slice.** The Wayland and scaled `corpus.hammer.ui` rows,
  and `corpus.hammer.mcp`.

### L1d (eighth slice): the last harnesses, display sessions and CI (2026-10-08)

- **Dedicated and USD runtimes.** `dedicated-linux` gains a `linux-dir`
  package and a launch. `bsp2_dedicated`, the BSP2 F1 gate and
  `usd_map_runtime` package their product into a private stage
  (`sepipe_loader.package_into`) instead of `portal_boot` staging; the F1
  gate takes `--dedicated-profile`/`--client-profile`. Dedicated and client
  BSP2 runs pass through kiln.
- **F-Stop.** `fstop_mechanics_check` packages the fstop profile and runs
  each scenario through `run_test` (mall_doors passes);
  `fstop_mechanics_map` and `fstop_puzzle_map` install into the fstop
  profile's packaged runtime and take `hammer_cli` from the hammer install.
- **Display sessions in `kiln.api`.** `Session::OpenDisplay`/`CloseDisplay`
  (sepipe `open_display`/`close_display`, `sepipe_loader.Display` and
  `run_under_display`) serve harnesses that run a whole session under a
  display. A `private-x11` session (headless mutter, SDL on its Xwayland)
  serves the 32-bit retail binary, Wine and RGP; the private socket is named
  from the scratch path so concurrent sessions never collide. kilntest
  142/0 on gcc and clang. Every `private_session` caller moved: the retail
  captures of audio, material shots, storybeats and physics (physics through
  the new `portal2-retail-mirror` profile as a kiln run request), map swipe,
  relight diagnostics, `d3d12_lane`, `rgp` and `hammer_ui_test` (properties
  on X11 9/0, visgroups on Wayland 13/0).
- **Demo playback.** `demo_frames` and `term_sweep` package their workload's
  profile (`portal2-fsr`) and play through kiln run requests; the intro4
  playback completes.
- **Mobile content.** The `portal2-content` profile (the portal2 package's
  content steps, no engine) replaces `stage_portal2_runtime.py
  --mount-custom` in the iOS/tvOS and Android Portal 2 scripts; file sets
  are identical (2764) apart from kiln's record and the AV1 mount.
- **Stale forwarding fixed.** The GI gates, paint, GI chamber, worldmesh
  oracle and resolution sweep still forwarded `--build` to tools that take
  profiles; `map_relight_diagnostics` had no client at all after the swipe
  tool moved. Tools that defaulted to the player's `run/runtime-p2` read the
  packaged runtime.
- **CI.** `fragments/linux-ci-ubuntu-24.04` pins the runner's GCC 13.3.0;
  `ci-dedicated-linux[-i386]`, `ci-tests-linux[-i386]` and
  `ci-toolchain-linux[-clang]` (Clang 18.1.3) carry each lane, and the
  build, tests and toolchain workflows run `./kiln build <profile>`. `kiln
  doctor` in an `ubuntu:24.04` container finds every pin. The dedicated lane
  fails there exactly as the legacy lane's own commands do
  (`public/tier0/threadtools.h:1173`, a template-id constructor GCC 13
  rejects in C++20; existing debt). The Windows lanes stay on `waf.bat`
  until kiln has a Windows host (MSVC runners are optional). Hosted CI not
  run.
- **Found on the way.** `.gitignore`'s bare `run/` had hidden
  `product/run/desktop.cpp`, so a clean checkout could not build kiln; the
  rule is anchored and the file committed. The host kiln now bootstraps with
  `--kiln-host` (kiln's projects only, no render core), so a clean
  `ubuntu:24.04` with g++, Python, pkg-config, bzip2 and zlib runs `./kiln`
  (before: the shader toolchain and DXC were required).
- **Manifest.** 40 suites that build through kiln no longer require legacy
  trees; Portal 2 content gates on `SOURCE_PORTAL2_STEAM_ROOT`.
  `launch_sandbox` also protects kiln's player runtimes.
- **A stale runtime record.** The first gate run of the day failed one check
  (play_p2's manifest had Workshop packs): the runtime's record predated
  6e3198091, so its Workshop entries carried no mount set and could not be
  removed. A clean package fixes it; the rerun passed 76/0. Only runtimes
  packaged before that commit (this unreleased worktree) are affected, so no
  record migration was added.

### L1e: the deletions, the root allowlist and the no-callers scan (2026-10-08)

- **Deleted** (fe92eb024), after the full launch-equivalence gate passed
  76/0: the launchers (`./play`, `./play_p2`, `./play_p2_fsr`, the three
  release wrappers, `./play_fstop`, `./play_p2_coop`, `run.sh`,
  `run.conf`); `render_flags.sh`, `launcher_ccache.sh`,
  `ensure_configured.py`, `profile_extends.py`; the three `stage_*.py`
  scripts and `portal_boot`'s legacy staging path (it now requires
  `--profile`); `private_session.py`; `tools/video/transcode_av1.py`; the
  four `scripts/*-ubuntu-*.sh` CI wrappers; and the launch-equivalence gate
  with its `exec_capture` shim, whose subject is gone. The stray root files
  the RFC lists were already absent on this branch.
- **Hammer's Build and Run** launches `./kiln play portal <map>`.
- **Root allowlist.** `tools/stylelint/root_files.py`, run by the style
  workflow: wscript, waf, waf.bat, kiln, the documentation and licence files,
  configuration and directories. Nine platform scripts are pending L7 in a
  shrink-only list (an entry whose file is gone fails). Five tests, with a
  seeded stray file and a stale pending entry.
- **No-callers scan.** `tools/kiln/retired_scan.py` (suites
  `kiln.l1.no-callers` 2/0 and `.selftest` 8/0): no retired path is tracked,
  and no tracked file outside RFC/ and Markdown names one except on a
  provenance line. Seeded callers (a launcher path join, an argv, an import,
  a CI step, Hammer's old argv, `run.conf`) are caught.
- **Docs.** AGENTS.md gains the kiln commands; READMEs, profiles, workloads
  and tool help name kiln.
- **Evidence.** tools/quality unit tests: 1707 run, no failure beyond the
  pre-change baseline's (25 known, listed in the seventh slice and the
  reflection-probe, lighting back-end, frame-floor and environment cases).
  Profile parity 39/0.
- **Not in L1, still open.** The Hammer build scripts (`hammer/gtk/build.sh`,
  `build_*_shell.sh`) keep callers (`viewport_smoke.sh`,
  `hammer_viewport_budget.py`, the Hammer profiles) and their own deletion
  row. The platform scripts are L7. The render_lab and r03 test trees are
  not launcher-driven. Retail captures were not rerun (no Steam client
  here); hosted CI has not run.


### Windows build and launch target on Linux: MSVC under Wine (2026-10-08, user direction)

User direction: "we should provide a wine toolchain that pulls in MSVC",
"like its a windows build target + launch target on linux", and "we should
also use it to build and test the dx12 backend".

- **Toolchain.** `tools/windows/msvc_wine.py provision|check|path` installs
  the pinned toolchain into `dependencies/windows-msvc-wine/14.44-10.0.26100/`
  from `fragments/windows-x86_64-msvc-wine.json`: the mstorsjo/msvc-wine
  commit and archive sha256, the Visual Studio 17.14.41 installer manifest
  by versioned URL and sha256 (vsdownload checks every payload against it),
  MSVC 17.14 (toolset 14.44, `cl` 19.44.35229, the hosted windows-2022
  runner's) and Windows SDK 10.0.26100, x64 and x86. Staged, stamped,
  renamed; a second run does nothing. It accepts the Build Tools licence on
  this machine and redistributes nothing.
- **Provider.** `product.toolchain.msvc-wine` (`windows-msvc-wine`) checks
  the install's stamp and `cl`'s banner against the pins, reports compiler,
  target (`x86_64-pc-windows-msvc`), SDK and runner, gives Waf
  `--msvc-wine=<install>` and downloads nothing. It passes the shared
  toolchain suite against a fixture install; the contract gains
  `ToolchainEnvironment::compilerProbe` (MSVC has no `--version`), and
  waf-engine now passes a toolchain's `wafOptions` to configure.
- **Waf.** `xcompile --msvc-wine` drives Waf's msvc tool without registry
  detection (the wrappers by absolute path, `/Z7` instead of `/Zi /FS`, which
  needs mspdbsrv); `masm` honours `AS`.
- **Profile and launch.** `dedicated-windows` builds the Portal dedicated
  server and packages a `windows-dir` runtime (the directory packager under
  its own form) that links only immutable assets; `launch.runner` (a new
  schema key, the host program found on PATH) runs it under Wine with the
  prefix in the tree and Wine's display drivers off.
- **Evidence.** `./kiln build dedicated-windows` (1,547 Waf tasks from
  scratch); `./kiln play dedicated-windows testchmb_a_00 -- +quit` exits 0
  and opens the map (24 opens of `testchmb_a_00.bsp` in a file trace);
  nothing is written into `run/runtime`. kilntest 146/0. The Windows
  defects the local build found are in the commit record (`b4439ef50`):
  C2375 declaration/definition linkage, case-sensitive source names, a
  static-initialization order crash in `CommandLine()`, the debug
  allocator's 8-byte blocks on 64-bit, and the redirected-stdin console.
  Hosted CI's Windows checkout also collided `iappsystem.h`/`IAppSystem.h`,
  `keyvalues.h`/`KeyValues.h` and `cegclientwrapper.h`/`CegClientWrapper.h`
  (lowercase forwarding shims overwrote their real headers); the shims are
  deleted, 70 files include the real names, and
  `tools/stylelint/case_collisions.py` (style workflow) refuses such pairs.
- **Open.** A client profile under Wine (it needs a display session and
  the D3D12 or Vulkan client path), x86 builds, a CI lane for the MSVC-Wine
  toolchain (a 3.5 GB install per run), and a Windows-native run.
