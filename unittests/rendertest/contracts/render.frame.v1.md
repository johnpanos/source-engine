# Contract: `render.frame.v1`, with `render.composition` and `render.legacy-frontend`

Modules: `render.frame` (port, layer 5), `render.renderer` and
`render.pass.present` (layer 6), `render.legacy-frontend` (layer 6,
legacy-interop), `render.composition` (layer 7)
Suites: `unittests/rendertest/core/renderer/test_renderer.cpp`,
`unittests/rendertest/core/composition/test_composition.cpp`
Rows: R87 (RFC 0016 K3; this is the wiring slice)

| Clause | Obligation |
| --- | --- |
| F1 | Stages follow the legacy frame: `kFrameBegin` first, `kFrameEnd` last; views open anywhere (also nested) and close innermost first; inside a view `kSkybox`..`kPostProcess` never go backwards; outside views only view begins, `kHud` and the end occur. Every violation is counted, none is fatal |
| F2 | A frame runs every feature's passes once, in feature order, on the device, and releases its transients |
| F3 | Begin/end misuse fails without breaking the next frame |
| F4 | Stage hooks see each stage with its view depth |
| F5 | A feature needing a capability the device lacks fails renderer creation, naming both |
| F6 | Capability negotiation (RFC 0016 K10; `render/composition/negotiation.h`, suites `render.composition.capabilities` and `.gl`): a feature whose requirements the device lacks is replaced by its `FeatureRequirements::fallback` only when the product profile declares `feature=fallback` (`RenderCoreConfig::fallbacks`, `-render-fallbacks`); every substitution is named in `RenderCoreResult::substitutions`; an undeclared fallback fails `RENDER_CORE_UNDECLARED_FALLBACK` naming it, and a feature without one fails `RENDER_CORE_MISSING_CAPABILITY`. A profile masks capabilities through the adapter (`RenderCoreConfig::maskedCapabilities`); an adapter that cannot mask fails composition. Seeded bad compositions (an undeclared fallback taken, a silent substitution) fail the suite's oracle |
| P1 | `RenderCore_Create` composes device, frontend and renderer, or fails with a structured result naming the unknown device or feature |
| P2 | The frontend's legacy provider keeps the wrapped backend's id and module and forwards creation through `createFor` (no global) |
| P3 | `RenderStageMarkers001` forwards the client's marks; the engine owns frame begin and end |
| P4 | A composed frame runs the legacy-stream pass, then present |
| P6 | Core-pass slots (`public/render/legacy/core_passes.h`, RFC 0016 K5): `RenderCoreConfig::corePasses` (`-render-core-passes`) selects the frontend's probe (`empty`, `seeded-clear`) and fails `RENDER_CORE_INVALID_CONFIG` naming any other value; with a probe the frontend queues one slot on the backend's `ICorePassSlots` at each view's opaque stage, tagged with the stage and the view depth (`CorePassTag`), in frame order through the render call queue (`render.legacy-capabilities`); without one it queues none. The backend records the recorder's pass for each slot as a section of its scene record and runs it at the slot outside its own render pass; an `empty` probe leaves the frame's pixels unchanged |

The frontend's ABI face, `public/render/legacy/stage_markers.h`, is a preserved
C++11 package (CAP010); its enum values equal `render::frame::Stage`.
