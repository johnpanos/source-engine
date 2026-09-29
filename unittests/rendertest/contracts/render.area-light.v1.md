# render.area-light.v1

`public/render/area_light.h` owns area lights: light that leaves an emitting
surface rather than a point. They are part of the RFC 0011 runtime light set,
version 2 (`Snapshot::areas` in `public/render/light_set.h`).
`public/render/emissive_area_lights.h` owns how an emissive model's
self-illuminated triangles become area lights, and which are lit each frame.

The client publishes each frame's area lights to the engine
(`game/client/emissive_area_lights.cpp`, `VEngineAreaLights001`). The engine
carries each in a black dlight slot flagged `DLIGHT_AREA`
(`engine/area_lights.cpp`). The consumers today:

- world lightmaps evaluate the exact form factor per luxel and bump basis
  (`engine/gl_lightmap.cpp`);
- models take a stand-in point light at their lighting origin
  (`engine/lightcache.cpp`);
- the light set publishes the rectangles for per-pixel consumers (the
  render core's clustered LTC evaluation, RFC 0016 K7, is judged against the
  same oracle).

## Definitions

- **Rectangle.** A center and two half-extent vectors. Its front is
  `halfU x halfV`, and it is one- or two-sided.
- **Radiance.** Linear RGB in the lightmap's unit: an emitter of radiance 1
  shows the brightness of a white surface under a lightmap value of 1.
- **Irradiance.** A point `p` with unit normal `n` receives radiance times
  the form factor from `p` to the rectangle, clipped to `n`'s hemisphere.
  The form factor is 0 behind a one-sided light. The result is in the
  lightmap's unit: a receiver whose whole hemisphere is covered by radiance
  `L` receives `L`.
- **Reach and window.** The reach is where the brightest channel's on-axis
  far-field irradiance `L A / (pi d^2)` falls to 1/256, capped at 768 units.
  The light is multiplied by `(1 - (d / reach)^4)^2`, where `d` is the
  distance to the rectangle. The window is part of the definition, so every
  consumer applies it.
- **Stand-in.** For a receiver without a single normal (a model), a point
  light along the unclipped vector form factor, at the rectangle's distance
  (at least 1 unit). Its inverse-square intensity at 100 units gives the
  receiver, facing it, the area light's full light.

## Emitters

- **Emission.** A triangle's radiance is the mean over its texels of what its
  material draws as self-illumination at tint 1. For VertexLitGeneric that is
  the linear base color times the mask: `$selfillummask`, the envmap mask's
  alpha, or the base alpha. The client multiplies it each frame by the live
  `$selfillumtint` and the entity's render color.
- **Grouping and splitting.** The caller groups triangles (bone, material,
  body part). Each group first splits by facing: every triangle goes to the
  nearest of the six axis directions of the group's space. Panels facing
  apart, like the two faces of a door, never share a rectangle, and a closed
  shape becomes one outward light per facing. Each facing then splits at its
  emission-weighted median along its longest axis until every part spans at
  most 48 units, up to 4 parts. Dark triangles (below 1/255) emit nothing.
- **Fit.** Each part becomes one rectangle:
  - centered on the emission-weighted mean;
  - in the plane of the area-weighted mean normal;
  - with axes and half extents from the emission's second moments in that
    plane, never thinner than 0.25 units;
  - two-sided when the normals mostly cancel (`|sum A n| / sum A < 0.35`).
    After the facing split this cannot happen for a model's parts; the rule
    is kept for callers that hand the fit a mixed part.
- **Power.** The fitted radiance keeps the part's power:
  `radiance x area x sides = sum A_i L_i`.
- **Selection.** At most `r_area_lights` (8 on desktop, RFC 0011 budget) are
  lit. They are ranked by emitted power times `reach^2 / (reach^2 + d^2)` at
  the view. A lit light keeps its light against a newcomer less than 1.25x
  stronger. Ties go to the lower index.

## Oracle

The form factor is judged against an independent oracle, not against
Lambert's formula:

- numerical integration of `cos cos' / (pi d^2)` over the rectangle, clipped
  at both horizons;
- the closed form for a parallel receiver under a corner, summed for the
  centered case;
- the limits: an infinite light gives form factor 1, and a distant light
  gives `A / (pi d^2)`.

Tolerances: 0.5% relative (2e-5 absolute) against integration on a 600- to
800-cell grid, and 0.01% against the closed forms.

## Sensitivity

Each seeded defect must be rejected by at least one check:

| Build | Defect |
| --- | --- |
| `no-clip` | form factor not clipped to the receiver's hemisphere |
| `two-sided` | one-sided lights lighting behind themselves |
| `point` | a point light at the center |
| `no-power` | the fit takes the mean radiance, not the power |
| `no-keep` | selection without hysteresis |
| `no-split` | one rectangle for distant patches |

## Known gaps

- Displacements, the WMSH world (BSP2 maps) and per-pixel native paths draw
  no area light yet. Per-pixel LTC belongs to RFC 0016 K7.
- There is no occlusion: a light reaches through a wall within its reach.
  The failing fixture for this is still to be written; it is the oracle for
  SDF shadowing later.
- Area lights are not imaged through portals.
