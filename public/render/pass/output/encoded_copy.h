//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.output's encoded copy: a finished frame whose texels
//			already hold the output encoding (the core shader API's 8-bit
//			colour target) drawn unchanged into a target of another colour
//			format, such as a presentation's BGRA back buffer. A device copy
//			needs equal formats; this pass converts the layout only.
//
//			Threads: the thread that records the frame's presentation.
//
//=============================================================================//

#ifndef RENDER_PASS_OUTPUT_ENCODED_COPY_H
#define RENDER_PASS_OUTPUT_ENCODED_COPY_H

#include "render/device/device.h"
#include "render/device/encoder.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace render::pass::output
{

struct EncodedCopyTargets
{
	// The frame, in `sourceUsage` (it must have kSampled) before and after.
	device::TextureId source;
	device::ResourceUsage sourceUsage = device::ResourceUsage::kSampled;
	// The target, in `targetUsage` (it must have kColorAttachment) before and
	// after; written whole from the top left.
	device::TextureId target;
	device::Format targetFormat = device::Format::kUnknown;
	device::ResourceUsage targetUsage = device::ResourceUsage::kColorAttachment;
	std::uint32_t width = 0; // the copied extent, within both textures
	std::uint32_t height = 0;
};

class EncodedCopy
{
public:
	// Null when the device cannot build the program.
	static std::unique_ptr<EncodedCopy> Create( device::IRenderDevice2 &device );
	// A suite's seeded fragment (SPIR-V) in place of the core one.
	static std::unique_ptr<EncodedCopy> CreateWithFragment(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> fragmentSpirv );
	~EncodedCopy();
	EncodedCopy( const EncodedCopy & ) = delete;
	EncodedCopy &operator=( const EncodedCopy & ) = delete;

	// Records the copy outside rendering; false (nothing recorded) for an
	// invalid request or a device failure.
	bool Record( device::CommandEncoder &encoder, const EncodedCopyTargets &targets );
	// After the submission that holds every Record so far: what they bound
	// is released behind `token` (never before that submission exists).
	void Collect( device::CompletionToken token );

private:
	explicit EncodedCopy( device::IRenderDevice2 &device ) : m_Device( device ) {}
	device::PipelineId PipelineFor( device::Format format );

	device::IRenderDevice2 &m_Device;
	std::vector<std::uint32_t> m_Fragment;
	device::BindGroupLayoutId m_Layout;
	device::SamplerId m_Sampler;
	struct Pipeline
	{
		device::Format format = device::Format::kUnknown;
		device::PipelineId pipeline;
	};
	std::vector<Pipeline> m_Pipelines;
	std::vector<device::BindGroupId> m_Pending;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::output

#endif // RENDER_PASS_OUTPUT_ENCODED_COPY_H
