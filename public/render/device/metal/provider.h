//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.metal, the Metal adapter of render.device.v2
//			(RFC 0025). The composition root names this header; nothing here
//			reaches a Metal, Objective-C, SDL or other native header (CAP007,
//			CAP005).
//
//			The adapter needs a Metal 3 device (MTLGPUFamilyMetal3: Apple
//			silicon Macs, A13 and later) on macOS 13 or iOS/tvOS 16. It
//			accepts Metal Shading Language 3.0 artifacts (ArtifactFormat::kMsl),
//			which the build translates from the SPIR-V with the pinned
//			SPIRV-Cross (tools/render/shader_artifacts.py metal_compile owns
//			their form): each bind group is one argument buffer, encoded per
//			pipeline stage through that stage function's own argument encoder.
//
//			One sequence calls the device at a time (the render sequence, RFC
//			0016 "Threading"); encoders record CPU command lists on any thread,
//			and Submit replays them into one command buffer per submission, in
//			submission order, on one command queue. Tokens complete in order;
//			their completion comes from the command buffers' completion
//			handlers.
//
//			The port's conventions are Metal's own: clip depth 0 to 1, clip +Y
//			up, row 0 the top row. No correction is applied.
//
//=============================================================================//

#ifndef RENDER_DEVICE_METAL_PROVIDER_H
#define RENDER_DEVICE_METAL_PROVIDER_H

#include "render/device/provider.h"

#include <cstdint>
#include <memory>

namespace render::device::metal
{

struct MetalAdapterOptions
{
	// Labels resources and command buffers, and keeps the command buffers'
	// error records (MTLCommandBufferErrorOption::EncoderExecutionStatus).
	// The Metal API validation layer itself is an environment setting
	// (MTL_DEBUG_LAYER=1).
	bool validation = false;
	// Bytes of the adapter-owned upload ring (at least 4096).
	std::uint64_t uploadRingBytes = std::uint64_t( 4 ) << 20;
	// The capabilities the device may claim: those it has and this allows
	// (RFC 0016 K10 "Capability negotiation").
	CapabilitySet allowed = CapabilitySet::All();
};

// "metal".
const DeviceProviderDescriptor &Describe();
DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const MetalAdapterOptions &options );

// Command buffers that ended with an error so far; 0 for a device not made
// by this adapter.
std::uint64_t CommandBufferErrors( const IRenderDevice2 &device );
// Tests only: the device reports kLost, as it does when a command buffer
// fails with a device-removed or not-permitted error, so the loss clause
// (D7) and Recover() run. False for a device not made by this adapter.
bool SimulateDeviceLoss( IRenderDevice2 &device );

} // namespace render::device::metal

#endif // RENDER_DEVICE_METAL_PROVIDER_H
