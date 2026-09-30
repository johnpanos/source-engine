//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shadows (RFC 0016 K11); see lab_shadows.h.
//
//=============================================================================//

#include "lab_shadows.h"

#include "render/graph/graph_builder.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/pass/shadows/atlas.h"
#include "render/pass/shadows/shadow_views.h"

#include <algorithm>
#include <cmath>

namespace render::lab
{

namespace
{

using namespace render::device;
namespace shadows = render::pass::shadows;

constexpr std::uint32_t kAtlasSize = 8192;
constexpr std::uint32_t kGuard = 4;
// Cube faces are this many degrees wider than 90 on each side.
constexpr float kFaceMarginDegrees = 12.0f;
constexpr float kPi = 3.14159265358979323846f;
// Receiver depth bias in clip depth (the soft shadows' normal offset
// carries most of the separation).
constexpr float kPerspectiveBias = 5e-5f;
constexpr float kOrthographicBias = 2e-5f;

struct Pending
{
	math::float4x4 viewProjection;
	float nearZ = 0.0f;
	float farZ = 0.0f;
	bool orthographic = false;
	float coverage = 0.25f; // the tile size it asks for, of the largest
	float priority = 0.0f;
	int *slot = nullptr; // where its first tile index goes (the first face)
	bool first = true;
};

math::float3 Vec( const float v[3] )
{
	return { v[0], v[1], v[2] };
}

math::float3 AnyPerpendicular( const math::float3 &axis )
{
	const math::float3 other =
	    std::fabs( axis.z ) < 0.9f ? math::float3{ 0, 0, 1 } : math::float3{ 1, 0, 0 };
	return math::Normalize( math::Cross( axis, other ) );
}

// One face of a cube (or hemicube) from `origin` looking along `forward`.
std::optional<shadows::ShadowView> Face( const math::float3 &origin, const math::float3 &forward,
    const math::float3 &up, float nearZ, float farZ )
{
	shadows::FlashlightShadowDesc desc;
	desc.position = origin;
	desc.forward = forward;
	desc.up = up;
	desc.horizontalFovRadians = desc.verticalFovRadians =
	    ( 90.0f + 2.0f * kFaceMarginDegrees ) * kPi / 180.0f;
	desc.nearZ = nearZ;
	desc.farZ = std::max( farZ, nearZ * 2.0f );
	auto view = shadows::BuildFlashlightShadowView( desc );
	if ( !view )
		return std::nullopt;
	return view.Value();
}

} // namespace

std::optional<std::string> DrawShadows( IRenderDevice2 &device,
    shadows::ShadowDepthRenderer &renderer, const LabLights &lights,
    std::span<const shadows::ShadowCaster> casters, const LabShadowCamera &camera, LabShadows &out )
{
	out = LabShadows();
	out.lightTiles.assign( lights.lights.size(), -1 );
	out.lightTileCount.assign( lights.lights.size(), 1 );
	out.areaTiles.assign( lights.areas.size(), -1 );
	out.projectorTiles.assign( lights.projectors.size(), -1 );
	std::vector<Pending> pending;
	auto add = [&]( const shadows::ShadowView &view, bool orthographic, float coverage,
	               float priority, int *slot, bool first )
	{
		Pending p;
		p.viewProjection = view.viewProjection;
		p.nearZ = view.nearZ;
		p.farZ = view.farZ;
		p.orthographic = orthographic;
		p.coverage = coverage;
		p.priority = priority;
		p.slot = slot;
		p.first = first;
		pending.push_back( p );
	};

	// The sun's cascades first: they cover the whole view.
	int sunSlot = -1;
	if ( lights.sun )
	{
		shadows::CascadeDesc desc;
		desc.cameraView = camera.view;
		desc.verticalFovRadians = camera.verticalFovRadians;
		desc.aspect = camera.aspect;
		desc.nearZ = camera.nearZ;
		desc.shadowDistance = camera.shadowDistance;
		desc.cascadeCount = 4;
		desc.resolution = 2048 - 2 * kGuard;
		const math::float3 toSun = lights.sun->toSun;
		desc.lightDirection = { -toSun.x, -toSun.y, -toSun.z };
		desc.casterDistance = camera.shadowDistance;
		auto cascades = shadows::BuildCascades( desc );
		if ( !cascades )
			return std::string( "the sun's cascades do not build" );
		for ( std::uint32_t c = 0; c < cascades.Value().count; ++c )
			add( cascades.Value().cascades[c].view, true, 1.0f, 100.0f - float( c ), &sunSlot,
			    c == 0 );
		out.sunCount = int( cascades.Value().count );
	}
	for ( std::size_t i = 0; i < lights.projectors.size(); ++i )
	{
		const projected_light::Light &light = lights.projectors[i];
		if ( !light.shadows )
			continue;
		shadows::FlashlightShadowDesc desc;
		desc.position = Vec( light.origin );
		desc.forward = Vec( light.forward );
		desc.up = Vec( light.up );
		desc.horizontalFovRadians = light.horizontalFovDegrees * kPi / 180.0f;
		desc.verticalFovRadians = light.verticalFovDegrees * kPi / 180.0f;
		desc.nearZ = std::max( light.nearZ, 1.0f );
		desc.farZ = light.farZ;
		auto view = shadows::BuildFlashlightShadowView( desc );
		if ( view )
			add( view.Value(), false, 0.5f, 90.0f, &out.projectorTiles[i], true );
	}
	for ( std::size_t i = 0; i < lights.lights.size(); ++i )
	{
		const light_set::RuntimeLight &light = lights.lights[i];
		const math::float3 position = Vec( light.position );
		const float far = light.radius > 0.0f ? light.radius : 4096.0f;
		const bool spot = light.shape == light_set::LightShape::Spot &&
		                  std::acos( std::clamp( light.outerCos, -1.0f, 1.0f ) ) <
		                      shadows::kMaxSpotHalfAngle;
		if ( spot )
		{
			shadows::SpotShadowDesc desc;
			desc.position = position;
			desc.direction = math::Normalize( Vec( light.direction ) );
			desc.outerCos = light.outerCos;
			desc.nearZ = 2.0f;
			desc.range = far;
			auto view = shadows::BuildSpotShadowView( desc );
			if ( view )
				add( view.Value(), false, 0.5f, 50.0f, &out.lightTiles[i], true );
			continue;
		}
		// A point light: the six faces of a cube.
		static const math::float3 kAxes[6] = {
		    { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
		bool all = true;
		std::vector<shadows::ShadowView> faces;
		for ( const math::float3 &axis : kAxes )
		{
			auto face = Face( position, axis, AnyPerpendicular( axis ), 2.0f, far );
			all = all && face.has_value();
			if ( face )
				faces.push_back( *face );
		}
		if ( !all )
			continue;
		out.lightTileCount[i] = 6;
		for ( std::size_t f = 0; f < faces.size(); ++f )
			add( faces[f], false, 0.0625f, 10.0f, &out.lightTiles[i], f == 0 );
	}
	for ( std::size_t i = 0; i < lights.areas.size(); ++i )
	{
		const area_light::AreaLight &light = lights.areas[i];
		const math::float3 u = Vec( light.rect.halfU );
		const math::float3 v = Vec( light.rect.halfV );
		const math::float3 n = math::Normalize( math::Cross( u, v ) );
		const math::float3 uAxis = math::Normalize( u );
		const math::float3 vAxis = math::Normalize( v );
		// The views start just off the emitter, so the surface it is set
		// into does not shadow it.
		const math::float3 origin = Vec( light.rect.center ) + n * 1.0f;
		const float far = light.reach > 0.0f ? light.reach : 768.0f;
		std::vector<std::pair<math::float3, math::float3>> directions = { { n, vAxis },
		    { uAxis, n }, { uAxis * -1.0f, n }, { vAxis, n }, { vAxis * -1.0f, n } };
		if ( light.rect.twoSided )
			directions.push_back( { n * -1.0f, vAxis } );
		std::vector<shadows::ShadowView> faces;
		for ( const auto &[forward, up] : directions )
		{
			auto face = Face( origin, forward, up, 1.0f, far );
			if ( !face )
				break;
			faces.push_back( *face );
		}
		if ( faces.size() != directions.size() )
			continue;
		for ( std::size_t f = 0; f < faces.size(); ++f )
			add( faces[f], false, 0.125f, 20.0f, &out.areaTiles[i], f == 0 );
	}
	if ( pending.empty() )
		return std::nullopt;

	// The plan: a face group must have all its faces (a light whose group
	// is cut short stays unshadowed).
	shadows::ShadowAtlasLimits limits;
	limits.atlasSize = kAtlasSize;
	limits.minTileSize = 64;
	limits.maxTileSize = 2048;
	limits.casterBudget = 4096;
	limits.guardTexels = kGuard;
	std::vector<shadows::ShadowRequest> requests;
	for ( std::size_t i = 0; i < pending.size(); ++i )
		requests.push_back( { std::uint64_t( i + 1 ), pending[i].priority, pending[i].coverage } );
	auto plan = shadows::PlanShadowAtlas( limits, requests );
	if ( !plan )
		return std::string( "the shadow atlas plan was refused" );
	std::vector<shadows::ShadowDepthView> views;
	for ( std::size_t i = 0; i < pending.size(); )
	{
		// The group: this view and the faces after it that share its slot.
		std::size_t end = i + 1;
		while ( end < pending.size() && !pending[end].first && pending[end].slot == pending[i].slot )
			++end;
		bool allTiles = true;
		for ( std::size_t k = i; k < end; ++k )
			allTiles = allTiles && plan.Value().allocations[k].HasTile();
		if ( allTiles )
		{
			*pending[i].slot = int( out.tiles.size() );
			for ( std::size_t k = i; k < end; ++k )
			{
				const Pending &p = pending[k];
				const shadows::ShadowTile &tile = plan.Value().allocations[k].tile;
				ShadowTileGpu gpu = shadows::PackShadowTile(
				    shadows::MakeTileProjection( p.viewProjection, tile, kAtlasSize, kGuard ),
				    kAtlasSize, p.orthographic ? kOrthographicBias : kPerspectiveBias );
				gpu.params[2] = p.orthographic ? -1.0f : p.nearZ;
				gpu.params[3] = p.orthographic ? p.farZ - p.nearZ : p.farZ;
				out.tiles.push_back( gpu );
				views.push_back( { p.viewProjection, tile, casters } );
			}
		}
		i = end;
	}
	if ( sunSlot >= 0 )
		out.sunFirst = sunSlot;
	else
		out.sunCount = 0;

	out.atlasDesc.format = Format::kD32Float;
	out.atlasDesc.width = out.atlasDesc.height = kAtlasSize;
	out.atlasDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
	auto atlas = device.CreateTexture( out.atlasDesc );
	if ( !atlas )
		return std::string( "the shadow atlas was refused" );
	out.atlas = atlas.Value();
	graph::GraphBuilder builder;
	const graph::ResourceRef atlasRef = builder.ImportTexture(
	    "atlas", out.atlas, out.atlasDesc, ResourceUsage::kUndefined, ResourceUsage::kSampled );
	auto stats = renderer.AddPasses( builder, { atlasRef, kAtlasSize, kGuard }, views );
	if ( !stats )
		return std::string( "the shadow depth passes were refused" );
	out.views = stats.Value().views;
	out.draws = stats.Value().draws;
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
		return std::string( "the shadow graph did not compile" );
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), device );
	if ( !executed )
		return std::string( "the shadow graph did not execute" );
	(void)device.WaitIdle();
	renderer.Collect( executed.Value().token );
	return std::nullopt;
}

void ReleaseShadows( IRenderDevice2 &device, LabShadows &shadows, CompletionToken token )
{
	if ( shadows.atlas.IsValid() )
		(void)device.Release( shadows.atlas, token );
	shadows.atlas = TextureId();
}

} // namespace render::lab
