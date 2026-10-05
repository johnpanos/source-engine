//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A map's participating media (RFC 0016 K11/K12); see
//			render/composition/map_media.h.
//
//=============================================================================//

#include "render/composition/map_media.h"

#include "render/light_set.h"
#include "render/projected_light.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace render::composition
{

namespace
{

namespace volumetric = render::pass::volumetric;

using namespace render::pass::lights;


} // namespace

std::vector<volumetric::MediumLight> MediumLightsFrom(
    std::span<const light_set::RuntimeLight> lights, std::uint32_t *unsupported )
{
	std::vector<volumetric::MediumLight> out;
	out.reserve( std::min<std::size_t>( lights.size(), volumetric::kMaxMediumLights ) );
	for ( const light_set::RuntimeLight &light : lights )
	{
		const bool shape = light.shape == light_set::LightShape::Point ||
		                   light.shape == light_set::LightShape::Spot;
		if ( !shape || light.falloff == light_set::LightFalloff::Attenuated ||
		     out.size() == volumetric::kMaxMediumLights )
		{
			if ( unsupported )
				++*unsupported;
			continue;
		}
		out.push_back( volumetric::MediumLightFrom( light ) );
	}
	return out;
}

MapMedia MediaFromEntities( const std::vector<Entity> &entities, EntityConvention convention )
{
	MapMedia media;
	for ( const Entity &entity : entities )
	{
		const std::string classname = Value( entity, "classname" );
		const math::float3 origin = Vector3( Numbers( entity, "origin", 3 ) );
		if ( classname == "env_volumetric_fog_volume" )
		{
			media.present = true;
			volumetric::FogVolume volume;
			const math::float3 lo = Vector3( Numbers( entity, "box_mins", 3 ) );
			const math::float3 hi = Vector3( Numbers( entity, "box_maxs", 3 ) );
			volume.mins = origin + lo;
			volume.maxs = origin + hi;
			volume.extinction = Numbers( entity, "density", 1 )[0];
			volume.albedo = Numbers( entity, "albedo", 1, 1.0f )[0];
			volume.anisotropy = Numbers( entity, "anisotropy", 1 )[0];
			volume.emission = Vector3( Numbers( entity, "emission", 3 ) );
			media.medium.volumes.push_back( volume );
		}
		else if ( classname == "env_volumetric_fog_controller" )
		{
			media.present = true;
			volumetric::HeightFog &fog = media.medium.fog;
			fog.density = Numbers( entity, "density", 1 )[0];
			fog.heightDensity = Numbers( entity, "height_fog_density", 1 )[0];
			fog.heightFalloff = Numbers( entity, "height_fog_falloff", 1 )[0];
			fog.baseHeight = Has( entity, "origin" ) ? origin.z : 0.0f;
			fog.albedo = Numbers( entity, "albedo", 1, 1.0f )[0];
			fog.anisotropy = Numbers( entity, "anisotropy", 1 )[0];
		}
	}
	// The lights and projectors are render.pass.lights' (the surfaces'
	// own, windows included), so the medium and the surfaces see one set.
	const MapLights map = MapLightsFromEntities( entities, convention );
	media.lights = MediumLightsFrom( map.lights, &media.unsupportedLights );
	media.unsupportedLights += map.unsupported;
	for ( std::size_t i = 0; i < map.projectors.size(); ++i )
	{
		volumetric::MediumProjector projector;
		projector.light = map.projectors[i];
		projector.cookieLayer = std::uint32_t( i );
		media.projectors.push_back( projector );
		media.cookieNames.push_back( i < map.cookieNames.size() ? map.cookieNames[i] : "" );
	}
	return media;
}

pass::volumetric::FroxelLayout FroxelLayoutOf( const pass::lights::ClusterGrid &grid )
{
	pass::volumetric::FroxelLayout layout;
	layout.tilesX = grid.tilesX;
	layout.tilesY = grid.tilesY;
	layout.slices = grid.slices;
	layout.tileSizePixels = grid.limits.tileSizePixels;
	layout.widthPixels = grid.widthPixels;
	layout.heightPixels = grid.heightPixels;
	layout.sliceScale = grid.sliceScale;
	layout.sliceBias = grid.sliceBias;
	layout.nearZ = grid.nearZ;
	layout.farZ = grid.farZ;
	layout.sliceDepths = grid.sliceDepths;
	layout.view = grid.view;
	layout.rayTopLeft = grid.cornerRays.front();
	layout.rayBottomRight = grid.cornerRays.back();
	return layout;
}

} // namespace render::composition
