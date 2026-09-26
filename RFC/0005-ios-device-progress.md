# RFC 0005 on the iOS device: conformance and performance runs (R29)

User request (2026-09-25): "on ios run any conformance testing that the RFCs
talk about. ie. performance and stuff". This record covers the device harness
built for that, what ran on the phone, what failed and why, and what remains
unverified. It certifies no R29 or R36 gate. The platform record for the iOS
product is [static composition](0001-static-composition-progress.md).

Device: iPhone 16 Pro (A18 Pro: 2 performance and 4 efficiency cores),
iOS 27.0 (24A435), reached through the user's macOS VM (`ssh macvm`,
`xcrun devicectl`). Toolchain: the iOS product profile's LLVM 22.1.8 and the
iPhoneOS 26.5 SDK, built on Linux.

## Harness (installed)

- `tools/quality/ios_device.py` is the one owner of the device plumbing. It
  resolves the connected device, copies files into and out of an app's data
  container, launches an app attached to its console with arguments, and
  terminates it after a timeout. `ConformanceHost` runs the test host's
  programs. Signing and installing stay with `ios-deploy.sh`.
- `tools/quality/ios_frame_pacing.py` is the device form of
  `frame_pacing.py`. It uses the same scenario, cfg chain, `-vkframestats`
  stream, analysis and budget checks. The scenario reaches the installed
  Portal app through `Documents/commandline.txt`, and the frame stream is
  copied back.
- `tools/quality/ios_conformance.py` covers the manifest's compiled suites
  on the phone, declared by the profile `quality/profiles/ios-arm64-device.json`
  (hosts: the Linux headless-core and native Vulkan GPU profiles).
  - Each suite compiles with `conformance.py`'s own compile commands through
    an iOS compiler wrapper, with `main` renamed.
  - Its objects are partially linked (`ld64 -r`) into one module object whose
    only external symbol is its entry, so suites cannot collide.
  - All suites link into one test-host app, `Conformance.app`
    (`com.panos.sourceengine.conformance`; SDL3 owns `main`; MoltenVK serves
    Vulkan). The repository files the suites read are bundled with it.
  - The app is launched once per suite, so each suite has its own process.
    Its console is classified by `conformance.classify_run`, and the evidence
    is the shared `conformance-evidence/v2`.
  - Negative compile tests are judged as `conformance.py` judges them, on the
    expected diagnostic.
- Profile declarations (reviewed facts, not workarounds):
  - suites that build a first-party shared library at run time, need a
    sanitizer runtime, or seed a libstdc++ dual-ABI mismatch (libc++ has one
    ABI) are skipped with the reason and never certified;
  - suite rows' Linux platform defines are replaced by the iOS product's;
  - `common/` is searched last for `sse2neon.h`, as ARM product targets do.
- The same app carries the benchmark hosts the budget tools drive:
  `vphysics_conformance`, `jobsystemthreadpoolbench` and
  `jobsystemhostframetest`.
  - Their sources, includes, defines and uses are read from their Waf
    targets by running each wscript's `build()` against a recording stub, so
    Waf stays the owner.
  - They build with the iOS product's release flags and link the iOS build's
    own libraries and module objects.
  - `physics_bench.py --ios` and `scheduler_budgets.py run --ios` run them on
    the phone. The physics host selects a linked provider through the typed
    catalog (`--provider linked:<name>`, `VPHYSICS_CONFORMANCE_LINKED_PROVIDERS`).

## Product findings fixed on the way

- **Exit on quit:** after `quit`, the app stayed on a blank screen, because
  SDL keeps a UIKit app alive after `main` returns. `ios_main.cpp` now exits
  with `LauncherMain`'s status, as the Android activity finishes.
- **60 Hz cap:** every frame took 16.66 ms with `fps_max 1000` and vsync off.
  Without `CADisableMinimumFrameDurationOnPhone`, iOS caps an iPhone app at
  60 Hz on a ProMotion display; the Info.plist now declares it.
- **`public/tier1/utlmemory.h`:** under `NO_MALLOC_OVERRIDE` on a non-Linux
  target, `CUtlMemoryAligned::AllocSize` used a `g_pMemAlloc` that
  `memalloc.h` does not declare, so it did not compile. Without the override,
  the size is now remembered, as on Linux. No product defines
  `NO_MALLOC_OVERRIDE`.
- **SDF portal-transport check:** `render.indirect-light.sdf` failed on the
  phone. The phone's SDF open-pair value (0.0957) equals Linux's exactly;
  the check was wrong on GPUs without ray query. There the erased-wall
  reference is the SDF's slab, which also removes floor and ceiling and so is
  darker, and the 0.5–1.5x ratio was still applied. The upper bound now
  applies only to the exact ray-query reference, as the test's own comment
  intended. This affects every GPU without ray query, including the Android
  profiles' GPUs.
- **Debug API socket fixture:** it used `/tmp`, which is outside an app
  sandbox. It now uses `$TMPDIR` with a name short enough for Apple's
  104-byte socket path limit.
- **Thread census in the physics host:** the census read Linux `/proc`, so
  the parallel-step contract could not count threads on Apple platforms. It
  uses `task_threads` there.

Each changed suite passes again on Linux (`quality-results/ios-fixes-linux.json`).

## Results on the phone

### Frame pacing (R32-FRAME-PACING workload)

`ios_frame_pacing.py` ran `quality/workloads/portal-frame-pacing-v1.json`
(testchmb_a_02, portals opened, walked through repeatedly; three passes).
Result: **pass** (`quality-results/ios-pacing-2`). Native Vulkan through
MoltenVK, fullscreen at the display's native size, `mat_queue_mode 0`.

| Pass | Median frame | CPU median | p99 | Max | Hitches |
| --- | --- | --- | --- | --- | --- |
| 1 (first use) | 9.20 ms | 3.87 ms | 13.69 ms | 36.57 ms | 5 |
| 2 | 9.16 ms | 4.00 ms | 13.02 ms | 27.26 ms | 2 |
| 3 (warm, gated) | 9.16 ms | 4.08 ms | 11.68 ms | 14.06 ms | 0 |

- The scenario's budget (`max_hitches` 0 on the warm pass) holds.
- Early-pass hitches are first-use costs: texture creation, single-submit
  uploads, and draw emission when the portals first render.
- Before the ProMotion fix the same run was pinned at 16.66 ms per frame
  with a 6.4 ms CPU median (`quality-results/ios-pacing-1`).
- This is no render budget: `render-v1` rows (R32-RENDER-BUDGETS) do not
  exist for any profile yet.

### Conformance suites

Final run (`quality-results/ios-conformance-final.json`, device build of
all 196 iOS-runnable suites of the two host profiles):

| Domain | Suites | Matched | Skipped (declared) | Mismatched | Checks |
| --- | --- | --- | --- | --- | --- |
| Q-CONTENT | 13 | 13 | 0 | 0 | 621 |
| Q-EDITOR | 61 | 61 | 0 | 0 | 1690 |
| Q-FOUNDATION | 40 | 36 | 3 | 1 | 2074 |
| Q-JOBS | 20 | 16 | 4 | 0 | 194859 |
| Q-PRESENTATION | 62 | 62 | 0 | 0 | 2814 |

- **Totals:** 196 suites; 188 matched, including the 7 negative compile tests
  judged on their diagnostics and the sensitivity rows detecting their seeded
  defects on the device; 7 skipped by the profile's declarations
  (2 shared-library loader fixtures, 4 TSan rows, the libstdc++ dual-ABI
  seed); 1 mismatched; 202058 checks.
- **The one mismatch** (`platform.clock.sensitivity`) was a lost console
  capture. The app launched and exited 0, but `devicectl` captured nothing
  from it. The runner now relaunches when the host's start line is missing
  and records the loss; it never repeats a run that has output. With it, the
  suite and `platform.clock` pass three of three attempts
  (`quality-results/ios-conformance-clock.json`).
- **Earlier full run** (`quality-results/ios-conformance-full.json`, before
  the fixes above): 186 matched, 6 skipped, 4 mismatched. The mismatches were
  the socket fixture (2), the SDF reference and the libstdc++ seed. All four
  fixes were checked on the phone and on Linux.
- **The GPU suites ran on the A18 Pro through MoltenVK 1.4.2**, presenting
  through UIKit. Ray query is absent, so ray-query checks are not run (for
  example, 26 checks in `render.indirect-light.sdf` against 35 on RADV).
- **Q-PHYSICS has no compiled device suites**: its manifest rows are command
  suites. The physics benchmark below covers it on the phone.

### Physics benchmark (RFC 0004/0013, `quality/budgets/physics-v1.json`)

`physics_bench.py --ios` ran the benchmark scenes on the phone through
the typed provider catalog, in the product's release build.

- **New profile:** `ios-arm64-iphone16pro`.
  - Timing limits: about twice the worst of three calibration rounds
    (`quality-results/ios-physics-calibration`), the file's own method.
  - 8 workers is declared unsupported: the A18 Pro has 6 logical CPUs, and
    Box3D's `GetMaxWorkerCount` clamps to that. `physics_bench.py` now
    removes a profile's unsupported worker counts from its runs and gates,
    and reports the dropped rules as not applicable. This has a unit test.
  - The memory rules are not declared: the calibration run did not keep
    per-run memory growth.
- **Verification** (`quality-results/ios-physics-verify2`):
  - box3d-parity (required): pass;
  - shape-inertia (required): pass;
  - all 15 sensitivity faults detected;
  - ccd-bullets (planned): fails as on Linux, with 56 of 64 projectiles
    tunneling;
  - parallel-step (required): fails on the speedup rules alone. From 1 to
    4 workers: 1.00x (pile-4096 p50; 1.49x at calibration), 1.18x
    (pile-1024) and 1.06x (ragdolls-128). Timings rose across the long
    session (pile-4096 at one worker, p50 3.96 -> 5.72 ms), consistent
    with thermal throttling, and 4 workers share 2 performance and 4
    efficiency cores. Worker-count invariance holds: one digest at 0, 1, 2
    and 4 workers in every round.
- **Update (2026-09-26): Box3D is the Apple products' provider (user
  decision), and the pool step scheduler fixes the scaling.** An A/B showed
  the per-task pool bridge, not the phone, was the bottleneck: Box3D's own
  scheduler scaled on the same build. With `CPoolStepScheduler`
  (`quality-results/ios-physics-bridge`):
  - box3d-parity and shape-inertia pass;
  - pile-1024 1.36x and ragdolls-128 1.32x pass;
  - pile-4096 is 1.49x against its 1.50x rule (1.57x in an interleaved A/B).
    Two workers are as fast as four, bound by the two performance cores.

  See the [RFC 0013 record](0013-progress.md#pool-step-scheduler-parallel-stepping-on-apple-silicon-2026-09-26).
- **Harness fixes found by these runs:**
  - a launch the device never started (its connection dropped) was recorded
    as an incomplete run; `ios_device` now retries it;
  - the thread census now works on Apple platforms (`task_threads`).

### Scheduler budgets (RFC 0003 R20, `quality/budgets/scheduler-v1.json`)

`scheduler_budgets.py run --ios` ran the pool microbenchmarks and the host
frame graph's overhead probe on the phone.

- **New profile:** `ios-arm64-iphone16pro`, calibrated from three runs at
  about twice the worst run.
- **Verification:** pass on all 13 rows (`quality-results/ios-scheduler-verify.json`).
  Examples: pool wake latency 20-23 us (limit 51/61), graph cost 5.8 ns per
  node (limit 13), 4-producer/4-consumer `CTSQueue` 134 ms (limit 300).
- **Capacity:** the row is declared from the pool policy (CmpJob 3, IOJob 4,
  SaveJob 1, MatQueue 1) and unverified on the device. The census reads
  Linux `/proc` thread names; no iOS census of a running product exists.

## Not done

- A thread census of the running iOS product (the scheduler capacity row) and
  iOS memory limits for the physics rules.
- `quality/baseline.json` does not yet list these iOS checks: the Apple
  profiles stay `runner_requirement: optional` (AGENTS.md), and registering
  device checks in the R01 audit is a separate change.
- The R29 iOS obligations beyond these runs: lifecycle (background and
  foreground, surface recreation), memory pressure, rotation and touch on the
  device (the touch UI, gyro and portrait support are built but untested),
  and an iOS simulator profile.
- R36 (release readiness): per-platform material images, loss and recovery,
  power and thermal budgets, and App Store checks.
