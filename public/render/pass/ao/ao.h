//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.ao (RFC 0016 render.lighting.v1 "Ambient occlusion"):
//			ground-truth ambient occlusion (Jimenez, Wu, Pesce and Jarabo
//			2016, "Practical Real-Time Strategies for Accurate Indirect
//			Occlusion") from the view's depth and normals, a depth- and
//			normal-aware blur, and the visibility written for the surface
//			program (kSurfaceAmbientOcclusion), which applies it to indirect
//			light only with the paper's multi-bounce fit.
//
//			The occlusion never counts twice what the bake already holds: each
//			pixel's search radius is the normalRoughness input's alpha when it
//			is above 0 (the prepass writes two lightmap texels' world size on a
//			lightmapped surface, so only detail finer than the bake resolves),
//			else the pass's radius (surfaces lit by the probe volume, whose
//			probes see no nearby occluder).
//
//			Per pixel, `slices` directions in the screen (fixed angles turned
//			by a 4 x 4 pattern), each marched `steps` samples each way to the
//			radius; a sample's horizon fades to none between `falloff` of the
//			radius and the radius. The slice's visibility is the paper's
//			cosine-weighted integral between the two horizons clamped to the
//			normal's hemisphere, times the projected normal's length.
//
//			Inputs in kSampled before and after: depth (D32) and the
//			octahedral normal (RGBA16F: oct xy, roughness, radius); the output
//			(RGBA16F, visibility in r) in `outputUsage` before and after.
//
//=============================================================================//

#ifndef RENDER_PASS_AO_AO_H
#define RENDER_PASS_AO_AO_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/math/matrix.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace render::pass::ao
{

struct AoParams
{
	float radius = 48.0f;       // Source units, where a pixel's own radius is 0
	float falloff = 0.6f;       // of the radius: where a sample's weight starts to fade
	std::uint32_t slices = 8;   // directions per pixel
	std::uint32_t steps = 8;    // samples per direction each way
	std::uint32_t blurRadius = 3; // texels; 0 leaves the raw visibility
	// Integrate once per 2 x 2 pixels, and upsample in the blur (depth- and
	// normal-aware): a quarter of the integration's cost.
	bool halfResolution = false;
};

enum class AoStatus : std::uint8_t
{
	kInvalidParams,
	kInvalidTargets,
	kDevice,
};

struct AoView
{
	math::float4x4 view;       // world to view (rows: right, up, -forward)
	math::float4x4 projection; // math::Perspective's depth range
	float eye[3] = { 0.0f, 0.0f, 0.0f };
};

struct AoTargets
{
	device::TextureId depth;
	device::TextureId normalRoughness;
	device::TextureId output;
	device::ResourceUsage outputUsage = device::ResourceUsage::kSampled;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

class AmbientOcclusion
{
public:
	static foundation::Expected<std::unique_ptr<AmbientOcclusion>, AoStatus> Create(
	    device::IRenderDevice2 &device, const AoParams &params = {} );
	// Test endpoint: the pass with another program (a suite's seeded variant
	// of render/pass/ao/gtao.comp, SPIR-V, the same interface).
	static foundation::Expected<std::unique_ptr<AmbientOcclusion>, AoStatus> CreateWithProgram(
	    device::IRenderDevice2 &device, const AoParams &params,
	    std::span<const std::uint32_t> spirv );
	~AmbientOcclusion();
	AmbientOcclusion( const AmbientOcclusion & ) = delete;
	AmbientOcclusion &operator=( const AmbientOcclusion & ) = delete;

	foundation::Expected<void, AoStatus> Record(
	    device::CommandEncoder &encoder, const AoTargets &targets, const AoView &view );
	// New parameters from the next Record (a quality setting); refused, and
	// the old kept, when they are not valid.
	foundation::Expected<void, AoStatus> SetParams( const AoParams &params );
	const AoParams &Params() const { return m_Params; }
	void Collect( device::CompletionToken token );

private:
	explicit AmbientOcclusion( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	AoParams m_Params;
	device::BindGroupLayoutId m_Layout;
	device::SamplerId m_Sampler;
	device::PipelineId m_Pipeline;
	device::BufferId m_Constants;
	device::BufferId m_Scratch;
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
	std::mutex m_PendingLock;
	std::vector<device::ResourceId> m_Pending;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::ao

#endif // RENDER_PASS_AO_AO_H
