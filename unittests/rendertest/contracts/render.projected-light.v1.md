# render.projected-light.v1

`public/render/projected_light.h` makes an `env_projectedtexture` a light of
the light model (RFC 0011, light set v2 `Snapshot::projected`). It lights like
every other light and is shadowed like every other light.

## The light

- A frustum: origin, unit forward/right/up, horizontal and vertical field of
  view, near and far.
- A linear color, with brightness and light style folded in.
- Attenuation factors (constant, linear, quadratic).
- A cookie texture and frame, whether it casts shadows, and whether it lights
  the world.

## The rule

The rule is the legacy flashlight shader's (`common_flashlight_fxc.h`
`DoFlashlight`):

    light = color x cookie( u, v ) x saturate( c + l / d + q / d^2 )
            x endFalloff( d ) x max( 0, n . L )

- `endFalloff` is 1 up to 0.6 × far and falls linearly to 0 at far.
- `( u, v )` is the point's place in the frustum: u runs along right, v runs
  down from up, and both span 0..1 across the field of view.
- Points outside the frustum or nearer than the near plane take nothing.
- Units are the lightmap's, as the flashlight pass added them.

## Consumers

- **The client** (`game/client/c_env_projectedtexture.cpp`,
  `game/client/projected_lights.cpp`) publishes each lit projected texture
  every frame (`VEngineProjectedLights001`), keyed by entity handle. It draws
  no flashlight pass for it, unless the light is limited to a target entity.
- **The engine** (`engine/projected_lights.cpp`) versions each light. A new,
  changed or removed light dirties the world surfaces within its frustum's
  bounding sphere.
- **World lightmaps** (`engine/gl_lightmap.cpp`, `R_AddProjectedLights`) take
  the rule per texel and per bump vector. The light is shadowed by the world
  (a cached trace per texel) and by moving objects
  (`render.dynamic-occlusion.v1`, a 4-unit disk). A surface keeps its result
  while the lights' and nearby boxes' versions hold.
- **Models and static props** (`engine/lightcache.cpp`) take a stand-in point
  light matched to the rule at their lighting origin, shadowed by the world.
  Moving objects shadow it through every model light's visibility
  (`l_studio.cpp`). The stand-ins are kept per lightcache entry while the
  lights' versions hold.

## Oracle

- The projection is judged against a matrix oracle: a world-to-light view
  matrix times a D3D perspective matrix, then the texture remap, as the
  client builds the flashlight's world-to-texture matrix. It uses 200 random
  lights with unequal fields of view, 200 points each; points within 1e-4 of
  the frustum's edge are skipped.
- The attenuation is checked against the shader's expression every 7 units.
- Also checked: the Lambert term, the cookie coordinates, and that the
  bounding sphere holds the frustum's corners.

## Sensitivity

| Build | Defect |
| --- | --- |
| `no-end-falloff` | no far-plane falloff |
| `swapped-fov` | horizontal and vertical fields of view swapped |
| `no-lambert` | no receiver Lambert term |

## Known gaps

- Lightmap resolution blurs sharp cookies; soft ones like `flashlight003` are
  unaffected.
- A light limited to its target entity keeps the flashlight pass.
- Displacements take no projected light.
- Per-pixel evaluation is the render core's (RFC 0016 K7, the shadow atlas).
