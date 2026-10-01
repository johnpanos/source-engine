# Contract: `render.skinning.v1`

Module: `render.pass.skinning` (RFC 0016 layer 6 feature)
Header: `public/render/pass/skinning/skinning.h`; kernel `render/pass/skinning/skin.comp`
Suites: `unittests/rendertest/core/skinning/test_skinning_reference.cpp`
(`render.skinning.reference`, headless), `test_skinning_vulkan.cpp`
(`render.skinning`, `linux-native-vulkan-gpu`), and `test_skinning_corpus.cpp`
(`render.skinning.corpus`: real models against studiorender's software
skinning, captured by `studiorender/skin_capture.cpp` and compacted by
`tools/render/skin_corpus.py`)
Rows: R89 (RFC 0016 K6)

The oracle is today's emit skinning in the native Vulkan backend
(`SkinPosition`, `WorldNormal` and the flex stream in
`materialsystem/shaderapivulkan/shaderapivulkan.cpp`), which follows
`common_vs_fxc.h`. `SkinReference` is its portable form.

| Clause | Obligation |
| --- | --- |
| S1 | Flex runs first: each delta adds weight × delta to the position, weight × normal delta to the normal and to the tangent S, and weight × wrinkle to the wrinkle. Its current and delayed stereo weights are each mix(w[0], w[1], side); the final weight is mix(current, delayed, delay). A zero delay does not read the delayed entry and preserves the original zero-reserved-byte capture layout |
| S2 | Three bones with weights w0, w1 and 1 − w0 − w1; indices in the low three bytes; an index past the palette reads bone 0; normal and tangent go through the rotation part, unnormalized; the tangent keeps its w sign |
| S3 | The oracle agrees with an independent double-precision matrix-blend reference (studiorender's `ComputeSkinMatrix` form) within 1e-3 on seeded meshes |
| S4 | The GPU kernel, dispatched as a graph compute pass, equals the oracle within 1e-3 in position (units), normal, tangent and wrinkle on synthetic meshes; on the real-content corpus it equals studiorender's software skinning with normal and tangent within 1e-3 and position within max(1e-3 units, 4 ulp of the coordinate) (tolerance version 2, RFC 0016 K6) |
| S5 | A seeded bone-index error and a seeded flex-weight error in the kernel each exceed the tolerance |
| S6 | A device without `Capability::kCompute` fails kernel creation with `kNoCompute`; a dispatch the kernel cannot record counts in `RecordFailures`, which the pass's owner checks |
| S7 | Bind groups of recorded dispatches are released behind the token passed to `Collect`; the kernel is destroyed only after every dispatch has been collected |

Open for K6: flexed vertices in the real-content corpus (the first capture
reached no flexed mesh in the software path), the product path (studiorender's hardware-skinned
draws reading the skinned buffer, CPU-skinned draw census zero), the
skinning and model-light pixel families, and the per-vertex cost record.
