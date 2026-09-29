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

#include "render/device/encoder.h"
#include "render/device/resources.h"

#include <cstdint>

namespace render::legacy
{

// The target a slot's pass draws into: the backend's open target at the
// slot, as port textures in their home usages (color kColorAttachment, depth
// kDepthWrite). Invalid textures when the backend has not imported that
// target (a render-target texture).
struct CorePassTarget
{
	device::TextureId color;
	device::TextureId depth;
	device::Format colorFormat = device::Format::kUnknown;
	device::Format depthFormat = device::Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t samples = 1;
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
