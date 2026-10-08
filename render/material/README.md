# Materials: where they are defined and how to change them

A material in this engine has three layers. Each layer has one owner, and a
change belongs in the layer that owns the fact you are changing:

1. **The definition**: the content's VMT and VTF files. They are licensed game
   data and are not in this repository.
2. **The interpretation**: how the engine reads a VMT and draws it. On core
   profiles this is `render.material` and the core shaders. On legacy profiles
   it is the frozen legacy material system.
3. **Policies over materials**: decisions that the VMT does not encode, such
   as whether a glowing surface also lights its surroundings. Each policy has
   one owner header.

Facts below were checked against the tree on 2026-10-04. Link to the owners
named here rather than copying their rules.

## 1. Definitions: VMT and VTF content

| Source | Where it comes from | Notes |
| --- | --- | --- |
| Portal 2 retail VPKs | `~/.local/share/Steam/steamapps/common/Portal 2`, packaged into the portal2 profile's runtime by kiln's linux-dir packager (`./kiln package portal2`) | The staged `portal2/gameinfo.txt` search order is `custom/*`, then `update`, `portal2_dlc2`, `portal2_dlc1` and `portal2` (each with its `pak01` VPK ahead of its loose files) |
| Portal / HL2 VPKs | The Portal install (kiln's portal-base content location), through `./kiln play portal` | The same search-path rules |
| Map pak lumps | Inside each BSP | Compile-time copies, such as `maps/<map>/...` cubemap-patched materials, override the VPK copy for that map |
| Published built maps | `run/maps/<map>/materials`, mounted as `custom/pbrt-<map>` | Output of the map pipelines ([PBRT map pipeline](../../quality/fixtures/pbrt-maps/README.md)), which this repository owns |
| P2:CE / Workshop PBR packs | Planned: read-only external mounts (AGENTS.md, "P2:CE high-quality PBR content mounts") | No mount manifest is installed yet |

Material names are content paths without `materials/` and `.vmt`, for
example `signage/indicator_lights/indicator_lights_floor`. A map-patched copy
is named `maps/<map>/<original path>[_x_y_z]`.

To audit what the content actually uses:

- [`tools/render/material_inventory.py`](../../tools/render/material_inventory.py)
  reports the shader, key, value and proxy usage across the Portal and Portal 2
  search paths and map pak lumps.
- [`tools/render/material_claim_inventory.py`](../../tools/render/material_claim_inventory.py)
  asks the real `render.material` claim code which VMTs the core draws. It
  writes the checked-in inventories
  [`quality/materials/portal2-all-claims.json`](../../quality/materials/portal2-all-claims.json)
  and `portal2-model-claims.json`. The suites
  `render.material.all-claim-inventory` and `render.material.model-claim-inventory`
  check them with `--verify`.

**Do not edit retail content to change behavior.** Do not edit VPKs, and do not
check loose VMT overrides into the repository. A loose file under a mounted
`custom/` folder does override retail content locally, but it is not
reproducible, it changes the compatibility reference, and it redistributes
licensed data if shipped. New authored content (PBR VMTs, a future `.surface`
sidecar from RFC 0015) goes through the content pipeline with recorded
provenance.

## 2. Interpretation: how a VMT becomes pixels

### The render core (`r_core_world 1`, the default for `./kiln play portal` and `./kiln play portal2`)

| Concern | Owner |
| --- | --- |
| VMT parsing: patches, fallbacks, conditionals, variables | [`vmt_import.cpp`](vmt_import.cpp) |
| Shader → family and VMT key → parameter rows; the one table both the importer and the family schemas are built from | [`vmt_mapping.cpp`](vmt_mapping.cpp), [`vmt_mapping.h`](../../public/render/material/vmt_mapping.h) |
| Families: claims and refusals by name, parameter blocks | `*_family.cpp` here, such as [`unlit_family.cpp`](unlit_family.cpp), [`lightmapped_family.cpp`](lightmapped_family.cpp), [`vertexlit_family.cpp`](vertexlit_family.cpp), [`pbr_family.cpp`](pbr_family.cpp), [`refract_family.cpp`](refract_family.cpp), [`water_family.cpp`](water_family.cpp), [`cable_family.cpp`](cable_family.cpp) |
| The one surface program and its terms | [`surface_program.cpp`](surface_program.cpp), GLSL in [`families/`](families/) (`surface_material.glsl`, `surface_lighting.glsl`, ...) |
| Shared shading math | [`render/shaders/common/`](../shaders/common/) (`pbr_brdf.glsl` mirrors [`pbr_brdf.h`](../../public/render/pbr_brdf.h)) |
| PBR VMT schema | [`pbr_material_schema.h`](../../public/render/pbr_material_schema.h) (RFC 0007) |
| Unmapped legacy shaders | `legacy_shaders.inc`; they run in the `legacy` family through the frontend's ports |
| Content-build compiler for `material.vmt` (RFC 0015) | [`content/material_compiler.cpp`](../../content/material_compiler.cpp) |

The design rules are in RFC 0016's
[surface model](../../RFC/0016-render-core.md#the-surface-model-legacy-definitions-in-the-modern-core-plan-2026-09-28-amended-2026-10-03).
Every term has a neutral value, and the legacy shaders are parameter values of
one general model. Each non-neutral VMT setting is either interpreted or
refused by name. Strict mode (`r_core_world_strict`) makes a refused claimed
material fatal. Proxies still run in the game (`game/client` and the engine's
overlay bind proxies). The core reads their results, such as the selected
`$frame`, `$alpha` and `$selfillumtint`.

### Legacy paths: frozen

`materialsystem/stdshaders/`, `materialsystem/shaderapidx9/` and
`materialsystem/shaderapivulkan/` (with the native backend's shading), plus the
engine's CPU runtime-lighting files, are frozen by
[RFC 0016's binding rules](../../RFC/0016-render-core.md#binding-rules-for-all-render-work-user-decision-2026-09-28).
They take only defect fixes, core plumbing or an explicit user request, and
such a commit carries a `Frozen-path:` line. The `render.legacy-freeze` ratchet
(`tools/render/retirement_scans.py legacy-freeze`) enforces this. Do not add
material behavior there.

## 3. Policies over materials

Some behavior is a product decision that the VMT does not specify. Each
decision has one owner. Change it there, with the decision's date and the
reason.

| Policy | Owner | Consumers |
| --- | --- | --- |
| Which surfaces publish **area light** onto core receivers (surface-source policy) | `SurfaceSource`, `IndicatorLineMaterial` and `UnlitRadiance` in [`public/render/emissive_area_lights.h`](../../public/render/emissive_area_lights.h); rationale in [RFC 0016](../../RFC/0016-render-core.md#surface-emission-sources-installed-indicator-slice-2026-10-04) | `engine/world_emitters.cpp` (world faces, brush entities, overlays); `game/client/emissive_area_lights.cpp` (live frame, tint and strength; `cl_surface_core_emission*`) |
| World panels (chamber signs, elevator movie screens): image, coatings and tile lights | [`public/render/world_panel.h`](../../public/render/world_panel.h) | `render.pass.panels`; client panels |
| Emissive model sources, security-camera eyes | `emissive_area_lights.h` (`AttachmentEmitter`) and the client publisher | `game/client/emissive_area_lights.cpp` |

Visible emission and light publication are separate decisions. Removing a
surface's light never changes how the surface itself draws.

Current surface-source policy:

- **v1 (2026-10-04)**: self-illuminated (`$selfillum`) and fullbright
  `UnlitGeneric` scene surfaces light core receivers. Sky, nodraw and faces
  already baked as texture lights are excluded.
- **v2 (user decision, 2026-10-04)**: Portal 2 indicator lines (antlines),
  `signage/indicator_lights/*` and `overlays/indicator_lights*`, also as
  map-patched copies, stay visible but publish no light. Door-state indicator
  boxes, signs, chamber boards, elevator screens and other sources keep v1.

## How to make a change

1. **Find the layer.** Does it change how a VMT looks? That is the
   interpretation (section 2). Does it change whether or how strongly a
   surface does something the VMT does not specify? That is a policy
   (section 3). Is the content itself wrong? Report it; do not patch retail
   files.
2. **Read the owning RFC section** linked above. For core render work, the
   binding rules apply: prove it in `render_lab` first, then integrate it in
   the game, and delete any old copy you replace.
3. **Change only the owner.** Route callers through it; do not add a second
   list of material names or parameters elsewhere. Name-based rules belong in
   the policy owner as reviewed, dated decisions, not scattered string checks.
4. **Add or adjust the oracle**, including a seeded negative:
   - policy and source math: `unittests/rendertest/test_area_light.cpp`
     (`render.area-light`) and the other `unittests/rendertest` suites;
   - family claims and terms: the `render.material.*` and `render.lab.*`
     suites, and the claim inventories (`material_claim_inventory.py
     --verify`; regenerate them only after reviewing the diff);
   - the real game: [`tools/quality/intro4_material_check.py`](../../tools/quality/intro4_material_check.py)
     through `intro4_strict_game.py`, in strict FSR-on and FSR-off modes
     ([guide](../../tools/quality/render_profile.md#strict-intro4-material-captures)).
     Run game windows in a private headless mutter, not on the desktop
     session.
5. **Record it.** Update the RFC section that owns the rule, and add evidence
   to `RFC/0016-progress.md`. Report the suites, captures and modes that
   passed, and what was unavailable.

### Worked example: no light from antlines (2026-10-04)

- Policy owner: `emissive::IndicatorLineMaterial`, applied by
  `emissive::SurfaceSource(selfIllum, shader, material)` in
  `public/render/emissive_area_lights.h`.
- Consumer: `engine/world_emitters.cpp` passes `material->GetName()` at both
  ingress points (brush faces and overlays). The client publishes only what
  the engine supplies, so it needs no change.
- Oracles: `render.area-light` asserts that antline paths are excluded in every
  path form while other sources stay included. `intro4_material_check.py
  --scene emissives` now requires that the floor line still changes its visible
  frame but changes none of its wall receivers, with seeded line-light defects.
- What the change exposed: the old "indicator box lights its wall" check was
  passing on a neighbouring wall antline's light. The box is flush with the
  wall and does not light that wall. The box case now uses an isolated source
  and a receiver in front of the box, plus the light report. Check what a
  receiver actually measures by isolating its source with
  `cl_surface_core_emission_filter <material>`.
