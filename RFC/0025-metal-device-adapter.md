# RFC 0025: Metal Device Adapter for the Render Core

- Status: Proposed (2026-10-07); first slice implemented, no gate passed.
- Date: 2026-10-07
- User direction (2026-10-07): "add a metal device adapter and rendercore
  backend".
- Render architecture: [RFC 0016](0016-render-core.md) owns
  `render.device.v2`, its conventions, capability negotiation, shader
  artifacts and the binding rules. This RFC adds an adapter and one artifact
  format; it changes no port clause and no portable module.
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  Q-PRESENTATION. The shared `render.device.v2` suite and its bad adapters
  are the oracle, as for Vulkan (K1), OpenGL (K10) and Direct3D 12
  ([RFC 0024](0024-direct3d12-device-adapter.md)).
- Tracking: no ranked row. A sibling of R92 (K10) and RFC 0024, a provider
  for R97 (device switching), and the native path for R29/R36's Apple
  targets. Ranking it is the user's decision.

## Why

macOS, iOS and tvOS reach the GPU today only through MoltenVK, which
implements a Vulkan portability subset over Metal and translates every
SPIR-V module at pipeline creation (AGENTS.md: "query and validate the
required feature profile and shader translations, rather than claiming
full desktop Vulkan equivalence"). A native Metal adapter removes that
layer for the core: shaders are translated once at build time with the
pinned SPIRV-Cross, bind groups map to Metal argument buffers directly, and
Apple's own limits (16 samplers per stage, no 24-bit depth) are named by the
adapter instead of hidden in a translation layer.

"Backend" means the core's adapter. It does not port the frozen legacy
backend (binding rule 1); the legacy stream reaches Metal only once R91
retires it, as for GL and D3D12. MoltenVK remains the Apple path of the
legacy host until then.

## Decisions (agent decisions under the user's standing instruction)

1. **Module.** `render.device.metal` in `render/device/metal/` (Objective-C++
   with ARC, `.mm` only) with `public/render/device/metal/provider.h`,
   provider name `"metal"`, in the composition's provider catalog under
   `RENDER_CORE_METAL`. Declared in `architecture/modules.json` in the adapter
   column; no Metal or Objective-C type appears in a port header. Waf builds
   it on every Apple target (`--apple-sdk`); `--render-core-device=metal`
   selects it as a product default.
2. **Device floor.** `MTLGPUFamilyMetal3` and argument buffers tier 2:
   Apple-silicon Macs, A13 and later, macOS 13 / iOS 16. Older devices fail
   creation with `kUnsupported`.
3. **Shaders.** New `ArtifactFormat::kMsl`: MSL 3.0 from the same SPIR-V by
   the pinned SPIRV-Cross (`shader_artifacts.py metal_compile`), one text for
   macOS and iOS (SPIRV-Cross writes identical MSL for both; checked on all
   core programs). The runtime compiles MSL source with
   `newLibraryWithSource`; precompiled `.metallib` needs Apple's `metal` tool
   and is a later build-time step (see M1).
   - Each bind group is one argument buffer at `[[buffer(group)]]`; member
     `[[id(n)]]` is binding n; every declared resource is emitted
     (`--msl-force-active-argument-buffer-resources`).
   - Discrete bindings were rejected: the surface program's fragment stage
     needs 22 samplers, and Metal's discrete table has 16.
   - Vertex buffer slot s is buffer 16+s; draw constants (D16) are
     `setBytes` at buffer 30; specialization constants (D20) are function
     constants with their types on a header line; a compute stage's
     threadgroup size is on another.
   - A row that does not translate is left out of the MSL header and the
     store, named in a comment, and its pipelines are refused by name.
     Today one: `kSkinCompute` (runtime array length needs SPIRV-Cross's
     buffer-size buffer). Products declare `skinning=skinning-cpu`.
4. **Bind groups.** A group is encoded once per (pipeline, stage) through
   that stage function's own `MTLArgumentEncoder`, so the adapter carries no
   layout rule of its own, and kept until the group or pipeline is released.
   Resources an argument buffer names are declared with `useResource` once
   per Metal encoder. A layout array of count c at binding n takes ids
   n..n+c-1; layouts where that overlaps a later binding are refused.
5. **Recording and submission.** As the GL adapter: CPU command lists on any
   thread, validated at Submit against usage state, replayed into one
   `MTLCommandBuffer` per submission on one queue. Usage transitions are no
   work (automatic hazard tracking). Tokens complete in order through
   completion handlers. Device loss: access-revoked/not-permitted errors
   (and device-removed on macOS) report `kLost`; `Recover()` starts a new
   epoch on a new device object.
6. **Memory.** Device-local buffers and all textures are private storage;
   upload and readback buffers and the upload ring are shared (unified
   memory). The memory budget reports `currentAllocatedSize` against
   `recommendedMaxWorkingSetSize`.
7. **Conventions.** Metal's own match the port (clip depth 0..1, clip +Y up,
   row 0 at the top); winding passes through unflipped.
8. **Capabilities claimed.** Compute, storage buffers, multi-draw indirect
   (one Metal indirect draw per record; the record layout is
   `MTLDrawIndexedPrimitivesIndirectArguments`), indirect first instance,
   cube arrays, and BC where `supportsBCTextureCompression`. Not claimed:
   timestamps (Apple GPUs sample counters only at stage boundaries),
   indirect count, async queues, parallel recording, transient aliasing,
   external images, ray query.
9. **Shared recording (debt).** The command list, its validator and the
   upload ring follow `render.device.gl` line for line. They move into one
   `render.device` helper used by GL, Metal and D3D12 when RFC 0024 needs the
   same; deletion condition: a second copy lands.

## Gates

| Gate | Done looks like | State |
| --- | --- | --- |
| M0 | Module, provider, Waf target and composition entry; `kMsl` artifacts for every core program or a named refusal; adapter compiles for iOS and macOS; archlint clean for the module | partial: all but a Waf build of an Apple product with the adapter (2026-10-07) |
| M1 | Every MSL artifact compiles with Apple's `metal` (macOS VM, offline); optional `.metallib` precompile | open (VM unavailable 2026-10-07) |
| M2 | Shared `render.device.v2` suite and bad adapters pass on an Apple-silicon Mac and the iPhone 16 Pro (Metal API validation silent), including conventions, D16, D20, D24, D30 | open |
| M3 | The five core pixel families within the recorded cross-backend tolerance of Vulkan/MoltenVK; `render.graph.v1` on the adapter | open |
| M4 | SDL3–Metal presentation bridge (CAMetalLayer, EDR from R16-DYNAMIC-RANGE) passes the shared presentation suite; `render_lab` presents through it on iPhone and Apple TV | open |
| M5 | Resolution-sweep frame times against MoltenVK on the same device | open |

## Evidence (2026-10-07)

- `tools/render/shader_artifacts.py headers` writes 17 `*_msl.h` headers;
  the store has 45 `kMsl` entries, `kSkinCompute` refused by name;
  `core_artifacts.cpp` compiles against the generated table.
- The four `.mm` files compile with `-Wall -Wextra -Werror` for
  `arm64-apple-ios17.0` (iPhoneOS 26.5 SDK) and `arm64-apple-macos13.0`
  (MacOSX 26.5 SDK) with the pinned LLVM 22.1.8.
- `archlint check --all`: no finding in the new module; the run's failures
  are pre-existing (`games/csgo`, `external/`).
- `tools/render/tests/test_shader*`: 33 of 36 pass; the 3 failures are
  identical at HEAD without this change.
- Not run: any Metal device, Apple's MSL compiler, a full Apple product
  build, any suite. No support claim follows.
