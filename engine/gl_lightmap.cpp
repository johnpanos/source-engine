//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//


#include "render_pch.h"
#include "gl_lightmap.h"
#include "view.h"
#include "gl_cvars.h"
#include "zone.h"
#include "gl_water.h"
#include "r_local.h"
#include "gl_model_private.h"
#include "mathlib/bumpvects.h"
#include "gl_matsysiface.h"
#include <float.h>
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imesh.h"
#include "tier0/dbg.h"
#include "tier0/vprof.h"
#include "tier1/callqueue.h"
#include "lightcache.h"
#include "cl_main.h"
#include "materialsystem/imaterial.h"
#include "modelloader.h"
#include "mapcontainer/probe_volume.h"
#include "area_lights.h"
#include "dynamic_occlusion.h"
#include "projected_lights.h"
#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "portal_dlights.h"

#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// globals
//-----------------------------------------------------------------------------

// Only enable this if you are testing lightstyle performance.
//#define UPDATE_LIGHTSTYLES_EVERY_FRAME

ALIGN128 Vector4D blocklights[NUM_BUMP_VECTS+1][ MAX_LIGHTMAP_DIM_INCLUDING_BORDER * MAX_LIGHTMAP_DIM_INCLUDING_BORDER ];

ConVar r_avglightmap( "r_avglightmap", "0", FCVAR_CHEAT | FCVAR_MATERIAL_SYSTEM_THREAD );
ConVar r_maxdlights( "r_maxdlights", "32" );
extern ConVar r_unloadlightmaps;
extern bool g_bHunkAllocLightmaps;

static int r_dlightvisible;
static int r_dlightvisiblethisframe;
static int s_nVisibleDLightCount;
static int s_nMaxVisibleDLightCount;


//-----------------------------------------------------------------------------
// Visible, not visible DLights
//-----------------------------------------------------------------------------
void R_MarkDLightVisible( int dlight )
{
	if ( (r_dlightvisible & ( 1 << dlight )) == 0 )
	{
		++s_nVisibleDLightCount;
		r_dlightvisible |= 1 << dlight;
	}
}

void R_MarkDLightNotVisible( int dlight )
{
	if ( r_dlightvisible & ( 1 << dlight ))
	{
		--s_nVisibleDLightCount;
		r_dlightvisible &= ~( 1 << dlight );
	}
}


//-----------------------------------------------------------------------------
// Must call these at the start + end of rendering each view
//-----------------------------------------------------------------------------
void R_DLightStartView()
{
	r_dlightvisiblethisframe = 0;
	s_nMaxVisibleDLightCount = r_maxdlights.GetInt();
}

void R_DLightEndView()
{
	if ( !g_bActiveDlights )
		return;
	for( int lnum=0 ; lnum<MAX_DLIGHTS; lnum++ )
	{
		if ( r_dlightvisiblethisframe & ( 1 << lnum ))
			continue;

		R_MarkDLightNotVisible( lnum );
	}
}


//-----------------------------------------------------------------------------
// Can we use another dynamic light, or is it just too expensive?
//-----------------------------------------------------------------------------
bool R_CanUseVisibleDLight( int dlight )
{
	r_dlightvisiblethisframe |= (1 << dlight);

	if ( r_dlightvisible & ( 1 << dlight ) )
		return true;

	if ( s_nVisibleDLightCount >= s_nMaxVisibleDLightCount )
		return false;

	R_MarkDLightVisible( dlight );
	return true;
}

//-----------------------------------------------------------------------------
// RFC 0016 K7: a light imaged through a portal (portal_dlights.h) lights a
// luxel only when the path through the portal reaches it. The luxel's world
// position: the surface's first luxel (R_ComputeSurfaceBasis), stepped along
// the lightmap axes, then out of the brush entity's space.
//-----------------------------------------------------------------------------
struct PortalLuxelClip
{
	int slot = -1;
	const Vector *pLuxelBase = NULL;
	const matrix3x4_t *pEntityToWorld = NULL;
};

static bool PortalLuxelLit( const PortalLuxelClip *pClip, SurfaceHandle_t surfID, int s, int t )
{
	if ( !pClip )
		return true;
	mtexinfo_t *pTexInfo = MSurf_TexInfo( surfID );
	const float fixupFactor = pTexInfo->worldUnitsPerLuxel * pTexInfo->worldUnitsPerLuxel;
	Vector local = *pClip->pLuxelBase;
	VectorMA(
	    local, s * fixupFactor, pTexInfo->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D(), local );
	VectorMA(
	    local, t * fixupFactor, pTexInfo->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D(), local );
	Vector world;
	VectorTransform( local, *pClip->pEntityToWorld, world );
	return PortalDLights_Reaches( pClip->slot, world );
}

//-----------------------------------------------------------------------------
// Adds a single dynamic light
//-----------------------------------------------------------------------------
static bool AddSingleDynamicLight( dlight_t &dl, SurfaceHandle_t surfID, const Vector &lightOrigin,
    float perpDistSq, float lightRadiusSq, const PortalLuxelClip *pClip )
{
	// transform the light into brush local space
	Vector local;
	// Spotlight early outs...
	if (dl.m_OuterAngle)
	{
		if (dl.m_OuterAngle < 180.0f)
		{
			// Can't light anything from the rear...
			if (DotProduct(dl.m_Direction, MSurf_Plane( surfID ).normal) >= 0.0f)
				return false;
		}
	}

	// Transform the light center point into (u,v) space of the lightmap
	mtexinfo_t* tex = MSurf_TexInfo( surfID );
	local[0] = DotProduct (lightOrigin, tex->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D()) + 
			   tex->lightmapVecsLuxelsPerWorldUnits[0][3];
	local[1] = DotProduct (lightOrigin, tex->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D()) + 
			   tex->lightmapVecsLuxelsPerWorldUnits[1][3];

	// Now put the center points into the space of the lightmap rectangle
	// defined by the lightmapMins + lightmapExtents
	local[0] -= MSurf_LightmapMins( surfID )[0];
	local[1] -= MSurf_LightmapMins( surfID )[1];
	
	// Figure out the quadratic attenuation factor...
	Vector intensity;
	float lightStyleValue = LightStyleValue( dl.style );
	intensity[0] = TexLightToLinear( dl.color.r, dl.color.exponent ) * lightStyleValue;
	intensity[1] = TexLightToLinear( dl.color.g, dl.color.exponent ) * lightStyleValue;
	intensity[2] = TexLightToLinear( dl.color.b, dl.color.exponent ) * lightStyleValue;

	float minlight = fpmax( g_flMinLightingValue, dl.minlight );
	float ooQuadraticAttn = lightRadiusSq * minlight;
	float ooRadiusSq = 1.0f / lightRadiusSq;

	// Compute a color at each luxel
	// We want to know the square distance from luxel center to light
	// so we can compute an 1/r^2 falloff in light color
	int smax = MSurf_LightmapExtents( surfID )[0] + 1;
	int tmax = MSurf_LightmapExtents( surfID )[1] + 1;
	for (int t=0; t<tmax; ++t)
	{
		float td = (local[1] - t) * tex->worldUnitsPerLuxel;
		
		for (int s=0; s<smax; ++s)
		{
			float sd = (local[0] - s) * tex->worldUnitsPerLuxel;

			float inPlaneDistSq = sd * sd + td * td;
			float totalDistSq = inPlaneDistSq + perpDistSq;
			if ( totalDistSq < lightRadiusSq && PortalLuxelLit( pClip, surfID, s, t ) )
			{
				// at least all floating point only happens when a luxel is lit.
				float scale = (totalDistSq != 0.0f) ? ooQuadraticAttn / totalDistSq : 1.0f;

				// Apply a little extra attenuation
				scale *= (1.0f - totalDistSq * ooRadiusSq);

				if (scale > 2.0f)
					scale = 2.0f;

				int idx = t*smax + s;

				// Compute the base lighting just as is done in the non-bump case...
				blocklights[0][idx][0] += scale * intensity[0];
				blocklights[0][idx][1] += scale * intensity[1];
				blocklights[0][idx][2] += scale * intensity[2];
			}
		}
	}
	return true;
}												

//-----------------------------------------------------------------------------
// Adds a dynamic light to the bumped lighting
//-----------------------------------------------------------------------------
static void AddSingleDynamicLightToBumpLighting( dlight_t &dl, SurfaceHandle_t surfID,
    const Vector &lightOrigin, float perpDistSq, float lightRadiusSq, Vector *pBumpBasis,
    const Vector &luxelBasePosition, const PortalLuxelClip *pClip )
{
	Vector local;
	// FIXME: For now, only elights can be spotlights
	// the lightmap computation could get expensive for spotlights...
	Assert( dl.m_OuterAngle == 0.0f );

	// Transform the light center point into (u,v) space of the lightmap
	mtexinfo_t *pTexInfo = MSurf_TexInfo( surfID );
	local[0] = DotProduct (lightOrigin, pTexInfo->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D()) + 
			   pTexInfo->lightmapVecsLuxelsPerWorldUnits[0][3];
	local[1] = DotProduct (lightOrigin, pTexInfo->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D()) + 
			   pTexInfo->lightmapVecsLuxelsPerWorldUnits[1][3];

	// Now put the center points into the space of the lightmap rectangle
	// defined by the lightmapMins + lightmapExtents
	local[0] -= MSurf_LightmapMins( surfID )[0];
	local[1] -= MSurf_LightmapMins( surfID )[1];

	// Figure out the quadratic attenuation factor...
	Vector intensity;
	float lightStyleValue = LightStyleValue( dl.style );
	intensity[0] = TexLightToLinear( dl.color.r, dl.color.exponent ) * lightStyleValue;
	intensity[1] = TexLightToLinear( dl.color.g, dl.color.exponent ) * lightStyleValue;
	intensity[2] = TexLightToLinear( dl.color.b, dl.color.exponent ) * lightStyleValue;

	float minlight = fpmax( g_flMinLightingValue, dl.minlight );
	float ooQuadraticAttn = lightRadiusSq * minlight;
	float ooRadiusSq = 1.0f / lightRadiusSq;

	// The algorithm here is necessary to make dynamic lights live in the
	// same world as the non-bumped dynamic lights. Therefore, we compute
	// the intensity of the flat lightmap the exact same way here as when
	// we've got a non-bumped surface.

	// Then, I compute an actual light direction vector per luxel (FIXME: !!expensive!!)
	// and compute what light would have to come in along that direction
	// in order to produce the same illumination on the flat lightmap. That's
	// computed by dividing the flat lightmap color by n dot l.
	Vector lightDirection(0, 0, 0), texelWorldPosition;
#if 1
	bool useLightDirection = (dl.m_OuterAngle != 0.0f) &&
		(fabs(dl.m_Direction.LengthSqr() - 1.0f) < 1e-3);
	if (useLightDirection)
		VectorMultiply( dl.m_Direction, -1.0f, lightDirection );
#endif

	// Since there's a scale factor used when going from world to luxel,
	// we gotta undo that scale factor when going from luxel to world
	float fixupFactor = pTexInfo->worldUnitsPerLuxel * pTexInfo->worldUnitsPerLuxel;

	// Compute a color at each luxel
	// We want to know the square distance from luxel center to light
	// so we can compute an 1/r^2 falloff in light color
	int smax = MSurf_LightmapExtents( surfID )[0] + 1;
	int tmax = MSurf_LightmapExtents( surfID )[1] + 1;
	for (int t=0; t<tmax; ++t)
	{
		float td = (local[1] - t) * pTexInfo->worldUnitsPerLuxel;
		
		// Move along the v direction
		VectorMA( luxelBasePosition, t * fixupFactor, pTexInfo->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D(), 
			texelWorldPosition );

		for (int s=0; s<smax; ++s)
		{
			float sd = (local[0] - s) * pTexInfo->worldUnitsPerLuxel;

			float inPlaneDistSq = sd * sd + td * td;
			float totalDistSq = inPlaneDistSq + perpDistSq;

			if ( totalDistSq < lightRadiusSq && PortalLuxelLit( pClip, surfID, s, t ) )
			{
				// at least all floating point only happens when a luxel is lit.
				float scale = (totalDistSq != 0.0f) ? ooQuadraticAttn / totalDistSq : 1.0f;

				// Apply a little extra attenuation
				scale *= (1.0f - totalDistSq * ooRadiusSq);

				if (scale > 2.0f)
					scale = 2.0f;

				int idx = t*smax + s;

				// Compute the base lighting just as is done in the non-bump case...
				VectorMA( blocklights[0][idx].AsVector3D(), scale, intensity, blocklights[0][idx].AsVector3D() );

#if 1
				if (!useLightDirection)
				{
					VectorSubtract( lightOrigin, texelWorldPosition, lightDirection );
					VectorNormalize( lightDirection );
				}
				
				float lDotN = DotProduct( lightDirection, MSurf_Plane( surfID ).normal );
				if (lDotN < 1e-3)
					lDotN = 1e-3;
				scale /= lDotN;

				int i;
				for( i = 1; i < NUM_BUMP_VECTS + 1; i++ )
				{
					float dot = DotProduct( lightDirection, pBumpBasis[i-1] );
					if( dot <= 0.0f )
						continue;
					
					VectorMA( blocklights[i][idx].AsVector3D(), dot * scale, intensity, 
						blocklights[i][idx].AsVector3D() );
				}
#else
				VectorMA( blocklights[1][idx].AsVector3D(), scale, intensity, blocklights[1][idx].AsVector3D() );
				VectorMA( blocklights[2][idx].AsVector3D(), scale, intensity, blocklights[2][idx].AsVector3D() );
				VectorMA( blocklights[3][idx].AsVector3D(), scale, intensity, blocklights[3][idx].AsVector3D() );
#endif
			}
		}

		// Move along u
		VectorMA( texelWorldPosition, fixupFactor, 
			pTexInfo->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D(), texelWorldPosition );

	}
}

//-----------------------------------------------------------------------------
// Compute the bumpmap basis for this surface
//-----------------------------------------------------------------------------
static void R_ComputeSurfaceBasis( SurfaceHandle_t surfID, Vector *pBumpNormals, Vector &luxelBasePosition )
{
	// Get the bump basis vects in the space of the surface.
	Vector sVect, tVect;
	VectorCopy( MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D(), sVect );
	VectorNormalize( sVect );
	VectorCopy( MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D(), tVect );
	VectorNormalize( tVect );
	GetBumpNormals( sVect, tVect, MSurf_Plane( surfID ).normal, MSurf_Plane( surfID ).normal, pBumpNormals );

	// Compute the location of the first luxel in worldspace

	// Since there's a scale factor used when going from world to luxel,
	// we gotta undo that scale factor when going from luxel to world
	float fixupFactor = 
		MSurf_TexInfo( surfID )->worldUnitsPerLuxel * 
		MSurf_TexInfo( surfID )->worldUnitsPerLuxel;

	// The starting u of the surface is surf->lightmapMins[0];
	// since N * P + D = u, N * P = u - D, therefore we gotta move (u-D) along uvec
	VectorMultiply( MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D(),
		(MSurf_LightmapMins( surfID )[0] - MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[0][3]) * fixupFactor,
		luxelBasePosition );

	// Do the same thing for the v direction.
	VectorMA( luxelBasePosition, 
		(MSurf_LightmapMins( surfID )[1] - 
		MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[1][3]) * fixupFactor,
		MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D(),
		luxelBasePosition );

	// Move out in the direction of the plane normal...
	VectorMA( luxelBasePosition, MSurf_Plane( surfID ).dist, MSurf_Plane( surfID ).normal, luxelBasePosition ); 
}

//-----------------------------------------------------------------------------
// RFC 0011 light set v2: the area lights of a generation (area_lights.h) that
// reach a surface, added to blocklights exactly: each luxel takes the light's
// form factor over its hemisphere (area_light::IrradianceAt), and each bump
// basis the vector form factor's projection on it. Records the light
// versions the lightmap now holds. Every rebuild does this, so a rebuild for
// any reason keeps the surface's area light.
//-----------------------------------------------------------------------------
static void R_AddAreaLightsTimed(
    SurfaceHandle_t surfID, const matrix3x4_t &entityToWorld, bool needsBumpmap, int generation );

static void R_AddAreaLights(
    SurfaceHandle_t surfID, const matrix3x4_t &entityToWorld, bool needsBumpmap, int generation )
{
	extern double g_AreaLightsSeconds;
	extern int g_AreaLightsSurfaces;
	const double start = Plat_FloatTime();
	R_AddAreaLightsTimed( surfID, entityToWorld, needsBumpmap, generation );
	g_AreaLightsSeconds += Plat_FloatTime() - start;
	++g_AreaLightsSurfaces;
}

static void R_AddAreaLightsTimed(
    SurfaceHandle_t surfID, const matrix3x4_t &entityToWorld, bool needsBumpmap, int generation )
{
	AreaLightSlot lights[area_light::kMaxAreaLights];
	// The render core lights the surfaces it draws per pixel: none here.
	const int count = AreaLights_CoreOwns( surfID ) ? 0 : AreaLights_Get( generation, lights );
	const bool worldSurface = AreaLights_IsWorldSurface( surfID );
	AreaLightSlot reaching[area_light::kMaxAreaLights];
	int reachingCount = 0;
	if ( count == 0 )
	{
		AreaLights_Applied( surfID, worldSurface, reaching, 0 );
		return;
	}

	const Vector &planeNormal = MSurf_Plane( surfID ).normal;
	const float planeDist = MSurf_Plane( surfID ).dist;
	Vector bumpNormals[NUM_BUMP_VECTS];
	Vector luxelBase, surfaceCenter;
	float surfaceRadius = 0.0f;
	bool basis = false;
	mtexinfo_t *tex = MSurf_TexInfo( surfID );
	const float fixupFactor = tex->worldUnitsPerLuxel * tex->worldUnitsPerLuxel;
	const int smax = MSurf_LightmapExtents( surfID )[0] + 1;
	const int tmax = MSurf_LightmapExtents( surfID )[1] + 1;
	const float n[3] = { planeNormal.x, planeNormal.y, planeNormal.z };

	for ( int l = 0; l < count; ++l )
	{
		// Into the surface's space (brush entities move).
		area_light::AreaLight light = lights[l].light;
		Vector center( light.rect.center[0], light.rect.center[1], light.rect.center[2] );
		Vector halfU( light.rect.halfU[0], light.rect.halfU[1], light.rect.halfU[2] );
		Vector halfV( light.rect.halfV[0], light.rect.halfV[1], light.rect.halfV[2] );
		Vector localCenter, localU, localV;
		VectorITransform( center, entityToWorld, localCenter );
		VectorIRotate( halfU, entityToWorld, localU );
		VectorIRotate( halfV, entityToWorld, localV );
		for ( int k = 0; k < 3; ++k )
		{
			light.rect.center[k] = localCenter[k];
			light.rect.halfU[k] = localU[k];
			light.rect.halfV[k] = localV[k];
		}
		// The plane must come within the light's reach of the rectangle, and
		// some corner must be in front of the plane.
		float corners[4][3];
		area_light::Corners( light.rect, corners );
		float nearest = FLT_MAX, farthestFront = -FLT_MAX;
		for ( int c = 0; c < 4; ++c )
		{
			const float d =
			    corners[c][0] * n[0] + corners[c][1] * n[1] + corners[c][2] * n[2] - planeDist;
			nearest = fpmin( nearest, fabsf( d ) );
			farthestFront = fpmax( farthestFront, d );
		}
		if ( nearest >= light.reach || farthestFront <= 0.0f )
			continue;

		if ( !basis )
		{
			R_ComputeSurfaceBasis( surfID, bumpNormals, luxelBase );
			// The sphere around the surface's lightmap rectangle.
			const Vector across =
			    ( smax - 1 ) * fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D() +
			    ( tmax - 1 ) * fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D();
			const Vector other =
			    ( smax - 1 ) * fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D() -
			    ( tmax - 1 ) * fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D();
			surfaceCenter = luxelBase + 0.5f * across;
			surfaceRadius = 0.5f * MAX( across.Length(), other.Length() );
			basis = true;
		}
		// No luxel within the light's reach of the rectangle: nothing to add.
		const float lightRadius =
		    sqrtf( area_light::detail::Dot( light.rect.halfU, light.rect.halfU ) +
		           area_light::detail::Dot( light.rect.halfV, light.rect.halfV ) );
		if ( ( surfaceCenter - localCenter ).Length() >= surfaceRadius + lightRadius + light.reach )
			continue;
		reaching[reachingCount++] = lights[l];
		for ( int t = 0; t < tmax; ++t )
		{
			Vector position;
			VectorMA( luxelBase, t * fixupFactor,
			    tex->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D(), position );
			for ( int s = 0; s < smax; ++s )
			{
				const float p[3] = { position.x, position.y, position.z };
				const float window =
				    area_light::Window( area_light::DistanceTo( light.rect, p ), light.reach );
				if ( window > 0.0f && area_light::Faces( light.rect, p ) )
				{
					const int idx = t * smax + s;
					const float flat = area_light::FormFactor( light.rect, p, n ) * window;
					for ( int k = 0; k < 3; ++k )
						blocklights[0][idx][k] += flat * light.radiance[k];
					if ( flat > 0.0f )
					{
						extern double g_AreaLightsAddedEnergy;
						extern int g_AreaLightsLitLuxels;
						g_AreaLightsAddedEnergy += flat * light.radiance[1];
						++g_AreaLightsLitLuxels;
					}
					if ( needsBumpmap )
					{
						float v[3];
						area_light::VectorFormFactor( corners, 4, p, v );
						for ( int b = 0; b < NUM_BUMP_VECTS; ++b )
						{
							const float dot = v[0] * bumpNormals[b].x + v[1] * bumpNormals[b].y +
							                  v[2] * bumpNormals[b].z;
							if ( dot <= 0.0f )
								continue;
							for ( int k = 0; k < 3; ++k )
								blocklights[b + 1][idx][k] += dot * window * light.radiance[k];
						}
					}
				}
				VectorMA( position, fixupFactor,
				    tex->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D(), position );
			}
		}
	}
	AreaLights_Applied( surfID, worldSurface, reaching, reachingCount );
}

//-----------------------------------------------------------------------------
// RFC 0011 render.dynamic-occlusion.v1: moving objects (props, physics
// objects, doors) block the world's lights. Each texel loses, for every world
// light, the share of that light's direct light the boxes of a generation
// block (dynamic_occlusion::Visibility), computed with the engine's world
// light formula (falloff times angle, as models are lit), wherever the bake
// let the light reach the texel. Lights are judged one by one, so shadows from
// several lights mix. Bumped lightmaps lose the same light along its
// direction, as a dlight adds it.
//-----------------------------------------------------------------------------
extern float Engine_WorldLightDistanceFalloff(
    const dworldlight_t *wl, const Vector &delta, bool bNoRadiusCheck );
extern float Engine_WorldLightAngle(
    const dworldlight_t *wl, const Vector &lnormal, const Vector &snormal, const Vector &delta );

// Direct light below this (the lightmap unit) is not worth a trace.
static const float kOcclusionThreshold = 1.0f / 512.0f;

// A surface's blocked light, kept while the boxes near it stay put: the texels
// some light is blocked at, and for each the lights the bake let reach it with
// the share the boxes let through. A rebuild for any other reason (a flickering
// style, a dlight) then only redoes the light arithmetic. When boxes move, only
// the texels in their regions (R_OcclusionRegion) at their old and new poses
// are evaluated again: every other texel has the same boxes in reach.
struct OcclusionTexel
{
	int idx;
	int first; // into OcclusionCache::lights
	int count;
};
struct OcclusionLight
{
	int light;
	float visible;
};
struct OcclusionCache
{
	uint64_t signature = 0; // the near boxes' keys and versions
	int map = -1;
	std::vector<OccluderEntry> boxes; // the near boxes it holds, with their versions
	std::vector<int> surfaceLights;   // the world lights that can reach the surface
	std::vector<OcclusionTexel> texels;
	std::vector<OcclusionLight> lights;
};
static std::unordered_map<SurfaceHandle_t, OcclusionCache> g_OcclusionCache;
static std::mutex g_OcclusionCacheMutex;

// Where a box can block a world light on a surface: the texels of the lightmap
// lattice (base + s stepS + t stepT) whose path to the light can cross the box.
// A texel's path to any of the light's samples stays within the samples' disk
// radius of its path to the light's center (a distant light: its one sample),
// so the box blocks it only if that path meets the box's bounding sphere grown
// by the radius: only at texels inside the shadow of the sphere's bounding cube
// cast from the light's center (along a distant light's direction) on the
// lattice's plane, the convex hull of the cube's eight projected corners. The
// region is that hull's texel bounds, a texel wider; it is the whole lattice
// where the projection does not bound it (a corner at or above the light).
// Visibility over the boxes whose regions hold a texel therefore equals
// Visibility over every box (r_dynamic_occlusion_verify checks it).
struct OcclusionLattice
{
	Vector base, stepS, stepT, normal;
	float ss, st, tt, det; // the Gram matrix of stepS and stepT, and its determinant
	int smax, tmax;
	bool valid;
};

struct OcclusionRegion
{
	int box;            // into the surface's near boxes
	int s0, s1, t0, t1; // inclusive texel bounds
};

static OcclusionLattice R_OcclusionLattice( const Vector &base, const Vector &stepS,
    const Vector &stepT, const Vector &planeNormal, int smax, int tmax )
{
	OcclusionLattice lattice;
	lattice.base = base;
	lattice.stepS = stepS;
	lattice.stepT = stepT;
	lattice.smax = smax;
	lattice.tmax = tmax;
	lattice.normal = CrossProduct( stepS, stepT );
	const float length = lattice.normal.NormalizeInPlace();
	if ( DotProduct( lattice.normal, planeNormal ) < 0.0f )
		lattice.normal = -lattice.normal;
	lattice.ss = DotProduct( stepS, stepS );
	lattice.st = DotProduct( stepS, stepT );
	lattice.tt = DotProduct( stepT, stepT );
	lattice.det = lattice.ss * lattice.tt - lattice.st * lattice.st;
	lattice.valid = length > 1e-6f && lattice.det > 1e-6f * lattice.ss * lattice.tt;
	return lattice;
}

// Whether the box's region for the light holds any texel; `out` is the region.
static bool R_OcclusionRegion( const OcclusionLattice &lattice, const dworldlight_t &wl,
    const dynamic_occlusion::Box &box, int boxIndex, OcclusionRegion &out )
{
	out = OcclusionRegion{ boxIndex, 0, lattice.smax - 1, 0, lattice.tmax - 1 };
	if ( !lattice.valid )
		return true;
	// A unit more than the radii, for the texel positions' rounding.
	const float grow =
	    dynamic_occlusion::Radius( box ) + DynamicOcclusion_WorldLightRadius( wl ) + 1.0f;
	const Vector center( box.center[0], box.center[1], box.center[2] );
	float sMin = FLT_MAX, sMax = -FLT_MAX, tMin = FLT_MAX, tMax = -FLT_MAX;
	auto include = [&]( const Vector &q )
	{
		const Vector d = q - lattice.base;
		const float u = DotProduct( d, lattice.stepS ), w = DotProduct( d, lattice.stepT );
		const float s = ( lattice.tt * u - lattice.st * w ) / lattice.det;
		const float t = ( lattice.ss * w - lattice.st * u ) / lattice.det;
		sMin = MIN( sMin, s );
		sMax = MAX( sMax, s );
		tMin = MIN( tMin, t );
		tMax = MAX( tMax, t );
	};
	if ( wl.type == emit_skylight )
	{
		// Along the light's direction: the sphere's bounding cube's corners,
		// projected in parallel.
		Vector direction = wl.normal;
		direction.NormalizeInPlace();
		const float directionDotN = DotProduct( direction, lattice.normal );
		if ( fabsf( directionDotN ) < 1e-3f )
			return true;
		for ( int corner = 0; corner < 8; ++corner )
		{
			const Vector v( center.x + ( corner & 1 ? grow : -grow ),
			    center.y + ( corner & 2 ? grow : -grow ),
			    center.z + ( corner & 4 ? grow : -grow ) );
			include( v - direction *
			                 ( DotProduct( v - lattice.base, lattice.normal ) / directionDotN ) );
		}
	}
	else
	{
		// From the light's center: the sphere's cone, inside the square pyramid
		// around it, whose four edges bound where the cone meets the plane.
		const float lightHeight = DotProduct( wl.origin - lattice.base, lattice.normal );
		const Vector toBox = center - wl.origin;
		const float distance = toBox.Length();
		if ( !( lightHeight > 0.0f ) || !( distance > grow * 1.001f ) )
			return true; // the light is behind the lattice, or inside the sphere
		const Vector axis = toBox / distance;
		const float spread = grow / sqrtf( distance * distance - grow * grow ); // tan(half angle)
		Vector side = fabsf( axis.x ) < 0.9f ? Vector( 1, 0, 0 ) : Vector( 0, 1, 0 );
		Vector u = CrossProduct( axis, side );
		u.NormalizeInPlace();
		const Vector v = CrossProduct( axis, u );
		Vector edges[4];
		int toward = 0, away = 0;
		for ( int i = 0; i < 4; ++i )
		{
			edges[i] = axis + spread * ( ( i & 1 ? u : -u ) + ( i & 2 ? v : -v ) );
			const float e = DotProduct( edges[i], lattice.normal );
			toward += e < -1e-4f;
			away += e >= 0.0f;
		}
		if ( away == 4 )
			return false; // every path from the plane to the light misses it
		if ( toward < 4 )
			return true; // the cone reaches the plane's horizon
		for ( const Vector &e : edges )
			include( wl.origin + e * ( lightHeight / -DotProduct( e, lattice.normal ) ) );
	}
	if ( !IsFinite( sMin ) || !IsFinite( sMax ) || !IsFinite( tMin ) || !IsFinite( tMax ) )
		return true;
	// Clamped before the integer conversion; a texel wider on each side.
	const float sLimit = float( lattice.smax + 1 ), tLimit = float( lattice.tmax + 1 );
	out.s0 = MAX( 0, int( floorf( clamp( sMin, -2.0f, sLimit ) ) ) - 1 );
	out.s1 = MIN( lattice.smax - 1, int( ceilf( clamp( sMax, -2.0f, sLimit ) ) ) + 1 );
	out.t0 = MAX( 0, int( floorf( clamp( tMin, -2.0f, tLimit ) ) ) - 1 );
	out.t1 = MIN( lattice.tmax - 1, int( ceilf( clamp( tMax, -2.0f, tLimit ) ) ) + 1 );
	return out.s0 <= out.s1 && out.t0 <= out.t1;
}

static void R_ApplyDynamicOcclusionTimed(
    SurfaceHandle_t surfID, const matrix3x4_t &entityToWorld, bool needsBumpmap, int generation );

static void R_ApplyDynamicOcclusion(
    SurfaceHandle_t surfID, const matrix3x4_t &entityToWorld, bool needsBumpmap, int generation )
{
	extern double g_OcclusionSeconds;
	extern int g_OcclusionSurfaces;
	const double start = Plat_FloatTime();
	R_ApplyDynamicOcclusionTimed( surfID, entityToWorld, needsBumpmap, generation );
	g_OcclusionSeconds += Plat_FloatTime() - start;
	++g_OcclusionSurfaces;
}

static void R_ApplyDynamicOcclusionTimed(
    SurfaceHandle_t surfID, const matrix3x4_t &entityToWorld, bool needsBumpmap, int generation )
{
	extern int g_nMapLoadCount;
	worldbrushdata_t *world = host_state.worldbrush;
	if ( !world || world->numworldlights <= 0 || !AreaLights_IsWorldSurface( surfID ) )
		return;
	const std::vector<OccluderEntry> &entries = DynamicOcclusion_Get( generation );

	// The surface's lightmap rectangle, as a sphere.
	Vector bumpNormals[NUM_BUMP_VECTS];
	Vector luxelBase;
	R_ComputeSurfaceBasis( surfID, bumpNormals, luxelBase );
	mtexinfo_t *tex = MSurf_TexInfo( surfID );
	const float fixupFactor = tex->worldUnitsPerLuxel * tex->worldUnitsPerLuxel;
	const int smax = MSurf_LightmapExtents( surfID )[0] + 1;
	const int tmax = MSurf_LightmapExtents( surfID )[1] + 1;
	const Vector stepS = fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D();
	const Vector stepT = fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D();
	const Vector across = ( smax - 1 ) * stepS + ( tmax - 1 ) * stepT;
	const Vector other = ( smax - 1 ) * stepS - ( tmax - 1 ) * stepT;
	const Vector surfaceCenter = luxelBase + 0.5f * across;
	const float surfaceRadius = 0.5f * MAX( across.Length(), other.Length() );

	// The boxes whose shadows can reach it, and their signature.
	std::vector<OccluderEntry> nearEntries;
	std::vector<dynamic_occlusion::Box> near;
	uint64_t signature = 1469598103934665603ull;
	for ( const OccluderEntry &entry : entries )
	{
		const Vector center( entry.box.center[0], entry.box.center[1], entry.box.center[2] );
		if ( ( center - surfaceCenter ).Length() >= surfaceRadius +
		                                                dynamic_occlusion::Radius( entry.box ) +
		                                                DynamicOcclusion_Reach( entry.box ) )
			continue;
		nearEntries.push_back( entry );
		near.push_back( entry.box );
		const uint64_t words[3] = { uint64_t( uint32_t( entry.box.entity ) ),
		    uint64_t( uint32_t( entry.box.part ) ), uint64_t( entry.version ) };
		for ( uint64_t word : words )
		{
			signature ^= word;
			signature *= 1099511628211ull;
		}
	}

	std::lock_guard<std::mutex> lock( g_OcclusionCacheMutex );
	if ( near.empty() )
	{
		g_OcclusionCache.erase( surfID );
		return;
	}
	OcclusionCache &cache = g_OcclusionCache[surfID];
	const Vector &n = MSurf_Plane( surfID ).normal;
	const bool verify = DynamicOcclusion_Verify();
	if ( cache.map != g_nMapLoadCount || cache.signature != signature || verify )
	{
		extern int g_OcclusionVerified, g_OcclusionMismatches, g_OcclusionTexelsEvaluated;
		const bool fresh = cache.map != g_nMapLoadCount;
		if ( fresh )
		{
			cache.map = g_nMapLoadCount;
			cache.boxes.clear();
			cache.texels.clear();
			cache.lights.clear();
			// The lights that can reach the surface at all.
			cache.surfaceLights.clear();
			const float planeDist = MSurf_Plane( surfID ).dist;
			for ( int i = 0; i < world->numworldlights; ++i )
			{
				const dworldlight_t &wl = world->worldlights[i];
				if ( wl.type == emit_skyambient || wl.type == emit_quakelight )
					continue;
				if ( wl.type == emit_skylight )
				{
					if ( DotProduct( wl.normal, n ) >= 0.0f )
						continue;
				}
				else
				{
					if ( DotProduct( wl.origin, n ) - planeDist <= 0.0f )
						continue;
					if ( wl.radius > 0.0f &&
					     ( wl.origin - surfaceCenter ).Length() > wl.radius + surfaceRadius )
						continue;
				}
				cache.surfaceLights.push_back( i );
			}
		}
		const std::vector<int> &lights = cache.surfaceLights;
		const OcclusionLattice lattice =
		    R_OcclusionLattice( luxelBase, stepS, stepT, n, smax, tmax );
		const size_t texelCount = size_t( smax * tmax );
		auto stamp = [&]( const OcclusionRegion &region, std::vector<uint8_t> &mask )
		{
			for ( int t = region.t0; t <= region.t1; ++t )
				memset(
				    &mask[size_t( t * smax + region.s0 )], 1, size_t( region.s1 - region.s0 + 1 ) );
		};

		// Each light's regions of the boxes now near; a texel no region holds
		// keeps every light.
		std::vector<std::vector<OcclusionRegion>> regions( lights.size() );
		std::vector<uint8_t> covered( texelCount, 0 );
		for ( size_t j = 0; j < lights.size(); ++j )
		{
			const dworldlight_t &wl = world->worldlights[lights[size_t( j )]];
			for ( size_t k = 0; k < near.size(); ++k )
			{
				OcclusionRegion region;
				if ( !R_OcclusionRegion( lattice, wl, near[k], int( k ), region ) )
					continue;
				regions[j].push_back( region );
				stamp( region, covered );
			}
		}

		// The texels to evaluate again: all of them for a new cache, otherwise
		// the regions of the boxes that changed, arrived or left (old and new
		// poses).
		std::vector<uint8_t> stale( texelCount, fresh ? 1 : 0 );
		if ( !fresh && cache.signature != signature )
		{
			std::unordered_map<uint64_t, uint32_t> before, after;
			auto key = []( const dynamic_occlusion::Box &box )
			{
				return ( uint64_t( uint32_t( box.entity ) ) << 32 ) | uint32_t( box.part );
			};
			for ( const OccluderEntry &entry : cache.boxes )
				before[key( entry.box )] = entry.version;
			for ( const OccluderEntry &entry : nearEntries )
				after[key( entry.box )] = entry.version;
			auto markChanged = [&]( const std::vector<OccluderEntry> &boxes,
			                       const std::unordered_map<uint64_t, uint32_t> &others )
			{
				for ( const OccluderEntry &entry : boxes )
				{
					const auto found = others.find( key( entry.box ) );
					if ( found != others.end() && found->second == entry.version )
						continue;
					for ( int li : lights )
					{
						OcclusionRegion region;
						if ( R_OcclusionRegion(
						         lattice, world->worldlights[li], entry.box, 0, region ) )
							stamp( region, stale );
					}
				}
			};
			markChanged( cache.boxes, after );
			markChanged( nearEntries, before );
		}

		// One texel's blocked lights, appended to (texels, lightsOut): the lights
		// the bake let reach it, with the share the boxes (`all`: every near box,
		// otherwise those whose regions hold it) let through.
		std::vector<dynamic_occlusion::Box> candidates;
		std::vector<OcclusionLight> reaching;
		auto evaluate = [&]( int s, int t, bool all, std::vector<OcclusionTexel> &texelsOut,
		                    std::vector<OcclusionLight> &lightsOut )
		{
			const int idx = t * smax + s;
			const Vector position = luxelBase + t * stepT + s * stepS;
			const float p[3] = { position.x, position.y, position.z };
			reaching.clear();
			bool anyBlocked = false;
			for ( size_t j = 0; j < lights.size(); ++j )
			{
				const int li = lights[j];
				const dworldlight_t &wl = world->worldlights[li];
				Vector delta = wl.type == emit_skylight ? -wl.normal : wl.origin - position;
				const float falloff = Engine_WorldLightDistanceFalloff( &wl, delta, false );
				if ( falloff <= 0.0f )
					continue;
				delta.NormalizeInPlace();
				const float angle = Engine_WorldLightAngle( &wl, wl.normal, n, delta );
				if ( angle <= 0.0f )
					continue;
				const Vector direct = wl.intensity * ( falloff * angle );
				if ( MAX( direct.x, MAX( direct.y, direct.z ) ) < kOcclusionThreshold )
					continue;
				candidates.clear();
				if ( !all )
				{
					for ( const OcclusionRegion &region : regions[j] )
						if ( s >= region.s0 && s <= region.s1 && t >= region.t0 && t <= region.t1 )
							candidates.push_back( near[size_t( region.box )] );
				}
				const std::vector<dynamic_occlusion::Box> &boxes = all ? near : candidates;
				const float visible =
				    boxes.empty()
				        ? 1.0f
				        : dynamic_occlusion::Visibility(
				              p, DynamicOcclusion_WorldLightSamples( wl, position ), boxes, 0 );
				anyBlocked |= visible < 1.0f;
				reaching.push_back( OcclusionLight{ li, visible } );
			}
			if ( !anyBlocked )
				return;
			// Only light the bake let reach the texel counts.
			OcclusionTexel texel = { idx, int( lightsOut.size() ), 0 };
			bool blocked = false;
			for ( const OcclusionLight &light : reaching )
			{
				if ( !DynamicOcclusion_StaticVisible( surfID, idx, light.light, position, n ) )
					continue;
				lightsOut.push_back( light );
				++texel.count;
				blocked |= light.visible < 1.0f;
			}
			if ( blocked )
				texelsOut.push_back( texel );
			else
				lightsOut.resize( size_t( texel.first ) );
		};

		if ( cache.signature != signature || fresh )
		{
			// Keep the texels outside the stale regions; evaluate the stale ones
			// the boxes now cover.
			std::vector<OcclusionTexel> texels;
			std::vector<OcclusionLight> kept;
			for ( const OcclusionTexel &texel : cache.texels )
			{
				if ( stale[size_t( texel.idx )] )
					continue;
				OcclusionTexel copy = { texel.idx, int( kept.size() ), texel.count };
				kept.insert( kept.end(), cache.lights.begin() + texel.first,
				    cache.lights.begin() + texel.first + texel.count );
				texels.push_back( copy );
			}
			for ( int t = 0; t < tmax; ++t )
				for ( int s = 0; s < smax; ++s )
				{
					const size_t idx = size_t( t * smax + s );
					if ( !stale[idx] || !covered[idx] )
						continue;
					++g_OcclusionTexelsEvaluated;
					evaluate( s, t, false, texels, kept );
				}
			cache.texels = std::move( texels );
			cache.lights = std::move( kept );
			cache.boxes = nearEntries;
			cache.signature = signature;
		}

		if ( verify )
		{
			// Every texel against every near box, as if built from nothing.
			std::vector<OcclusionTexel> texels;
			std::vector<OcclusionLight> full;
			for ( int t = 0; t < tmax; ++t )
				for ( int s = 0; s < smax; ++s )
					evaluate( s, t, true, texels, full );
			std::unordered_map<int, const OcclusionTexel *> held;
			for ( const OcclusionTexel &texel : cache.texels )
				held[texel.idx] = &texel;
			g_OcclusionVerified += int( texelCount );
			int differ = 0, common = 0;
			for ( const OcclusionTexel &texel : texels )
			{
				const auto found = held.find( texel.idx );
				if ( found == held.end() )
				{
					++differ; // blocked only in the build from nothing
					continue;
				}
				++common;
				const OcclusionTexel &mine = *found->second;
				bool same = mine.count == texel.count;
				for ( int l = 0; same && l < texel.count; ++l )
				{
					const OcclusionLight &a = cache.lights[size_t( mine.first + l )];
					const OcclusionLight &b = full[size_t( texel.first + l )];
					same = a.light == b.light && a.visible == b.visible;
				}
				differ += !same;
			}
			differ += int( held.size() ) - common; // blocked only in the cache
			g_OcclusionMismatches += differ;
		}
	}

	// Apply: the blocked share of each light, never more than the texel holds
	// (the formula's light scaled to the bake's where the two disagree).
	for ( const OcclusionTexel &texel : cache.texels )
	{
		const int idx = texel.idx;
		const Vector position = luxelBase + ( idx / smax ) * stepT + ( idx % smax ) * stepS;
		Vector bakedDirect( 0, 0, 0 ), blocked( 0, 0, 0 );
		Vector removedDirection[64];
		Vector removed[64];
		int nRemoved = 0;
		for ( int l = texel.first; l < texel.first + texel.count; ++l )
		{
			const OcclusionLight &light = cache.lights[size_t( l )];
			const dworldlight_t &wl = world->worldlights[light.light];
			Vector delta = wl.type == emit_skylight ? -wl.normal : wl.origin - position;
			const float falloff = Engine_WorldLightDistanceFalloff( &wl, delta, false );
			delta.NormalizeInPlace();
			const float angle = Engine_WorldLightAngle( &wl, wl.normal, n, delta );
			const Vector direct = wl.intensity * ( MAX( falloff, 0.0f ) * MAX( angle, 0.0f ) *
			                                         LightStyleValue( wl.style ) );
			bakedDirect += direct;
			if ( light.visible < 1.0f && nRemoved < 64 )
			{
				removed[nRemoved] = direct * ( 1.0f - light.visible );
				removedDirection[nRemoved] = delta;
				blocked += removed[nRemoved];
				++nRemoved;
			}
		}
		const float bakedLuminance = 0.2126f * blocklights[0][idx][0] +
		                             0.7152f * blocklights[0][idx][1] +
		                             0.0722f * blocklights[0][idx][2];
		const float directLuminance =
		    0.2126f * bakedDirect.x + 0.7152f * bakedDirect.y + 0.0722f * bakedDirect.z;
		if ( !( directLuminance > 0.0f ) || blocked.LengthSqr() <= 0.0f )
			continue;
		const float scale = MIN( 1.0f, MAX( bakedLuminance, 0.0f ) / directLuminance );
		{
			extern int g_OcclusionLuxels;
			extern double g_OcclusionRemoved;
			++g_OcclusionLuxels;
			g_OcclusionRemoved += blocked.y * scale;
		}
		for ( int r = 0; r < nRemoved; ++r )
		{
			const Vector take = removed[r] * scale;
			blocklights[0][idx].AsVector3D() -= take;
			if ( needsBumpmap )
			{
				const float lDotN = MAX( DotProduct( removedDirection[r], n ), 1e-3f );
				for ( int b = 0; b < NUM_BUMP_VECTS; ++b )
				{
					const float dot = DotProduct( removedDirection[r], bumpNormals[b] );
					if ( dot > 0.0f )
						blocklights[b + 1][idx].AsVector3D() -= take * ( dot / lDotN );
				}
			}
		}
		for ( int b = 0; b < ( needsBumpmap ? NUM_BUMP_VECTS + 1 : 1 ); ++b )
			for ( int k = 0; k < 3; ++k )
				blocklights[b][idx][k] = MAX( blocklights[b][idx][k], 0.0f );
	}
}

//-----------------------------------------------------------------------------
// RFC 0011 render.projected-light.v1: env_projectedtextures light the world
// like every other light. Each texel in a light's frustum takes its cookie x
// attenuation x Lambert term (projected_light::IrradianceAt), shadowed by the
// world (a cached trace) and by moving objects (render.dynamic-occlusion.v1);
// bumped lightmaps take it along its direction, as a dlight adds light. A
// surface keeps what it takes while the lights and the boxes near it stay put.
//-----------------------------------------------------------------------------
struct ProjectedContribution
{
	int idx;
	Vector light; // flat
	Vector direction;
};
struct ProjectedCache
{
	uint64_t signature = 0;
	int map = -1;
	std::vector<ProjectedContribution> contributions;
};
static std::unordered_map<SurfaceHandle_t, ProjectedCache> g_ProjectedCache;
static std::mutex g_ProjectedCacheMutex;

static void R_AddProjectedLights(
    SurfaceHandle_t surfID, bool needsBumpmap, int generation, int occlusionGeneration )
{
	extern int g_nMapLoadCount;
	const std::vector<ProjectedLightEntry> &lights = ProjectedLights_Get( generation );
	if ( !AreaLights_IsWorldSurface( surfID ) )
		return;
	std::lock_guard<std::mutex> lock( g_ProjectedCacheMutex );
	if ( lights.empty() )
	{
		g_ProjectedCache.erase( surfID );
		return;
	}

	Vector bumpNormals[NUM_BUMP_VECTS];
	Vector luxelBase;
	R_ComputeSurfaceBasis( surfID, bumpNormals, luxelBase );
	mtexinfo_t *tex = MSurf_TexInfo( surfID );
	const float fixupFactor = tex->worldUnitsPerLuxel * tex->worldUnitsPerLuxel;
	const int smax = MSurf_LightmapExtents( surfID )[0] + 1;
	const int tmax = MSurf_LightmapExtents( surfID )[1] + 1;
	const Vector stepS = fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D();
	const Vector stepT = fixupFactor * tex->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D();
	const Vector across = ( smax - 1 ) * stepS + ( tmax - 1 ) * stepT;
	const Vector other = ( smax - 1 ) * stepS - ( tmax - 1 ) * stepT;
	const Vector surfaceCenter = luxelBase + 0.5f * across;
	const float surfaceRadius = 0.5f * MAX( across.Length(), other.Length() );
	const Vector &n = MSurf_Plane( surfID ).normal;
	const float planeDist = MSurf_Plane( surfID ).dist;

	// The lights that reach it, and the boxes near them: the signature.
	std::vector<const ProjectedLightEntry *> reaching;
	uint64_t signature = 1469598103934665603ull;
	auto mix = [&signature]( uint64_t word )
	{
		signature ^= word;
		signature *= 1099511628211ull;
	};
	for ( const ProjectedLightEntry &entry : lights )
	{
		if ( !entry.light.lightsWorld )
			continue;
		float center[3], radius;
		projected_light::BoundingSphere( entry.light, center, &radius );
		if ( ( Vector( center[0], center[1], center[2] ) - surfaceCenter ).Length() >=
		     radius + surfaceRadius )
			continue;
		const Vector origin( entry.light.origin[0], entry.light.origin[1], entry.light.origin[2] );
		if ( DotProduct( origin, n ) - planeDist <= 0.0f )
			continue; // behind the surface
		reaching.push_back( &entry );
		mix( uint64_t( uint32_t( entry.key ) ) );
		mix( entry.version );
	}
	if ( reaching.empty() )
	{
		g_ProjectedCache.erase( surfID );
		return;
	}
	std::vector<dynamic_occlusion::Box> near;
	for ( const OccluderEntry &box : DynamicOcclusion_Get( occlusionGeneration ) )
	{
		const Vector center( box.box.center[0], box.box.center[1], box.box.center[2] );
		if ( ( center - surfaceCenter ).Length() >=
		     surfaceRadius + dynamic_occlusion::Radius( box.box ) + kMaxShadowReach )
			continue;
		near.push_back( box.box );
		mix( uint64_t( uint32_t( box.box.entity ) ) ^ ( uint64_t( box.box.part ) << 32 ) );
		mix( box.version );
	}

	extern double g_ProjectedBuildSeconds, g_ProjectedApplySeconds;
	extern int g_ProjectedBuilds, g_ProjectedTexels;
	ProjectedCache &cache = g_ProjectedCache[surfID];
	const double buildStart = Plat_FloatTime();
	if ( cache.map != g_nMapLoadCount || cache.signature != signature )
	{
		++g_ProjectedBuilds;
		cache.map = g_nMapLoadCount;
		cache.signature = signature;
		cache.contributions.clear();
		for ( const ProjectedLightEntry *entry : reaching )
		{
			const projected_light::Light &light = entry->light;
			const Vector origin( light.origin[0], light.origin[1], light.origin[2] );
			const float lightSamples[3] = { light.origin[0], light.origin[1], light.origin[2] };
			const int lightKey = 2048 | int( entry->version & 2047 );
			for ( int t = 0; t < tmax; ++t )
			{
				Vector position = luxelBase + t * stepT;
				for ( int s = 0; s < smax; ++s, position += stepS )
				{
					const float p[3] = { position.x, position.y, position.z };
					const float normal[3] = { n.x, n.y, n.z };
					float rgb[3];
					projected_light::IrradianceAt(
					    light, p, normal,
					    [&light]( float u, float v, float out[3] )
					    {
						    ProjectedLights_Cookie( light, u, v, out );
					    },
					    rgb );
					if ( MAX( rgb[0], MAX( rgb[1], rgb[2] ) ) < 1.0f / 1024.0f )
						continue;
					const int idx = t * smax + s;
					float visible = 1.0f;
					if ( light.shadows )
					{
						if ( !DynamicOcclusion_StaticVisibleTo(
						         surfID, idx, lightKey, position, n, origin, false ) )
							continue;
						if ( !near.empty() )
							visible = dynamic_occlusion::Visibility( p,
							    dynamic_occlusion::DiskSamples(
							        lightSamples, projected_light::kSourceRadius, p ),
							    near, 0 );
					}
					if ( !( visible > 0.0f ) )
						continue;
					Vector direction = origin - position;
					direction.NormalizeInPlace();
					cache.contributions.push_back( ProjectedContribution{
					    idx, Vector( rgb[0], rgb[1], rgb[2] ) * visible, direction } );
					++g_ProjectedTexels;
				}
			}
		}
	}

	const double applyStart = Plat_FloatTime();
	g_ProjectedBuildSeconds += applyStart - buildStart;
	for ( const ProjectedContribution &c : cache.contributions )
	{
		blocklights[0][c.idx].AsVector3D() += c.light;
		if ( needsBumpmap )
		{
			const float lDotN = MAX( DotProduct( c.direction, n ), 1e-3f );
			for ( int b = 0; b < NUM_BUMP_VECTS; ++b )
			{
				const float dot = DotProduct( c.direction, bumpNormals[b] );
				if ( dot > 0.0f )
					blocklights[b + 1][c.idx].AsVector3D() += c.light * ( dot / lDotN );
			}
		}
	}
	g_ProjectedApplySeconds += Plat_FloatTime() - applyStart;
}

//-----------------------------------------------------------------------------
// Purpose: Compute the mask of which dlights affect a surface
//			NOTE: Also has the side effect of updating the surface lighting dlight flags!
//-----------------------------------------------------------------------------
unsigned int R_ComputeDynamicLightMask( dlight_t *pLights, SurfaceHandle_t surfID, msurfacelighting_t *pLighting, const matrix3x4_t& entityToWorld )
{
	ASSERT_SURF_VALID( surfID );
	Vector bumpNormals[3];
	Vector luxelBasePosition;

	// Displacements do dynamic lights different
	if( SurfaceHasDispInfo( surfID ) )
	{
		return MSurf_DispInfo( surfID )->ComputeDynamicLightMask(pLights);
	}

	if ( !g_bActiveDlights )
		return 0;

	int lightMask = 0;
	for ( int lnum = 0, testBit = 1, mask = r_dlightactive; lnum < MAX_DLIGHTS; lnum++, mask >>= 1, testBit <<= 1 )
	{
		if ( mask & 1 )
		{
			// not lit by this light
			if ( !(pLighting->m_fDLightBits & testBit ) )
				continue;

			// This light doesn't affect the world
			if ( pLights[lnum].flags & (DLIGHT_NO_WORLD_ILLUMINATION|DLIGHT_DISPLACEMENT_MASK))
				continue;

			// This is used to ensure a maximum number of dlights in a frame
			if ( !R_CanUseVisibleDLight( lnum ) ) 
				continue;

			// Cull surface to light radius
			Vector lightOrigin;

			VectorITransform( pLights[lnum].origin, entityToWorld, lightOrigin );

			// NOTE: Dist can be negative because muzzle flashes can actually get behind walls
			// since the gun isn't checked for collision tests.
			float perpDistSq = DotProduct (lightOrigin, MSurf_Plane( surfID ).normal) - MSurf_Plane( surfID ).dist;
			if (perpDistSq < DLIGHT_BEHIND_PLANE_DIST)
			{
				// update the surfacelighting and remove this light's bit
				pLighting->m_fDLightBits &= ~testBit;
				continue;
			}

			perpDistSq *= perpDistSq;

			// If the perp distance > radius of light, blow it off
			float lightRadiusSq = pLights[lnum].GetRadiusSquared();
			if (lightRadiusSq <= perpDistSq)
			{
				// update the surfacelighting and remove this light's bit
				pLighting->m_fDLightBits &= ~testBit;
				continue;
			}

			lightMask |= testBit;
		}
	}

	return lightMask;
}


//-----------------------------------------------------------------------------
// Purpose: Modifies blocklights[][][] to include the state of the dlights 
//			affecting this surface.
//			NOTE: Can be threaded, should not reference or modify any global state 
//			other than blocklights.
//-----------------------------------------------------------------------------
void R_AddDynamicLights( dlight_t *pLights, SurfaceHandle_t surfID, const matrix3x4_t& entityToWorld, bool needsBumpmap, unsigned int lightMask )
{
	ASSERT_SURF_VALID( surfID );
	VPROF( "R_AddDynamicLights" );

	Vector bumpNormals[3];
	bool computedBumpBasis = false;
	Vector luxelBasePosition;

	// Lights imaged through portals (portal_dlights.h) are clipped per luxel;
	// displacements cannot clip yet, so they leave them out.
	const unsigned int portalImages = PortalDLights_ImageMask();

	// Area lights (area_lights.h) are added by R_AddAreaLights, not as points.
	for ( int lnum = 0; lnum < MAX_DLIGHTS; ++lnum )
		if ( pLights[lnum].flags & DLIGHT_AREA )
			lightMask &= ~( 1u << lnum );

	// Displacements do dynamic lights different
	if( SurfaceHasDispInfo( surfID ) )
	{
		MSurf_DispInfo( surfID )->AddDynamicLights( pLights, lightMask & ~portalImages );
		return;
	}

	// iterate all of the active dynamic lights.  Uses several iterators to keep 
	// the light mask (bit), light index, and active mask current
	for ( int lnum = 0, testBit = 1, mask = lightMask; lnum < MAX_DLIGHTS && mask != 0; lnum++, mask >>= 1, testBit <<= 1 )
	{
		// shift over the mask of active lights each iteration, if this one is active, apply it
		if ( mask & 1 )
		{
			// Cull surface to light radius
			Vector lightOrigin;

			VectorITransform( pLights[lnum].origin, entityToWorld, lightOrigin );

			// NOTE: Dist can be negative because muzzle flashes can actually get behind walls
			// since the gun isn't checked for collision tests.
			float perpDistSq = DotProduct (lightOrigin, MSurf_Plane( surfID ).normal) - MSurf_Plane( surfID ).dist;
			if (perpDistSq < DLIGHT_BEHIND_PLANE_DIST)
				continue;

			perpDistSq *= perpDistSq;

			// If the perp distance > radius of light, blow it off
			float lightRadiusSq = pLights[lnum].GetRadiusSquared();
			if (lightRadiusSq <= perpDistSq)
				continue;

			// Here, I'm precomputing things needed by bumped lighting (and by
			// the portal clip) that are the same for a surface...
			const bool portalImage = ( portalImages & testBit ) != 0;
			if ( !computedBumpBasis && ( needsBumpmap || portalImage ) )
			{
				R_ComputeSurfaceBasis( surfID, bumpNormals, luxelBasePosition );
				computedBumpBasis = true;
			}
			PortalLuxelClip clip;
			clip.slot = lnum;
			clip.pLuxelBase = &luxelBasePosition;
			clip.pEntityToWorld = &entityToWorld;
			const PortalLuxelClip *pClip = portalImage ? &clip : NULL;

			if ( !needsBumpmap )
			{
				AddSingleDynamicLight(
				    pLights[lnum], surfID, lightOrigin, perpDistSq, lightRadiusSq, pClip );
				continue;
			}

			AddSingleDynamicLightToBumpLighting( pLights[lnum], surfID, lightOrigin, perpDistSq,
			    lightRadiusSq, bumpNormals, luxelBasePosition, pClip );
		}
	}
}


// Fixed point (8.8) color/intensity ratios
#define I_RED		((int)(0.299*255))
#define I_GREEN		((int)(0.587*255))
#define I_BLUE		((int)(0.114*255))


//-----------------------------------------------------------------------------
// Sets all elements in a lightmap to a particular opaque greyscale value
//-----------------------------------------------------------------------------
static void InitLMSamples( Vector4D *pSamples, int nSamples, float value )
{
	for( int i=0; i < nSamples; i++ )
	{
		pSamples[i][0] = pSamples[i][1] = pSamples[i][2] = value;
		pSamples[i][3] = 1.0f;
	}
}


//-----------------------------------------------------------------------------
// Computes the lightmap size
//-----------------------------------------------------------------------------
static int ComputeLightmapSize( SurfaceHandle_t surfID )
{
	int smax = ( MSurf_LightmapExtents( surfID )[0] ) + 1;
	int tmax = ( MSurf_LightmapExtents( surfID )[1] ) + 1;
	int size = smax * tmax;

	int nMaxSize = MSurf_MaxLightmapSizeWithBorder( surfID );
	if (size > nMaxSize * nMaxSize)
	{
		ConMsg("Bad lightmap extents on material \"%s\"\n", 
			materialSortInfoArray[MSurf_MaterialSortID( surfID )].material->GetName());
		return 0;
	}
	
	return size;
}


//-----------------------------------------------------------------------------
// Compute the portion of the lightmap generated from lightstyles
//-----------------------------------------------------------------------------
static void AccumulateLightstyles( ColorRGBExp32* pLightmap, int lightmapSize, float scalar ) 
{
	Assert( pLightmap );
	for (int i=0; i<lightmapSize ; ++i)
	{
		blocklights[0][i][0] += scalar * TexLightToLinear( pLightmap[i].r, pLightmap[i].exponent );
		blocklights[0][i][1] += scalar * TexLightToLinear( pLightmap[i].g, pLightmap[i].exponent );
		blocklights[0][i][2] += scalar * TexLightToLinear( pLightmap[i].b, pLightmap[i].exponent );
	}
}

static void AccumulateLightstylesFlat( ColorRGBExp32* pLightmap, int lightmapSize, float scalar ) 
{
	Assert( pLightmap );
	for (int i=0; i<lightmapSize ; ++i)
	{
		blocklights[0][i][0] += scalar * TexLightToLinear( pLightmap->r, pLightmap->exponent );
		blocklights[0][i][1] += scalar * TexLightToLinear( pLightmap->g, pLightmap->exponent );
		blocklights[0][i][2] += scalar * TexLightToLinear( pLightmap->b, pLightmap->exponent );
	}
}


static void AccumulateBumpedLightstyles( ColorRGBExp32* pLightmap, int lightmapSize, float scalar ) 
{
	ColorRGBExp32 *pBumpedLightmaps[3];
	pBumpedLightmaps[0] = pLightmap + lightmapSize;
	pBumpedLightmaps[1] = pLightmap + 2 * lightmapSize;
	pBumpedLightmaps[2] = pLightmap + 3 * lightmapSize;

	// I chose to split up the loops this way because it was the best tradeoff
	// based on profiles between cache miss + loop overhead
	for (int i=0 ; i<lightmapSize ; ++i)
	{
		blocklights[0][i][0] += scalar * TexLightToLinear( pLightmap[i].r, pLightmap[i].exponent );
		blocklights[0][i][1] += scalar * TexLightToLinear( pLightmap[i].g, pLightmap[i].exponent );
		blocklights[0][i][2] += scalar * TexLightToLinear( pLightmap[i].b, pLightmap[i].exponent );
		Assert( blocklights[0][i][0] >= 0.0f );
		Assert( blocklights[0][i][1] >= 0.0f );
		Assert( blocklights[0][i][2] >= 0.0f );

		blocklights[1][i][0] += scalar * TexLightToLinear( pBumpedLightmaps[0][i].r, pBumpedLightmaps[0][i].exponent );
		blocklights[1][i][1] += scalar * TexLightToLinear( pBumpedLightmaps[0][i].g, pBumpedLightmaps[0][i].exponent );
		blocklights[1][i][2] += scalar * TexLightToLinear( pBumpedLightmaps[0][i].b, pBumpedLightmaps[0][i].exponent );
		Assert( blocklights[1][i][0] >= 0.0f );
		Assert( blocklights[1][i][1] >= 0.0f );
		Assert( blocklights[1][i][2] >= 0.0f );
	}
	for ( int i=0 ; i<lightmapSize ; ++i)
	{
		blocklights[2][i][0] += scalar * TexLightToLinear( pBumpedLightmaps[1][i].r, pBumpedLightmaps[1][i].exponent );
		blocklights[2][i][1] += scalar * TexLightToLinear( pBumpedLightmaps[1][i].g, pBumpedLightmaps[1][i].exponent );
		blocklights[2][i][2] += scalar * TexLightToLinear( pBumpedLightmaps[1][i].b, pBumpedLightmaps[1][i].exponent );
		Assert( blocklights[2][i][0] >= 0.0f );
		Assert( blocklights[2][i][1] >= 0.0f );
		Assert( blocklights[2][i][2] >= 0.0f );

		blocklights[3][i][0] += scalar * TexLightToLinear( pBumpedLightmaps[2][i].r, pBumpedLightmaps[2][i].exponent );
		blocklights[3][i][1] += scalar * TexLightToLinear( pBumpedLightmaps[2][i].g, pBumpedLightmaps[2][i].exponent );
		blocklights[3][i][2] += scalar * TexLightToLinear( pBumpedLightmaps[2][i].b, pBumpedLightmaps[2][i].exponent );
		Assert( blocklights[3][i][0] >= 0.0f );
		Assert( blocklights[3][i][1] >= 0.0f );
		Assert( blocklights[3][i][2] >= 0.0f );
	}
}

//-----------------------------------------------------------------------------
// Compute the portion of the lightmap generated from lightstyles
//-----------------------------------------------------------------------------
static void ComputeLightmapFromLightstyle( msurfacelighting_t *pLighting, bool computeLightmap, 
				bool computeBumpmap, int lightmapSize, bool hasBumpmapLightmapData )
{
	VPROF( "ComputeLightmapFromLightstyle" );

	ColorRGBExp32 *pLightmap = pLighting->m_pSamples;

	// Compute iteration range
	int minmap, maxmap;
#ifdef USE_CONVARS
	if( r_lightmap.GetInt() != -1 )
	{
		minmap = r_lightmap.GetInt();
		maxmap = minmap + 1;
	}
	else
#endif
	{
		minmap = 0; maxmap = MAXLIGHTMAPS;
	}

	for (int maps = minmap; maps < maxmap && pLighting->m_nStyles[maps] != 255; ++maps)
	{
		if( r_lightstyle.GetInt() != -1 && pLighting->m_nStyles[maps] != r_lightstyle.GetInt())
		{
			continue;
		}

		float fscalar = LightStyleValue( pLighting->m_nStyles[maps] );

		// hack - don't know why we are getting negative values here.
//		if (scalar > 0.0f && maps > 0 )
		if (fscalar > 0.0f)
		{
			const float &scalar = fscalar;

			if( computeBumpmap )
			{
				AccumulateBumpedLightstyles( pLightmap, lightmapSize, scalar );
			}
			else if( computeLightmap )
			{
				if (r_avglightmap.GetInt())
				{
					pLightmap = pLighting->AvgLightColor(maps);
					AccumulateLightstylesFlat( pLightmap, lightmapSize, scalar );
				}
				else
				{
					AccumulateLightstyles( pLightmap, lightmapSize, scalar );
				}
			}
		}

		// skip to next lightmap. If we store lightmap data, we need to jump forward 4
		pLightmap += hasBumpmapLightmapData ? lightmapSize * ( NUM_BUMP_VECTS + 1 ) : lightmapSize;
	}
}

// instrumentation to measure locks
/*
static CUtlVector<int> g_LightmapLocks;
static int g_Lastdlightframe = -1;
static int g_lastlock = -1;
static int g_unsorted = 0;
void MarkPage( int pageID )
{
	if ( g_Lastdlightframe != r_framecount )
	{
		int total = 0;
		int locks = 0;
		for ( int i = 0; i < g_LightmapLocks.Count(); i++ )
		{
			int count = g_LightmapLocks[i];
			if ( count )
			{
				total++;
				locks += count;
			}
			g_LightmapLocks[i] = 0;
		}
		g_Lastdlightframe = r_framecount;
		g_lastlock = -1;
		if ( locks )
		Msg("Total pages %d, locks %d, unsorted locks %d\n", total, locks, g_unsorted );
		g_unsorted = 0;
	}
	if ( pageID != g_lastlock )
	{
		g_lastlock = pageID;
		g_unsorted++;
	}
	g_LightmapLocks.EnsureCount(pageID+1);
	g_LightmapLocks[pageID]++;
}
*/
//-----------------------------------------------------------------------------
// Update the lightmaps...
//-----------------------------------------------------------------------------
static void UpdateLightmapTextures( SurfaceHandle_t surfID, bool needsBumpmap )
{
	ASSERT_SURF_VALID( surfID );

	if( materialSortInfoArray )
	{
		int lightmapSize[2];
		int offsetIntoLightmapPage[2];
		lightmapSize[0] = ( MSurf_LightmapExtents( surfID )[0] ) + 1;
		lightmapSize[1] = ( MSurf_LightmapExtents( surfID )[1] ) + 1;
		offsetIntoLightmapPage[0] = MSurf_OffsetIntoLightmapPage( surfID )[0];
		offsetIntoLightmapPage[1] = MSurf_OffsetIntoLightmapPage( surfID )[1];
		Assert( MSurf_MaterialSortID( surfID ) >= 0 && 
			MSurf_MaterialSortID( surfID ) < g_WorldStaticMeshes.Count() );
		// FIXME: Should differentiate between bumped and unbumped since the perf characteristics
		// are completely different?
//		MarkPage( materialSortInfoArray[MSurf_MaterialSortID( surfID )].lightmapPageID );

		if( needsBumpmap )
		{
			materials->UpdateLightmap( materialSortInfoArray[MSurf_MaterialSortID( surfID )].lightmapPageID,
				lightmapSize, offsetIntoLightmapPage, 
				&blocklights[0][0][0], &blocklights[1][0][0], &blocklights[2][0][0], &blocklights[3][0][0] );
		}
		else
		{
			materials->UpdateLightmap( materialSortInfoArray[MSurf_MaterialSortID( surfID )].lightmapPageID,
				lightmapSize, offsetIntoLightmapPage, 
				&blocklights[0][0][0], NULL, NULL, NULL );
		}
	}
}


unsigned int R_UpdateDlightState( dlight_t *pLights, SurfaceHandle_t surfID, const matrix3x4_t& entityToWorld, bool bOnlyUseLightStyles, bool bLightmap )
{
	unsigned int dlightMask = 0;
	// Mark the surface with the particular cached light values...
	msurfacelighting_t *pLighting = SurfaceLighting( surfID );

	// Retire dlights that are no longer active
	pLighting->m_fDLightBits &= r_dlightactive;
	pLighting->m_nLastComputedFrame = r_framecount;

	// Here, it's got the data it needs. So use it!
	if ( !bOnlyUseLightStyles )
	{
		// add all the dynamic lights
		if( bLightmap && ( pLighting->m_nDLightFrame == r_framecount ) )
		{
			dlightMask = R_ComputeDynamicLightMask( pLights, surfID, pLighting, entityToWorld );
		}

		if ( !dlightMask || !pLighting->m_fDLightBits )
		{
			pLighting->m_fDLightBits = 0;
			MSurf_Flags(surfID) &= ~SURFDRAW_HASDLIGHT;
		}
	}
	return dlightMask;
}

//-----------------------------------------------------------------------------
// Purpose: Build the blocklights array for a given surface and copy to dest
//			Combine and scale multiple lightmaps into the 8.8 format in blocklights
// Input  : *psurf - surface to rebuild
//			*dest - texture pointer to receive copy in lightmap texture format
//			stride - stride of *dest memory
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// RFC 0011: brush entities (doors, func_brush) on a map with a probe volume.
// The pipeline bakes no light into them (they move, so the bake leaves them
// out), and a surface without light data draws fullbright. Instead each
// luxel takes the probe volume's light at its world position and normal
// (visibility-tested), in the lightmap's unit (irradiance / pi, what the
// world's own lightmap holds), refreshed whenever a new volume is published.
// It replaces vrad's data too: on these maps the volume is the lighting
// authority (vrad bakes no lights into them), and a legacy map without a
// volume never gets here.
//-----------------------------------------------------------------------------
static std::unordered_map<SurfaceHandle_t, std::vector<Vector4D>> g_ProbeLitSurfaces;

void R_BuildLightMapGuts( dlight_t *pLights, SurfaceHandle_t surfID,
    const matrix3x4_t &entityToWorld, unsigned int dlightMask, bool needsBumpmap,
    bool needsLightmap, int areaGeneration, int occlusionGeneration, int projectedGeneration );

static bool R_ProbeLitSamples( SurfaceHandle_t surfID, Vector4D *samples, int size )
{
	const auto found = g_ProbeLitSurfaces.find( surfID );
	if ( found == g_ProbeLitSurfaces.end() || int( found->second.size() ) != size )
		return false;
	for ( int i = 0; i < size; ++i )
		samples[i] = found->second[i];
	return true;
}

// The world position of luxel (s, t): on the surface's plane, where the
// lightmap vectors give s and t (luxel centres sit at whole coordinates).
static bool LuxelPosition( SurfaceHandle_t surfID, int s, int t, Vector *out )
{
	const mtexinfo_t *info = MSurf_TexInfo( surfID );
	const cplane_t &plane = MSurf_Plane( surfID );
	const short *mins = MSurf_LightmapMins( surfID );
	const Vector4D &u = info->lightmapVecsLuxelsPerWorldUnits[0];
	const Vector4D &v = info->lightmapVecsLuxelsPerWorldUnits[1];
	const float rows[3][3] = {
	    { u.x, u.y, u.z }, { v.x, v.y, v.z }, { plane.normal.x, plane.normal.y, plane.normal.z } };
	const float rhs[3] = { float( s + mins[0] ) - u.w, float( t + mins[1] ) - v.w, plane.dist };
	const float det = rows[0][0] * ( rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1] ) -
	                  rows[0][1] * ( rows[1][0] * rows[2][2] - rows[1][2] * rows[2][0] ) +
	                  rows[0][2] * ( rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0] );
	if ( fabsf( det ) < 1e-12f )
		return false;
	for ( int k = 0; k < 3; ++k )
	{
		float m[3][3];
		for ( int r = 0; r < 3; ++r )
			for ( int c = 0; c < 3; ++c )
				m[r][c] = c == k ? rhs[r] : rows[r][c];
		( *out )[k] = ( m[0][0] * ( m[1][1] * m[2][2] - m[1][2] * m[2][1] ) -
		                  m[0][1] * ( m[1][0] * m[2][2] - m[1][2] * m[2][0] ) +
		                  m[0][2] * ( m[1][0] * m[2][1] - m[1][1] * m[2][0] ) ) /
		              det;
	}
	return true;
}

// A luxel's light from the volume. Luxels pad past their face, so one on a
// face at the volume's edge (a door's bottom row, on the floor) can lie just
// outside every grid: it takes the light of the nearest point inside one.
static bool ProbeSampleClamped( const mapcontainer::ProbeVolumeView &view, const float at[3],
    const float normal[3], float light[3], std::vector<uint32_t> *probes )
{
	uint32_t read[8];
	if ( view.Sample( at, normal, mapcontainer::ProbeVolumeLayer::Total, true, light ) )
	{
		if ( probes )
			probes->insert( probes->end(), read, read + view.SampleProbes( at, normal, read ) );
		return true;
	}
	const mapcontainer::ProbeVolumeLayout &layout = view.Layout();
	float best[3] = {};
	float bestDistance2 = -1.0f;
	for ( uint32_t g = 0; g < layout.gridCount; ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
		const float smallest = fminf( grid.spacing[0], fminf( grid.spacing[1], grid.spacing[2] ) );
		// Inside by more than the sampler's normal bias, which moves the point.
		const float margin = 1.01f * mapcontainer::kProbeNormalBias * smallest;
		float clamped[3];
		float distance2 = 0.0f;
		for ( int k = 0; k < 3; ++k )
		{
			const float hi = grid.origin[k] + float( grid.dims[k] - 1 ) * grid.spacing[k];
			clamped[k] = fminf( fmaxf( at[k], grid.origin[k] + margin ), hi - margin );
			distance2 += ( clamped[k] - at[k] ) * ( clamped[k] - at[k] );
		}
		if ( bestDistance2 < 0.0f || distance2 < bestDistance2 )
		{
			bestDistance2 = distance2;
			best[0] = clamped[0], best[1] = clamped[1], best[2] = clamped[2];
		}
	}
	if ( bestDistance2 < 0.0f ||
	     !view.Sample( best, normal, mapcontainer::ProbeVolumeLayer::Total, true, light ) )
		return false;
	if ( probes )
		probes->insert( probes->end(), read, read + view.SampleProbes( best, normal, read ) );
	return true;
}

// The probes each probe-lit surface's luxels read, from the last relight of
// every surface: a sparse relight redoes the surfaces that read a changed one.
static std::unordered_map<SurfaceHandle_t, std::vector<uint32_t>> s_ProbeLitReads;

// One surface's probe light: kept for its later rebuilds (dynamic lights
// rebuild from it) and built into its lightmap now. With the queued material
// system every lightmap rebuild runs on the render thread (R_BuildLightMap
// queues R_BuildLightMapGuts), which then owns g_ProbeLitSurfaces and the
// blocklights scratch, and CMaterialSystem::UpdateLightmap ignores a main-
// thread update; so there the relight is queued too.
static void ApplyProbeLitSurface( SurfaceHandle_t surfID, std::vector<Vector4D> luxels )
{
	g_ProbeLitSurfaces[surfID] = std::move( luxels );
	matrix3x4_t identity;
	SetIdentityMatrix( identity );
	R_BuildLightMapGuts( NULL, surfID, identity, 0, SurfNeedsBumpedLightmaps( surfID ), true,
	    AreaLights_Generation(), DynamicOcclusion_Generation(), ProjectedLights_Generation() );
}

static void ClearProbeLitSurfaces()
{
	g_ProbeLitSurfaces.clear();
}

class CQueuedProbeLitSurface : public CFunctorBase
{
public:
	CQueuedProbeLitSurface( SurfaceHandle_t surfID, const std::vector<Vector4D> &luxels )
	    : m_surfID( surfID ), m_luxels( luxels )
	{
	}
	void operator()() override { ApplyProbeLitSurface( m_surfID, std::move( m_luxels ) ); }

private:
	SurfaceHandle_t m_surfID;
	std::vector<Vector4D> m_luxels;
};

bool R_RelightBrushEntitiesFromProbes(
    const mapcontainer::ProbeVolumeView *view, const uint32_t *changedProbes, uint32_t changedCount )
{
	CMatRenderContextPtr pRenderContext( materials );
	ICallQueue *pCallQueue = pRenderContext->GetCallQueue();
	if ( !view )
	{
		s_ProbeLitReads.clear();
		if ( pCallQueue )
			pCallQueue->QueueCall( ClearProbeLitSurfaces );
		else
			ClearProbeLitSurfaces();
		return true;
	}
	// Not yet: the map's lightmap pages are allocated after its volume loads.
	if ( !host_state.worldbrush || !materialSortInfoArray )
		return false;
	static ConVarRef report( "r_indirect_report" );
	const double start = Plat_FloatTime();
	float smallest = FLT_MAX;
	for ( uint32_t g = 0; g < view->Layout().gridCount; ++g )
		for ( const float spacing : view->Layout().grids[g].spacing )
			smallest = fminf( smallest, spacing );
	const float offset = smallest < FLT_MAX ? 0.5f * smallest : 1.0f;
	std::vector<Vector4D> luxels;
	int lit = 0, noLight = 0, baked = 0, noLightmap = 0, whitePage = 0, unchanged = 0;
	// Sparse: the changed probes marked; the reads are rebuilt on a full pass.
	const bool sparse = changedProbes && !s_ProbeLitReads.empty();
	std::vector<uint8_t> changed;
	if ( sparse )
	{
		for ( uint32_t i = 0; i < changedCount; ++i )
		{
			if ( changedProbes[i] >= changed.size() )
				changed.resize( size_t( changedProbes[i] ) + 1, 0 );
			changed[changedProbes[i]] = 1;
		}
	}
	else
	{
		s_ProbeLitReads.clear();
	}
	size_t luxelCount = 0, unsampled = 0;
	double luxelSum = 0.0;
	for ( int model = 1; model < host_state.worldbrush->numsubmodels; ++model )
	{
		char name[16];
		Q_snprintf( name, sizeof( name ), "*%d", model );
		model_t *brush = modelloader->GetModelForName( name, IModelLoader::FMODELLOADER_CLIENT );
		if ( !brush || brush->type != mod_brush )
			continue;
		for ( int i = 0; i < brush->brush.nummodelsurfaces; ++i )
		{
			const SurfaceHandle_t surfID =
			    SurfaceHandleFromIndex( brush->brush.firstmodelsurface + i, brush->brush.pShared );
			if ( MSurf_Flags( surfID ) & SURFDRAW_NOLIGHT )
			{
				++noLight;
				continue;
			}
			if ( SurfHasLightmap( surfID ) )
				++baked; // replaced: the volume is these maps' lighting authority
			if ( !SurfNeedsLightmap( surfID ) )
			{
				++noLightmap;
				continue;
			}
			const int page = materialSortInfoArray[MSurf_MaterialSortID( surfID )].lightmapPageID;
			if ( page == MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE ||
			     page == MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE_BUMP )
			{
				++whitePage;
				continue;
			}
			if ( sparse )
			{
				const auto reads = s_ProbeLitReads.find( surfID );
				bool readsChanged = reads == s_ProbeLitReads.end();
				for ( size_t k = 0; !readsChanged && k < reads->second.size(); ++k )
					readsChanged =
					    reads->second[k] < changed.size() && changed[reads->second[k]];
				if ( !readsChanged )
				{
					++unchanged;
					continue;
				}
			}
			++lit;
			std::vector<uint32_t> &reads = s_ProbeLitReads[surfID];
			reads.clear();
			const int width = MSurf_LightmapExtents( surfID )[0] + 1;
			const int height = MSurf_LightmapExtents( surfID )[1] + 1;
			// The face's own plane: SURFDRAW_PLANEBACK only relates it to its node.
			const Vector normal = MSurf_Plane( surfID ).normal;
			luxels.assign( size_t( width ) * height, Vector4D( 0, 0, 0, 1 ) );
			// Degenerate lightmap vectors (an axis along the normal) place no
			// luxel: the whole face then takes its centroid's light.
			Vector centroid;
			Surf_ComputeCentroid( surfID, &centroid );
			for ( int t = 0; t < height; ++t )
				for ( int s = 0; s < width; ++s )
				{
					Vector position;
					if ( !LuxelPosition( surfID, s, t, &position ) )
						position = centroid;
					// Half a probe spacing off the face, toward the side it shows,
					// so a thin door takes the probes of its own side.
					const float at[3] = { position.x + normal.x * offset,
					    position.y + normal.y * offset, position.z + normal.z * offset };
					const float facing[3] = { normal.x, normal.y, normal.z };
					float light[3] = {};
					++luxelCount;
					if ( ProbeSampleClamped( *view, at, facing, light, &reads ) )
					{
						luxels[size_t( t ) * width + s].Init( light[0], light[1], light[2], 1.0f );
						luxelSum += ( light[0] + light[1] + light[2] ) / 3.0f;
					}
					else
						++unsampled;
				}
			std::sort( reads.begin(), reads.end() );
			reads.erase( std::unique( reads.begin(), reads.end() ), reads.end() );
			if ( pCallQueue )
			{
				CFunctor *apply = new CQueuedProbeLitSurface( surfID, luxels );
				pCallQueue->QueueFunctor( apply );
				apply->Release();
			}
			else
			{
				ApplyProbeLitSurface( surfID, luxels );
			}
		}
	}
	if ( report.IsValid() && report.GetBool() )
		Msg(
		    "indirect light: %d brush-entity surface(s) lit from probes (%d replacing baked light; "
		    "skipped: %d unlit, %d without a lightmap, %d on the white page, %d reading no "
		    "changed probe); %zu luxels, %zu outside the volume, mean %.4f; %.3f ms\n",
		    lit, baked, noLight, noLightmap, whitePage, unchanged, luxelCount, unsampled,
		    luxelCount > unsampled ? luxelSum / double( luxelCount - unsampled ) : 0.0,
		    ( Plat_FloatTime() - start ) * 1000.0 );
	return true;
}

void R_BuildLightMapGuts( dlight_t *pLights, SurfaceHandle_t surfID,
    const matrix3x4_t &entityToWorld, unsigned int dlightMask, bool needsBumpmap,
    bool needsLightmap, int areaGeneration, int occlusionGeneration, int projectedGeneration )
{
	VPROF_("R_BuildLightMapGuts", 1, VPROF_BUDGETGROUP_DLIGHT_RENDERING, false, 0);
	int bumpID;

	// Lightmap data can be dumped to save memory - this precludes any dynamic lighting on the world
	Assert( !host_state.worldbrush->unloadedlightmaps );

	// Mark the surface with the particular cached light values...
	msurfacelighting_t *pLighting = SurfaceLighting( surfID );

	int size = ComputeLightmapSize( surfID );
	if (size == 0)
		return;

	bool hasBumpmap = SurfHasBumpedLightmaps( surfID );
	bool hasLightmap = SurfHasLightmap( surfID );

	// clear to no light
	if( needsLightmap )
	{
		// set to full bright if no light data
		InitLMSamples( blocklights[0], size, hasLightmap ? 0.0f : 1.0f );
	}
	// A brush entity on a map with a probe volume takes its light from the
	// volume, in place of whatever vrad wrote (R_RelightBrushEntitiesFromProbes).
	const bool probeLit = needsLightmap && R_ProbeLitSamples( surfID, blocklights[0], size );

	if( needsBumpmap )
	{
		// set to full bright if no light data
		for( bumpID = 1; bumpID < NUM_BUMP_VECTS + 1; bumpID++ )
		{
			InitLMSamples( blocklights[bumpID], size, hasBumpmap ? 0.0f : 1.0f );
		}
	}

	// add all the lightmaps
	// Here, it's got the data it needs. So use it!
	if ( probeLit )
	{
		for ( bumpID = 1; needsBumpmap && bumpID < NUM_BUMP_VECTS + 1; bumpID++ )
		{
			for ( int i = 0; i < size; i++ )
				blocklights[bumpID][i] = blocklights[0][i];
		}
	}
	else if ( ( hasLightmap && needsLightmap ) || ( hasBumpmap && needsBumpmap ) )
	{
		ComputeLightmapFromLightstyle( pLighting, ( hasLightmap && needsLightmap ),
			( hasBumpmap && needsBumpmap ), size, hasBumpmap );
	}
	else if( !hasBumpmap && needsBumpmap && hasLightmap )
	{
		// make something up for the bumped lights if you need them but don't have the data
		// if you have a lightmap, use that, otherwise fullbright
		ComputeLightmapFromLightstyle( pLighting, true, false, size, hasBumpmap );

		for( bumpID = 0; bumpID < ( hasBumpmap ? ( NUM_BUMP_VECTS + 1 ) : 1 ); bumpID++ )
		{
			for (int i=0 ; i<size ; i++)
			{
				blocklights[bumpID][i].AsVector3D() = blocklights[0][i].AsVector3D();
			}
		}
	}
	else if( needsBumpmap && !hasLightmap )
	{
		// set to full bright if no light data
		InitLMSamples( blocklights[1], size, 0.0f );
		InitLMSamples( blocklights[2], size, 0.0f );
		InitLMSamples( blocklights[3], size, 0.0f );
	}
	else if( !needsBumpmap && !needsLightmap )
	{
	}
	else if( needsLightmap && !hasLightmap )
	{
	}
	else
	{
		Assert( 0 );
	}

	// add all the dynamic lights
	if ( dlightMask && (needsLightmap || needsBumpmap) )
	{
		R_AddDynamicLights( pLights, surfID, entityToWorld, needsBumpmap, dlightMask );
	}

	// and the area lights that reach it (displacements take none yet), less
	// the light moving objects block (render.dynamic-occlusion.v1)
	if ( ( needsLightmap || needsBumpmap ) && !SurfaceHasDispInfo( surfID ) )
	{
		R_AddAreaLights( surfID, entityToWorld, needsBumpmap, areaGeneration );
		R_ApplyDynamicOcclusion( surfID, entityToWorld, needsBumpmap, occlusionGeneration );
		R_AddProjectedLights( surfID, needsBumpmap, projectedGeneration, occlusionGeneration );
	}

	// Update the texture state
	UpdateLightmapTextures( surfID, needsBumpmap );
}

void R_BuildLightMap( dlight_t *pLights, ICallQueue *pCallQueue, SurfaceHandle_t surfID, const matrix3x4_t &entityToWorld, bool bOnlyUseLightStyles )
{
	bool needsBumpmap = SurfNeedsBumpedLightmaps( surfID );
	bool needsLightmap = SurfNeedsLightmap( surfID );

	if( !needsBumpmap && !needsLightmap )
		return;
	
	if( materialSortInfoArray )
	{
		Assert( MSurf_MaterialSortID( surfID ) >= 0 && 
			    MSurf_MaterialSortID( surfID ) < g_WorldStaticMeshes.Count() );
		if (( materialSortInfoArray[MSurf_MaterialSortID( surfID )].lightmapPageID == MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE )	||
		   ( materialSortInfoArray[MSurf_MaterialSortID( surfID )].lightmapPageID == MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE_BUMP ) )
		{
			return;
		}
	}

	bool bDlightsInLightmap = needsLightmap || needsBumpmap;
	unsigned int dlightMask = R_UpdateDlightState( pLights, surfID, entityToWorld, bOnlyUseLightStyles, bDlightsInLightmap );

	// update the state, but don't render any dlights if only lightstyles requested
	if ( bOnlyUseLightStyles )
		dlightMask = 0;

	// The area lights and occluders current now, even if the build runs on
	// another thread.
	const int areaGeneration = AreaLights_Generation();
	const int occlusionGeneration = DynamicOcclusion_Generation();
	const int projectedGeneration = ProjectedLights_Generation();
	if ( !pCallQueue )
	{
		R_BuildLightMapGuts( pLights, surfID, entityToWorld, dlightMask, needsBumpmap,
		    needsLightmap, areaGeneration, occlusionGeneration, projectedGeneration );
	}
	else
	{
		pCallQueue->QueueCall( R_BuildLightMapGuts, pLights, surfID, RefToVal( entityToWorld ),
		    dlightMask, needsBumpmap, needsLightmap, areaGeneration, occlusionGeneration,
		    projectedGeneration );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Save off the average light values, and dump the rest of the lightmap data.
// Can be used to save memory, at the expense of dynamic lights and lightstyles.
//-----------------------------------------------------------------------------
void CacheAndUnloadLightmapData()
{
	Assert( !g_bHunkAllocLightmaps );
	if ( g_bHunkAllocLightmaps )
	{
		return;
	}

	worldbrushdata_t *pBrushData = host_state.worldbrush;
	msurfacelighting_t *pLighting = pBrushData->surfacelighting;
	int numSurfaces = pBrushData->numsurfaces;

	// This will allocate more data than necessary, but only 1-2K max
	byte *pDestBase = (byte*)malloc( numSurfaces * MAXLIGHTMAPS * sizeof( ColorRGBExp32 ) );
	byte *pDest = pDestBase;

	for ( int i = 0; i < numSurfaces; ++i, ++pLighting )
	{
		int nStyleCt = 0;
		for ( int map = 0 ; map < MAXLIGHTMAPS; ++map )
		{
			if ( pLighting->m_nStyles[map] != 255 )
				++nStyleCt;
		}

		const int nHdrBytes = nStyleCt * sizeof( ColorRGBExp32 );
		byte *pHdr = (byte*)pLighting->m_pSamples - nHdrBytes;

		// Copy just the 0-4 average color entries
		Q_memcpy( pDest, pHdr, nHdrBytes );

		// m_pSamples needs to point AFTER the average color data
		pDest += nHdrBytes;
		pLighting->m_pSamples = (ColorRGBExp32*)pDest;
	}

	// Update the lightdata pointer
	free( host_state.worldbrush->lightdata );
	host_state.worldbrush->lightdata = (ColorRGBExp32*)pDestBase;
	host_state.worldbrush->unloadedlightmaps = true;
}

//sorts the surfaces in place
static void SortSurfacesByLightmapID( SurfaceHandle_t *pToSort, int iSurfaceCount )
{
	SurfaceHandle_t *pSortTemp = (SurfaceHandle_t *)stackalloc( sizeof( SurfaceHandle_t ) * iSurfaceCount );
	
	//radix sort
	for( int radix = 0; radix != 4; ++radix )
	{
		//swap the inputs for the next pass
		{
			SurfaceHandle_t *pTemp = pToSort;
			pToSort = pSortTemp;
			pSortTemp = pTemp;
		}

		int iCounts[256] = { 0 };
		int iBitOffset = radix * 8;
		for( int i = 0; i != iSurfaceCount; ++i )
		{
			uint8 val = (materialSortInfoArray[MSurf_MaterialSortID( pSortTemp[i] )].lightmapPageID >> iBitOffset) & 0xFF;
			++iCounts[val];
		}

		int iOffsetTable[256];
		iOffsetTable[0] = 0;
		for( int i = 0; i != 255; ++i )
		{
			iOffsetTable[i + 1] = iOffsetTable[i] + iCounts[i];
		}

		for( int i = 0; i != iSurfaceCount; ++i )
		{
			uint8 val = (materialSortInfoArray[MSurf_MaterialSortID( pSortTemp[i] )].lightmapPageID >> iBitOffset) & 0xFF;
			int iWriteIndex = iOffsetTable[val];
			pToSort[iWriteIndex] = pSortTemp[i];
			++iOffsetTable[val];
		}
	}
}

void R_RedownloadAllLightmaps()
{
#ifdef _DEBUG
	static bool initializedBlockLights = false;
	if (!initializedBlockLights)
	{
		memset( &blocklights[0][0][0], 0, MAX_LIGHTMAP_DIM_INCLUDING_BORDER * MAX_LIGHTMAP_DIM_INCLUDING_BORDER * (NUM_BUMP_VECTS + 1) * sizeof( Vector ) );
		initializedBlockLights = true;
	}
#endif

	double st = Sys_FloatTime();

	bool bOnlyUseLightStyles = false;

	if( r_dynamic.GetInt() == 0 )
	{
		bOnlyUseLightStyles = true;
	}

	// Can't build lightmaps if the source data has been dumped
	CMatRenderContextPtr pRenderContext( materials );
	ICallQueue *pCallQueue = pRenderContext->GetCallQueue();
	if ( !host_state.worldbrush->unloadedlightmaps )
	{		
		int iSurfaceCount = host_state.worldbrush->numsurfaces;
		
		SurfaceHandle_t *pSortedSurfaces = (SurfaceHandle_t *)stackalloc( sizeof( SurfaceHandle_t ) * iSurfaceCount );
		for( int surfaceIndex = 0; surfaceIndex < iSurfaceCount; surfaceIndex++ )
		{
			SurfaceHandle_t surfID = SurfaceHandleFromIndex( surfaceIndex );
			pSortedSurfaces[surfaceIndex] = surfID;
		}

		SortSurfacesByLightmapID( pSortedSurfaces, iSurfaceCount ); //sorts in place, so now the array really is sorted

		if( pCallQueue )
			pCallQueue->QueueCall( materials, &IMaterialSystem::BeginUpdateLightmaps );
		else
			materials->BeginUpdateLightmaps();
		
		matrix3x4_t xform;
		SetIdentityMatrix(xform);
		for( int surfaceIndex = 0; surfaceIndex < iSurfaceCount; surfaceIndex++ )
		{
			SurfaceHandle_t surfID = pSortedSurfaces[surfaceIndex];

			ASSERT_SURF_VALID( surfID );
			R_BuildLightMap( &cl_dlights[0], pCallQueue, surfID, xform, bOnlyUseLightStyles );
		}

		if( pCallQueue )
			pCallQueue->QueueCall( materials, &IMaterialSystem::EndUpdateLightmaps );
		else
			materials->EndUpdateLightmaps();		

		if ( !g_bHunkAllocLightmaps && r_unloadlightmaps.GetInt() == 1 )
		{
			// Delete the lightmap data from memory
			if ( !pCallQueue )
			{
				CacheAndUnloadLightmapData();
			}
			else
			{
				pCallQueue->QueueCall( CacheAndUnloadLightmapData );
			}
		}
	}

	float elapsed = ( float )( Sys_FloatTime() - st ) * 1000.0;
	DevMsg( "R_RedownloadAllLightmaps took %.3f msec!\n", elapsed );

	g_RebuildLightmaps = false;
}

//-----------------------------------------------------------------------------
// Purpose: flag the lightmaps as needing to be rebuilt (gamma change)
//-----------------------------------------------------------------------------
bool g_RebuildLightmaps = false;

void GL_RebuildLightmaps( void )
{
	g_RebuildLightmaps = true;
}


//-----------------------------------------------------------------------------
// Purpose: Update the in-RAM texture for the given surface's lightmap
// Input  : *fa - surface pointer
//-----------------------------------------------------------------------------

#ifdef UPDATE_LIGHTSTYLES_EVERY_FRAME
ConVar mat_updatelightstyleseveryframe( "mat_updatelightstyleseveryframe", "0" );
#endif
void FASTCALL R_RenderDynamicLightmaps ( dlight_t *pLights, ICallQueue *pCallQueue, SurfaceHandle_t surfID, const matrix3x4_t &xform )
{
	VPROF_BUDGET( "R_RenderDynamicLightmaps", VPROF_BUDGETGROUP_DLIGHT_RENDERING );
	ASSERT_SURF_VALID( surfID );

	int fSurfFlags = MSurf_Flags( surfID );

	if( fSurfFlags & SURFDRAW_NOLIGHT )
		return;

	// check for lightmap modification
	bool bChanged = false;
	msurfacelighting_t *pLighting = SurfaceLighting( surfID );
	if( fSurfFlags & SURFDRAW_HASLIGHTSYTLES )
	{
#ifdef UPDATE_LIGHTSTYLES_EVERY_FRAME
		if( mat_updatelightstyleseveryframe.GetBool() && ( pLighting->m_nStyles[0] != 0 || pLighting->m_nStyles[1] != 255 ) )
		{
			bChanged = true;
		}
#endif
		for( int maps = 0; maps < MAXLIGHTMAPS && pLighting->m_nStyles[maps] != 255; maps++ )
		{
			if( d_lightstyleframe[pLighting->m_nStyles[maps]] > pLighting->m_nLastComputedFrame )
			{
				bChanged = true;
				break;
			}
		}
	}

	// was it dynamic this frame (pLighting->m_nDLightFrame == r_framecount) 
	// or dynamic previously (pLighting->m_fDLightBits)
	bool bDLightChanged = ( pLighting->m_nDLightFrame == r_framecount ) || pLighting->m_fDLightBits;
	// or it holds an area light that is gone or changed (area_lights.h)
	if ( !bDLightChanged && ( fSurfFlags & SURFDRAW_HASDLIGHT ) )
		bDLightChanged = AreaLights_IsDirty( surfID ) || DynamicOcclusion_IsDirty( surfID );
	bool bOnlyUseLightStyles = false;

	if( r_dynamic.GetInt() == 0 )
	{
		bOnlyUseLightStyles = true;
		bDLightChanged = false;
	}

	if ( bChanged || bDLightChanged )
	{
		R_BuildLightMap( pLights, pCallQueue, surfID, xform, bOnlyUseLightStyles );
	}
}
