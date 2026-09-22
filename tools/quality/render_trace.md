# Native render diagnostics

The opt-in DXVK provider capture records draw submissions, actual native device
state, requested shader permutations, shader load failures and presentation
results. Its analyzer explains missing contributions by material without treating
a successful draw as proof that pixels reached the screen.

## Capture an installed Portal run

Use the configured Linux SDL3/Vulkan product build and a private staged runtime:

```sh
python3 tools/quality/portal_boot.py \
  --runtime /path/to/installed/portal-runtime \
  --build build-portal-vulkan \
  --shader-artifacts build-portal-vulkan/shaders/portal-source-matched \
  --require-vulkan --require-sdl3 --require-wayland --render-trace \
  --out /tmp/portal-render-investigation
python3 tools/quality/render_trace.py \
  --input /tmp/portal-render-investigation/render-trace.jsonl \
  --out /tmp/portal-render-investigation/render-trace-report.json
```

The harness copies writable game files and installs artifacts into its private
runtime. The shader pack must match the source's permutation schema; the compiler
and dependency pins belong to `quality/product_profiles/portal-linux-wayland.json`.
See `shader_artifacts.py --help` for rebuilding the pack from a shader demand dump.
Unmatched Steam shader caches can select missing dynamic slots even when the
static shader load did not set its failure flag.

For a direct engine launch, set `SOURCE_RENDER_TRACE` to the JSONL output path.
The following environment variables bound detail capture:

| Variable | Default | Meaning |
| --- | --- | --- |
| `SOURCE_RENDER_TRACE_FIRST` | `200` | First numbered frame with detailed native state |
| `SOURCE_RENDER_TRACE_FRAMES` | `4` | Number of consecutive frames eligible for details |
| `SOURCE_RENDER_TRACE_DRAWS` | `20000` | Maximum detailed draws across those frames |

The collector writes per-frame summaries and presentation results outside the
detail interval as well. Shader failure detail is capped at 256 events; its
terminal counter reports the full count. Draw details include viewport and
target dimensions/format, bound shader pointers, requested shader names and
indices, texture occupancy, depth/color write state, culling and blending.
Native query errors are counted instead of interpreting an unavailable state
as an actual zero viewport or null shader. All acquired device references are
released after the query. With capture disabled, no file or native state query
is created. Enabled capture changes timing and is unsuitable for performance
measurement.

## Read the report

The JSON report includes aggregate material/target data, capture coverage,
observations and a separate `actionable_errors` list. A shader request marked
failed with a nonzero actual bound shader identifies continued drawing with an
existing shader. When the pointer appeared earlier with a different successful
request, the report includes that prior permutation as `stale_shader_binding`.
A named successful request with a null actual shader is reported separately as
`requested_shader_unbound`; a missing or skipped dynamic bytecode slot can cause
this even when static loading succeeded.

Zero primitives and disabled color writes are observations: empty passes and
depth/occlusion passes can be intentional. Native draw/present errors, unavailable
state queries, zero viewports/targets, and failed or unbound named shader requests
are actionable diagnostics. Reports retain the first observed state for each
material/stage condition.

Exit status is `0` for a valid capture without observed actionable problems, `1`
for observed problems, and `2` for invalid/incomplete evidence. Wrong schemas,
missing fields, malformed/truncated JSONL, inconsistent counts, missing frame
summaries, no detailed draws, no presents or missing `capture_end` fail validation.
A detail limit produces explicit bounded coverage, not a claim about uncaptured
draws. A successful trace does **not** validate geometry visibility, occlusion,
texture contents, lighting equations or scene fidelity. Retain screenshots and
the installed-product image acceptance result alongside the trace.

## Check the collector and analyzer

```sh
python3 -m unittest discover -s tools/quality/tests -p 'test_render_trace.py' -v
```

This suite compiles the actual SDK-independent collector with a small C++ driver
and reads its emitted JSON. It checks escaped names, disabled output, detail
limits and terminal counts. Independent negative fixtures test a constant shader
left bound after a failed request, missing shaders, failed native queries, draw
and present errors, malformed/truncated records and absent evidence.

## Native Vulkan pixel conformance

With the product profile already configured, build the existing Waf targets:

```sh
WAFLOCK=.lock-waf-portal-vulkan python3 waf build \
  --targets=dxvk_math_conformance,dxvk_presentation_conformance
build-portal-vulkan/unittests/shaderextensiontest/dxvk_math_conformance
SDL_VIDEODRIVER=wayland DXVK_WSI_DRIVER=SDL3 \
  build-portal-vulkan/unittests/shaderextensiontest/dxvk_presentation_conformance
SDL_VIDEODRIVER=wayland DXVK_WSI_DRIVER=SDL3 \
  build-portal-vulkan/unittests/shaderextensiontest/dxvk_presentation_conformance \
  --negative-pixel-oracle
```

The last command deliberately exits `1` and prints the first pixel divergence;
this proves the comparator rejects incorrect expected pixels. The ordinary
presentation run must exit `0`. A missing Wayland display or hardware Vulkan
adapter fails; software rendering does not certify this hardware profile.

The math test checks independent analytic transform/projection and matrix-stack
outcomes. The native test creates an SDL3 Wayland surface, uploads a 2×2 texture,
draws a full-screen quad with compiled vertex/pixel shaders, and verifies every
pixel against four independently specified colors. It also checks clear colors,
presentation, resize/reset from 320×240 to 400×300, repeated textured rendering
and resource release. Shader fixture source and compiler/source hashes are kept
in `unittests/shaderextensiontest/sampled_quad*`; no runtime HLSL compiler is
needed. These tests isolate native draw/upload/readback behavior and supplement
the actual Portal scene gate.
