//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.bounce (RFC 0016 render.lighting.v1, "Indirect
//			diffuse", the projected lights' share; RFC 0011 decision: a
//			projected light is never baked): the one-bounce light of the view's
//			projectors, gathered into an atlas of the probe volume's layout
//			(render.probe-volume.v1) that the surface program samples with
//			the volume's own weights and visibility (kSurfaceProbeBounce).
//
//			Each projector's reflective shadow map (Dachsbacher and
//			Stamminger 2005: its view's surface albedo and depth, drawn by the
//			caller) is a set of point lights, one per texel: a patch at the
//			texel's world position, its normal from the neighbouring depths,
//			reflecting albedo x the projector's light (render.projected-
//			light.v1: color x cookie x attenuation) over the texel's solid
//			angle times distance squared, so the patch's reflected power
//			does not depend on its orientation to the projector (the cosine
//			cancels). Every probe texel of the volume's layer 0 sums, for its
//			direction n, each patch's irradiance / pi,
//
//			    power x max(0, n_patch . -w) x max(0, n . w) / (pi r^2),
//
//			w the direction from the probe to the patch and r its distance
//			(at least a quarter of the probe spacing), weighted by the probe's
//			visibility of the patch (the volume's depth moments, Chebyshev,
//			as probe_volume.glsl weighs probes). Units: the lightmap unit
//			(irradiance / pi), as the volume's.
//
//			The atlas is written whole (zero outside the layer-0 tiles).
//
//=============================================================================//

#ifndef RENDER_PASS_BOUNCE_BOUNCE_H
#define RENDER_PASS_BOUNCE_BOUNCE_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/math/matrix.h"
#include "render/projected_light.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace render::pass::bounce
{

enum class BounceStatus : std::uint8_t
{
	kInvalidInputs,
	kDevice,
};

// One projector's reflective shadow map: its albedo (RGBA16F, rgb the
// surfaces' diffuse reflectance) and depth (D32) through `viewProjection`
// (math::Perspective's depth range), both kSampled, square.
struct ReflectiveShadowMap
{
	device::TextureId albedo;
	device::TextureId depth;
	std::uint32_t size = 0;
	math::float4x4 viewProjection;
	projected_light::LightGpu light; // origin.w: its cookie's layer
};

struct BounceInputs
{
	// The probe volume as the surface program reads it: its atlas (RGBA16F)
	// and grid table (RGBA32F), kSampled.
	device::TextureId probeAtlas;
	device::TextureDesc probeAtlasDesc;
	device::TextureId probeGrids;
	std::uint32_t gridCount = 0;
	std::uint32_t maxProbesPerGrid = 0;
	device::TextureId cookies; // the projectors' cookie array (2D array), kSampled
	std::span<const ReflectiveShadowMap> maps;
	// The bounce atlas (RGBA16F, the probe atlas's size, kStorageWrite
	// usable), in `outputUsage` before and after.
	device::TextureId output;
	device::ResourceUsage outputUsage = device::ResourceUsage::kSampled;
};

class ProjectorBounce
{
public:
	static foundation::Expected<std::unique_ptr<ProjectorBounce>, BounceStatus> Create(
	    device::IRenderDevice2 &device );
	// Test endpoint: the pass with another program (a suite's seeded variant
	// of render/pass/bounce/bounce.comp, SPIR-V, the same interface).
	static foundation::Expected<std::unique_ptr<ProjectorBounce>, BounceStatus> CreateWithProgram(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> spirv );
	~ProjectorBounce();
	ProjectorBounce( const ProjectorBounce & ) = delete;
	ProjectorBounce &operator=( const ProjectorBounce & ) = delete;

	foundation::Expected<void, BounceStatus> Record(
	    device::CommandEncoder &encoder, const BounceInputs &inputs );
	void Collect( device::CompletionToken token );

private:
	explicit ProjectorBounce( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_Layout;
	device::SamplerId m_Linear;
	device::SamplerId m_Point;
	device::PipelineId m_Gather;
	std::mutex m_PendingLock;
	std::vector<device::ResourceId> m_Pending;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::bounce

#endif // RENDER_PASS_BOUNCE_BOUNCE_H
