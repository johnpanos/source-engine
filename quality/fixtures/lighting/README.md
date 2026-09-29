# RFC 0016 K11 lighting fixtures

The versioned scenes, cameras and Cycles references that `render_lab` judges
the core's lighting model against ([RFC 0016 K11](../../../RFC/0016-render-core.md#k11-lighting-model-proven-in-render_lab),
"Hard parts first, proven outside the game"). They extend the RFC 0011 GI
gallery (`quality/fixtures/gi/`): every synthetic fixture is a
`gi-fixture/v1` stage and record, so the gallery's tools read it, plus a
`lighting` block.

Everything here except `README.md`, `tolerances.json`, `references/` and
`results/` is written by `tools/quality/lighting_fixtures.py generate`.
Do not edit generated files by hand.

## Fixtures

| Fixture | Scene | Terms (manifest ids) | Cameras | States |
| --- | --- | --- | --- | --- |
| `cornell-floors` | Cornell box, floor split into a rough (0.5) and a polished (0.05) half, ceiling rectangle, dynamic probe sphere | brdf, filtered-roughness, area-lights, direct-visibility, indirect-diffuse-static, indirect-diffuse-dynamic, image-based-specular, screen-space-reflections, ambient-occlusion, specular-occlusion | `front`, `floor` | `default` |
| `area-room` | 8 x 6 m room, 64 rectangles (48 colored ceiling panels, 16 low wall strips grazing the floor), floor stripes at roughness 0.1/0.3/0.6, pedestals, an emissive sign that is not a light | area-lights, brdf, direct-visibility, indirect-diffuse-static, emission, image-based-specular | `overview`, `grazing`, `wall` | `default` |
| `projector-cookie` | dark room, `env_projectedtexture` with a window cookie over a Lambertian floor and back wall, a shadowing pillar, a glossy sphere, a dim fill rectangle | projected-lights, direct-visibility, brdf, indirect-diffuse-static | `room`, `wall` | `default`, `projector` (fill off) |
| `sun-colonnade` | 30 m portico: ten columns, roof slab, back wall, yard; sun at 35 degrees elevation and a blue sky | sun, direct-visibility, indirect-diffuse-static, indirect-diffuse-dynamic, ambient-occlusion, brdf | `along`, `yard` | `default` |
| `foggy-hall` | 24 m hall with a homogeneous medium (scattering 0.06/m, absorption 0.01/m, g 0.3), 16 ceiling spots + 240 wall bulbs (256 clustered lights), a projector shaft shadowed by a pillar | participating-media, runtime-lights, projected-lights, direct-visibility, indirect-diffuse-static, brdf | `nave`, `side` | `fog`, `clear` (density zero) |
| `mirror-corridor` | 18 m corridor, floor from a metal mirror (0.02) through gloss 0.1 and 0.25 to 0.5 (above the SSR cutoff), colored wall panels, ceiling strips, a block near the camera whose reflection leaves the screen | screen-space-reflections, image-based-specular, filtered-roughness, specular-occlusion, brdf, area-lights, indirect-diffuse-static | `down`, `low` | `default` |
| `material-sweep` | a wall of 24 smooth spheres, roughness 0.05, 0.1, 0.2, 0.3, 0.45, 0.6, 0.8, 1.0: gold metal, clear coat (coat roughness swept) over red, white dielectric; key rectangle and a rim spot | brdf, image-based-specular, filtered-roughness, area-lights, runtime-lights, indirect-diffuse-static, specular-occlusion | `front`, `grazing` | `default` |
| `portal-pair` | two sealed rooms joined only by a linked `prop_portal` pair on perpendicular walls (A's east, B's south): A's ceiling rectangle and a spot aimed through portal A past a post; B has a dim lamp of its own, a rough/polished floor split, a block and a dynamic probe sphere | brdf, area-lights, runtime-lights, direct-visibility, indirect-diffuse-static, indirect-diffuse-dynamic, image-based-specular, portal-transport | `b-portal`, `b-floor`, `a-portal` | `closed` (the portal world; the map's bake), `open` |
| `portal-chamber` | Portal's `testchmb_a_00` as relit by `legacy_bsp_relight.py` (map `testchmb_a_00_relit`) | brdf, runtime-lights, direct-visibility, indirect-diffuse-static, image-based-specular, emission | `vault`, `room2` (the K0 view-oracle poses) | `default` |
| `portal2-chamber` | `sp_gi_chamber_01` as built by `portal2_gi_chamber.py` | brdf, runtime-lights, direct-visibility, indirect-diffuse-static, image-based-specular, emission, ambient-occlusion | `spawn`, `chamber` | `default` |

Terms and their negative controls are in `manifest.json` (`terms`), each
naming its owning definition (binding rule 6). `output` is listed for
completeness and judged by `render.output`, not here: every comparison is of
linear scene radiance before exposure and output.

Film: 512 x 384, 90 degree horizontal field of view (Source's `fov 90` at
4:3, as the gallery).

## Files

Per synthetic fixture `<name>/`:

- `<name>.usda` (+ `states/*.usda` override layers): the authored scene,
  Z up, meters. The first state is the base stage; other states switch
  lights (`inputs:intensity` 0) so geometry is identical across states.
- `fixture.json`: `gi-fixture/v1` (cameras as eye/forward/up in meters,
  regions by mesh name, film, states) plus `lighting`: `terms`, light counts,
  `projectors` (the `render.projected-light.v1` fields in Source units),
  `media` per state, and `map` (published map name and BSP path, baked
  state, the preview bake overrides, collision solids).
- `entities.json` (`lighting-entities/v1`): the lights once, as declared
  (`lights`, meters and the lightmap unit), and as the Source entities the
  map's entity lump carries (`entities`, Source units).
- `window_gobo.png` (projector fixtures): the cookie, 128 x 128, values linear
  (pixel / 255).
- `references/`: `<state>.<camera>.total.exr` (Cycles Combined, linear
  scene radiance, uncompressed half-float scanline EXR: relative precision
  5e-4, far below any tolerance),
  `<state>.<camera>.index.png` (16-bit object index; the ids are in
  `references.json`), and `references.json` (`lighting-references/v1`).

The chamber fixtures have only `fixture.json` and `references/`: their scene
is the relight build's derived stage (`external_stage.path`, untracked under
`quality-results/`), rebuilt by the command in `external_stage.rebuild`.
Their maps are the published `run/maps/testchmb_a_00_relit` and
`run/maps/sp_gi_chamber_01`, with the maps' own entity lumps.

## Light through portals (`portal-pair`)

The term `portal-transport` is how every other term is evaluated across an
open, linked portal pair. Its owner is `render.portal-lights.v1`
(`unittests/rendertest/contracts/render.portal-lights.v1.md`, RFC 0011), as
the render-core owner decided on 2026-09-29. It is not a row of its own in
the lighting model.

- **The portal world** is the base state `closed`, which the map is built
  and baked from. Each portal wall is the wall with the portal's opening cut
  out, plus a plug that fills it. The pair is in the entity lump as
  `prop_portal` (`Activated 1`, linkage group 0, `PortalTwo` on B). The
  lighting block lists the portals (`portals`): centre, frame, half size in
  Source units, and `aperture: ellipse`.
- **The reference** is the state `open`. The plugs are out, and each room is
  joined through the opening to a copy of the other room (`Joined*` meshes
  and lights), placed behind its portal wall by the pair's transform. The
  transform is Source's `MatrixThisToLinked`: local forward and right
  negated, up kept. For one pair this is the exact equivalent of every light
  path through the pair, with any number of crossings. The copies carry no
  portal wall, because each room's own portal wall is the other's under the
  transform.
- **The opening is the portal's visible shape**: the ellipse inscribed in
  Portal's 64 x 108 units (64 segments), as decided with the render-core
  owner. `render.portal-lights.v1` P2 clips images with the rectangle. That
  deviation shows up here as extra light at the corners of each
  through-portal pool.
- Every camera's `through` region is the copy seen through the opening in
  `open`, and the plug in `closed`. The lab passes it only by drawing the
  view through the portal.
- The negative control is the pair closed, judged against `open`.

## Lights in the entity lump

One declaration per light (`LightingScene` in the generator) writes both
the UsdLux light (what Cycles and the bake see) and its entity (what the lab
reads), in the pipeline's lightmap unit (a white Lambertian surface under
irradiance E shows E / pi):

| Light | USD | Entity | Conversion |
| --- | --- | --- | --- |
| point | `SphereLight`, radius 2 units | `light`, `_quadratic_attn 1` | vrad world light intensity L r^2 (r = 2 units) |
| spot | `DiskLight` + `sourceEngine:cone*` | `light_spot`, `_inner_cone`, `_cone`, `_exponent`, `pitch`/`angles` | same, on axis |
| sun + sky | `DistantLight` (normalized) + `DomeLight` | `light_environment`, `_light`, `_ambient`, `SunSpreadAngle` | sun world light E / pi; sky ambient the dome radiance |
| rectangle | `RectLight` | `light_rect`: `origin`, `angles` (forward = emission direction), `width` along -right and `height` along up (halfU = -right x width/2, halfV = up x height/2, so halfU x halfV = forward), `color` (linear 0..255), `brightness`, `two_sided` | the `render.area-light.v1` `Rect` and radiance: radiance = color / 255 x brightness |
| projector | none (not bakeable) | `env_projectedtexture`: `angles`, `lightfov`, `nearz`, `farz`, `lightcolor`, `texturename` | Portal's attenuation (0, 100, 0), color GammaToLinear(rgb) x A / 255 |
| medium | none | `env_volumetric_fog_volume`: `origin`, `box_mins`, `box_maxs` (relative), `density` (extinction per unit), `albedo`, `anisotropy`, `emission`; plus one `env_volumetric_fog_controller` (`density`, `height_fog_density`, `height_fog_falloff`, `anisotropy`; all 0 here) | homogeneous, Henyey-Greenstein |

Stock lights and `light_rect` carry their declared name as `_fixture_light`,
not `targetname`: vbsp makes every named light* a switchable style and stops at
32. `light_rect`, `env_volumetric_fog_volume` and `env_volumetric_fog_controller`
were decided by the render-core owner (source-engine-43, 2026-09-29, binding
rule 5) with Source 2's names; the game entities themselves are RFC 0016
K12's. `build` compiles the entities with vbsp/vrad and requires
vrad's compiled world lights to equal the stage's lights within 1%
(`world-lights.json` in the build directory); the unit tests replay vrad's
arithmetic on the converters.

## References

`lighting_fixtures.py render` extracts each state's stage through
`usd_scene.py extract` (the map pipeline's USD front end) and renders every
camera with `lighting_reference_blender.py`: `gi_reference_blender.py`
unchanged (pipeline material, emitter and lamp policy, the `gi-reference`
light paths: 64 diffuse and 16 glossy bounces, no clamping, no denoising,
CPU, fixed seed 20260929) plus two additions the USD stage cannot carry:

- **Projectors**: a lamp whose node tree evaluates
  `render.projected-light.v1` (`public/render/projected_light.h`): cookie
  through the perspective frustum, near and far culling,
  `saturate(c + l/d + q/d^2)`, end falloff and n.l. Projected lights are
  never baked (RFC 0011), so the lamp lights depth-1 paths only (camera-ray
  hits and single scattering); its bounce is not in the reference, as it is
  not in the model's baked indirect light. `render` checks the result
  against an independent numpy evaluation of the contract on the projector
  state's Lambertian floor and back wall (median ratio within 3% of 1, 80%
  of pixels within 10%); on the preview references the median ratio is
  0.99998 (`room`) and 0.99999 (`wall`), with 96% and 99% of pixels within
  10% (the rest are shadow and cookie edges).
- **Media**: the homogeneous medium as the world volume. With no volume
  bounces the reference holds single scattering of the lights, which is
  what the model's froxel volume defines. Medium states take their object
  index from the medium-free state (same geometry and cameras), because
  scattered camera samples leave the index at 0.

Visible light shapes (`LightQuad*`, `LightDisk*` meshes) are in the images
but not judged.

**Sample counts.** The checked-in references are previews at 16 samples
per pixel (`status: preview`), rendered 2026-09-29 in 56 s for all nine
fixtures. A comparison against a preview is recorded but certifies nothing.
The final references are a separate step, not run yet:

```sh
python3 tools/quality/lighting_fixtures.py render --samples 2048
python3 tools/quality/lighting_fixtures.py check
```

(or one fixture at a time with `--fixture NAME`). A recorded comparison
names its reference by digest; after a re-render `check` rejects it as
stale until it is run again.

## Tolerances and comparisons

`tolerances.json` (`lighting-tolerances/v1`) holds each fixture's mean and
99th-percentile error bound, fixed on 2026-09-29 before any comparison
existed. The metric (`manifest.json` `error_metric`):

    e(p) = max over R, G, B of |lab(p) - ref(p)| / Y_ref

with `Y_ref` the mean Rec. 709 luminance of the reference over the judged
pixels (scene meshes, not the background or sky and not light shapes);
`mean` averages `e`, `p99` is its 99th percentile.

Rules, enforced by `check`:

- an entry's digest covers fixture, version, mean, p99 and fixed time; an
  entry edited in place is rejected;
- a changed tolerance is a new version with a later `fixed` time and a
  `reason`; a version fixed after a recorded comparison of its fixture with
  no reason is rejected;
- a recorded comparison (`results/*.json`, `lighting-comparison/v1`) names
  its tolerance version and digest and its reference's digest; one whose
  tolerance was fixed after it or changed since, or whose reference has
  changed since, is rejected.

## Maps

`build` makes each synthetic fixture's map with the existing front and back
ends, no new pipeline code: the scene front end's collision VMF
(`pbrt_collision_vmf.py --no-fallback-light`) plus `entities.json`,
compiled by `vmf_map_build.py` (vbsp, vvis and vrad `-fast`), then the one
lighting back end (`map_lighting.light`, bsp + authored scene, profile
`gi-fixture`) and `playable_maps` publishing to `run/maps/lt_<name>`
(`./play lt_<name>`). The bake is a preview too: `fixture.json`
`lighting.map.overrides` sets lightmap samples 64, probe-volume samples 256,
reflection probes (where the fixture has glossy surfaces) at 512 wide, 128
per face and 16 samples, and no radiosity transfer or SDF volume (K11 reads
neither). The packed BSP2 carries WMSH, LMAP (direct and indirect layers),
PRBV and, with probes, RPRB; the gameplay identity gate passes. The
projector's cookie is compiled by VTEX into the published map as
`materials/lighting/cookies/window_gobo.vtf` (uncompressed).

Built on 2026-09-29 (preview bake, CPU): `lt_cornell_floors` (39 s),
`lt_area_room` (68 s), `lt_projector_cookie` (24 s), `lt_sun_colonnade`
(78 s), `lt_mirror_corridor` (61 s), `lt_material_sweep` (61 s).
`lt_foggy_hall` is **not built**: its compile passes and all 256 compiled
world lights match the stage, but the pack step
(`usd_worldmesh_pack.py`, `EMITTER_NAME = r"Light(?:Quad|Disk)\d{2}"`)
takes the 101st UsdLux emitter mesh, `LightQuad100`, for an unbound world
mesh and stops. Widening the pattern to `\d{2,}` (its owner's change) lets
`build --fixture foggy-hall` finish; until then `check` fails on that map.

The final bakes (not run) drop the preview overrides for the profiles'
sample counts (lightmap 2048, probe volume 4096; reflection probes 1024
wide, 256 per face, 64 samples):

```sh
OMP_NUM_THREADS=1 python3 tools/quality/lighting_fixtures.py build --final
```

## Commands

```sh
# fixtures (OpenUSD Python); --check regenerates into a temporary tree and diffs
PYTHONPATH=build/toolchains/openusd-25.11/lib/python \
    /usr/bin/python3.12 tools/quality/lighting_fixtures.py generate [--check]
# preview references (16 samples; about 1 minute on this host for all)
python3 tools/quality/lighting_fixtures.py render [--fixture NAME]...
# maps: collision VMF + entities -> vmf_map_build (vbsp/vvis/vrad fast) ->
# map_lighting with the authored scene (gi-fixture profile, preview bake
# overrides) -> published to run/maps/lt_<name> (./play lt_<name>)
OMP_NUM_THREADS=1 python3 tools/quality/lighting_fixtures.py build [--fixture NAME]...
# manifest, references, tolerances and recorded comparisons
python3 tools/quality/lighting_fixtures.py check
# score a lab image (linear PFM or EXR, same film and pose)
python3 tools/quality/lighting_fixtures.py compare --fixture cornell-floors \
    --state default --camera front --image lab.pfm [--record]
# the visual comparison for review: render_lab beside Cycles and the error map for
# every view, one self-contained HTML page (render_lab from --lab, $RENDER_LAB or a
# build-rc-lab tree); prints the page's path
python3 tools/quality/lighting_fixtures.py gallery [--fixture NAME]... [--lab PATH] [--out DIR]
# self-tests (negative fixtures included)
python3 -m unittest tools/quality/tests/test_lighting_fixtures.py
```

`render_lab` takes a camera as `--eye`, `--forward`, `--up` in Source units
(meters x 39.3700787) and `--hfov 90 --size 512x384`.
