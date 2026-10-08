# RFC 0029: WebAssembly Platform and WebGPU Device Adapter

- Status: Proposed (2026-10-07). W3 done 2026-10-08
  ([progress](0029-progress.md)): the WebGPU adapter passes the shared
  device suite natively on the pinned Dawn and as WebAssembly in headless
  Chrome; Emscripten is pinned. W0–W2 and W4–W6 are open.
- Date: 2026-10-07
- User direction (2026-10-07): "let's make this an rfc to use webgpu and
  wasm", after a feasibility discussion and "can we use multiple web
  workers based on the job system?".
- Render architecture: [RFC 0016](0016-render-core.md) owns
  `render.device.v2`, its conventions, capability negotiation, shader
  artifacts and the binding rules. This RFC adds one adapter and one
  artifact format. It changes no port clause.
- Platform architecture: [RFC 0001](0001-capability-based-platform-architecture.md)
  owns composition and providers; the browser is one more profile with
  static first-party composition, as iOS is.
- Jobs: [RFC 0003](0003-dependency-aware-job-system.md) owns the executor,
  the serial oracle and low-capacity mode; this RFC only sizes the pool.
- Content: [RFC 0015](0015-asset-identity-content-build-graph.md) owns
  packages and the resolver; the browser adds a fetch/OPFS package source.
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md).
  The shared `render.device.v2` suite and its bad adapters are the render
  oracle, as for Vulkan (K1), OpenGL (K10), Direct3D 12
  ([RFC 0024](0024-direct3d12-device-adapter.md)), Metal
  ([RFC 0025](0025-metal-device-adapter.md)) and Direct3D 9
  ([RFC 0028](0028-direct3d9-device-adapter.md)).
- Tracking: no ranked row. A fifth target beside the four north-star
  platforms, not one of them; it delays none of their gates. Ranking it is
  the user's decision.

## Why

The engine already has the pieces a browser build needs most: static
first-party composition with typed factories and a link-map check
(R29-IOS-STATIC), SDL3 for window, input and audio (SDL3 has an Emscripten
backend), a backend-neutral render core with a shared adapter suite, and a
job system whose providers borrow one root-owned pool and whose serial
graph is a required mode. What is missing is a graphics API the browser
offers, a main loop that never blocks the page, content that is not
preloaded, and a network transport without UDP.

WebGL2 is not a target: it is OpenGL ES 3.0, without compute, storage
buffers or indirect draws, so it cannot run clustered Forward+, GPU
skinning or GPU culling. RFC 0022's ES 3.1 preset does not help. WebGPU has
all three, and its model (submission completion, bind groups, immutable
pipelines) maps onto `render.device.v2` directly.

## Decisions (agent decisions under the user's standing instruction)

1. **Toolchain.** Emscripten, pinned by version in a new product profile
   `quality/product_profiles/portal-wasm32-webgpu.json` (compiler, sysroot,
   linker flags, `INITIAL_MEMORY`, pool size, required browser features).
   The target is `wasm32`; `wasm64` (Memory64) is a later profile only if a
   measured scene does not fit (decision 9). Waf remains the build entry
   point (`--emscripten`), as `--apple-sdk` is for Apple.
2. **Composition.** Static, exactly as iOS: `--static-composition`, no
   `dlopen`, `static_composition.py check` on the final `.wasm` link map.
   The checker gains a wasm reader; its seeded defects run for ELF,
   Mach-O and wasm.
3. **Device.** `render.device.webgpu` in `render/device/webgpu/` with
   `public/render/device/webgpu/provider.h`, provider name `"webgpu"`,
   catalog entry under `RENDER_CORE_WEBGPU`, adapter column of
   `architecture/modules.json`. It is written against the standard
   `webgpu.h` C API, so the same adapter also builds natively against
   Dawn or wgpu-native for a desktop lane (decision 11). No `webgpu.h` type
   reaches a port header.
4. **Mapping.** Completion tokens are `onSubmittedWorkDone` futures
   counted per queue submission; bind groups are WebGPU bind groups (four,
   inside the default `maxBindGroups` of 4); transient pools follow WebGPU
   buffer and texture usage flags. Features beyond core WebGPU
   (`timestamp-query`, `float32-filterable`, BC/ASTC/ETC2 compression,
   `shader-f16`) are capabilities negotiated per device and refused by
   name when absent; nothing silently falls back.
5. **Shaders.** New `ArtifactFormat::kWgsl`: WGSL generated from the same
   SPIR-V by Tint, pinned through the Dawn revision of the native lane
   (decision 11) and built as a host tool. SPIR-V stays the one
   intermediate; `tools/render/shader_toolchain.py` stays the one owner of
   which translator serves which format. Tint is not adopted for other
   backends: its SPIR-V reader accepts only what WGSL expresses, which would
   cap every backend at WebGPU's feature set (bindless, push bindings,
   specialization constants, subgroups), and SPIRV-Cross (GLSL/ESSL, MSL,
   HLSL) plus FXC (D3D9) already pass their adapters' suites. A program
   that does not translate is left out of the store and refused by name on
   this device only; core shader source is not reduced to WebGPU's level.
   No second WGSL validator is pinned (user decision, 2026-10-07): WGSL
   that Firefox rejects is caught by W4's Firefox runs, not at build time.
   SPIRV-Cross has no WGSL backend and browsers accept no SPIR-V, so a WGSL
   translator is required; Tint is the only one.
6. **Threads.** Emscripten pthreads (Web Workers sharing one
   `SharedArrayBuffer` heap). The root creates the `CmpJob` pool before the
   engine starts, sized from `navigator.hardwareConcurrency` through the
   existing `-compute_workers`, and preallocated (`PTHREAD_POOL_SIZE`)
   because worker start-up is asynchronous; no product thread is created
   lazily. The R94 thread census applies unchanged.
7. **Main loop.** The whole engine runs in a dedicated worker rendering to
   an `OffscreenCanvas`, so the job system's "main thread" may block. The
   page thread only forwards input, focus, resize and lifecycle events.
   Asyncify is not used.
8. **GPU ownership.** WebGPU objects belong to the thread that created
   them, so one render thread owns the device, encodes and submits.
   Culling, sorting, CPU skinning, draw lists and per-draw data are built
   on the pool; only the adapter's encode step is serial. RFC 0016's K9
   "recording scales" goal is met on this profile by the work before
   encoding, and the GPU-driven submission phases (S0–S8) are its main
   draw-count lever.
9. **Memory.** Fixed `INITIAL_MEMORY` per profile, no growth with threads
   unless measured cheaper. Portal is the first product; Portal 2 joins
   when its measured working set fits `wasm32`.
10. **Without isolation.** If the page is not cross-origin isolated (no
    `SharedArrayBuffer`), the same build runs the serial executor with zero
    workers and reports the reduced mode; it never fails silently and never
    claims the threaded budgets.
11. **Lanes.** Headless: Node with the null device for foundation, jobs
    and physics suites. Browser: Chromium and Firefox under a pinned
    Playwright, headless with WebGPU enabled, for the device suite and
    pixel families. Native: the same adapter on Dawn for fast iteration;
    native passes do not certify the browser.
12. **Content.** A `package source` (RFC 0015) that fetches archive
    ranges on demand and caches them in OPFS, read synchronously from the
    engine worker through sync access handles. The user supplies the game
    content; nothing in this RFC redistributes it, and the repository's
    provenance warning stands.
13. **Networking.** Single-player with the in-process listen server first.
    Multiplayer needs a WebSocket or WebRTC net-channel transport and a
    relay; it is out of scope until a later decision.
14. **SIMD.** `-msimd128` with Emscripten's SSE headers for mathlib and
    tier0; a scalar build remains a declared fallback profile.
15. **Audio and video.** SDL3 audio on WebAudio. Bink stays unsupported
    on this profile (as on iOS) and is refused by name.

## Gates

| Gate | Done looks like | State |
| --- | --- | --- |
| W0 | Profile declared and pinned; `--emscripten` configure; dedicated/test products link statically to `.wasm`; `static_composition.py` reads wasm with seeded defects; archlint clean | open |
| W1 | Node lane: foundation, `platform.task-runner.v1`, jobs (serial and pooled, `jobsystem.continuous`) and `physics.conformance` suites pass with their counts; thread census matches the declared pool; serial mode passes without `SharedArrayBuffer` | open |
| W2 | Headless Portal boots a map in the engine worker with the null device and exits cleanly; content through the OPFS package source; missing content named | open |
| W3 | `render.device.webgpu`: shared `render.device.v2` suite and bad adapters pass for every claimed capability on Dawn and in headless Chromium, conventions section included; `kWgsl` artifacts for every translatable core program, the rest refused by name | done (2026-10-08, [record](0029-progress.md)): 792 checks pass natively on the pinned Dawn and as WebAssembly in headless Chrome on the GPU and on SwiftShader (Chrome host software, not pinned); 57 of 58 rows translate, `kClusterBuildCompute` refused by name |
| W4 | `render.graph.v1` on the adapter; core pixel families within recorded cross-backend tolerance of Vulkan in Chromium and Firefox | open |
| W5 | Portal playable in a browser on the core: input, audio, resize, focus loss and tab backgrounding; matched captures against native Vulkan | open |
| W6 | Resolution sweep and frame-floor runs against native Vulkan on the same host, threaded and serial; budgets for the profile set before measuring | open |

## Out of scope

WebGL2; multiplayer transport (decision 13); Hammer and the tools;
executable updates or plugin download; redistributing game content; any
claim about mobile browsers until a device lane exists.

## Open questions

- Whether Portal 2 fits `wasm32` or needs a Memory64 profile.
- Hosting: the COOP/COEP headers and range requests the page host must
  serve, and who owns that deployment.
