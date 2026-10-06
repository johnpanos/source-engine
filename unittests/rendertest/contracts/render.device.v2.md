# Contract: `render.device.v2`

Module: `render.device` (port; RFC 0016 layer 1)
Contract headers: `public/render/device/` (`device.h`, `encoder.h`, `usage.h`,
`completion.h`, `resources.h`, `pipeline.h`, `bind_group.h`, `facts.h`,
`errors.h`, `conventions.h`, `validation.h`, `provider.h`)
Shared suite: `unittests/rendertest/core/device/device_conformance.h`
Adapters: `render.device.null` (`test_device_null.cpp`),
`render.device.vulkan` (`test_device_vulkan.cpp`, GPU runner),
`render.device.gl` (`test_device_gl.cpp`, GPU runner, RFC 0016 K10).
Raster clauses (`RunRasterConformance`: ordered dispatches, texel centers, a BC1
block decoded as DXT1, a 4x resolve with back faces culled) run on every
adapter that executes work.
Sensitivity: `unittests/rendertest/core/device/test_device_negative.cpp`;
`test_device_vulkan.cpp` and `test_device_gl.cpp` built with their
`RENDER_DEVICE_*_SENSITIVITY` define.
Rows: R86 (RFC 0016 K1), R92 (K10)

The backend-neutral device port. Portable render code records work through it;
only `render.composition` and test fixtures name an adapter (archlint CAP011).
No graphics API type, stage mask, access mask or image layout appears in it.

## Clauses

Check names in the suite are `<driver>.<clause> <what>`.

| Clause | Obligation | Bad adapter that must fail it |
| --- | --- | --- |
| D1 | Facts are immutable, report exactly four bind groups, single-sample support, usable limits and a diagnostic backend name | facts change after first use |
| D2 | Invalid descriptions fail with `kInvalidDescription` and leave nothing live | creates a buffer before rejecting a texture |
| D3 | More than four bind-group layouts fail with `kTooManyBindGroups` | silently keeps the first four |
| D4 | A reflected binding missing from the layouts fails with `kLayoutMismatch`; a foreign artifact format with `kUnsupported` | strips reflection before creating |
| D5 | `Release` never frees before its token completes; `Poll` frees after; a released handle is never valid again. Work submitted before the release still runs | frees at once, ignoring the token |
| D6 | Submissions on one queue complete in order; values rise by one per submission within an epoch | reports a later token complete first |
| D7 | After a device loss, `Submit` rejects waits on old-epoch tokens with `kStaleEpoch`; recovery starts a new epoch with nothing live | drops old-epoch waits (skipped, and reported, where loss cannot be forced) |
| D8 | Encoders run in `Submit` order; a submission with an erroring encoder fails with `kInvalidState` and none of it runs | drops erroring encoders and runs the rest |
| D9 | Written, copied and cleared bytes read back unchanged | — (covered by D8, D10) |
| D10 | Upload ranges are reused only after their token completes; a full ring defers | retires ranges at submission (on GL and on Vulkan, `unsafeUploadReuse`; both suites run D10 and D5 with submissions held, so the defect is observable every run) |
| D11 | Recording one encoder from a second thread is diagnosed (`SequenceViolations`); the check lives in the port | — (port-owned) |
| D12 | Transitions name the resource's current usage and a usage it was created with; clears, copies and attachments need their usage. The current usage is the state after every accepted submission, complete or not: a submission may continue from one still running | accepts transitions from any usage |
| D14 | Distinct encoders may be recorded concurrently, one thread per encoder, and submitted together; recording shares the upload ring safely and their bytes land | — (TSan lane of the pooled graph executor) |
| D16 | Draw constants: a pipeline declares a block of at most 128 bytes (a multiple of four) that its stages read, and a stage reflecting more fails `kLayoutMismatch`; binding a pipeline leaves the block undefined, and a draw or dispatch with any word unset since then, or a write outside the block, fails the submission (`kInvalidState`); on rasterizing adapters each draw sees its own constants. Vulkan: push constants; OpenGL: a uniform block | zero-fills the block when a pipeline is bound |
| D17 | Color write masks: a pipeline may give one `kColorWrite*` mask per color format (empty writes every channel); a count other than the formats' or a mask outside the four channels fails `kInvalidDescription`; on rasterizing adapters the channels a mask leaves out keep what the attachment held. Vulkan: `colorWriteMask`; OpenGL: `glColorMaski`. Needed by material families whose ports leave destination alpha (RFC 0016 K4) | Vulkan sensitivity knob `ignoreColorWriteMasks` (writes every channel) |
| D18 | External images (`external_images.h`): `IRenderDevice2::ExternalImages()` is non-null exactly when the facts claim `kExternalImages`; `CreateTexture` refuses `kExternal` (`kInvalidDescription`); `CreateExported` makes a 2D texture of one mip and layer whose usages include `kExternal`, in `kRGBA8Unorm`, `kRGBA8Srgb`, `kBGRA8Unorm` or `kBGRA8Srgb` (another format fails `kUnsupported`, a description outside the rules `kInvalidDescription`), and returns a handle and one plane's description (a dmabuf fd, DRM fourcc and modifier, offset, stride; opaque integers). After a submission whose last use is `kExternal` completes, the memory read through the description equals what the port reads back from the texture. Vulkan: LINEAR (`DRM_FORMAT_MOD_LINEAR`) images in dedicated exportable memory, host-visible where a device-local type allows, `kExternal` is `GENERAL` | Vulkan sensitivity knobs `staleExport` (the export names other memory) and `nullExternalImages` (claims the capability, exports nothing) |
| D19 | Block-compressed formats (`kBC1Unorm`…`kBC7Srgb`, 4x4 blocks: D3D9's DXT1/DXT3/DXT5, ATI1N, ATI2N, and BC6H/BC7, which the suite also round-trips): a device without `Capability::kTextureCompressionBC` fails such a texture `kUnsupported`; with it, a texture takes only `kSampled`, `kCopySource` and `kCopyDestination`, one sample and no volume (else `kInvalidDescription`); a region takes whole blocks (`RegionBytes`); a copy covers whole blocks or reaches the mip's edge, at a buffer offset that is a multiple of the block's bytes, and a clear is refused (else the submission fails `kInvalidState`); two mips copy in and back unchanged. On rasterizing adapters a BC1 block decodes as D3D9's DXT1: its three-color mode keeps the one-bit alpha (Vulkan: `BC1_RGBA`) | a bad adapter that creates a block-compressed attachment |
| D20 | Specialization constants (`PipelineDesc::constants`, `SpecializationConstant{stage, id, value}`, 32-bit values): a stage takes each id at most once (else `kInvalidDescription`); an id the stage does not declare is ignored. On rasterizing adapters the value reaches the shader: `specialized.frag` draws red by default and green with constant 7 at 1. Adapters translate them to their API (Vulkan: `VkSpecializationInfo`; a GL adapter through SPIRV-Cross's specialization). The material families' permutations are specialization constants, so a neutral term costs nothing at runtime | the Vulkan adapter with its specialization info dropped fails the green draw |
| D21 | Transmittance blending (`BlendMode::kTransmittance`): color is src + dst * a and the destination's alpha is kept, so a source carrying a transmittance in alpha (a participating medium's, RFC 0016) applies it at the target's own precision, where `kPremultiplied`'s 1 - a loses a small transmittance to the format. On rasterizing adapters, over a half-float target cleared to (0.8, 0.6, 0.4, 0.25), a draw of (0.01, 0.02, 0.03, 0.05) gives (0.05, 0.05, 0.05, 0.25) within 2^-10 relative. Vulkan: color ONE, SRC_ALPHA; alpha ZERO, ONE. OpenGL: `glBlendFuncSeparatei( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_ONE )` | sensitivity knob `transmittanceAsPremultiplied` (Vulkan and OpenGL: the mode drawn as `kPremultiplied`) fails the pixel check |
| D22 | Region copies (`TextureBufferCopy::x, y`): a buffer-to-texture copy at (x, y) writes that rectangle alone, a texture-to-buffer copy at (x, y) reads it back, and a region past the mip's edge (or off a block boundary of a compressed format) fails its submission with `kInvalidState`. Vulkan: `imageOffset`. OpenGL: the offsets of `glTextureSubImage2D` and `glGetTextureSubImage` | bad adapter `kDropsRegionOrigin` (every region copied to (0, 0)) fails the placement check |
| D23 | Timestamps (`Capability::kTimestamps`, `CommandEncoder::WriteTimestamp`): the GPU's time after the commands recorded before it, in ticks of `DeviceFacts::timestampPeriodNs` (set exactly when the capability is claimed), lands as 64 bits in a `kReadback` buffer in `kCopyDestination` (here and at the end of the submission; offset a multiple of 8) once the submission completes. Allowed inside rendering. Timestamps do not decrease in recording order, and a later submission's are no earlier. Device-local memory or an unaligned offset fails with `kInvalidState`; without the capability the submission fails with `kUnsupported`. Vulkan: a query pool per submission, reset at its start, `vkCmdWriteTimestamp` at bottom of pipe, copied to the buffer at its end. OpenGL: `glQueryCounter` and a query buffer object write (`glGetQueryBufferObjectui64v`), nanosecond ticks. Null: a clock that advances 10 ticks per command | bad adapter `kDropsTimestamps` (records none) fails the landing check |
| D24 | `SamplerDesc::comparison` is absent for ordinary sampling or holds the depth comparison operation. The reference is the left operand; each texel is compared before filtering. Nearest samples one result; linear sampling interpolates the four comparison results, and address modes govern the footprint. D32 tests distinguish less, less-equal and greater at equality, half-texel positions and clamped edges. Invalid operations fail creation with `kInvalidDescription` before allocating. Vulkan uses sampler comparison state; OpenGL uses `GL_COMPARE_REF_TO_TEXTURE`. Resource compatibility remains the caller's obligation: comparison sampling uses a depth texture | null decorator accepts an invalid comparison; Vulkan and GL `reverseSamplerComparison` reverse less-equal and fail the analytical pixel check |
| D30 | Multi-draw indirect (`Capability::kMultiDrawIndirect`, `CommandEncoder::DrawIndexedIndirect`): `drawCount` indexed draws whose `DrawIndexedIndirectCommand` records (20 bytes) the GPU reads from a buffer in `kIndirect`, everything else bound as for `DrawIndexed`; offset and stride multiples of 4, stride at least 20, every record inside the buffer (`IndirectRecordsFit`, the one rule every adapter uses), else the submission fails `kInvalidState`; a draw count of zero draws nothing; unclaimed, the submission fails `kUnsupported`. Rasterizing adapters: two records draw both halves of the target | draws the records that fit instead of failing |
| D31 | Indirect count (`Capability::kDrawIndirectCount`, `CommandEncoder::DrawIndexedIndirectCount`): as D30, with the draw count a 32-bit value the GPU reads at a 4-aligned offset of a buffer in `kIndirect` (it may be the records' buffer), clamped to `maxDrawCount`, whose records must all fit; a count outside its buffer fails `kInvalidState`; unclaimed, `kUnsupported`. Rasterizing adapters: a GPU count of 1 of at most 2 draws only the first record. Claimed on Vulkan where the device has `multiDrawIndirect` and 1.2's `drawIndirectCount`; the GL and ES adapters claim neither yet | reads the count at offset 0 whatever was asked |
| D13 | Conventions on real pixels: clip depth 0 to 1, clip Y up, row 0 at the top (rasterizing adapters only) | not yet: a flipped-Y and a −1..1-depth adapter need the GPU lane |

## Vulkan host interop (private to the Vulkan family)

`render/device/vulkan/host_device.h` is not part of the port: only the
Vulkan adapter, the legacy native Vulkan backend and their suites use it
(CAP007). Its clauses run in `test_device_vulkan.cpp` as `vulkan.host`
(memory, completion, host work inside port encoders, binary semaphores) and
`vulkan.import` (RFC 0016 K5): `ImportImage` makes a host image a port
texture with a home usage. Host work in the same encoder finds it in its home
usage (a submission whose host work or end finds it elsewhere fails with
`kInvalidState`); the port reads what host work wrote before it and host work
reads what the port wrote; releasing the texture frees only the adapter's
views. Synchronization validation, enabled on the host instance whenever the
layer is, reports nothing, and a host that skips its own barrier is reported.
The legacy backend's depth-stencil format is `kD24UnormS8` or `kD32FloatS8`
(RADV has no D24S8).

## OpenGL adapter (render.device.gl, K10)

- **Artifacts:** GLSL 4.50 (`ArtifactFormat::kGlsl450`), cross-compiled at build
  time from the SPIR-V with the pinned SPIRV-Cross; the form is owned by
  `tools/render/shader_artifacts.py` `cross_compile`. Binding (group, b) is GL
  slot `group * 16 + b` of its kind; a sampled texture and a sampler are named
  `rg_t<g>_<b>` and `rg_s<g>_<b>`, so each combined sampler SPIRV-Cross builds
  sits on its texture's slot and names its sampler, which the adapter binds to
  that unit. The suite's fixtures are the generated
  `spv/device_fixtures_glsl.h` (`DeviceDriver::artifact` maps each).
- **Selection:** consumers take artifacts from the core store
  (`render/shaderlib/core_artifacts.h`) through `Resolve` with the device's
  `Facts().artifactFormat`; no consumer names a format.
- **D16:** the draw constants are the uniform block `RenderDrawConstants` at
  uniform slot 64; each submission's blocks go into one buffer, one range per
  draw or dispatch.
- **D20:** the line after `#version` lists each specialization constant's id
  and type; the adapter defines SPIRV-Cross's `SPIRV_CROSS_CONSTANT_ID_<n>`
  macro as a literal of that type after the `#version` line (a non-finite
  float value fails the pipeline). Programs are cached by their stage sources
  and constants.
- **D13:** `glClipControl( GL_UPPER_LEFT, GL_ZERO_TO_ONE )`. Facing is judged
  in clip space, so a counter-clockwise triangle with clip Y up is front, as on
  Vulkan.
- **Tokens (D5, D6):** one fence sync object per submission on the one
  context; fences complete in order.
- **D7:** runs: `gl::SimulateContextLoss` reports `kLost` as a context reset
  (GL_KHR_robustness) would; `Recover` makes a new context and epoch.
- **D10:** the ring is a persistently mapped, coherent buffer; a full ring
  copies from the command's own storage (`glNamedBufferSubData`). The suite
  runs D10 and D5 with submissions held (`gl::HoldSubmissions`, a test-only
  hook: validated, not dispatched until release), the slowest schedule the
  port allows, so an early reuse or an early release is observable on every
  driver rather than only when the GPU happens to lag.
- **D14:** encoders record CPU command lists on any thread; `Submit` replays
  them in order on the calling thread with the context current, and every
  device call restores the caller's own current context afterwards.
- **D18:** not claimed (no exporter). **D19:** claimed with
  `EXT_texture_compression_s3tc` and sRGB S3TC; RGTC is core.
- **Narrower than Vulkan:** texture copies of `kD24UnormS8` are refused (GL has
  no 32-bit transfer that equals the port's depth-aspect copy); storage
  textures of sRGB or depth formats are `kUnsupported`; a layout binding past
  slot 15 of its group is `kUnsupported`.

## Obligations not yet enforced by the shared rules

The Vulkan and OpenGL adapters apply these at `Submit` or creation; the null adapter does
not yet, which is a substitution risk until they move into `validation.cpp`:
draws need a pipeline whose formats and sample count match the attachments,
bound vertex slots and index buffer, and bind groups of the pipeline's layouts;
a dispatch needs a compute pipeline; a bind group naming a released resource is
rejected; the render area fits the attachments; texture copies need a
texel-aligned offset and a single-sample texture. Commands in one usage run in
order (the Vulkan adapter inserts write-after-write barriers). State is tracked
per resource, not per subresource.

## Not claimed

Transient aliasing, parallel native recording, async compute and transfer
queues, ray query, pipeline caches.

## D25: initialized upload buffers (2026-10-02)

`IRenderDevice2::CreateUploadBuffer(bytes)` synchronously snapshots nonempty bytes
into an owned, immutable, copy-source-only buffer in `kCopySource` usage. The
caller may destroy or mutate its bytes on return. Empty input fails with
`kInvalidDescription` in `kCreateBuffer`, without leaked resources. The caller
releases the buffer behind its last consumer's completion token (D5); creation
is not GPU completion. No encoder write is needed. Vulkan initializes mapped
coherent storage before queue submission; GL initializes its buffer on its
context; null owns a byte copy. This additive core-port operation does not alter
legacy engine or extension vtables; all in-tree core providers implement it.

The shared suite checks independent snapshots after caller mutation, use without
an encoder write, rejection of destination use, and cleanup. The texture cache
is its first consumer, eliminating the ring-to-staging GPU transfer.

## D26: memory budget snapshots

`IRenderDevice2::ReadMemoryBudget()` is a nonblocking point-in-time query.
Vulkan reports VMA's per-heap usage and budget when `VK_EXT_memory_budget` is
enabled; otherwise it reports allocation usage as an estimate and marks the
budget unknown. OpenGL reports logical buffer/texture storage as an estimate
and leaves the driver budget unknown. Null reports the bytes held by its own
buffer and texture storage, with no physical budget. Every snapshot carries
the device epoch. Providers that cannot make a snapshot return
`supported=false`; estimates and unknown budgets cannot drive a physical
budget/eviction claim.


### Optional resource activity diagnostics

`IRenderDevice2::ReadResourceActivity` has the semantics documented by
`public/render/device/device.h`. Unsupported adapters return `supported=false`;
they do not claim zero allocations. Vulkan implements the diagnostic, with the
native `render_lab suite cost-overlay --validate` fixture checking successful and
failed allocation, release versus destruction, duplicate release, buffer bytes,
and recovery discontinuities. Logical handle metrics do not certify allocator
or physical VRAM use. Reading diagnostics must not wait for GPU completion.

## D27: packed unsigned-float targets (2026-10-05)

`Format::kRG11B10Float` (Vulkan `B10G11R11_UFLOAT_PACK32`, GL `R11F_G11F_B10F`)
is a 4-byte colour format: R and G 11-bit, B 10-bit unsigned floats with a
5-bit exponent, no alpha. It is sampled, copied and used as a colour
attachment on every adapter (ES needs `EXT_color_buffer_float` to render to
it). `PackRG11B10Float` is the one CPU encoding: round to nearest even,
negatives and NaN to 0, values past the largest finite clamp to it. A
cleared 4x4 attachment copies out as the packed clear colour.

