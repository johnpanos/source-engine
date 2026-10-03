# render.dynamic-occlusion.v1

`public/render/dynamic_occlusion.h` owns how moving objects block light: one
rule for every prop, physics object, NPC part and door, and for every light
(RFC 0011). The occluders travel with the frame's light set
(`Snapshot::occluders`, `public/render/light_set.h` v2).

## Occluders

- **Who publishes them.** The client (`game/client/dynamic_occluders.cpp`,
  `VEngineOccluders001`) publishes, each frame after its views are drawn, every
  drawn entity that casts a shadow and every brush entity, within 3072 units of
  the view, nearest first, at most 512 boxes.
- **Box shapes.**
  - An animated model drawn that frame gives its hitboxes on its bones.
  - Anything else gives its collision box, or its render box without one.
  - Boxes larger than radius 160 (hollow environment pieces such as elevator
    cars) are left out.
- **Key.** A box is keyed by its entity handle (index and serial, the key the
  render core's scene instances use) and its part.
- **Versions.** The engine (`engine/dynamic_occlusion.cpp`) versions each box.
  It keeps the version while the box stays within 0.05 units (center) and 0.02
  (axes) of the pose it was published with. Any larger change, its arrival and
  its loss are new versions.

## Visibility

- **Samples.** A light reaches a receiver through the fraction of its samples
  whose segment from the receiver no box crosses:
  - a point, spot or surface light: a 9-sample disk facing the receiver
    (radius 6, or 16 for a surface light), so shadows have penumbrae;
  - a distant light: one sample 32768 units up its direction;
  - an area light: 4×4 samples over its rectangle.
- **Mixing.** Each light is judged on its own
  (`MixLights`: `sum_i contribution_i x visibility_i`), so shadows from several
  lights mix. Each light loses only its own blocked share.
- **Self.** A receiver that belongs to an entity (a model) ignores that
  entity's boxes.

## Consumers

- **World lightmaps** (`engine/gl_lightmap.cpp`, `R_ApplyDynamicOcclusion`):
  - Each texel loses, for each world light the bake let reach it, the light's
    direct light times one minus its visibility. The world is traced once per
    (texel, light) and cached for the map.
  - The direct light comes from the engine's world-light formula (falloff times
    angle times style). It is scaled down where it exceeds the texel's baked
    light, so a shadow never removes more than the texel holds.
  - Bumped lightmaps lose the same light along its direction.
  - A box dirties the world surfaces within its shadow reach (6 × its radius,
    at most 384) when it changes. A surface keeps its blocked light, cached per
    surface, while the nearby boxes' versions stay the same.
- **Model lighting** (`engine/l_studio.cpp`): each of a model's local lights is
  scaled by its visibility at the model's lighting origin, the model's own boxes
  ignored.
- **The GI path** (`engine/indirect_light_host.cpp`): the SDF and ray-query
  proxies and `DirectOcclusion` take the same boxes, as world-space bounds.
- **Blob shadows.** Render-to-texture and blob shadows are not drawn while
  this is active (`r_dynamic_occlusion 1`, the default). Projected-texture
  shadows are unchanged.

## Oracle

- The segment/box test is judged against marching the segment in 4,000 steps
  over 4,000 random rotated boxes. Segments that only graze a box are excluded:
  those are where the march disagrees with itself when the box grows or shrinks
  by 2%.
- Also checked:
  - a cube's shadow on the floor behind it and under it;
  - a model's own boxes;
  - another entity's box;
  - a half-hidden area light's penumbra;
  - distant lights;
  - the mixing of two lights.

## Sensitivity

| Build | Defect |
| --- | --- |
| `aabb` | boxes tested without their rotation |
| `no-self` | a model shadowed by its own boxes |
| `hard` | a light blocked entirely when any sample is |
| `shared` | one visibility applied to every light |

## Known gaps

- Boxes approximate shapes, and hollow pieces larger than radius 160 are left
  out.
- The ambient cube of model lighting (lights folded beyond a model's local
  lights) is not occluded.
- Static props' baked vertex lighting and displacements take no occlusion.
- Dynamic lights (dlights) and area lights are not occluded yet.

## Core physical caster extension

The preceding box/CPU contract remains v1. The core additionally consumes
`VEngineOccluders002`, defined in [dynamic_occlusion.h](../../../../public/render/dynamic_occlusion.h).
Its client producer selects opaque animated models using custom box and ray
collision, and follows their authored bone-follower selection (including the
server's multi-solid fallback). Physical triangles on the current client bones
block core lights even when the model disables its legacy blob shadow.

Publication follows render-start interpolation, animation and bone work, before
the engine publishes lighting. The main sequence owns publication; queued views
own copies. Borrowed client storage is consumed synchronously. Invalid triangle
lists, duplicate keys, nonfinite positions and capacity violations reject the
whole frame, preserving geometry and revisions. Unchanged geometry keeps its
revision; an empty publication removes the parts. Map shutdown clears the set
and model cache. The v1 interface and CPU receivers retain their original inputs.

The core's existing shadow atlas draws these meshes in place of that entity's
coarse boxes. Both imported BSP surfaces and WMSH stages use the same area-light
shadow mechanism. The CPU publication tests exercise ownership and rollback;
`render_lab shadowed-lights` checks closed/partial/open/reclosed triangle leaves
against an independent aperture oracle and rejects missing leaves. The installed
`render.product.fizzler-door-light` fixture supplies the real animated-door and
receiver check. This cohort uses authored physical geometry; full visual-mesh
skinning/cutout shadow coverage and complete-frame performance remain separate
RFC 0016 gates.
