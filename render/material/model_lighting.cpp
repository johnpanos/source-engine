//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Source's model lighting packing (RFC 0016 K11); see
//			model_lighting.h.
//
//=============================================================================//

#include "render/material/model_lighting.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace render::material
{

namespace
{

constexpr float kPi = 3.14159265358979323846f;

int TypeOrder( ModelLightType type )
{
	return type == ModelLightType::kSpot ? 0 : ( type == ModelLightType::kPoint ? 1 : 2 );
}

} // namespace

ModelLighting PackSourceModelLighting(
    const float eye[3], const float cube[6][3], std::span<const ModelLightDesc> lights )
{
	ModelLighting packed;
	for ( int c = 0; c < 3; ++c )
		packed.eye[c] = eye[c];
	for ( int face = 0; face < 6; ++face )
	{
		for ( int c = 0; c < 3; ++c )
			packed.cube[face][c] = cube[face][c];
	}
	// SortLights: an insertion sort by type, stable for equal types.
	std::array<const ModelLightDesc *, kMaxModelLights> order = {};
	std::size_t count = 0;
	for ( const ModelLightDesc &light : lights )
	{
		if ( count == kMaxModelLights )
			break;
		std::size_t j = count;
		while ( j > 0 && TypeOrder( order[j - 1]->type ) > TypeOrder( light.type ) )
		{
			order[j] = order[j - 1];
			--j;
		}
		order[j] = &light;
		++count;
	}
	packed.eye[3] = float( count );
	for ( std::size_t n = 0; n < count; ++n )
	{
		const ModelLightDesc &light = *order[n];
		ModelLighting::Light &out = packed.lights[n];
		const bool spot = light.type == ModelLightType::kSpot;
		for ( int c = 0; c < 3; ++c )
		{
			out.color[c] = light.color[c];
			out.direction[c] = light.direction[c];
			out.position[c] = light.position[c];
			out.attenuation[c] = light.attenuation[c];
		}
		out.color[3] = light.type == ModelLightType::kDirectional ? 1.0f : 0.0f;
		out.direction[3] = spot ? 1.0f : 0.0f;
		if ( spot )
		{
			// CShaderAPIDx8::SetLight's cone adjustment.
			const float phi = std::min( light.phi, kPi );
			const float theta = light.theta - phi > -1e-3f ? phi - 1e-3f : light.theta;
			const float stopdot = std::cos( theta * 0.5f );
			const float stopdot2 = std::cos( phi * 0.5f );
			out.spot[0] = light.falloff;
			out.spot[1] = stopdot;
			out.spot[2] = stopdot2;
			out.spot[3] = stopdot > stopdot2 ? 1.0f / ( stopdot - stopdot2 ) : 0.0f;
		}
		else
		{
			out.spot[1] = out.spot[2] = out.spot[3] = 1.0f;
		}
	}
	return packed;
}

} // namespace render::material
