# RFC 0029 progress: WebAssembly Platform and WebGPU Device Adapter

Design: [RFC 0029](0029-webassembly-and-webgpu-platform.md). No ranked
roadmap row; ranking it is the user's decision.

## W3: the WebGPU adapter on Dawn and in the browser (2026-10-08)

W3 is done: the shared device suite passes on the adapter natively on the
pinned Dawn, and compiled to WebAssembly in headless Chrome, both on the host
GPU and on SwiftShader. The first slice below installed the adapter and the
native lane; [the browser lane](#the-browser-lane-w3-second-slice-2026-10-08)
completed the gate.

### W3, first slice: the WebGPU adapter on the native Dawn lane (2026-10-08)

User direction: "start the wasm backend but do not add anymore legacy at
all". This slice adds the render core's WebGPU device adapter and its WGSL
artifacts, proven natively against the pinned Dawn. Nothing in it touches a
legacy render path: no file under `materialsystem/`, `stdshaders` or any
`shaderapi*` changed, and the `legacy-backends` ratchet
(`tools/render/retirement_scans.py legacy-backends`) has no new line from it.
The browser half of W3 (headless Chromium) needs W0's Emscripten build and is
open.

#### What is installed

| Piece | Where | What it does |
| --- | --- | --- |
| Pin | `quality/toolchain/webgpu.json` | Dawn release `v20260930.214659` (revision `9af2744f`): the Linux build (`bin/tint`, `bin/tint_info`, `lib64/libwebgpu_dawn.a`, headers) and the matching `emdawnwebgpu` package for Emscripten, each with its sha256. `shader_toolchain.webgpu_release` fetches, verifies and extracts them into `dependencies/webgpu/`; `tint()` is the translator |
| Artifact format | `public/render/device/facts.h` | `ArtifactFormat::kWgsl` |
| WGSL artifacts | `tools/render/shader_artifacts.py` (`wgsl_compile`, the `WEBGPU_*` contract) | Every core program and suite fixture's SPIR-V, rewritten where WGSL has no form for it, translated by the pinned tint, and headed by lines that give the WebGPU binding types the stage uses. `WGSL_GENERATED` headers (`<stem>_wgsl.h`, namespace `::wgsl`) and `kWgsl` entries in the core artifact store's table |
| Coverage ratchet | `shader_artifacts.WGSL_REFUSED`, `check_wgsl` | `render.shader-artifacts` fails for a row tint refuses that is not listed with its reason, and for a listed row that translates again; seeded fault `wgsl-refusal` |
| Adapter | `render.device.webgpu`: `public/render/device/webgpu/provider.h`, `render/device/webgpu/` | `render.device.v2` over the standard `webgpu.h` |
| Composition | `render/composition/render_core.cpp`, `render/composition/wscript` | Provider `"webgpu"` in the catalog under `RENDER_CORE_WEBGPU` |
| Waf | `wscript` (`--render-core-webgpu`, `--render-core-device=webgpu`), `render/wscript`, `render/device/webgpu/wscript`, `quality/toolchain/policy.json` (strict `cxx20`) | Links the pinned Dawn natively |
| Architecture | `architecture/modules.json` | Module `render.device.webgpu` (backend; `webgpu/webgpu.h` its one external header; layer-contract adapter of `render.device`), Waf target `render_device_webgpu` |
| Native lane | `tools/render/webgpu_lane.py` (`fetch`, `run`, `suite`) | Builds the adapter and a suite with the host compiler against the pinned Dawn and runs it on the host GPU |
| Suite | `unittests/rendertest/core/device/test_device_webgpu.cpp`, manifest `render.device.v2.webgpu` | The shared suite plainly and with WebGPU validation errors counted, adapter clauses and two bad configurations |

#### The WGSL artifacts

SPIRV-Cross has no WGSL target and browsers take no SPIR-V, so tint is the
one WGSL translator (RFC 0029 decision 5, no Naga by user decision). Tint's
SPIR-V reader accepts only what WGSL expresses; `webgpu_spirv` rewrites the
rest before it:

| SPIR-V | Rewrite | Why |
| --- | --- | --- |
| The push-constant block (draw constants, D16) | A uniform block at group 3, binding 255 | WebGPU has no push constants; the adapter binds the block with a dynamic offset |
| A texture sampled with a comparison | Retyped as a depth image; one also sampled plainly samples through the depth type, and its sampler is listed non-filtering | Tint requires depth images for comparisons; WebGPU binds a depth texture read plainly through a non-filtering sampler |
| A vector `OpSpecConstantOp Select` | Scalar selects and an `OpSpecConstantComposite` | WGSL overrides are scalars |
| `OpIsNan`, `OpIsInf` | Bit tests of the float's bits | WGSL has neither |
| `NonReadable` storage buffers | The decoration dropped | WGSL storage buffers are read or read-write |

Tint also gets `--allow-non-uniform-derivatives` (derivatives in
non-uniform control flow, as SPIR-V allows; it adds WGSL's own diagnostic
directive). The header lines (`// render.device.webgpu binding <group>
<binding> <kind> ...`, `draw-constants <bytes>`, `override <id> <type>`) come
from the WGSL declarations and tint's per-entry-point list of the bindings a
stage statically uses: a float texture the stage only loads is
`unfilterable-float`, so depth and 32-bit float textures bind to it.

Coverage at this slice: 57 of 58 rows (49 core programs and the 9 suite
fixtures). The one refused row is `cluster_assign_wgsl.h:kClusterBuildCompute`
(`'workgroupBarrier' must only be called from uniform control flow`): its
GLSL has a barrier in non-uniform control flow, which WGSL forbids; its
pipelines are refused on a WebGPU device by name.

#### The adapter

The model is the Metal adapter's: encoders record the port's shared command
lists (`render/device/recording.h`), Submit validates them and replays them
into one command buffer. What differs, and why:

- **Layouts.** WebGPU layouts need sample types, view dimensions, sampler
  kinds and storage formats the port's layouts do not carry. A pipeline's
  layouts come from its artifacts' binding lines (stages merged), cached by
  their entries and shared between pipelines; a port bind group becomes one
  WebGPU group per layout it is used with. An unused group in between is the
  empty layout and group.
- **Draw constants.** Group 3 gains binding 255, a uniform buffer with a
  dynamic offset into the submission's own constants buffer (one slot per
  change of the constants, at the device's uniform offset alignment). A
  draw group is built per submission for pipelines that read them.
- **Uploads.** `WriteBuffer` bytes travel in their commands into a
  submission buffer the queue writes before the command buffer runs, then
  are copied in command order. Each submission's buffers are freed with it.
- **Completion and readback.** Tokens advance as `Poll`, `IsComplete` and
  `WaitIdle` process WebGPU's events. A readback buffer is a GPU buffer and a
  `MapRead` copy: after each submission that names it, it is copied and
  mapped, and its bytes kept on the CPU. The token completes once the work
  is done and those maps landed, so `ReadBuffer` never waits, which a
  browser requires.
- **Copies.** WebGPU's row pitch is a multiple of 256 bytes: tightly packed
  regions whose rows are not go through a padded submission buffer, row by
  row. WebGPU copies no buffer into a depth texture: the region is drawn by
  an internal pipeline that writes `frag_depth` from the buffer's floats.
  A block-compressed texture whose first mip is not whole blocks (WebGPU
  requires it) is made whole blocks larger: copies keep the port's regions,
  and sampling it is refused by name. Writes and copies of partial words are
  refused by name.
- **Pipelines.** A graphics pipeline without a fragment stage gets one that
  writes none of its targets, so its color targets match the pass. Shader
  modules and pipelines are created under a validation error scope, so a
  refusal is a creation failure, not a later error.
- **Facts.** `kWgsl`; sample counts 1 and 4; claims compute, storage
  buffers, cube arrays and indexed indirect draws (one draw per record),
  indirect first instance and BC when Dawn has the features, float targets
  when float32-filterable, RG11B10 rendering and depth32float-stencil8 are
  all present. Not claimed: parallel recording, async queues, transient
  aliasing, ray query, external images, indirect count, timestamps between
  commands, exact occlusion counts, line fill, ETC1 and RGBA4. `kD24UnormS8`
  is refused (WebGPU's depth24plus copies to no buffer).

#### Evidence

Host: Fedora 44, AMD Radeon 8060S (RADV STRIX_HALO), Dawn picked its Vulkan
backend. g++ 16.

| Check | Result |
| --- | --- |
| `python3 tools/render/webgpu_lane.py run` | `CONFORMANCE 792 0`: the shared suite plainly and with validation errors counted (0), adapter clauses, and both bad configurations fail the suite (`webgpu.sensitivity.release-before-token`, `webgpu.sensitivity.complete-on-submit`) |
| `python3 tools/quality/conformance.py check --suite render.device.v2.webgpu` | pass, 792 checks |
| `python3 tools/render/shader_artifacts.py check` | `CONFORMANCE 1559 0` (before the WGSL ratchet) |
| `python3 tools/render/shader_artifacts.py sensitivity` | `CONFORMANCE 17 0`: the control passes and each of the four seeded faults, `wgsl-refusal` included, fails exactly its check |
| `python3 tools/render/shader_toolchain.py check` | `CONFORMANCE 355 0` |
| `WAFLOCK=.lock-waf-webgpu ./waf configure --tests ... --render-core-webgpu --render-core-device=webgpu -o build-webgpu`, then `./waf build --targets=render_device_webgpu,render_composition` | builds (strict C++20 module rules, link-dependency check) |
| `python3 tools/archlint/archlint.py check --all` | no finding for the new module (the run's 211 new findings are elsewhere, from `games/csgo/`) |
| `python3 tools/stylelint/stylelint.py <the new C++ files>` | 0 failures |

Reproduce: `python3 tools/render/webgpu_lane.py fetch`, then
`python3 tools/render/webgpu_lane.py run --out /tmp/claude-1000/webgpu-lane`.

#### Open after the first slice

- W3's browser half: done in the second slice, below.
- WGSL that Firefox rejects is caught only by W4's Firefox runs (no second
  validator, user decision).
- `kClusterBuildCompute` has no WGSL artifact (above).
- A depth texture sampled plainly through a filtering sampler, and a
  float texture both sampled and bound as depth, have no WebGPU layout; no
  core program needs either today, and a bind group that would is refused
  by name at submission.
- W4 (the render graph and the core pixel families on the adapter) and every
  other gate are open.

### The browser lane (W3, second slice, 2026-10-08)

`python3 tools/render/webgpu_lane.py browser [--adapter gpu|swiftshader]`
compiles the adapter and the same suite to WebAssembly and runs it in
headless Chrome:

- **Emscripten pinned:** `quality/toolchain/emscripten.json` pins emsdk
  6.0.10 (release `666337b5`, emcc `d6c521a7`), the newest release older than
  two weeks, by its archive's sha256. The lane installs and activates exactly
  that version under `dependencies/` and refuses an emcc that reports
  anything else. The browser's `webgpu.h` is the pinned Dawn release's
  emdawnwebgpu package, used as a local Emscripten port (the same
  `webgpu.h` core the native lane compiles against).
- **Threads and waits:** the build has pthreads with a preallocated pool of 8
  Web Workers (`-sPTHREAD_POOL_SIZE=8`) and a fixed 512 MB heap (RFC 0029
  decisions 6 and 9). It runs on the browser's main thread with JSPI
  (`-sJSPI`), so the adapter's `wgpuInstanceWaitAny` waits (pipeline
  creation's error scopes, `WaitIdle`, a readback copy still mapping) and the
  suite's waits for a token (`emscripten_sleep`) yield to the event loop,
  which delivers WebGPU's callbacks. The suite's encoder-thread clauses run
  on the worker pool. The adapter needed no change for the browser other
  than two fixes it found: a readback map was waited for by spinning (now its
  future is waited for), and the depth-upload pass left webgpu.h's NaN
  `depthClearValue`, which the browser refuses even when nothing is cleared.
- **Serving:** the lane serves the build with
  `Cross-Origin-Opener-Policy: same-origin` and
  `Cross-Origin-Embedder-Policy: require-corp` (SharedArrayBuffer), takes the
  page's output as it runs and its exit status at the end, and prints the
  output, so the suite's `CONFORMANCE` record reaches the conformance runner.
- **Adapters:** headless Chrome reaches the host GPU through ANGLE's Vulkan
  backend (`--use-angle=vulkan`); without it, it falls back to SwiftShader.
  A GPU run that reports SwiftShader fails (exit 3).
  `--adapter swiftshader` runs on SwiftShader on purpose
  (`--use-webgpu-adapter=swiftshader`), a lane that needs no GPU.

Evidence (Google Chrome 157.0.8089.0 canary, host software, not pinned):

| Run | Adapter | Claimed capabilities | Result |
| --- | --- | --- | --- |
| `webgpu_lane.py run` (native Dawn, Vulkan) | AMD Radeon 8060S (RADV STRIX_HALO) | compute, storage-buffers, texture-compression-bc, multi-draw-indirect, indirect-first-instance, cube-arrays, float-targets | `CONFORMANCE 792 0` |
| `webgpu_lane.py browser --adapter gpu` | Chrome: `amd rdna-3` | the same | `CONFORMANCE 792 0`, exit 0 |
| `webgpu_lane.py browser --adapter swiftshader` | Chrome: `google swiftshader` | the same | `CONFORMANCE 792 0`, exit 0 |

Manifest: `render.device.v2.webgpu.browser` (GPU) and
`render.device.v2.webgpu.swiftshader` (no GPU).

### Open after W3

- W0: the product profile `portal-wasm32-webgpu.json`, `--emscripten` in
  Waf, statically linked products as `.wasm`, and `static_composition.py`
  reading wasm. The pinned Emscripten and the browser harness exist now.
- W1–W2: the Node lane (foundation, jobs, physics), and Portal's headless
  boot in the engine worker with the null device.
- W4: the render graph and the core pixel families on the adapter, and
  Firefox (whose WGSL compiler is Naga; JSPI support there is unverified).
- The browser is not pinned; a pinned Chromium build would make the browser
  lane reproducible.
- `kClusterBuildCompute` still has no WGSL artifact.

## W0, W2 and W5 first slice: Portal in the browser through kiln (2026-10-08)

`./kiln play portal-wasm32-webgpu` builds the Emscripten product, serves it
(`tools/web/serve.py`) and opens Firefox on Wayland with hardware WebGPU;
`testchmb_a_01` renders at 1280x720 on the Radeon 8060S at about 37 fps,
the same image as the native Dawn lane (`portal-webgpu-core`). Commits
`91887397a`, `6eabeb23a`, `55882d3a7`, `288801e3c`.

- **Launch.** The `browser-page` run provider (kiln) starts the page server
  outside the display session, then the browser in it once the server
  listens; the run's status is the engine's exit status, which the page
  posts. `tools/web/browser_lane.py` is the same launch through `kiln.api`
  in kiln's private compositor (`--display private`), and copies the page
  console and captures out.
- **Browser.** Firefox, because Chrome on Linux exposes its Vulkan WebGPU
  adapter only with `--enable-features=Vulkan`, which it refuses with
  Wayland ozone ("not compatible with Vulkan"; hardware acceleration then
  off). On Wayland Chrome offers only SwiftShader or the GL ES
  compatibility adapter; on X11 ozone (Xwayland) it works.
- **WGSL portability.** The adapter writes specialization values into the
  WGSL as consts, and boolean `|`/`&` in module-scope bool declarations as
  `||`/`&&`: Firefox's naga refused the surface programs' derived overrides.
  `SOURCE_WEBGPU_DUMP_WGSL=<dir>` dumps every module; all 196 game modules
  validate on naga 26, 29 and 30; `render.device.v2.webgpu` 798/0 on Dawn.
- **Fixes on the way.** Range reads padded against byte-order-mark sniffing
  (UTF-16 close-caption tables halved the page's synchronous reads and spun
  the reader); the wasm profile on the core shader API (it still composed
  the null one); canvas sized by the presenter; `prc_GetNextN_t` returning
  void (a wasm indirect-call trap); kiln cancels runs on SIGINT/SIGTERM and
  the POSIX spawner's Terminate stops the whole process group.

- **Input (W5).** The page takes scripted DOM input from the harness
  (`SOURCE_WEB_PAGE_INPUT="<at s>:<key code>:<for s>,..."`, through
  `serve.py`); the engine's console reaches the page (spew to stderr in the
  browser, as on the 3DS). Holding S from 35 s moves the player from
  `setpos 608 32 64` to `622.9 16.0` (`getpos` polled by an alias loop),
  and Escape opens the pause menu over the live scene. The player spawns
  about 26 s after the page opens; keys sent before that do nothing.

- **Real input (W5).** `tools/web/compositor_input.py` plays input through
  mutter's RemoteDesktop API on kiln's private session bus, so the browser
  sees trusted events: a click grants pointer lock, relative motion turns
  the view (yaw 120.0 to 72.9), W walks the player along it (608, 32 to
  752, 171), the mouse turns back (104.6) and S walks back (734.6, -43.0),
  all from the default `kiln run portal-wasm32-webgpu`. (The session drops
  the first key press, so a throwaway Shift goes first.)
- **Audio is off in the browser by default** (`-nosound`; `--set sound` turns
  it on): once the page's AudioContext runs (Firefox after the first click,
  Chrome at once) the page deadlocks in SDL's Emscripten audio callback
  (`scriptProcessorNode.onaudioprocess`), which re-enters wasm while the
  engine's stack is suspended (JSPI). Pushing the mix from the engine's
  frame instead of SDL's callback did not change it; the wait is inside
  SDL's port and is open.

Open: audio (above); audio in a page without a user gesture;
load time (map in 14 s, about 2,400 lazily read files) and frame time; the
Node lane and W1/W4/W6.
