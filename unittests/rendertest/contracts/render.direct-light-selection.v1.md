# render.direct-light-selection.v1

`public/render/direct_light_selection.h` owns which unbaked lights a renderer
with a fixed direct-light budget takes from the frame's light set
(`render.light-set.v1`). The native Vulkan world path keeps at most
`CVulkanContext::kMaxDirectLights` point and spot lights per frame.

Obligations:

- **Candidates.** Only unbaked point and spot lights of nonzero luminance
  are taken. Baked, directional and dark lights never are.
- **Strongest first.** A light's importance is its luminance, times its own
  falloff at half its reach (legacy dlight or inverse-square), times its
  view coverage `reach² / (reach² + d²)`. Here `d` is the distance from the
  snapshot's view origin, the last rendered main view, which the engine
  stamps on each snapshot. Without a view the coverage is 1. The budget
  keeps the most important lights, ordered strongest first, whatever their
  table order.
- **Determinism.** Equal importance goes to the lower ID.
- **Budget.** A budget above the candidates takes every candidate.

The sensitivity builds substitute the original consumer (the first
candidates in table order), a ranking that ignores the view, and ties to
the higher ID. The oracle must reject each.
