# Contract: `render.ssr.v1`

Module: `render.pass.ssr` (RFC 0016 layer 6, the "Screen-space reflections"
term of `render.lighting.v1`)
Definition: `public/render/pass/ssr/ssr.h` (the term's inputs, trace, hit,
confidence, filter and composite; the RFC's model table row names the term
and links there)
Reference: `render/lab/ssr_reference.h` (the definition in double precision,
walking every texel), judged on analytic scenes (`render/lab/ssr_scene.h`)
Suite: `render_lab suite ssr` (`render/lab/ssr_suite.cpp`), manifest row
`render.lab.ssr`
Rows: R95 (proven in `render_lab`), R96 (product integration)

| Clause | Obligation | State |
| --- | --- | --- |
| S1 | The reference finds each mirror ray's true reflection within 1.5 pixels wherever the camera sees it, and reflects its light within 3 percent + 0.01; rays stopped within the thickness behind a thinner occluder are the definition's ambiguity, counted and at most 2 percent | reference: pass |
| S2 | The thickness test is live: a ray whose screen path crosses a surface it passes far behind is not stopped by it, and is with an unbounded thickness | reference: pass |
| S3 | A reflection whose true point is off the screen does not hit; hits within `edgeFade` of the screen's edge have confidence below 1, and within 1 percent of it below 0.1 | reference: pass |
| S4 | Roughness at or above the cutoff, the background and every pixel of confidence 0 are the lit input, bitwise; between the fade start and the cutoff the confidence is at most the fade | reference: pass |
| S5 | The GPU pass (hierarchical depth walk) agrees with the reference: the same hit texel, confidence and composited value within tolerance, on the analytic scenes | planned |
| S6 | Seeded defects are caught: the thickness ignored, no edge fade, the wrong mip | planned |
| S7 | "No SSR" (the pass not run, or every confidence 0) is the image-specular frame, bitwise | planned |
| S8 | On the mirror-corridor fixture, on-screen hits are within the fixture's tolerance of the Cycles reference (`lighting_fixtures.py gallery`) | planned |
| S9 | The fallback seam: along a camera walk where rays leave the screen or are occluded, the frame-to-frame step between SSR and the probes is at most the R50 walk gate's 0.047; a control that switches without the confidence fade fails | planned |
| S10 | The Khronos validation layer (synchronization validation) reports no message | planned |
