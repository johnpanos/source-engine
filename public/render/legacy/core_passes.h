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

#include <cstdint>

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
};

// The target a slot's pass draws into: the backend's open target at the
// slot, as port textures in their home usages (color kColorAttachment, depth
// kDepthWrite). Invalid textures when the backend has not imported that
// target (a render-target texture).
struct CorePassTarget
{
	// The device the slot records on: the backend's.
	device::IRenderDevice2 *device = nullptr;
	device::TextureId color;
	device::TextureId colorSrgb; // the same image through its sRGB view, when it has one
	device::TextureId depth;
	device::Format colorFormat = device::Format::kUnknown;
	device::Format colorSrgbFormat = device::Format::kUnknown;
	device::Format depthFormat = device::Format::kUnknown;
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
};

// The backend's textures as port textures (RFC 0016 K5 step 4): the image
// behind a material system texture handle (ITexture::GetTextureHandle, or a
// lightmap page's), imported on first use in the usage it rests in between
// uses (kSampled) and released, behind the frame being recorded, when the
// backend deletes or replaces it. Called on the render sequence while a slot
// records. A cube map (six layers) imports as a kCube texture. An invalid id
// when the handle names no uploaded 2D or cube image (a volume, an array), a
// render target, or a format with no port format.
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

protected:
	~ICoreTextures() = default;
};

// A slot's tag: the stage it was marked at (frame::Stage) and the number of
// views open then.
inline std::uint32_t CorePassTag( std::uint32_t stage, std::uint32_t depth )
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

// What records a slot's pass (the frontend's, bound by the composition root).
class ICorePassRecorder
{
public:
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
