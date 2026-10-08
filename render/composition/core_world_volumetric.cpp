//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): the volumetric medium at a slot.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

void CoreWorld::RecordVolumetric( device::CommandEncoder &encoder,
    const legacy::CorePassTarget &target, const ViewLightInputs &in )
{
	using namespace render::device;
	const auto refuse = [&]( const char *why )
	{
		if ( m_VolumetricRefused.fetch_add( 1, std::memory_order_relaxed ) == 0 )
			std::fprintf( stderr, "render core: volumetric medium refused: %s\n", why );
	};
	// The composite samples the scene's depth: a single-sample target with
	// sampled depth (the backend imports its multisampled depth without).
	if ( target.samples != 1 )
		return refuse( "the target is multisampled" );
	if ( !target.depth.IsValid() || !target.color.IsValid() )
		return refuse( "the target has no imported color or depth" );
	const float *viewport = in.viewport;
	if ( std::uint32_t( viewport[2] ) != target.width ||
	     std::uint32_t( viewport[3] ) != target.height )
		return refuse( "the view does not cover its target" );
	IRenderDevice2 &device = *target.device;
	BindStageDevice( device );
	// Linear radiance blends through the sRGB view of an 8-bit target.
	const TextureId color = target.colorSrgb.IsValid() ? target.colorSrgb : target.color;
	const Format format = target.colorSrgb.IsValid() ? target.colorSrgbFormat : target.colorFormat;
	if ( !m_Volumetric || m_VolumetricFormat != format )
	{
		auto created = pass::volumetric::VolumetricRenderer::Create( device, format );
		if ( !created )
			return refuse( "the volumetric pass was refused by the device" );
		m_Volumetric = std::move( created ).Value();
		m_VolumetricFormat = format;
	}
	if ( target.frame != m_VolumetricFrame )
	{
		m_Volumetric->Collect( target.submitted );
		m_VolumetricFrame = target.frame;
	}

	// The froxels: this view's light grid (render.pass.lights, the owner of
	// the depth split) subdivided 8 x 4, as render_lab's.
	auto matrix = []( const float m[16] )
	{
		math::float4x4 out;
		for ( int r = 0; r < 4; ++r )
			out.rows[r] = { m[r * 4 + 0], m[r * 4 + 1], m[r * 4 + 2], m[r * 4 + 3] };
		return out;
	};
	pass::lights::ClusterViewDesc desc;
	desc.view = matrix( in.worldToView );
	desc.projection = matrix( in.viewToClip );
	desc.widthPixels = target.width;
	desc.heightPixels = target.height;
	// The medium's slices span the shared fog range, not the view's planes.
	desc.nearZ = kFroxelNearZ;
	desc.farZ = kFroxelFarZ;
	auto grid = pass::lights::CreateClusterGrid( desc, pass::lights::DesktopClusterLimits() );
	if ( !grid )
		return refuse( "the view's light grid does not build" );
	auto fine = pass::lights::SubdivideClusterGrid(
	    grid.Value(), kFroxelTileDivisor, kFroxelSliceMultiplier );
	if ( !fine )
		return refuse( "the light grid does not subdivide" );
	const pass::volumetric::FroxelLayout layout = FroxelLayoutOf( fine.Value() );

	// The medium in the unit the surfaces draw: lights and emission times
	// the output's linear (tone-mapping) scale; transmittance is unitless.
	const float scale = target.outputScale;
	pass::volumetric::Medium medium = m_Media->medium;
	for ( pass::volumetric::FogVolume &volume : medium.volumes )
		volume.emission = volume.emission * scale;
	std::uint32_t unsupported = 0;
	std::vector<pass::volumetric::MediumLight> lights = MediumLightsFrom( in.lights, &unsupported );
	for ( pass::volumetric::MediumLight &light : lights )
		light.color = light.color * scale;
	pass::volumetric::VolumetricView view;
	view.froxels = &layout;
	view.projection = desc.projection;
	{
		// The eye from the world-to-view matrix: -R^T t.
		const math::float4x4 &v = desc.view;
		const float t[3] = { v.rows[0].w, v.rows[1].w, v.rows[2].w };
		view.eye = { -( v.rows[0].x * t[0] + v.rows[1].x * t[1] + v.rows[2].x * t[2] ),
		    -( v.rows[0].y * t[0] + v.rows[1].y * t[1] + v.rows[2].y * t[2] ),
		    -( v.rows[0].z * t[0] + v.rows[1].z * t[1] + v.rows[2].z * t[2] ) };
	}
	pass::volumetric::VolumetricFrame frame;
	frame.medium = &medium;
	frame.lights = lights;
	frame.sampling = kFroxelSampling;
	pass::volumetric::VolumetricTargets targets;
	targets.color = color;
	targets.depth = target.depth;
	targets.depthUsage = ResourceUsage::kDepthWrite;
	targets.width = target.width;
	targets.height = target.height;
	if ( !m_Volumetric->Record( encoder, view, frame, targets ) )
		return refuse( "the volumetric pass did not record" );
	if ( m_VolumetricViews.fetch_add( 1, std::memory_order_relaxed ) == 0 )
		std::fprintf( stderr,
		    "render core: volumetric medium over %ux%u: %zu lights (%u unsupported), froxels "
		    "%ux%ux%u; projectors not in the medium yet\n",
		    target.width, target.height, lights.size(), unsupported, layout.tilesX, layout.tilesY,
		    layout.slices );
}

} // namespace render::composition
