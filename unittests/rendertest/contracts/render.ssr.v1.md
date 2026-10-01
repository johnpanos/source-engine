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
| S5 | The GPU pass (hierarchical depth walk) agrees with the reference on the analytic scenes (see "GPU against the reference" below) | pass |
| S6 | Seeded defects are caught: the thickness ignored, no edge fade, the wrong mip, and an analytic camera-only emitter treated as an ordinary hit | pass |
| S7 | "No SSR" (the pass not run, or every confidence 0) is the image-specular frame, bitwise | planned |
| S8 | On the mirror-corridor fixture, on-screen hits are within the fixture's tolerance of the Cycles reference (`lighting_fixtures.py gallery`) | planned |
| S9 | The fallback seam: along a camera walk where rays leave the screen or are occluded, the frame-to-frame step between SSR and the probes is at most the R50 walk gate's 0.047; a control that switches without the confidence fade fails | planned (gate on mirror-corridor; thin-bar stress recorded) |
| S10 | The Khronos validation layer (synchronization validation) reports no message | pass (analytic scenes) |
| S11 | A hit on a camera-only analytic emitter (`normalRoughness.w = 2`) keeps the lit/probe result bitwise; the reference and Vulkan trace agree, and a seed that ignores the marker changes those pixels | pass (2026-10-01: 25 validated checks; ignored-marker seed fails on 13,564 hit decisions and 13,354 unchanged pixels) |

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

## GPU against the reference (S5, S6), fixed before its first run

The suite uploads a scene's inputs (depth R32F, normal and roughness
RGBA32F, the rest RGBA16F) and gives the reference the same values (the
half-float ones rounded to half). A pixel is judged when the reference's
outcome is stable: the same hit texel, or no hit, when its reflected ray is
tilted by 1e-4 along each world axis both ways (six runs). On every judged
pixel the GPU agrees: hit or no hit, the same hit texel, confidence and mip
within 1e-3, and the output within 2e-3 of its magnitude + 1e-3 (the output
is half float). On every pixel the reference leaves unchanged (not traced,
no hit, confidence 0) the GPU's output is the lit input, bitwise, whether
judged or not when the GPU also leaves it unchanged; a judged pixel the GPU
changes there fails. Unjudged pixels are counted in the detail. The seeded
traces (the thickness ignored, no edge fade, the wrong mip) must each fail a
GPU check.

## The fallback seam (S9), fixed before its first measurement

The measure follows the R50 walk gate (RFC 0007 progress, "Walk gate"),
whose 0.047 is the largest step of the blend share between neighbouring
floor pixels within a frame over a walk. At each station of a camera walk
the SSR share is the pass's confidence c per pixel (the diagnostics trace).
The step is the largest |c(p) - c(q)| over horizontally or vertically
adjacent traced floor pixels p, q, except pairs whose two hits differ in
view depth (clip w) by more than the thickness: an edge inside the
reflected image, which the reference image has too, not a fallback. A hit
beside a miss, and hits on one surface, count.

- Gate: on the mirror-corridor walk, the largest step over every station is
  at most 0.047 (amended with the decision below; the walk's stations are
  fixed with its first measurement's rule).
- Stress walk, recorded and not gating: the analytic mirror scene at 512 x
  384 (the fixtures' film), 16 stations: the camera 48 units up, advancing
  from x = 0 to 150 while its target rises from z = 0 to 60, so reflections
  cross the top edge of the screen and pass behind the bars.
- Control: a seeded hard-switch trace (c the roughness fade alone: no edge
  and no thickness fade) must reach at least 0.6, the R50 seam minimum.

### First result, and the decision it led to (post-data, 2026-09-29)

The rule above was fixed before its first measurement, but the owner's
instruction to wait for its confirmation arrived while that measurement was
running, so the decision below is post-data. The first result (the fades of
the first definition: the edge fade over 10 percent of the screen at the
hit, the thickness fade over [0.5 T, T]) on the thin-bar walk: largest step
1.0, at station 0 between pixels (189,183), which stops at a bar texel it
crosses (c 1), and (190,183), which enters the bar's texels 214.9 units
behind and ends at the far plane (c 0); the reference's walk gives both the
same c, so this is the definition's seam, not the pass's. Of 24,280
adjacent pairs above 0.047 over 16 stations: 552 a hit beside a miss that
left the screen, 518 behind an occluder (the true point hidden), 468
passing behind a visible surface by the thickness or more, 3,740 both hits
by the edge fade, 18,752 both hits by the thickness fade, 250 other.

The render-core owner's decision (source-engine-43), relayed 2026-09-29:
1. The metric does not change.
2. The fades become continuous in the ray's own parameters (ssr.h step 4):
   the thickness fade a smooth ramp over behind in [0, T], and the edge fade
   over a screen distance scaled by the hit's footprint.
3. The gating walk is the mirror-corridor fixture (RFC 0016 K11,
   "Screen-space reflections"); the thin-bar walk stays as a stress case,
   its number recorded, not gating.
4. Hit-beside-miss pairs behind an occluder are not declared away: they are
   measured on mirror-corridor; within 0.047 there they pass, else the check
   fails with its number recorded and the next step is a definition fix
   (for example a miss that passed behind an occluder keeping a confidence
   that falls off with its behind distance). Only the user can make them a
   declared limitation.
5. The hard-switch control stays: at least 0.6 on counted pairs.
6. The final word is the Cycles comparison on mirror-corridor.

### Under the decision (2026-09-29)

With the fades of ssr.h step 4 (the footprint-scaled edge fade and the
thickness ramp over [0, T]), the reference and the GPU pass agree on the
analytic scenes as before (S1-S6 and S10 pass, 21 checks with the seam
control, g++ and clang++, and the seeded traces each fail a check). The
thin-bar stress walk, recorded and not gating: largest step 1.0, the same
pair (189,183) and (190,183); the hard-switch control 1.0 (at least 0.6:
pass). Of 32,552 pairs above 0.047: 522 a hit beside a miss that left the
screen, 494 behind an occluder, 444 passing behind a visible surface by the
thickness or more, 0 by the edge fade (3,740 before), 30,854 by the thickness
fade (18,752 before). 30,852 of those have both hits on one rectangle, the
larger `behind` of the two at median 4.2 and 90th percentile 6.8 units: the
depth buffer holds one depth per texel, so a ray meeting a surface at a
grazing angle enters a texel already behind it by up to that texel's depth
span, and adjacent rays enter different texels. The ramp over [0, T] now
reads that staircase as confidence; the first definition's [0.5 T, T] hid it
below 4 units. This is measured, not decided: whether `behind` should be
measured against the hit texel's plane (its depth and normal) rather than its
constant depth is the render-core owner's decision, to be judged on
mirror-corridor.
