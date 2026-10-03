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
