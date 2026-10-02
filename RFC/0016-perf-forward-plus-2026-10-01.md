# Forward+ profiling slice, 2026-10-01

This implements the user's four targets from the
[original profiling report](../quality-results/perf1001/summary.md), with the
subsequent policy: "When rendercore is enabled, only rendercore shaders for
now", using Forward+. RFC 0016 owns that policy in
[Core-only product shading](0016-render-core.md#core-only-product-shading-user-decision-2026-10-01).
The selected native Vulkan product uses `r_core_world 1`; compatibility mode
remains available at `r_core_world 0`.

## Changes and ownership

1. `render.pass.lights::AssignAreaLights` assigns rectangle/reach support to
   the existing view's froxels. Two 32-bit masks cover all 64 area-light slots;
   their offset is appended to the existing cluster index buffer, without a
   new shader binding. The shader evaluates set bits in original light order.
   Assignment is conservative at tile boundaries and clamped depth slices.
   Viewport origins are subtracted during lookup, including offset subviews.
   More than 64 inputs are rejected without changing the output. A zero mask
   offset selects the same modern shader's full-loop oracle for standalone
   unassigned views; it does not select a compatibility shader.
2. `render.composition::CoreWorld` shares one immutable lighting input/result
   across same-view world, static-prop and posed-model slots. The bounded
   main-thread cache keys host frame, light publication revision, matrices,
   viewport and shadow settings. `call_once` publishes the result on recording;
   each queued cohort retains ownership and its separate caster/draw work.
   Map/world replacement clears the cache. Reuse never crosses a frame or a
   light publication.
3. `render/shaders/common/ltc.glsl` streams the final horizon clip into its
   edge integral, removing the duplicate dynamically indexed polygon array.
   The surface shader constructs rectangle corners after shadow evaluation,
   shortening their live ranges. Both horizon clips and the existing LTC
   magnitude, transpose and one-sided controls remain exercised.
4. The frame's core-only marker rejects legacy shader draws before native
   vertex conversion. Unsupported materials remain in the rejection census.
   For world-stage fixtures, queued lightmap work captures that the stage owns
   runtime light and omits CPU point/area/projected light and dynamic occlusion
   integration. This removes the requested area reintegration cost rather
   than adding another revision cache. Mode transitions rebuild lightmaps
   once and clear retained contribution bookkeeping. Retail BSP core surfaces
   keep CPU dynamic lightmaps until their per-pixel consumer migrates.
5. Core CPU and compute skinning skip zero-weight influences. Rigid studio
   vertices avoid their two unused transforms. This is a bounded improvement;
   it does not claim a new GPU skinning integration for product posed models.

Frozen-path: core plumbing R96/R91 — engine lightmap policy capture and native
queue rejection implement the user's explicit core-only request. Compatibility
shading and conversion remain available when that mode is disabled. The old
duplicate LTC clip implementation is deleted; no second light owner/cache or
new frozen-backend shading implementation was introduced.

## Measurements

The native captures used Radeon 8060S/RADV, the actual 1024×768 SDL offscreen
drawable, queued materials and 4× MSAA. AO, shadows, depth prepass and runtime
direct light were enabled in the retained scenarios. The host was shared;
these are descriptive measurements, not a controlled same-image speedup gate.

| Fixture | Original render-thread CPU | Core-only CPU | Original GPU render | Core-only GPU render |
| --- | ---: | ---: | ---: | ---: |
| intro4 reverse | 13.2–13.4 ms | 4.9–5.4 ms | 20.1–20.9 ms | 19.8–19.9 ms |
| laser chamber | 10.8 ms | 5.219 ms | 7.15 ms | 5.271 ms |

The intro captures' frame medians are 20.575 and 20.871 ms; laser is 5.881 ms.
Intro remains GPU bound. CPU improvement includes rejection of unsupported
legacy cohorts, so the table does not establish whole-frame appearance parity
or attribute all savings to assignment reuse.

RADV's retained surface variants use **192 VGPRs and eight subgroups/SIMD**,
with 13,629–14,284 instructions, versus the original 252 VGPRs, six
subgroups/SIMD and roughly 16,800 instructions. This resolves part of the
register pressure; it does not reach the area-disabled variants' 120–144 VGPRs.
See [final compiler output](../quality-results/perf1001-opt/stats/stdout.log).

The product reports 2,539 lit cohort views / 634 lighting builds in intro and
3,179 / 624 in laser, approximately four and five cohorts per build. These
counters measure assignment/shadow-plan reuse, not reuse of cohort draws.
All marked steady core-only native samples have zero legacy stream/program
draws, converted vertex/index bytes and `emit_convert` cost. A final 109-frame
steady interval reports zero CPU area luxels, zero held area contributions and
zero lightmap builds, while serial and queued compatibility intervals resume
legacy draw/conversion work.

## Correctness and checks

The retained relevant render suites pass **407 checks**:

| Oracle | Checks | Evidence |
| --- | ---: | --- |
| clustered CPU assignment and area masks | 249 | [receipt](../quality-results/perf1001-opt/clusters.json) |
| clustered seeded defects | 10 | same receipt |
| composition and core-only frame marker/transitions | 24 | [receipt](../quality-results/perf1001-opt/composition-final.json) |
| null world consumer | 33 | [receipt](../quality-results/perf1001-opt/world-final.json) |
| CPU / GPU skinning | 19 / 8 | [receipt](../quality-results/perf1001-opt/skinning.json) |
| captured studio skinning corpus | 8 | [retained-input receipt](../quality-results/perf1001-opt/skinning-retained.json) |
| native lab area-light images, including offset/resized views | 52 | [log](../quality-results/perf1001-opt/offset-lab.log) |
| seeded LTC defects | 4 | [log](../quality-results/perf1001-opt/ltc-negative.log) |

The area-mask oracle checks 459,144 reaching receiver/light samples with zero
misses and 9,269,016 culled samples, including rotated rectangles, translated
cameras, high mask bits and atomic capacity refusal. Lab diffuse images match
the full-loop shader bit for bit across ten cases; specular cases retain their
analytic oracle and negative controls. This is area-light correctness coverage,
not a Cycles gallery or product whole-image acceptance claim.

The first skinning receipt skipped its unset corpus input; that skip certifies
nothing. The subsequent retained corpus run passes with eight checks and no
skips, covering 178 meshes and 305,078 vertices. That input contains no flexed
vertices and does not establish captured flex coverage. Existing
composition/world manifest source omissions were repaired by
adding their already-required scene-color/refract implementations, and the
composition oracle now recognizes the already-implemented Refract family.
Initial failed receipts are retained and superseded, not counted as passes.

The native launcher, engine and shaderapivulkan build passes
([final build](../quality-results/perf1001-opt/final3-build.log)). Lab builds
and native runs pass. Archlint's 162 fixtures, stylelint's 38 fixtures,
legacy-freeze's 18 checks and inventory verification pass. Changed-file style
checks pass for 31 changed source files against the slice's starting
revision, including the initial changes committed during the session and
concurrent work in the shared checkout
([log](../quality-results/perf1001-opt/style-slice.log)). `git diff --check` passes.

Full archlint and baseline verification remain failed on two unrelated
`ARCH105` CreateInterfaceFn expansions in `game/shared/fstop/blob_networkbypass`
([log](../quality-results/perf1001-opt/arch-final.log)). Their baseline was not
updated. Branch-wide style checking against `origin/master` encounters existing
style debt and terminates on `common/matchmaking/mm_helpers.h`'s invalid UTF-8
without an unchanged Git baseline
([log](../quality-results/perf1001-opt/style-branch.log)). These failures do not
certify a clean full-tree gate. Sanitizer, mobile and whole-product parity lanes
were not run for this slice.

## Reproduction and evidence index

Run from the repository root, preserving the existing configured output trees:

```sh
WAFLOCK=.lock-waf-p2 ./waf build --targets=launcher,engine,shaderapivulkan -j8
WAFLOCK=.lock-waf-rc-lab-main ./waf build --targets=render_lab -j8
LD_LIBRARY_PATH=build-rc-lab/tier0 build-rc-lab/render/lab/render_lab suite area-lights --validate
LD_LIBRARY_PATH=build-rc-lab/tier0 build-rc-lab/render/lab/render_lab suite area-lights --sensitivity
python3 tools/quality/conformance.py check --suite render.lights.clusters.gpu --config release --out quality-results/perf1001-opt/clusters-gpu-current.json
RENDER_SKIN_CORPUS=quality-results/perf1001-opt/k0.skcorpus python3 tools/quality/conformance.py check --suite render.skinning.corpus --config release --out quality-results/perf1001-opt/skinning-retained.json
python3 tools/quality/frame_pacing.py --runtime run/runtime-p2 --build build-p2 --scenario quality-results/perf1001-opt/intro-core.json --out quality-results/perf1001-opt/repro --passes 1 --mat-queue-mode 2 --width 1024 --height 768 --timeout 240
python3 quality-results/perf1001-opt/verify.py
```

The analogous laser and transition scenarios are `laser-core.json` and
`transitions.json`. `RADV_DEBUG=shaderstats` supplies compiler statistics.
Product composition links `render_composition` into **launcher**; rebuilding
only engine/backend leaves stale core code. The initial `intro-spatial`,
`intro-ltc` and `intro-core` directories did that and are excluded as
optimization evidence. `intro-core-linked` establishes the corrected build;
`intro-final`, `laser-final`, `stats` and `t-final` carry the final native
evidence. `transitions-final` exceeded the legacy command-line limit before
launch; the shorter `t-final` retry supplies coverage.

Each conformance/native receipt records its own source revision, dirty state,
toolchain, inputs and invocation. Early runs used `6baf61348`; later runs use
`ffbf34022` plus the working changes. The last build adds the retail-BSP policy
distinction; it does not change the captured WMSH fixtures' behavior. The
[audit script](../quality-results/perf1001-opt/verify.py) checks retained receipt
counts, mask/compiler results and zero legacy work. For transition phases it
checks the first 80 frames after each marker, before the next transition wait;
the complete steady-core interval is checked. Its
[result](../quality-results/perf1001-opt/verification.json) records the audited
working diff digest and retained corpus SHA-256.

Native evidence receipts:
[intro](../quality-results/perf1001-opt/intro-final/evidence.json),
[laser](../quality-results/perf1001-opt/laser-final/evidence.json),
[compiler capture](../quality-results/perf1001-opt/stats/evidence.json),
[serial/queued transitions](../quality-results/perf1001-opt/t-final/evidence.json).
Raw frame samples and runtime console/census logs are beside those receipts.

R95, R96 and R91 remain open. Rejected HUD/UI, particles, sky, glass and model
cohorts still require migration and visual comparison. Core-only rejection is
the user's requested interim shader policy, not completion of those gates.
