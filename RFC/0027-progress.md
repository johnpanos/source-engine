# RFC 0027 progress: One Product Pipeline and `kiln`

Design: [RFC 0027](0027-product-pipeline-lowering-streaming-kiln.md).
Roadmap: the RFC's L0/L1/L7 row (named R98 in the RFC's roadmap table, an id
RFC 0026 also took for physics on 2026-10-07; the user ranks and renames the
row). Nothing in this record ranks it.

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
- `quality/product_profiles/`: three fragments and eleven schema v2 desktop
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
  as data (`--core-probe=MODE` became two switches; `--native` and
  `--dxvk` became profiles), each with a description, arguments and
  conflicts.
- `fragments/desktop-launch.json`: `ivp`, `serial-jobs`, `sync-queue` and
  `bink`, the environment variables of `run.conf` and `play_p2`.

Profiles (`source-product-profile/v2`), each with a host alias:

| File | Alias | Tree |
| --- | --- | --- |
| `portal-linux-native-vulkan.json` (extends the v1 slice) | `portal` | `out/portal-linux-native-vulkan/<flavor>` |
| `portal-linux-dxvk.json` (extends the v1 DXVK slice) | `portal-dxvk` | `out/portal-linux-dxvk/<flavor>` |
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
| `portal-dxvk` | 2,687 Waf tasks, after fixes 4 and 5 | no-op |

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
   which Waf's `install_files` resolves relative to the source tree; it now
   passes the node.

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
  are links to the main checkout's (the DXVK package and its archive are copies,
  because the wscript refuses a linked archive),
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
