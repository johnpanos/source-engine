# RFC 0020: Native Game UI Qualification Scenes

- Status: Proposed (2026-10-03); qualification cases only, no toolkit selected or
  implementation gate complete
- Date: 2026-10-03
- Scope: Two required visual and interaction cases for evaluating a future
  native game UI beside VGUI. They establish what the toolkit must be able to
  express without requiring a browser or web runtime.
- Legacy compatibility: [RFC 0010](0010-portable-vgui-surface.md) owns the
  frozen VGUI API, its authored content, input and UI draw-list migration.
- Render ownership and budgets: [RFC 0016](0016-render-core.md) owns graph
  execution, GPU resources and the hard complete-frame targets;
  [`render-v1.json`](../quality/budgets/render-v1.json) owns their numbers.
- Temporal output: [RFC 0019](0019-temporal-upscaling-contract.md) owns the
  output-resolution placement of display-space HUD and text.
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md),
  Q-PRESENTATION and Q-PRODUCT.
- Tracking: unranked proposed UI evaluation. These cases do not change a
  product profile or retire a VGUI caller.

## Evaluation boundary

The candidate keeps authored UI state, tree structure, event routing, layout,
focus/navigation and text entry as distinct, composable responsibilities. The
two scenes below share those responsibilities and use presentation transforms
for their different looks. Library choices for layout and text support remain
open; HarfBuzz is the assumed text shaper. No new render effect is implemented
in a frozen legacy render path: new rendering is proved in `render_lab` before
product integration under RFC 0016's binding rules.

## Required test scenes

Both scenes are required for qualification. A static screenshot or an isolated
shader demo does not pass either scene.

### UI-T1: Curved visor HUD

Build a Halo-like, live gameplay HUD whose authored elements follow a subtle
visor curvature. Include shaped text, a status meter, a directional indicator
and a damage response. The HUD remains readable while the camera moves, the
viewport changes aspect ratio and scale, and the indicators update. A shallow
camera-relative projection, controlled parallax, masks and authored motion
must be expressible without changing the tree's layout or gameplay-data owner.
The [Halo 3 HUD design spec](https://www.cand.land/halohud) is a visual and
behavioral reference for subtle curvature and motion, not a source of assets.

Acceptance evidence includes motion captures and image review at the declared
desktop and mobile output extents, plus checks that scaling or projection does
not clip critical information or leave stale text. Display-space text and HUD
compose at output resolution under RFC 0019 when temporal reconstruction is
selected. Any interactive visor element must map pointer coordinates through
its presentation transform before hit testing.

### UI-T2: Portal 2 flipping menu panels

Build the Portal 2 settings-menu transition between two live menu states.
Capture the outgoing and incoming composed screens, divide the affected region
into tiles, and rotate each tile in perspective on an authored, staggered
timeline. The visible face selects the correct screen, and edge shading and
sound cues follow the tile's progress. The existing
[`CBaseModTransitionPanel`](../game/client/portal2/gameui/portal2/transitionpanel.cpp)
is the behavior and image oracle: it records screen images, rotates tile quads
through 180 degrees and holds modal input during the transition. The new
implementation belongs to the core and UI owners, not that frozen render path.

Acceptance evidence compares the complete transition, including its first and
last frames, against captures from the existing menu. Test forward, reverse,
interrupted and rapid repeated navigation; focus handoff, input containment,
resize and surface recreation; and release of captured images only after GPU
completion. The destination menu keeps its authored state while tiles animate.
No stale tile, wrong face or input action may appear during a transition.

## Performance gate

Before optimizing either scene, record per-profile numeric UI ceilings for
CPU update/layout/shaping/recording, GPU composition/effects, input-to-visible
feedback, glyph upload spikes and retained image/atlas memory. Measure the
steady visor during gameplay and every frame of the menu flip, including the
capture, offscreen and perspective passes. Run matched complete gameplay-frame
measurements on the declared desktop and Fold7 profiles with all selected
effects enabled. The UI ceilings diagnose cost; RFC 0016's hard full-frame
budget decides performance acceptance. A miss or missing required evidence
leaves promotion open, and no effect or sample count is cut to obtain a pass.
