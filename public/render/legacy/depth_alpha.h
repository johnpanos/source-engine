//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.legacy-frontend's depth-alpha copy (RFC 0016 K9): D3D9 PC
//			keeps the opaque scene's projected depth in destination alpha
//			(WRITE_DEPTH_TO_DESTALPHA), so a frame copy carries it and soft
//			particles read it (CoreMeshDraw::depthAlphaHandle). The core's
//			passes do not write it; a frontend's frame copy gets it here, from
//			the depth the scene drew with, into the copy's alpha only.
//
//			A frontend reaches it through ICorePassRecorder::RecordDepthAlpha
//			(the legacy frontend's recorder owns one per device).
//
//			Threads: the recording thread of the frontend that owns it.
//
//=============================================================================//

#ifndef RENDER_LEGACY_DEPTH_ALPHA_H
#define RENDER_LEGACY_DEPTH_ALPHA_H

#include "render/device/device.h"
#include "render/device/encoder.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace render::legacy
{

struct DepthAlphaCopy
{
	// The device it records on, and its last submission (what earlier
	// copies read is released behind it).
	device::IRenderDevice2 *device = nullptr;
	device::CompletionToken submitted;
	// The frontend's recording this copy goes into, not yet submitted: what
	// copies recorded into it read stays held until a later recording's copy
	// (or Collect) releases it behind a submission that contains it.
	std::uint64_t recording = 0;
	// The copy: an RGBA colour texture in kCopyDestination after the colour
	// copy; it is left there. Its rectangle (texels from the top left) matches
	// the depth's, as the colour copy kept its texels' place.
	device::TextureId target;
	device::Format targetFormat = device::Format::kUnknown;
	std::uint32_t targetWidth = 0;
	std::uint32_t targetHeight = 0;
	std::uint32_t x = 0;
	std::uint32_t y = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	// The scene's depth, in `depthUsage`, where it is left; it must have kSampled.
	device::TextureId depth;
	device::ResourceUsage depthUsage = device::ResourceUsage::kDepthWrite;
	// The draw projection's z column (zScale, zOffset, wScale, wOffset) and the
	// dest-alpha depth range.
	float projection[4] = {};
	float range = 0.0f;
};

class DepthAlphaPass
{
public:
	// Null when the device cannot build the program.
	static std::unique_ptr<DepthAlphaPass> Create( device::IRenderDevice2 &device );
	// A suite's seeded fragment (SPIR-V) in place of the core one; null on a
	// device of another artifact format.
	static std::unique_ptr<DepthAlphaPass> CreateWithFragment(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> fragmentSpirv );
	~DepthAlphaPass();
	DepthAlphaPass( const DepthAlphaPass & ) = delete;
	DepthAlphaPass &operator=( const DepthAlphaPass & ) = delete;

	// Records the copy's alpha outside rendering; false (nothing recorded) for
	// an invalid request or a device failure.
	bool Record( device::CommandEncoder &encoder, const DepthAlphaCopy &copy );
	// What copies recorded before `recording` read was submitted by `token`
	// and is released behind it; what `recording`'s own copies read is kept
	// (that recording is still open, its encoder unsubmitted).
	void Collect( device::CompletionToken token, std::uint64_t recording );

private:
	explicit DepthAlphaPass( device::IRenderDevice2 &device ) : m_Device( device ) {}
	device::PipelineId PipelineFor( device::Format format );

	device::IRenderDevice2 &m_Device;
	std::vector<std::uint32_t> m_Fragment; // a seeded fragment; empty for the core one
	device::BindGroupLayoutId m_Layout;
	device::SamplerId m_Sampler;
	struct Pipeline
	{
		device::Format format;
		device::PipelineId pipeline;
	};
	std::vector<Pipeline> m_Pipelines;
	struct PendingGroup
	{
		device::BindGroupId group;
		std::uint64_t recording = 0;
	};
	std::vector<PendingGroup> m_Pending;
	device::CompletionToken m_LastToken;
};

} // namespace render::legacy

#endif // RENDER_LEGACY_DEPTH_ALPHA_H
