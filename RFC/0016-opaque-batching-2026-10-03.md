# Opaque world/model batching: game evidence (2026-10-03)

The game now gathers a compatible opaque world ticket and following static/posed
model tickets into one view preparation and depth sequence. World and model color
draws remain separate and keep their prior order, including equal-depth ties.
This reduces whole-frame time on the Intro4 arrival view. It does not meet the
[hard render budget](0016-render-core.md#hard-render-budgets-user-decision-2026-10-01)
or close R96's complete-content acceptance. The [in-game pipeline rule](0016-render-core.md#in-game-pipeline-performance-user-decision-2026-10-03)
owns the performance objective.

## Ownership and boundaries

`render.pass.world` owns batch eligibility against immutable queued snapshots,
merged preparation and execution. `render.material` owns eligible material
semantics. `render.composition` and `render.legacy` forward compatible prefixes;
the native bridge supplies target/state-compatible candidates and preserves one
section per original slot. Consumed followers have empty sections, keeping native
replay indexing intact. The bridge reuses the exact existing legacy suppression
predicate, so it can cross commands replay already skips, but never an executing
draw, clear, copy or query. The ordered custom effects remain boundaries.

The first ticket may include world surfaces; a later world ticket stops the batch.
Host frame, camera, viewport, lighting snapshots and debug terms must agree.
Stencil, depth overrides, special depth ranges, blended materials, transmission,
water, refraction and dynamic effects keep their original slots. Different shader
times are allowed only for the eligible time-independent programs. Skin and
surface selection participate in eligibility. The per-ticket color order precedes
material sorting, so a later equal-depth model still wins.

Queued tickets retain their original snapshots for captures. Validation rejects an
invalid prefix before consuming any ticket. Existing submitted GPU tokens govern
resource reuse and destruction. No image effect, resolution, sample count or
material fallback was removed to obtain these numbers.

Frozen-path: core plumbing in `materialsystem/shaderapivulkan` forwards opaque
prefixes into the render core, preserves replay sections, and adds diagnostic
selection/counters; it adds no native shading policy.

## Same-binary game measurements

Private worktree: `/home/john/.codex/worktrees/opaque-view-batching/source-engine`,
based on `d117ea50b`. Evidence directory: `quality-results/opaque-batching/` there.
The private runtime uses independent copies of its libraries and a frozen native
`sp_a1_intro4_probe64` export; installed retail content remains read-only.
Each run records all 29 library hashes before/after, revision and dirty digest,
settings receipt, console, frames, device identity and sampled power/temperature.
All four runs used identical library bytes. GPU: AMD Radeon 8060S, RADV
STRIX_HALO. Display: 1920x1080 compositor, High, 4x MSAA. The ordinary collector
keeps the hard floor checks enabled and uses `--no-stop` to finish the short route.
All five map/camera checks and the quality receipt pass in each run.

Off/on/on/off results (median milliseconds; A1, B1, B2, A2 execution order):

| Phase/metric | A1 off | B1 on | B2 on | A2 off |
| --- | ---: | ---: | ---: | ---: |
| Arrival frame interval | 34.623 | 30.199 | 29.549 | 36.380 |
| Arrival GPU render | 34.381 | 27.304 | 27.994 | 36.085 |
| Arrival CPU | 26.936 | 24.258 | 23.820 | 26.284 |
| Reverse frame interval | 20.726 | 20.970 | 21.664 | 22.236 |
| Reverse GPU render | 20.531 | 20.746 | 21.394 | 22.006 |
| Return frame interval | 34.835 | 29.945 | 30.949 | 36.880 |
| Return GPU render | 34.562 | 27.354 | 28.145 | 36.574 |

Averaging the two run medians gives about 16% lower arrival frame interval and
22% lower arrival GPU time. Reverse-view differences overlap the control drift;
no reverse-view gain is claimed. Arrival frame p99 is 35.769/38.142 ms off and
32.697/31.523 ms on. Spikes remain in the complete route, including a 111 ms
batched frame. Every run **fails** the 120 FPS acceptance floor. These are bounded
optimization observations, not a new budget, a statistical confidence claim or
proof of every map/view. Per-run JSON/TSV retain all phases and tails.

Separate `preview-off` and `preview-on` runs capture arrival/reverse/return at
1920x1080 through screenshot replay. Visual inspection finds no lost geometry or
changed coverage. Mean absolute RGB differences are respectively at most 0.030,
0.0015 and 0.020 on a 0–255 scale; these live captures are not byte-identical.
Their added capture work excludes them from the performance comparison.

## Correctness and reproduction

- `render.world.null`: 97 checks, including host/view/material boundaries,
  transmission with opaque blend, invalid-prefix rollback, capture replay and
  manual completion lifetime.
- `render.composition`: 58 checks.
- Native Vulkan `posed-model --validate`: 77 checks. Exact-pixel comparisons cover
  combined models, world plus models, cutout world coverage and equal-depth models
  with reversed material IDs. `view-state --validate`: 16 checks.
- The equal-depth negative control removes cohort order from the sort tuple;
  the oracle rejects it with two named equal-depth failures; the restored
  implementation passes all 77 checks.
- Gameplay collector/scenario fixtures: 62 checks, including rejection of retail
  Intro4 masquerading as the native fixture. VScript returns the canonical authored
  map name; the separate bracketed server `status` check verifies `_probe64`.
- Architecture, inventory, baseline and changed-line style checks pass.

Commands from this worktree (build configurations are isolated by Waf lock):

```sh
WAFLOCK=.lock-waf-opaque-game ./waf build --targets=launcher,shaderapivulkan -j8
WAFLOCK=.lock-waf-opaque-lab ./waf build --targets=render_lab -j8
python3 tools/render/lab.py suite posed-model --tree build-opaque-lab --validate
python3 tools/render/lab.py suite view-state --tree build-opaque-lab --validate
python3 tools/quality/conformance.py check --suite render.world.null \
  --suite render.composition --build-dir build-opaque-checks \
  --out quality-results/opaque-batching/contract-final.json
python3 tools/quality/frame_floor.py --no-stop --opaque-batching on \
  --workload quality/workloads/portal2-intro4-perf-v1/workload.json \
  --runtime quality-results/opaque-batching/runtime --skip-stage \
  --build build-opaque-game --out quality-results/opaque-batching/NEW
```

Use `off` for the matched control and fresh output directories. The local `run.py`
evidence helper adds library hashes and power samples around the installed
collector; `*-identity.json` retains each exact command. Ordinary runs omit pass
timers, so the detailed analyzer warns that pass attribution is unavailable;
whole-frame CPU/GPU and interval data remain present. Use a separate `--profile`
run for pass diagnosis. See [the collector guide](../tools/quality/render_profile.md#quick-opaque-worldmodel-comparison).

Other GPUs, mobile/native Apple profiles and full gameplay/content routes remain
unverified by this slice. The next optimization should be selected from complete
in-game frame costs with the same full-image requirements.


## Landing validation

Rebased onto `subsystem-refactor` at `493481177`, retaining its intervening panel,
material-specialization and documentation work. The combined launcher/native
adapter build passes. Native Vulkan posed-model (77) and view-state (16), world
null (97), composition (58), and changed-line style/architecture checks pass again
on this combined source. The original matched timing table above remains tied to
its recorded pre-rebase binaries; it is not relabeled as a measurement of the
intervening changes. Integration logs use the `integration-` prefix in the same
evidence directory. Architecture checker fixtures pass 166 tests and style
checker fixtures pass 38 tests.

The rebuilt launcher/native adapter also complete `integration-on --preview`:
all five map/camera checks pass, the quality receipt has no failures, and three
screenshots exercise replay with batching active. Two CPU-duration records are
invalid in that capture run, so its timing analyzer rejects them; it contributes
no performance evidence. The matched off/on/on/off measurements above are the
performance evidence for this slice.
