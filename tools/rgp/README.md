# RGP traces, read headlessly (`tools/rgp`)

`rgp.py` captures Radeon GPU Profiler traces from RADV and reads them through
AMD's real RGP, headlessly, with structured output. It answers "where does
this shader spend its cycles" from instruction timing measured on the GPU,
without changing the shader.

```sh
# One frame of any Vulkan program on RADV, 45 s after start (here the intro4 demo).
python3 tools/rgp/rgp.py capture --out /tmp/claude-1000/i4/frame.rgp --after 45 -- \
  python3 tools/quality/demo_frames.py run --runtime /tmp/claude-1000/i4rt --no-stage --out /tmp/claude-1000/i4/run

# Pages to read (PNG): overview, barriers, expensive (events by work), event (--event N).
python3 tools/rgp/rgp.py shot /tmp/claude-1000/i4/frame.rgp --page expensive --out /tmp/claude-1000/i4/pages

# Instruction timing of events as JSON (one RGP session for all of them).
python3 tools/rgp/rgp.py isa /tmp/claude-1000/i4/frame.rgp --event 2374 --event 2373 --out /tmp/claude-1000/i4/isa

python3 -m unittest discover -s tools/rgp/tests
```

`isa` writes, per event:

- `event-N.json` (`rgp-instruction-timing/v1`): `api_call`; `summary` with the
  traced `waves`, cost % and executed instructions per wave by class (`valu`,
  `salu`, `vmem`, `smem`, `lds`, `wait`), the costliest basic blocks with
  `iterations_per_wave` and their memory instructions, the `loops`, and the
  costliest instructions; `instructions`, every ISA row (line, opcode,
  operands, hits, cost %, latency, block).
- `event-N.txt`: RGP's table exactly as copied.
- `event-N.png`: RGP's event view, including the shader statistics panel
  (vector registers, occupancy), which RGP does not let you copy.

Pick events from the `expensive` page (sorted by work duration). Cost % is
RGP's share of the shader's cycles; `hits / waves` counts loop iterations.

## Setup

- RGP: AMD's Radeon Developer Tool Suite for Linux from gpuopen.com (the
  link on the `GPUOpen-Tools/radeon_gpu_profiler` release page), unpacked to
  `~/.local/opt/RadeonDeveloperToolSuite-*`, or `RGP=/path/to/RadeonGPUProfiler`.
  Built against RGP 2.7.0.32.
- `mutter`, `dbus-run-session`, `xclip`, python `gi` (Gio), `Xlib` and `PIL`.
- Captures need an AMD GPU on RADV (`MESA_VK_TRACE=rgp`). A 1080p frame is
  about 130–200 MB; RADV writes it to `/tmp`, and `capture` moves it.

## How it works and its limits

RGP has no command line, and its bundled Qt has no accessibility bridge. The
tool starts RGP in a private headless mutter (nothing appears on the
desktop), clicks through the compositor's RemoteDesktop input, selects an
event with the Event timing filter, and copies tables with RGP's own Ctrl+A,
Ctrl+C. Only the instruction-timing table copies whole; event lists and
statistics are screenshots. Rootless Xwayland has no readable root window, so
screenshots composite each mapped X window.

Coordinates are RGP 2.7's at 2560x1440. Every step is checked, and a run
fails at once with a screenshot (`failure.png`) instead of returning wrong
data: the trace must load, the window must maximize, the selected event's
label must name the requested ID, and the copied text must parse as an
instruction table. A new RGP release may need new coordinates.

Alternatives checked (2026-10-05): OpenRGP (no licence, contains decompiled
RGP code) and ROCprofGUI (experimental `.rgp` import, needs AMD's closed
decoder) were not adopted.
