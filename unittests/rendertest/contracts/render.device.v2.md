# Contract: `render.device.v2`

Module: `render.device` (port; RFC 0016 layer 1)
Contract headers: `public/render/device/` (`device.h`, `encoder.h`, `usage.h`,
`completion.h`, `resources.h`, `pipeline.h`, `bind_group.h`, `facts.h`,
`errors.h`, `conventions.h`, `validation.h`, `provider.h`)
Shared suite: `unittests/rendertest/core/device/device_conformance.h`
Adapters: `render.device.null` (`test_device_null.cpp`),
`render.device.vulkan` (`test_device_vulkan.cpp`, GPU runner);
`render.device.gl` is RFC 0016 K10.
Sensitivity: `unittests/rendertest/core/device/test_device_negative.cpp`
Rows: R86 (RFC 0016 K1)

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
| D10 | Upload ranges are reused only after their token completes; a full ring defers | retires ranges at submission |
| D11 | Recording one encoder from a second thread is diagnosed (`SequenceViolations`); the check lives in the port | — (port-owned) |
| D12 | Transitions name the resource's current usage and a usage it was created with; clears, copies and attachments need their usage. The current usage is the state after every accepted submission, complete or not: a submission may continue from one still running | accepts transitions from any usage |
| D14 | Distinct encoders may be recorded concurrently, one thread per encoder, and submitted together; recording shares the upload ring safely and their bytes land | — (TSan lane of the pooled graph executor) |
| D16 | Draw constants: a pipeline declares a block of at most 128 bytes (a multiple of four) that its stages read, and a stage reflecting more fails `kLayoutMismatch`; binding a pipeline leaves the block undefined, and a draw or dispatch with any word unset since then, or a write outside the block, fails the submission (`kInvalidState`); on rasterizing adapters each draw sees its own constants. Vulkan: push constants; OpenGL: a uniform block | zero-fills the block when a pipeline is bound |
| D13 | Conventions on real pixels: clip depth 0 to 1, clip Y up, row 0 at the top (rasterizing adapters only) | not yet: a flipped-Y and a −1..1-depth adapter need the GPU lane |

## Obligations not yet enforced by the shared rules

The Vulkan adapter applies these at `Submit` or creation; the null adapter does
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
queues, ray query, per-heap budgets (VMA is not pinned yet), pipeline caches.
