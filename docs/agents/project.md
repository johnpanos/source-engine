# Project plan

Part of [AGENTS.md](../../AGENTS.md). Owned by the project manager session
(source-engine-17 since 2026-10-10), which reviews the tree every 30 minutes,
assigns work and keeps this page current. Sessions work only on what is
assigned to them here or by the user.

## The deliverable

A Source 2 / id Tech 6 class engine: Source content and gameplay running on
a job-based render core at Source 2 image quality, at a hard frame floor, on
Linux, Android and Apple. It is done when every line below holds, measured:

| # | Exit criterion | Measure | Now (date) |
| --- | --- | --- | --- |
| D1 | Every frame ≤ 8.33 ms (120 FPS floor) at 1920×1080 High, 4x MSAA, Radeon 8060S | `frame_floor.py`, row `linux-desktop-high-120` | p50 ≈ 15.3 ms, p99 ≈ 36.9 ms on intro4 (2026-10-06): **2× / 4.5× off** |
| D2 | Same floor through the resolution sweep (720p–4K) on the 8060S and RTX 3070 | RFC 0016 sweep | 3070 4K p50 20.0 ms (2026-10-06) |
| D3 | Source 2 lighting parity: game frames match `render_lab` frames of the same scenes, FSR on and off | game/lab captures, visual review | partial (R95/R96) |
| D4 | The game draws from `render.scene` | CAP012 `render-scene-bypass` = 0 | 345 (2026-10-10) |
| D5 | Legacy stream retired; three device adapters (Vulkan, GL/GLES, null) | K9 ratchet; CAP011 rule 10 | 7 → 3 adapters landing 2026-10-10 |
| D6 | One task API; render off the main thread; recording scales with workers | `jobs_ratchet.py` `legacy-job-api` = 0; K9 checks | 297 sites |
| D7 | Android arm64 device lifecycle and package; Apple builds and runs | R29 / R36 device runs | builds only |
| D8 | Hammer always builds and plays a map | `corpus.hammer.loop`, `corpus.hammer.ui` | passing |

## Milestones

In order; a milestone closes only when its exit criterion is measured.

| ID | Milestone | Exit criterion | Status |
| --- | --- | --- | --- |
| M0 | Reset | Adapters cut to three; AGENTS.md split; no orphaned work in the tree | finishing (adapter deletion, fb) |
| M1 | Measured baseline | Frame floor and sweep at HEAD on the 8060S and 3070, with per-thread CPU and per-pass GPU time, recorded below as the budget table | not started |
| M2 | Scene authority (R89) | `render-scene-bypass` 0, in slices: S1 static props, S2 BSP world, S3 posed models (feeds GPU skinning), S4 view and frame terms, S5 baked lighting environment, S6 dynamic draws | S1 next |
| M3 | Frame off the main thread (R94, K9) | Render recording off the main thread, scaling with workers; `legacy-job-api` falling every week | not started |
| M4 | Lighting complete in game (R96, R90, R65/R66) | Every declared term matched game/lab | partial |
| M5 | Cohorts and legacy retirement (R91) | Legacy stream use 0 on Portal and Portal 2 | partial |
| M6 | Performance acceptance | D1 and D2 pass | blocked on M1–M3 |
| M7 | Platform acceptance | D7 passes | not started |

M1 starts now, beside M2: optimization work is chosen from its budget table,
not from guesses.

## Budget table (filled by M1)

Per-frame milliseconds at 1080p High on the 8060S, intro4 relit, p50 / p99.

| Owner | CPU (main) | CPU (render) | GPU | Target |
| --- | --- | --- | --- | --- |
| (to be measured) | | | | 8.33 total |

## Workstreams and owners

One active task per session. Change a row only through the project manager.

| Session | Workstream | Current task | Files it owns now |
| --- | --- | --- | --- |
| source-engine-17 | Project management | Reviews, assignments, orphan cleanup, this page | `docs/agents/*` |
| source-engine-fb | Render core | M0 adapter deletion, then M2 S1 (static props through `render.scene`) | render/, materialsystem/shaderapicore, deletion files |
| source-engine-f1 | Performance | M1 baseline on the 8060S (bazzite) and 3070 | its progress entry only |
| source-engine-7b | Jobs | M3: first `legacy-job-api` cohort onto the job system | jobsystem/, the cohort's call sites |
| source-engine-3a | Performance (bisect) | Performance bisect in detached worktrees | `../source-engine-bisect-*` |

## Risks

| Risk | Effect | Response |
| --- | --- | --- |
| The frame-time gap (D1) is 2–4.5× and has no owner today | The deliverable fails at its hardest gate | M1 now; every render slice records its frame cost |
| Main-thread and render-thread CPU (p50 9–14 ms render thread at 1080p) | CPU-bound frames even with a fast GPU | M2 S3 (GPU skinning), M3 |
| Shared working tree with several sessions | Lost or tangled work | File ownership above; `tools/agent/commit_paths.py`; orphan review every 30 min |
| Hardware access (bazzite, 3070, Tab S8) | Gates can't be measured | Measurement sessions book the device; unavailable runs are recorded, not skipped |
| Mod shaders no longer render (stdshader passes removed) | Compatibility claim narrower than RFC 0001 says | User decision needed |

## Decisions needed from the user

1. The `ivp` submodule fix (C++17 `inline`, needed for MSVC) is uncommitted
   in the fork since 2026-09-21: push it to the fork and bump the pointer?
2. Mod-shader compatibility: declare it unsupported, or keep a narrow
   bytecode path behind the frontend?
3. Keep tvOS and the MSVC/Wine dedicated server as extra scope?
