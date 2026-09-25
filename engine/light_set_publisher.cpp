//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Publishes the frame's runtime light set (render.light-set.v1,
//          RFC 0011) to the renderer.
//
//===========================================================================//

#include "light_set_publisher.h"

#include "indirect_light_host.h"

#include "cl_main.h"
#include "convar.h"
#include "gl_model_private.h"
#include "materialsystem/imaterialsystem.h"
#include "mathlib/mathlib.h"
#include "r_local.h"
#include "render.h"
#include "render/direct_light_selection.h"
#include "render/light_set.h"

#include <cmath>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_nMapLoadCount;
extern float g_flMinLightingValue;

namespace
{
using namespace light_set;

ConVar r_lightset_report( "r_lightset_report", "0", FCVAR_CHEAT,
    "Print the next published light set (RFC 0011 render.light-set.v1) once" );

Builder s_builder;
int s_builtMap = -1;
ILightSetConsumer *s_consumer = nullptr;

void ToLinear( const ColorRGBExp32 &color, float out[3] )
{
	// The linear unit Source adds a dlight to a lightmap in.
	out[0] = TexLightToLinear( color.r, color.exponent );
	out[1] = TexLightToLinear( color.g, color.exponent );
	out[2] = TexLightToLinear( color.b, color.exponent );
}

void GatherDynamic(
    const dlight_t *lights, int count, LightKind kind, std::vector<DynamicLightInput> *out )
{
	for ( int i = 0; i < count; ++i )
	{
		const dlight_t &dl = lights[i];
		if ( !dl.IsRadiusGreaterThanZero() )
			continue;
		DynamicLightInput input;
		input.kind = kind;
		input.slot = uint32_t( i );
		input.key = dl.key;
		for ( int k = 0; k < 3; ++k )
		{
			input.position[k] = dl.origin[k];
			input.direction[k] = dl.m_Direction[k];
		}
		ToLinear( dl.color, input.color );
		const float style =
		    dl.style >= 0 && dl.style < MAX_LIGHTSTYLES ? LightStyleValue( dl.style ) : 1.0f;
		for ( float &channel : input.color )
			channel *= style;
		input.radius = dl.GetRadius();
		input.minLight = fpmax( g_flMinLightingValue, dl.minlight );
		if ( dl.flags & DLIGHT_INVERSE_SQUARE )
			input.falloff = LightFalloff::InverseSquare;
		// m_OuterAngle 0 is a point light; below 180 degrees a spot.
		input.spot = dl.m_OuterAngle > 0.0f && dl.m_OuterAngle < 180.0f;
		if ( input.spot )
		{
			input.innerCos = std::cos( DEG2RAD( dl.m_InnerAngle ) );
			input.outerCos = std::cos( DEG2RAD( dl.m_OuterAngle ) );
		}
		out->push_back( input );
	}
}

} // namespace

void LightSet_PublishFrame()
{
	worldbrushdata_t *world = host_state.worldbrush;
	if ( !world || !materials )
		return;
	if ( s_builtMap != g_nMapLoadCount )
	{
		s_builtMap = g_nMapLoadCount;
		s_builder.BeginMap( uint64_t( g_nMapLoadCount ) );
		s_consumer = static_cast<ILightSetConsumer *>(
		    materials->QueryInterface( kLightSetConsumerInterface ) );
	}
	std::vector<WorldLightInput> worldLights;
	worldLights.reserve( size_t( world->numworldlights ) );
	for ( int i = 0; i < world->numworldlights; ++i )
	{
		const dworldlight_t &wl = world->worldlights[i];
		WorldLightInput input;
		input.index = uint32_t( i );
		input.shape = wl.type == emit_skylight ? LightShape::Directional
		              : wl.type == emit_spotlight || wl.type == emit_surface ? LightShape::Spot
		                                                                     : LightShape::Point;
		for ( int k = 0; k < 3; ++k )
		{
			input.position[k] = wl.origin[k];
			input.direction[k] = wl.normal[k];
			input.color[k] = wl.intensity[k];
		}
		input.radius = wl.radius;
		input.innerCos = wl.stopdot;
		input.outerCos = wl.stopdot2;
		input.style = wl.style;
		worldLights.push_back( input );
	}
	float styles[MAX_LIGHTSTYLES];
	for ( int i = 0; i < MAX_LIGHTSTYLES; ++i )
		styles[i] = LightStyleValue( i );
	std::vector<DynamicLightInput> dynamic;
	GatherDynamic( cl_dlights, MAX_DLIGHTS, LightKind::Dynamic, &dynamic );
	GatherDynamic( cl_elights, MAX_ELIGHTS, LightKind::Entity, &dynamic );
	Snapshot snapshot = s_builder.Build( worldLights, styles, dynamic );
	// Renderers with a direct-light budget rank lights from the last main view.
	snapshot.hasView = true;
	for ( int k = 0; k < 3; ++k )
		snapshot.viewOrigin[k] = MainViewOrigin()[k];
	if ( s_consumer )
		s_consumer->PublishLightSet( snapshot );
	// The indirect-light producers see the same frame's lights.
	IndirectLight_Frame( snapshot );
	if ( r_lightset_report.GetBool() )
	{
		r_lightset_report.SetValue( 0 );
		int counts[3] = {};
		for ( const RuntimeLight &light : snapshot.lights )
			++counts[int( light.kind )];
		Msg( "light set map %llu epoch %llu: %d world, %d dynamic, %d entity\n",
		    (unsigned long long)snapshot.mapSerial, (unsigned long long)snapshot.epoch, counts[0],
		    counts[1], counts[2] );
		// The direct-light rank of each light (render/direct_light_selection.h);
		// a renderer with a budget of N takes ranks 1..N.
		const std::vector<size_t> ranked = SelectDirectLights( snapshot, snapshot.lights.size() );
		std::vector<int> rank( snapshot.lights.size(), 0 );
		for ( size_t r = 0; r < ranked.size(); ++r )
			rank[ranked[r]] = int( r + 1 );
		for ( size_t i = 0; i < snapshot.lights.size(); ++i )
		{
			const RuntimeLight &light = snapshot.lights[i];
			if ( light.kind != LightKind::World )
				Msg( "  light %u: %s at %.0f %.0f %.0f radius %.0f color %.3f %.3f %.3f rank %d "
				     "importance %.3g\n",
				    light.id, light.kind == LightKind::Dynamic ? "dlight" : "elight",
				    light.position[0], light.position[1], light.position[2], light.radius,
				    light.color[0], light.color[1], light.color[2], rank[i],
				    DirectImportance( light, snapshot ) );
		}
	}
}
