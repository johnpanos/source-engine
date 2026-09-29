//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.ssr (RFC 0016, render.ssr.v1); see
//			public/render/pass/ssr/ssr.h.
//
//=============================================================================//

#include "render/pass/ssr/ssr.h"

#include <cmath>

namespace render::pass::ssr
{

std::optional<std::string> ValidateParams( const SsrParams &params )
{
	if ( !( params.roughnessCutoff > 0.0f && params.roughnessCutoff <= 1.0f ) )
		return std::string( "the roughness cutoff is outside (0, 1]" );
	if ( !( params.roughnessFadeStart >= 0.0f && params.roughnessFadeStart < 1.0f ) )
		return std::string( "the roughness fade start is outside [0, 1)" );
	if ( !( params.thickness > 0.0f ) || !std::isfinite( params.thickness ) )
		return std::string( "the thickness is not positive and finite" );
	if ( !( params.edgeFade > 0.0f && params.edgeFade <= 0.5f ) )
		return std::string( "the edge fade is outside (0, 0.5]" );
	if ( params.maxSteps == 0 )
		return std::string( "a walk needs at least one step" );
	return std::nullopt;
}

} // namespace render::pass::ssr
