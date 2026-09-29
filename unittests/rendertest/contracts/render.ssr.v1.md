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
| S1 | The reference finds each mirror ray's true reflection within 1.5 pixels wherever the camera sees it (the same rectangle over the 5 x 5 pixels around it), and reflects its light within 3 percent + 0.01, except the screen-space ambiguity set, which is computed exactly from the analytic scene and must equal the set of rays that stop early (see "The ambiguity set" below) | reference: pass |
| S2 | The thickness test is live: a ray whose screen path crosses a surface it passes far behind is not stopped by it, and is with an unbounded thickness | reference: pass |
| S3 | A reflection whose true point is off the screen does not hit; hits within `edgeFade` of the screen's edge have confidence below 1, and within 1 percent of it below 0.1 | reference: pass |
| S4 | Roughness at or above the cutoff, the background and every pixel of confidence 0 are the lit input, bitwise; between the fade start and the cutoff the confidence is at most the fade | reference: pass |
| S5 | The GPU pass (hierarchical depth walk) agrees with the reference: the same hit texel, confidence and composited value within tolerance, on the analytic scenes | planned |
| S6 | Seeded defects are caught: the thickness ignored, no edge fade, the wrong mip | planned |
| S7 | "No SSR" (the pass not run, or every confidence 0) is the image-specular frame, bitwise | planned |
| S8 | On the mirror-corridor fixture, on-screen hits are within the fixture's tolerance of the Cycles reference (`lighting_fixtures.py gallery`) | planned |
| S9 | The fallback seam: along a camera walk where rays leave the screen or are occluded, the frame-to-frame step between SSR and the probes is at most the R50 walk gate's 0.047; a control that switches without the confidence fade fails | planned |
| S10 | The Khronos validation layer (synchronization validation) reports no message | planned |

## The ambiguity set (S1), fixed before its first run

Screen space has no thickness: a ray that passes behind a surface by less
than `thickness` stops there (ssr.h step 3). For each floor pixel of an
analytic scene the suite walks the reflected ray from the definition's
origin O (parallel to the true ray from P) to the plane of the rectangle the
true ray hits, in steps of 0.05 pixels on the screen.
Steps inside the texel that holds O are skipped, as the walk skips it. At
each other step it casts the camera ray through the step's screen point and
the 8 points one pixel away (a texel the step crosses may hold what any of
them sees). For each rectangle seen there other than the one the ray truly
hits (the pixel's own included: its flat texels can stop a grazing ray that
leaves it), `behind` is the ray's view distance (clip w) minus that
rectangle's plane's under the step; the rectangle is interior when all 9
points see it, else an edge. With delta the band (half the plane's w change
per pixel, both axes summed, plus the ray's w change per pixel of screen
travel, plus 1e-3 of w):

- **ambiguous** when some interior step has delta <= behind <= thickness -
  delta: the ray must stop early (miss its true reflection by more than 1.5
  pixels);
- **unclassified** when not ambiguous and some interior step has behind
  within delta of 0 or of thickness, or some edge step has
  -delta < behind < thickness + delta: excluded, and counted in the check's
  detail;
- **clear** otherwise: the ray must hit within 1.5 pixels of its true
  reflection.

Amended 2026-09-29 after the rule's first runs, which exposed two false
premises and changed no band: the pixel's own rectangle was excluded, and a
step considered only the rectangle seen at its own point, so a ray passing a
rectangle's edge inside a texel whose centre is on that rectangle was called
clear (14 pixels).

The check is set equality: every ambiguous pixel stops early, and every pixel
that stops early is ambiguous or unclassified. A seeded reference that
applies the thickness in front of the surface instead of behind must fail it.
