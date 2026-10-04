//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Core passes inside the legacy stream (RFC 0016 K5 plan, step 3).
//
//			A legacy backend (the native Vulkan shader API) buffers a frame as
//			one ordered stream and replays it in the frame graph's scene pass
//			(frame_source.h). A core pass draws at a *slot* of that stream:
//			when the client marks a stage the recorder asks for, the frontend
//			queues a slot in frame order (as its capability adapters queue
//			their calls), and the backend appends it to the stream. When the
//			scene pass records, the backend has the recorder record each
//			slot's pass into the pass's encoder as a section of its scene
//			record (render/device/vulkan/host_device.h), and its replay runs
//			the section at the slot, between closing its own render pass and
//			reopening it. A frame without a recorder, or with no slot, replays
//			as before.
//
//			ABI-facing: the backend builds with the engine's standard-library
//			ABI and the frontend with the core's, so only interfaces and port
//			types without library strings or containers cross here.
//
//=============================================================================//

#ifndef RENDER_LEGACY_CORE_PASSES_H
#define RENDER_LEGACY_CORE_PASSES_H

#include "render/device/device.h"
#include "render/device/encoder.h"
#include "render/device/resources.h"
#include "render/material/surface_program.h"

#include <cstdint>
#include <span>

namespace render::legacy
{

class ICoreTextures;

// The view's fog at a slot, as the legacy pixel shaders read it
// (common_ps_fxc.h CalcPixelFogFactor and BlendPixelFog, and the backend's
// SetPixelShaderFogParams and UpdatePixelFogColorConstant for a pass that
// writes sRGB and fogs to the scene's color).
struct CorePassFog
{
	float type = -1.0f; // -1 none, 0 range, 1 height (below the water's z)
	// Linear, and scaled by the tone-mapping scale in integer HDR.
	float color[3] = { 0.0f, 0.0f, 0.0f };
	// Range: start / range, water z, max density, 1 / range. Height: 0,
	// water z, 1, 1 / range.
	float params[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float eyeZ = 0.0f; // the camera's world z
	bool operator==( const CorePassFog & ) const = default;
};

// The target a slot's pass draws into: the backend's open target at the
// slot, as port textures in their home usages (color kColorAttachment, depth
// kDepthWrite). Invalid textures when the backend has not imported that
// target (a render-target texture).
struct CorePassTarget
{
	material::SurfaceDrawState drawState{};
	float clipPlanes[6][4] = {};
	float minDepth = 0.0f;
	float maxDepth = 1.0f;
	// The device the slot records on: the backend's.
	device::IRenderDevice2 *device = nullptr;
	device::TextureId color;
	device::TextureId colorSrgb; // the same image through its sRGB view, when it has one
	device::TextureId depth;
	device::Format colorFormat = device::Format::kUnknown;
	device::Format colorSrgbFormat = device::Format::kUnknown;
	device::Format depthFormat = device::Format::kUnknown;
	bool colorCopySource = false; // the imported color supports a scene-color capture
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t samples = 1;
	// The backend's textures, for the pass's materials; null when the backend
	// imports none.
	ICoreTextures *textures = nullptr;
	// No earlier than every submission made before this frame's: what the
	// previous frames used can be released behind it.
	device::CompletionToken submitted;
	// The serial of the frame being recorded: it rises with each frame
	// submission (a frame recorded again for a capture keeps or raises it).
	// 0 when the backend does not count frames.
	std::uint64_t frame = 0;
	// CPU stream identity, unchanged when a capture replays the same stream
	// in another GPU submission. Changes only when that stream is discarded.
	std::uint64_t streamEpoch = 0;
	// The frame's light terms at the slot: the lightmap scale for how the
	// pages encode light, and the output's linear (tone-mapping) scale.
	float lightmapScale = 1.0f;
	float outputScale = 1.0f;
	// The eye's world position (c10), ENV_MAP_SCALE (16 in integer HDR, else
	// 1), and whether specular shows (mat_fastspecular, and not
	// mat_fullbright 2: else env map tints are zero).
	float eye[3] = { 0.0f, 0.0f, 0.0f };
	float envmapScale = 1.0f;
	bool specular = true;
	// The running game's shaders scale every ssbump's basis weights by
	// 1/sqrt(3) (Portal 2's LightmappedGeneric; the backend's policy).
	bool ssbumpNormalized = false;
	CorePassFog fog; // the view's fog at the slot
	// The shaders' time at the slot (the backend's CurrentTime, in seconds),
	// which animated terms read (the water point's flow).
	float time = 0.0f;
	float foliage[2][4] = {}; // client-owned wind xy and time z, current/previous frame
	bool foliageAvailable = false;
	// The scale of the water point's reflection tint: 4 in integer HDR, where
	// the client draws the water views at a quarter of the tone-map scale
	// (SetLightmapScaleForWater) and Water multiplies its tint back; else 1.
	float waterReflectTintScale = 1.0f;
};

// The backend's textures as port textures (RFC 0016 K5 step 4): the image
// behind a material system texture handle (ITexture::GetTextureHandle, or a
// lightmap page's), imported on first use in the usage it rests in between
// uses (kSampled) and released, behind the frame being recorded, when the
// backend deletes or replaces it. Called on the render sequence while a slot
// records. A cube map (six layers) imports as a kCube texture. A render
// target imports too: it rests in kSampled between the passes that draw into
// it, and a slot reads what the stream drew into it before the slot (a
// resize replaces its image: the caller re-imports per frame). An invalid id
// when the handle names no uploaded 2D or cube image (a volume, an array) or
// a format with no port format.
class ICoreTextures
{
public:
	// srgb asks for linear values: an 8-bit or BC image through its sRGB view
	// (an invalid id when it has none); a 16-bit or float image as it is.
	virtual device::TextureId Import( int handle, bool srgb ) = 0;
	// The sampler the backend binds the texture with: its sampler state
	// (wrap, filters, mips) and the anisotropy setting. The port has one
	// address mode, which clamps only when both axes do.
	virtual device::SamplerDesc Sampler( int handle ) = 0;
	// Nonzero content epoch, changed on every accepted pixel update and handle
	// reuse. Zero means unknown (including render targets): do not reuse composites.
	// Sampled contents are ordered before the slot, like Import; render sequence only.
	virtual std::uint64_t ContentRevision( int ) { return 0; }

protected:
	~ICoreTextures() = default;
};

// A slot's tag: the stage it was marked at (frame::Stage) and the number of
// views open then.
inline constexpr std::uint32_t CorePassTag( std::uint32_t stage, std::uint32_t depth )
{
	return ( stage & 0xffu ) | ( depth << 8 );
}
inline std::uint32_t CorePassTagStage( std::uint32_t tag )
{
	return tag & 0xffu;
}
inline std::uint32_t CorePassTagDepth( std::uint32_t tag )
{
	return tag >> 8;
}
// A tag with the high bit set was marked by a pass the composition root
// forwards to (ILegacyFrontend::SetForwardedRecorder), not by a stage.
inline constexpr std::uint32_t kCorePassForwarded = 0x80000000u;
// A forwarded tag with this bit too turns the legacy stream off from its slot
// to the end of the frame (RFC 0014: a pixel view, or cl_render_debug_legacy
// 2). The backend's replay then records no legacy draw, copy or scene capture
// and clears only depth and stencil, and the frame presents without the
// monitor gamma ramp; its slots still run. The composition root marks it at
// the frame's first slot, where it records the not-applicable hatch.
inline constexpr std::uint32_t kCorePassLegacyOff = 0x40000000u;
// Product-only exception to LegacyOff: retain PortalRefract stages 0/2 and
// SolidEnergy at their original stream positions. Only portal refraction keeps
// its consumed framebuffer snapshot. Portal stage 1 remains core.
// Pixel diagnostics/legacy-skip never set this bit (R91, 2026-10-02).
inline constexpr std::uint32_t kCorePassCustomEffects = 0x10000000u;
// A forwarded tag with this bit marks the end of the frame's stream, after
// every draw and before the present (RFC 0014 cl_render_debug_legacy 1: the
// composition root tints what the core did not draw there). The backend runs
// it as any slot.
inline constexpr std::uint32_t kCorePassFrameEnd = 0x20000000u;

// An ordinary stage slot at the top-level HUD boundary permits legacy UI
// after a core-only scene. The composition requests this slot only for the
// normal game view; pixel diagnostics and legacy-skip views never request it.
inline constexpr std::uint32_t kCorePassLegacyHud = CorePassTag( 8u, 0u );

// A backend's slots (LegacyShaderServices::corePassSlots). Called in frame
// order on the thread that replays the material system's calls.
class ICorePassSlots
{
public:
	// Appends a slot to the frame's stream at this point.
	virtual void MarkSlot( std::uint32_t tag ) = 0;

protected:
	~ICorePassSlots() = default;
};

// The frame's output (RFC 0016 "Output", render.output.v1) in the backend's
// present stage: the scene (the back buffer, read as linear values: an 8-bit
// one through its sRGB view) to the acquired swapchain image, both port
// textures in their home usage kColorAttachment. The extents may differ (the
// video mode on a larger drawable). The headroom is the presentation's,
// read for this frame; exactly 1 for an 8-bit swapchain.
struct CoreOutputTargets
{
	device::IRenderDevice2 *device = nullptr;
	device::TextureId scene;
	device::Format sceneFormat = device::Format::kUnknown;
	std::uint32_t sceneWidth = 0;
	std::uint32_t sceneHeight = 0;
	device::TextureId target;
	device::Format targetFormat = device::Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	float exposure = 1.0f;
	float scenePeak = 1.0f;
	float headroom = 1.0f;
	float linearScale = 1.0f;
	bool toneMap = true; // false for a debug view (RFC 0014): the encoding alone
	// No earlier than every submission made before this frame's.
	device::CompletionToken submitted;
};

// Value views of one material-system mesh draw. QueueMesh copies every byte
// synchronously; neither proxies nor the legacy mesh may be borrowed afterward.
// Positions and tangent frames are world space; matrices are row-major with
// column vectors, matching FamilyDrawConstants. Triangles use the core's
// counterclockwise front-face convention. No native shading crosses here.
struct CoreMeshVariable
{
	const char *key = nullptr;
	const char *value = nullptr;
	const char *defaultValue = nullptr;
	int textureHandle = 0;
};
enum class CoreMeshKind : std::uint8_t
{
	kSurface,
	kDecal,        // authored projected surface cohort, independent of other dynamic draws
	kUnlit,        // simple emissive surfaces, including the sky behind transmitting glass
	kTransmission, // scene-color glass, independent of other dynamic draws
	kModelSurface, // VertexLitGeneric model surfaces, including window frames and coated glass
	kDepthMask,
	kStencilClear,
	kLightmappedSurface, // moving brushes and proxy-selected indicator panels
	kCable               // gameplay-expanded rope ribbons with captured vertex illumination
};
struct CoreMeshDraw
{
	CoreMeshKind kind = CoreMeshKind::kSurface;
	const char *name = nullptr;
	const char *shader = nullptr;
	const CoreMeshVariable *variables = nullptr;
	std::uint32_t variableCount = 0;
	const material::SurfaceWorldVertex *vertices = nullptr;
	std::uint32_t vertexCount = 0;
	const std::uint32_t *indices = nullptr;
	std::uint32_t indexCount = 0;
	// The captured foliage root; vertices remain in world space.
	float modelToWorld[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	float toClip[16] = {};
	float worldToView[16] = {};
	float viewToClip[16] = {};
	device::Viewport viewport;
	int lightmapPage = 0; // native texture handle for captured lightmap coordinates
	bool capturedLightmap = false;
	bool mesh = false; // model/refraction point, rather than a lightmapped surface
};

// What records a slot's pass (the frontend's, bound by the composition root).
class ICorePassRecorder
{
public:
	virtual bool AcceptsMeshes() const { return false; }
	// The render sequence, before marking its stream slot. 0 refuses the whole
	// draw; a returned tag promises to draw it or record an explicit failure.
	virtual std::uint32_t QueueMesh( const CoreMeshDraw & ) { return 0; }
	// The stages (bit 1 << frame::Stage) at which the frontend queues a slot;
	// none, no slot.
	virtual std::uint32_t SlotStages() const = 0;
	// Records slot `tag`'s pass into `encoder`, as a section of the scene
	// pass: it begins and ends outside rendering, leaves the target's
	// textures in their home usages, closes the labels it opens and binds
	// what it uses. Called on the render sequence, while the scene pass
	// records, once per slot in stream order.
	virtual void RecordSlot(
	    std::uint32_t tag, device::CommandEncoder &encoder, const CorePassTarget &target ) = 0;
	// Record a compatible prefix of adjacent slots with identical target state
	// except shader time. Only time-independent draws may consume a follower;
	// a refused follower records later with its original target, including time.
	// Records the prefix with the first slot's target,
	// returning its length. The host may omit only commands replay itself skips;
	// every draw, clear, query and copy that executes is an ordering boundary.
	// The default consumes one slot; callers keep one section per original slot
	// (empty sections for consumed followers), including during capture replay.
	virtual std::size_t RecordOpaqueBatch( std::span<const std::uint32_t> tags,
	    device::CommandEncoder &encoder, const CorePassTarget &target )
	{
		if ( tags.empty() )
			return 0;
		RecordSlot( tags.front(), encoder, target );
		return 1;
	}
	// Records the frame's output into `encoder` as a section of the present
	// stage, outside rendering, leaving both textures in their home usages.
	// False when it recorded nothing (then the backend presents as before).
	// Called on the render sequence while the present stage records.
	virtual bool RecordOutput(
	    device::CommandEncoder &encoder, const CoreOutputTargets &targets ) = 0;
	// The native host's submission result, on the same render sequence as
	// recording. This token fences its GPU work; a composition renderer using
	// another device cannot substitute its own token. Failure submits no work.
	virtual void FrameSubmitted( device::CompletionToken, bool ) {}
	// The backend's device is about to go, after an idle wait: release every
	// object made on it now (later releases would reach a destroyed device).
	virtual void ReleaseDevice( device::IRenderDevice2 &device ) = 0;

protected:
	~ICorePassRecorder() = default;
};

} // namespace render::legacy

// The composition root binds the frontend's recorder into the native Vulkan
// backend (next to NativeVulkanShaderBackend_BindFrameExecutor). Without one
// the backend marks no slot.
extern "C" void NativeVulkanShaderBackend_BindCorePassRecorder(
    render::legacy::ICorePassRecorder *recorder );

#endif // RENDER_LEGACY_CORE_PASSES_H
