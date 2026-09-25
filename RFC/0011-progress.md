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
| G1 probe volume, baked producer | active | Bake, pack, engine load and fallback, per-pixel `model_pbr` sampling and the CPU ambient cube done; all native oracles pass; the DXVK check (G1.7) is deferred by user direction; corpus load and memory being recorded |
| G2 light set, separated bake, policy | done (2026-09-24, native Vulkan) | `render.light-set.v1` published each frame, with seeded ID reuse rejected; separated-bake consistency with swapped and doubled layers rejected; `render.indirect-policy.v1` with the double count rejected by the furnace in the CPU model, the GPU and the engine; native WMSH direct light from unbaked lights |
| G3 producer contract and switching | planned | — |
| G4 precomputed radiosity | planned | — |
| G5 GPU compute foundation | planned | — |
| G6 SDF-traced producer | planned | — |
| G7 ray-query producer | planned | — |
| G8 product defaults and soak | planned | — |

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
