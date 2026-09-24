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
| G1 probe volume, baked producer | planned | — |
| G2 light set, separated bake, policy | planned | — |
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
