# RFC 0028: Direct3D 9 Device Adapter for the Render Core

- Status: Proposed (2026-10-07); nothing implemented, no gate passed.
- Date: 2026-10-07
- User direction (2026-10-07): "let's unrule it out, we should have DX9 the
  same way we have the [D3D12] + metal ports". This lifts RFC 0016's
  exclusion of a native D3D9 adapter and of running mod shader DLL bytecode
  on the core.
- Render architecture: [RFC 0016](0016-render-core.md) owns
  `render.device.v2`, its conventions, capability negotiation, shader
  artifacts and the binding rules. This RFC adds an adapter, one artifact
  format, and one legacy-frontend path for mod bytecode. It changes no port
  clause.
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  Q-PRESENTATION. The shared `render.device.v2` suite and its bad adapters
  are the oracle, as for Vulkan (K1), OpenGL (K10), Direct3D 12
  ([RFC 0024](0024-direct3d12-device-adapter.md)) and Metal
  ([RFC 0025](0025-metal-device-adapter.md)).
- Tracking: no ranked row. A sibling of R92, RFC 0024 and RFC 0025, and a
  provider for R97 (device switching). Ranking it is the user's decision.

## Why

The core runs on Vulkan, GL/GLES, D3D12 and Metal, while the D3D9 path is
still the frozen `shaderapidx9` backend beside the core (native on Windows,
DXVK Native on Linux, ToGL on the SDL2 legacy profiles). Two things follow:

- Mod shader DLLs (`ShaderDLL004`) ship D3D9 bytecode, which no core adapter
  runs, so on core profiles their materials follow the missing-shader rule.
- `shaderapidx9` and `materialsystem/stdshaders/` are a second render stack
  that RFC 0016's "old copy deleted" rule cannot retire while it is the only
  way to run D3D9.

A D3D9 adapter makes D3D9 one more provider under the same port: the core
draws through it, mod bytecode runs on it, and the legacy backend becomes a
candidate for deletion once the adapter and the frontend carry its users.

## Decisions (agent decisions under the user's standing instruction)

1. **Module.** `render.device.d3d9` in `render/device/d3d9/` with
   `public/render/device/d3d9/provider.h`, provider name `"d3d9"`, in the
   composition catalog under `RENDER_CORE_D3D9`, adapter column of
   `architecture/modules.json`. No `d3d9.h` type in a port header.
2. **API floor.** `IDirect3D9Ex`/`IDirect3DDevice9Ex`, shader model 3.0.
   9Ex removes the managed-pool/device-lost reset dance (lost becomes
   `kLost` and `Recover()`), gives shared surfaces for presentation, and is
   what DXVK implements. Plain `IDirect3D9` fails creation with
   `kUnsupported`.
3. **Lanes.** Windows native (MSVC runner, optional per AGENTS.md); Linux
   through DXVK Native (the pinned DXVK SDK the legacy profiles already use)
   and MinGW + Wine, as RFC 0024's X-lane. The Linux lanes prove the
   adapter's contract, not Windows driver behavior.
4. **Shaders.** New `ArtifactFormat::kD3d9Bytecode`: SM 3.0 bytecode built
   from the same SPIR-V by the pinned SPIRV-Cross HLSL backend
   (`--shader-model 30`) and compiled by the pinned FXC already used by
   `legacy_shader_conformance.py`. A program that does not translate (SM3
   limits: 224 ps constants, 16 samplers, no integer ops, no storage
   buffers) is left out of the store and refused by name.
5. **Bind groups.** Emulated: each group's bindings map to fixed constant
   register and sampler ranges recorded in the artifact header; the
   adapter uploads them at draw. Groups needing more than SM3 offers are
   refused at layout creation.
6. **Capabilities claimed.** Graphics only: no compute, storage buffers,
   indirect draws, timestamps beyond `D3DQUERYTYPE_TIMESTAMP` (claimed if
   the query works), cube arrays, async queues or parallel recording. BC1–3
   only. The core's passes that need more (clustered assignment, GPU
   culling and skinning, RPRB cube arrays, the full surface program's 22
   samplers) refuse on this adapter by name or take their declared CPU or
   reduced fallbacks. Per AGENTS.md's LSP rule the adapter claims a narrower
   capability set; it never pretends to the full lighting model. RFC 0003's
   GPU placement rule is unaffected: on this adapter the GPU path does not
   exist, so the CPU path is not a convenience default.
7. **Recording and submission.** The shared `render.device` recording
   helper (RFC 0025 decision 9); replay on the device's owning thread
   (D3D9 is single-threaded); tokens complete in order through event
   queries.
8. **Conventions.** Clip depth 0..1 matches the port; the half-pixel offset
   and clip-space Y are applied in the generated vertex epilogue, so passes
   see the port's conventions unchanged.
9. **Mod bytecode.** The legacy frontend gains one path, active only when
   the selected adapter claims `kD3d9Bytecode`: a `ShaderDLL004` shader's
   own vertex/pixel bytecode is passed through as a pipeline artifact, with
   its D3D9 constant and sampler registers bound directly. On every other
   adapter the missing-shader rule stays. No bytecode translation to other
   APIs is in scope.
10. **Legacy backend retirement.** Once D9-5 passes, `shaderapidx9` and its
    DXVK profiles move onto the core with this adapter, and the legacy
    backend is deleted in that change (binding rule 3). ToGL's retirement
    remains a separate decision about the SDL2 legacy profiles.

## Gates

| Gate | Done looks like | State |
| --- | --- | --- |
| D9-0 | Module, provider, Waf target, composition entry; `kD3d9Bytecode` artifacts for every translatable core program, the rest refused by name; archlint clean | open |
| D9-1 | Shared `render.device.v2` suite and bad adapters pass for every claimed capability on DXVK Native and Wine (D3D9 debug runtime or DXVK validation silent), conventions section included | open |
| D9-2 | `render.graph.v1` on the adapter; capability negotiation selects declared fallbacks or fails composition by name for every core pass | open |
| D9-3 | SDL3–D3D9Ex presentation bridge passes the shared presentation suite (resize, loss, zero size) | open |
| D9-4 | The core pixel families within recorded cross-backend tolerance of Vulkan for every family the adapter claims | open |
| D9-5 | Legacy frontend on the adapter: material pixel families and the 268-case legacy shader set match the `shaderapidx9` references; a mod `ShaderDLL004` fixture draws through the bytecode path with a seeded register-binding defect caught | open |
| D9-6 | Resolution-sweep frame times against `shaderapidx9` on the same host; old backend deleted | open |
