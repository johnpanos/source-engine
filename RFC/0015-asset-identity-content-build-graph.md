# RFC 0015: Asset Identity, Content Build Graph, Packages and Live Reload

- Status: Proposed (2026-09-26); no implementation gate complete. Open
  decisions 1–6 were answered on 2026-09-26 at the user's direction (see
  [Decisions](#decisions-2026-09-26))
- Date: 2026-09-26
- Scope: How every kind of game content is named, compiled, recorded,
  packaged, resolved at runtime and reloaded during development. This
  covers asset identity and references, one content build graph for all
  asset kinds, compiler contracts, a content-addressed output store, atomic
  publication, per-profile packages, a runtime asset index and a desktop
  live-reload loop.
- Formats: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md) owns
  the World Stage, BSP2, KTX2, the render lumps and the modern model
  resource (F9). This RFC owns how their producers run, cache and publish;
  it does not define map, texture or model encodings
- Map source: [RFC 0009](0009-usd-native-map-authoring.md) owns the editable
  USD map, its validator and the native map compiler
- Lighting: [RFC 0007](0007-physically-based-lighting-pipeline.md) owns the
  light baker contract and its providers
- Editor: [RFC 0002](0002-hammer-responsibility-factorization.md) owns the
  asset catalog, browser and build job presentation
- Platform: [RFC 0001](0001-capability-based-platform-architecture.md) owns
  virtual paths (R11), the tool-process contract (R40) and static
  composition
- Execution: [RFC 0003](0003-dependency-aware-job-system.md) owns the job
  graph executors this RFC's build graph runs on
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  (Q-CONTENT, Q-PRODUCT)

## Render performance obligations (user decision, 2026-10-01)

This RFC follows RFC 0016's
[high-performance clustered lighting requirement](0016-render-core.md#high-performance-clustered-lighting-user-decision-2026-10-01)
and [hard render budgets](0016-render-core.md#hard-render-budgets-user-decision-2026-10-01).
Computation placement follows RFC 0003's binding
[CPU/GPU policy](0003-dependency-aware-job-system.md#cpugpu-execution-placement-user-decision-2026-10-01).
Those owners define the policies and numbers; this RFC defines its domain's
obligations and does not certify implementation by this amendment.

Asset publication and live reload invalidate derived scene/light/shadow
resources by their owning revisions. Package GPU-ready resources through the
existing build graph and profile, and measure residency, upload and reload
costs without reintroducing per-frame legacy conversion or a second cache owner.

## Decision and boundary

Source 2 compiles every source asset in `content/` into a compiled resource
in `game/`. Each compiled resource records what it references and what it
was built from, and the tools recompile changed assets while the game is
running. This engine has pieces of that for maps only: two map compilers,
three map build scripts with separate caching rules, and a BSP2 container.
It has nothing that names an asset, records its references or dependencies,
or builds a package. This RFC adds that layer to the engine's own
formats. It does not copy Source 2's file formats.

1. **One asset identity.** An asset is named by `AssetRef` = (kind,
   canonical name). The name is the one game code and content use today
   (`models/props/crate.mdl`, `materials/tile/floor`), so legacy references
   stay valid. It is not a GUID and has no sidecar file.
2. **One build graph for every kind.** Maps, models, materials, textures,
   particles, sounds, scenes, captions and nav meshes are nodes of one
   content build graph, with one cache key rule, one output store and one
   publication step. RFC 0008's map pipeline (F6) is a client of this graph,
   not a separate graph.
3. **Compilers are providers.** Each asset kind has compilers that
   implement one contract (`content.asset-compiler.v1`): declared inputs,
   dependencies discovered during the compile, deterministic outputs, and
   reported references. In-process compilers and external tool processes
   (vbsp, vrad, Blender, KTX-Software) pass the same shared suite.
4. **Legacy formats pass through byte-identically.** MDL, VMT, VTF, PCF,
   soundscripts, WAV and BSP stay loadable by their existing loaders. A
   legacy asset takes part in the graph through a *passthrough compiler*.
   It validates the file, extracts its references and copies it unchanged.
   Legacy files are never wrapped in a new container.
5. **Packages carry an index.** A package is the reference closure of a set
   of root assets for one product profile. It carries an asset index
   (entries, variants, references, provenance). The runtime resolves
   profile variants and checks closure through that index. Legacy search
   paths remain for unindexed content.
6. **Live reload is a development composition.** On desktop, a watcher
   rebuilds changed assets through the same graph. It publishes them to an
   overlay package, and the running game reloads them at a frame boundary.
   Installed, store and mobile products contain no compiler, watcher or
   reload channel.

This RFC defines contracts, owners and gates. Nothing here is installed.

## Implementation progress (2026-10-03)

The first bounded C++ slice is installed, without claiming C0, C1 or C5's
full gates. `content/` owns the ten-kind `AssetRef` table, normalization and
BLAKE2b name hash; the common hash implementation moved out of
`mapcontainer/` without changing BSP2's hash algorithm. The C++ index
writer/reader uses the 64-byte block-container directory layout for a loose
development package, with an independent Python conformance reader at
`tools/quality/content_index_check.py`. The Python reader is an oracle, not a
second build or runtime authority.

`content.build-graph` has a serial compiler seam, declared input snapshots,
one BLAKE2b-128 action-key rule, a per-profile content-addressed store, and
atomic version-directory and `current`-symlink publication. The first compiler
is `material.vmt`: it uses the existing C++ KeyValues parser, records its
texture and patch-include references, and copies VMT bytes unchanged. Its
`texture.passthrough` dependency validates the legacy VTF header and copies
the file byte-identically. `content_build` is a C++ CLI over these libraries.
`content.asset-resolver` mounts the index, distinguishes indexed from
unindexed files, and reports a missing required reference with its referrer.
`render_lab`'s `GameFiles` is the first runtime caller: it asks that resolver
for indexed paths, verifies indexed bytes before decoding, and retains its
legacy loose-file search for unindexed content.

The direct C++ `contenttest` passes clean/miss/hit/edit change classes, byte preservation,
corrupt/truncated index, closure, failed-compile preservation, and two simultaneous
builds of one package. Cache and package staging use exclusive temporary directories
so concurrent writers do not share a partial file.
`lab_support.cpp` compiles with the new resolver. The independent Python
reader decoded the C++-produced test package. `archlint check --all`, baseline
verification and changed-line style checking passed. Isolated Waf configure
did not complete because the worktree lacks the pinned shader toolchain; the
submodule-dependent inventory check could not inspect the uninitialized
Box3D vendor paths. These are recorded as unavailable evidence, not passes.

Reproduce the direct native controls from the repository root:

```sh
g++ -std=c++20 -Ipublic content/hash.cpp content/asset_identity.cpp content/asset_index.cpp content/asset_resolver.cpp content/build_graph.cpp content/material_compiler.cpp kvtext/keyvalues.cpp unittests/contenttest/contenttest.cpp -o /tmp/contenttest-rfc0015
/tmp/contenttest-rfc0015
g++ -std=c++20 -Ipublic content/hash.cpp mapcontainer/map_container.cpp unittests/mapcontainertest/test_map_container.cpp -o /tmp/mapcontainertest-rfc0015
/tmp/mapcontainertest-rfc0015
python3 tools/archlint/archlint.py check --all
python3 tools/stylelint/stylelint.py --changed
```

The remaining C0–C1 gates include base VPK scans and closure reports,
full VMT/MDL/PCF/soundscript extraction, a shared compiler suite with bad
providers, pooled execution, cancellation/killed-build evidence, and
incremental-versus-clean corpus comparison. C2 still moves all three map
scripts onto node sequences and retires their private caches. C4 still needs
archive packages; C5 still needs `CTexture` variant selection, native Vulkan
pixel evidence and dedicated-server link evidence. Production Python map
entry points must become wrappers around the canonical C++ libraries or CLI
as their caller cohorts migrate.

## Observed starting point (2026-09-26)

Observed by reading source at `35e07f4b` plus the dirty tree. Nothing
here was measured for this RFC.

### Runtime

- **No asset identity.** No `AssetId`, resource handle or dependency record
  exists. Every kind is found by a string path plus a filesystem path ID
  (`GAME`, `MOD`, `BSP`), and each subsystem has its own cache:
  - models: `CMDLCache::FindMDL` (`datacache/mdlcache.cpp:909`), companion
    files by extension swap, and `models/error.mdl` on failure;
  - materials: `LoadVMTFile` (`materialsystem/cmaterial.cpp:3622`);
  - textures: `CTexture` builds `materials/%s.vtf`
    (`materialsystem/ctexture.cpp:3214`);
  - particles: `particles_manifest.txt` and per-map manifests
    (`game/shared/particle_parse.cpp`);
  - sounds: `scripts/game_sounds_manifest.txt`
    (`soundemittersystem/soundemittersystembase.cpp`) and
    `sound.cache`, rebuilt at runtime through a temporary path ID
    (`engine/audio/snd_wave_source.cpp:2576`);
  - scenes: `scenes/scenes.image`, looked up by CRC32 of the `.vcd` name
    (`scenefilecache/SceneFileCache.cpp`);
  - nav meshes: `maps/<map>.nav` from `MOD`, then from the map's pak
    (`game/server/nav_file.cpp:1377`).
- **Precache lists** are network string tables (`engine/precache.h`).
  `-makereslists` writes `reslists/<map>.lst`. These are runtime
  observations, not build-time reference records.
- **KTX2 can't be selected at runtime.** `texturecontainer::ReadKtx2Image`
  (`texturecontainer/ktx2_reader.cpp`) is called only by the Hammer preview
  and tests. `CTexture` knows only `.vtf`.
- **Paths contract.** `public/platform/contracts/paths.h` reports base
  locations only and states that asset search policy is out of scope. It
  has only test providers (R11).
- **Mobile content is pushed, not packaged.** Android pushes the
  profile's `content.directories` with `adb`
  (`build-android-apk.sh:536-575`). iOS copies content into Documents
  (`ios-deploy.sh`); tvOS uses `Library/Caches`. No app bundles game content.

### Build tooling

- **Three map build scripts, three caching rules:**
  - `tools/quality/pbrt_map_build.py` has a per-step cache (`steps.json`).
    Its key hashes the input files, the step settings, the script's import
    closure and each tool's identity (`step_key`, `script_closure`,
    `pbrt_map_toolchain.identity`). It moves old outputs aside and
    restores them on failure. Steps run serially, and `steps.json` isn't
    written atomically.
  - `tools/quality/usd_map_compile.py` gives each stage a private directory
    and records input and output hashes and a key. It doesn't cache, and
    scripts are not part of the key.
  - `tools/quality/vmf_map_build.py` has no cache. Hammer reaches it through
    `ToolProcessMapBuilder` and `MapBuildQueue`.
- **Publication isn't atomic.** `playable_maps.publish` renames the old map
  directory away and then renames the new one in. Between the two renames,
  no map directory exists.
- **Content resolution is hard-coded.** `source_content.ContentResolver`
  mirrors Portal's `gameinfo.txt` search paths in a fixed list
  (`tools/quality/source_content.py:16`). `usd_map_compile.py` writes a
  private `gameinfo.txt` with hard-coded VPK paths.
- **KTX2 packing** (`tools/texture/ktx2_pack.py`, `ktx2_select.py`) is
  atomic and profile-aware, but only conformance scripts call it.
- **Compilers in Waf:** `vtex` and `vtexconv`, `vbsp`, `vvis`, `vrad`,
  `vbsp2`, `bsp2tool` and `rtrntool`.
  - `utils/studiomdl`, `utils/captioncompiler`, `pcffix`, `dmxconvert` and
    `utils/scenemanager` have no wscript.
  - The `scenes.image` writer (`game/shared/sceneimage.cpp`) builds inside
    `choreoobjects`. Its `makescenesimage` driver, which
    `utils/itemtest_lib` launches, isn't in the tree.
  - Nav meshes are generated only in-game (`nav_generate`).
  - No new model can be compiled on Linux.
- **No job system in content tools.** None of them uses `public/jobsystem`.
  vvis and vrad use their own threads (`utils/common/threads_linux.cpp`).
- **Library layering.** `world.map-container` (`mapcontainer/`) owns the
  BSP2 block layout: 4CC directory, 64-bit offsets, BLAKE2b-128 hashes and
  alignment. `content.keyvalues-text`, `content.vmf`,
  `content.texture-contract`, `content.ktx2-reader` and
  `content.vtf-reader` are the existing content modules.

## Goals

- One `AssetRef` for every asset kind. Legacy names stay valid as they are.
- One build graph with one cache key rule, one output store and one atomic
  publication. It runs in-process for the editor and the CLI, and it has a
  serial mode that serves as its oracle.
- Every compiler reports the inputs it reads and the references it emits.
  A compiler that reads an undeclared input fails a test.
- An incremental build produces the same bytes as a clean build, and a
  change rebuilds exactly the nodes that depend on it.
- Per-profile packages that carry exactly their reference closure, with
  profile-specific variants such as BC7 or ASTC textures.
- A runtime asset index that selects variants, reports a missing reference
  with its referrer, and gives map loads their closure.
- A desktop edit → rebuild → reload loop measured in seconds, with no stale
  state compared with a cold start.
- Each legacy kind either has an adopted compiler that builds under Waf or
  a recorded decision to stay passthrough-only.
- The canonical asset identity, index, compiler contract, build graph and
  runtime resolver live in C++ libraries. Python commands used by existing
  map and quality workflows are thin wrappers around those libraries or their
  CLI, with independent Python readers reserved for conformance checks. Python
  must not become a second owner of key, normalization, reference or package
  policy. Migrate the existing Python map scripts as bounded caller cohorts.

## Non-goals

- New particle, sound, animation-graph or choreography formats or editors.
  Those kinds stay in their legacy formats here. Changing any of them needs
  its own RFC.
- Replacing VMT. RFC 0008 keeps VMT as the material file.
- Shader compilation. `tools/quality/shader_artifacts.py` owns shaders, and
  they may become graph nodes later without changing this RFC.
- A distributed or remote cache, remote execution, or a build farm.
- Content signing, attestation, DRM or a custom updater (AGENTS.md
  distribution rules). Store-managed updates stay the executable path.
- Compiling content on mobile devices, or shipping any compiler in an
  installed product.
- Converting legacy content by default. Passthrough is byte-identical.

## Layers and owners

| Layer | Module (proposed) | Owns | Depends on |
| --- | --- | --- | --- |
| Block container | `content.block-container` | The 4CC block layout, hashes and alignment, extracted from `mapcontainer/`. BSP2 and the new files each use it under their own magic | foundation |
| Asset identity | `content.asset-identity` | `AssetKind`, the kind table, name normalization, `AssetRef`, the 64-bit name hash | foundation |
| Asset index | `content.asset-index` | The package index format, a reader and a writer | identity, block container |
| Compiler contract | `content.asset-compiler` | `IAssetCompiler`, `IBuildInputs`, compile records | identity |
| Build graph | `content.build-graph` | Nodes, keys, the store, executors, publication and traces | compiler contract, index, `jobsystem`, `platform.tool-process` |
| Compilers | one module per kind (for example `content.compile.texture`) | One kind's compile or passthrough | compiler contract, that kind's format library |
| Runtime resolution | `content.asset-resolver` | Mounting package indexes, variant selection and closure queries | asset index |
| Live reload | `content.live-reload` (desktop development composition only) | The watcher, overlay package, change notices and reloader registration | build graph, resolver |
| Applications | `content_build` CLI, Hammer, engine roots, `./play` | Composing providers, choosing profiles, presentation | the layers above |

Arrows point down. Format libraries (`world.map-container`, KTX2, VMF,
KeyValues, the World Stage) never depend on the build graph. The runtime
resolver never depends on a compiler. The dedicated server links the
identity, index and resolver layers only, with no render or texture
dependency (R12 link evidence).

## Asset identity (`content.asset-identity.v1`)

### Kinds and names

The kind table is the one owner of each kind's naming rule. A name is the
string game code and content already use:

| Kind | Canonical name | Runtime files today | Loader today |
| --- | --- | --- | --- |
| `model` | `models/…/x.mdl` | `.mdl`, `.vvd`, `.*.vtx`, `.phy`, `.ani` | `CMDLCache` |
| `material` | `materials/…/x` (no extension) | `.vmt` | `LoadVMTFile` |
| `texture` | `materials/…/x` (no extension) | `.vtf`; `.ktx2` after C5 | `CTexture` |
| `particle-file` | `particles/…/x.pcf` | binary DMX | `CParticleSystemMgr` |
| `soundscript` | `scripts/…/x.txt` | KeyValues | `CSoundEmitterSystemBase` |
| `sound` | `sound/…/x.wav` (or `.mp3`) | wave data | `snd_wave_source` |
| `scene` | `scenes/…/x.vcd` | an entry of `scenes/scenes.image` | `CSceneFileCache` |
| `caption` | `resource/closecaption_<lang>` | `.dat` | `hud_closecaption` |
| `nav` | `maps/<map>.nav` | nav file | `CNavMesh::Load` |
| `map` | `maps/<map>.bsp` (either container) | BSP v19–21 or BSP2 | `IMapContainer` |

- **Normalization.** Names are UTF-8 with `/` separators and no `.`, `..`,
  empty or absolute components. They are lowercased with ASCII rules. Two
  sources that normalize to the same `AssetRef` are a build error; neither
  wins by search order. Names compiled from new sources must also use the
  portable character set in [decision 2](#2-names-ascii-lowercase-identity-and-a-portable-set-for-new-content).
  Material-relative references in a VMT, such as
  `$basetexture brick/wall`, gain their `materials/` prefix in the
  material compiler, not in each consumer.
- **Hash.** `AssetNameHash` is 64 bits of BLAKE2b over the kind and the
  normalized name. The index writer fails on a collision. A hash is a
  lookup key, never an identity on its own.
- **Modern kinds** such as RFC 0008 F9's model resource get their own kind
  rows and name forms when their contracts are recorded. A modern kind
  never reuses a legacy kind's name for a different format.
- **Renames** are a new identity plus an optional redirect record in the
  package index. The loader follows at most one redirect and logs it.

### References

`AssetRef` values appear in two places, which Source 2 also separates:

- **Runtime references:** the assets a compiled asset needs when loaded
  (model → materials; material → textures; soundscript → sounds;
  map → models, materials, particles, sounds, nav). They define a package's
  closure and a map's prefetch set.
- **Build dependencies:** everything a compile read, including source files,
  includes, tool identities and profile facts. They define cache
  invalidation and are recorded per node, not in the shipped asset.

A reference to an asset that the package doesn't contain and no declared
base package provides is a build error that names the referrer. A reference
that is optional by the kind's contract (for example a material's
`$bumpmap` fallback) is recorded as optional.

## Compilers (`content.asset-compiler.v1`)

```cpp
// Proposed; spellings are fixed by the first implementation.
class IAssetCompiler
{
public:
	virtual CompilerIdentity Identity() const = 0; // id, version, binary/script digest
	virtual AssetKindSet Produces() const = 0;
	virtual Expected<CompilePlan, CompileError> Plan( const CompileRequest &request,
	    IBuildInputs &inputs ) = 0;              // declared inputs and outputs
	virtual Expected<CompileResult, CompileError> Compile( const CompilePlan &plan,
	    IBuildInputs &inputs, IStagingOutputs &outputs, ICancellation &cancel ) = 0;
};
```

- **Inputs are read only through `IBuildInputs`.** The inputs are asset
  sources, other nodes' outputs and legacy entries in a VPK. Every read is
  recorded with its content hash. The record lists the dependencies
  discovered during a compile, such as QC includes or a VMT's `include`, and
  those dependencies take part in the next key check.
- **Outputs are written only to a private staging area.** The compile
  result lists each output with its hash, its `AssetRef`, its runtime
  references and a diagnostics list.
- **External tools are compilers too.** A tool process such as vbsp, vrad,
  Blender or `ktx` runs through the R40 tool-process contract in a staging
  root. The root holds only the declared inputs, read-only, the pinned
  toolchain and an output directory. The environment and argv are
  structured values. A read outside the root fails because the file isn't
  there, and the shared suite checks that.
- **Determinism is declared.** A compiler is `Exact` when the same inputs,
  identity and profile give identical bytes, or `Statistical` when outputs
  are reproducible only within a declared tolerance. Cycles bakes, for
  example, are statistical unless seeded exactly. A statistical compiler's
  outputs are cached by key but are never compared byte for byte in the
  incremental-equals-clean oracle; they use the domain's own tolerance.
- **Cancellation** returns promptly with `CompileError::Cancelled`. It
  publishes nothing, and the staging area is discarded.
- **Ownership.** Compilers keep no state between compiles, use no global
  search paths and never write to the source tree.

### Shared suite and bad providers

One suite runs against every claiming compiler, including fakes. The
deliberately bad compilers below must each fail:

- reads a file it didn't declare (an absolute path into the source tree);
- omits a runtime reference that its output contains;
- produces different bytes for the same inputs while declaring itself
  `Exact`;
- ignores its tool identity, so a changed binary gives a stale cache hit;
- writes outside its staging area;
- ignores cancellation;
- reports success after a partial write.

## Build graph (`content.build-graph.v1`)

### Nodes and keys

A node is (compiler, `AssetRef` or named aggregate, profile). Its key is a
BLAKE2b-128 hash over:

- the compiler identity (id, version, and the digest of its binary or its
  script closure, as `pbrt_map_build.py` computes today);
- the profile facts the compiler declares that it reads, not the whole
  profile, so an unrelated profile edit doesn't rebuild everything;
- the parameters;
- the sorted set of (input, content hash), including the dependencies
  discovered and recorded by the previous compile.

The graph checks the recorded discovered dependencies before it decides a
node is fresh. A node whose inputs are unchanged is a cache hit, and its
outputs come from the store without running the compiler. The hash
algorithm is the block container's, so RFC 0008 open decision 2 decides it
once. The F1 prototype's BLAKE2b-128 is the starting point.

### Store and publication

- **The store** is content-addressed per profile under the build output
  directory, for example `build-content/<profile>/store/`. An action record
  maps each key to its output manifest. The store never holds source files.
- **A build request** names root assets (a map, a package list or a single
  asset) and a profile. It expands the reference closure, plans the nodes,
  executes them and writes a package version to
  `packages/<package>/<version>/`, where the version is a hash of the
  request and its outputs.
- **Publication** swaps a `current` symbolic link to the new version with
  one `rename(2)` of a temporary link. It replaces `playable_maps.publish`'s
  two renames. A failed or cancelled request never swaps. The previous
  version stays until a retention policy removes it.
- **The previous package stays intact** through failure, cancellation and a
  killed build. Q-CONTENT injects each of these.

### Execution

- **In-process compilers** run as nodes of an RFC 0003 job graph on an
  injected pool. `DeterministicExecutor`, the serial mode, is the oracle, and
  serial and parallel runs must publish identical packages.
- **External tools** are started through the tool-process provider. Their
  completion arrives on a sequence runner, so no worker blocks waiting on a
  process.
- **Resource classes** bound concurrency. For example, `cycles-bake` is
  exclusive, and `cpu-heavy` shares the process-wide worker budget. The
  classes are declared per compiler and set per profile, and they share
  the one worker budget AGENTS.md requires.
- **Traces.** Every request writes a trace: nodes, keys, hit or miss, the
  first changed input for each miss, durations and the executor used.
  `content_build explain <asset>` prints why an asset was rebuilt.

### Map pipelines on the graph

RFC 0008's conversion ledger names one owner per map transformation. Each
row becomes one node kind with that owner's compiler:

- `vbsp2`, the RFC 0009 native USD compiler, `vvis`, the `ILightBaker`
  providers, the texture encoder and transcoder, `WorldPacker` and
  `LegacyLightingExporter`;
- the `pbrt_map_build.py` steps, the `usd_map_compile.py` stages and
  `vmf_map_build.py` become node sequences. Their `steps.json`, stage keys
  and publication code are removed when each pipeline runs on the graph.

RFC 0008 still owns the ledger rows and their cache-key contents. This RFC
owns how those keys are computed and stored.

## Legacy kinds: adoption decisions

Agent decision under the user's standing instruction, 2026-09-26. The
recommendation is to keep every legacy format and bring each kind into the
graph at the cheapest level that gives closure and references:

| Kind | Decision | First compiler |
| --- | --- | --- |
| `model` | Port `studiomdl` to Waf (`tools` group) as the legacy model compiler, QC/SMD/DMX → MDL; passthrough for shipped MDLs | `model.studiomdl` and `model.passthrough` (references: `$cdmaterials` × skin textures, `$includemodel`) |
| `material` | VMT stays source and runtime; the compiler validates and copies | `material.vmt` (KeyValues parse through `content.keyvalues-text`, shader known, texture references resolved, `include`/patch followed as build dependencies) |
| `texture` | Legacy VTF through `vtex`; KTX2 variants through the RFC 0008 encoder and transcoder, chosen by the profile's texture pipeline | `texture.vtex`, `texture.ktx2` (wraps `ktx2_pack.py`), `texture.passthrough` |
| `particle-file` | Passthrough with reference extraction (materials, child systems, models) | `particle.passthrough` (binary DMX read through `dmxloader`) |
| `soundscript`, `sound` | Passthrough; the soundscript compiler records wave references. A package-time `sound.cache` node replaces the runtime rebuild in C4 | `soundscript.passthrough`, `sound.passthrough` |
| `scene` | Build `scenes.image` as an aggregate node from VCDs, using the writer extracted into `content.scene-image` (decision 6) | `scene.image` |
| `caption` | Port `captioncompiler` to Waf | `caption.compile` |
| `nav` | An optional node per game that runs the headless dedicated server's `nav_generate` against the compiled map; shipped `.nav` files pass through | `nav.generate`, `nav.passthrough` |
| `map` | Existing compilers become node sequences (see above); shipped BSPs pass through | per the RFC 0008 ledger |

A kind without an adopted compiler is packaged as *unindexed* content. It is
copied with a recorded reason and counted in the package report. That count
is an exact ratchet per package. Unindexed content can't take part in
closure checks, so no gate that requires closure can pass with it.

## Packages (`content.package-index.v1`)

- **A package** is the output of one build request. It holds the asset
  index, the compiled or passthrough files, and a report.
- **The index** is a block-container file with its own magic. Its blocks
  record:
  - the package header: package id, profile, revision and request hash;
  - one entry per asset: `AssetRef`, name hash, variant (for example
    `bc7` or `astc-4x4`), location, sizes and content hash;
  - runtime reference edges, with optional ones marked;
  - redirects;
  - provenance: compiler identity and key per entry.
  
  A Python reader in `tools/quality/`, sharing no code with the C++ reader,
  validates it the way the BSP2 reader does. `content_build dump` writes it
  as JSON for review.
- **Base packages** such as the shipped Portal and HL2 VPKs are indexed once
  per archive hash. A game package declares its base packages and must
  close over itself plus those bases.
- **Profiles** own the facts: texture formats, which kinds are required,
  and packaging, meaning directory, VPK or platform package. They live in
  the existing product profile files (`quality/product_profiles/`), not in
  a new registry.
- **Mobile packages** (RFC 0008 F7, R58) are profile packages written into
  the platform's normal container: APK assets or Play asset delivery on
  Android, and the app bundle or its container directory on iOS. This
  replaces `adb push` and `ios-deploy --with-content` as the release path.
  Those commands remain development shortcuts.

[Decision 1](#1-package-form-block-container-archives-vpk-stays-read-only)
fixes the on-disk forms: an archive for published packages and a loose
directory for development overlays. [Decision 3](#3-retention-the-store-is-a-cache-roots-one-rollback-and-leases)
fixes retention and garbage collection.

## Runtime resolution (`content.asset-resolver.v1`)

- **Mounting.** The filesystem owner mounts each package's index when it
  mounts the package. The resolver answers `Find(AssetRef, variant
  preference)`, `References(entry)` and `Closure(roots)`. It is read-only
  after mounting and safe to share across threads once published, so
  readers need no lock.
- **Loaders** keep their names and entry points. The first consumer is
  texture variant selection: `CTexture` asks the resolver for the best
  variant the device supports, which closes R55's "material-system file
  selection" item. Models, particles and sounds follow as separate
  cohorts.
- **Missing references.** When an indexed package lacks an asset, the
  error names the referrer, which is known from the index. The legacy
  error fallback (`models/error.mdl`, the missing-texture checker) is kept
  and logged with that name.
- **Unindexed content** still loads through legacy search paths. The
  resolver reports it as unindexed and never pretends it closed.
- **Map-load prefetch** reads a map's closure from the index and issues
  reads as a job graph: decode on the pool, commit on the owning thread. A
  budget is set before it becomes a default (AGENTS.md: measure first).
- **Static composition.** Mobile static products link the resolver and
  the index reader as ordinary modules. They never link a compiler.

## Live reload (`content.live-reload.v1`, desktop development only)

- **The loop.**
  1. A watcher on the content roots (a platform provider, inotify on
     Linux) coalesces changes and submits a build request for the affected
     roots.
  2. Changed outputs publish to an overlay package mounted above the
     base.
  3. A change notice `{revision, [(AssetRef, old hash, new hash)]}` goes
     to each subscribed game over the watcher's endpoint
     ([decision 4](#4-reload-channel-a-dedicated-endpoint-owned-by-the-watcher)).
     Hammer's play-in-editor child subscribes like any other game.
- **Reloaders.** The owning subsystem registers a reloader per kind: the
  material system for materials and textures, `mdlcache` for models, the
  particle manager, and the sound emitter. A reloader runs at a frame
  boundary on the owning sequence.
  - A reloader keeps object identity where consumers hold pointers.
    `IMaterial*` and `studiohdr_t*` are examples; the material system
    already reloads in place.
  - A kind without a safe reloader declares `restart-required`. Maps
    declare it until RFC 0008 F11.
- **The oracle.** After an edit → reload sequence, the game's observable
  state and a captured frame equal a cold start with the same package
  version. A reloader that leaves a stale texture, a stale model bound or
  a leaked handle fails.
- **Budget.** Edit-to-visible latency is measured per kind on the declared
  desktop profile before any default is set. The first targets are a
  material parameter and a texture.
- **F11 uses this loop.** RFC 0008 F11's USD map iteration goes through the
  same watcher, graph, overlay and notice. It does not get its own loader
  pipeline.

## Editor and agents

- **The asset catalog** (RFC 0002 state table: "Asset identity and metadata
  revision") is the resolver plus the index of the workspace's packages.
  It is not a separate scan of search paths. The browser, thumbnails and
  material previews key off `AssetRef` and content hash.
- **Builds.** Hammer builds through the same library. `IMapBuilder` gains a
  graph-backed provider, and `MapBuildQueue` keeps its threading. The
  `ToolProcessMapBuilder` → `vmf_map_build.py` path is deleted when the VMF
  pipeline runs on the graph.
- **MCP.** The command catalog gains `find_asset`, `asset_references` and
  `build_assets`, so agents use the same authority (`hammer_cli --mcp`).

## Delivery plan and gates

Each phase lists its gate. Every gate needs negative controls, recorded
evidence (revision, profile, inputs, counts, first divergence and
reproduction commands) and the AGENTS.md reporting rules.

### C0: Identity, index and reference extraction

- Deliver the kind table, name normalization, `AssetRef` and its hash, the
  block container extracted from `mapcontainer/` (BSP2 bytes unchanged),
  the index writer and reader, and the independent Python reader.
- Add passthrough reference extraction for MDL, VMT, PCF and soundscripts.
- Build the eagerly hashed base-package indexes (decision 5) and run the
  case-fold scan (decision 2).
- Gate:
  - Base-package indexes of the Portal and HL2 VPKs, with their one-time
    hashing cost recorded, plus a closure report for each Portal map.
  - Every case-fold collision found by the scan has a reviewed redirect.
  - The BSP2 corpus stays byte-identical after the container extraction.
  - Negative fixtures: a dangling reference, a case-fold collision, a
    hash collision (forced by a test hash), an unknown kind, a truncated
    index and a wrong block hash.

### C1: Build graph core

- Deliver the compiler contract and shared suite with the bad compilers
  listed above.
- Deliver the store, action records, publication, the serial and pooled
  executors, traces, and the `content_build` CLI.
- First compilers: `material.vmt`, `texture.ktx2` (external, wrapping
  `ktx2_pack.py`) and the C0 passthroughs.
- Gate:
  - An incremental build is byte-identical to a clean build for `Exact`
    nodes.
  - Change-class traces: touching a texture rebuilds that texture and
    the package only; touching a VMT rebuilds the material and the package
    only.
  - Serial and pooled runs publish identical packages.
  - Cancellation and a killed build leave `current` unchanged.
  - Every bad compiler is caught.

### C2: Map pipelines on the graph (RFC 0008 F6, R57)

- Move `usd_map_compile.py`, `vmf_map_build.py` and `pbrt_map_build.py`
  onto node sequences.
- Point Hammer's F9 at the graph.
- Remove their private caches and `playable_maps.publish`'s two-rename
  commit.
- Gate:
  - RFC 0008's incremental-build criterion (a light-only change runs
    bake and pack only; a texture-only change runs encode, transcode and
    pack only).
  - RFC 0009 acceptance item 5.
  - `corpus.hammer.loop` and `corpus.usd-map.compiler` pass on the graph
    with their existing oracles.
  - The loop time is recorded next to the current 9 s author → playable
    time.

### C3: Legacy compiler adoption

- One child per kind: `studiomdl` on Waf; `captioncompiler` on Waf; the
  `content.scene-image` library and the `scene.image` compiler; the
  `nav.generate` node; the `sound.cache` package node.
- Gate, per kind:
  - The tool's outputs are byte-identical to the shipped or legacy-built
    file for a versioned source corpus, or the difference is explained
    per field.
  - The kind's loader accepts the output in client and dedicated
    products.
  - A seeded source defect fails the compile.

### C4: Profile packages

- Build closure packages per product profile, with per-profile texture
  variants.
- Write desktop packages for `./play`, and mobile packages in the
  platform's container. This supplies the mechanics for RFC 0008 F7.
- Gate:
  - Each package's closure equals its index's reference closure.
  - The unindexed ratchet holds.
  - Installed-package smoke tests pass on the desktop profile. For
    mobile, see R58 and R29.
  - A package that is missing a required profile format fails the build,
    not the device.

### C5: Runtime resolver

- Mount the index; select texture variants in `CTexture`; report missing
  references with their referrer.
- Add map-load closure prefetch behind a switch, with a serial mode.
- Gate:
  - A BSP2 map with KTX2 variants renders on native Vulkan through
    ordinary material loads, and the pixel families match their fixtures.
  - The dedicated server's link evidence shows no render dependency.
  - A package missing an entry reports that entry and its referrer.
  - Prefetch on and off gives identical loaded state, and its load cost is
    measured.

### C6: Live development loop

- Deliver the watcher, overlay package, change notices, and the material
  and texture reloaders, then models, particles and sounds as cohorts.
- Add the editor asset browser over the index, and the MCP tools.
- Gate:
  - Reload equals a cold start (the oracle above) for each cohort.
  - Seeded stale-reload reloaders are detected.
  - Edit-to-visible latency meets its recorded budget on the desktop
    profile.
  - Installed and mobile products contain no watcher, compiler or reload
    channel (link map and package inspection).

C0 and C1 are ready now: R02, R05 and R10 are done. C2 needs C1. C3 needs
C1, and its children are independent of one another. C4 needs C1, and its
installed smoke test with texture variants needs C5. C5 needs C1, and its
KTX2 consumer needs R55's native formats. C6 needs C1 and C5.

## Roadmap

AGENTS.md owns ranks and states. Agent decision under the user's standing
instruction, 2026-09-26: rows are added as `planned`.

| Phases | Row |
| --- | --- |
| C0–C1 | R81, ranked directly before R57; R62 depends on it |
| C2 | R57, re-scoped from RFC 0008 F6 to carry C2; it now depends on R81, and R60 depends on it |
| C3 | R82 |
| C4 | R84; R58 depends on it |
| C5 | R83 |
| C6 | R85; R64 depends on it |

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| The graph becomes a second build system beside Waf | Waf keeps building code and tools; the graph builds content only and consumes tool installs as pinned identities. Neither calls into the other's configuration |
| Hashing large legacy VPKs is slow | Index base archives once per archive hash; memoize file hashes by (path, size, mtime) as `pbrt_map_build.py` already does; measure on the Portal corpus in C0 |
| Hidden inputs make stale caches | Staged roots for external tools, `IBuildInputs` for in-process compilers, and bad compilers in the shared suite |
| Statistical compilers break the incremental-equals-clean oracle | Determinism is declared per compiler; statistical outputs use their domain tolerance, and seeded exact modes are preferred |
| Name normalization changes behavior for mixed-case legacy content | The C0 corpus scan lists every case-fold collision before normalization is enforced; collisions are resolved as reviewed decisions |
| Live reload leaves stale state | The reload-equals-cold-start oracle, kinds that aren't safe declare `restart-required`, and no reload channel in installed products |
| The archive reader is slower than `CPackedStore` | C4 measures load time and memory against VPK on desktop and the Fold7, and a regression blocks the gate until the reader is fixed (decision 1) |
| The store grows without bound, or GC deletes live data | Mark and sweep from kept versions, leases for running sessions, and a size budget (decision 3) |

## Alternatives considered

### Extend `pbrt_map_build.py`'s step cache to everything

Its key rule is good and is kept: input hashes, the script closure and tool
identities. But it covers one map pipeline and is serial Python. The editor
and runtime can't call it in-process, and it has no reference records,
packages or runtime side. Rejected as the owner; kept as prior art for the
key.

### Use Waf, Bazel or ninja for content

Waf is the engine's code build entry point (AGENTS.md). Content
dependencies are found by reading asset bytes: QC includes, VMT textures,
map entity references. They vary by profile, and the editor and a running
game must drive builds in-process. Bazel's sandboxing is the right idea, and
the staged roots here copy it. But Bazel would need a build file per asset
and a second toolchain authority. Ninja keys on mtime and needs a
generator. Rejected.

### Source 2's layout: compiled `_c` files next to sources

A single `game/` tree can't hold several profiles' variants, such as BC7
and ASTC, side by side without renaming. It also mixes build outputs with
content that is under version control. Per-profile stores and packages are
used instead.

### GUID asset identity with sidecar files

GUIDs survive renames, but every legacy asset would need a sidecar file,
and game code and content reference assets by path. Paths plus redirect
records keep legacy references valid.

### Wrap legacy files in a resource container

A wrapper would change bytes that legacy loaders, tools and old engines
read, and it adds nothing the index can't record beside the file.
Rejected.

## Decisions (2026-09-26)

The six open decisions were answered at the user's direction ("answer the
open questions given what will pay off in the long term"). Each answer
favors one owner, one reader and no later format migration over the
cheapest first slice. The evidence each one names is a gate check on the
phase that installs it. A failing measurement is fixed in the
implementation; it doesn't reopen the decision.

### 1. Package form: block-container archives; VPK stays read-only

Three forms, by use:

- **Published packages** are a block-container archive: the
  `content.block-container` layout under its own magic, which the first
  fixture spells.
  - The index is one block, and each file is one entry.
  - Entries of GPU-bulk kinds (KTX2 variants, mesh payloads) are aligned
    to 4096 bytes; others to 16.
  - Every entry has a BLAKE2b-128 hash and a compression flag.
  - Entries stay uncompressed until zstd is pinned (RFC 0008 open decision
    2). Bulk GPU data stays uncompressed even then, so it can be mapped and
    uploaded directly.
- **Development overlays**, the live-reload output, are a loose directory
  with the same index file. An edit then writes one file, not a new
  archive.
- **Shipped base content**, meaning Valve's VPKs, stays VPK. It is mounted
  read-only by the existing `CPackedStore` and never written by the graph.

The filesystem mounts an archive as a pack backend next to `CPackedStore`,
so a legacy loader opens an entry by its legacy path without changing.

On Android, the archive is stored uncompressed in the APK (`noCompress`),
so it can be mapped at its asset offset. Content over the base APK size
limit goes in install-time Play Asset Delivery packs holding the same
archive. On iOS the archive goes in the app bundle.

Why this pays off:

- Maps and packages share one container library, one independent reader
  and one fuzz corpus.
- VPK entries carry only a CRC32, with no alignment and no per-entry
  compression. Direct upload on mobile needs both alignment and strong
  hashes.
- Writing packages doesn't depend on Valve's VPK tools.

Gate evidence (C4): load time and memory of a package compared with the
same content in VPK, on desktop and on the Fold7. A regression blocks C4
until the reader is fixed.

### 2. Names: ASCII-lowercase identity and a portable set for new content

- **Identity** is the name lowercased with ASCII rules. Matching is
  case-insensitive, and the original spelling is kept only for
  diagnostics.
  - There is no Unicode case folding. It depends on locale and differs
    between platforms.
  - Backslashes become `/`.
- **Legacy names** are accepted as they are after normalization.
- **New names** must use the portable set. This covers every name the
  graph compiles from a source in a content root, including USD maps and
  new kinds.
  - Each component uses `[a-z0-9_.-]`.
  - No component is empty, `.` or `..`, or ends in `.`.
  - No component is a Windows reserved device name (`con`, `nul`, `com1`
    and so on).
  - The compiler rejects a name outside the set, so content stays valid
    on case-sensitive filesystems (ext4, Android storage) and
    case-insensitive ones (APFS, NTFS).

Why this pays off:

- The engine already treats names case-insensitively: `CMDLCache`'s
  dictionary and the filesystem.
- The compile tools' failures on uppercase paths show that mixed case keeps
  costing time.
- Restricting new names costs nothing now and would be impossible once
  content exists.

Gate evidence (C0): a scan of the base VPKs and fixtures lists every
case-fold collision. Each collision needs a reviewed redirect before the
rule is enforced.

### 3. Retention: the store is a cache; roots, one rollback and leases

- **The store is never an authority.** Deleting it costs only rebuild
  time.
- **Kept per package:** `current`, the one previous version (the rollback
  target), and any version with a live lease. A running game or editor
  session writes a lease holding its process id and removes it on exit. A
  lease whose process is gone is ignored.
- **Garbage collection** is mark and sweep:
  - It marks from the kept versions and from action records used in the
    last 14 days. Last use is recorded in the action record, not taken
    from file access times.
  - It then evicts least-recently-used entries until the store is under
    the workspace's size budget. The default is 64 GiB, set in the
    workspace configuration.
  - It runs after a publish that exceeds the budget, and on
    `content_build gc`.
- **Concurrency.** Store writes are a temporary file plus a rename. GC
  holds the store lock and never deletes an object written after its mark
  began.

Why this pays off:

- With content-addressed data, sweeping from roots is the only correct way
  to collect garbage.
- One previous version gives the rollback that RFC 0005 requires without
  unbounded growth.
- Leases stop GC from deleting files that a running game has mapped.

### 4. Reload channel: a dedicated endpoint owned by the watcher

- **Endpoint.** `content_build watch` owns the `content.live-reload.v1`
  endpoint. The play-in-editor bridge doesn't.
- **Transport.** A Unix domain socket in the per-user runtime directory
  (`$XDG_RUNTIME_DIR` on Linux, the per-user temporary directory on
  macOS). The name is short and derived from the workspace, so it stays
  under the socket path limit.
- **Messages** are newline-delimited JSON-RPC 2.0 notifications, the same
  framing as `hammer_cli --mcp`. They carry identities and the overlay
  version, never file bytes. The game reads data from the mounted overlay.
- **Clients.** Games and the editor connect as clients and subscribe, and
  any number can subscribe. Hammer's play-in-editor child gets the
  endpoint path in its environment.
- **Failure handling.** The protocol is versioned, and unknown messages
  are ignored. A disconnect means no reloads, never a crash.
- **Products.** Installed and mobile products don't compile the endpoint
  in. The optional Windows profile uses a named pipe with the same framing
  if it ever needs reload.
- **JSON library.** The MCP adapter's private JSON value becomes a small
  shared library when this second consumer arrives.

Why this pays off:

- Reload works the same for `./play` and for editor-launched games.
- Asset notices stay separate from frames and input, which have their own
  lifecycle.
- MCP and reload share one message framing.

### 5. Base archives: hash eagerly, once per archive

- **One full hash per archive.** A base archive is hashed in full the
  first time it is seen.
- **The result is a base-package index**, an ordinary package-index file.
  It is keyed by the hash of the archive's directory file, plus each
  chunk's hash memoized by size and mtime. It is rebuilt only when an
  archive changes.
- **It is a cached graph output.** Products may ship it so the runtime
  resolver treats base content as indexed.

Why this pays off:

- Cache keys and closures need a content hash for every legacy input.
- Lazy hashing would make first-build keys depend on which entries a
  build happened to read, and would make build time unpredictable.
- CRC32 isn't strong enough to address content.

Gate evidence (C0): record the one-time cost for the Portal and HL2 VPKs
as a budget row.

### 6. Scene images: a format library, with the compiler as its driver

Correction to the starting point: the writer already builds.
`CSceneImage::CreateSceneImageFile` (`game/shared/sceneimage.cpp`) is
compiled into the Waf `choreoobjects` library. What is missing is the
`makescenesimage` driver that `utils/itemtest_lib` launches.

- **A new `content.scene-image` library** takes that writer and the format
  in `public/scenefilecache/SceneImageFile.h`. It has no game dependency.
  Its `scriplib`/`cmdlib` file enumeration is replaced by `IBuildInputs`.
- **The `scene.image` compiler is the driver.** `makescenesimage` isn't
  ported.
- **Readers.** `CSceneFileCache` keeps reading as it does now. Moving it
  onto the library's reader is a later cohort.
- **`choreoobjects`** keeps the choreography objects and calls the library.

Why this pays off: one owner for the format's writer and reader, following
the layered-library rule. The graph can also record each VCD as a build
dependency of the image.

## Source references

- `datacache/mdlcache.cpp`, `materialsystem/cmaterial.cpp`,
  `materialsystem/ctexture.cpp`, `game/shared/particle_parse.cpp`,
  `particles/particles.cpp`, `soundemittersystem/soundemittersystembase.cpp`,
  `engine/audio/snd_wave_source.cpp`, `scenefilecache/SceneFileCache.cpp`,
  `game/client/hud_closecaption.cpp`, `game/server/nav_file.cpp`
- `engine/precache.h`, `engine/MapReslistGenerator.cpp`,
  `filesystem/basefilesystem.cpp`, `public/filesystem_init.cpp`
- `public/mapcontainer/map_container_format.h`, `content/hash.cpp`,
  `public/texturecontainer/texture_image.h`
- `tools/quality/pbrt_map_build.py`, `tools/quality/usd_map_compile.py`,
  `tools/quality/vmf_map_build.py`, `tools/quality/playable_maps.py`,
  `tools/quality/source_content.py`, `tools/texture/ktx2_pack.py`,
  `tools/texture/ktx2_select.py`
- `public/hammer/ports/map_builder.h`,
  `hammer/adapters/platform/tool_process_map_builder.cpp`,
  `public/hammer/app/map_build_queue.h`
- `build-android-apk.sh`, `build-ios-app.sh`, `launcher_main/android_main.cpp`,
  `launcher_main/ios_main.cpp`

## Proposed decision

Adopt one asset identity, one content build graph with provider compilers,
per-profile packages with an asset index, and a desktop-only live-reload
loop. Keep every legacy format loadable by passing it through byte for
byte. RFC 0008's F6 becomes this graph's first large client. F7, F9 and F11
use its packages, compiler contract and reload loop. The recommended first
work is C0 and C1 (R81), because both are testable on their own and every
later map, model and packaging row depends on them.
