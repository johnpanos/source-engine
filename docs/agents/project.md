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
| D1 | Every frame ≤ 8.33 ms (120 FPS floor) at 1920×1080 High, 4x MSAA, Radeon 8060S | `frame_floor.py`, row `linux-desktop-high-120` | Bazzite (RTX 3070) at HEAD: mean ≈ 23 ms flat over 720p–1440p, p50 21.5, p99 66, p99.9 ≈ 205; 450 of 1,991 frames over 33 ms (5–7 before the R91 cutover, which ran 11.3 / 12.4 / 13.8 ms at 720p / 1080p / 1440p). CPU-bound: main thread 98–99%, GPU 28–31% (2026-10-10, 043566cd6) |
| D2 | Same floor through the resolution sweep (720p–4K) on the 8060S and RTX 3070 | RFC 0016 sweep | 3070 4K p50 20.0 ms (2026-10-06) |
| D3 | Source 2 lighting parity: game frames match `render_lab` frames of the same scenes, FSR on and off | game/lab captures, visual review | partial (R95/R96) |
| D4 | The game draws from `render.scene` | CAP012 `render-scene-bypass` = 0 | 345 → 340 in S1's working tree (2026-10-10 05:09) |
| D5 | Legacy stream retired; three device adapters (Vulkan, GL/GLES, null) | K9 ratchet; CAP011 rule 10 | 3 adapters (2026-10-10); legacy stream open |
| D6 | One task API; render off the main thread; recording scales with workers | `jobs_ratchet.py` `legacy-job-api` = 0; K9 checks | 279 sites |
| D7 | Android arm64 device lifecycle and package; Apple builds and runs | R29 / R36 device runs | builds only |
| D8 | Hammer always builds and plays a map | `corpus.hammer.loop`, `corpus.hammer.ui` | passing |

## Milestones

In order; a milestone closes only when its exit criterion is measured.

| ID | Milestone | Exit criterion | Status |
| --- | --- | --- | --- |
| M0 | Reset | Adapters cut to three; AGENTS.md split; no orphaned work in the tree | done (2026-10-10: 410ff4b56, 8d665920e) |
| M1 | Measured baseline | Frame floor and resolution sweep at HEAD on bazzite (RTX 3070, fullscreen 2560×1440 plus the sweep points), with per-thread CPU and per-pass GPU time, recorded below as the budget table; the R91 cutover's +9.1 ms attributed per pass and thread against 615223388 | done except 4K (blocked on bazzite VRAM), 043566cd6 |
| M1b | Recover the R91 regression | intro4 timedemo on bazzite back to ≤ 12.4 ms at 1080p and ≤ 13.8 ms at 1440p, ≤ 7 frames over 33 ms of 1,991, image unchanged: (a) imported textures uploaded on change only (6.3 ms/frame), (b) no per-frame lightmap rebuild (6.5 ms/frame), (d) first-use texture import off the main thread (prefetch at load or async; the 85–175 ms spikes behind p99.9, [spikes](../../RFC/0016-progress.md#heads-hitches-at-1080p-frame-by-frame-m1-2026-10-10-source-engine-3a), f96005255), (c) queued rendering on the core shader API | queued for fb after R89 S1, in order a, d, b, c; 3a measures each |
| M2 | Scene authority (R89) | `render-scene-bypass` 0, in slices: S1 static props, S2 BSP world, S3 posed models (feeds GPU skinning), S4 view and frame terms, S5 baked lighting environment, S6 dynamic draws | S1 next |
| M3 | Frame off the main thread (R94, K9) | Render recording off the main thread, scaling with workers; `legacy-job-api` falling every week | not started |
| M4 | Lighting complete in game (R96, R90, R65/R66) | Every declared term matched game/lab | partial |
| M5 | Cohorts and legacy retirement (R91) | Legacy stream use 0 on Portal and Portal 2 | partial |
| M6 | Performance acceptance | D1 and D2 pass | blocked on M1–M3 |
| M7 | Platform acceptance | D7 passes | not started |

M1 starts now, beside M2: optimization work is chosen from its budget table,
not from guesses.

## Budget table (filled by M1)

Per-frame milliseconds on bazzite (RTX 3070), High, intro4 relit, p50 / p99, at 2560×1440 and each sweep point.

Measured 2026-10-10 by source-engine-3a at `6ed908187` (HEAD before the M1b
fixes) against `615223388` (before the R91 cutover); evidence and method in
[RFC 0016 progress](../../RFC/0016-progress.md#where-the-r91-cutovers-91-ms-goes-m1-part-1-2026-10-10-source-engine-3a).
`+timedemoquit` of `sp_a1_intro4_relit.dem`, fullscreen, `mat_vsync 0`, the
kiln `portal2` profile's arguments; frame times per frame from MangoHud over the
playback window, two interleaved rounds.

Whole frame:

| Build | Point | Mean | p50 | p99 | p99.9 | Frames > 33 ms (of 1,991) | GPU busy | Main thread | Render thread |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| HEAD `6ed908187` | 1280×720 | 22.9 | 21.4 | 66.2 | 205 | 446 | 31 % | 98 % | none |
| HEAD | 1920×1080 | 23.1 | 21.8 | 65.7 | 207 | 455 | 28 % | 98 % | none |
| HEAD | 2560×1440 | 23.2 | 21.7 | 66.6 | 203–264 | 459 | 30 % | 99 % | none |
| HEAD | 3840×2160 | not measured | | | | | | | |
| `615223388` | 1280×720 | 11.3 | 9.6 | 31.1 | 37.0 | 5 | 53 % | 33 % | 72 % |
| `615223388` | 1920×1080 | 12.4 | 10.4 | 31.1 | 37.6 | 7 | 60 % | 30 % | 66 % |
| `615223388` | 2560×1440 | 13.8 (timedemo mean, 5 runs) | | | | | 65 % | 30 % | 64 % |

HEAD is CPU-bound and flat across resolution: everything runs on the main
thread (the core shader API runs the material system single-threaded), and
about 23 % of frames hitch past 33 ms. The 4K point and `615223388`'s 1440p
distribution are open: bazzite's gnome-shell holds 5.8 of the 3070's 8 GB, and
`615223388` now fails its shadow-atlas and scene-color allocations at 1440p
(it passed five times earlier the same day); a GNOME session restart needs
the user's OK.

Per owner, HEAD at 2560×1440 (ms per frame, means from `perf` DWARF stacks;
no per-owner p99 source exists on the core shader API):

| Owner | CPU (main) | CPU (render) | GPU | Target |
| --- | --- | --- | --- | --- |
| Core shader API: imported-texture refill per pass slot (`CFacadeCorePassSlots::MarkSlot` → `UploadTexture`), M1b (a) | 6.3 | — | — | 0 |
| Core shader API + engine lightmaps: per-frame rebuild and lock/upload (`EndUpdateLightmaps` → `R_BuildLightMap` → `TexLock`/`TexSubImage2D`), M1b (b); 0.08 before the cutover | 6.5 | — | — | ≤ 0.1 |
| Studio model draws through `EmitSurfaceToCore` | 4.9 | — | — | |
| Portal early-Z and stencil views (overlaps the two rows above) | 7.1 | — | — | |
| Core world view (`cl_render_debug_gpu_timers`) | — | — | 0.46 (4.7 before the cutover) | |
| Job system (J8) | ≤ 0.02 | — | — | |
| Whole frame | 22.9 | none (single-threaded, M1b (c)) | 30 % busy | 8.33 total |

## Workstreams and owners

One active task per session. Change a row only through the project manager.

| Session | Workstream | Current task | Files it owns now |
| --- | --- | --- | --- |
| source-engine-17 | Project management | Reviews, assignments, orphan cleanup, this page | `docs/agents/*` |
| source-engine-fb | Render core | M2 S1 (static props through `render.scene`, the user's goal for this session, mid-flight), then M1b fixes in order (a), (d), (b), (c) | render/, materialsystem/shaderapicore, deletion files |
| source-engine-f1 | Platforms | M7: lifecycle lane and fake-adb self-test landed (6204ca560, 13 seeded faults); **device run blocked: Tab S8 Ultra not reachable** | product/android, tools/quality/android_*, its progress entry |
| source-engine-7b | Jobs | M3 slice 1: `engine/host_saverestore.cpp` (14 sites: the save thread and deferred writes) onto an injected blocking runner; oracle: byte-identical saves with `save_async` 1 and 0 | `engine/host_saverestore.cpp`, the root's runner wiring |
| source-engine-3a | Performance | M1 done (043566cd6, spikes f96005255); measures each M1b fix on bazzite (sole user) | `../source-engine-bisect-{pre,head}`, its progress entry |

## Risks

| Risk | Effect | Response |
| --- | --- | --- |
| **The R91 cutover made frames 65% slower** (bazzite, 1440p intro4: 13.8 → 22.9 ms), CPU-bound (GPU 31% busy): queued rendering silently off, 6.3 ms texture re-uploads and 6.5 ms lightmap rebuilds per frame on the main thread ([attribution](../../RFC/0016-progress.md#where-the-r91-cutovers-91-ms-goes-m1-part-1-2026-10-10-source-engine-3a), 6ed908187); J8 excluded | D1 moves further away; every later gain is measured from a worse base | M1b: fb fixes in order, 3a measures each on bazzite |
| The frame-time gap (D1) is 2–4.5× and has no owner today | The deliverable fails at its hardest gate | M1 now; every render slice records its frame cost |
| Main-thread and render-thread CPU (p50 9–14 ms render thread at 1080p) | CPU-bound frames even with a fast GPU | M2 S3 (GPU skinning), M3 |
| Shared working tree with several sessions | Lost or tangled work | File ownership above; `tools/agent/commit_paths.py`; orphan review every 30 min |
| Hardware access (bazzite is the only benchmark box, one session at a time, and its gnome-shell holds 5.8 of the 3070's 8 GB, so 1440p allocations now fail; Tab S8 unreachable) | Gates can't be measured | Measurement sessions book the device; unavailable runs are recorded, not skipped |
| Mod shaders no longer render (stdshader passes removed) | Compatibility claim narrower than RFC 0001 says | User decision needed |

## Decisions needed from the user

0. **Action:** OK to restart the GNOME session (or reboot) on bazzite? Its
   gnome-shell holds 5.8 GB of VRAM and blocks the 1440p and 4K points.
0. **Order for fb:** finish R89 S1 first (its current goal from you,
   mid-flight), or switch now to the R91 regression fixes (M1b)?
0. **Action:** attach the Galaxy Tab S8 Ultra by USB, or enable wireless
   debugging and give its ip:port. M7 is blocked on it.
1. The `ivp` submodule fix (C++17 `inline`, needed for MSVC) is uncommitted
   in the fork since 2026-09-21: push it to the fork and bump the pointer?
2. Mod-shader compatibility: declare it unsupported, or keep a narrow
   bytecode path behind the frontend?
3. Keep tvOS and the MSVC/Wine dedicated server as extra scope?
4. D1's binding row (`linux-desktop-high-120`) names the Radeon 8060S, which
   is the dev host, but benchmarks may not run on the host (user direction,
   2026-10-06; they run on bazzite, an RTX 3070). Which machine certifies the
   120 FPS floor: allow certification runs on the 8060S, or re-target the
   row to bazzite?
