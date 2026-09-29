//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's check kernels (RFC 0016 K11): a compute program that
//			evaluates a render/shaders/common term per case so a suite can judge
//			it against its C++ oracle. The kernel's draw group (set 3) holds its
//			sampled textures at bindings 0..n-1, its samplers at n..n+s-1 (each
//			linear, clamped to edge), the cases at n+s (a storage buffer: a
//			uvec4 header whose x is the case count, then the cases) and the
//			results at n+s+1 (a storage buffer); 64 invocations per group, one
//			per case.
//			Private to render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_COMPUTE_H
#define RENDER_LAB_LAB_COMPUTE_H

#include "render/device/device.h"
#include "render/resources/texture_cache.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace render::lab
{

class CheckKernel
{
public:
	explicit CheckKernel( device::IRenderDevice2 &device ) : m_Device( device ) {}
	~CheckKernel();
	CheckKernel( const CheckKernel & ) = delete;
	CheckKernel &operator=( const CheckKernel & ) = delete;

	// The kernel's SPIR-V module reading `textures` sampled textures and
	// `samplers` samplers; the reason when the device refuses it.
	std::optional<std::string> Create( std::span<const std::uint32_t> module,
	    std::uint32_t textures, std::uint32_t samplers, std::string_view name );

	// Records the cache's pending uploads, dispatches one invocation per case
	// over `textures` (in binding order) and reads `resultBytes` back.
	std::optional<std::string> Run( resources::TextureCache &cache,
	    std::span<const device::TextureId> textures, std::uint32_t caseCount,
	    std::span<const std::byte> cases, std::uint64_t resultBytes, std::vector<std::byte> &out );

private:
	device::IRenderDevice2 &m_Device;
	std::uint32_t m_Textures = 0;
	std::uint32_t m_Samplers = 0;
	device::BindGroupLayoutId m_Layout;
	device::PipelineId m_Pipeline;
	device::SamplerId m_Sampler;
};

} // namespace render::lab

#endif // RENDER_LAB_LAB_COMPUTE_H
