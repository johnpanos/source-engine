# RFC 0027: One Product Pipeline: Canonical Codecs, Target Lowering, Streamable Packages and the `kiln` CLI

- Status: Proposed (2026-10-07, user direction); nothing implemented. No
  ranked roadmap row yet: proposed rows are under [Roadmap](#roadmap), and
  ranking is a user decision. Amended the same day (user direction): a
  breaking workflow change. `kiln` is the only way to build and play; the
  launchers, `run.conf` and the per-platform scripts are deleted rather than
  wrapped; and concerns the scripts each re-implement are lifted into common
  tools (see [Developer workflow](#developer-workflow-breaking-change) and
  [Shared concerns](#shared-concerns-lifted-into-common-tools))
- Date: 2026-10-07
- Scope: one C++ pipeline from engine source and game content to an
  installed, running product on every declared platform. It covers
  canonical texture encoders and decoders; intermediate representations
  (IRs) of textures, materials and models; per-target *lowering* of those
  IRs into the best format each device and backend can use; package layouts
  built for streaming and LOD; and one command line, `kiln`, that builds,
  packages, deploys, runs and tests every product profile. The runtime,
  Hammer, the bakers and Python tools share the same libraries
- Builds on [RFC 0015](0015-asset-identity-content-build-graph.md), which
  owns asset identity, the content build graph, the compiler contract,
  packages, the asset index, the resolver and live reload. This RFC adds
  compilers, IRs and package layout rules to that graph. It adds no second
  graph, store, index, resolver or mount registry. It proposes four
  amendments to RFC 0015, listed under
  [Amendments to other RFCs](#amendments-to-other-rfcs)
- Formats: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md) owns
  KTX2, BSP2, the World Stage and the modern model resource (F9). This RFC
  proposes F9's geometry block as a first slice and leaves its encoding to
  RFC 0008
- Filtering: [RFC 0012](0012-antialiasing-msaa-specular-alpha-coverage.md)
  A4 (R66) owns the normal-variance roughness and alpha-coverage mip math.
  Texture lowering hosts it and adds no math of its own
- Materials and residency: [RFC 0016](0016-render-core.md) owns the surface
  model (`render.material`'s families and parameter blocks; the VMT mapping
  is the translator `render.vmt-translation` under RFC 0016's
  [anti-corruption boundary](0016-render-core.md#the-anti-corruption-boundary-user-decision-2026-10-08),
  amended 2026-10-08), the reduced
  models a device declares, `TextureCache`, `MeshCache`, mip feedback, LOD
  selection and every residency policy. This RFC supplies the data layouts
  and asynchronous reads those owners consume
- Bakers: [RFC 0007](0007-physically-based-lighting-pipeline.md) owns
  `ILightBaker` and its providers. Their outputs are encoded through this
  RFC's codecs
- Editor: [RFC 0002](0002-hammer-responsibility-factorization.md) owns the
  asset catalog, thumbnails and build presentation
- Platform: [RFC 0001](0001-capability-based-platform-architecture.md) owns
  the tool-process contract (R40), static composition and the platform facts
  in product profiles. AGENTS.md's platform, packaging and distribution rules
  bind every packager and transport here
- Execution: [RFC 0003](0003-dependency-aware-job-system.md) owns the job
  graph executors and the CPU/GPU placement rule
- Release flavors: [RFC 0023](0023-release-play-builds.md)
- 3DS: RFC 0026 (PICA200 device adapter) is on the `render-3ds` branch in
  the `source-engine-3ds` worktree, which has not merged here. This RFC takes
  0027 so the numbers don't collide
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  (Q-CONTENT, Q-PRODUCT)

## User direction (2026-10-07)

- "some sort of intermediary representation for models/materials/textures
  … so that we can take one format (retail, or P2:CE) and transpile it as a
  deployment step to let's say the 3DS. ie. each platform gets the best
  format for its hardware/backend."
- "a unified build/deployment CLI across all our different platforms, and
  sets the stage for our Asset pipeline shared across hammer, our rebaker,
  etc. Ideally it's all in modern C++ to make it sharable between runtime,
  hammer, python, and bakers"
- "It needs to be in C++ for perf too - and portability"
- "Canonical Encoders/Decoders"
- "We must also think about streaming and LOD - ie. putting formats into a
  friendly format"
- "Make sure we have liskov substitution principle here so we can add new
  platforms, targets, formats, and pipelines without changing the 'core'
  cli"
- "Let's make breaking changes here. ie. changing our workflow. I'd love to
  use kiln to build and play without having all these adhoc scripts and .sh
  files"
- "and lift shared concerns to common tools"

## Decision summary

1. **One CLI, and the only entry point.** `kiln <command> <profile>`
   builds, packages, deploys, runs and tests every product profile on every
   declared platform. `kiln play <profile> [map]` is the everyday command.
   It is a thin C++ client of the pipeline library. Waf stays the engine's
   code build engine, and `kiln` drives it. This is a breaking workflow
   change: the launchers (`./play*`, `run.sh`), `run.conf`, the
   per-platform build and deploy scripts, the CI build scripts and the
   Python staging scripts are deleted, not wrapped. Configuration comes
   from profiles plus one workspace-local file, not environment variables.
   The repository keeps two executable entry points: `./waf` and a small
   `./kiln` bootstrap.
2. **Shared concerns have one owner.** Profile resolution,
   configure-if-changed, tree naming, the compiler cache, pinned downloads,
   dependency recipes, runtime staging, content mounts, derived media,
   launch arguments, display sessions, device sync, logs, crashes and
   evidence are each re-implemented in several places today (up to 20 for
   runtime staging). Each becomes one `kiln` stage, provider or library
   that every product and harness uses.
3. **Canonical codecs.** Every pixel format has exactly one encoder and one
   decoder, reached through one C++ API (`content.texture-codec`). The
   third-party codec libraries are pinned and private to that module.
   Shipped products may link decoders. They never link encoders.
4. **Three IRs.** Textures, materials and models are imported once into an
   IR that records what the data *means*: a texture's class and colour
   space, a material's family and slots, and a model's render geometry kept
   apart from its gameplay data. Any source format reaches any target
   through the IR, so N importers plus M lowerings replace N × M
   converters.
5. **Lowering is an RFC 0015 compiler.** A lowering node reads an IR and
   the target facts it declares, and writes that target's payload. Import
   nodes are profile-independent and cached once. Lowering is opt-in per
   profile. A profile that declares none packages byte-identical
   passthrough, as RFC 0015 requires today.
6. **Packages are streamable.** Every streamable payload is split into
   aligned, upload-ready units ordered coarse to fine. The coarse unit (a
   texture's mip tail, a model's coarsest LOD) is small and always
   resident. The index records every unit's size, offset, hash and LOD
   metric, so residency is planned without reading payloads.
7. **Libraries first; the CLI is one client.** Every capability is a
   standalone C++ library with a public header. That includes the codecs,
   IRs, readers and writers, lowerings, graph, resolver, profiles,
   packagers, transports and the pipeline itself (`kiln.api`). Each can be
   used without the CLI, without a profile or workspace, and without the
   layers above it. `kiln`'s `main` only parses arguments and prints
   results. Engine code, Hammer, bakers and new tools link the libraries.
   Python binds the same libraries (`sepipe`), not the CLI. `kiln --json`
   remains for processes that can't link, such as Blender's bundled
   Python. An exported SDK lets out-of-tree tools build on the same
   libraries. Python owns no format, codec, key or packaging policy, and
   independent oracles never import the bindings.
8. **Extension by substitution (user direction).** The CLI core depends
   only on narrow contracts: toolchains, recipe builders, pipeline stages,
   packagers, transports, codecs, device layouts, importers and lowerings.
   Each contract records its behavioural obligations and has one shared
   suite that runs against every provider, including fakes, plus
   deliberately bad providers the suite must reject. A new platform,
   target, format or pipeline is a new provider plus a profile that selects
   it by name. The core's command, stage and evidence code never changes
   for it and never branches on a platform's identity. A fixture platform
   added this way is a gate check.

## Observed starting point (2026-10-07)

Read from source at `1eb4e12bf` (this branch) and `92aaacbb6` (the
`render-3ds` branch, which was just rebased onto `1eb4e12bf`). Nothing was
measured for this RFC. Line counts are `wc -l`.

### Product build and deployment

| Platform | Build | Package | Content delivery | Launch |
| --- | --- | --- | --- | --- |
| Linux desktop | `run.sh` (98) runs `waf build` in `build/`; `play` (63) and `play_p2` (216) pick trees such as `build-p2[-fsr][-release]` by flags | none; `run/runtime*` trees staged by `tools/quality/stage_runtime.py` (83) over `portal_boot` staging, `stage_portal2_runtime.py` (474) and `stage_fstop_runtime.py` (507) | symlinks into installed retail content; published maps from `playable_maps.py` | `run.sh` with `run.conf` |
| Linux remote (the bazzite RTX 3070 box) | the local tree | a manual `rsync` recipe and symlink retargeting, recorded in an agent memory note, not in the repository | the same rsync | ssh |
| Android | `build-android-apk.sh` (634): fetch pins, CMake dependency recipes, Waf per ABI, APK assembly, verifier | APK through aapt2, d8, zipalign and apksigner | `adb push` of files whose size or mtime differ, inside the build script | `adb shell am start` |
| iOS, tvOS, macOS | `build-apple-app.sh` (588) plus 14–58-line wrappers | unsigned `.app` built on Linux | `ios-deploy.sh` (250): rsync to the Mac VM, then `xcrun devicectl` copies into `Documents` (iOS) or `Library/Caches` (tvOS) | `devicectl` on the Mac |
| 3DS (`render-3ds`) | `build-3ds.sh` (34) | `tools/n3ds/package_cia.sh` (makerom CXI/CIA) | `build-3ds-content/<map>` from `stage_3ds_content.py` | `tools/n3ds/n3ds.py` (Azahar) |
| Windows (RFC 0024 lane) | MinGW cross build | none | none | Wine |

- **Profiles exist and are the right owner.** `quality/product_profiles/*.json`
  (`source-product-profile/v1`) hold pins, configure options, content
  directories, required checks and evidence. `quality/baseline.json`
  references these files rather than copying them. The `extends` merge rule
  is owned by Python (`tools/quality/profile_extends.py`).
- **The texture pipeline is already declared per profile.**
  `intent.texture_pipeline.format_preferences` selects per-class formats.
  The host-tool profile `ktx2-linux-tools.json` defines the classes
  (`base-color`, `mrao`, `normal`, `mask`, `hdr`) and their Vulkan formats.
  Only `tools/texture/ktx2_select.py` and conformance scripts read them.
- **Dependency recipes are duplicated.** Both `build-android-apk.sh` and
  `build-apple-app.sh` cross-build SDL3, the static KTX reader, freetype,
  libpng, libjpeg and curl with CMake. Each has its own recipe text and a
  hand-bumped recipe version that forces rebuilds.
  `tools/n3ds/build_dependencies.sh` is a third copy for the 3DS.
- **The 3DS build has no profile.** `build-3ds.sh` names
  `quality/product_profiles/portal2-3ds-pica.json`, which does not exist. Its
  configure options live in the script. devkitARM is copied out of the
  `devkitpro/devkitarm` container image with no digest pin, which is an
  unpinned download (AGENTS.md).
- **Deployment facts are prose.** Profiles describe the device content
  location in words ("the app's data container (Documents), copied with
  xcrun devicectl on the Mac"). `ios-deploy.sh` carries default values for
  the Mac VM's host, team and keychain unlock in source.
- **Trees are chosen per script:** `build`, `build-p2`, `build-p2-fsr`,
  `build-p2-release`, `build-android`, `build-ios`, `build-tvos`,
  `build-macos`, `build-3ds` and others. The Android profile declares its
  tree (`android.build_directory`); the desktop profiles do not.
- **Half the support matrix has no profile file.** 9 of the 18 rows in
  `quality/baseline.json` name no product profile. Those rows are the
  dedicated server, tools, test composition, iOS device and simulator, the
  Windows client and dedicated server, MSVC legacy Hammer,
  `linux-i386-legacy`, `android-armv7a-legacy` and `freebsd-legacy`. The
  legacy rows' build facts live in `scripts/build-ubuntu-*.sh`,
  `scripts/build-android-armv7a.sh` and `scripts/tests-ubuntu-*.sh`, which
  `.github/workflows/build.yml` and `tests.yml` call.

### Launch surface

- **The launchers.** `./play` (63 lines), `./play_p2` (216), and the
  wrappers `play_p2_fsr`, `play_p2_release`, `play_p2_fsr_release` and
  `play_release`. Also `play_fstop` (80), `play_p2_coop` (364 lines of
  Python, which runs two peers and swaps `commandline.txt`), `run.sh` (98)
  and `run_portal_box3d.sh`. The last is obsolete: it boots the DXVK
  renderer from a `quality-results` runtime, and Box3D has since become the
  default.
- **Ways to configure a launch:**
  - `run.conf`, with `MAP`, `WIDTH`, `HEIGHT`, `WINDOWED`, `FPS_MAX`,
    `RENDERER`, `PHYSICS*`, `JOB_ARGS`, `MAT_ARGS`, `EXTRA_ARGS`, `BUILD`,
    `BUILD_DIR`, `RUNTIME` and `SDL_VIDEODRIVER`;
  - about 15 environment variables in `play_p2` (`P2_STEAM_ROOT`, `P2_FSR`,
    `P2_FLAVOR`, `P2_WORKSHOP`, `P2_BUILD_DIR`, `P2_RUNTIME`, `P2_WAFLOCK`,
    `P2_NO_BUILD`, `P2_AV1_MEDIA`, `PHYSICS`, `PHYSICS_ARGS`, `JOB_ARGS`,
    `MAT_ARGS`, `QUEUE_ARGS`, `VIDEO_ARGS`, `CORE_WORLD`);
  - leading flags (`--release`, `--fsr`, `--workshop`, `--retail`,
    `--prepare`);
  - the render switches of `tools/quality/render_flags.sh`, a sourced bash
    table of 16 switches.
- **Each launch does its own configure, build and staging.** `play_p2`
  checks the tree's `c4che` for the game, FSR, flavor, AV1 and GL options
  by `grep`. It replays configure through `ensure_configured.py`, sets up
  ccache through `launcher_ccache.sh`, builds, transcodes movies through
  `transcode_av1.py` on every launch, stages through
  `stage_portal2_runtime.py`, and execs the game.
- **Harnesses call the launchers.** 35 modules in `tools/` call `./play*`
  or `run.sh`, among them `frame_floor.py`, `portal2_scenarios.py`,
  `demo_frames.py`, `legacy_ports_views.py`, the map builders and
  `d3d12_lane.py`.
- **Other shared helpers:** 20 modules use `portal_boot`'s staging, and 14
  start a private compositor through `private_session.py`.
- **Stray files at the repository root.** Nothing in the tree references
  any of them:
  - `1eb4e12bf` added 1-byte files left by shell redirects (`BC7.patch`,
    `Decompress.patch`, `zlib.h.patch`, `vtf_decompress`,
    `content.vtf-container`) and a `vkd3d-proton` pipeline cache;
  - earlier commits added a vim swap file (`.bruh.swp`), `patch.diff` and
    15 one-off `fix_*.py`, `patch*.py` and `add_posix.py` migration scripts
    (2026-09-22).

### Content, formats and codecs

- **RFC 0015's first slice is installed.** It provides the `content_build` C++ CLI, the
  `material.vmt` and `texture.passthrough` compilers, a per-profile store,
  and `render_lab` as the resolver's first runtime consumer.
- **C++ decoders exist:**
  - `content.vtf-container`, `content.vtf-reader` and
    `content.vtf-decompress`: VTF 7.0–7.6, including P2:CE's `AXC`
    Deflate/Zstandard runs and BC7 format 70;
  - `content.block-decode`: BC1–BC7 to RGBA8, R8, RG8 or RGBA16F. The
    Vulkan adapter uses it at runtime on devices without BC
    (`SOURCE_VK_NO_BC`, the Galaxy Tab S8 Ultra);
  - `content.ktx2-reader`, over the pinned `libktx_read`;
  - `content.studio-model`, a strict C++20 reader of MDL 44–49, VVD 4 and
    VTX 7 drawable LODs. It has no writer.
- **One pixel-format owner, missing formats.** `texturecontainer::PixelFormat`
  with `LayoutForPixelFormat` is the single layout owner. It has no ETC1, no
  PICA formats (ETC1A4, RGB565, RGBA4444, LA8) and no ASTC block size other
  than 4×4.
- **No canonical encoder.**
  - No BC, ASTC or ETC encoder exists in C++ build code.
  - Encoding goes through Python: `tools/texture/bc_codec.py` runs the
    pinned `ktx` tool, and the bakers' LMAP and RPRB writers call it
    (`pbrt_map_build.py`, `world_lightmap_v3.py`,
    `reflection_probe_set.py`).
  - The legacy material system's runtime DXT encode calls NVIDIA's
    proprietary `S3TCencode` (`public/nvtc.h`).
  - The 3DS has its own ETC1 encoder in device code
    (`materialsystem/shaderapipica/pica_texture.cpp`).
- **Python format readers sit on production paths.**
  `tools/quality/vtf_decode.py` decodes VTFs for the rebaker's scene export
  (`legacy_bsp_scene.py`). `tools/quality/source_content.py` (a VPK reader
  plus hard-coded search paths) is imported by 22 tool modules.
- **Two owners of the VMT patch rules.** `render/material/vmt_import` states
  that it *is* the `material.vmt` compiler ("runtime and build share one
  mapping"). `content/material_compiler.cpp` walks VMT `patch` and
  `include` blocks itself over `kvtext`.
- **The 3DS pipeline is the transpile-per-platform need, done ad hoc:**
  - `tools/n3ds/stage_3ds_content.py` (1,159 lines of Python) computes each
    map's closure itself (BSP texdata, static props, MDL `$cdmaterials`, VMT
    texture keys). It also rewrites VMTs, re-encodes base textures as DXT1
    or DXT5 VTFs at 128 px or less in Python, rewrites the BSP pak lump and
    writes a VPK.
  - `tools/n3ds/model_lod.py` (336) trims MDL, VVD and VTX to one LOD.
  - On the console, `pica_texture.cpp` decodes those DXT textures and
    encodes ETC1 at load time on the 3DS CPU. Then
    `render/device/pica/texel_layout.*` tiles them.
  - The reduced 3DS material model's rules (`render/material/surface_reduced.*`,
    370 lines) are applied at load.
  - The memory audit at `92aaacbb6` records a 98.6 MB main heap with 71 MB
    used on `sp_a1_intro4`, a 22 MB linear heap, and a ~13 MB saving from
    releasing each model's VVD after its meshes are built.

### Streaming and LOD

- `IRenderDevice2::ReadMemoryBudget()` reports per-heap usage and budget.
- `render.resources::MipFeedbackFrame` computes the mip each visible
  texture needs. It has no consumer, so its collector has been off since
  2026-10-05.
- `TextureCache` and `MeshCache` have `EvictToBudget`, but no product owner
  wires a budget.
- On the core, model LODs are separate allocations (`WorldData::StaticMeshLod`).
  A level no view selects for `kModelLevelIdleFrames` (120 frames) is
  released. It is re-uploaded from the world's CPU staging, which stays in
  memory for the world's lifetime.
- The legacy texture manager loads whole textures. A VTF stores every mip
  in one run, so a partial read can't be planned from an index.
- The resolver reads whole entries synchronously. It has no ranged or
  asynchronous read.
- The High profile, `run.conf` and `play_p2` pin `r_lod 0`. Changing LOD
  selection semantics is a recorded user decision (RFC 0016's 2026-10-05
  LOD and mip audit).

## Goals

- One command line for every profile, `kiln build|content|package|deploy|run|test`.
  It runs on Linux hosts first. Its code stays portable to macOS and
  Windows hosts (C++20, `std::filesystem`, the tool-process contract), with
  no bash dependency.
- All pipeline logic in strict C++20 libraries, used in-process by the
  runtime, Hammer, the bakers and `kiln`. They are fast because they run
  compiled, in parallel on the RFC 0003 pool, with no interpreter start-up
  or process hop for each asset.
- One canonical encoder and one canonical decoder per pixel format.
- Any supported source can reach any target through an IR, with no
  source-to-target converter. Sources include retail Portal and Portal 2
  VPKs, P2:CE VTF 7.6/BC7, Workshop PBR packs, loose authored images, and
  later USD and glTF.
- Each target gets content in the best format for its hardware and
  backend. The choice follows the target facts declared in its profile and
  is checked against the device's own facts at start-up.
- Packages are streamable. The coarse data is small and always resident,
  and finer data comes in addressable, aligned, upload-ready units planned
  from the index alone.
- Lowering never changes gameplay data: hitboxes, attachments, collision,
  animation, events, entity data or map logic.
- Desktop profiles stay byte-identical passthrough unless they declare a
  lowering.
- Python and Blender keep working: Python through bindings over the same
  libraries, Blender as an external tool and through `kiln --json`.
- New platforms, targets, formats and pipelines are added without changing
  the CLI core ([Extension contracts](#extension-contracts-and-substitution)).
- New tools are built on the libraries directly, not on CLI wrappers
  ([Libraries first](#libraries-first)).

## Non-goals

- New source formats or editors. RFC 0015's non-goals stand.
- Replacing Waf for engine code, or CMake for third-party dependency builds.
- Custom signing, attestation, updaters, store-submission automation or
  any security programme beyond the platform's normal tooling (AGENTS.md).
- Compiling or lowering content on devices, or shipping an encoder, IR,
  graph or packager in any installed product.
- A remote cache or a build farm (RFC 0015). Remote *deployment* targets
  are in scope.
- Changing shipped LOD selection (`r_lod`, `$lod` thresholds) or shipped
  render quality. Lowering records the metrics a future policy needs; the
  policy is RFC 0016's and the user's.
- Redistributing retail or Workshop content. Packages are built on the
  user's machine from content they have installed. Lowered retail or
  Workshop bytes are still that content (AGENTS.md P2:CE rules).
- Retiring the legacy runtime `S3TCencode` callers. They are runtime
  procedural textures in the frozen material system, and are recorded as a
  follow-up.

## Architecture

### Pipeline shape

```
                         ┌─ deps: pinned archives → CMake recipes per target ─┐
 product profile ─► kiln ┼─ engine: Waf configure/build/install (profile tree) ┼─► package ─► deploy ─► run/test
                         └─ content (RFC 0015 graph):                           ┘   (platform    (transport  (launch,
                              sources ─► import ─► IR ─► lower(target facts)        form)        per device)  logs, crash)
                                                   └─ shared, profile-free store
```

### Layers and owners

Module names are proposed. Arrows point down. archlint enforces the rows
through `architecture/modules.json` as each module lands, with a seeded
violation per new rule.

| Layer | Module | Owns | Depends on |
| --- | --- | --- | --- |
| JSON values | `foundation.json` | One JSON value, parser and writer shared by profiles, `kiln --json`, `hammer_cli --mcp` and the reload endpoint. This is RFC 0015 decision 4's "second consumer" | foundation |
| Product profiles | `product.profile` | Profile schema v2, the `extends` merge, validation, and derived facts: Waf options, target facts, package, deploy, launch and required checks | `foundation.json` |
| Product contracts | `product.contracts` | The extension contracts of [Extension contracts](#extension-contracts-and-substitution): `ITargetToolchain`, `IRecipeBuilder`, `IProductStage`, `IPackager`, `IDeployTransport`, and the typed `ProviderCatalog` | `product.profile`, `platform.contracts` |
| Pipeline API | `kiln.core`, public as `kiln.api` (`public/kiln/`) | The pipeline as a library: sessions, requests (build, content, package, deploy, play, run, test), the stage graph runner, provider selection by name, results and evidence records. Depends on contracts only, never on a provider | `product.contracts`, `content.build-graph`, `jobs.graph` |
| Default composition | `kiln.composition` | `ComposeDefaultCatalog()`: the standard providers in one typed catalog, so any tool can embed exactly what the CLI uses, or compose its own subset | every provider module, `kiln.core` |
| Pixel formats | `content.texture-contract` (exists) | `PixelFormat` and `LayoutForPixelFormat`, extended with ETC1, ETC2/EAC, ASTC 6×6 and 8×8, and the PICA formats | foundation |
| Codecs | `content.texture-codec` | Encode and decode for each format, provider selection, quality settings, deterministic block parallelism | `content.texture-contract`, `jobs.graph`, pinned codec libraries (private) |
| Device layouts | `content.device-layout.<device>`, first `pica` | Upload-ready byte orders a device needs beyond its format, such as PICA Morton tiling and block order. Extracted from `render/device/pica/texel_layout.*`, so the device and the lowering share one owner | `content.texture-contract` |
| Image IR | `content.image` | The texture IR, resize, channel operations, and mip generation hosting RFC 0012 A4's filters | `content.texture-contract` |
| Mesh IR | `content.mesh` | The render-geometry IR, LOD generation, attribute quantization, bone partitioning and cache optimization (pinned meshoptimizer, private) | foundation, `jobs.graph` |
| VMT reading | `content.vmt` | The one VMT reader: patches with `include`/`insert`/`replace`, `[$SYMBOL]` tags, `cond?$key`, fallback blocks, evaluated for a `VmtProfile`. Extracted from `render/material/vmt_import` | `content.keyvalues-text` |
| Format readers and writers | existing `content.vtf-*`, `content.ktx2-reader`, `content.studio-model`; new `content.ktx2-writer` and the upload-image writer ([decision 5](#5-device-native-layouts-go-in-upload-images-standard-formats-in-ktx2)) | One container each | texture contract, block container |
| Importers and lowerings | `content.compile.<kind>.import`, `content.compile.<kind>.lower` | RFC 0015 compilers | the IR modules, readers and writers; `render.vmt-translation` and `render.material`'s parameter blocks for material lowering only |
| Graph, index, resolver | RFC 0015 modules | Unchanged owners. Additions: the shared IR store region, streaming units in index entries, asynchronous ranged reads ([amendments](#amendments-to-other-rfcs)) | — |
| Packagers | `product.package.<form>`: `linux-dir`, `android-apk`, `apple-app`, `n3ds-cxi`, `windows-dir` | Platform package assembly through the platform's own tools | `product.profile`, `content.asset-index`, `platform.tool-process` |
| Transports | `product.deploy.<transport>`: `local`, `ssh`, `adb`, `apple-relay`, `azahar`, `n3ds-net`, `wine` | Install, content sync, launch, log and crash collection for one kind of device link | `product.profile`, `platform.tool-process` |
| Toolchains and recipe builders | `product.toolchain.<target>` (`linux-gcc`, `linux-clang`, `android-ndk`, `apple-llvm`, `devkitarm`, `mingw`), `product.recipe.cmake` | One target's compilers, SDK, sysroot and identity; building one third-party recipe for a toolchain | `product.contracts`, `platform.tool-process` |
| Application | `kiln` | Argument parsing and output formatting only. It calls `kiln.composition` and `kiln.api` and holds no pipeline logic | `kiln.composition`, `kiln.api` |
| Python | `sepipe` (nanobind) | Bindings over the public libraries, including `kiln.api` | the libraries |
| SDK | `kiln sdk export` | Static libraries, public headers, and CMake and pkg-config package files for out-of-tree tools, pinned to one revision and toolchain | the libraries |

Installed products link the resolver, package sources, the index reader
and the decoders they declare, for example `content.block-decode` as the
CPU fallback where a device lacks BC. They never link an encoder, IR,
lowering, graph, packager or transport. Link maps and package inspection
check this per product (C6's existing rule, extended).

## Libraries first

User direction, 2026-10-07: "make sure the content tools can also be used
as libraries standalone so we can build tools on top of these. I don't want
to be confined to CLI wrappers".

### Rules for every library

- **A public header is the contract.** Each module in
  [Layers and owners](#layers-and-owners) below the application row has a
  public header under `public/<area>/`. Its versioned contract name follows
  the repository's `.v1` convention. That header is the whole interface;
  no tool needs a private header, a CLI or a subprocess to use the module.
- **Usable alone, with only its own dependencies:**
  - `content.texture-codec` encodes an in-memory image with no graph,
    profile or package;
  - `content.studio-model`, `content.vmt` and the VTF and KTX2 readers
    take bytes;
  - `content.image` and `content.mesh` run their operations on values;
  - a lowering runs on one IR and a target-facts value;
  - the resolver mounts any `IPackageSource`;
  - the graph runs a tool's own compilers;
  - `kiln.api` runs a whole pipeline for a tool that composes it.
- **No ambient context.** Libraries never read:
  - environment variables, the current directory or `.kiln/local.json`;
  - global search paths or a hard-coded repository root.

  Files reach them as bytes, through injected interfaces (`BuildInputs`,
  `IPackageSource`, a file-system provider) or as explicit paths in a
  request.
- **No hidden effects.** Libraries keep no global state or singletons and
  write nothing to stdout or stderr. Diagnostics go to an injected sink,
  progress to an injected observer, and work to an injected pool.
  Recoverable errors are `foundation::Expected`. Cancellation is a token
  the caller passes. Each header documents its threading and lifetime
  rules.
- **The CLI is a client.** `kiln`'s `main` parses arguments into a
  `kiln.api` request, runs it, and prints the result as text or JSON.
  Every command maps to one public library call, so anything the CLI does,
  a tool can do in-process. An archlint rule allows the `kiln` application
  to depend only on `kiln.api`, `kiln.composition`, `foundation.json` and
  the platform's process entry, with a seeded violation.

### Ways in

| Consumer | Uses |
| --- | --- |
| Engine and runtime products | the resolver, package sources, asynchronous reads and declared decoders, linked statically |
| Hammer | the resolver, codecs, image IR, in-process lowerings for target previews, the graph for builds, and `kiln.api` for play-in-editor |
| Bakers and the rebaker | compilers in the graph; codecs for LMAP, RPRB and LSMK; the C++ readers for scene export |
| New in-tree tools | any library, alone or composed; `kiln.composition` when they want the CLI's provider set |
| Python tools | `sepipe`, bindings over the same libraries, including `kiln.api` requests and results (images through the buffer protocol) |
| Processes that can't link | `kiln --json`, for example scripts running inside Blender's bundled interpreter |
| Out-of-tree tools | the exported SDK |

### The SDK

`kiln sdk export <dir>` installs:

- the static libraries and public headers;
- a CMake package file (`find_package(SourcePipeline)`) and a pkg-config
  file;
- a manifest naming the revision, toolchain identity and pinned third-party
  libraries.

The API is source-level C++20 for the same toolchain family, with no
binary-compatibility promise across revisions. A versioned contract
changes only with a version bump, and in-tree callers migrate in the same
change. A C API is not offered until a consumer needs one.

### Proving it

- **Sample tools.** `tools/samples/` holds small tools built only on the
  public libraries, each with a test:
  - `texture_inspect`: resolver, codec and image IR;
  - `lower_preview`: one asset lowered for one target in-process, then
    decoded;
  - `package_diff`: two package indexes compared;
  - `play_embedded`: a `kiln.api` play request from a custom catalog.

  None links the `kiln` application or starts a `kiln` process.
- **Python.** A `sepipe` example does the same as `lower_preview`.
- **Out of tree.** A CMake project in a temporary directory builds against
  an exported SDK, encodes and decodes a texture and resolves an asset.
- **The CLI really is thin.** The `kiln` application's dependency rule
  catches its seeded violation.

## Extension contracts and substitution

User direction, 2026-10-07: "Make sure we have liskov substitution
principle here so we can add new platforms, targets, formats, and
pipelines without changing the 'core' cli".

### The rule

- **The core knows no platform.** `kiln.core` knows verbs, stage roles,
  artifacts, the provider catalog and evidence. It names no OS, SDK,
  package form, transport, pixel format or asset kind, and it never
  compares a profile's target identity to choose behaviour. It asks the
  provider the profile names, as AGENTS.md requires: portable code
  requests behaviour, not backend identity.
- **Profiles select providers by name.** The keys are `build.toolchain`,
  `package.form`, the device registry's `transport`, `content.lowering`
  and `pipeline.stages`. An unknown name fails composition, naming the
  profile key and the value. There is no fallback provider.
- **Composition is explicit.** The catalog (`ProviderCatalog`) is a typed
  value. `kiln.composition` builds the standard one from the provider
  modules it links; a tool may build its own, adding or omitting
  providers, and pass it to `kiln.api`. There is no global registry, no
  self-registration from static initializers, and no plugin discovery by
  file name (AGENTS.md: no service locator or capability bag).
- **Enforcement:**
  - An archlint rule lets `kiln.core` depend on `product.contracts` and
    the content graph, never on a `product.toolchain.*`,
    `product.package.*`, `product.deploy.*`, codec or device-layout
    module.
  - A scan finds platform-identity literals (`"android"`, `"ios"`,
    `"n3ds"`, SDK names) in `kiln.core` sources. Its count stays at zero.

  Each rule has a seeded violation that must fail.

### What adding something touches

| Adding | New code | Edits elsewhere | Never edited |
| --- | --- | --- | --- |
| A platform | a toolchain provider, a packager, a transport, a platform I/O provider, and a device layout if its GPU needs one | one entry per contract in `kiln.composition`'s provider list; a profile; the pixel-format table only if the GPU has new formats | `kiln.core`, the graph, every other provider |
| A target of an existing platform (a device class, an ABI, a quality tier, a game) | none | a profile that `extends` the platform's, with its own `target_facts` | any code |
| A pixel format | a codec provider, or an entry in an existing one | `content.texture-contract`'s format table, the one owner of byte layouts | `kiln.core`, packagers, transports, lowerings that don't select it |
| A package form or content container | a packager or a container writer | `kiln.composition`'s provider list; the profiles that select it | `kiln.core` |
| An asset kind or asset pipeline | importer and lowering compilers | RFC 0015's kind table, the one owner of kinds | `kiln.core`, the graph |
| A product pipeline stage (for example shader artifacts, a symbol archive, store metadata) | an `IProductStage` provider | `kiln.composition`'s provider list; profiles' `pipeline.stages` | `kiln.core` |

The two shared tables are edited on purpose. They are the single owners
of format layouts and asset kinds (DRY), so a new entry is a contract
change reviewed once, not a CLI change.

### Contracts and their obligations

Each contract records what substitution requires:

- the inputs it must accept;
- its results and effects, and the invariants it keeps;
- failure atomicity, cancellation and idempotence;
- what must change its identity (and so invalidate caches);
- lifetime and threading.

One shared suite per contract runs against every claiming provider,
including fakes. The deliberately bad providers listed must each fail.
Spellings below are proposed; the first implementation fixes them.

**`ITargetToolchain`**: one target's compilers, SDK and sysroot.

```cpp
class ITargetToolchain
{
public:
	virtual ~ITargetToolchain() = default;
	virtual ToolchainIdentity Identity() const = 0;
	virtual foundation::Expected<ToolchainEnvironment, ToolchainError> Prepare(
	    const ResolvedProfile &profile, ICancellation &cancel ) = 0;
	virtual void CheckHost( DoctorReport &report ) const = 0;
};
```

- **Identity** covers every input that can change an output: compiler and
  linker digests, SDK version, sysroot hash and deployment target.
- **`Prepare`:**
  - verifies pins before use;
  - writes only under its own `dependencies/<target>/` directory;
  - is idempotent;
  - returns the environment that Waf and the recipe builder consume:
    tools, sysroot, CMake toolchain file and Waf cross options.
- **Failure** names the missing prerequisite. Downloads only pinned,
  verified archives.
- Bad providers:
  - an identity that omits the SDK version;
  - an unpinned download;
  - a write into the source tree;
  - success with a missing compiler.

**`IRecipeBuilder`**: builds one declared third-party recipe for a
toolchain into a prefix.

- Output is keyed by recipe, pin and toolchain identity.
- The prefix is published atomically, and a failure leaves the previous
  prefix in place.
- Bad providers: one that ignores the toolchain identity (a stale prefix
  reused across targets), and one that publishes a partial prefix.

**`IProductStage`**: one step of a product pipeline.

```cpp
class IProductStage
{
public:
	virtual ~IProductStage() = default;
	virtual StageDescriptor Describe( const ResolvedProfile &profile ) const = 0;
	virtual foundation::Expected<StageResult, StageError> Run(
	    StageInputs &inputs, StageOutputs &outputs, ICancellation &cancel ) = 0;
};
```

- **Artifacts.** `Describe` declares the stage's role, the artifacts it
  consumes and produces, and its resource class. Artifacts are named,
  typed and hashed, for example `engine-install`, `content-package`,
  `platform-package` and `deployed-install`.
- **Roles.** The roles are `deps`, `engine`, `content`, `package`,
  `deploy`, `run`, `test` and `extra`.
- **Ordering.** The runner orders stages by their artifacts on an RFC 0003
  job graph, with the serial executor as its oracle. A command runs the
  graph up to its role.
- **The built-in steps are ordinary providers.** The Waf engine build and
  the RFC 0015 content build are stage providers like any other; the core
  has no special case for them.
- **Obligations:**
  - a stage reads only its declared input artifacts and writes only its
    declared outputs, through staging;
  - publication is atomic;
  - its determinism is declared (exact, or statistical with a tolerance)
    as for RFC 0015 compilers;
  - cancellation publishes nothing.
- Bad providers:
  - reads an undeclared artifact;
  - produces an undeclared artifact;
  - publishes a partial output;
  - ignores cancellation.

**`IPackager`**: build artifacts plus content packages in, one platform
package and its manifest out.

- **The manifest is complete.** It lists every file with its hash and role.
- **Contents match it exactly.** The package contains exactly the
  manifest's files, and none from outside the declared inputs.
- **Determinism.** The same inputs give the same manifest. Bytes may differ
  only where the platform's tool stamps a time or signature, and each form
  declares where.
- **Signing.** It goes through the platform's tooling, with credentials
  from the registry, keychain or environment. Credentials are never
  written into the package, log or evidence.
- **Atomicity.** A failure leaves no partial package.
- Bad providers:
  - an undeclared file;
  - an omitted declared module;
  - a partial publish;
  - an embedded credential;
  - an unstable manifest.

**`IDeployTransport`**: one kind of device link, split into capabilities
so that no transport carries no-op methods.

- **Capabilities.** `IInstall`, `IContentSync`, `ILaunch`, `ILogStream`,
  `ICrashCollect`, `IDeviceFacts` and `IAttach`. A transport returns only
  the ones it implements.
- **Required capabilities are checked first.** A profile's `deploy` section
  declares the capabilities its workflow needs. A missing one fails by
  name before the device is touched. For example, `azahar` boots a CXI
  without `IInstall`, and `n3ds-net` installs a CIA.
- **Obligations:**
  - content sync transfers exactly the entries whose hashes differ, then
    verifies them;
  - it never writes or deletes outside the device's declared content root;
  - an interrupted install or sync leaves the previous install runnable,
    or reports `NeedsReinstall` explicitly;
  - an unreachable device returns `Unavailable`, which never counts as a
    pass;
  - every operation is idempotent;
  - a launch handle's lifetime bounds its log stream;
  - cancellation stops transfers and keeps the previous state;
  - no credential reaches a log or evidence record.
- **Where the suite runs:**
  - an in-memory fake device;
  - `local`;
  - `ssh` against the loopback host;
  - `adb` against the x86_64 emulator;
  - headless `azahar`.

  `apple-relay` and `n3ds-net` need real hardware. Their runs are separate
  evidence, recorded as unavailable without it, like the optional Apple
  runners.
- Bad providers:
  - success after a partial transfer;
  - a delete outside the root;
  - a credential in the log;
  - a claimed capability with no effect (the fake device observes it);
  - ignored cancellation;
  - resending unchanged entries (counted).

**`ICodecProvider`**, per format family. Obligations, beyond the
[codec rules](#canonical-encoders-and-decoders-contenttexture-codec):

- decode of encode stays within the class budget;
- the bytes don't depend on the worker count;
- concurrent block encoding is thread-safe;
- an unsupported format or parameter is refused by name, never served by
  a fallback encoder;
- its identity covers every effective setting.

**`IDeviceLayout`**:

- `Apply` and `Remove` are exact inverses.
- Sizes and padding follow `LayoutForPixelFormat` and the device's
  documented tile rules.
- Bad providers: an off-by-one tile and wrong padding.

**Importers and lowerings** pass RFC 0015's compiler suite. Lowerings also
must:

- leave gameplay files byte-identical;
- report every dropped term;
- refuse by name rather than approximate silently;
- produce outputs within the target facts: a format from the target's
  list, an extent within its limit;
- declare every target fact they read. A seeded lowering that reads an
  undeclared fact gives a stale cache hit, which the suite must catch.

**Platform I/O providers for `IPackageSource::ReadAsync`:**

- exact bytes for the range;
- completion exactly once;
- prompt cancellation;
- never a wait on a frame thread.

Bad providers: one that completes twice, and one that returns a short
read as success.

### Proving the core stays closed

- **A fixture platform.** `fixture-platform` lives under `unittests/` and
  has one provider per product contract:
  - the host compiler as its toolchain;
  - a directory packager with a manifest;
  - the in-memory fake device as its transport;
  - a deliberately unusual byte swizzle as its device layout;
  - a trivial test pixel format with its codec;
  - one `extra` stage.

  A test root composes it, not `kiln.composition`, and its modules are
  ones `kiln.core` may not depend on.
- **The gate.** `kiln.core`, compiled unchanged, runs build → content →
  package → deploy → run on the fixture platform. This shows structurally
  that adding a platform needs no core change.
- **Every real provider** passes the same suites, and every contract's
  bad providers are rejected.

## Product profiles, schema v2

Schema v2 extends `source-product-profile/v1` in place. It adds no registry,
and existing keys keep their meaning. `product.profile` owns the parse,
the `extends` merge and validation. `profile_extends.py` is deleted in the
cutover; its callers use `kiln profiles resolve --json`.

Every product `kiln` builds has a profile file, including the nine
support-matrix rows that have none today.
[Shared fragments](#shared-fragments) carry what several profiles share,
so a game, a flavor or a variant is a short profile that `extends` its
platform's.

New or formalized sections:

- `aliases`: short names (`portal2`) resolved together with the host's OS
  and architecture.
- `build`: the toolchain provider by name (`toolchain`), `configure_options`
  (the existing key), flavors per RFC 0023, the compiler cache, and the
  host-tool profile it needs. The tree is always
  `out/<profile>/<flavor>/`, with its own Waf lock.
- `pipeline`: extra stage providers by name (`stages`), each consuming and
  producing declared artifacts. The built-in roles need no entry.
- `content`:
  - package roots: maps, menus and a boot set;
  - base packages and the mount order (`gameinfo.txt` order);
  - named `mount_sets` selectable at launch: the P2:CE Workshop packs of
    `quality/materials/p2ce-workshop-mounts.json`, published maps,
    transcoded media;
  - the content locators each mount needs (Steam library, directory,
    depot);
  - `lowering`. An absent `lowering` means passthrough.
- `target_facts`: what the target can use. It starts from the existing
  `intent.texture_pipeline` and the host profile's texture classes, which
  move here rather than being copied. Example shape (placeholders, not
  decided values):

  ```json
  "target_facts": {
    "texture_formats": { "base-color": ["etc1"], "mask": ["l8"], "...": [] },
    "texture_layout": "pica-tiled",
    "max_texture_extent": "<n>",
    "material_model": "reduced-pica",
    "mesh": { "position": "<format>", "normal": "<format>", "max_bones_per_draw": "<n>" },
    "lod": { "max_triangles_per_model": "<n>" },
    "streaming": { "resident_tail_bytes": "<n>", "unit_alignment": 4096 },
    "budgets": { "texture_bytes": "<n>", "mesh_bytes": "<n>", "streaming_pool_bytes": "<n>" },
    "vmt_profile": { "dx_level": "<n>", "gpu_level": "<n>", "platform_symbol": "<tag>" }
  }
  ```

  Numbers are set per profile, before measuring, by the profile's owner.
  `vmt_profile` is `content.vmt`'s `VmtProfile` (moved out of
  `render.material` with the reader, amended 2026-10-08), so VMT fallback
  blocks are evaluated for the target at build time.
- `package`: the form (`linux-dir`, `android-apk`, `apple-app`,
  `n3ds-cxi`, `windows-dir`). The existing platform blocks (`android`,
  `ios`, `tvos`, `macos`) keep their names and are the form's facts.
- `deploy`: the on-device content location as data (an app container
  directory, external storage, romfs or SD), replacing today's prose.
- `launch`:
  - the game directory and default map;
  - default game arguments and environment, absorbing `run.conf` and the
    launchers' defaults;
  - the display session (`user`, `private`, `none`);
  - `switches`: named entries, each with a description, game arguments and
    the switches it conflicts with;
  - the run provider (`single`, `coop-pair`, `external-install`).

### Shared fragments

Fragments are profiles with no target, listed under
`quality/product_profiles/fragments/`, that other profiles `extend`.
Examples:

- `render-switches`: the 16 render switches;
- `desktop-launch`: physics, job, material, queue and video defaults;
- `third-party-recipes`: the CMake recipes;
- `source-content`: Portal and Portal 2 mount orders and locators.

The merge rule is `product.profile`'s and is the only one.

### The workspace file

`.kiln/local.json` is untracked and personal. It holds:

- the default profile and map;
- resolution, windowed mode and the frame cap;
- content locations;
- the device registry;
- per-profile extra arguments.

It may override launch values but never build facts. A workspace value
that would change a tree's configuration is refused by name, so two
checkouts building the same profile produce the same tree.

### Target facts are checked twice

- At build time, a package missing a format its target requires fails the
  build (RFC 0015 C4).
- At start-up, the device's `render.device.v2` facts must cover the
  package's variants, or composition fails by name.

A conformance row also compares each profile's declared facts with the
facts a real device run reports, so a profile can't claim a format its
device lacks.

## Canonical encoders and decoders (`content.texture-codec`)

```cpp
// Proposed; spellings are fixed by the first implementation.
namespace content::codec
{
enum class Effort { Fast, Normal, Best };

struct EncodeParams
{
	TextureClass usage;   // base-color, normal, mask, ... (the IR's class)
	Effort effort = Effort::Normal;
	bool perceptual = true;
};

struct CodecIdentity      // part of every lowering node's key
{
	std::string_view provider;  // e.g. "astcenc"
	std::string_view version;   // pinned source revision
	std::uint64_t settings;     // digest of the effective settings
};

[[nodiscard]] foundation::Expected<texturecontainer::TextureImage, CodecError> Encode(
    const texturecontainer::TextureImage &source, texturecontainer::PixelFormat target,
    const EncodeParams &params, jobs::IWorkerPool &pool );
[[nodiscard]] foundation::Expected<texturecontainer::TextureImage, CodecError> Decode(
    const texturecontainer::TextureImage &encoded, texturecontainer::PixelFormat to );
CodecIdentity EncoderIdentity( texturecontainer::PixelFormat format, const EncodeParams &params );
} // namespace content::codec
```

| Formats | Encoder | Decoder | Decoder in products |
| --- | --- | --- | --- |
| BC1–BC5 | bc7enc_rdo (`rgbcx`) | `content.block-decode` (exists) | yes, as the CPU fallback |
| BC6H | Compressonator `CMP_Core`, subject to the L2 selection gate | `content.block-decode` | yes |
| BC7 | bc7enc_rdo | `content.block-decode` | yes |
| ASTC LDR and HDR, 4×4, 6×6, 8×8 | astcenc | astcenc | host tools only |
| ETC1, ETC2 RGB/RGBA, EAC R11/RG11 | etcpak | new in-tree decoder, starting from the 3DS adapter's ETC1 decoder | host tools only |
| ETC1A4 (PICA) | etcpak's ETC1 plus an in-tree 4-bit alpha block | in-tree | host tools only |
| RGB565, RGBA4444, RGBA5551, L8, A8, LA8, RGBA8, RGBA16F | in-tree | in-tree | where a device needs them |
| Legacy VTF-only formats (one-bit DXT1, I8, IA88, UV88, BGR*) | none: import only | `content.vtf-reader` and `content.block-decode` | as today |

Rules:

- **One provider per format.** The provider is a recorded decision with
  corpus evidence: quality per texture class at `Normal` effort, and
  throughput. Changing it changes the codec identity, so every dependent
  node rebuilds.
- **Pinned and private.** Each codec library is pinned by source archive
  and hash in the host-tool profile (as KTX-Software is) and built for the
  host. Only `content.texture-codec` includes it, enforced by an archlint
  rule with a seeded violation.
- **Deterministic.** Blocks are encoded independently on the pool, and the
  bytes are identical for any worker count. Every codec node is `Exact` in
  RFC 0015's sense. Serial and pooled runs are compared byte for byte.
- **Quality oracles per class.** Each class has an error measure:
  - base colour: PSNR and ΔE;
  - normals: angular error;
  - masks and MRAO: per-channel error;
  - HDR: logarithmic error.

  Thresholds are recorded in a budget file before measuring. Seeded broken
  encoders must fail: swapped endpoints, an sRGB/linear mix-up, a wrong
  channel order, dropped alpha, a wrong block order and a wrong mip count.
- **Independent decoding.** Each decoder is checked against Khronos or
  vendor test vectors where they exist. The Python decoders that remain
  become oracles with no production caller.
- **Placement.** CPU providers come first. A GPU encoder is admissible
  under RFC 0003's placement rule when it is measured faster at equivalent
  output; the API does not assume the CPU.

## Intermediate representations

Shared rules:

- **What an IR is.** An IR is a C++ value type with a versioned serialized
  form for the content store. It is a build intermediate: never packaged,
  never read by a product.
- **Import nodes** read a source and write an IR. Their key carries no
  profile facts, so one import serves every profile. IRs live in a shared,
  profile-independent store region (amendment to RFC 0015's per-profile
  store).
- **Lowering nodes** read an IR plus the target facts they declare
  (RFC 0015's `ProfileFacts`), and write a payload and a report.
- **Nothing is dropped silently.** Each lowering lists, per asset, every
  term it dropped or approximated and every refusal by name, in the
  package report. RFC 0026's reduced model already works this way.
- **IRs record meaning, not file layout.** For example, a texture's class
  comes from the material slot that uses it, not from its file name.

### Texture IR (`content.image`)

- **Fields:**
  - class: `base-color`, `mrao`, `normal`, `ssbump`, `mask`, `hdr`,
    `lightmap`, `ui`, `detail`, plus the existing profile classes;
  - colour space and alpha use (none, cutout with its reference, or
    blend);
  - topology: 2D, cube, array, volume or animated frames;
  - the source encoding, and whether it was lossy;
  - decoded levels (RGBA8, RGBA16F or RGBA32F);
  - provenance: package, path and Workshop ID (AGENTS.md P2:CE mount
    policy).
- **Classes come from materials.** The material IR labels each texture
  reference with its slot's class (`$bumpmap` is `normal`). A texture used
  under two classes with different encoding policies is a build error that
  names both referrers.
- **Importers:**
  - VTF 7.0–7.6 through the existing readers;
  - KTX2;
  - PNG, TGA and EXR authored sources, through pinned decoders added when
    first needed.
- **Passthrough without loss.** When a target accepts the source's
  encoding, layout and mip set, the lowering emits the source bytes
  unchanged. P2:CE BC7 on desktop is never decoded and re-encoded.
- **Lowering steps:**
  1. choose the format from the target's per-class preferences;
  2. resize to the target's extent and budget;
  3. generate mips with RFC 0012 A4's filters (normal-variance roughness
     is a paired node keyed on both the normal and the roughness texture);
  4. encode;
  5. apply the device layout;
  6. split into streaming units.

### Material IR

- **The IR is the surface model's own import result.** That is
  `render.vmt-translation`'s result (the translator outside the core that
  owns the VMT mapping since 2026-10-08; `render.material` takes the
  parameter blocks): family, parameter block values,
  texture slots (`AssetRef` plus class), the proxies block as written,
  unmapped keys and editor keys. It gets a serialized form, and there is no
  second mapping authority.
- **One VMT reader.** VMT reading moves into `content.vmt`, so content
  compilers don't depend on render code for parsing. `render.material` and
  every content compiler use it. `content/material_compiler.cpp`'s own
  include walk is deleted in the same change.
- **Lowering per target:**
  - Passthrough profiles get the original VMT bytes.
  - Reduced-model profiles (first: the 3DS, RFC 0026 decision 8) get the
    family's reduced form, evaluated at build time for the target's
    `VmtProfile`. It is written as a reduced VMT holding only what that
    target's frontend reads (base texture, lightmap or vertex lighting,
    vertex colour, tint, alpha test, blend, self-illumination tint), so the
    3DS's legacy material-system frontend keeps loading VMTs. The dropped
    terms go to the report.
  - A compiled material record that the core reads without parsing VMT is
    deferred until the legacy frontend retires on a target. It needs its
    own decision.
- **References stay identities.** Texture references remain `AssetRef`s;
  the resolver picks the target's variant. Lowering never rewrites paths.

### Model IR: render geometry only

- **The split.** Gameplay data passes through byte-identical on every
  target: the MDL header's bones, hitboxes, attachments, sequences and
  events, plus `.ani` and `.phy`. The IR is the render geometry only: VVD
  vertices, VTX strips, LODs, per-mesh material references and skin
  weights.
- **Import:** `content.studio-model`, extended to read every LOD and the
  bone weights. A model with flexes keeps passthrough geometry until flex
  is supported, and lowering refuses it by name.
- **Lowering:**
  - select authored LODs, or generate them with meshoptimizer's simplifier,
    recording each LOD's geometric error;
  - optimize for the vertex cache and vertex fetch;
  - quantize attributes to the target's formats;
  - partition by the target's bone limit;
  - emit one geometry payload per LOD.

  The payload encoding is RFC 0008 F9's geometry block, proposed here as
  F9's first slice; RFC 0008 records the encoding when it lands.
- **Consumers:**
  - The render core's model path reads per-LOD payloads on targets that
    lower geometry.
  - The legacy studiorender paths keep reading VVD and VTX.
  - A lowered target omits VVD or VTX only when no consumer on that target
    reads them, checked by the runtime asset-open census. On the 3DS,
    studiorender's CPU skinning still reads VVD until the core skins.

## Streamable packages and LOD

### Layout rules

- **Units.** A streamable payload is a sequence of units, ordered coarse
  to fine. Each unit is aligned to 4096 bytes (RFC 0015 decision 1's
  GPU-bulk alignment), stored uncompressed, and already in upload layout,
  so it can be mapped or read straight into an upload buffer.
- **Textures:**
  - one *tail* unit holds every mip at or below the target's
    `resident_tail_bytes`;
  - each finer mip is its own unit (two levels per unit for small mips if
    measurement shows per-unit overhead matters);
  - cube faces and array slices of one mip share that mip's unit.
- **Models.** One unit per LOD's geometry payload, coarsest first, with
  its geometric error and triangle count.
- **Maps.** World geometry stays whole in this RFC. Region or cluster
  streaming of BSP2 or World Stage data needs its own design under RFC
  0008.
- **Index.** RFC 0015's index entries gain a unit table: unit kind (tail,
  mip *n*, LOD *n*), offset, size, hash, and the extent or geometric error.
  This is an index format version bump owned by RFC 0015, so a residency
  owner can plan every read from the index alone.

### Runtime

- **Asynchronous ranged reads.** `IPackageSource` gains
  `ReadAsync(entry, unit, destination) → completion`. Platform I/O
  providers implement it:
  - POSIX `pread` on a reader thread first, with `io_uring` only after
    measurement;
  - Android: file descriptors of `noCompress` APK assets;
  - iOS and macOS: mapped bundle files;
  - 3DS: the romfs or SD filesystem.

  Completion arrives on a sequence runner (RFC 0003). No frame thread waits
  on I/O.
- **Residency is RFC 0016's.**
  - Proven in `render_lab` first, under the render binding rules.
  - `TextureCache` and `MeshCache` take requests from `MipFeedbackFrame`
    and the LOD the engine selected, and fetch units coarse to fine. They
    raise a per-texture minimum-LOD clamp as finer mips arrive, and evict
    under the profile's budgets and `ReadMemoryBudget()`.
  - Eviction retires behind completion tokens. A tail or coarsest LOD is
    loaded with its material or model and never evicted, so nothing ever
    draws missing.
- **CPU copies go.** With streaming, a released model level is re-read
  from its package rather than kept in CPU staging. That meets the
  2026-10-04 user request "drop the CPU copies once a model is uploaded",
  and applies the 3DS's VVD release generally.
- **Map loads.** RFC 0015 C5's closure prefetch reads every root's coarse
  units first.

### Oracles

- **Settled equals resident.** After streaming settles at a fixed camera,
  the frame equals the all-resident frame on the same device, exactly.
- **No holes.** Every drawn texture has at least its tail and every drawn
  model has a LOD, on every frame. A seeded "evict the tail" fault and a
  seeded "drop the coarsest LOD" fault must be caught.
- **Budget.** Residency never exceeds the profile's budgets over the
  frame-floor route.
- **No stalls.** A counter of frame-thread waits on I/O stays at zero. A
  seeded synchronous read must be caught.
- **Cost.** Measured with RFC 0016's resolution sweep and the frame-floor
  suite, ABBA interleaved. Streaming must not regress the High row, and the
  Fold7 and 3DS measurements are recorded.

## The `kiln` CLI

| Command | Does | Replaces |
| --- | --- | --- |
| `kiln profiles list\|resolve\|explain <profile>` | Resolved profile, provenance of each value, derived facts | `profile_extends.py` |
| `kiln doctor <profile>` | Host tools, pins, SDKs and reachable devices, each reported as present or unavailable | ad hoc checks in each script |
| `kiln deps <profile>` | Fetch and verify pinned archives; build the third-party recipes for the target | steps 1–2 of `build-android-apk.sh`, 3–4 of `build-apple-app.sh`, `tools/n3ds/build_dependencies.sh`, the devkitARM container copy |
| `kiln build <profile> [--flavor dev\|release]` | Waf configure (only when the configure-options digest changed), build and install in the profile's tree under its lock | `run.sh` step 1, the scripts' Waf steps, `build-3ds.sh` |
| `kiln content <profile> [roots…]` | An RFC 0015 build request with the profile's lowering | `content_build`, `stage_*_runtime.py`, `stage_3ds_content.py`, `model_lod.py` |
| `kiln package <profile>` | The platform package from the build and the content packages | APK assembly, `.app` assembly, `package_cia.sh`, `run/` staging |
| `kiln deploy <profile> --device <name>` | Install the package; sync content by index-hash difference | `adb push`, `ios-deploy.sh`, the bazzite rsync recipe, `n3ds.py` content |
| `kiln play <profile> [map] [--set <switch>]… [--device <name>] [-- args]` | The everyday command: run the stage graph up to `run`, rebuilding, re-lowering, repackaging and resyncing only what changed, then launch | `./play*`, `run.sh`, the build-then-deploy-then-launch sequences of every platform script |
| `kiln run <profile> [--device <name>] [-- args]` | Launch what is already built and deployed, without building; stream the log; collect and symbolize crashes | `P2_NO_BUILD=1`, `ios-deploy.sh --console`, `n3ds.py run` |
| `kiln switches <profile>` | The profile's named launch switches, with their descriptions | `./play --render-help` |
| `kiln test <profile>` | The profile's `required_checks` through the RFC 0005 conformance runner | verifier calls inside each script |
| `kiln content watch\|explain\|gc` | RFC 0015 live reload, cache explanation and garbage collection | `content_build watch\|explain\|gc` |

- **Structured output.** Every command takes `--json` and writes
  versioned, machine-readable output.
- **Evidence.** Every invocation writes an RFC 0005 record:
  - revision and dirty digest;
  - the resolved profile's hash;
  - toolchain and dependency identities, and Waf's recorded invocations;
  - package version and index hash;
  - the device's reported facts;
  - logs;
  - the reproduction command.
- **Composition.** `kiln.composition` composes the tool-process
  provider, the job pool, the I/O providers, the packagers, the transports
  and the display sessions explicitly, and the `kiln` application only
  calls it. There is no service locator.
- **Waf stays the authority for code.** `kiln` passes the profile's
  configure options and reads Waf's status and installed outputs. It never
  re-implements configure, never compiles engine code itself, and never
  reconfigures a tree whose options are unchanged.
- **Third-party recipes are declared once.** Each one lives in a shared
  profile fragment that target profiles `extend`. A recipe is instantiated
  per target with that target's CMake toolchain, into the existing
  `dependencies/<target>/` layout. It is keyed by the recipe, the pin and
  the toolchain identity, which replaces the hand-bumped recipe versions.
- **Packagers use the platform's own tools:**
  - aapt2, d8, zipalign and apksigner for Android, with the content
    archive stored `noCompress` per RFC 0015 decision 1 and Play Asset
    Delivery above the base-APK limit;
  - actool and codesign on a Mac for Apple, reached by `apple-relay`;
  - makerom for the 3DS;
  - plain directories for Linux and Windows.

  Existing verifiers judge the packages without sharing code with them:
  `android_apk.py`, `static_composition.py` and the APK checks.
- **Transports:**
  - `local`;
  - `ssh` for remote Linux hosts such as the bazzite box, syncing content
    by index-hash difference rather than size and mtime, and launching with
    the session environment the target needs;
  - `adb`;
  - `apple-relay`: ssh to a Mac, then `devicectl`;
  - `azahar`: the 3DS emulator, headless in a private compositor;
  - `n3ds-net`: 3dslink or FTP to a homebrew console;
  - `wine`.
- **Device registry.** Devices are listed in a workspace-local, untracked
  file: name, transport, address, serial, and the compatible profiles.
  Credentials come only from the ssh agent, the OS keychain or the
  environment, and never appear in repository files, profiles, logs or
  evidence. The defaults in `ios-deploy.sh` are deleted in the change that
  moves Apple deployment.
- **No wrappers.** The launchers and platform scripts are deleted, not
  kept as wrappers ([Developer workflow](#developer-workflow-breaking-change)).

## Developer workflow (breaking change)

User direction, 2026-10-07: "Let's make breaking changes here. ie.
changing our workflow. I'd love to use kiln to build and play without
having all these adhoc scripts and .sh files".

### The commands

```sh
./kiln play portal2                         # build what changed, stage, launch the default map
./kiln play portal2 sp_a1_intro4            # a map
./kiln play portal2 --flavor release        # RFC 0023's release tree
./kiln play portal2-fsr                     # FSR is a profile: it configures a different tree
./kiln play portal2 --set validate --set no-core-world   # named switches from the profile
./kiln play portal2 --mounts p2ce-workshop  # a declared mount set
./kiln play portal2-coop                    # two paired peers (a run provider)
./kiln play portal2-retail                  # the installed retail game, for comparisons
./kiln play portal2 --device bazzite        # build here, sync, launch on the remote box
./kiln play portal2-android --device fold7  # build, package, install, sync, launch, logcat
./kiln play portal2-ios --device iphone     # build on Linux, sign and install through the Mac
./kiln play portal2-3ds --device azahar     # lowered content, CXI, the emulator
./kiln play hammer                          # the GTK editor
./kiln run portal2                          # launch the staged runtime as is (a second copy)
./kiln test portal2-high                    # the profile's required checks
```

- **`./kiln` is the one bootstrap**, like `./waf`. It is a short, tracked
  POSIX script. It builds the host `kiln` binary with Waf into
  `out/host-<os>-<arch>/` the first time, and again when `kiln`'s sources
  change, then execs it. No other script remains at the repository root.
- **Short names.** Profiles carry `aliases` (`portal2` →
  `portal2-linux-native-vulkan` on a Linux x86_64 host). The workspace file
  can set a default profile, so plain `./kiln play` works.
- **Configuration has two places.**
  - The profile holds launch defaults, the switch table and the mount sets.
  - The workspace-local file `.kiln/local.json` (untracked) holds personal
    values: default profile and map, resolution, windowed mode, the frame
    cap, content locations (the Steam library, the F-Stop depot), devices,
    and per-profile extra arguments.

  `run.conf` and the launchers' environment variables are deleted. One
  `-- args` tail passes anything else to the game.
- **Switches are data.** The 16 render switches of `render_flags.sh`, the
  physics, job, material, queue and video defaults of `play_p2` and
  `run.conf`, and the F-Stop and coop specifics become entries in profile
  fragments. Each entry is a name, a description, game arguments and any
  conflicts. `kiln.core` has no switch of its own (the
  [extension rule](#the-rule)).
- **Trees move.** A tree is `out/<profile>/<flavor>/` with its own Waf
  lock. The existing `build*` trees are not adopted, because Waf records
  absolute paths. The first `kiln build` of each profile rebuilds,
  shortened by the shared compiler cache. The old trees are deleted with
  `kiln clean --legacy-trees` once the user has moved.
- **Harnesses launch through `kiln.api`.** The 35 tool modules that call
  `./play*` or `run.sh` make `kiln.api` play and run requests through
  `sepipe` instead. The typed result reports the staged runtime, process,
  log, screenshot directory and exit status, so harnesses don't recompute
  tree or runtime paths. Their private-compositor handling moves to the
  `private` display session (below).
- **CI calls `kiln`.** `.github/workflows/build.yml` and `tests.yml` run
  `kiln build <profile>` and `kiln test <profile>`. The legacy rows
  (`linux-i386-legacy`, `android-armv7a-legacy`, `freebsd-legacy`, the
  Windows client) get profile files. `scripts/build-*.sh`,
  `scripts/tests-*.sh` and `scripts/deploy.sh` are deleted.

### Old to new

| Today | With `kiln` |
| --- | --- |
| `./run.sh`, `./play [map]`, `run.conf` | `./kiln play portal [map]`, `.kiln/local.json` |
| `./play dxvk` | `./kiln play portal-dxvk` (a profile: a different tree) |
| `./play_p2`, `P2_STEAM_ROOT` | `./kiln play portal2`, content location in `.kiln/local.json` (or found by `kiln doctor` from Steam's library folders) |
| `./play_p2_fsr`, `./play_p2 --fsr` | `./kiln play portal2-fsr` |
| `./play_release`, `./play_p2_release`, `./play_p2_fsr_release` | `--flavor release` on the same profiles |
| `./play_p2 --workshop` | `--mounts p2ce-workshop` |
| `./play_p2 --retail` | `./kiln play portal2-retail` |
| `./play_p2 --prepare` | `./kiln package portal2` |
| `P2_NO_BUILD=1 ./play_p2` | `./kiln run portal2` |
| `./play_p2_coop` | `./kiln play portal2-coop` |
| `./play_fstop`, `FSTOP_CONTENT` | `./kiln play fstop`, content location in `.kiln/local.json` |
| `--native`, `--null`, `--core-world`, `--validate` and the other render flags | `--set <switch>`; `kiln switches <profile>` lists them |
| `PHYSICS=vphysics`, `JOB_ARGS=`, `QUEUE_ARGS=`, `VIDEO_ARGS=` | `--set ivp`, `--set serial-jobs`, `--set sync-queue`, `--set bink`, or profile fragments |
| `run_portal_box3d.sh` | deleted (obsolete) |
| `build-android-apk.sh`, `build-android-portal2-apk.sh` | `./kiln package portal-android`, `./kiln play portal2-android --device <phone>` |
| `build-{ios,tvos,macos}-app.sh`, `build-*-portal2-app.sh`, `build-apple-app.sh`, `ios-deploy.sh` | `./kiln package <apple profile>`, `./kiln play <apple profile> --device <name>` |
| `build-3ds.sh`, `tools/n3ds/package_cia.sh`, `n3ds.py` build and run | `./kiln play portal2-3ds --device azahar` |
| `hammer/gtk/build.sh`, `unittests/hammertest/adapters/build_*_shell.sh` | `./kiln build hammer`, `./kiln play hammer` |
| `scripts/build-*.sh`, `scripts/tests-*.sh`, `scripts/deploy.sh` | `./kiln build <legacy profile>`, `./kiln test <legacy profile>` |

Test-internal scripts that the conformance manifest runs as fixtures, such
as `hammer/gtk/tests/*.sh`, are test code, not workflow, and stay unless
their suite moves. AGENTS.md's command sections and the agent memory
notes that name the old launchers are rewritten in the cutover change.

### The repository root

After the cutover, tracked root files are limited to an allowlist:

- `wscript`, `waf`, `waf.bat`, `kiln`;
- the documentation and licence files (`README.md`, `AGENTS.md`,
  `LIMITS.md`, `LICENSE`, `thirdpartylegalnotices.txt`);
- configuration (`.clang-format`, `.gitignore`, `.gitmodules`);
- directories.

A check in the style suite fails on any other tracked root file, with a
seeded violation. The stray files listed under
[Launch surface](#launch-surface) are deleted in L1.

## Shared concerns lifted into common tools

User direction, 2026-10-07: "and lift shared concerns to common tools".
Each concern below has one owner afterwards. Products, harnesses, Hammer
and CI reach it through `kiln` or its libraries, and the copies are deleted
in the change that moves their last caller.

| Concern | Copies today | One owner |
| --- | --- | --- |
| Profile resolution and `extends` | `profile_extends.py`, plus each script's own profile reads (`jq`, inline Python) | `product.profile` |
| Configure if the options changed | `ensure_configured.py`, `run.sh`, `play_p2`'s `grep` of `c4che`, `build-3ds.sh`, the platform scripts | the Waf engine stage: one configure-options digest per tree |
| Tree, lock and runtime naming | `play_p2`'s suffix logic, `play_release`, each platform script | profile plus flavor gives `out/<profile>/<flavor>`; runtimes go under the tree |
| Compiler cache | `launcher_ccache.sh` (4 callers), `--use-ccache` in `build-3ds.sh` | the engine stage, with `build.cache` in the profile fragment |
| Pinned downloads and verification | each platform script, `tools/ios/build_toolchain.py`, the devkitARM container copy | `kiln deps`: one fetcher and one verifier, digest-pinned |
| Third-party dependency recipes | the Android, Apple and 3DS scripts | `IRecipeBuilder` over recipes declared once |
| Runtime staging, module overlay, dead 32-bit `.so` cleanup | `portal_boot` staging (20 callers), `stage_runtime.py`, `stage_portal2_runtime.py`, `stage_fstop_runtime.py` | the `linux-dir` packager |
| Content mounts (retail VPK order, Workshop packs, published maps, F-Stop depot) | `stage_portal2_runtime.py`, `stage_fstop_runtime.py`, `playable_maps.py`, `play_p2`'s flags, `source_content.py`'s fixed list | profile mount sets over the RFC 0015 mount stack. Content locators (Steam library, directory, depot) are providers |
| Derived media (AV1 movie transcodes) | `transcode_av1.py`, run by `play_p2` on every launch | a content compiler (`video.av1`), cached by key like any other node |
| Launch arguments and defaults | `run.conf`, `play`, `play_p2`, `play_fstop`, `render_flags.sh`, harness command lines | profile `launch` and switch fragments, plus `.kiln/local.json` |
| Display sessions (the user's session or an isolated compositor) | `private_session.py` (14 callers), harness-specific setup | `IDisplaySession` providers: `user`, `private` (headless mutter), `none` |
| Device content sync | `adb push` in the APK script, rsync plus `devicectl` in `ios-deploy.sh`, the bazzite rsync recipe, `n3ds.py` | `IContentSync`, by index-hash difference |
| Logs, crash dumps and symbolization | per-harness `coredumpctl`, `gdb` and `lldb` recipes; `logcat` and `devicectl --console` by hand | `ILogStream` and `ICrashCollect`, plus one symbolizer over the unstripped modules kept by the engine stage |
| Device facts | `frame_pacing_device.py`, `ios_device.py`, ad hoc `vulkaninfo` calls | `IDeviceFacts` |
| Evidence records | each harness writes its own | `kiln.core`'s evidence record, which harnesses extend with their own results |

`IDisplaySession` joins the contracts of
[Extension contracts](#contracts-and-their-obligations):

- a session provides a display environment, starts and stops cleanly, and
  never touches the user's desktop unless it is `user`;
- a bad provider that leaks a compositor process, or reports ready before
  its socket exists, must fail the shared suite.

Multi-process runs (coop pairs) and the retail-reference launch are
`run`-role stage providers. They need no new contract.

## Sharing across runtime, Hammer, bakers and Python

[Libraries first](#libraries-first) sets the rules and the ways in. The
specifics:

- **Hammer:**
  - the asset catalog over the resolver (RFC 0015's cohort 2);
  - thumbnails and previews through `content.texture-codec`;
  - target previews (how this material looks on the 3DS) by running a
    lowering in-process and decoding the result;
  - map builds through the graph (RFC 0015 C2);
  - play-in-editor through a `kiln.api` play request.
- **Bakers, including the rebaker** (`legacy_bsp_relight.py` and
  `map_lighting.py`):
  - Blender stays an external tool, run by C++ compilers in staged roots
    (RFC 0015 compiler rules). It renders; it never writes a final format.
  - LMAP, RPRB and LSMK encoding moves from `bc_codec.py` to
    `content.texture-codec`.
  - The rebaker's VTF decoding for scene export moves from `vtf_decode.py`
    to the C++ readers through `sepipe`.
- **Python bindings.** `sepipe` is a nanobind module, pinned by source
  archive, host tools only, built for the pinned host Python. It binds the
  public libraries themselves (codecs, image IR, readers, resolver, graph,
  `kiln.api`), not the CLI. Harnesses use it to run play and run requests
  and read typed results.
- **Python owns no policy.** It never owns key, normalization, reference,
  codec, layout, lowering or packaging policy (RFC 0015's goal, widened).
- **Independent oracles stay independent.** RFC 0005 oracles never import
  `sepipe` or call `kiln` to judge `kiln`'s own outputs. Those oracles are
  `content_index_check.py`, `vtf_decode.py` once it is an oracle,
  `android_apk.py` and `static_composition.py`.
- **Shrink-only ratchet.** A proposed
  `tools/quality/pipeline_python_scan.py`, with a sensitivity suite, counts
  production Python modules that parse, encode or stage formats. Every
  migration commit lowers the count, in the style of RFC 0015's lookup
  ratchet.

## Per-platform shapes

| Target | Profiles | Content form | Package | Transport | Notes |
| --- | --- | --- | --- | --- | --- |
| Linux desktop | `portal-linux-wayland-native-vulkan`, `portal2-linux-native-vulkan-high`, plus flavors | passthrough; retail VPKs mounted read-only, overlays and published maps as packages | `linux-dir` | `local` | tests in a private compositor, never on the user's desktop |
| Linux remote | the desktop profiles | as desktop | `linux-dir` | `ssh` | benchmark host per the user's rule |
| Dedicated server | its profile (R12) | server closure only: maps, models' gameplay files, scripts, no textures | `linux-dir` | `local`, `ssh` | links no render or codec code |
| Hammer GTK | `hammer-gtk-linux` | the workspace mount stack | `linux-dir` | `local` | in-process lowering for previews |
| Android | `portal(2)-android-native-vulkan` | per target facts; first non-passthrough target on this branch ([recommended first work](#proposed-decision)) | `android-apk` | `adb` | content archive in the APK in release; `adb` sync for development only (RFC 0015) |
| iOS, tvOS, macOS | the Apple profiles | per target facts | `apple-app` | `apple-relay` | content in the bundle in release; `Documents`/`Library/Caches` sync for development |
| 3DS | `portal2-3ds-pica`, created on `render-3ds` | ETC1/ETC1A4/RGB565 tiled upload images within the target extent; reduced VMTs; lowered single-LOD quantized geometry | `n3ds-cxi` (CXI and CIA) | `azahar`, `n3ds-net` | devkitARM pinned by digest |
| Windows (RFC 0024 lane) | `render-d3d12-windows` | passthrough | `windows-dir` | `wine` | lab lane only |

Formats for Android and Apple are not chosen here. Each profile's owner
sets its `target_facts` from device queries, which `kiln doctor` and the
start-up check record.

## Migration and deletions

The old copy is deleted in the change that replaces it, with no wrapper
period (user direction). Each cohort names its deletion condition:

| Cohort | Replaced by | Deleted when |
| --- | --- | --- |
| `run.sh`, `run.conf`, `play`, `play_p2`, `play_p2_fsr`, `play_release`, `play_p2_release`, `play_p2_fsr_release`, `play_fstop`, `play_p2_coop`, `run_portal_box3d.sh` | `kiln play` and `kiln run` with the desktop profiles, fragments and `.kiln/local.json` | the L1 launch-equivalence check passes for every launcher mode, and no tracked file calls them |
| `tools/quality/render_flags.sh`, `launcher_ccache.sh`, `ensure_configured.py`, `profile_extends.py` | switch fragments, the engine stage, `product.profile` | L1, in the same change as the launchers |
| `stage_runtime.py`, `stage_portal2_runtime.py`, `stage_fstop_runtime.py`, `portal_boot`'s staging functions | the `linux-dir` packager and mount sets | `portal_boot`, the frame-floor suite and the corpus suites pass on `kiln`-staged runtimes |
| `private_session.py` | the `private` display session | its 14 callers use `kiln.api` run requests |
| `transcode_av1.py` as a launch step | the `video.av1` compiler | `kiln play portal2` plays the transcoded movies from the store |
| `scripts/build-*.sh`, `scripts/tests-*.sh`, `scripts/deploy.sh` | `kiln build` and `kiln test` with the legacy profiles | the CI workflows run `kiln` and produce the same outcomes on the legacy rows |
| `hammer/gtk/build.sh`, `unittests/hammertest/adapters/build_*_shell.sh` | `kiln build hammer` | `corpus.hammer.ui` passes on the `kiln` tree |
| Stray root files | nothing | L1, with the root allowlist check |
| Dependency recipe blocks in `build-android-apk.sh`, `build-apple-app.sh`, `build_dependencies.sh` | shared recipes plus `kiln deps` | `kiln deps` reproduces each prefix with the same pins |
| `build-android-apk.sh`, `build-android-portal2-apk.sh` | `kiln` with `android-apk` and `adb` | `android_apk.py` passes on `kiln`'s APKs for every declared ABI and the Fold7 smoke runs |
| `build-apple-app.sh`, its wrappers, `ios-deploy.sh` | `kiln` with `apple-app` and `apple-relay` | `static_composition.py` passes and the iPhone/Apple TV smoke runs (or is recorded unavailable) |
| `build-3ds.sh`, `package_cia.sh`, `n3ds.py` build and content paths | `kiln` with `n3ds-cxi`, `azahar`, `n3ds-net` | the Azahar `sp_a1_intro4` run passes from `kiln` |
| `stage_3ds_content.py`, `model_lod.py` | the 3DS lowering profile | the L3–L5 gates pass on the 3DS |
| ETC1 encoding and reduction at load in `pica_texture.cpp` and the shader API | build-time lowering | the 3DS asset-open census shows only upload images and reduced VMTs |
| `bc_codec.py`, and the Python encode paths of `ktx2_pack.py` and `lightmap_ktx2.py` | `content.texture-codec` | baker outputs match within the codec quality budget, and LMAP/RPRB readers accept them |
| `content/material_compiler.cpp`'s VMT walk | `content.vmt` | the VMT corpus and the claim inventory are unchanged |
| `content_build` | `kiln content` | its tests and the RFC 0015 commands are moved |
| `source_content.py`, `vtf_decode.py` in production paths | the resolver and C++ readers | the Python ratchet reaches zero for them; `vtf_decode.py` remains only as an oracle |

## Phases and gates

Each gate needs negative controls, recorded evidence (revision, profile,
inputs, counts, first divergence, reproduction commands) and the AGENTS.md
reporting rules. Missing devices are recorded as unavailable, and an
unavailable device never certifies a gate.

### L0: The pipeline library, profiles and `kiln build`

Status: implemented 2026-10-07; evidence and decisions in the
[progress record](0027-progress.md#l0-the-pipeline-library-profile-schema-v2-and-kiln-build-2026-10-07).

- `foundation.json`, `product.profile`, schema v2 and the shared fragments.
- Profile files for every desktop product: Portal, Portal DXVK, Portal 2,
  Portal 2 FSR, Portal 2 High, coop, retail, F-Stop, the dedicated server,
  tools and Hammer.
- `product.contracts`, `kiln.core` published as `kiln.api`,
  `kiln.composition`, and the thin `kiln` application.
- The `./kiln` bootstrap.
- The Waf engine stage and the `linux-gcc`/`linux-clang` toolchains as the
  first providers.
- The shared suites for toolchains, recipe builders, stages, packagers,
  transports and display sessions, each with its fakes and bad providers,
  and the fixture platform.
- `kiln profiles`, `kiln doctor` and `kiln build`.
- Gate:
  - `kiln profiles resolve` equals `profile_extends.py` on every existing
    profile.
  - Seeded faults fail: an `extends` cycle, an unknown key, an unknown
    provider name, a missing pin, a conflicting configure option, and a
    workspace value that would change a build fact.
  - Every contract's bad providers are rejected.
  - The fixture platform runs end to end through an unchanged `kiln.core`.
  - The dependency rules for the core and for the thin application, and
    the platform-literal scan, each catch their seeded violation.
  - A second `kiln build` of an unchanged profile neither reconfigures nor
    rebuilds.

### L1: Desktop cutover (breaking)

- The `linux-dir` packager, mount sets and content locators.
- The `user`, `private` and `none` display sessions.
- The `single`, `coop-pair` and `external-install` run providers.
- The `video.av1` compiler, `.kiln/local.json`, and `kiln play`, `kiln run`
  and `kiln switches`.
- `sepipe` with the `kiln.api` bindings, and the `play_embedded` sample.
- The 35 harness modules and the CI workflows moved to `kiln`.
- AGENTS.md's command sections and the agent memory notes that name the
  old launchers, rewritten.
- The deletions in the first nine rows of the
  [migration table](#migration-and-deletions), and the root allowlist
  check.
- Gate:
  - **Launch equivalence**, recorded once before the old launchers are
    deleted. For every launcher mode, `kiln play --dry-run` produces the
    same game argv and environment as the old launcher (with paths
    normalized), and the staged runtime has the same file manifest by
    hash, apart from recorded differences.

    The modes: `./play` native, dxvk and null; `./play_p2` with FSR,
    release, workshop, retail and the render switches; `./play_fstop`;
    `./play_p2_coop`. A seeded difference (a dropped switch, a wrong mount
    order) must be caught.
  - `portal_boot`, the frame-floor route, `corpus.hammer.ui` and the
    corpus suites pass through `kiln.api`.
  - The coop pair starts and connects as `./play_p2_coop` does today.
  - CI's legacy rows give the same outcomes as before.
  - No tracked file calls a deleted script; a scan proves it.
  - The root allowlist catches its seeded violation.

### L2: Canonical codecs

- `content.texture-codec` with the providers above, the new pixel formats,
  and the ETC and PICA decoders.
- The `texture_inspect` sample.
- Gate:
  - Per-class quality thresholds, set before measuring, pass on a texture
    corpus drawn from Portal, Portal 2 and the P2:CE packs.
  - Serial and pooled output are byte-identical.
  - Every seeded bad encoder is caught.
  - Every corpus VTF decodes the same through the codec API as through
    `content.vtf-reader` and `content.block-decode`.
  - Installed products' link maps contain no encoder.
  - The BC6H provider choice is recorded with its evidence.

### L3: Texture IR and lowering

- Import from VTF (including P2:CE) and KTX2.
- Lowering for passthrough desktop, one non-passthrough mobile profile,
  and the 3DS (after `render-3ds` merges).
- Mips with RFC 0012 A4's filters, the upload-image writer and streaming
  units.
- The `lower_preview` and `package_diff` samples, and their `sepipe`
  equivalent.
- Gate:
  - Incremental builds equal clean builds.
  - Change classes: touching a texture rebuilds its import, its lowering
    per profile and the package only.
  - Lowered images meet the class budgets against the IR.
  - Desktop packages stay byte-identical passthrough.
  - On the 3DS, the device's ETC1 encode is deleted, and load time and
    heap are recorded against the `92aaacbb6` audit.
  - The device-without-BC case (Tab S8 class) loads lowered variants with
    zero CPU block decodes, and its memory is recorded.

### L4: Material IR and lowering

- `content.vmt` extracted and the material IR serialized.
- The reduced-VMT writer, with the 3DS reduction moved from load to build.
- Gate:
  - The VMT corpus and the Portal 2 claim inventory are unchanged.
  - On the 3DS (Azahar), build-time reduction gives the same images as
    load-time reduction on the RFC 0026 scenes, and the dropped-term lists
    are equal.
  - A seeded wrong rule (a dropped tint, a lost alpha test) is caught.

### L5: Model IR and lowering

- Full LOD read in `content.studio-model`, pinned meshoptimizer, LOD
  generation, quantization, bone partitioning and the per-LOD geometry
  payload.
- The core's model path reads the payload on lowered targets.
- Gate:
  - Gameplay files are byte-identical on every target.
  - Each LOD stays within its recorded geometric error.
  - Core-drawn images of lowered models stay within a per-LOD tolerance of
    the source on desktop and the 3DS.
  - Seeded faults are caught: a wrong bone remap, a lost material
    reference, and a quantization out of range.
  - 3DS heap is recorded.

### L6: Streaming

- Asynchronous ranged reads in the package sources.
- RFC 0016's residency consumer, proven in `render_lab` first, then in the
  game.
- CPU staging copies dropped.
- Gate: [the streaming oracles](#oracles) on desktop, with the Fold7 and
  3DS rows recorded.

### L7: Platform cutovers (breaking)

One child slice per platform. Each delivers that platform's toolchain,
recipes, packager and transport, and deletes its scripts in the same
change ([migration table](#migration-and-deletions)):

- Android: `android-ndk`, `android-apk`, `adb`.
- Apple: `apple-llvm`, `apple-app`, `apple-relay`.
- 3DS, on `render-3ds` or after it merges: `devkitarm` pinned by digest,
  `n3ds-cxi`, `azahar`, `n3ds-net`, and the `portal2-3ds` profile.
- The Windows lane: `mingw`, `windows-dir`, `wine`.
- The remote Linux host: `ssh`.

Gate, per platform:

- The package passes its existing independent verifier.
- `kiln play <profile> --device <name>` builds, packages, installs, syncs
  and launches on each available device.
- A second play with one changed asset transfers only that asset's
  entries.
- Seeded faults fail: a credential in a profile, an unreachable device and
  a partial transfer.
- Unavailable devices are recorded as unavailable.

### L8: Shared consumers and the SDK

- Hammer thumbnails, previews and play-in-editor through the libraries.
- Baker outputs through the codecs.
- The Python ratchet installed and lowered.
- `kiln sdk export`, and the out-of-tree consumer test.
- Gate:
  - Hammer's viewport suites pass on both compilers.
  - Baker outputs are accepted by their readers and meet the codec quality
    budget.
  - The ratchet's sensitivity seeds are caught.
  - The out-of-tree CMake project builds against the exported SDK and
    passes its checks.

**Dependencies:**

- L0 and L2 are ready now.
- L1 needs L0.
- L3 needs L2. It runs on RFC 0015's installed serial graph; its
  pooled-equivalence check waits for C1's pooled executor.
- L4 needs L3.
- L5 needs L2.
- L6 needs L3, L5, RFC 0015 C5's package sources, and RFC 0016's residency
  owner.
- L7 needs L1. Lowered content on a platform also needs L3.
- L8 needs L2–L4.

## Decisions

Agent decisions under the user's standing instruction (choose the
recommended long-term option and record why). The user may change any of
them.

### 1. One binary, named `kiln`; `content_build` folds into it

One CLI is one owner of argument parsing, profiles, evidence and
composition. `kiln` collides with nothing in the tree. `forge` was
rejected: Hammer's resources already use it (`hammer/res/forge.ico`).
Renaming is cheap until the first external caller exists.

### 2. Lowering is a compiler, not a new graph concept

RFC 0015's node key already includes declared profile facts, staged
inputs and the compiler's identity, so lowering needs no new key rule. The
only addition is a profile-independent store region, so imports are shared
across profiles.

### 3. Lowering is opt-in per profile; passthrough stays the default

Desktop profiles keep RFC 0015's byte-identical passthrough, and its
oracles. Lowering is declared, reported and gated per profile. This
refines RFC 0015's non-goal "Converting legacy content by default" rather
than reversing it.

### 4. Codec providers: astcenc, bc7enc_rdo, etcpak, and CMP_Core for BC6H

These are portable C++ under permissive licences, maintained, and standard
choices for their formats. ISPC-based encoders were rejected because they
need the ISPC compiler on every host. Basis Universal as the single
runtime format was rejected:

- the PICA200 can't sample any of its transcode targets;
- transcoding to BC7 loses quality against direct encoding on desktop.

The BC6H choice is gated in L2 because the field is thinner.

### 5. Device-native layouts go in upload images; standard formats in KTX2

KTX2 (RFC 0008) holds every standard Vulkan format. A device-native layout
(PICA Morton tiling, ETC1A4, which has no Vulkan format) goes in a minimal
*upload image*: a block-container entry with format, extents, level
offsets and device-layout bytes. Its layout functions are owned by
`content.device-layout.<device>`, which the device adapter also uses. This
keeps the 3DS from transforming bytes at load, which is also what
streaming needs: map and upload.

### 6. Models: gameplay data passes through; render geometry is lowered

Gameplay data must never change, and the MDL header carries it. Render
geometry is where targets differ. Splitting along that line lets lowering
change vertex formats and LODs, which a VVD cannot express, without
touching gameplay. The geometry payload is proposed as RFC 0008 F9's first
slice, so it is not a second model format.

### 7. Reduced materials are written as VMTs for now

The 3DS still runs the material system's frontend over the PICA adapter.
A reduced VMT keeps that frontend working while the reduction moves to
build time. A compiled material record pays off only once a target's
frontend no longer reads VMTs, so it waits for that decision.

### 8. Streaming units: tail plus per-mip units, per-LOD units, aligned and uncompressed

- Coarse-to-fine units with an always-resident coarse unit give no holes
  and simple residency.
- 4096-byte alignment and uncompressed GPU payloads allow mapping and
  direct upload (RFC 0015 decision 1).
- Compressing non-GPU entries follows RFC 0008's zstd decision.

### 9. Python binds the libraries; `kiln --json` is only for processes that can't link

`sepipe` binds the public libraries, including `kiln.api`, so Python tools
get typed results and in-process speed instead of parsing CLI output.
nanobind is small and fast to compile. The CLI's JSON stays for
interpreters that can't load the module, such as Blender's bundled one,
because a process boundary has no ABI coupling. Oracles stay independent
of both.

### 10. Devices and credentials are workspace-local

AGENTS.md requires that credentials stay out of source and logs, and
device inventories are personal. The registry is untracked, and
credentials come only from the agent, the keychain or the environment.

### 11. Waf owns code builds; recipes are declared once in profile fragments

This follows AGENTS.md's build rules: Waf is the entry point, and profiles
own facts. Shared fragments use the existing `extends` rather than a new
registry.

### 12. Trees move to `out/<profile>/<flavor>/` (user direction: breaking)

This revises the first draft, which kept the old names. One naming rule
replaces the suffix logic in `play_p2`, `play_release` and every platform
script, and gives each tree its lock and runtime. Waf trees record
absolute paths, so the old trees are rebuilt rather than moved; the
compiler cache shortens that. `kiln clean --legacy-trees` removes them
once the user has moved.

### 13. Extension through contracts and typed composition, not plugins

`kiln.core` sees only `product.contracts`, and `kiln.composition` composes a
typed `ProviderCatalog` from linked modules.

- Typed composition can be tested, works the same in static and host
  builds, and keeps one visible list of providers.
- Shared-library plugin discovery would add a loader, ABI and trust
  surface that AGENTS.md keeps out of portable code. It would also make
  "which providers exist" depend on files found at run time.

An out-of-tree platform links its providers into its own root over the
same unchanged core.

### 14. Transports are split into capabilities

Devices genuinely differ: an emulator boots an image without installing
it, and a console has no crash dumps a host can pull.

- A transport with no-op methods would claim behaviour it doesn't have.
  That is the substitution failure AGENTS.md's LSP rules forbid.
- Capability interfaces let a profile require what its workflow needs, and
  fail by name before touching a device.

### 15. Built-in pipeline steps are ordinary stage providers

The Waf engine build and the RFC 0015 content build implement
`IProductStage` like any added stage. A new pipeline step therefore gets
the same artifact, staging, cancellation and evidence rules as the
built-in ones, and the runner has no special cases to keep in step with
new stages.

### 16. `kiln` is the only entry point; nothing is wrapped (user direction)

Wrappers would keep two interfaces alive, each documented and tested, and
let the old scripts' environment variables and flags live on as a second
configuration surface. Deleting them in the change that reproduces them,
proven by the launch-equivalence check, keeps one interface.

The one bootstrap script, `./kiln`, exists because a fresh checkout has no
C++ binary yet. It does only that, as `./waf` does for Waf.

### 17. Two configuration places: profiles and `.kiln/local.json`

Tracked facts (how a product is built, launched and mounted) live in
profiles and fragments, so they are reviewed and shared. Personal facts
(paths, devices, default map, window size) live in one untracked file.

Environment variables and `run.conf` are removed as configuration. That
eliminates the precedence rules (built-in defaults, then `run.conf`, then
the environment) that `run.sh` and `play_p2` implement differently.
Workspace values can't change build facts, so a tree depends only on its
profile.

### 18. Launch switches are profile data

The render switches change with the render core every week. Keeping them
as data in a fragment lets them change without touching `kiln.core`, which
the extension rule requires, and `kiln switches` documents them from the
same entries.

### 19. Harnesses use `kiln.api`, not their own launch code

The 35 harness modules each rebuild tree paths, staging and launch
command lines. A typed play or run request returns the runtime, process,
log and screenshot locations, so harness code shrinks to its oracle. The
launch-equivalence check proves that a harness sees the same game process
as before.

### 20. Libraries first; the CLI holds no logic (user direction)

"I don't want to be confined to CLI wrappers." Every command is one
public library call. A tool built on the libraries can therefore do
everything the CLI does, in-process, with typed results and its own
provider set. Hammer, the bakers, `sepipe`, the sample tools and the SDK
all use that same surface.

### 21. The repository root has an allowlist

Stray files reached the root through broad commits in a shared worktree.
An allowlist check stops that class of mistake at review time rather than
by later clean-ups.

## Amendments to other RFCs

To be applied in the change that implements each phase, so no owner
carries a conflicting statement before then:

- **RFC 0015:**
  - (a) Non-goals: lowering is opt-in per profile and passthrough stays the
    default ([decision 3](#3-lowering-is-opt-in-per-profile-passthrough-stays-the-default)).
  - (b) The store gains a profile-independent IR region.
  - (c) Index entries gain the streaming unit table (an index version bump),
    and `IPackageSource` gains `ReadAsync`.
  - (d) `content_build` becomes `kiln content`, and decision 4's endpoint
    owner becomes `kiln content watch`.
- **RFC 0008:** F9's geometry block is proposed as its first slice (L5).
  The KTX2 writer is `content.ktx2-writer`.
- **RFC 0016:** the residency consumer of `MipFeedbackFrame` and of model
  levels reads streaming units (L6, lab first). Its 2026-10-05 note that
  mip feedback has no consumer closes when L6 lands.
- **RFC 0012:** A4's filters run in `content.image` during lowering.
- **RFC 0026** (`render-3ds`): load-time ETC1 encoding and reduction move to
  build-time lowering. `texel_layout.*` moves to
  `content.device-layout.pica`, which the adapter keeps using.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| The cutover breaks harnesses or habits that still call the old launchers | Harness callers move in the cutover change, and a scan proves no tracked file calls a deleted script. AGENTS.md and the agent memory notes are rewritten in the same change. The launch-equivalence check runs before anything is deleted |
| Other sessions in the shared worktree are mid-task on the old launchers | The cutover lands as one change at a checkpoint, announced to peer sessions first. The old trees remain on disk until `kiln clean --legacy-trees` |
| The library surface grows unstable as tools build on it | Versioned contracts, in-tree callers migrated with each version bump, sample tools and the SDK consumer test in CI |
| Platform branches creep into the core (`if (os == "android")`) | The core's archlint dependency rule and its zero-count platform-literal scan, both with seeded violations, and the fixture platform gate |
| A provider passes its suite but misbehaves on real hardware | Suites run on fakes and on the real providers wherever a runner exists. Device runs are separate recorded evidence, never inferred from fakes |
| Contracts grow until every provider stubs methods | Capability interfaces, not wide ones. A no-op or downcast in a provider is a review signal, and a narrower capability is split out instead |
| `kiln` becomes a second build system beside Waf | `kiln` never compiles engine code or re-implements configure; it passes profile options to Waf and records Waf's own invocations |
| Lossy lowering hides quality regressions | Class budgets set before measuring, seeded bad encoders, passthrough when the target accepts the source, and per-asset reports of every dropped term |
| Re-encoding already-compressed sources compounds loss | Passthrough of an accepted encoding; otherwise a single decode to the IR and one encode, never transcoding one block format to another |
| Gameplay drift from model lowering | Gameplay files pass through byte-identical, enforced per target |
| Two owners creep back (VMT rules, layouts, codecs) | `content.vmt`, `content.device-layout.<device>` and `content.texture-codec` are single owners, enforced by archlint rules and deletion in the same change |
| Streaming stalls or holes | Always-resident coarse units, the no-hole and no-stall oracles with seeded faults, completion-token eviction, and measurement before any default |
| Third-party codecs drift or break determinism | Source-archive pins, codec identity in every key, and the serial-equals-pooled byte check |
| Python callers bypass the libraries | The shrink-only Python ratchet, and wrappers that exec `kiln` |
| Credentials leak through deploy tooling | A workspace-local registry, credentials from the agent, keychain or environment only, and a seeded credential-in-profile fault |
| The 3DS branch diverges from this design | RFC 0026's amendments are listed here, and the 3DS lowering target lands only after `render-3ds` merges |

## Alternatives considered

### Keep per-platform scripts and Python staging

This is the status quo. It is fast to extend for one platform, but every
platform repeats closure, format and packaging logic. The 3DS stager is the
evidence: it re-derives closure, rewrites VMTs and encodes textures in
Python. It can't run inside Hammer or the runtime, it is slower than
compiled code, and it ties tooling to a POSIX shell. Rejected.

### Python orchestration over C++ codecs

This would keep the CLI in Python and move only the codecs to C++. The user
asked for C++ for performance and portability. A Python orchestrator still
needs a matching interpreter on every host, and it can't be linked into
Hammer or the runtime. Rejected; Python remains a caller.

### CMake, Bazel or ninja as the product build

Waf is the engine's entry point (AGENTS.md). RFC 0015 already rejected
these for content. A second code build authority would duplicate toolchain
and dependency facts. Rejected.

### Lower on the device at load time

This is the 3DS today. It spends device CPU, memory and load time to
re-encode the same textures on every launch, and it can't produce
streamable units. Rejected.

### One universal runtime format (Basis Universal or UASTC everywhere)

The PICA200 samples none of its targets, and desktop quality would drop
against direct BC7. Rejected as the default. It may still be offered as an
encoding where a target's facts select it.

## Roadmap

AGENTS.md owns ranks and states. R102 is ranked; the other rows are
unranked (ranking is a user decision):

| Phases | Proposed row |
| --- | --- |
| L0, L1, L7 | R102 (was R98, which RFC 0026 also took; ranked 75 in AGENTS.md, 2026-10-07): the pipeline library, profile schema v2, the `kiln` CLI, the desktop cutover and the platform cutovers |
| L2, L3 | R99: canonical codecs, texture IR and lowering |
| L4, L5 | R100: material and model lowering |
| L6 | R101: streaming units and residency, a child of R89/R91 with RFC 0016 |
| L8 | under R102 and R99–R100, as their consumer cohorts and the SDK |

## Proposed decision

Adopt one product pipeline, built from:

- standalone C++ libraries, with `kiln.api` as the pipeline;
- `kiln` as the single, thin CLI over profile schema v2;
- canonical C++ encoders and decoders;
- texture, material and model IRs, lowered per target as RFC 0015
  compilers;
- streamable package units;
- extension by substitution.

Delete the launchers and platform scripts as `kiln` replaces them, with
no wrapper period. Keep Waf as the code build engine, and passthrough as
the default for desktop.

Recommended order:

1. L0 and L1: the breaking desktop cutover, so `./kiln play` replaces the
   launchers for everyday work.
2. L2, alongside L1 since it is independent: the codecs.
3. L3 for one mobile profile on this branch. That profile ships P2:CE BC7
   textures to a device without BC. Today the Galaxy Tab S8 class decodes
   them to RGBA8 on the CPU at load, so lowering removes that decode and
   cuts their memory.
4. After `render-3ds` merges, L3 extends to the 3DS, where it deletes the
   on-device ETC1 encoder.
