//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's projected lights (projected_lights.h).
//
//===========================================================================//

#include "render_pch.h"
#include "projected_lights.h"

#include "cmodel_engine.h"
#include "convar.h"
#include "dynamic_occlusion.h"
#include "sample_textures.h"

#include <cmath>
#include <cstring>
#include <map>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_nMapLoadCount;

static ConVar r_projected_lights( "r_projected_lights", "1", 0,
    "env_projectedtexture lights like every other light (RFC 0011 render.projected-light.v1): "
    "in world lightmaps and model lighting, shadowed by the world and moving objects; 0 draws "
    "the legacy flashlight pass" );
static ConVar r_projected_lights_report( "r_projected_lights_report", "0", FCVAR_CHEAT,
    "Print the next published projected lights, once" );

// Diagnostics since the last report (gl_lightmap.cpp R_AddProjectedLights).
double g_ProjectedBuildSeconds = 0.0;
double g_ProjectedApplySeconds = 0.0;
int g_ProjectedBuilds = 0;
int g_ProjectedTexels = 0;

namespace
{
using projected_light::Light;

constexpr int kGenerations = 4;
constexpr float kPositionTolerance = 0.05f;
constexpr float kDirectionTolerance = 0.001f;
constexpr float kValueTolerance = 1.0f / 512.0f;

struct State
{
	int map = -1;
	int generation = 0;
	std::vector<ProjectedLightEntry> ring[kGenerations];
	uint32_t nextVersion = 1;
};

State &S()
{
	static State state;
	return state;
}

bool Near( const float *a, const float *b, int n, float tolerance )
{
	for ( int i = 0; i < n; ++i )
		if ( std::fabs( a[i] - b[i] ) > tolerance )
			return false;
	return true;
}

bool SameVersion( const Light &a, const Light &b )
{
	return Near( a.origin, b.origin, 3, kPositionTolerance ) &&
	       Near( a.forward, b.forward, 3, kDirectionTolerance ) &&
	       Near( a.right, b.right, 3, kDirectionTolerance ) &&
	       Near( a.up, b.up, 3, kDirectionTolerance ) &&
	       std::fabs( a.horizontalFovDegrees - b.horizontalFovDegrees ) <= 0.01f &&
	       std::fabs( a.verticalFovDegrees - b.verticalFovDegrees ) <= 0.01f &&
	       std::fabs( a.nearZ - b.nearZ ) <= 0.01f && std::fabs( a.farZ - b.farZ ) <= 0.01f &&
	       Near( a.color, b.color, 3, kValueTolerance ) && Near( a.atten, b.atten, 3, 1e-4f ) &&
	       std::strcmp( a.cookie, b.cookie ) == 0 && a.cookieFrame == b.cookieFrame &&
	       a.shadows == b.shadows;
}

void Dirty( const Light &light )
{
	float center[3], radius;
	projected_light::BoundingSphere( light, center, &radius );
	DynamicLightmaps_DirtySphere( Vector( center[0], center[1], center[2] ), radius );
}

class CProjectedLights final : public projected_light::IProjectedLights
{
public:
	void SetProjectedLights( const Light *lights, const int *keys, int count ) override
	{
		State &s = S();
		if ( s.map != g_nMapLoadCount )
		{
			s.map = g_nMapLoadCount;
			s.generation = 0;
			for ( std::vector<ProjectedLightEntry> &g : s.ring )
				g.clear();
		}
		const std::vector<ProjectedLightEntry> &previous = s.ring[s.generation % kGenerations];
		std::map<int, const ProjectedLightEntry *> last;
		for ( const ProjectedLightEntry &entry : previous )
			last[entry.key] = &entry;
		std::vector<ProjectedLightEntry> next;
		int changed = 0;
		const bool enabled = r_projected_lights.GetBool();
		for ( int i = 0;
		    enabled && lights && keys && i < count && i < projected_light::kMaxProjectedLights;
		    ++i )
		{
			ProjectedLightEntry entry;
			entry.key = keys[i];
			entry.light = lights[i];
			const auto found = last.find( keys[i] );
			if ( found != last.end() && SameVersion( found->second->light, lights[i] ) )
				entry = *found->second;
			else
			{
				entry.version = s.nextVersion++;
				++changed;
				Dirty( entry.light );
				if ( found != last.end() )
					Dirty( found->second->light );
			}
			if ( found != last.end() )
				last.erase( found );
			next.push_back( entry );
		}
		for ( const auto &gone : last )
		{
			Dirty( gone.second->light );
			++changed;
		}
		s.generation += 1;
		s.ring[s.generation % kGenerations] = std::move( next );

		if ( r_projected_lights_report.GetBool() )
		{
			r_projected_lights_report.SetValue( 0 );
			const std::vector<ProjectedLightEntry> &now = s.ring[s.generation % kGenerations];
			Msg( "projected lights generation %d: %zu lit, %d changed; since the last report: %d "
			     "surface build(s) lit %d texel(s) in %.3f ms, applying took %.3f ms\n",
			    s.generation, now.size(), changed, g_ProjectedBuilds, g_ProjectedTexels,
			    g_ProjectedBuildSeconds * 1000.0, g_ProjectedApplySeconds * 1000.0 );
			g_ProjectedBuildSeconds = g_ProjectedApplySeconds = 0.0;
			g_ProjectedBuilds = g_ProjectedTexels = 0;
			for ( const ProjectedLightEntry &e : now )
				Msg( "  projected key %d v%u at %.0f %.0f %.0f facing %.2f %.2f %.2f fov %.0fx%.0f "
				     "near %.0f far %.0f color %.3f %.3f %.3f atten %.1f %.1f %.1f cookie %s%s\n",
				    e.key, e.version, e.light.origin[0], e.light.origin[1], e.light.origin[2],
				    e.light.forward[0], e.light.forward[1], e.light.forward[2],
				    e.light.horizontalFovDegrees, e.light.verticalFovDegrees, e.light.nearZ,
				    e.light.farZ, e.light.color[0], e.light.color[1], e.light.color[2],
				    e.light.atten[0], e.light.atten[1], e.light.atten[2], e.light.cookie,
				    e.light.shadows ? "" : " (unshadowed)" );
		}
	}
};

CProjectedLights s_ProjectedLights;

} // namespace

EXPOSE_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE( CProjectedLights, projected_light::,
    IProjectedLights, projected_light::kProjectedLightsVersion, s_ProjectedLights );

bool ProjectedLights_Enabled()
{
	return r_projected_lights.GetBool();
}

int ProjectedLights_Generation()
{
	return S().generation;
}

const std::vector<ProjectedLightEntry> &ProjectedLights_Get( int generation )
{
	static const std::vector<ProjectedLightEntry> none;
	State &s = S();
	if ( s.map != g_nMapLoadCount || !r_projected_lights.GetBool() )
		return none;
	if ( generation > s.generation || generation <= s.generation - kGenerations )
		generation = s.generation;
	return s.ring[generation % kGenerations];
}

void ProjectedLights_Cookie( const Light &light, float u, float v, float rgb[3] )
{
	rgb[0] = rgb[1] = rgb[2] = 1.0f;
	const vtf_sample::Texture *pCookie = EngineSampleTexture( light.cookie );
	if ( !pCookie )
		return;
	float alpha;
	// Clamp: the frustum's edge, not a wrap.
	u = u < 0.0f ? 0.0f : ( u > 0.999f ? 0.999f : u );
	v = v < 0.0f ? 0.0f : ( v > 0.999f ? 0.999f : v );
	pCookie->Fetch( u, v, rgb, &alpha );
}

bool ProjectedLights_Representative(
    const ProjectedLightEntry &entry, const Vector &receiver, dworldlight_t &out )
{
	const Light &light = entry.light;
	const float p[3] = { receiver.x, receiver.y, receiver.z };
	float u, v, depth;
	if ( !projected_light::Project( light, p, &u, &v, &depth ) )
		return false;
	const Vector origin( light.origin[0], light.origin[1], light.origin[2] );
	Vector toLight = origin - receiver;
	const float distance = toLight.Length();
	if ( !( distance > 0.0f ) )
		return false;
	// Facing the light, the receiver takes color x cookie x attenuation.
	float cookie[3];
	ProjectedLights_Cookie( light, u, v, cookie );
	float scale = projected_light::Attenuation( light, distance );
	// Shadowed by the world. Moving objects shadow it where every model light
	// is shadowed (l_studio.cpp R_SetNonAmbientLightingState), its own boxes
	// ignored there.
	if ( light.shadows )
	{
		Ray_t ray;
		ray.Init( receiver, origin - toLight * ( 2.0f / distance ) );
		trace_t tr;
		CM_BoxTrace( ray, 0, MASK_OPAQUE, false, tr );
		if ( tr.fraction < 0.999f )
			return false;
	}
	if ( !( scale > 0.0f ) )
		return false;
	std::memset( &out, 0, sizeof( out ) );
	out.type = emit_point;
	out.origin = origin;
	// An inverse-square light giving that at this distance.
	const float toReference = distance * distance / ( 100.0f * 100.0f );
	out.intensity.Init( light.color[0] * cookie[0] * scale * toReference,
	    light.color[1] * cookie[1] * scale * toReference,
	    light.color[2] * cookie[2] * scale * toReference );
	out.quadratic_attn = 1.0f / ( 100.0f * 100.0f );
	out.radius = 2.0f * distance + 1.0f;
	out.cluster = CM_LeafCluster( CM_PointLeafnum( out.origin ) );
	return true;
}
