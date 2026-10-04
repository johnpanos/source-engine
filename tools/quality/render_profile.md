# Native renderer profiling

Collect the complete High workload with the existing backend and core GPU timers:

```sh
python3 tools/quality/frame_floor.py --profile --no-stop \
  --workload quality/workloads/portal2-intro4-perf-v1/workload.json \
  --out quality-results/profile-NEW
```

Use a new output directory. The harness stages a private runtime, pins and queries
the product profile, uses a borderless native-size window, and retains the command,
driver identity, CPU costs, GPU timestamps, QA checks and console output. Profiling
adds timestamps and readback overhead; it is a diagnostic run, not performance
acceptance. It does not permit arbitrary quality overrides. A failed budget still
retains the entire run when `--no-stop` is selected.

## Resolution sweep before and after optimization

Follow RFC 0016's authoritative
[resolution-sweep policy](../../RFC/0016-render-core.md#optimization-resolution-sweep-user-decision-2026-10-03)
before selecting optimization work and when evaluating the candidate. Collect
matched complete-game baseline/candidate pairs from 1024×768 through
3840×2160, including the intermediate and declared profile extents required by
that policy. Report CPU critical-path, GPU render and frame-interval medians and
tails, variability and absolute/percentage changes for each resolution. Identify
the limiting work with timings and diagnostic traces; a flat interval can also
come from VSync, a frame cap or a wait. No frame gain at a CPU-limited 1024×768
does not rule out a GPU optimization improving 4K.

Use identical content, route, quality, samples and mode within each pair, repeated
interleaved runs and a fresh evidence directory per run. Verify actual extents
and scene coverage; account for the 4:3/16:9 aspect change, LOD, dynamic resolution,
clocks and thermal drift. Keep pass profiling, shader dumps and screenshots in
separate diagnostic runs. For upscaling, record input and output sizes and the
preset; compare modes at the same output size with their explicit quality policies.

**Current collector limitation:** `frame_floor.py` has `--width` and `--height`,
but a workload naming `render_budget_row` is pinned by `configure_budget` to that
row's exact dimensions. The High command above therefore cannot perform the
sweep by changing those arguments, even with `--profile` or `--no-stop`.
This collector has no sweep command. Its offscreen mode also caps the back
buffer at 1024×768 and cannot establish the larger points. The sweep needs a
separately recorded diagnostic workload/collector setup that preserves the same
complete route, quality receipts and CPU/GPU evidence while permitting the
diagnostic extents.
Do not remove or relax the shipping budget row to obtain sweep results. Record
unavailable points until that setup exists and retain the declared-resolution
High acceptance run separately. Existing single-resolution receipts establish
only their recorded point, not a 1024×768-to-4K result.

Analyze any retained scenario's `frames.jsonl`:

```sh
python3 tools/quality/render_profile.py \
  quality-results/profile-NEW/sp_a1_intro4_probe64/frames.jsonl \
  --console quality-results/profile-NEW/sp_a1_intro4_probe64/console.log \
  --json quality-results/profile-NEW/profile.json \
  > quality-results/profile-NEW/profile.tsv
rg '^gpu_segment|^core_gpu_inclusive' quality-results/profile-NEW/profile.tsv
```

The JSON report includes interval/CPU/GPU percentiles, inclusive CPU costs,
backend GPU segment durations, per-phase results, missing frame IDs, and the core's
per-second reports. TSV has one grepable row per metric or pass. Existing captures
without pass timers still produce totals and name the missing coverage; missing
timing is never reported as zero. Exit 0 means complete timing coverage, **not**
passing performance or image quality; 1 means incomplete timing coverage; 2 means
malformed evidence or an incomplete measurement bracket. Corrupt/truncated records,
duplicate JSON keys, missing CPU frames and conflicting GPU results are rejected.

## Strict Intro4 material captures

[RFC 0016](../../RFC/0016-render-core.md#the-rules) owns the strict FSR-on/off
game integration requirement. Use the actual Portal 2 native Vulkan product,
`sp_a1_intro4_relit`, `r_core_world 1`, `r_core_world_strict 1` and default
cohorts (`r_core_dynamic_draws 0`). Run matching captures twice: FSR active at
the recorded reconstruction scale, and FSR off with `r_temporal_scale 0`.
Query both `r_core_world_strict` and `r_temporal_scale` in each run. Retain
initial-spawn coverage as well as the affected gameplay/material states.
Use the user's launcher physics, job settings and presentation configuration
when reproducing a reported crash; record deviations explicitly.

`intro4_material_check.py --scene materials|doors|cables --commands` emits each
installed `portal_boot.py` console sequence. Capture both modes at the same
1024×768 requested viewport (desktop HiDPI is retained), using the corresponding
`--startup-command "r_temporal_scale SCALE"`. Pass each capture to the oracle:

```sh
python3 tools/quality/intro4_material_check.py --scene doors \
  --capture quality-results/intro4-rendercore-completion/door-box-paired-game \
  --out quality-results/intro4-rendercore-completion/door-box-pixels.json
```

Repeat with `--scene materials` for decals, floor indicators and glass, and
`--scene cables` for the lit rope on/off/on cycle. These are bounded pixel
checks, with seeded missing-surface controls; they do not certify all materials,
glass optics, presented HDR output or whole-frame performance. A game boot pass
without the image oracle and zero queried claimed-view failures is insufficient.
Do not turn strict mode off to obtain a passing receipt.

## Timing semantics

- GPU results arrive in a later CPU record. `gpu[0]` is the originating frame ID;
  the analyzer joins it before phase selection. Repeated results are counted once.
- Backend `gpu_passes` are sequential segments between timestamp marks. Their
  labels identify targets, copies, resolve and presentation. They can contain core
  work and waiting between passes; they are not individual shader timings.
- Core reports time existing graph/encoder labels: forward shading, prepass,
  GTAO, clustering and shadow depth/copies. Nested scopes are **inclusive**. A
  parent already contains its children; do not add both to obtain frame time.
  Counts describe encoder sections, not necessarily cameras or full-screen draws.
- Core reports are means over completed frames, taken every second. Their current
  format has no frame IDs. The analyzer preserves each whole-console window
  (including warmup) and nesting, without fabricating phase correlation or p99s.
  Dropped timestamps make coverage incomplete.
- CPU counters also nest: `emit_convert` belongs to `emit`, for example. They
  measure wall time on the issuing sequence; thread CPU time and frame interval
  measure different things. CPU and GPU run concurrently and must not be added.
- `gpu_render` excludes the final swapchain presentation span.
  `gpu_including_present` includes it. Frame interval is presentation to
  presentation, which is what the player experiences.
- `gpu_sequence` in the raw JSONL preserves ordered backend segments for every
  120th resolved frame. This is sampled GPU ordering, not a correlated CPU/GPU
  activity trace. No absolute GPU clock correlation is inferred.

The native stats owner is `materialsystem/shaderapivulkan/vulkan_frame_stats.h`;
core scopes are owned by `render/graph/pass_timers.*`. This analyzer reuses
`frame_pacing.py`'s percentile and GPU-segment summary. The separate
[`render_trace.py`](render_trace.md) captures DXVK draw correctness and changes
timing; it must not be used as a performance baseline.

## Comparing another renderer

Match map bytes, camera/route, drawable size, sample count, texture/LOD settings,
GPU and power state. Record the executable/build ID, driver, exact commands and
observed settings. Report differences in rendering work (baked versus runtime
lighting, probes, shadow filtering, materials, post effects). A different image is
a contextual baseline, not proof of an equivalent-output optimization.

Do not label another application's presentation intervals as GPU execution time.
When pass timestamps are unavailable, explicitly leave that breakdown unavailable.
Keep warmup/loading out of the measured bracket, retain completion and position
checks, and run GPU workloads serially on a shared GPU.

### P2:CE presentation capture

[`p2ce_present.py`](p2ce_present.py) uses P2:CE's localhost netconsole and
MangoHud's local control socket. Start the installed executable against a private
`-game` directory, with `-netconport 21213`. Preserve installed configuration and
content. Install the same versioned camera script as the engine workload, replacing
`vk_frame_mark <name>` with `echo PROFILE_MARK <name>` for this external renderer.
The script must report the four map/position checks and `QA_DONE checks=4 failures=0`.
Retain executable/build ID, native map export, any material adapters, settings and
launch environment with the resulting evidence.

Enable the pinned MangoHud Vulkan layer, and set its configuration to
`no_display=1,autostart_log=0,control=source-p2ce-profile,log_interval=0,output_folder=<absolute-path>`.
The tool starts logging explicitly: MangoHud 0.8.3 rc1 does not start its automatic
logger with `no_display=1`. No overlay is drawn. Do not run another GPU workload
at the same time. Settings are applied by a private `profile_high.cfg` before the
camera script starts:

```sh
python3 tools/quality/p2ce_present.py collect --port 21213 \
  --control source-p2ce-profile --settings profile_high \
  --script qa/profile_intro4 --out quality-results/p2ce-NEW/observations
python3 tools/quality/p2ce_present.py analyze quality-results/p2ce-NEW/p2ce_TIMESTAMP.csv \
  --marks quality-results/p2ce-NEW/observations/marks.jsonl \
  --clock quality-results/p2ce-NEW/observations/clock.json \
  --json quality-results/p2ce-NEW/report.json > quality-results/p2ce-NEW/report.tsv
```

Collection times out or fails when the camera workload does not complete.
Analysis rejects missing/failed/duplicate checks, unordered clocks, malformed or
truncated records and empty phases. It records input hashes, presentation
percentiles and observed GPU clocks/temperature. Logging starts on the present
after the control command; netconsole markers are host observations. A fixed
0.5-second guard at both phase edges avoids attributing boundary frames to the
wrong view. This is a contextual steady-view comparison, never a trimmed High
floor verdict or a claim of GPU/pass duration. P2:CE's pass breakdown remains
unavailable through this collector.

A mounted PBR pack does not prove a PBR frame: verify the material shader used by
the map. The exported probe map's legacy BSP retains original material names, while its
modern world stage selects generated material names. Read the native texture-string
lump and capture `mat_crosshair_printmaterial` / `mat_showmaterials PBR` after the
measurement bracket to establish the actual shader and mounted content. A VMT
from the modern stage alone cannot establish what the external renderer used.

## RADV compiled shader work

For a separate diagnostic run on Mesa RADV, inherit `RADV_DEBUG=shaderstats` when
launching the private engine runtime. Retain the complete `stdout.log` and analyze
it with the strict compiler-statistics reader:

```sh
python3 tools/quality/radv_shader_stats.py quality-results/profile-NEW/SCENE/stdout.log \
  --json quality-results/profile-NEW/shaders.json > quality-results/profile-NEW/shaders.tsv
rg 'Pixel Shader' quality-results/profile-NEW/shaders.tsv
```

The [Mesa variable documentation](https://docs.mesa3d.org/envvars.html#radv-driver-environment-variables)
owns this backend option. The reader records shader and pipeline hashes,
registers, spills, scratch storage, code size, instruction count and the compiler's
maximum subgroups per SIMD. It rejects missing fields, duplicate keys, truncated
blocks and absent shader statistics. These are static compiled-program counts,
**not execution time or measured dynamic occupancy**. Repeated compilations remain
records; their number is not a draw count. Pipeline hashes alone do not identify
a material or correlate a shader with a timed graph pass. A high register count
can limit residency and latency hiding; it does not independently prove a stall.
Do not enable driver dumps for accepted production performance measurements.


## Quick opaque world/model comparison

Use the installed Intro4 workload for short iterations on the complete game
frame. With one private, already staged runtime and the same executable/library
bytes, collect `frame_floor.py` runs with `--opaque-batching off`, then `on`,
then `on`, then `off`. Set `--workload
quality/workloads/portal2-intro4-perf-v1/workload.json` and a fresh `--out` for each
run. `--skip-stage` still installs the selected workload scripts and preview
options. The ordinary High resolution, effects and 4x MSAA remain mandatory;
this switch only changes compatible opaque-slot grouping. The native equivalent
is `-vkopaquebatch 0/1` (default 1).

Compare arrival, reverse and return phases separately using whole-frame interval,
CPU and GPU render time. Retain settings, binary hashes, frame logs and power
conditions. A faster isolated pass or laboratory scene cannot accept this change.
For optimization selection and evaluation, repeat the comparison under the
[resolution-sweep policy](#resolution-sweep-before-and-after-optimization), subject
to the collector limitation above. The pinned High comparison alone does not
establish batching's CPU/GPU crossover or benefit at other resolutions.
The frame JSON field `opaque_batch` contains candidate prefixes, accepted batches
and consumed follower slots for the latest stream recording; it is not a count
of meshes or lifetime totals. A capture can re-record that stream.

Run separate on/off `--preview` captures to inspect the three settled views.
Screenshots exercise stream replay and add work; their timings are not performance
evidence. The script checks the authored map name returned by VScript, while the
bracketed `status` console check independently requires the actual `_probe64`
fixture. A retail map with the same authored name cannot pass fixture identity.
Use `--profile` only for separate pass-timing diagnosis. Ordinary runs deliberately
omit pass timers; the detailed analyzer's missing-pass warnings do not provide a
pass breakdown and must not be reported as one.
