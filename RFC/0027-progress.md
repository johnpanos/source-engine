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

## L1: desktop cutover (in progress, 2026-10-07)

User direction: "Continue on L1." Worked in slices on `kiln-l0` in
`../source-engine-kiln`; nothing is deleted until the whole launch-
equivalence gate (argv, environment and staged-runtime manifest) passes.

| Slice | Scope | State |
| --- | --- | --- |
| L1a | launch model, `.kiln/local.json` bindings, `kiln play`/`run --dry-run`, `kiln switches`, argv/environment equivalence | done (below) |
| L1b | `linux-dir` packager, mount sets and content locators, `video.av1`, manifest equivalence, real `kiln play` | done (below) |
| L1c | `coop-pair` and `external-install` run providers, `user`/`private`/`none` display sessions | done (below) |
| L1d | `sepipe` and `play_embedded`; the 35 harness modules and the CI workflows | planned |
| L1e | AGENTS.md and memory notes, the deletions, the root allowlist | planned |

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
