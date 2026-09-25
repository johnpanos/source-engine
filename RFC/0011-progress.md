# RFC 0011 progress: Runtime indirect lighting

Updated: 2026-09-24
Rows: RFC 0011 proposes R70–R78 (G0–G8). They are not yet ranked in
AGENTS.md. On 2026-09-24 the user directed implementation of G0 through G8 in
order; this file is the gate-decision record for that work.

Where this file disagrees with the versioned artifacts, the artifacts win:

- the conformance manifest rows named below;
- the fixture declarations under `quality/fixtures/gi/`;
- local evidence under `quality-results/rfc0011-*` (not versioned; the
  reproduction commands regenerate it).

## Gate status

| Gate | State | Summary |
| --- | --- | --- |
| G0 baseline, fixtures and runner | done (2026-09-24) | Six fixtures with Cycles total/indirect references; GPU runner; `mat_indirect_view` matches Cycles on five fixtures and rejects a seeded double; budgets per profile; models receive no baked indirect light today |
| G1 probe volume, baked producer | done (2026-09-25, native Vulkan; the G1.7 DXVK capture deferred by user direction) | Bake, pack, engine load and fallback, per-pixel `model_pbr` sampling and the CPU ambient cube; all native oracles pass; corpus load and memory recorded: living-room and bedroom are well within budget, staircase2 is over (21 MB, 44.7 ms) and its remedy is per-map spacing; bathroom not built at the user's direction ([G1 corpus](#g1-corpus-recorded-2026-09-25)) |
| G2 light set, separated bake, policy | done (2026-09-24, native Vulkan) | `render.light-set.v1` published each frame, with seeded ID reuse rejected; separated-bake consistency with swapped and doubled layers rejected; `render.indirect-policy.v1` with the double count rejected by the furnace in the CPU model, the GPU and the engine; native WMSH direct light from unbaked lights |
| G3 producer contract and switching | done (2026-09-24, native Vulkan) | Shared suite (Baked, radiosity, fake; seven bad producers and a one-bounce producer rejected); `r_indirect_producer` validation; native switching with no black frame, no early free and device loss mid-fade; backgrounding on the Fold7; `portal-view` through a real portal pair matches Cycles for baked and radiosity |
| G4 precomputed radiosity | done (2026-09-24, native Vulkan; see open notes) | RTRN bake/reader/fuzzing; furnace on the real map (9 of 30 frames, one-bounce rejected); room-states panel/screen toggles converge in 7 frames and match Cycles in game (baked control fails); serial/pooled byte identity under TSan; desktop median 0.78–0.87 ms and Fold7 about 1.2 ms per update (one first update 2.75 ms), 10.4 MB; runs on the Fold7 APK with background/resume |
| G5 GPU compute foundation | done (2026-09-24) | `vulkan_compute` on the native device: features queried and enabled through a `VkPhysicalDeviceFeatures2` chain, compute programs, storage buffers and images dispatched on the graphics queue, retirement by completion serial; `kStorageImages`/`kRayQuery` bits claimed only as enabled (`ValidateDeviceClaims` rejects a bad provider); `render.compute` passes on the Radeon 8060S (validation-clean) and on the Fold7 |
| G6 SDF-traced producer | done (2026-09-25, native Vulkan desktop; Android declared unsupported) | SDFV bake/pack/reader; shared suite plus a traced-field thin-wall oracle with a seeded leak; in game the door closes the far room to Cycles' dark within 64 frames while radiosity and baked fail; the sun move settles toward Cycles; 0.49 ms per update for 400 probes (budget 2.0) |
| G7 ray-query producer | done (2026-09-25, native Vulkan desktop; Android declared unsupported) | Same producer and shader as G6, traced by ray queries against the world's triangles and proxy boxes; `rayquery` offered only with device ray query; shared suite plus thin-wall oracle; door (indirect and shaded) and sun gates pass at SDF's tolerance; 1.0 ms per update for 400 probes on a loaded host (budget 1.5) |
| G8 product defaults and soak | done (2026-09-25; Android soak shortened to 5 min by the user; Apple unverified) | Per-profile defaults chosen from measurements, recorded in the product profiles and generated into the engine (`r_indirect_producer auto`; rollback `baked`); Linux 30-minute soak under validation passes (777 switches, 56 map changes, 0 validation messages, flat memory); Android soak passes; Apple has no R29 runner |
| G9 moved lights (amendment) | active (2026-09-25) | Swinging-bulb `swing` fixture; SDF shadows (decision 4) and inverse-square `light_dynamic`; traced producers take unbaked lights. See [G9](#g9-moved-lights-active) |

## G0: Baseline, fixtures and runner

### G0.2 GPU runner profile (done 2026-09-24)

The conformance runner has a second runner class. A profile declares
`runner` (`headless`, the default, or `gpu`). An unselected
`conformance.py check` still runs only the headless class, so a GPU-less CI
host keeps its gate. `--runner gpu` (or naming a suite, domain, RFC or profile)
selects GPU suites. A class with no suites is a fatal selection error, like
any unmatched selector.

Profiles may now also declare:

- `link_flags`, appended to every suite's link step;
- `requires`, providers applied to every suite in the profile;
- `run_env` and `run_env_unset`, the environment suites and provider probes
  run under.

A new provider kind, `vulkan-device:gpu`, parses `vulkaninfo --summary` and
needs an integrated or discrete GPU; llvmpipe does not satisfy it. The
evidence records every device vulkaninfo reported under the profile's
environment. A negative row may name the defect it must be rejected for with
`expected_divergence`.

[`linux-native-vulkan-gpu`](../quality/profiles/linux-native-vulkan-gpu.json)
compiles the native device core with each suite under C++20 `-Werror`, links
the system SDL3 and Vulkan loader, and renders headless through SDL's
offscreen driver (`VK_EXT_headless_surface`), so no window reaches a desktop.
The existing standalone programs now run as manifest rows with `checks-v1`
records:

| Row | Checks |
| --- | --- |
| `render.world-pbr.native-pixels` | 66 (53 before the indirect-view checks) |
| `render.model-pbr.native-pixels` | 42 (34 before the indirect-view checks) |
| `render.pbr-direct.native-pixels` | 31 (converted to `checks-v1`) |
| `render.world-glass.native-pixels` | 36 |
| `render.gpu-runner.zero-checks` (sensitivity) | renders one frame, reports zero checks and exits 0; must fail with `zero checks executed` |

Evidence: `python3 tools/quality/conformance.py check --runner gpu` passed
5/5 on the Radeon 8060S (RADV, Mesa 26.2.2), evidence
`quality-results/conformance.20260924T214756Z.json`. The runner self-tests
(`python3 -m unittest tools/quality/tests/test_conformance.py`) pass 55 tests,
including new ones for class selection, profile environment, link flags,
providers, vulkaninfo parsing and `expected_divergence`. Two
enum/`sizeof` conditional expressions in `vulkan_device.cpp` were cast so the
core builds under `-Werror`.

Not covered: an Android GPU runner profile (the Fold7 runs through the APK
harnesses) and a hosted CI lane with a GPU.

### G0.1 Fixtures and Cycles references (done 2026-09-24)

[`gi_fixtures.py`](../tools/quality/gi_fixtures.py) generates every fixture
under `quality/fixtures/gi/`; `--check` regenerates into a scratch directory
and fails on any difference. Each fixture has a `gi-fixture/v1`
`fixture.json` (cameras, film 256×192 at a 90° horizontal fov, regions,
states, dynamic models, analytic values) and a `map.json` built by the
`gi-fixture` export profile. That profile uses a 1024² atlas, 2048 samples,
the `gi-reference` light paths (64 diffuse bounces, no clamping) and
separated `direct`/`indirect` layers.

| Fixture | States | Oracle |
| --- | --- | --- |
| `furnace` | default | ρ = 0.6, Le = 0.3: total 0.75, direct 0.30, indirect 0.45 (closed form) |
| `thin-wall` | default | the room behind a 4-unit panel gets no light; the leak bound is provisional at 2% |
| `room-states` | default, panel-off, screen-off, sun-low | `usd-maps/room`, probe sphere under the ceiling panel |
| `door` | closed, open (baked: open) | a room lit only through a doorway |
| `probe-grid` | sky, sun | floor radiance ρ·L_sky and ρ·E·sin(e)/π (closed form) |
| `portal-view` | default | a chamber seen directly and through a portal pair |

[`gi_reference.py render`](../tools/quality/gi_reference.py) renders each
state through
[`gi_reference_blender.py`](../tools/quality/gi_reference_blender.py) into
uncompressed multilayer EXRs and records the reference in `references.json`:

- Blender 5.2.1 and the Cycles device;
- 2048 samples, seed 20260924, no denoising or adaptive sampling;
- the light-path policy and the renderer's hash;
- the composed stage's hash and the hash of every stage layer.

`references.json` also holds the region means of Combined, DiffDir and DiffInd
(E/π, the lightmap's unit). `gi_reference.py check` fails on a changed
renderer, stage layer, fixture field or file.

Two reference policies were decided by experiment:

- **Dynamic models are receivers only.** They are seen by the camera and lit,
  but they neither occlude nor bounce light, which is the light a baked or
  radiosity world gives a dynamic object. With the model occluding, the
  thin-wall lit room read 7.8% below the bake; with this policy it is within
  0.2%.
- **No normal maps.** The lightmap layers and the probe volume hold irradiance
  at the smooth normal, as the bake computes it. With normal maps, the
  room-states world indirect light was 16–48% below the bake.
  - Removing normal maps alone (specular and metals kept) brought every
    region within 5%.
  - Removing normal maps, specular and metals together gave 1%.

  Normal-map detail belongs to the directional lightmap, not to these
  comparisons.

The analytic checks pass: furnace total 0.7501, direct 0.2999, indirect
0.4500. Probe-grid gives sky 0.3999 vs 0.4000 and sun 0.3659 vs 0.3658. A
negative control fails as intended: the furnace rendered with Blender's
default light paths (4 diffuse bounces) is rejected. Reference storage is 6.2 MB.

The map pipeline gained what these scenes needed:

- dynamic models (`sourceEngine:model` Xforms) become retargeted
  `prop_dynamic_override` entities, and they are excluded from the bake and
  the WMSH;
- separated light layers: LMAP v2, a KTX2 array whose layer roles are fixed
  by count, with a built-in validator in `mapcontainer`;
- stage metadata is copied into state override layers;
- an unlit layer passes through the denoiser unfiltered;
- an open-world collision shell gets player headroom;
- meshlet bounding spheres are padded for float32 containment. The validator
  rejected a 2743-unit sphere by one float rounding step.

Reproduce:

```sh
PYTHONPATH=$(python3 -c "import json; print(json.load(open('build/toolchains/pbrt-map-toolchain.json'))['usd_pythonpath'])") \
    /usr/bin/python3.12 tools/quality/gi_fixtures.py --check
python3 tools/quality/gi_reference.py render      # about 16 min for all states on the 8060S (HIP)
python3 tools/quality/gi_reference.py check
python3 tools/quality/pbrt_map_build.py --manifest quality/fixtures/gi/<fixture>/map.json \
    --out quality-results/rfc0011-maps/<fixture> --no-publish
```

### G0.3 Indirect-light debug view and oracle (done 2026-09-24)

`mat_indirect_view` (a cheat ConVar) selects:

- 0: normal shading;
- 1: indirect diffuse light only, without albedo, in E/π;
- 2: that light times the diffuse base.

`mat_indirect_view_scale` sets the exposure. The world shows the LMAP
indirect layer, and `model_pbr` shows its ambient cube. Local lights,
specular, emission and `$envmap` are excluded. The shaders are built as
`-DINDIRECT_VIEW` variants of `world_pbr.frag` and `model_pbr.frag`; the
existing SPIR-V arrays are byte-identical.

The engine uploads each LMAP layer as its own RGBA16F texture through
`WorldMeshUpload004`'s `UploadLightmap` request. Headless coverage:

- `world.lightmap-layers` (35 checks): the case table and fuzzing;
- `world.lightmap-layers.sensitivity` (4): four permissive validators, each
  detected.

GPU coverage: the world and model pixel rows check both views against a CPU
model.

[`gi_runtime.py indirect-view`](../tools/quality/gi_runtime.py) boots the
built map headless on native Vulkan at 1024×768. It runs two captures:

- the view at scale 1, which must pass;
- a seeded defect, scale 2 declared as 1 (indirect light doubled), which must
  fail.

Each capture first takes a shaded proof frame, so `portal_boot`'s
scene-detail gate proves a rendered world. A camera no light reaches (the
unlit thin-wall room) takes its proof frame from a lit camera in the same run.

The comparison works on the reference film: it decodes sRGB, divides by the
scale and averages 4×4 blocks. Each world region's allowance is 10% of its
reference plus 1% of the brightest reference region in that state.

Model regions are measured, not gated, until G1. The oracle refuses a state
with no indirect light, because doubling zero cannot fail. That covers
probe-grid (one floor under the sky), which is judged in G1 by probe
irradiance instead.

| Fixture/camera | Region | Cycles | Engine | Seeded double |
| --- | --- | --- | --- | --- |
| furnace/inside | walls | 0.4501 | 0.4506 | 0.8985, rejected |
| thin-wall/lit | walls | 0.2128 | 0.2125 | 0.4253, rejected |
| thin-wall/dark | walls, panel face | 0.0000 | 0.0003, 0.0000 | — (nothing to double) |
| room-states/model | ceiling, floor, walls | 0.0518, 0.0449, 0.0434 | 0.0507, 0.0432, 0.0425 | 0.1013, 0.0864, 0.0849, rejected |
| door/far (open) | walls | 0.0194 | 0.0194 | 0.0387, rejected |
| portal-view/direct | floor, red wall | 0.2582, 0.2165 | 0.2579, 0.2163 | rejected |
| portal-view/through | floor, red wall | 0.2105, 0.2217 | 0.2104, 0.2216 | rejected |

All five oracles pass. Evidence:
`quality-results/rfc0011-g0-view/<fixture>/{oracle.json,view,seeded-double}`.

```sh
python3 tools/quality/gi_runtime.py indirect-view --fixture <fixture> --state <baked state> \
    --map-build quality-results/rfc0011-maps/<fixture> --out quality-results/rfc0011-g0-view/<fixture>
```

### G0.4 Budgets (done 2026-09-24)

[`quality/budgets/indirect-light-v1.json`](../quality/budgets/indirect-light-v1.json)
records, before any producer exists, per-producer CPU, GPU, memory, load and
switching budgets. Each row has an owner (R71–R77).

| Profile | Frame baseline | Rows |
| --- | --- | --- |
| Linux desktop (Radeon 8060S) | 6.1 ms median warm frame from the frame-pacing scenario | baked, radiosity, SDF, ray query, switching |
| Android (Fold7) | 7.0–7.7 ms GPU | baked, radiosity, switching; SDF and ray query are unsupported until measured |
| macOS, iOS | — | the Android rows, inherited until an Apple runner measures its own (R29) |

[`gi_budgets.py`](../tools/quality/gi_budgets.py) `check` validates the file,
and `report` judges a producer's measurement against its row. A missing
measurement leaves the row unverified. `quality/baseline.json` gained the
checks `gi.references`, `gi.budgets` and `conformance.gpu`.

### G0.5 Dynamic-model "before" measurement (done 2026-09-24)

Every fixture model is drawn by `model_pbr`; the log shows the probe sphere's
material on the model pipeline. In the indirect view, every model reads 0 at
8 bits.

The maps' legacy leaf-ambient lumps (55/56) come from `vrad -bounce 0` on the
collision VMF. That VMF is lit by the pipeline's fallback light, not by the
Cycles bake. The samples' largest component is 0.0078, and none of the Cycles
light reaches a dynamic model today.

| Fixture/camera | Model indirect, Cycles | Engine (leaf ambient) |
| --- | --- | --- |
| room-states/model (under the ceiling panel) | 0.0390 | 0.0000 |
| furnace/inside | 0.4497 | 0.0000 |
| thin-wall/lit | 0.2249 | 0.0000 |
| door/far | 0.0158 | 0.0000 |
| portal-view/direct, through | 0.3352, 0.2977 | 0.0000 |

This is the error G1 must reduce: the model regions become gated when the
probe volume feeds `model_pbr`.

### G0 verification

- Headless conformance passes 123/123 suites in three configurations: g++
  default, g++ release and clang++. Evidence:
  `quality-results/rfc0011-g0-conformance/headless-{default,release,clang}.json`.
- The GPU runner passes 5/5 (g++, g++ release, clang++). Evidence:
  `quality-results/rfc0011-g0-conformance/gpu-{default,release,clang}.json`.
- Tool tests pass: `test_gi_tools.py` and `test_conformance.py` (62 tests),
  the `test_pbrt*` suite (26) and the `test_world*` suite.
- `baseline.py validate` passes.
- Stylelint on every C++ file changed since `cbb5cbc0` is clean (26 files).
- Archlint: the new LMAP bridge and test includes are declared in
  `architecture/modules.json`:
  - `vulkan_world_lightmap` moved into `render.bridge.world-mesh-vulkan`;
  - new module `world.map-container.test-cases`.

  The 64 remaining new findings are outside this work: Portal 2 gameui,
  physics tests and the SDL2 window slice.

Not covered by G0:

- no hosted GPU CI lane;
- no Fold7 or Apple measurement of the new views;
- the leak bound stays provisional until G1 measures a visibility-disabled
  build.

## G1: Probe volume, baked producer and consumer

Scope decision (user, 2026-09-24): "do not care about dxvk... we're focused on
our vulkan native runtime." G1 is verified on native Vulkan only. The DXVK
capture that item 7 asks for is deferred, not attempted. The leaf-ambient lumps
are still derived from the volume (item 7's exporter half), because the native
engine uses them for maps or settings without a volume.

### G1.1 Bake and pack (done 2026-09-24)

[`probe_volume_bake.py`](../tools/quality/probe_volume_bake.py) runs in
Blender on the lightmap bake's normalized stage. It uses the same
smooth-normal material policy and light paths as the bake and the references.

**Method.** This departs from the RFC text's "panoramic Cycles radiance and
depth renders":

- **Irradiance.** Each probe owns one receiver quad per 6×6 tile texel,
  facing that texel's direction. A Cycles DIFFUSE bake without colour gives
  each texel the exact irradiance/π for that normal. The total layer uses
  DIRECT+INDIRECT; the indirect layer uses INDIRECT, the lightmap layers'
  split.
  - The quads are invisible to diffuse, glossy, transmission and shadow rays.
    Cycles skips baking an object with no ray visibility at all, so camera
    visibility, which a bake never traces, keeps them bakeable.
  - This handles every light type with its shadows, where a panorama misses
    lamps and suns.
- **Visibility and placement.** 4096 BVH rays per probe, against the world
  meshes only (dynamic models are left out).
  - Distance moments are cos⁵⁰-weighted per 14×14 visibility texel.
  - A probe that sees more than 25% backfaces moves toward its nearest
    backface; one within 5% of the spacing of a surface is pushed off it.
    Offsets are limited to 0.45 of the spacing per axis. A probe still
    inside after that is inactive.
- **Grid.** The world meshes' bounds inset by 5% of the spacing, or declared
  bounds (`probe_volume.bounds_m`) for an open scene, at `spacing_m` rounded
  to span the bounds.

Three defects were found and fixed by the analytic oracle:

- Cycles' default adaptive sampling stopped texels at a 1% noise estimate;
  the bake now takes every sample from a recorded seed.
- 2 µm quads 5 m from the origin had their face normals tilted by float32
  vertex precision; quads are now 5 mm across.
- A quad covering only part of its texel wasted jittered samples; each quad
  now covers its whole texel.

**Pipeline.** A map export profile or manifest with `probe_volume` adds the
`probe-volume` step (in the `gi-fixture` and `source2` profiles, at 1 m).
Such a map then has:

- no vrad fallback light;
- leaf ambient derived from the volume by
  [`leaf_ambient_from_prbv.py`](../tools/quality/leaf_ambient_from_prbv.py):
  a sample at every active probe in each leaf, the HDR and LDR lumps
  identical, the other lumps left in place;
- the PRBV lump, packed by `bsp2tool pack-world-probed`, which validates it
  first and rejects a malformed volume (exit 2, `PRBV <error>`).

Without the fallback light the engine would have forced `mat_fullbright 1`
("Level unlit", `gl_rmisc.cpp`). A map carrying a probe volume is now
exempt from that rule.

Costs of the fixture volumes at 1 m, 4096 samples:

| Fixture | Probes (active, relocated) | PRBV | Bake |
| --- | --- | --- | --- |
| furnace | 64 (64, 0) | 257 KB | 5.3 s |
| thin-wall | 128 (128, 2) | 530 KB | 7.5 s |
| door | 200 (200, 1) | 842 KB | 12.7 s |
| portal-view | 216 (216, 2) | 902 KB | 17.7 s |
| room-states | 400 (400, 252) | 1605 KB | 21.1 s |

Room-states' relocations come from the clearance rule: the room is full of
furniture and the probes sit near it.

### G1.2 Engine load and fallback (done 2026-09-24)

`CModelLoader::Map_LoadProbeVolume` runs for BSP2 maps. It reads the PRBV
lump, verifies its container hash, and validates it with
`mapcontainer::ValidateProbeVolume`. It then owns the bytes and a
`ProbeVolumeView`, which `worldbrushdata_t::pProbeVolume` borrows until
unload. A rejected lump leaves the map playable. The engine logs:

- `PRBV version, flags or size unsupported`;
- `PRBV read or hash failed`;
- `PRBV rejected (<structured error>)`;

each followed by "models use the leaf ambient". A successful load logs its
cost, e.g. room-states: `PRBV v1, 1 grid, 400 probes (400 active), 320x642
atlas, 1605 KB, 4.07 ms`.

`gi_probes.py malformed` boots room-states with its PRBV lump replaced by
four variants:

- intact (the control, which must load);
- grid count 0 (correctly hashed);
- truncated (correctly hashed);
- one corrupted byte (hash mismatch).

All four boot. The control loads the volume; each malformed variant reports
its diagnostic and loads none. Evidence:
`quality-results/rfc0011-g1/malformed/malformed.json`.

### G1.3 Consumers (done 2026-09-24)

**CPU (every model family).** `lightcache.cpp` evaluates the volume's
ambient cube at a model's lighting origin before the leaf ambient
(`r_radiosity 4`, static and dynamic props), from the total layer with
visibility. The cheat ConVars:

- `r_probevolume`: 0 the leaf ambient, 1 the volume, 2 the volume through
  the ambient cube only;
- `r_probevolume_visibility`: 0 disables the visibility test.

Changing either flushes the light cache. Legacy maps carry no volume and are
unchanged.

**A finding.** Native `PBRMetalRough` never set
`MATERIAL_VAR2_LIGHTING_VERTEX_LIT`. The engine sets up a studio model's
lighting only when one of its materials is vertex-lit, so every PBR model
had a black ambient cube and no local lights. That was the real cause of
G0.5's zeros, not the leaf ambient. The shader now sets the flag; only
studio-model loading reads it.

**GPU (native `model_pbr`).**

- Upload: `WorldMeshUpload005` adds `UploadProbeVolume`. The engine uploads
  the atlas (RGBA16F) and the grid table (`mapcontainer::WriteProbeGridTable`,
  RGBA32F).
- Sampling: `shaders/probe_volume.glsl` is a line-for-line port of the C++
  sampler. The `-DPROBE_VOLUME` variants of `model_pbr.frag` take their
  diffuse light from it per pixel, with the total layer when shaded and the
  indirect layer in `mat_indirect_view 1`. Outside the grid they fall back to
  the ambient cube.
- Device requirement: those variants use a separate 9-set layout, created
  only when `maxBoundDescriptorSets >= 9`. Other devices keep the ambient
  cube, and the log says so. Nothing else changes, and no new device feature
  is needed.

The `render.model-pbr.native-pixels` row adds four probe runs against the C++
sampler, using the new fixture `quality/fixtures/gi/prbv/gpu.prbv`:

- indirect view on the indirect layer;
- the same with a tilted normal;
- shaded on the total layer;
- shaded with visibility off.

Each run judges 260 pixels inside the volume and 100 on the ambient-cube
fallback, with a 3-level bound; every pixel matches. Controls require that
visibility and the other layer change the answer at those pixels. The
captured frame follows the backend's D3D9 clip space (+Y at the top, integer
pixel centres). The headless `world.probe-volume` suite adds the GPU fixture
and grid-table checks.

### G1.4–G1.6 Oracles (done 2026-09-24)

| Oracle | Result | Bound |
| --- | --- | --- |
| probe-grid sky, at probe normals | total 0.58%, indirect 0.68% of floor radiance | 3% |
| probe-grid sun, at probe normals | total 3.4%, indirect 0.62% | 5% |
| thin-wall leak, dark/lit indirect | 0.026% | < 2% |
| thin-wall, visibility disabled | 4.47% (exceeds the bound, as the control must) | > 2% |
| room-states model, per pixel, indirect view vs Cycles DiffInd | 0.0369 vs 0.0390 (5.4%; G0: 0, 100%) | 10% + 1% of level |

The analytic reference is integrated exactly over the finite floor. From
1–2 m up, the sky below the horizon past the 40 m floor carries about 6% of
a horizontal normal's cosine weight.

Between texel normals the 6×6 tile's filtering is measured, not gated:

- up to 6% for the sky;
- up to 18% of peak for the sun. The tile cannot follow a sun's sharp cosine
  edge. That is a policy input for G2: direct light from strong sources
  belongs to the light set, not the total layer.

Every model gated per pixel in the indirect view, with seeded doubles
rejected:

| Fixture/camera | Cycles | Engine |
| --- | --- | --- |
| furnace/inside | 0.4497 | 0.4507 |
| thin-wall/lit, dark | 0.2249, 0 | 0.2227, 0 |
| room-states/model | 0.0390 | 0.0369 |
| door/far | 0.0158 | 0.0160 |
| portal-view/direct, through | 0.3352, 0.2977 | 0.3383, 0.2997 |

Through the ambient cube only (`r_probevolume 2`: the non-PBR families'
path), models against Cycles' DiffDir + DiffInd are measured, not gated:

| Fixture | Error |
| --- | --- |
| furnace | +0.5% |
| thin-wall | +4.4% |
| door | +15% |
| portal-view | −16%, −18% |
| room-states | +41% |

Evaluated at the same point in Python, the six-axis cube and per-pixel
sampling differ by factors of 0.7 to 1.65. That is the representation error
of Source's ambient cube, not a sampling defect.

Reproduce:

```sh
python3 tools/quality/probe_volume.py fixture          # PRBV conformance fixtures
python3 tools/quality/gi_probes.py bake --fixture probe-grid --state sun \
    --out quality-results/rfc0011-probes/probe-grid/sun
python3 tools/quality/gi_probes.py analytic --fixture probe-grid --state sun \
    --prbv quality-results/rfc0011-probes/probe-grid/sun/probes.prbv --out sun.json
python3 tools/quality/gi_probes.py leak --fixture thin-wall \
    --prbv quality-results/rfc0011-maps/thin-wall/lighting/probe_volume.prbv --out leak.json
python3 tools/quality/gi_probes.py malformed --map-build quality-results/rfc0011-maps/room-states \
    --out quality-results/rfc0011-g1/malformed
python3 tools/quality/gi_runtime.py indirect-view --gate-models --fixture room-states \
    --map-build quality-results/rfc0011-maps/room-states --out quality-results/rfc0011-g1/view/room-states
python3 tools/quality/conformance.py check --suite render.model-pbr.native-pixels
```

### G1 corpus (recorded 2026-09-25)

Each built corpus map booted headless on native Vulkan (`portal_boot.py`,
the Radeon 8060S). The engine's own `PRBV v1 … KB, … ms` line gives the load
cost: read, content hash, validation and the host's copy. Memory is the PRBV
payload, held once on the CPU (the ambient cube's view) and once on the GPU
(the atlas); the budget row is quality/budgets/indirect-light-v1.json, baked:
load 20 ms, 16 MB.

| Map | Probes (active) | Atlas | PRBV | Load | CPU + GPU | Budget |
| --- | --- | --- | --- | --- | --- | --- |
| living-room | 168 (166) | 208 × 417 | 677 KB | 1.48 ms | 1.4 MB | within |
| bedroom | 252 (252) | 256 × 513 | 1026 KB | 2.28 ms | 2.0 MB | within |
| staircase2 | 5280 (4628) | 1168 × 2341 | 21361 KB | 44.69 ms | 41.7 MB | **over**: load 2.2×, memory 2.6× |
| bathroom | — | — | — | — | — | not built: its lightmap bake was stopped at the user's request |

staircase2 is over because the pipeline's default 1 m spacing is applied
over its whole large exterior: about 3 KB per probe times 5280 probes. The
remedy is a per-map spacing or bounds (`probe_volume.spacing_m` /
`bounds_m` in its manifest), not a runtime change. The load is dominated by
hashing and validating 21 MB. The other two maps are an order of magnitude
inside the budget.

Evidence: `quality-results/rfc0011-corpus/load/<map>/runtime/engine.log`.

### G1.7 Legacy leaf ambient and DXVK

The exporter half is done: pipeline maps with a volume carry leaf ambient
derived from it, and no fallback light. The DXVK capture is deferred by the
user's native-Vulkan scope decision. An exploratory DXVK boot found two
DXVK-only defects, recorded here and not pursued:

- pipeline maps need the source-matched `sprite_ps20b` artifacts
  (`--shader-artifacts`);
- the leaf-ambient interpolation reads about 58% brighter than the volume at
  the room-states model.

## G2: Light set, separated bake, policy (done 2026-09-24, native Vulkan)

### G2.1 Runtime light set

[`render/light_set.h`](../public/render/light_set.h) (`render.light-set.v1`)
holds the frame's lights as an immutable value snapshot with no native types:

- **IDs.** A world light's ID is fixed by its map index. A dynamic or entity
  light keeps its ID while it stays alive in the same slot with the same key,
  and gets a new one when the slot is reused or it goes dark. No ID is
  reused within a map.
- **Baked state.** Style scalars scale world light colours. A world light
  `matchesBaked` at its baked scalar of 1.
- **Epoch.** Advances by one per built frame.

The publisher, [`engine/light_set_publisher.cpp`](../engine/light_set_publisher.cpp),
builds the snapshot on the main thread in `V_RenderView` before the view
renders:

- from `worldlights`, `LightStyleValue` and the active `cl_dlights` and
  `cl_elights`, with the legacy dlight minimum light;
- publishing to the renderer's `ILightSetConsumer`, which the material
  system returns from `QueryInterface`, alongside the world-mesh uploader.

A dlight created while the view renders is published next frame.
`r_lightset_report 1` prints one snapshot.

Suites (headless): `render.light-set` runs 32 checks over a scripted
sequence:

- slot reuse;
- a light going dark and returning;
- key changes and style changes;
- a map change;
- earlier snapshots keeping their values.

Three sensitivity rows must be rejected, and each is rejected by its own
checks:

- slot-index IDs (identity reuse), 6 rejections;
- ignored style scalars, 2;
- a frozen epoch, 3.

In the engine, a `light_dynamic` spawned in room-states was published as
`1 dynamic ... radius 300`
(`quality-results/rfc0011-g2/dlight-boot`).

### G2.2–G2.3 Separated bake

The bake has written the `direct` and `indirect` layers as independent bakes
since G0, and LMAP v2 carries them; the reader validates them in
`world.lightmap-layers`.
[`lightmap_layers.py check`](../tools/quality/lightmap_layers.py) verifies:

- each layer against its bake receipt's hash;
- the sum per 16×16 block: the mean residual total − (direct + indirect)
  within four standard errors of zero;
- the atlas mean within four standard errors plus 0.1% of the total;
- for the furnace, the analytic layers (direct 0.3, indirect 0.45) within 1%.

All six fixtures pass; the furnace's direct and indirect means are 0.3000
and 0.4499. The seeded controls fail as they must:

- swapped layers fail the furnace's analytic check (0.4499 vs 0.30), while
  the sum still holds;
- doubled indirect fails the sum on every block, for the furnace and for
  room-states.

Unit tests cover the oracle (`test_gi_tools.py`, `LightmapLayersTest`).
Evidence: `quality-results/rfc0011-g2/layers/`.

### G2.4 Indirect policy

[`render/indirect_policy.h`](../public/render/indirect_policy.h)
(`render.indirect-policy.v1`) holds:

- the policy table: `Baked`, `BakedPlusDelta`, `RuntimeIndirect`;
- `Available`: `RuntimeIndirect` needs the direct and indirect layers;
- `WorldDiffuseLight`, the composition every renderer implements.

**Headless.** `render.indirect-policy` (12 checks) covers the table, the
availability rules and the furnace: every policy with the baked producer
gives 0.75, and a doubling producer is detected. The sensitivity row
`render.indirect-policy.double-count`, which reads the total layer under the
`RuntimeIndirect` variant, is rejected.

**Native.** The policy is part of `world_pbr`'s variant key.
`-DRUNTIME_INDIRECT` binds the direct layer as the lightmap and adds the
producer's indirect atlas in set 8 (at G2, the baked producer's LMAP
indirect layer). Controls:

- `r_indirect_policy`: 0, 1 or 2, falling back to Baked without the layers;
- `r_indirect_policy_seed_double`: the sensitivity control.

`BakedPlusDelta` reads the total layer; no producer publishes a delta until
G4.

GPU (`render.world-pbr.native-pixels`), on a furnace-valued LMAP:

- Baked 173, RuntimeIndirect 173;
- seeded double count 213, rejected;
- RuntimeIndirect unavailable on a total-and-indirect LMAP.

Engine, on the furnace map with the tone map pinned:

- Baked mean 223.69, RuntimeIndirect 223.68, at most 2 levels apart (bake
  noise between separately denoised layers);
- seeded double +28 levels.

Evidence: `quality-results/rfc0011-g2/policy-boot`.

### G2.5 Direct light from unbaked lights

The native consumer keeps each frame's unbaked point and spot lights, at
most seven. They travel as one block in the per-frame constants ring, whose
slot is fence-reused like the skin constants.
`world_pbr.frag -DDIRECT_LIGHTS` (set 7) adds them:

- the legacy dlight falloff (`light_set::Falloff`, matching
  `AddSingleDynamicLight`) times the Lambert cosine;
- through the layered BRDF, in `model_pbr`'s units: diffuse albedo times the
  diffuse light, plus π times the incident light through GGX.

The shared GGX term is one function, `SpecularBrdf`. The directional
`lightRadiance` push light stays as the pixel suite's hook; the engine never
set it. The extended variants use nine sets, created only where
`maxBoundDescriptorSets >= 9`; otherwise the world draws the bake alone
under Baked, and the log says so.

GPU checks:

- one point light: 113 against the CPU model's 113.1;
- seeded controls without the cosine and without the falloff each miss by
  more than 6;
- a light beyond its radius and a spot aimed away add nothing;
- two lights add.

Engine: the room-states `light_dynamic` brightened 116k pixels of the native
world.

### Verification

- Headless conformance passes 129/129 (six new rows), and the GPU runner
  5/5. Evidence: `quality-results/rfc0011-g2/{headless,gpu}.json`.
- Style is clean on every changed file.
- Archlint: the new contracts belong to `render.contracts`. The light-set
  key is a struct, because `<tuple>` is not on the standard-header list. No
  finding is in G2 files.

## G3: Producer contract and runtime switching (done 2026-09-24, native Vulkan)

This section was written on 2026-09-24 from another session's evidence and
an independent rerun. It records where each done criterion stands; the work
is still in progress.

| Done criterion | State |
| --- | --- |
| 1. `render.indirect-light.v1` and its shared suite | Passes. Baked, the radiosity producer (`BakedPlusDelta` over the furnace transfer) and a scripted fake pass. Seven deliberately bad producers and a one-bounce radiosity producer are rejected. |
| 2. `r_indirect_producer` saved and validated | Passes in engine (18:19–18:29 boots, below). |
| 3. `render.indirect-switching` on native Vulkan | Passes for baked↔fake, a failed `Begin` and a simulated device loss mid-fade. Android backgrounding passes on the Galaxy Z Fold7 (installed APK, radiosity active): two HOME/resume cycles log `background (radiosity stops scheduling)` and `resume (radiosity)`, and the published volume is unchanged across them (generation 67, mean indirect 0.20244), so no black frame. Backgrounding mid-convergence is covered by the headless suite only. |
| 4. `portal-view` | Passes. The map now carries a linked, activated `prop_portal` pair (fixture `collision.portals`). A camera 5 cm in front of the south portal renders the room through the pair and is judged against the Cycles `through` reference (`reference_cameras`). Baked and radiosity: floor 0.1%, red wall 0.7%, model 4.5%, all within tolerance. Evidence: `quality-results/rfc0011-g3-portal/{baked,radiosity}/gate.json`, `proof.png`. |

**Contract and switcher.**
[`render/indirect_light.h`](../public/render/indirect_light.h) holds the
producer contract. [`render/indirect_switcher.h`](../public/render/indirect_switcher.h)
owns the active and pending producers, the fade, and retirement behind
completion serials. Device loss republishes the baked volume at once, and
backgrounding stops scheduling. The engine host is
[`engine/indirect_light_host.cpp`](../engine/indirect_light_host.cpp).

**Headless.** `render.indirect-light` also runs the switching scenarios
against a fake GPU timeline: baked↔fake, a failed `Begin`, device loss
mid-fade, backgrounding and map change.

**Native.** `render.indirect-switching.native-pixels` (RADV 8060S, real frames
and completion serials): baked mean 164.2, fake 197.0. The darkest frame
during a switch is 1.000 of the seed's mean, so there is no black frame, and
fade residency is within budget.
- The 18:22 run had the validation layer and passed 83 checks, with 0
  messages.
- The rerun had no layer and passed 82; the validation check needs the layer.

**In engine** (`gi_room_states`, native Vulkan, headless):
- `saved-value-boot`: a saved `rayquery` logs "is not offered here
  (unavailable); using baked (offered: baked)".
- `ui-boot`: Advanced Video lists only the offered producers
  ("indirect lighting options: Baked").
- `engine-switch`: baked → fake → an unavailable `rayquery` ("keeping
  fake") → baked.

These boots predate the radiosity producer being offered on a map.

## G4: Precomputed radiosity (done 2026-09-24, native Vulkan)

| Done criterion | State |
| --- | --- |
| 1. RTRN bake, reader, fuzzing | Passes. [`radiosity_transfer_bake.py`](../tools/quality/radiosity_transfer_bake.py) runs as the `radiosity` step of `pbrt_map_build.py` after the probe volume. `world.radiosity-transfer` (C++) and `test_gi_tools.py` (Python) validate the contract transfer, reject all 27 malformations with the same codes, reject a transfer against the wrong volume, and fuzz without a crash. |
| 2. `furnace` | Passes on the real furnace map (`gi_radiosity.py furnace`): from a cold start the probes reach 0.4498 (analytic 0.45) within 2% after 9 frames, of 30 declared. The one-bounce defect settles at 0.288 and never reaches it. The headless contract furnace passes too (`render.indirect-radiosity`). |
| 3. `room-states` | Passes. Panel-off and screen-off converge within 2% in 7 frames (`gi_radiosity.py convergence`), 15 frames on the Fold7. In game (`gi_radiosity.py states`), default, panel-off and screen-off match the Cycles indirect-only references in world and model regions within 10%. The baked producer after the panel toggle fails, as it must. |
| 4. `jobsystem.radiosity` | Passes: serial bytes reproduced with 1, 2 and 32 workers (up to 33 threads per batch) on a 40000-patch transfer; TSan (clang++) clean; the seeded race is caught. |
| 5. CPU and memory budgets | Passes. Desktop, one thread, while converging: median 0.78–0.87 ms per update (budget 1.0), measured on a loaded host. Fold7 in game: 0.87–1.9 ms per update, median about 1.2 ms (budget 2.0); the first update after a toggle took 2.75 ms once. Memory 10.4 MB (64 desktop, 24 Fold7). No separate low-core run: the budget is single-threaded. |
| 6. Android installed APK | Passes on the Galaxy Z Fold7 (`build-android-apk.sh --install`): RTRN loads, radiosity runs, the panel toggle converges, and two background/resume cycles keep the published volume (generation 67, mean 0.20244 before and after). Not covered on the device: backgrounding mid-convergence (headless suite only). |

**Design decisions.**
- Policy: `BakedPlusDelta`. The producer publishes the bake plus the change from switched sources, so an unchanged scene is the bake byte for byte.
- World: the change is sampled per pixel from a change volume (a PRBV-layout atlas, world_pbr `-DDELTA_VOLUME`, ten descriptor sets). This resolves open decision 3: probes only, no surface atlas; room-states quality passes with it.
- Patches of 0.5 m (the 0.25 m transfer had the same 6.5% model error at four times the cost). One Jacobi iteration per update; half the probes gathered per update, direct light every update.
- Albedo is measured in Cycles per material (plane under a white sky, base 0 and 1). Pure diffuse albedo was 24% dark against Cycles, analytic Fresnel 19% bright; the calibration leaves 6%. Lambertian fixtures (door) match within 0.8% without it.
- Switchable sources compile as zero-brightness named `light` entities (styles 32+). `ent_fire <name> TurnOff/TurnOn` switches them; the pack step strips their vrad world lights.

**Found on the way.** The compile tools write 100-byte world-light records under lump version 0, which the engine reads as 88-byte records: every record after the first is garbage (a SIGSEGV in `AddWorldLightToLightingState`). Pipeline maps now carry no world lights; the tool bug itself is open.

**Open (not G4 gates).** On Adreno, per-pixel model sampling and the world change volume are unavailable (descriptor-set limit), so the Fold7 uses the ambient cube and the world keeps the bake. World direct light of a switched baked light stays in the lightmap (needs per-style layers, RFC 0008 LSTY).

Evidence: `quality-results/rfc0011-g4/{furnace,convergence,budget}.json`, `quality-results/rfc0011-g4/states/states.json`.

### Verification (independent rerun, 2026-09-24 19:51–19:53)

At `502bb424` plus the dirty tree (another session was editing these files),
suites built in a private directory:

- Headless: 6/6 suites, 125 checks.
  - `render.indirect-light` 37;
  - `render.indirect-radiosity` 23;
  - `world.radiosity-transfer` 42;
  - `jobsystem.radiosity` 10;
  - `render.indirect-policy` 12, and its double-count row 1.
- TSan (clang++): `jobsystem.radiosity.tsan` passes (10 checks). The
  `.sensitivity` row fails as expected.
  - g++ has no TSan runtime on this host (`libtsan.so.2.0.0` missing), as the
    R01 baseline records. That evidence file shows the link failure only.
- GPU: `render.indirect-switching.native-pixels` passes (82 checks).

Evidence: `quality-results/rfc0011-g34-verify/{headless,tsan-clang++,gpu}.json`.
The room-state captures were not rerun.

```sh
python3 tools/quality/conformance.py check --runner headless \
    --suite render.indirect-light --suite render.indirect-radiosity \
    --suite world.radiosity-transfer --suite jobsystem.radiosity \
    --suite render.indirect-policy --suite render.indirect-policy.double-count
CONFORMANCE_TSAN=1 python3 tools/quality/conformance.py check --cxx clang++ \
    --suite jobsystem.radiosity.tsan --suite jobsystem.radiosity.tsan.sensitivity
python3 tools/quality/conformance.py check --runner gpu \
    --suite render.indirect-switching.native-pixels
python3 tools/quality/gi_runtime.py compare --fixture room-states --state panel-off \
    --capture quality-results/rfc0011-g4-room/radiosity-panel-off \
    --out quality-results/rfc0011-g4-room/radiosity-panel-off/gate.json
```

## G5: GPU compute foundation (done 2026-09-24)

| Done criterion | State |
| --- | --- |
| 1. Compute pipelines, storage images and buffers, dispatch on the graphics queue, features through a `VkPhysicalDeviceFeatures2` chain | Passes. [`vulkan_compute.{h,cpp}`](../materialsystem/shaderapivulkan/vulkan_compute.h): `QueryComputeCaps` (queue flags, RGBA16F/R32F storage formats, Vulkan 1.2 features, ray query and acceleration-structure extensions and features, timeline semaphores), `DeviceFeatureChain` (enables exactly what the queries found), `ComputeResources` (buffers, storage images, programs, `RecordDispatch` with its barrier). `CVulkanContext` now creates its device through the chain (instance API 1.2 where the loader has it) and records queued compute work at the start of each frame, before any render pass. The game boots: `compute on, storage images on, ray query on (device API 1.4)`. |
| 2. Truthful feature bits | Passes. `RenderFeature` gains `kStorageImages` and `kRayQuery`. The engine's native adapter report takes them from the created device's enabled caps (`ComputeFeatureBits`); the render-backend provider claims compute and storage images from queries and never claims ray query, which it does not enable. `render_profile.h` `ValidateDeviceClaims` fails a claim beyond the device's enables (`kClaimNotEnabled`); `render.profile` (138 checks) and `render.compute` reject a provider that claims ray query without enabling it. |
| 3. Retirement by completion serial | Passes. Buffers, images, programs and each dispatch's descriptor set retire behind their submission's serial, collected with the managed textures. `render.compute` shows a retired buffer alive while its submission is pending and freed after; the seeded early-free defect fails that check. |
| 4. Compute conformance on Linux and the Fold7 | Passes. `render.compute` (GPU runner, RADV 8060S): 14 checks, 0 validation messages with the layer. The same source cross-compiled with NDK r30 runs from `/data/local/tmp` on the Galaxy Z Fold7 (Adreno 840, Vulkan 1.4): 13 checks (no layer on the device). Both devices report ray query. All seven GPU rows pass with the new device creation. |

## G6: SDF-traced producer (done 2026-09-25, native Vulkan desktop; Android declared unsupported)

| Done criterion | State |
| --- | --- |
| 1. Shared producer suite, including `thin-wall` | Passes. `render.indirect-light.sdf` (GPU runner, RADV 8060S) runs the shared suite ([`indirect_contract.h`](../unittests/rendertest/indirect_contract.h)) against [`SdfTracedProducer`](../public/render/indirect_sdf.h) through the renderer's compute service on the contract SDFV: seed, light response, a proxy box that darkens the probe it encloses, no GPU wait in `Schedule`, and the suite's per-publication leak check behind the thin wall. Because the composition could hide a leak in the traced field (a dark bake times any ratio stays dark), the field's own converged reference is also judged: lit side 0.740 (analytic 0.75), dark side 0.0000. The seeded leak defect (the wall's voxels erased) reads 0.042 behind the wall and is caught. 9 checks, validation-clean with the layer. The same suite ran on the Fold7 (Adreno 840) before the memory change below. |
| 2. `door` | Passes in game (`gi_sdf.py door`, [`gate.json`](../quality-results/rfc0011-g6/door/gate.json)). The door is a moving `func_brush` (`Door`, open at spawn), so the bakes and the SDFV never contain it, and the engine passes drawn brush entities to the producer as proxy boxes. SDF open matches Cycles `open` (walls 0.0197 against 0.0194, model 0.0161 against 0.0158). SDF closed, captured 64 frames after `ent_fire Door Enable`, matches `closed` (walls 0.0010 and model 0.0000 against 0; the allowance is 0.0019). Radiosity and baked closed still read the open room's 0.019 and fail. The log shows the proxy arriving in all three closed captures. |
| 3. `room-states`: sun angle changes converge | Passes (`gi_sdf.py sun`). `r_indirect_light_direction 33 <dir>` moves the sun (light style 33) to the fixture's recorded sun-low direction. The capture 64 frames after the move agrees with one 240 frames later (settled), and every region moves toward the Cycles sun-low reference; radiosity does not. The absolute accuracy is recorded, not gated: floor 0.063 and model 0.062 against Cycles 0.094 and 0.088 (walls 0.048/0.052, ceiling 0.052/0.061). At the probes the field is closer: mean error 0.009 against the probes of a Cycles sun-low bake, against 0.047 for the unchanged bake. The world and model residual sits next to the brushed-metal sphere, which the SDF treats as diffuse (a hypothesis, not verified). |
| 4. GPU cost | Desktop passes: 0.489 ms median per update (p95 0.559) with 400 probes on room-states, and 0.422 ms (p95 0.455) with 200 on the door (budget 2.0 ms, median over warm updates; `indirect_sdf_native_conformance --bench`, [`cost/`](../quality-results/rfc0011-g6/cost)). The trace is per probe, so the cost does not depend on resolution. Android is declared unsupported, as the budget file already stated: the engine does not offer `sdf` on Android builds (`kSdfProfileSupported`). |

**Pipeline.** [`sdf_volume_bake.py`](../tools/quality/sdf_volume_bake.py) runs as
the `sdf` step of `pbrt_map_build.py` when a profile or manifest sets
`sdf_volume` (the gi-fixture profile: 0.1 m voxels). It reuses the radiosity
bake's Cycles-calibrated reflectance and source styles from its receipt, so
both producers agree on the scene. Per voxel it records the nearest static
surface's distance (Blender BVH; negative inside closed meshes), reflectance
and emission, including textured emission (the room's screen). The scene's
lights become analytic records: rectangles, distant lights and a mean-radiance
dome. `bsp2tool pack-world-sdf` packs the SDFV lump beside RTRN. The engine
validates it on load (`world.sdf-volume`: 23 checks, 13 malformations with the
Python reader's codes, fuzzing), and offers `sdf` only with a valid field and
a device that runs compute.

**Engine and renderer.**
- The renderer exposes `render/gpu_compute.h` through the material system's
  `QueryInterface` (`RenderGpuCompute001`). Dispatches are recorded at the
  end of the frame's command buffer and stamped with its serial.
- Buffers take a use: `Readback` is host-cached. Before, the field lived in
  memory that was slow for the GPU as well as the CPU. The change cut the
  update from 1.87 to 0.49 ms and removed a flaky 5 ms `Schedule` (the
  CPU reading the field back).
- Composition: light removed is relative (the bake times the field's ratio
  to its own reference), light added is absolute. Without that, the door's
  far room kept 5–25% of its light; with it the probes read 0.
- `warmupFrames` was understated: two dispatches in flight give 1.5 frames
  per update. It is now 104, and the thin-wall check asserts the reference
  phase ends within it.
- `vbsp2`'s World Stage emitter now writes WorldSpawn only and reports the
  brush entity models it leaves out, instead of failing any map with one.

**Open (not G6 gates).** Thin walls and grid-aligned planes: a plane between
voxel centres can be stepped over, and the fixtures align walls to the grid.
Proxies are brush-entity boxes only (models are not proxies, because the
references render them receiver-only). Specular surfaces are traced as diffuse.

```sh
python3 tools/quality/pbrt_map_build.py --manifest quality/fixtures/gi/door/map.json \
    --out quality-results/rfc0011-maps/door
python3 tools/quality/conformance.py check --runner gpu --suite render.indirect-light.sdf
python3 tools/quality/conformance.py check --suite world.sdf-volume
python3 tools/quality/gi_sdf.py door --out quality-results/rfc0011-g6/door
python3 tools/quality/gi_sdf.py sun --out quality-results/rfc0011-g6/sun
build/unittests/shaderapivulkantest/indirect_sdf_native_conformance --bench \
    quality-results/rfc0011-maps/room-states/lighting/probe_volume.prbv \
    quality-results/rfc0011-maps/room-states/lighting/field.sdfv
```

### G6 follow-ups (2026-09-25, from play-testing)

- **Flicker in a still scene.** The traced producers hashed all 64
  light-style scalars into their configuration. The stock styles 1–11 are
  animated flicker patterns, so the "configuration" changed several times a
  second, and each change restarted the update at alpha 0.5 with fresh noisy
  traces. The fix: only the styles the SDFV uses (its lights' and emissive
  voxels' styles) count.
  [`gi_temporal.py`](../tools/quality/gi_temporal.py) now scores it. It takes
  a burst of screenshots of a still scene per producer and measures per-pixel
  temporal variation in linear light (limits 0.2% RMS and 0.2% per step).
  Baked is the control and must score exactly 0; synthetic flicker and a
  one-frame pop fail (`test_gi_tools.py`). On `door` before the fix SDF
  scored 2.6% RMS with a 29% per-pixel range; after it, baked, radiosity, SDF
  and ray query all score 0.
- **A closed door left the far room lit.** The producers change indirect
  light only. The bright floor patch was the panel's direct light through
  the doorway, baked into the lightmap with the door open, and the gates
  above looked only at the indirect view. Two fixes, independent of the
  producer:
  - [`direct_occlusion.h`](../public/render/direct_occlusion.h), baked
    direct light: each lightmap texel's position and normal come from the
    WMSH triangles rasterized in lightmap space, grown two texels past
    coverage and oriented per texel to the side its lights reach (the door
    floor's fan triangles are wound both ways). When the proxies change, each
    texel loses the share of its direct layer that the proxies block. The
    share is taken of the light it actually saw (its baked direct light over
    the lights' unoccluded estimate), since static occlusion is in the bake.
    The host re-uploads the lightmap total: 12–35 ms on the pool per door
    change for 88k texels on this loaded host.
    `render.direct-occlusion` has 17 headless checks: an exact footprint under
    a sun, a rectangle-light shadow, untouched texels beside and behind, and
    a downward-wound floor.
  - `OccludeProbeVisibility`: probes near a proxy get their visibility
    distance moments cut at the proxy, so probes behind a closed door stop
    lighting through it. The doorframe glow is gone.
  - `r_indirect_occlusion 0` (cheat) turns both off; the door gate uses it as
    its negative control.
- **The door drew red:** the dev texture on the legacy brush path. Doors now
  name their material; the door fixture uses the map's wall material.
- **SDFV v2 (another session's format change)** made the engine reject every
  v1 lump, silently dropping `sdf`/`rayquery`. The loader now accepts
  versions 1 through the current one, and `bsp2tool` stamps the payload's
  own version.
- The per-frame `PRBV ready` log now prints once per map.

## G7: Ray-query producer (done 2026-09-25, native Vulkan desktop)

| Done criterion | State |
| --- | --- |
| 1. The device queries and enables acceleration-structure and ray-query features; devices without them do not offer the option | Passes. G5's `DeviceFeatureChain` enables them when queried. `vulkan_compute` now builds acceleration structures: `CreateGeometry` makes a triangle BLAS, `CreateScene` a TLAS of instances, and both are built in the compute service's queue ahead of the dispatches that use them and retired by completion serial. The program binding is `ComputeBinding::AccelerationStructure`. The engine offers `rayquery` only when the device reports ray query and the map has an SDFV and a WMSH (`offered: baked radiosity sdf rayquery` on the Radeon 8060S). Without ray query, `RayQueryProducer::Begin` fails with missing-feature and creates nothing (checked in `render.indirect-light.sdf`). Android is declared unsupported until measured, as the budget file states. |
| 2. Same producer suite and scenarios as G6, tolerances at least as tight | Passes. `RayQueryProducer` is the G6 producer with its tracer swapped: `sdf_probe_trace.comp -DRAY_QUERY` traces rays and visibility against a scene of the world's triangles (instance 0) and a unit cube per proxy (instances 1..). The SDFV still supplies surface attributes and lights. It declares SDF's response tolerance (0.1). In `render.indirect-light.sdf` it passes the shared suite; its field's thin-wall oracle reads lit 0.716 and dark 0.0000, and removing the wall's triangles leaks 0.072 and is caught. It is validation-clean. `gi_sdf.py door --producer rayquery`: open matches, closed matches, and radiosity and baked closed fail. `gi_sdf.py sun --producer rayquery` settles, moves every region toward sun-low, and radiosity does not (floor 0.061 and model 0.075 against Cycles 0.094 and 0.088). |
| 3. GPU cost (budget 1.5 ms at 1920×1080) | Desktop: 1.005 ms median (p95 1.125) with 400 probes on room-states, and 0.265 ms with 200 on the door (`indirect_sdf_native_conformance --bench <prbv> <sdfv> <wmsh>`). These are per-probe costs, independent of resolution, and were measured at host load 66 (three CPU bakes). An earlier contended run read 0.378 ms. Fold7: declared unsupported. |

**Door, shaded view (both producers, rebuilt door map, 2026-09-25).**
`gi_sdf.py door` passes for `sdf` (G6.2) and `rayquery` (G7.2). The
indirect-view checks pass as before; the new shaded checks give:

| Capture | Walls | Model |
| --- | --- | --- |
| open | 0.0172 | 0.0100 |
| closed | 0.0005 | 0.0000 |
| closed, `r_indirect_occlusion 0` (the control) | 0.0063 | — |

The control stays lit and fails, as it must. `gi_temporal.py` on `door`
scores 0 for baked, radiosity, sdf and rayquery.

**Door face lit from the probe volume (2026-09-25).** The door brush drew
flat and unlit-bright: vbsp could not resolve its material, so its faces
were `SURF_NOLIGHT` on the white page. Three fixes:

- `pbrt_collision_vmf.py` writes a stub VMT for a door's named material, so
  the faces get lightmaps.
- It gives each side Hammer's world-aligned axes for its dominant axis. Every
  side had used `[1 0 0]`/`[0 -1 0]`, which leave vbsp's lightmap vectors
  degenerate on x- and y-facing sides.
- `R_RelightBrushEntitiesFromProbes` (engine/gl_lightmap.cpp) relights every
  brush-entity surface from each consumed volume:
  - Each luxel samples half a probe spacing off its face, along the face's
    own plane normal. `SURFDRAW_PLANEBACK` relates a face to its node, not
    to its plane.
  - A luxel outside every grid takes the nearest point inside one (lightmaps
    pad past their face).
  - A luxel with degenerate lightmap vectors takes the face centroid.
  - `R_BuildLightMapGuts` uses these samples in place of the lightstyle bake.

`r_indirect_report 1` prints the counts, the mean and the time per relight.

With the door closed, from room A the face reads 145–187 in 8-bit sRGB
against 138–156 on the wall beside it; the probes are brighter near the
light. From room B it reads 0.0 against 0.22 on B's back wall. Before the
half-spacing offset it read 14.9 from room B, with light leaking from A's
probes.

The door gates still pass after this change:
- `gi_sdf.py door` passes G6.2 for sdf and G7.2 for rayquery
  (`quality-results/rfc0011-g8/door-recheck2`, `door-rq2`).
- `gi_temporal.py` scores 0 flicker for all four producers
  (`quality-results/rfc0011-temporal/door2`).

A relight takes 0.14–0.37 ms for the door's 6 surfaces (104 luxels). It
runs once per published volume, about 100 times over the 2.5 s the producer
takes to converge after a change. Maps with many brush entities should
measure it.

**Open:**
- The occlusion recompose takes 12–35 ms in one frame per door change.
- The G7 cost should be re-measured on an idle host.

Evidence: `quality-results/rfc0011-g{6,7}/{door,sun}/gate.json`,
`quality-results/rfc0011-g7/cost/`, `quality-results/rfc0011-temporal/door/temporal.json`.

## G8: Product (done 2026-09-25)

| Done criterion | State |
| --- | --- |
| 1. Per-profile default producer chosen from measurements, recorded in the product profiles; rollback is a setting | Passes. The measurements are below. The product profiles now carry `indirect_light.default_producers`: Linux desktop `sdf, radiosity, baked`, Android `radiosity, baked`. [`indirect_defaults.py`](../tools/quality/indirect_defaults.py) generates [`engine/indirect_light_defaults.h`](../engine/indirect_light_defaults.h) from them; `--check` and a `test_gi_tools` test fail if the header drifts from the profiles. `r_indirect_producer` now defaults to `auto`, the first producer in the profile's list that the map and device offer; `baked` closes every list, so a map without the others' data still lights. Rollback: `r_indirect_producer baked`, no rebuild. |
| 2. Installed Linux and Android products pass a switching soak | See below. [`gi_soak.py`](../tools/quality/gi_soak.py) boots the installed product (Linux: portal_boot's private runtime with the build's binaries; Android: the installed APK, maps pushed as a custom folder, the script as autoexec). It plays a looping script: on each GI map every ordered producer pair with the door toggled between switches, then a map change, with the stock chambers between (no switcher there). It stops on the wall clock through a stop cfg. Gate: no crash; every offered producer taken and no unoffered one; the per-loop memory floor grows by at most 64 MB after the first loop; no Vulkan validation message (desktop; no layer ships in the APK). |
| 3. Apple profiles | Unverified: R29 has no Apple runner, so baked and radiosity have not run on macOS or iOS. |

**Measurements that chose the defaults** (desktop, Radeon 8060S; Android,
Galaxy Z Fold7):

| Producer | Light toggles (room-states, `gi_radiosity.py states --producer`) | Door | Sun | Flicker (`gi_temporal`) | Cost per update (budget) |
| --- | --- | --- | --- | --- | --- |
| baked | control: fails, as it must | fails | fails | 0 | — |
| radiosity | passes | fails | fails | 0 | 0.8 ms CPU desktop (1.0); about 1.2 ms Fold7 (2.0) |
| sdf | passes | passes | passes | 0 | 0.49 ms GPU desktop (2.0); Android unsupported |
| rayquery | passes | passes | passes | 0 | 1.0 ms GPU desktop (1.5); Android unsupported |

sdf is the cheapest producer that passes every scenario within the desktop
budget. rayquery passes the same scenarios at twice the cost, and needs the
same map data plus ray-query hardware, so it stays opt-in. On Android only
radiosity has a measured budget. The baked control in `states` now selects
`baked` explicitly: before, it silently took a runtime's saved producer and
could pass.

**Linux soak** (30 minutes, installed product, Khronos validation layer on):
passes (`quality-results/rfc0011-g8/soak-linux/soak.json`).
- 11 loops over gi_door, gi_room_states, gi_portal_view and the stock
  chambers testchmb_a_00 and testchmb_a_01: 56 map changes and 777 switches
  (baked, radiosity and sdf 203 each; rayquery 168).
- 0 validation messages.
- The per-loop memory floor holds at 430–452 MB, +0.06 MB from the second
  loop to the last.
- It quit cleanly on schedule through the stop cfg. The boot wrapper's own
  final screenshot is skipped, so its "no screenshot" note is expected and
  not part of the gate.

**Android soak** (5 minutes rather than 30, at the user's direction): passes.
- 8 loops over gi_door, gi_room_states and testchmb_a_00: 25 map changes and
  80 switches taken (baked 40, radiosity 40). sdf and rayquery were refused
  96 times as unavailable, as the Android profile declares.
- The memory floor (dumpsys total PSS) holds at 858–863 MB, +4.9 MB over the
  run. Map-load transients reach 1.6 GB and return. No crash.
- The run completed its 5 minutes, but the app was then ended by a task
  removal (the phone was picked up), not by the scripted quit. The relaunch
  replaced the on-device engine.log, so the soak's engine output was
  recovered from logcat. The phone was restored afterwards (cfgs and pushed
  maps removed, the displaced custom folder back), then updated to the
  current APK with `r_indirect_producer auto`.

**Found on the way.**
- Every headless native-Vulkan boot ended with a validation error
  (VUID-vkDestroyInstance-instance-00629: a surface alive at instance
  destruction). SDL_Vulkan_DestroySurface does nothing under SDL's
  offscreen driver; the surface host now destroys its VkSurfaceKHR with
  vkDestroySurfaceKHR, and a plain boot is validation-clean.
- `SDL3 relative mouse mode failed` was logged every frame under a driver
  without relative mode (4 MB of log in the soak). It now warns once, until
  the call next succeeds.
- The engine's `echo` puts a timestamp between words. Screenshots requested
  while one is being written are dropped. A console line is cut at about 512
  characters. The soak and flicker harnesses work within all three.

## Traced producers at map scale: SDFV v2, light cells, probe focus (2026-09-25)

A user-directed follow-up to G6–G7, prompted by relighting shipped maps
(`tools/quality/legacy_bsp_relight.py`). The SDF producer refused a relit
`testchmb_a_00`: its SDFV held 64 light records, and the producer's limit was
16. Each point light cost six rectangle records. The bigger cost was
elsewhere. Every update traced every probe against every light, and a whole
Portal map has 30× the fixtures' probes.

| Part | Change | Evidence |
| --- | --- | --- |
| Native lights | SDFV v2 (`tools/quality/sdf_volume.py` owns the format) adds `sphere` and `spot`. A spot is a one-sided disk with vrad's cone: full inside the inner cone, `((cos − outer)/(inner − outer))^exponent` between, nothing outside. The C++ and Python readers still accept v1. The scene model carries each emitter's analytic `shape` and a DiskLight's Source cone (`sourceEngine:cone*`). The Cycles bakes apply the same cone as an emission mask (`pbrt_blender.emitter_material`). `DirectOcclusion` samples both kinds. | `world.sdf-volume` 40 checks (20 malformations, the every-kind fixture, v1 compatibility). `render.direct-occlusion` 23 checks: a box on a corner's path darkens it under a sphere, but not under a spot whose cone misses the corner. GPU: probe analytic light against L r²/d² cos, sphere 0.1% and spot 0.5% worst texel error; the spot's cone leaves a probe 45° off axis at 0, where the sphere control gives 0.74. The relit map drops from 64 records to 14. A scratch Cycles bake of the cone matched analytic within 0.3% on axis. |
| Light cells | The bake lists, per 2 m cell, the lights that can reach it (`tools/quality/sdf_light_cells.py`). A light is culled only where its brightest light, at 2.1× for styled lights, is below 0.001, where the cell is behind a one-sided light, or, for a compiled map, by the BSP's PVS between their open leaves. The trace program lights probes and ray hits from their cell's lists, which live in storage buffers, so there is no light limit. | Python: a conservativeness property over random lights and points, a seeded wrong variant (distance to the cell centre) detected, visibility, side and buried cases. GPU: a cell that omits the light leaves its probe unlit. Relit `testchmb_a_00`: 37k cells, 0.51 lights per cell on average, PVS culled 182k pairs. |
| Probe focus | Each probe has its own reference and update counts. The host (`engine/indirect_light_host.cpp` ProbeFocus) passes `FrameWork::focusProbes`: probes with an open-leaf cluster in the camera's PVS or a touching cluster. It falls back to camera distance (`r_indirect_focus_radius`) when visibility does not narrow the set. Other probes take turns, `r_indirect_probe_budget` (128) per update. `r_indirect_focus 0` restores every probe every update. An update's live dispatch lists only its traced probes and those its ring slot is stale for. | GPU: a focus of probe 0 with no budget references probe 0 only; a focus of every probe is the no-focus schedule byte for byte; after a change a focused probe updates while an unfocused one waits; a budget brings every probe to rest. In game the host's focus equals the independent `tools/quality/gi_focus.py` (94 identical probes at the relit spawn). |

GPU cost, Radeon 8060S, `indirect_sdf_native_conformance --bench` (median /
p95 per update; live = a door-sized proxy appearing in view). Evidence:
`quality-results/relight/sdf-gates/bench/bench-final.txt`.

| Map | Probes | Focus | Reference | Live |
| --- | --- | --- | --- | --- |
| relit `testchmb_a_00`, 4 m | 4862 | none | 3.48 / 3.79 ms | 4.11 / 4.59 ms |
| relit `testchmb_a_00`, 4 m | 4862 | 94 | 0.61 / 0.72 ms | 0.48 / 1.07 ms |
| 2 m grid, open-leaf probes only | 647 active | 301 | 0.85 / 0.94 ms | 1.58 / 2.61 ms |
| 2 m grid, every probe active | 34272 | 630 | 1.57 / 1.67 ms | 0.50 / 3.32 ms |

The same 2 m grid without a focus costs 21.4 ms per reference update.

Kept green on the final build (`quality-results/relight/sdf-gates/final2`):
- `render.indirect-light.sdf`: 28 checks.
- `gi_sdf.py door`: G6.2 and G7.2 pass. `gi_sdf.py sun`: G6.3 and G7.2 pass.
- `gi_temporal.py door`: 0 flicker for every producer.
- `render.direct-occlusion`: 23 checks. `world.sdf-volume`: 40 checks.

A first rerun failed G6.3. Room-states probes inside brushes (cluster −1)
had fallen out of the focus. A probe's clusters now include its six
half-spacing neighbours, and the gate passes.

Open:
- The live p95 exceeds 2 ms at 2 m spacing; the medians fit.
- A relit map's probe bake keeps probes in the void outside the map active:
  4859 active at 4 m, but 116 in open leaves. The budget spends turns on them.
- The validation layer is absent on this host, so these runs are not
  validation-checked.
- Android was not measured.

## SDF GI performance fixtures (2026-09-25, user request)

Scope: microbenchmarks of the SDF-traced path and versioned performance
fixtures to burn its performance debt down. Row R76 (SDF producer), not yet
ranked. Before this, only the trace dispatch's GPU time was measured
(`--bench`, G6.4). The CPU work on the frame thread had no measurement.

Installed:

- [`indirect_sdf_bench.cpp`](../unittests/rendertest/indirect_sdf_bench.cpp)
  (manifest `render.indirect-sdf.bench-smoke`, 14 checks) times the
  producer's `Schedule` against `HostCompute`, a host-memory
  `IGpuCompute` whose dispatches write a deterministic field outside the
  timed region. It also times the host's work per published volume and
  `DirectOcclusion` build and compose. Oracles on every sample:
  - the unchanged scene publishes the bake byte for byte;
  - every converged active probe publishes the bake times the field's ratio;
  - inactive probes keep the bake;
  - the change atlas is published minus baked;
  - occlusion without proxies is the bake;
  - pooled compose equals serial compose byte for byte.
- `ChangeAtlas` moved from `engine/indirect_light_host.cpp` into
  `render/indirect_light.h`, so the bench times the engine's code.
- The GPU `--bench` also reports `Schedule`'s CPU time on the device.
- [`gi_sdf_bench.py`](../tools/quality/gi_sdf_bench.py) runs the workloads of
  [`indirect-sdf-perf-v1.json`](../quality/budgets/indirect-sdf-perf-v1.json)
  in interleaved rounds and judges 75 rows, each against a budget and a
  reviewed baseline:
  - `regressed` fails: over max(budget, baseline) x 1.5 + 0.05 ms.
  - `debt`: the baseline is over budget. These rows form the burn-down list,
    ranked, with owners.
  - `improved`: under 0.67 x baseline. `--update-baselines` then lowers the
    baseline; nothing raises a baseline without review.
  - A crash, a failed oracle, an unmeasured stage or a result that does not
    repeat across rounds fails. A missing map scene is `unverified`.

  GPU budgets come from `indirect-light-v1.json` (`budget_ref`). The CPU
  budgets were set with these fixtures, before any optimization:
  - one publishing frame: 1.0 ms (the radiosity row);
  - a still scene: 0.01 ms;
  - the occlusion recompose: 4 ms pooled, 10 ms serial;
  - load stages: 20 ms;
  - the occlusion build: 100 ms.
- Scenes:
  - synthetic scenes sized like gi_door (200 probes) and relit
    testchmb_a_00 at 4 m (4862 probes, 14 lights in light cells), generated by
    the formats' own writers and cached by specification;
  - optional map outputs: gi_door, relit 4 m, and the 2 m grid of 34272
    probes.
- Self-tests: 26 in `test_gi_sdf_bench.py` (validation, verdicts, parsing,
  ratchet), run in `conformance.yml` with `gi_sdf_bench.py validate`.
  `gi.sdf-perf-fixtures` is in the R01 baseline.

Calibration (`quality-results/gi-sdf-perf/calibrate-20260925`, 5 rounds). The
runner is the Radeon 8060S desktop, 32 threads, at host load 30 to 60 from
other sessions' builds, so the baselines are conservative. 51 of 75 rows are
debt. The largest, per publishing frame on the frame thread (median / p95 ms):

| Scene | Frame total | Schedule + publish | Change atlas | Visibility copy |
| --- | --- | --- | --- | --- |
| gi_door (200 probes) | 0.92 / 1.53 | 0.25 | 0.49 | 0.18 |
| relit 4 m, spawn focus | 24.5 / 41.1 | 6.9 | 10.3 | 5.1 |
| synthetic 4 m, focus | 25.6 / 41.1 | 6.8 | 11.5 | 5.5 |
| synthetic 4 m, no focus | 48.5 / 64.9 | 17.6 | 21.8 | 5.4 |
| relit 2 m, spawn focus | 160 / 226 | 40.9 | 72.6 | 29.8 |

The synthetic 4 m scene tracks the real relit map, so the checked-in
fixture stands in when map outputs are absent. The producer publishes
every completed live update, about 100 times over a change. Each
publication costs about 25 ms at 4 m, so the 2 ms GPU budget is not the
binding cost. Causes found:
- `ChangeComposer::Compose` copies the whole volume per publication, and
  `Publish` walks every probe.
- `ChangeAtlas` re-encodes the whole atlas even when 94 probes changed.
- `Consume` copies the whole volume again for `OccludeProbeVisibility`.

`DirectOcclusion` (per door change, and its build per map):

| Scene | Build | Compose serial | Compose pooled (8) |
| --- | --- | --- | --- |
| gi_door (842k texels) | 369 ms | 112 ms | 19.2 ms |
| relit 4 m (3.04M texels) | 1107 ms | 625 ms | 112 ms |

Its prefilter bounds the samples of all lights together, so on a map
whose lights span it no texel is rejected. Compose's blocks are 4096
texels, so small maps get no parallelism.

GPU (contended GPU; median / p95 per update): gi_door 1.43 / 2.65 ms, relit
4 m focus 3.29 / 7.23 ms. Schedule CPU on the device is 3.8 ms at 4 m.

Verification: `render.indirect-sdf.bench-smoke` passes with g++ (default and
release) and clang++; `render.indirect-light.sdf` passes (29 checks) with the
bench change. The verification run and its sensitivity controls are in
`quality-results/gi-sdf-perf/verify-20260925`. The sensitivity controls make
the change atlas, `Schedule` and the pooled compose 4x slower and require
their rows to regress.

Reproduce:

```sh
python3 tools/quality/gi_sdf_bench.py run [--gpu] [--rounds 5] [--sensitivity]
python3 tools/quality/gi_sdf_bench.py run --workload 'producer/*' --update-baselines
python3 -m unittest tools/quality/tests/test_gi_sdf_bench.py
```

Open:
- Android, macOS and iOS have no rows.
- The engine-only stages have no bench: `ProbeFocus`, brush relighting, the
  ambient-cache reset and the uploads.
- The ray-query producer has no workloads.
- Hosted CI runs only the smoke row and the rule self-tests. The timed gate
  needs the declared runner.

## G9: Moved lights (active)

Scope, added 2026-09-25 at the user's direction: a light that moves must
move its direct light, its shadows and its bounce. The pass scenario is a
bulb swinging on a rope (fixture `swing`). The gate text is in the RFC's
[G9](0011-runtime-indirect-lighting.md#g9-moved-lights-amendment-2026-09-25).

Starting point (observed):

- The traced producers claim `kLightMotion`, backed only by G6.3's moved
  sun. That changes indirect light; the sun's baked direct light and its
  shadow stay at the baked angle.
- Their lights come from the SDFV alone; a light's position cannot change.
- The world's unbaked lights (G2.5) add direct light with Source's dlight
  falloff and no shadows.

Steps: the light set's falloff kind; the SDF shadow in the world and model
shaders; the producers' unbaked lights; the fixture and its references; the
in-game lamp; the gate harness; costs.


### G9.1 Inverse-square dynamic lights (done 2026-09-25)

- `DLIGHT_INVERSE_SQUARE` (0x10, public/dlight.h) is `light_dynamic`
  spawnflag 16: the entity's flags reach the dlight unchanged.
- The light set carries `LightFalloff` and `sourceRadius`.
  `light_set::InverseSquareFalloff` is (100 / d)² clamped at the 2-unit
  source radius, with the window ( 1 − (d / radius)⁴ )² falling to 0 at the
  radius (radius 0: unbounded).
- The native world path (`world_pbr.frag -DDIRECT_LIGHTS`) evaluates it from
  the direct-light block's spare floats. Models reach the same light through
  the engine's dlight-to-world-light conversion (engine/lightcache.cpp), with
  a quadratic attenuation of 1 / 100².
- Legacy lightmapped surfaces (brush entities) keep the dlight falloff; they
  never had the Lambert term either.

Evidence:
- `render.world-pbr.native-pixels`, 86 checks: the inverse-square light
  draws 166 against the CPU model's 165.8, and the seeded legacy-falloff
  reading misses it.
- `render.light-set` and its three seeded builds, 36 checks: the falloff
  kind and source radius travel per light; the scale is 1 at 100 units and
  0.25 at 200, clamped at the source, and 0 at the radius.

## Spark bursts emit light (2026-09-25, user request)

Trail-spark bursts now light their surroundings through the light set. The
policy lives in [`render/spark_light.h`](../public/render/spark_light.h)
(`render.spark-light.v1`). It is tier0-free C++11, so the client and the
suite run the same code.

- **One light per burst.** A burst gets one light, not one per spark,
  because the native world path takes at most seven unbaked lights. The
  light sits at the emission-weighted centroid of the live sparks. A spark
  emits its drawn ramp times its remaining life fraction.
- **Fade.** The light's strength is the frame's emission over the burst's
  peak, and it goes dark with the last spark. The drawn ramp moved into the
  header, and `CTrailParticles::RenderParticles` now uses it.
- **Transport.** `CTrailParticles::EmitLight`
  ([`fx_sparks.cpp`](../game/client/fx_sparks.cpp)) carries each lit burst
  as a dlight keyed `LIGHT_INDEX_SPARK` plus a serial. The light is lit
  where the burst starts, because the first simulate is a frame later. It
  refreshes each simulate with a 0.1 s hold, so the engine never gives its
  slot away mid-burst. On release it is darkened and expired.
- **Budget.** `fx_spark_lights` (default 4) caps how many bursts hold a
  light at once. A burst never takes a slot another dlight holds.
- **Wired effects.** `FX_ElectricSpark` (env_spark, TE sparks), `FX_Sparks`,
  `FX_MetalSpark` (ricochets), `FX_MetalScrape` and `FX_SparkFan`.
- **Color and strength.** The color is effects/spark's bright-texel warm
  white at 30% strength: mantissas 77/67/51. Exponent 2, or 1 for scrapes.
  Radius 64 + 64 × magnitude, capped at 384.

**Headless.** `render.spark-light` runs 28 checks, on g++ in the default and
`-O2 -DNDEBUG` configurations:

- the ramp matches a copy of the original expression bit for bit, including
  the double-vs-float `0.3` edge;
- the weighted centroid, the peak-relative fade and batching;
- a scripted burst of falling sparks: the light only fades, stays within the
  sparks and goes dark with the last one;
- the budget.

Three sensitivity builds each have 4–5 checks rejected: an unweighted
centroid, a light that never fades, and a leaking budget. Evidence:
`quality-results/spark-light/headless-{default,release}.json`.

**Engine** (native Vulkan, headless). An `env_spark` (magnitude 2) is fired
with `fx_spark_lights` 4 and then 0:

- the light set carries one dlight at the burst, color 1.19 1.04 0.78 and
  radius 192, and none with the budget at 0;
- testchmb_a_01 (legacy lightmap dlights): a warm pool on the wall and floor
  around the burst; the frame mean is 106.3 against 88.5 unlit;
- an earlier full-strength run on living_room (WMSH `world_pbr` direct
  lights) lit the floor, baseboard and wall around the burst, and nearby
  models.

The 30% living_room rerun is not evidence. Its no-spark baseline already
rendered placeholder materials, because the shared build carried other
sessions' in-progress renderer changes. Evidence:
`quality-results/spark-light/{legacy-ab,wmsh-living_room}`.

**Open:**

- PCF (DMX) particle sparks emit no light. Their operators run on the
  pooled batch, so they would need a host-thread commit.
- The native consumer keeps the first seven unbaked lights in slot order,
  not the strongest.
- Sparks that fall through the floor (an existing particle-collision
  behavior) carry their faint tail light below it.
- No DXVK run, per the native-focus decision.

### G9.2 SDF shadows for unbaked lights (done 2026-09-25)

Decision 4 is SDF shadows (recorded in the RFC).

- **Upload.** The engine uploads the map's SDFV distances once per map
  (`IWorldMeshUpload::UploadShadowField`, interface `WorldMeshUpload006`;
  re-sent after device recovery). The native backend holds them as an R16F
  volume with trilinear, clamped filtering. Volume uploads now accept R16F.
- **Shader.** `world_pbr.frag -DDIRECT_LIGHTS` shadows each unbaked light
  with `SdfShadow`:
  - a sphere trace from a voxel off the surface toward the light, at most
    96 steps;
  - softened by the light's angular size: the minimum of `k · d / t`, with
    `k` the distance to the light over its source radius (at least 1 unit);
  - outside the field nothing occludes.
- **Binding.** The field is its own descriptor set after the variant's
  others: set 9 (the direct-light layout, now 10 sets) or set 10 (the
  BakedPlusDelta layout, now 11). A map without an SDFV binds a white
  volume, and the block's header says the field is absent.
- **Direct-light block.** It gained the field's origin, voxel and
  dimensions: 3 header `vec4`s plus 7 lights, 496 of the 512-byte block.

GPU check (`render.world-pbr.native-pixels`, 90 checks), with an exact
sphere field of 0.02-unit voxels across the path to a bulb of source
radius 0.05:

| Case | Red |
| --- | --- |
| Occluder on the path | 0 |
| Occluder beside the path | 166, against the unshadowed 165.8 |
| Grazing occluder | 15, a penumbra |
| Seeded control: field removed | lit, and rejected by the occluded check |

**Found on the way: present semaphores.** With the validation layer
active, `render.pbr-direct.native-pixels` and
`render.indirect-switching.native-pixels` failed on
VUID-vkQueueSubmit-pSignalSemaphores-00067: a submit signaled the
per-frame present semaphore while an earlier present still held it. The
context's own present now uses one semaphore per swapchain image
(`GrowRenderFinished`), as the SDL3 bridge already did. These suites had
passed earlier only because validation was off in that run. All 8 GPU
suites now pass with validation on (`quality-results/conformance.20260925T085440Z.json`).
