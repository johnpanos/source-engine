# K11 receiver image review: 39-view lab gallery (2026-10-01)

Source: [`oracles-rerendered-frozen-lab-20261001`](../quality-results/lighting-gallery/oracles-rerendered-frozen-lab-20261001/index.html), its `summary.json` and `provenance.json`, and a visual review of the lab/reference/error-map rows at their embedded resolution. All 39 Cycles views were rerendered with authored normal maps enabled, while the lab frames were reused from [`analytic-emitter-reflections-full-20261001`](../quality-results/lighting-gallery/analytic-emitter-reflections-full-20261001/index.html) to isolate reference changes. The gallery includes the per-emitter radiance and cone changes, probe captures without camera-only analytic-light meshes, and SSR rejection of those meshes as hits. It passes **16 receiver gates**, with no gate changes from the previous gallery. The earlier [`normals-on-full-20261001`](../quality-results/lighting-gallery/normals-on-full-20261001/index.html) gallery passed 10; the user's original [`emitter-full-fixed-20260930`](../quality-results/lighting-gallery/emitter-full-fixed-20260930/index.html) gallery was reviewed first. The table below uses the current references and receiver scores. This review does not certify K11.

**Active Cycles receiver set after user review:** Portal-pair is removed because its joined-copy Blender scene is not a meaningful oracle for the runtime portal view. It remains in this document as historical diagnostic evidence, not a K11 receiver gate. The default `render` and `gallery` commands now select the ten eligible fixtures; an explicit Portal-pair receiver run is refused. The new [27-view gallery](../quality-results/lighting-gallery/cycles-oracles-no-portal-pair-20261001/index.html) scores **15/27** with the same Cycles references and current lab renderer. The reduction from 16/39 removes the one passing Portal-pair receiver view as well as its 11 failures; it is a scope correction, not an image-quality improvement. Material-sweep remains failed at **0.261 / 1.020** front and **0.067 / 0.501** grazing.

The table gives receiver **mean / p99** error, normalized using each fixture's reference receiver luminance. `Pass` is the gallery's existing gate, not a judgment that the image has no visible difference. The receiver mask excludes visible emitter pixels. Error-map colors are relative to each fixture's tolerance; red across different fixtures need not mean the same absolute error. Observations below describe the images. Explanations marked *candidate* need term isolation or fixture checks before they become a cause claim.

## Obvious failures across the gallery

1. **Portal pair, all states:** the lab keeps a black opening in every state. The closed reference is also black, but the open/glow references change the opening, room light and floor reflection. Even in the closed B-room view, the lab has a bright yellow floor streak and excess warm light on the sphere and ceiling. This is the dominant full-frame error, especially `closed/b-floor` and `glow-only/a-portal`.
2. **Door room, `closed/b-door`:** a hard bright wedge crosses the default lab floor toward the camera; the reference floor is nearly unlit. Term isolation below shows the direct closed-door shadow is close when the core direct path is selected, while the total still carries excess indirect light.
3. **Portal 2 chamber:** floor and wall tile patterns, ceiling lights and doorway details visibly differ. The disagreement follows surface grids and edges across a large area. Restoring normal maps in the Cycles oracle adds fine relief but leaves this dominant grid mismatch; material mapping/content parity remains a candidate.
4. **Projector state:** `projector/room` and `projector/wall` keep a bright room in the lab while the reference has a black surround with only the projected shapes bright. These are different scene-lighting states in the images. The reference's deliberate omission of projector bounce is already noted in [0016 progress](0016-progress.md); the state comparison must be reconciled before attributing all of this to a shader.
5. **Material sweep:** the rough gold and red spheres, reflections and ground shadows diverge in shape and brightness. This remains visible in the later cone-enabled captures, which fix the disk emitter but do not pass either receiver view.

### Worst failures first

Ranking the 23 failed views by `mean / mean tolerance` puts nine Portal-pair views in the first nine places. This is a triage order, not a claim that their normalized errors have the same visual scale across fixtures. The top rows were rechecked side by side in the rerendered-oracle gallery:

| View | Mean / limit | What the images and controls establish |
| --- | ---: | --- |
| Portal pair `glow-only/a-portal` | 7.667 / 0.07 (110x) | Lab is white/pink while the reference room is blue with a cyan panel. The lab image hash is identical to its `closed`, `open` and `open-glow` A-camera frames; the state is absent from the lab input, so this is a confirmed scene-state coverage failure before a color or BRDF comparison. |
| Portal pair `closed/b-floor` | 4.092 / 0.07 (58x) | Both openings are black, but the lab has a bright yellow floor streak, warm spill and a white sphere; the reference has a dimmer sphere and room. This is the worst p99 (96.546) as well. The closed-state light/reflection terms need isolation; state coverage alone will not explain this row. |
| Remaining seven Portal-pair views in the top nine | 0.884–1.619 / 0.07 (13–23x) | The opening, room lighting, sphere and floor reflections disagree. The four distinct reference states still map to one lab image per camera, so these rows cannot yet separate portal transport from lighting-model error. |
| Door room `closed/b-door` | 0.992 / 0.08 (12x) | The default path leaves a hard bright floor wedge from an open-state bake. The core-direct control removes it; direct-only then passes, while total still fails because static surfaces and the moving door receive excess indirect/probe light. This is a confirmed direct-path selection problem plus a remaining state-aware indirect problem. |
| Material sweep `default/front` | 0.261 / 0.05 (5.2x) | Rough gold is too weak in red while glossy gold direct light is close. Moving a probe near Gold7 helps; raising that capture from 16 to 512 samples barely changes it. The remaining rough-metal lobe/material response is unresolved, rather than reference noise. |

The next non-state-heavy group is Portal chamber `vault` (0.433 / 0.12), Portal 2 chamber `spawn` (0.376 / 0.12) and the projector-state room (0.217 / 0.06). The vault error concentrates at window/frame halos, the spawn error follows ceiling lights and tile edges, and the projector image compares different room-lighting states. Their image observations and narrower hypotheses appear in the per-view sections below.

## Analytic emitter reflection check

Cycles gives an analytic light's visible mesh to camera rays, while its glossy rays see the analytic lamp. The old reflection-probe face renderer used a camera and therefore captured both the lamp-lit scene and that visible emitter mesh; the runtime then added analytic direct specular to the probe sample. Hiding only meshes that `map_scene.analytic_emitter_kind` identifies as analytically replaced in the probe capture removes that double count. `material-sweep/front` improves from **0.275 / 1.285** to **0.261 / 1.020** mean / p99, and `grazing` from **0.083 / 0.611** to **0.067 / 0.501**. On gold spheres 0–3, the lab's direct-specular regional RGB was already close to Cycles' glossy-direct contribution; the probe change brings the image contribution much closer. Rougher gold, especially spheres 5–7, still lacks the reference's red indirect response. The receiver gate stays failed for both views.

An initial probe-only full gallery passed 12/39 but regressed `mirror-corridor/low` from **0.076** to **0.209** mean error. SSR was sampling the camera-visible light strips in its `lit` source and replacing the improved probe value with those pixels, although a Cycles glossy ray cannot see those meshes. The surface program now marks only analytically replaced emitter meshes in the normal/roughness target; SSR keeps the probe result when a ray hits one. A diagnostic rejecting *every* emissive surface gave the same mirror result, but the material flag avoids applying that policy to mesh-only emitters. The current gallery passes **16/39**, including all three area-room views, both mirror-corridor views, the Cornell floor, and the sun-colonnade yard. No previously passing view became failing. Some still-failing views change slightly in either direction, so this is not a blanket improvement claim.

The lab SSR reference now checks a marked analytic-emitter wall, and the Vulkan suite passes 25 checks with validation. A seeded trace that ignores the marker fails on 13,564 judged hit decisions and 13,354 unchanged-pixel checks. The full fixture rebuild and gallery use the same new material and probe policy. This correction does not resolve the Portal-pair state mismatch or the remaining rough-metal, shadow and indirect-light differences.

## All receiver views

### Area room

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / grazing | 0.049 / 0.607 | Pass | The large light/reflection balance is closer after excluding camera-only emitters from probes and SSR hits. The lab still draws distinct rectangular floor reflections where Cycles has softer colored pools; error follows those shapes, the strips and block edge. |
| default / overview | 0.041 / 0.734 | Pass | The room layout aligns, but ceiling and side-strip light levels differ; reflected pools around the blocks and floor vary. |
| default / wall | 0.023 / 0.174 | Pass | Surfaces are close. Error is concentrated at the side light strips and their floor reflections. The separate emitter diagnostic is now substantially closer. |

### Cornell floors

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / floor | 0.035 / 0.183 | Pass | Colored bounce and block shading are close, but the near block and floor are differently shaded, with error following the block silhouette and lower face. |
| default / front | 0.027 / 0.212 | Pass | The sphere has a faceted or dimpled lab silhouette/shading absent in the smooth reference; its rim and highlight dominate the local error. The model-mesh mismatch noted in progress is a candidate, not proof of a lighting fault. |

### Door room

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| closed / a-door | 0.121 / 0.388 | Fail | The bright circular patch broadly aligns, but the door opening is lighter and its border/near wall shadows differ. Error outlines the doorway and adjacent wall. |
| closed / b-door | 0.992 / 4.839 | Fail | The default lab path has a large bright, hard-edged floor wedge and warm light around the closed door; the reference has neither. See the direct/indirect isolation below. |
| open / a-door | 0.032 / 0.195 | Pass | Opening and circular light align; remaining error hugs the door frame, threshold and nearby wall. |
| open / b-door | 0.014 / 0.157 | Pass | Images are close; a small difference remains at the bright threshold and the dark far doorway. |

#### Closed-door term isolation

The fixture bakes the **open** room and adds the door as a moving occluder in the `closed` state. The default total gallery does not pass `--core-direct`; its `closed/b-door` wedge and **0.992 / 4.839** mean / p99 are therefore not evidence that the core's direct shadow test is wrong. A diagnostic total capture with `--core-direct` removes the wedge and scores **0.540 / 0.872**; it remains a receiver failure. A direct-diffuse capture with the same core path and baked, probe, IBL, reflection, AO, emission and volumetric terms disabled scores **0.024 / 0.245**, within the separate direct-light tolerance. The other three direct-only door views pass as well. Reproduction: `python3 tools/quality/lighting_fixtures.py gallery --fixture door-room --direct --resolution 2 --out quality-results/lighting-gallery/door-direct-analytic-20261001`.

At corresponding receiver regions in `closed/b-door`, linear luminance averages for Cycles total / direct / residual are floor **0.178 / 0.086 / 0.092**, walls **0.183 / 0.058 / 0.125**, and moving door **0.019 / 0.004 / 0.015**. The lab core-direct total is **0.236 / 0.294 / 0.034** for those regions, whereas its direct-only result is **0.086 / 0.056 / 0.004**. Turning off baked lighting in a diffuse-only diagnostic reduces the lab floor from 0.222 to 0.086 and the walls from 0.282 to 0.056. The baked contribution thus exceeds even the reference's total indirect-plus-specular residual on both static regions. The moving door also gets excess probe and IBL response. These measurements point to state-aware indirect lighting and its composition as the remaining closed-door fault; they do not justify weakening the Cycles oracle or declaring the total view passed. The current diagnostic files are under `quality-results/lighting-gallery/door-core-direct-analytic-20261001/` and `door-direct-analytic-20261001/`.

### Foggy hall

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| clear / nave | 0.072 / 0.322 | Pass | Lab is more evenly filled; the reference has stronger colored bulb rows and brighter localized pools. Error is concentrated on the rows and reflective floor. |
| clear / side | 0.082 / 0.274 | Pass | Colored rows appear as broad, soft lab patches versus distinct bright reference bulbs and halos; pillar shading also differs. |
| fog / nave | 0.083 / 0.735 | Pass | Beam silhouettes roughly match, but lab haze appears smoother and its floor pool differs; error also follows colored rows and bright beam tips. |
| fog / side | 0.080 / 0.332 | Pass | The three volumetric cones are present in both. Their intensity/shape and the colored wall-row response differ, with some noisy reference detail. |

### Material sweep

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / front | 0.261 / 1.020 | Fail | The left glossy-gold spheres are now close, while the right rough-gold spheres are too pale in red and have different image-light lobes. Red and white spheres also differ at highlights and rims; foreground shadow edges and reflected shapes differ. The disk emitter's cone is represented. |
| default / grazing | 0.067 / 0.501 | Fail | Gold-sphere reflections and red-sphere highlights differ along the receding row. The floor's shadow/reflection outlines disagree; most plain wall area is close. |

#### Rough-metal and lobe controls

In `default/front`, the reference object-index mask keeps the sphere comparison separate from the floor and wall. After the probe correction, glossy gold sphere 0 averages **(0.255, 0.167, 0.067)** in the lab versus **(0.247, 0.158, 0.063)** in the Cycles linear RGB reference. Rough gold sphere 7 (roughness 1.0) instead averages **(0.504, 0.356, 0.137)** versus **(0.955, 0.449, 0.142)**: its red channel is now 47% low. Sphere 5 is **(0.577, 0.451, 0.246)** versus **(0.795, 0.485, 0.247)**. The white dielectric row remains closer, for example sphere 5 **(0.692, 0.653, 0.681)** versus **(0.729, 0.689, 0.720)**. These are object-mask averages, not matched individual rays; silhouette differences can affect them.

Cycles' multilayer EXR separates `GlossDir` and `GlossInd`; their color is factored out, so this comparison apportions the metal-only Combined color per pixel between those two passes. That estimate puts the reference gold sphere 0 direct / indirect red near **0.078 / 0.169**, versus the lab's isolated direct / IBL **0.079 / 0.186** after probe correction. Sphere 7 direct red is near **0.268** in the reference and **0.265** in the lab, but indirect is near **0.686** versus lab IBL **0.246**. The direct lobe is already close at both ends; the high-roughness image response remains the material gap.

The `--debug-brdf 3` negative control disables multiple-scattering compensation. On the current map it cuts rough gold sphere 7 to **(0.237, 0.192, 0.102)** and worsens the full receiver from **0.261 / 1.020** to **0.267 / 1.225**, so removing compensation is not a remedy. A `--core-direct` total control is **0.286 / 1.218**. The remaining rough-gold discrepancy needs the IBL prefilter, split-sum tint and Cycles `multi_ggx` response compared at matched angles before changing the BRDF. Diagnostic captures remain in `/tmp/material-sweep-*-front.pfm`; the gate is the unchanged full gallery.

The recorded RPRB has five captures, all at x <= 4.875 m, while Gold7's visible points average x = 8.368 m. Sampling its pixels through the CPU RPRB oracle at roughness 1 gives mean radiance **(0.246, 0.239, 0.256)**. A one-probe diagnostic captured in free space at **(7.2, 2.8, 1.63) m** and packed with the same 512-wide GGX prefilter gives **(0.394, 0.357, 0.360)**; a closer valid capture at **(7.8, 3.2, 2.25) m** gives **(0.403, 0.349, 0.344)**. Rerendering that closer capture at **512 samples per face** instead of 16, with position and prefilter fixed, gives **(0.404, 0.351, 0.345)**. The red change is only **0.0008** (0.2%), so noisy 16-sample probe faces do not explain this deficit. Raising the GGX prefilter from 256 to 1024 samples on the same 512-sample faces instead lowers red to **0.390**; prefilter sample noise is likewise not the missing energy. A 512-sample probe only 0.53 m in front of Gold7's surface reads **(0.362, 0.289, 0.272)**, worse than the farther free-space capture. The near capture's red-pixel pattern still correlates with Cycles' `GlossInd` red pass (r = 0.83), but its mean is much lower. The reference's Gold7 `GlossInd` red mean is **0.695**, versus `GlossDir` **0.291** and Combined **0.954**; the earlier color-apportioned indirect estimate was 0.686. Thus room-probe placement contributes to the red deficit, but capture quality or simply moving one probe closer does not close it. The remaining candidate is the high-roughness directional/multiple-scattering response or local transport missing from the split-sum approximation; it needs a matched uniform-incident-light control before changing the BRDF. These are probe-radiance diagnostics before the lab's material weighting and screen-space terms, not new receiver scores. The fixture's default probe set and the 27-view active gallery remain unchanged. Reproduction captures, RPRB files, receipts, logs and hashes are under [`quality-results/lighting-probe-diagnostics/material-sweep-rough-20261001`](../quality-results/lighting-probe-diagnostics/material-sweep-rough-20261001/summary.json); the fixture RPRB is in `quality-results/lighting-fixtures/maps/material-sweep/lighting/lighting/reflection_probes.rprb`.

### Mirror corridor

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / down | 0.077 / 0.249 | Pass | The mirror-plane composition aligns, but the ceiling light shapes and their reflected copies differ in brightness and extent. The reflected distant ball/floor is also locally different. |
| default / low | 0.063 / 0.198 | Pass | Geometry and reflection placement are close. Light strips in the ceiling and mirror remain too different in brightness/shape. |

### Portal chamber

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / room2 | 0.244 / 1.445 | Fail | Lab is darker and less evenly lit across the concrete wall/floor; an overbright band appears along the right floor-wall seam. Seams and frame edges also light differently. |
| default / vault | 0.433 / 2.062 | Fail | Strong excess light and edge halos appear around the window, upper frame and right vertical frame; the rear wall/door contrast differs. This is visually prominent despite matching broad geometry. |

### Portal pair

The lab keeps a black opening in **every** state. The closed reference has a black opening too; the glow references instead show cyan or yellow portal emission. The receiver mask omits an opening tagged as an emitter, but walls, objects, floor and their reflections still record the differences. The description below records each distinct state/view, rather than assuming one fix explains every surface.

The gallery's saved image hashes isolate a state-coverage failure before any BRDF diagnosis: for each of the three cameras, the lab frame has **one identical SHA-256 across all four states**, while the four Cycles reference hashes are distinct. `lighting_gallery.render_camera` selects the same closed-state BSP for all four because this fixture has no `state_maps` or movers; `render_lab` has no portal-state argument or portal view producer. The open and glow rows therefore compare a closed lab scene to different reference scenes. Their receiver numbers are real errors, but cannot certify portal transport or isolate its shading until the lab represents the state. The gallery applies each row's reference-based exposure to both displayed images, so these identical raw lab frames can *look* brighter or darker across states. Even `closed/a-portal` passes the receiver mask while its black opening contains a gray circular spot absent from the reference, reinforcing the need for a separate portal-image check.

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| closed / a-portal | 0.060 / 0.181 | Pass | Both openings are black, but the lab adds a gray circular spot within it and a brighter patch at the threshold. Column and adjacent wall shading differ. |
| closed / b-floor | 4.092 / 96.546 | Fail | Both openings are black. The lab's yellow floor streak and warm ceiling/sphere light are absent or much dimmer in the reference. Gross receiver mismatch within the same nominal state. |
| closed / b-portal | 0.884 / 10.612 | Fail | Both openings are black; lab has a yellow threshold pool, warm wall/ceiling spill and a yellow rim on the sphere, while the reference room is cooler. |
| glow-only / a-portal | 7.667 / 24.429 | Fail | Lab remains the closed scene: black opening and nearly white/pink room under this row's exposure. Reference room is blue, with a cyan panel and dark ceiling emitter. Broad state and illumination mismatch. |
| glow-only / b-floor | 1.422 / 4.897 | Fail | Lab remains the closed scene with a black opening and dark room under this row's exposure; reference is warm yellow/orange with a yellow panel and darker sphere silhouette. Floor reflection, walls and sphere all differ. |
| glow-only / b-portal | 1.619 / 5.668 | Fail | Same B-room reversal: lab has a black opening and dark room; reference has a yellow panel and glow throughout the room. |
| open-glow / a-portal | 0.333 / 1.130 | Fail | Lab opening stays black; reference is cyan/blue. The reference room is cooler, and threshold and adjacent-wall light differ. |
| open-glow / b-floor | 1.201 / 5.571 | Fail | Lab opening stays black with its yellow floor streak; reference has a yellow portal, warm surfaces and a different bright reflective-floor shape. Sphere shading differs. |
| open-glow / b-portal | 1.294 / 6.047 | Fail | Yellow reference portal lights walls, ceiling, cube and floor; lab black opening leaves these mostly dark under this row's exposure. Sphere and reflection differ. |
| open / a-portal | 0.377 / 0.851 | Fail | Both openings appear black, but the lab retains the circular spot and brighter threshold patch; adjacent wall and floor light differ. |
| open / b-floor | 1.067 / 7.187 | Fail | Both openings appear black. Lab retains a yellow floor streak, while reference has white portal-view streaks and different sphere/room illumination. |
| open / b-portal | 0.937 / 7.591 | Fail | Both openings appear black. Lab retains a yellow threshold pool and warm sphere edge; reference has white portal-view light/shadow streaks. |

### Portal 2 chamber

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / chamber | 0.226 / 1.996 | Fail | Lab floor tiles are faint and differently spaced relative to the dense dark reference grid. Wall tile borders and back-wall surface contrast differ. Error tracks grid lines over most of the frame after normal maps are enabled in Cycles; material mapping/content is a candidate. |
| default / spawn | 0.376 / 1.550 | Fail | Lab ceiling has dark striping/vent shapes where the reference shows three luminous strips. Wall frames, floor grids and doorway interior differ; error follows many straight boundaries after normal maps are enabled. |

### Projector cookie

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / room | 0.090 / 0.746 | Fail | Projected window shape is broadly in place, but lab has brighter fill on the left wall, ceiling and floor. The pedestal, sphere and shadow edges disagree. |
| default / wall | 0.056 / 0.379 | Pass | The wall projection aligns. Differences concentrate on the glass sphere's reflection/refraction and the pedestal rim/side, plus floor-shadow edges. |
| projector / room | 0.217 / 0.920 | Fail | Reference surround is black outside the projected shapes; lab keeps a bright white room. This is a scene-state/reference mismatch in addition to any projector shader error. |
| projector / wall | 0.085 / 0.405 | Fail | Same black-surround versus lit-room difference; the glass sphere and pedestal also have different shading. |

### Sun colonnade

| State / view | Mean / p99 | Gate | Visual receiver difference |
| --- | ---: | --- | --- |
| default / along | 0.074 / 0.334 | Fail | Broad shadow geometry aligns, but the near right exterior floor is much brighter in the lab, and long wall/column shadow bands differ. |
| default / yard | 0.050 / 0.294 | Pass | Wide composition and sky match; subtle shading differences sit on the columns and ground. The distant sphere is more conspicuous in the lab. There is no obvious gross failure at this overview scale. |

## Next checks suggested by the images

These are hypotheses to test, in priority order, rather than accepted fixes: (1) isolate the Portal pair panel/emission, indirect and reflection terms per state; (2) resolve the closed-door **indirect** state response after the direct-shadow result above; (3) compare Portal 2 chamber material identifiers, UV transforms and texture samples at corresponding pixels; (4) compare material-sweep's roughness-indexed probe prefilter and split-sum/multiple-scatter tint with Cycles `multi_ggx`, now that gold direct specular and glossy-end IBL are close; (5) reconcile the projector-state reference definition and capture direct-only versus baked contributions separately. The passing area-room and mirror-corridor views still merit local shape checks against their reference images.
