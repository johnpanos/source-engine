# particles.visibility-inputs.v1

`public/particles/particle_visibility_inputs.h` decides how a sprite
renderer's visibility inputs scale its particles' alpha and radius.
`SetupParticleVisibility` (`particles/builtin_particle_render_ops.cpp`)
gathers the terms and applies the rule for render_animated_sprites,
render_sprite_trail, the blob and screen-velocity renderers.

Obligations, as Portal 2's retail `SetupParticleVisibility` and
`C_OP_RenderSprites::InitializeContextData` (Mac `client.dylib` build 841,
decompiled) and CS:GO's:

- Visibility is computed when the renderer has a visibility control point or a
  positive "Visibility Radius FOV Scale base".
- With a control point, each term whose input range is not empty multiplies
  the visibility (starting at 1):
  - pixel visibility: the occlusion query of proxy-radius size at the control
    point, times the proxy radius remapped through "Visibility input
    minimum/maximum" (retail remaps the radius, not the query);
  - view dot: dot( control point forward, normalize( control point - view
    origin ) ) remapped through "Visibility input dot minimum/maximum";
  - distance: | control point - camera | remapped through "Visibility input
    distance minimum/maximum".
- Alpha and radius scales are Lerp( visibility, min, max ) of their ranges.
- A nonzero FOV base multiplies the radius scale by
  ( 1 / tan( base / 2 ) ) / projection[0][0].
- A renderer that sets none of the new inputs, with a proxy radius of at least
  1 and the default input range, keeps the previous rule's exact bits. No
  Portal or Half-Life 2 renderer has a visibility control point, so their
  effects never reach this rule.

The view origin of the dot term is the camera of the view being drawn;
retail's client answers `CurrentViewOrigin()`, the same point. Retail's
screen-space-effect branch of the distance term waits for the "screen space
effect" system field.

The sensitivity build restores the previous rule; the retail cases must reject
it.
