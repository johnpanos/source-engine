//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A map's authored lights (RFC 0016 K12); see
//			public/render/pass/lights/map_lights.h.
//
//=============================================================================//

#include "render/pass/lights/map_lights.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace render::pass::lights
{

namespace
{

constexpr double kPi = 3.14159265358979323846;

} // namespace

// The values of a key as floats ( "a b c" ), padded with `fill`.
std::vector<float> Numbers(
    const Entity &entity, const std::string &key, std::size_t count, float fill )
{
	std::vector<float> values;
	const auto found = entity.find( key );
	if ( found != entity.end() )
	{
		const char *at = found->second.c_str();
		char *end = nullptr;
		for ( ;; )
		{
			const float value = std::strtof( at, &end );
			if ( end == at )
				break;
			values.push_back( value );
			at = end;
		}
	}
	values.resize( std::max( values.size(), count ), fill );
	return values;
}

bool Has( const Entity &entity, const std::string &key )
{
	return entity.count( key ) != 0;
}

std::string Value( const Entity &entity, const std::string &key )
{
	const auto found = entity.find( key );
	return found == entity.end() ? std::string() : found->second;
}

math::float3 Vector3( const std::vector<float> &v )
{
	return { v[0], v[1], v[2] };
}

// GammaToLinear( rgb / 255 ) times a brightness / 255.
math::float3 GammaColor( const std::vector<float> &light )
{
	const auto channel = []( float c )
	{
		return float( std::pow( double( c ) / 255.0, 2.2 ) );
	};
	const float scale = light[3] / 255.0f;
	return {
	    channel( light[0] ) * scale, channel( light[1] ) * scale, channel( light[2] ) * scale };
}

// The engine's AngleVectors (pitch positive looks down).
void AngleVectors(
    const std::vector<float> &angles, math::float3 &forward, math::float3 &right, math::float3 &up )
{
	const double p = angles[0] * kPi / 180.0;
	const double y = angles[1] * kPi / 180.0;
	const double r = angles[2] * kPi / 180.0;
	const double sp = std::sin( p ), cp = std::cos( p );
	const double sy = std::sin( y ), cy = std::cos( y );
	const double sr = std::sin( r ), cr = std::cos( r );
	forward = { float( cp * cy ), float( cp * sy ), float( -sp ) };
	right = {
	    float( -sr * sp * cy + cr * sy ), float( -sr * sp * sy - cr * cy ), float( -sr * cp ) };
	up = { float( cr * sp * cy + sr * sy ), float( cr * sp * sy - sr * cy ), float( cr * cp ) };
}

std::optional<std::vector<Entity>> ParseEntityLump( const std::string &text )
{
	std::vector<Entity> entities;
	std::size_t at = 0;
	const auto skipSpace = [&]()
	{
		while ( at < text.size() &&
		        ( std::isspace( static_cast<unsigned char>( text[at] ) ) || text[at] == '\0' ) )
			++at;
	};
	const auto quoted = [&]( std::string &out ) -> bool
	{
		skipSpace();
		if ( at >= text.size() || text[at] != '"' )
			return false;
		const std::size_t end = text.find( '"', at + 1 );
		if ( end == std::string::npos )
			return false;
		out = text.substr( at + 1, end - at - 1 );
		at = end + 1;
		return true;
	};
	for ( ;; )
	{
		skipSpace();
		if ( at >= text.size() )
			return entities;
		if ( text[at] != '{' )
			return std::nullopt;
		++at;
		Entity entity;
		for ( ;; )
		{
			skipSpace();
			if ( at < text.size() && text[at] == '}' )
			{
				++at;
				break;
			}
			std::string key, value;
			if ( !quoted( key ) || !quoted( value ) )
				return std::nullopt;
			entity.emplace( key, value ); // the first of a repeated key wins
		}
		entities.push_back( std::move( entity ) );
	}
}

MapLights MapLightsFromEntities( const std::vector<Entity> &entities )
{
	MapLights out;
	std::uint32_t nextId = 1;
	for ( const Entity &entity : entities )
	{
		const std::string classname = Value( entity, "classname" );
		const math::float3 origin = Vector3( Numbers( entity, "origin", 3 ) );
		if ( classname == "light" || classname == "light_spot" )
		{
			// vrad's nonzero _fifty_percent_distance solves a separate
			// inverse-quadratic curve. Until that solver is in this owner, do
			// not claim a light with a different compiled falloff.
			if ( Numbers( entity, "_fifty_percent_distance", 1 )[0] > 0.0f )
			{
				++out.unsupported;
				continue;
			}
			float attn[3] = { Numbers( entity, "_constant_attn", 1 )[0],
			    Numbers( entity, "_linear_attn", 1 )[0],
			    Numbers( entity, "_quadratic_attn", 1 )[0] };
			for ( float &term : attn )
				term = term >= 1e-6f && std::isfinite( term ) ? term : 0.0f;
			if ( attn[0] == 0.0f && attn[1] == 0.0f && attn[2] == 0.0f )
				attn[0] = 1.0f;
			const float reference = attn[0] + 100.0f * attn[1] + 10000.0f * attn[2];
			if ( !std::isfinite( reference ) || reference <= 0.0f )
			{
				++out.unsupported;
				continue;
			}
			light_set::RuntimeLight light;
			light.id = nextId++;
			light.kind = light_set::LightKind::World;
			light.baked = true;
			light.matchesBaked = true;
			light.style = int( Numbers( entity, "style", 1 )[0] );
			const bool inverseSquare = attn[0] == 0.0f && attn[1] == 0.0f && attn[2] == 1.0f;
			light.falloff = inverseSquare ? light_set::LightFalloff::InverseSquare
			                              : light_set::LightFalloff::Attenuated;
			light.sourceRadius = light_set::kInverseSquareSourceRadius;
			light.position[0] = origin.x;
			light.position[1] = origin.y;
			light.position[2] = origin.z;
			const math::float3 color = GammaColor( Numbers( entity, "_light", 4 ) );
			light.color[0] = color.x;
			light.color[1] = color.y;
			light.color[2] = color.z;
			if ( !inverseSquare )
			{
				for ( int c = 0; c < 3; ++c )
					light.color[c] *= reference;
				std::copy( attn, attn + 3, light.attenuation );
			}
			// The bake has no window: the radius where the light's diffuse
			// light falls to 1e-4 of the lightmap unit, so the window's error
			// is below that.
			const float peak = std::max( { color.x, color.y, color.z } );
			light.radius = Numbers( entity, "_distance", 1 )[0];
			if ( inverseSquare && light.radius <= 0.0f )
				light.radius = float( light_set::kInverseSquareReferenceDistance *
				                      std::sqrt( std::max( peak, 1e-6f ) / 1e-4f ) );
			if ( classname == "light_spot" )
			{
				const std::vector<float> angles = Numbers( entity, "angles", 3 );
				const float pitch =
				    Has( entity, "pitch" ) ? Numbers( entity, "pitch", 1 )[0] : angles[0];
				const double p = pitch * kPi / 180.0;
				const double y = angles[1] * kPi / 180.0;
				light.direction[0] = float( std::cos( y ) * std::cos( p ) );
				light.direction[1] = float( std::sin( y ) * std::cos( p ) );
				light.direction[2] = float( std::sin( p ) );
				float inner = Numbers( entity, "_inner_cone", 1, 10.0f )[0];
				float outer = Numbers( entity, "_cone", 1, 0.0f )[0];
				if ( outer == 0.0f )
					outer = inner;
				outer = std::max( outer, inner );
				if ( !( inner == 180.0f && outer == 180.0f ) )
				{
					light.shape = light_set::LightShape::Spot;
					light.innerCos = float( std::cos( std::min( inner, 90.0f ) * kPi / 180.0 ) );
					light.outerCos = float( std::cos( std::min( outer, 90.0f ) * kPi / 180.0 ) );
					light.spotExponent = Numbers( entity, "_exponent", 1, 0.0f )[0];
				}
			}
			out.lights.push_back( light );
		}
		else if ( classname == "light_rect" )
		{
			// halfU = -right x width / 2, halfV = up x height / 2, so halfU x
			// halfV is forward, the emission direction.
			math::float3 forward, right, up;
			AngleVectors( Numbers( entity, "angles", 3 ), forward, right, up );
			const float width = Numbers( entity, "width", 1 )[0];
			const float height = Numbers( entity, "height", 1 )[0];
			area_light::AreaLight light;
			light.rect.center[0] = origin.x;
			light.rect.center[1] = origin.y;
			light.rect.center[2] = origin.z;
			light.rect.halfU[0] = -right.x * width * 0.5f;
			light.rect.halfU[1] = -right.y * width * 0.5f;
			light.rect.halfU[2] = -right.z * width * 0.5f;
			light.rect.halfV[0] = up.x * height * 0.5f;
			light.rect.halfV[1] = up.y * height * 0.5f;
			light.rect.halfV[2] = up.z * height * 0.5f;
			light.rect.twoSided = Value( entity, "two_sided" ) == "1";
			const std::vector<float> color = Numbers( entity, "color", 3 );
			const float brightness = Numbers( entity, "brightness", 1, 1.0f )[0];
			for ( int c = 0; c < 3; ++c )
				light.radiance[c] = color[std::size_t( c )] / 255.0f * brightness;
			light.reach = area_light::Reach( light.rect, light.radiance );
			out.areas.push_back( light );
		}
		else if ( classname == "light_environment" )
		{
			// vrad (SetupLightNormalFromProps): the light's normal is
			// ( cos yaw cos pitch, sin yaw cos pitch, sin pitch ) with the
			// pitch key's pitch (else the angles'); the sun shines along it.
			const std::vector<float> angles = Numbers( entity, "angles", 3 );
			const float pitch =
			    Has( entity, "pitch" ) ? Numbers( entity, "pitch", 1 )[0] : angles[0];
			const double p = pitch * kPi / 180.0;
			const double y = angles[1] * kPi / 180.0;
			const math::float3 forward{ float( std::cos( y ) * std::cos( p ) ),
			    float( std::sin( y ) * std::cos( p ) ), float( std::sin( p ) ) };
			MapSun sun;
			sun.toSun = { -forward.x, -forward.y, -forward.z };
			sun.color = GammaColor( Numbers( entity, "_light", 4 ) );
			sun.ambient = GammaColor( Numbers( entity, "_ambient", 4 ) );
			sun.spreadDegrees = Numbers( entity, "SunSpreadAngle", 1, 0.0f )[0];
			out.sun = sun;
		}
		else if ( classname == "env_projectedtexture" )
		{
			projected_light::Light light;
			math::float3 forward, right, up;
			AngleVectors( Numbers( entity, "angles", 3 ), forward, right, up );
			const float axes[3][3] = { { forward.x, forward.y, forward.z },
			    { right.x, right.y, right.z }, { up.x, up.y, up.z } };
			std::memcpy( light.forward, axes[0], sizeof( light.forward ) );
			std::memcpy( light.right, axes[1], sizeof( light.right ) );
			std::memcpy( light.up, axes[2], sizeof( light.up ) );
			light.origin[0] = origin.x;
			light.origin[1] = origin.y;
			light.origin[2] = origin.z;
			light.horizontalFovDegrees = light.verticalFovDegrees =
			    Numbers( entity, "lightfov", 1, 90.0f )[0];
			light.nearZ = Numbers( entity, "nearz", 1, 4.0f )[0];
			light.farZ = Numbers( entity, "farz", 1, 750.0f )[0];
			const math::float3 color = GammaColor( Numbers( entity, "lightcolor", 4 ) );
			light.color[0] = color.x;
			light.color[1] = color.y;
			light.color[2] = color.z;
			light.shadows = Value( entity, "enableshadows" ) != "0";
			light.lightsWorld = Value( entity, "lightworld" ) != "0";
			out.projectors.push_back( light );
			out.cookieNames.push_back( Value( entity, "texturename" ) );
		}
	}
	return out;
}

std::vector<light_set::RuntimeLight> MergeMapLights(
    const std::vector<light_set::RuntimeLight> &frame, const MapLights &map )
{
	std::vector<light_set::RuntimeLight> result = frame;
	std::uint32_t nextId = 0x40000000u;
	for ( light_set::RuntimeLight light : map.lights )
	{
		// A style-controlled lamp belongs to the engine's live light set.
		// A fallback copy would keep it on after the game switches it off.
		if ( light.style != 0 )
			continue;
		const bool present = std::any_of( frame.begin(), frame.end(),
		    [&]( const light_set::RuntimeLight &other )
		    {
			    if ( other.kind != light_set::LightKind::World || other.shape != light.shape )
				    return false;
			    for ( int axis = 0; axis < 3; ++axis )
				    if ( std::fabs( other.position[axis] - light.position[axis] ) > 0.1f )
					    return false;
			    return true;
		    } );
		if ( !present )
		{
			light.id = nextId++;
			result.push_back( light );
		}
	}
	return result;
}

} // namespace render::pass::lights
