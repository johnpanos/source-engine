# RenderDoc workflow

Capture a frame of the engine headless and inspect it from the command line,
without opening a window. Use it to answer "which draw, which binding, which
value" questions about native Vulkan frames.

## Capture

A capture boots a private runtime under `renderdoccmd` (the SDL offscreen
driver, no compositor), then records the frame after the final screenshot
command:

```sh
# Any map: portal_boot.py arguments; --out is required.
python3 tools/renderdoc/rdc.py capture --out /tmp/cap --map gi_door \
    --content-root quality-results/rfc0011-maps/door/content \
    --console-command "ent_fire Door Enable"

# A GI fixture camera (camera placement, producer, scene change):
python3 tools/quality/gi_runtime.py capture --fixture door --camera far \
    --map-build quality-results/rfc0011-maps/door --build build --view 0 \
    --capture-wait 700 --out /tmp/cap --renderdoc \
    --console-command "r_indirect_producer sdf; wait 150; ent_fire Door Enable"
```

The `.rdc` path is printed. It is also recorded in `evidence.json`
(`renderdoc.captures`) and in `capture.json` for `gi_runtime`.
`portal_boot.py --renderdoc` is the underlying option. A boot that writes no
capture fails.

Both commands also run the debug shader variants (`portal_boot.py
--shader-debug`, below). `rdc.py capture --release-shaders` captures the
embedded release shaders instead.

## What a capture shows

The native backend makes captures readable in three ways:

- **Object names.** `VK_EXT_debug_utils` names are set on images, views,
  framebuffers, shader modules and pipelines. Examples:
  - Source's own texture names (`gi_door/wall/basecolor`, `_rt_poweroftwofb`);
  - the backend's textures (`RPRB reflection probes`, `PRBV probe atlas`,
    `LMAP total`, `SDFV shadow field`, `PBR split-sum LUT`, `back buffer 4x
    MSAA`);
  - pipelines by their shaders (`skin.vert / model_pbr.frag -DPROBE_VOLUME
    (MSAA)`).
- **Labels.** Each render pass gets a marker naming its target (`pass: back
  buffer (sRGB)`). Backend phases are regions: `compute work`, `texture
  uploads`, `copy to render target`, `MSAA resolve`, `scene capture (glass)`
  and `present: gamma`. Compute dispatches are labelled with their program.
  Source's PIX events (`IShaderAPI::BeginPIXEvent`) become regions in record
  order: `CSimpleWorldView::Draw`, `DrawViewModels`, `DoEnginePostProcessing`
  and Portal's `Portal_Step*`. The cvar `mat_pix_events` decides at run time
  which are gathered (`PIXEventLevel`, `public/materialsystem/imaterialsystem.h`):
  - `-1` (the default) is auto: 1 while names and labels are on, else 0;
  - `0` gathers none, so a plain run only compares an integer at each site;
  - `1` gathers the view events;
  - `2` also gathers per-object events: `R_StudioDrawGroupHWSkin
    (<model>)`, brush models and flex.

  Portal's portal-rendering events call `BeginPIXEvent` directly and are
  always sent. A `PIX_ENABLE` build gathers every event, as Valve's did.
- **Debug shaders.** `shaders/regen_material_spv.py --debug-out DIR` compiles
  every embedded shader again with source-level debug information
  (`glslangValidator -gVS`: NonSemantic.Shader.DebugInfo.100, unoptimized).
  With `SOURCE_VK_SHADER_DIR=DIR`, the backend creates its modules from these
  variants. Bindings then have their GLSL names (`reflectionProbes`,
  `probeAtlas`), and `debug-pixel` reports GLSL variables (`environment`,
  `flags`) and steps source.
  - Each file is keyed by the FNV-1a hash of the embedded array it replaces
    (`material_spv_index.h`, generated with `material_spv.h`). A file whose
    shader has changed since it was written is never used; the boot log
    lists it with the reason.
  - The variants are unoptimized, so floating-point results can differ
    slightly from the release shaders. In the gi_door example below, the
    sphere pixel was identical.

When names and labels are emitted is one policy
(`render_vulkan::DebugLabelsWanted`):

- by default, when the validation layer is on or a capture tool reports that
  it wants debug markers through `VK_EXT_tooling_info` (RenderDoc does);
- always with `-vkdebuglabels`;
- never with `-novkdebuglabels`.

A plain run pays nothing. The boot log says which applied:
`[NativeVulkan] debug names and labels on (RenderDoc)`. The shader directory
is an environment variable because the launcher's command line is limited to
512 characters, and harness boots already come close.

To see all of this in the qrenderdoc window on your own desktop (not run by
the tooling, since it opens a window):

```sh
python3 materialsystem/shaderapivulkan/shaders/regen_material_spv.py --debug-out /tmp/vkdebug
SOURCE_VK_SHADER_DIR=/tmp/vkdebug renderdoccmd capture --opt-hook-children ./play gi_door
```

## Inspect

```sh
rdc.py info CAPTURE                        # API, frame, draw/resource counts
rdc.py draws CAPTURE [--bindings]          # every draw: targets, set.binding reads
rdc.py bindings CAPTURE --event E --stage pixel   # textures + push/uniform values
rdc.py pixel CAPTURE --x X --y Y [--event E]      # pixel history: which draw wrote what
rdc.py debug-pixel CAPTURE --event E --x X --y Y  # step the shader for one pixel
rdc.py save-texture CAPTURE --texture ID --out f.exr [--event E]
rdc.py script CAPTURE my_query.py [ARG ...]       # anything else, in the replay
rdc.py gui CAPTURE                         # the qrenderdoc window, for a person
```

Every inspection command accepts `--json` and prints the raw response instead.
A `script` file runs inside qrenderdoc's Python 3.14 with these names
available:

- `replay`, the `rdc_replay.Replay` object;
- `controller`, the `renderdoc.ReplayController`;
- `rd`, the `renderdoc` module;
- `args`;
- `pipeline_stage_info` and `variable_value`, the helpers.

The script fills a `result` dict, and that dict is the response.

## How it works

`rdc.py` (on the host) writes a request and starts
`qrenderdoc --python rdc_replay.py` headless. It unsets `DISPLAY` and
`WAYLAND_DISPLAY`, sets `QT_QPA_PLATFORM=offscreen`, and points
`XDG_DATA_HOME` at a private directory under
`~/.cache/source-engine/renderdoc-home`. The worker replays the capture with
RenderDoc's Python API, writes a JSON response, and exits before qrenderdoc
would open its main window. Timeouts, worker exceptions and missing responses
are errors that point at the RenderDoc log in `/tmp/RenderDoc`.

Traps found while building this (RenderDoc 1.45, Fedora package):

- The distribution ships no standalone `renderdoc` Python module. The API is
  only available inside `qrenderdoc --python`.
- On first run, qrenderdoc blocks on a modal analytics prompt, and a headless
  run then hangs forever. The private `UI.config` opts out. It needs the magic
  key `"rdocConfigData": 1`; without it the file is rejected ("Converted
  config data is invalid") and a second modal dialog blocks instead. The
  user's own `~/.local/share/qrenderdoc` is never touched.
- `hl2_launcher` re-executes itself to set its library path. `renderdoccmd
  capture` therefore needs `--opt-hook-children`, or the game process runs
  without RenderDoc and `vk_renderdoc_capture` reports "not running under
  RenderDoc".
- The embedded SPIR-V is built with `glslc -O`, which strips names. A capture
  made with `--release-shaders` shows bindings as `set.binding` and registers
  (`_2047`) instead of GLSL variables.
- `str()` of a RenderDoc enum is its number. Compare against `rd.VarType.*`
  instead.

## Tests

```sh
python3 -m unittest discover -s tools/renderdoc/tests -v
```

These cover the host protocol against fake qrenderdoc executables: an
answer, a worker error, no response, a hang, and missing tools. The replay
worker needs RenderDoc and a capture, so it is exercised by running the
commands above on a real capture.

The engine side is covered by the `render.vulkan-debug-tools` conformance
suite (`python3 tools/quality/conformance.py check --suite
render.vulkan-debug-tools`), which needs no device. It checks that:

- the generated index agrees with every embedded array;
- the shader library rejects stale, malformed and unsupported variants;
- the label policy table holds;
- labels stay balanced in every command buffer under seeded random sequences.

The GPU suites that enable validation run with names and labels on, and are
validation-clean.

## Worked example: gi_door's sphere stays lit with the door closed (2026-09-25)

`gi_runtime.py capture --renderdoc` (above) with the door closed, then:

This was found before names and debug shaders existed, so it uses event ids
and set/binding numbers.

1. `rdc.py draws`: EID 98 draws 2,280 indices, the sphere (`model_pbr`).
2. `rdc.py pixel --x 600 --y 330`: the wall behind the sphere writes 0.0012
   (dark, as expected), but the sphere writes 0.0227.
3. `rdc.py bindings --event 98 --stage pixel`:
   - the push constant `params.y` is 8 (`kMapProbe`);
   - the ambient cube is about 1.6e-5 (dark);
   - set 0 binding 5 is the 1024x515 RPRB texture.
4. `rdc.py save-texture --texture <id> --out rprb.exr`: the room B probe
   still shows room B lit and room A's light through an open doorway. It was
   baked from `states/open.usda`.

The sphere's specular comes from static reflection probes baked with the door
open. `model_pbr` reads them unrelit, and RPRB v1 carries no relight bands.
`mat_reflection_probes 0` makes the sphere black.

With the debug shaders, `rdc.py debug-pixel --event <sphere> --x 600 --y 330`
shows the same thing by name. `environment` (the probe radiance) is 1.63, and
`flags` is 8.
