# Contract: `render.output.v1`

Module: `render.pass.output` (RFC 0016 layer 6, the "Output" frame term)
Definition: [RFC 0016 "Output"](../../../RFC/0016-render-core.md#output-renderoutputv1-amended-2026-09-28)
(this file lists the obligations the suite checks; it does not restate the
model)
Header: `public/render/pass/output/output.h`
Sources: `render/pass/output/output.cpp`, the shaders `output.vert` and
`output.frag`, and the one copy of each curve:
`render/shaders/common/tone_map.glsl` and
`render/shaders/common/color_encoding.glsl` (SPIR-V the build generates with
the pinned glslc into `spv/output_spv.h`)
Suite: `unittests/rendertest/core/pass/output/test_output_vulkan.cpp` with the
reference in `output_oracle.h` and the seeded programs in
`spv/output_defects_spv.h`
Rows: R95 (proven in `render_lab`), R96 (product integration)

| Clause | Obligation |
| --- | --- |
| O1 | The legacy point: on an 8-bit target with scene peak 1 and headroom 1 the pass equals the clip to [0, 1] of `exposure * x` followed by the encoding, byte for byte |
| O2 | The encodings: an 8-bit UNORM target gets the sRGB curve (IEC 61966-2-1) within one level; an sRGB-view target gets linear values its attachment encodes, within one level of the shader's curve; a half-float target gets linear values (extended linear sRGB), negatives clipped to 0 |
| O3 | Headroom at least the scene peak: each channel clipped to the peak and nothing else, within one half-float step |
| O4 | Headroom below the peak: the BT.2390 EETF of max(R, G, B) from [0, peak] onto [0, headroom] (PQ domain, reference white 203 cd/m^2), one scale for the three channels, within 0.3 percent; the peak lands on the headroom; the map is monotonic; values below the knee pass unchanged |
| O5 | A debug view (`toneMap` false, RFC 0014 "Post-processing is bypassed") gets the encoding alone: no exposure, no tone map |
| O6 | Exposure that is not finite or negative, a scene peak outside (0, 10000/203], a headroom below 1 or not finite, a headroom other than 1 on an 8-bit target, a target of another format and scene or target extents other than the pass's are refused before any pass is added; a target format without an encoding fails `Create` |
| O7 | The Khronos validation layer (synchronization validation) reports no message |
