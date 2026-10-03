# RFC 0019: Temporal Upscaling Contract

- Status: Proposed (2026-10-03); initial lab provider and opt-in [game integration](0019-fsr-game-wip-2026-10-03.md) installed, no qualification gate complete
- Date: 2026-10-03
- Scope: An opt-in temporal reconstruction boundary for the render core, with
  AMD FSR Upscaling 4.1.1 as the first candidate provider. Frame generation is
  outside this contract.
- Render architecture: [RFC 0016](0016-render-core.md) owns frame/view, graph,
  device and output semantics and its binding rules apply to all work here.
- Antialiasing policy: [RFC 0012](0012-antialiasing-msaa-specular-alpha-coverage.md)
  owns MSAA, alpha to coverage and specular AA. Its existing profile defaults
  and oracles remain authoritative.
- Platform composition: [RFC 0001](0001-capability-based-platform-architecture.md).
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md),
  Q-PRESENTATION and Q-PRODUCT.
- Tracking: an unranked proposed render slice. Adding this contract does not
  enable an upscaler or change a product profile.

## Target cutover (user direction, 2026-10-03)

Once FSR has passed the contract's input, image, portal and lifecycle gates,
it is the intended replacement for 4x MSAA in the selected High product profile.
That profile runs one antialiasing policy for the main view: FSR reconstruction
without 4x MSAA. The profile and its budget row change together at promotion,
with the output extent, chosen render extent, FSR preset, required image quality
and the existing 120 FPS target explicit. Performance is measured and any miss
is recorded; it does not block implementation, selection or release under
[RFC 0016's FSR temporal exception](0016-render-core.md#binding-rules-for-all-render-work-user-decision-2026-09-28).
This records a future target, not a claim that the current 4x MSAA High gate
has changed or that an FSR product path is installed.

## Decision and ownership

`render.temporal-upscale.v1` is the proposed provider-neutral contract between
one frame-owned view and one temporal reconstruction provider. It names image
meaning, extents, time, history, failures and output, not Vulkan handles or an
FSR-specific descriptor list. A provider may claim only the formats, ratios,
platforms and features it can actually run. Native-resolution output remains
the existing path when no temporal provider is selected.

| Authority | Obligation |
| --- | --- |
| `render.frame` | Select a stable view identity and explicit discontinuity; supply current/previous camera state, jitter and frame time |
| `render.scene` and its producer | Publish current and previous committed object, bone and deformation state; mark missing correspondence rather than inventing zero motion |
| `render.pass.temporal-upscale` (proposed) | Validate the portable inputs, place one pass per selected view, own per-view history metadata and provider lifetime |
| Selected provider | Declare input requirements and capability limits; own its private persistent images and dispatch; report failure without changing a different view's state |
| `render.graph` and `render.device` | Declare accesses and synchronize resources and completion; keep native handles in a named private adapter bridge if one is needed |
| `render.pass.output` | Apply the existing exposure, tone map and presentation encoding after reconstruction |
| Product profile and composition root | Choose the mode and provider before the frame; record capability refusal and a declared native path |

No provider may infer view identity from a viewport rectangle, `viewBit`,
recursion depth alone, or the last provider dispatch. One scene can have several
eyes, portals, monitors and water views in the same host frame.

## Per-view input and output contract

All coordinates below use `render.device.v2`'s clip/depth conventions; the
provider adapter converts to its own API conventions explicitly. Input textures
belong to the same committed scene/view frame. Their graph declarations name
each read and write, including persistent history.

| Input | Required meaning |
| --- | --- |
| Extents | Positive render and output extents, active viewport and sample count. The provider reports its supported ranges before composition. No implicit resize of an input. |
| Scene color | Single-sample, finite linear scene radiance at render extent, before output exposure/tone mapping and presentation encoding. Resolve an MSAA scene through its declared policy before dispatch. |
| Depth | Single-sample render-extent depth from the same jittered view; depth direction, near/far and finite/infinite projection are declared. An MSAA depth resolve must specify which surface wins at an edge. |
| Motion | Two-component render-extent displacement in render pixels from the current *unjittered* surface position to its previous *unjittered* position. Camera, world, moving/posed models, cutouts and visible translucent surfaces need coverage; invalid correspondence is explicit. |
| Camera/time | Stable view key, current and previous unjittered transforms, applied render-pixel jitter, positive frame delta in milliseconds, host frame sequence and reset reason. The pass rejects mismatched dimensions, nonfinite values and a stale sequence before recording. |
| Exposure | The linear color's scale and the downstream output exposure are explicit. A provider may request auto exposure or a supplied exposure, with the choice fixed for a history epoch. |
| Optional coverage | A provider declares whether reactive or transparency/composition masks are required or optional. Omitted optional masks have a documented neutral value; a required mask cannot be silently omitted. |
| Output | Single-sample linear scene color at output extent, in the same radiance domain as the input. The output pass then owns tone mapping, encoding and display headroom. |

The renderer applies jitter to *all* render-resolution 3D color and depth
producers in the selected view and reports that same offset to the provider.
Motion excludes the jitter displacement; a provider that expects another
convention adapts it at its boundary. A zero vector means genuinely stationary
in screen space. Unsupported or missing motion for a contributing cohort fails
the selected mode's quality gate; the product cannot count that cohort as
temporally reconstructed. Exposure changes must use the provider's documented
compensation or start a new history epoch.

The first lab reference should exercise 1x and at least one larger output
extent, so the boundary proves both temporal AA and upscaling. A provider's
named presets and mip bias remain provider policy; the render core owns only
the actual extents and sampler state supplied to materials.

## Graph placement and view history

For a selected view, the graph records scene lighting, transparency and
scene-dependent effects at render extent, resolves its declared MSAA inputs,
then reconstructs linear color. `render.pass.output` consumes the reconstructed
color. Display-space HUD/text and effects that would be destabilized by
temporal accumulation are composed at output extent at their established stage;
moving a legacy effect across this boundary needs its own image oracle. The
default native path preserves its existing draw order and output bytes.

The history key comprises the scene generation, stable view-generator identity,
eye, portal/mirror recursion chain and destination role. The producer supplies
the identity; a provider does not derive it from matrices. A parent view and a
nested view never share history. A nested view reconstructed into a texture is
completed before its parent consumes it. A view known to lack valid
motion/depth selects its separately qualified native mode before graph
recording and reports that choice; a late input failure fails the frame. At a
portal boundary, the main view's motion and validity identify pixels whose visible
surface changed across the portal. Such pixels cannot borrow ordinary wall
history. The portal fixture must cover entry, exit, moving portal surfaces and
recursion, not only a stationary aperture.

A history epoch resets on first use, camera cut/teleport, scene replacement,
view-key change, skipped or out-of-order view frame, render/output extent or
mode change, exposure convention change, device recovery, or missing required
inputs. A dynamic object without previous state invalidates its covered pixels;
it does not necessarily reset the whole view. A history image and provider
context outlive every GPU use. Rebuild and teardown retire them behind the
device's completion token, including resize and partial startup; a fixed
number of frames in flight is not a completion proof. Failure during a frame
cannot leave partially updated history visible as a successful output.

## Capability and profile policy

The composition root queries required GPU features, shader artifacts, formats,
memory, extents and provider availability before selecting the mode. The
portable contract exposes no backend or OS identity. A Vulkan implementation
is an adapter under the core; a provider that cannot run on OpenGL, MoltenVK or
a mobile GPU reports unavailable there. A missing required capability fails
the requested mode by name before the graph mutates. A product may explicitly
select its separately qualified native mode; it must not silently turn a
requested temporal mode into a different quality claim.

The current `linux-desktop-high-120` profile in
[`quality/budgets/render-v1.json`](../quality/budgets/render-v1.json) requires a
native 1920x1080 High image with 4x MSAA. At the cutover above, its owner
updates that profile's sample policy and render extent as an explicit user
decision, while retaining the 1920x1080 output and 120 FPS target. The FSR
mode's budget verdict becomes advisory at that cutover; a miss is noted with
measured evidence and follow-up, without blocking promotion. Image acceptance
requires a matched complete-image comparison against 4x MSAA across motion,
cutouts, fine texture, specular highlights, transparency and portal boundaries;
passing an upscaler microbenchmark or an incomplete frame is insufficient.
Profiles without a qualifying FSR provider retain their own declared native
policy. RFC 0012's specular AA and offline texture filtering remain applicable
where their material/profile policies say so; alpha coverage follows the
selected sample count.

## Acceptance slices

1. **Input oracle in `render_lab`.** A moving camera, rigid and skinned
   objects, UV motion, cutout, glass and portal fixtures compare color, depth,
   motion and jitter with independent analytic/supersampled references. Seeded
   sign, scale, stale-transform and missing-cohort defects fail.
2. **Provider contract.** A fake and every claiming provider run the same
   sequence: create, first frame, accumulation, reset, two simultaneous views,
   resize, skipped view, loss/recovery, failed dispatch and teardown. Deliberately
   bad providers are detected. Graph traces show exact accesses and completion
   tokens; validation reports zero sync or shader errors.
3. **Image quality.** Static, panning and disocclusion sequences compare
   against native and supersampled references for edge stability, detail,
   ghosting, transparency and portal boundaries. The user reviews matched
   clips, not only still frames or synthetic hashes. A named provider/version
   and its masks, jitter and reset settings are in the evidence.
4. **Products and profiles.** After the lab gate, an installed Portal 2 client
   exercises mode selection, camera cuts, portal views, resize and recovery.
   Each claimed platform has native hardware evidence. GPU time, CPU record/
   submission time, memory and every presented-frame interval are recorded
   against the selected profile's target with the complete image enabled.
   A performance miss is reported and does not block this FSR path. Unsupported
   profiles report unavailable and retain their existing qualified path.

The full acceptance slices above remain open. The initial lab provider and
its positive/sensitivity suites are installed in the shared conformance registry;
[implementation evidence](0019-fsr-lab-2026-10-03.md) records their commands and
limited claims. Those synthetic sequences do not certify the full input, image,
portal, platform or product gates.

## Observed starting point (2026-10-03)

At `30e07e9cd`, `public/render/frame/renderer.h` has frame extents and target
but no temporal view key or previous transforms. `public/render/scene/view.h`
has current view/projection and a visibility bit, not temporal identity.
`render.pass.output` already consumes a linear float scene and owns tone mapping
and encoding. The graph supports imported persistent textures and compute
passes, while the device port does not expose raw Vulkan command buffers.
RFC 0012 expressly excludes temporal implementation. The separate
`~/src/scaling` experiment records seven synthetic byte-exact Vulkan/reference
outputs; it does not establish this engine's frame inputs, lifecycle or quality.

AMD's [FSR 4.1.1 integration guide](https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/blob/v2.3.0/Kits/FidelityFX/docs/techniques/super-resolution-ml.md)
informs the first provider's input checklist: linear color, depth, motion,
jitter, frame time, exposure and camera-cut reset. Its resource-requirements
query distinguishes optional from required masks. This RFC owns the engine
contract; the selected provider owns any SDK-specific flags or limits.
