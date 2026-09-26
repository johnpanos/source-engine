# RFC 0005 on the iOS device: conformance and performance runs (R29)

User request (2026-09-25): "on ios run any conformance testing that the RFCs
talk about. ie. performance and stuff". This record covers the device harness
built for that, what ran on the phone, what failed and why, and what remains
unverified. It certifies no R29 or R36 gate. The platform record for the iOS
product is [static composition](0001-static-composition-progress.md).

Devices, both reached through the user's macOS VM (`ssh macvm`,
`xcrun devicectl`):
- iPhone 16 Pro (A18 Pro: 2 performance and 4 efficiency cores), iOS 27.0
  (24A435);
- Apple TV 4K, 3rd generation (A15), for the tvOS product profile (2026-09-26;
  [below](#apple-tv-4k-tvos-profile-2026-09-26)). tvOS is extra product scope,
  not a north-star target.

Toolchain: the product profiles' LLVM 22.1.8 with the iPhoneOS and
AppleTVOS 26.5 SDKs, built on Linux.

## Harness (installed)

- `tools/quality/ios_device.py` is the one owner of the device plumbing. It
  resolves the connected device of a platform (`iOS` or `tvOS`, as
  `devicectl` names them), copies files into and out of an app's data
  container, launches an app attached to its console with arguments, and
  terminates it after a timeout. `ConformanceHost` runs the test host's
  programs. Signing and installing stay with `ios-deploy.sh`.
- `tools/quality/ios_frame_pacing.py` is the device form of
  `frame_pacing.py`. It uses the same scenario, cfg chain, `-vkframestats`
  stream, analysis and budget checks.
  - `--profile` names the product profile. The profile gives the platform,
    the app and the content directory in the app's container
    (`content.container_directory`: `Documents` on iOS, `Library/Caches` on
    tvOS).
  - The scenario reaches the installed app through that directory's
    `commandline.txt`, which the app appends to its arguments
    (`launcher_main/ios_main.cpp`). The frame stream is copied back, and
    `commandline.txt` is emptied again.
  - `--budget-row` judges a `render-v1.json` row: its vsync limits with
    `--vsync`, its headroom limits without.
  - `--setting CVAR=VALUE` goes into a cfg exec'd before the map, off the
    engine's 512-character command line. `--env` sets the app's environment,
    and `--extra-arg` adds engine arguments.
  - `tools/quality/frame_pacing_device.py` is the earlier standalone runner
    that the Apple TV optimization used. `ios_frame_pacing.py --profile` now
    covers it, and it can be deleted once nothing refers to it.
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

## Apple TV 4K (tvOS profile, 2026-09-26)

The user's target: a locked 60 fps at 1080p on the Apple TV 4K. The product
record is [static composition](0001-static-composition-progress.md#on-an-apple-tv-4k-2026-09-26),
and the optimization record is
[frame pacing](0001-native-vulkan-frame-pacing-progress.md#apple-tv-4k-at-60-fps-tvos-profile-2026-09-26).
This section covers how the device was tested.

### Budget for a display that only presents in FIFO

The budget row is `tvos-portal-frame-pacing-60` in
`quality/budgets/render-v1.json`. It was set before any optimization, then
revised once, and the reasons are recorded in the row.
- **Vsync off measures nothing.** The Apple TV presents only in FIFO, so
  "vsync off" measured the same paced frames. It is not headroom.
- **Frame-start jitter is not a missed frame.** Under vsync, a 20 ms interval
  followed by a 13 ms one still shows a new image at every refresh, so a p99
  on frame intervals flagged frames the player never saw drop.
- **With vsync (what the player sees):** the row counts missed refreshes
  (`frame_pacing.count_missed_refreshes`). Each frame counts
  max(0, round(interval / period) − 1). Its limits are 0 missed refreshes, 0
  hitches and no frame over 25 ms.
- **Headroom (the frame's cost):** GPU render p99 and CPU p99 of the
  presenting thread, each at most 16.0 ms.
- **GPU render time needs `-vkgputimers`.** The frame's whole GPU span
  includes the present's wait for vsync. The `gpu` record's render-only span
  excludes that wait. `-vkgputimers` also reports GPU time per pass, copy
  and capture (`gpu_passes`, printed by the runner for the warm pass) and
  one ordered frame in every 120 (`gpu_sequence`).

### Methods

- **Deploy:** `./build-tvos-app.sh`, then
  `./ios-deploy.sh --profile quality/product_profiles/portal-tvos-native-vulkan.json --with-content`.
  The app's platform picks the Apple TV (`--device tv` names it). Content goes
  to `Library/Caches`, and unchanged files are skipped on later runs.
- **Measure:** run `ios_frame_pacing.py` twice per candidate, once with
  `--vsync` and once without, both with `-vkgputimers`. Frame intervals and
  missed refreshes come from the vsync run; GPU render and CPU p99 come from
  the other.
  ```sh
  P=quality/product_profiles/portal-tvos-native-vulkan.json
  python3 tools/quality/ios_frame_pacing.py --profile $P --vsync \
      --budget-row tvos-portal-frame-pacing-60 --extra-arg -vkgputimers --out <dir>/vsync
  python3 tools/quality/ios_frame_pacing.py --profile $P \
      --budget-row tvos-portal-frame-pacing-60 --extra-arg -vkgputimers --out <dir>/headroom
  ```
- **Judge on the warm pass.** Earlier passes include first-use costs. One
  run is noisy near the limit: a vsync run of the final build read GPU p99
  16.05 ms. Repeat a candidate before keeping or rejecting it.
- **Disposable shader experiments** found the bottleneck without a rebuild or
  reinstall:
  - Write a modified SPIR-V module as `<array>.<embedded hash>.spv`, the name
    the backend's debug-variant lookup uses
    (`materialsystem/shaderapivulkan/vulkan_shader_library.cpp`; real
    variants come from `shaders/regen_material_spv.py --debug-out`, see
    `tools/renderdoc/README.md`).
  - Push it into `Library/Caches/<dir>` and run with
    `--env SOURCE_VK_SHADER_DIR=<dir>`. The app's working directory is its
    content directory, so a relative path works.
  - A constant-color lightmapped shader cut GPU render from 14.3 to 7.9 ms.
    That showed fragment-shader cost on the tile-based GPU was the
    bottleneck, before any engine change.
  - Experiments that barely moved it: smaller textures (`mat_picmip 4`) and
    no lighting (`mat_fullbright 1`), set with `--setting`.
- **Identity oracle for each backend change:** `material_pixel_conformance.py`,
  run on Linux on all 14 families with HDR none and integer, before and after.
  The device run shows the speed, and the pixel oracle shows nothing else
  changed.
- **Settings sweep:** raise one setting at a time with `--setting`. Keep it
  only if both budget modes still pass. The kept set became the tvOS
  defaults.

### Results

Final build, warm passes: median 16.7 ms, max 21.1 ms and 0 missed
refreshes with vsync. GPU render is 10.4 ms median and 15.8 ms p99; CPU p99
is 12.2 ms. Both modes of the row pass. The baseline was a GPU render median
of 16.3 ms and an 18.4 ms frame median.

- Sweep:
  - **Kept:** textures High, models High, 16x anisotropic filtering, shadows
    Medium.
  - **Rejected:** shader detail High (GPU p99 16.4 ms), color correction (p99
    18.4 ms), MSAA 2x (18.5 ms) and 4x (21.2 ms).
  - **Crashes:** shadows High ends the app, because flashlight shadow depth
    is not implemented on native Vulkan.
- **Controller:** an Xbox Wireless Controller over Bluetooth drives the game.
  Its buttons survive a relaunch since `341a2bed`.
- **Evidence retention:** the runs' output directories were scratch and are
  not kept. The numbers are recorded in the budget row and the frame-pacing
  record. Reproduce them with the commands above at `1255e01b` or later.

## Lessons for device runs

These are traps that cost time on the Apple TV and the phone. Each has a fix
in the tools or a rule for future runs.

- **Read the device's console.log, not only devicectl's console.** Engine
  output that happens before the console attaches, and some later lines
  (joystick connects), never reach the `devicectl` stream. Run with
  `-condebug`, then copy `<content>/<game>/console.log` back with
  `ios_device.Device.get`. The controller bug was found this way: the log
  listed 16 `"A_BUTTON" isn't a valid key` errors from `config.cfg`.
- **The 512-character command line.** The engine rejects a longer command
  line, and harness arguments come close. Put settings in a cfg
  (`--setting`; on tvOS, product defaults go in `tvos_defaults.cfg`) and
  tool paths in the environment (`--env`).
- **Archived settings leak between runs.** `config.cfg` archives cvars such
  as `volume`. A benchmark that ran with `+volume 0` muted the user's next
  ordinary launch. Runs no longer mute sound. A run that changes an archived
  setting must restore it.
- **Stale output.** A failed launch left the previous run's frame stream in
  the container, and it was analyzed as the new run. The runner now empties
  the stream before launching and doesn't analyze a failed run.
- **Launches the device never started.** When the device connection drops,
  `devicectl` reports no launch. `ios_device` retries up to three times, and
  it never repeats a launch that produced output.
- **The macOS VM reboots under heavy load.** Long device sessions lose it.
  Check `ssh macvm true` before blaming the device, and don't run large
  builds on the Mac during a measurement.
- **Wait loops.** `pgrep -f NAME` in a shell loop matches the loop's own
  command line, so the loop never ends. Use `pgrep -x` or match on a
  process's own output file.
- **Asleep devices.** A sleeping Apple TV fails launches until someone
  wakes it. The user had to wake it once in this session.
- **Thermals.** Long phone sessions slowed the physics timings (pile-4096 at
  one worker, p50 3.96 -> 5.72 ms). Interleave A/B runs rather than running
  one side after the other.
- **Purged content on tvOS.** The system may purge `Library/Caches` while
  the app is not running. The app then shows an alert, and
  `ios-deploy.sh --with-content` recopies the content.

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
- Apple TV:
  - one map and one device are measured, and the GPU margin is thin;
  - the iOS profile was not re-measured after the Apple TV changes;
  - no conformance, physics or scheduler runs on the Apple TV;
  - no tvOS simulator profile;
  - `quality/budgets/render-v1.json` has no iPhone row.
