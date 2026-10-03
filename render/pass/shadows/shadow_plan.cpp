//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frame's shadow plan (RFC 0016 render.shadows.v1); see
//			public/render/pass/shadows/shadow_plan.h.
//
//=============================================================================//

#include "render/pass/shadows/shadow_plan.h"

#include "render/pass/shadows/shadow_passes.h"
#include "render/pass/shadows/shadow_views.h"

#include <algorithm>
#include <cmath>

namespace render::pass::shadows
{

namespace
{

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
std::optional<ShadowView> Face( const math::float3 &origin, const math::float3 &forward,
    const math::float3 &up, float nearZ, float farZ )
{
	FlashlightShadowDesc desc;
	desc.position = origin;
	desc.forward = forward;
	desc.up = up;
	desc.horizontalFovRadians = desc.verticalFovRadians =
	    ( 90.0f + 2.0f * kFaceMarginDegrees ) * kPi / 180.0f;
	desc.nearZ = nearZ;
	desc.farZ = std::max( farZ, nearZ * 2.0f );
	auto view = BuildFlashlightShadowView( desc );
	if ( !view )
		return std::nullopt;
	return view.Value();
}

} // namespace

std::optional<std::string> PlanShadows( const ShadowPlanInput &input, ShadowPlan &out )
{
	out = ShadowPlan();
	out.atlasSize = input.atlasSize;
	out.guardTexels = input.guardTexels;
	const std::uint32_t atlasSize = input.atlasSize;
	const std::uint32_t guard = input.guardTexels;
	out.lightTiles.assign( input.lights.size(), -1 );
	out.lightLayouts.assign( input.lights.size(), RuntimeShadowLayout::kSingle );
	out.areaTiles.assign( input.areas.size(), -1 );
	out.projectorTiles.assign( input.projectors.size(), -1 );
	std::vector<Pending> pending;
	auto add = [&]( const ShadowView &view, bool orthographic, float coverage, float priority,
	               int *slot, bool first )
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
	if ( input.toSun )
	{
		CascadeDesc desc;
		desc.cameraView = input.camera.view;
		desc.verticalFovRadians = input.camera.verticalFovRadians;
		desc.aspect = input.camera.aspect;
		desc.nearZ = input.camera.nearZ;
		desc.shadowDistance = input.camera.shadowDistance;
		desc.cascadeCount = 4;
		desc.resolution = 2048 - 2 * guard;
		const math::float3 toSun = *input.toSun;
		desc.lightDirection = { -toSun.x, -toSun.y, -toSun.z };
		desc.casterDistance = input.camera.shadowDistance;
		auto cascades = BuildCascades( desc );
		if ( !cascades )
			return std::string( "the sun's cascades do not build" );
		for ( std::uint32_t c = 0; c < cascades.Value().count; ++c )
			add( cascades.Value().cascades[c].view, true, 1.0f, 100.0f - float( c ), &sunSlot,
			    c == 0 );
		out.sunCount = int( cascades.Value().count );
	}
	for ( std::size_t i = 0; i < input.projectors.size(); ++i )
	{
		const projected_light::Light &light = input.projectors[i];
		if ( !light.shadows )
			continue;
		FlashlightShadowDesc desc;
		desc.position = Vec( light.origin );
		desc.forward = Vec( light.forward );
		desc.up = Vec( light.up );
		desc.horizontalFovRadians = light.horizontalFovDegrees * kPi / 180.0f;
		desc.verticalFovRadians = light.verticalFovDegrees * kPi / 180.0f;
		desc.nearZ = std::max( light.nearZ, 1.0f );
		desc.farZ = light.farZ;
		auto view = BuildFlashlightShadowView( desc );
		if ( view )
			add( view.Value(), false, 0.5f, 90.0f, &out.projectorTiles[i], true );
	}
	for ( std::size_t i = 0; i < input.lights.size(); ++i )
	{
		const light_set::RuntimeLight &light = input.lights[i];
		const math::float3 position = Vec( light.position );
		const float far = light.radius > 0.0f ? light.radius : 4096.0f;
		const bool spot =
		    light.shape == light_set::LightShape::Spot &&
		    std::acos( std::clamp( light.outerCos, -1.0f, 1.0f ) ) < kMaxSpotHalfAngle;
		if ( spot )
		{
			SpotShadowDesc desc;
			desc.position = position;
			desc.direction = math::Normalize( Vec( light.direction ) );
			desc.outerCos = light.outerCos;
			desc.nearZ = 2.0f;
			desc.range = far;
			auto view = BuildSpotShadowView( desc );
			if ( view )
				add( view.Value(), false, 0.5f, 50.0f, &out.lightTiles[i], true );
			continue;
		}
		// A point light: the six faces of a cube.
		static const math::float3 kAxes[6] = {
		    { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
		bool all = true;
		std::vector<ShadowView> faces;
		for ( const math::float3 &axis : kAxes )
		{
			auto face = Face( position, axis, AnyPerpendicular( axis ), 2.0f, far );
			all = all && face.has_value();
			if ( face )
				faces.push_back( *face );
		}
		if ( !all )
			continue;
		out.lightLayouts[i] = RuntimeShadowLayout::kWorldCube;
		for ( std::size_t f = 0; f < faces.size(); ++f )
			add( faces[f], false, 0.0625f, 10.0f, &out.lightTiles[i], f == 0 );
	}
	for ( std::size_t i = 0; i < input.areas.size(); ++i )
	{
		const area_light::AreaLight &light = input.areas[i];
		const math::float3 u = Vec( light.rect.halfU );
		const math::float3 v = Vec( light.rect.halfV );
		const math::float3 n = math::Normalize( math::Cross( u, v ) );
		const math::float3 uAxis = math::Normalize( u );
		const math::float3 vAxis = math::Normalize( v );
		// The views start just off the emitter, so the surface it is set
		// into does not shadow it.
		const math::float3 origin = Vec( light.rect.center ) + n * 1.0f;
		const float far = light.reach > 0.0f ? light.reach : 768.0f;
		std::vector<std::pair<math::float3, math::float3>> directions = {
		    { n, vAxis }, { uAxis, n }, { uAxis * -1.0f, n }, { vAxis, n }, { vAxis * -1.0f, n } };
		if ( light.rect.twoSided )
			directions.push_back( { n * -1.0f, vAxis } );
		std::vector<ShadowView> faces;
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
	ShadowAtlasLimits limits;
	limits.atlasSize = atlasSize;
	limits.minTileSize = 64;
	limits.maxTileSize = 2048;
	limits.casterBudget = 4096;
	limits.guardTexels = guard;
	std::vector<ShadowRequest> requests;
	for ( std::size_t i = 0; i < pending.size(); ++i )
		requests.push_back( { std::uint64_t( i + 1 ), pending[i].priority, pending[i].coverage } );
	auto plan = PlanShadowAtlas( limits, requests );
	if ( !plan )
		return std::string( "the shadow atlas plan was refused" );
	for ( std::size_t i = 0; i < pending.size(); )
	{
		// The group: this view and the faces after it that share its slot.
		std::size_t end = i + 1;
		while (
		    end < pending.size() && !pending[end].first && pending[end].slot == pending[i].slot )
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
				const ShadowTile &tile = plan.Value().allocations[k].tile;
				ShadowTileGpu gpu =
				    PackShadowTile( MakeTileProjection( p.viewProjection, tile, atlasSize, guard ),
				        atlasSize, p.orthographic ? kOrthographicBias : kPerspectiveBias );
				gpu.params[2] = p.orthographic ? -1.0f : p.nearZ;
				gpu.params[3] = p.orthographic ? p.farZ - p.nearZ : p.farZ;
				out.tiles.push_back( gpu );
				out.views.push_back( { p.viewProjection, tile } );
			}
		}
		i = end;
	}
	if ( sunSlot >= 0 )
		out.sunFirst = sunSlot;
	else
		out.sunCount = 0;
	return std::nullopt;
}

} // namespace render::pass::shadows
