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
- The engine's SPIR-V is built with `glslc -O`, which strips names. Bindings
  are therefore shown as `set.binding`. `debug-pixel` reports registers
  (`_2047`) instead of GLSL variables, and native objects have generic names
  (`2D Image 1402`). Match set and binding against the shader source.
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

## Worked example: gi_door's sphere stays lit with the door closed (2026-09-25)

`gi_runtime.py capture --renderdoc` (above) with the door closed, then:

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
