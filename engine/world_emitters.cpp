//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Self-illuminated world geometry as area lights (world_emitters.h).
//
//===========================================================================//

#include "render_pch.h"
#include "world_emitters.h"

#include "Overlay.h"
#include "convar.h"
#include "gl_model_private.h"
#include "materialsystem/selfillum_emission.h"
#include "render/emissive_area_lights.h"
#include "sample_textures.h"

#include <map>
#include <set>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_nMapLoadCount;

static ConVar r_area_lights_world( "r_area_lights_world", "1", 0,
    "Self-illuminated world faces and overlays (chamber icons, exit signs) light their "
    "surroundings as area lights" );

namespace
{

struct State
{
	int map = -1;
	std::vector<WorldEmitter> emitters;
};

State &S()
{
	static State state;
	return state;
}

struct Loader
{
	const vtf_sample::Texture *operator()( const char *pName ) const
	{
		return EngineSampleTexture( pName );
	}
};

// The emission rule of each material, once.
struct Materials
{
	std::map<IMaterial *, selfillum_emission::Emission> emissions;
	std::set<IMaterial *> none;

	const selfillum_emission::Emission *Of( IMaterial *pMaterial )
	{
		if ( !pMaterial || none.count( pMaterial ) )
			return NULL;
		const auto found = emissions.find( pMaterial );
		if ( found != emissions.end() )
			return &found->second;
		selfillum_emission::Emission emission;
		Loader load;
		if ( !emission.Init( pMaterial, load ) )
		{
			none.insert( pMaterial );
			return NULL;
		}
		return &( emissions[pMaterial] = emission );
	}
};

struct Builder
{
	Materials materials;
	std::vector<emissive::Triangle> triangles;
	std::vector<IMaterial *> groupMaterials;

	// A polygon of one group on a surface facing `normal`, fanned into
	// triangles (each facing the surface's way, whatever the winding) and
	// sampled.
	void AddPolygon( int group, const selfillum_emission::Emission &emission, const Vector &normal,
	    const Vector *positions, const Vector2D *uvs, int count )
	{
		for ( int i = 1; i + 1 < count; ++i )
		{
			int corner[3] = { 0, i, i + 1 };
			const Vector geometric =
			    CrossProduct( positions[i] - positions[0], positions[i + 1] - positions[0] );
			if ( DotProduct( geometric, normal ) < 0.0f )
				V_swap( corner[1], corner[2] );
			emissive::Triangle triangle;
			triangle.group = group;
			float uv[3][2];
			for ( int c = 0; c < 3; ++c )
			{
				for ( int k = 0; k < 3; ++k )
					triangle.p[c][k] = positions[corner[c]][k];
				uv[c][0] = uvs[corner[c]].x;
				uv[c][1] = uvs[corner[c]].y;
			}
			emissive::SampleTriangle( uv, emission.m_pBase->width, emission.m_pBase->height,
			    emission, triangle.radiance );
			triangles.push_back( triangle );
		}
	}

	// Each overlay is one group, however many faces clip it into fragments.
	std::map<int, int> overlayGroups;
	int OverlayGroup( int iOverlay, IMaterial *pMaterial )
	{
		const auto found = overlayGroups.find( iOverlay );
		if ( found != overlayGroups.end() )
			return found->second;
		return overlayGroups[iOverlay] = NewGroup( pMaterial );
	}

	int NewGroup( IMaterial *pMaterial )
	{
		groupMaterials.push_back( pMaterial );
		return int( groupMaterials.size() ) - 1;
	}
};

void VisitOverlayFragment( void *pContext, int iOverlay, IMaterial *pMaterial, const Vector &normal,
    const Vector *pPositions, const Vector2D *pTexCoords, int nCount )
{
	Builder &builder = *static_cast<Builder *>( pContext );
	const selfillum_emission::Emission *pEmission = builder.materials.Of( pMaterial );
	if ( !pEmission )
		return;
	builder.AddPolygon( builder.OverlayGroup( iOverlay, pMaterial ), *pEmission, normal, pPositions,
	    pTexCoords, nCount );
}

void Build( State &s )
{
	for ( WorldEmitter &emitter : s.emitters )
		emitter.material->DecrementReferenceCount();
	s.emitters.clear();
	worldbrushdata_t *world = host_state.worldbrush;
	const model_t *worldModel = host_state.worldmodel;
	if ( !world || !worldModel || !r_area_lights_world.GetBool() )
		return;

	// Texture lights vrad baked: their faces' light is already in the lightmaps.
	std::set<int> bakedTexinfos;
	for ( int i = 0; i < world->numworldlights; ++i )
		if ( world->worldlights[i].type == emit_surface )
			bakedTexinfos.insert( world->worldlights[i].texinfo );

	Builder builder;
	// Brush faces of the world model.
	CUtlVector<Vector> positions;
	CUtlVector<Vector2D> uvs;
	for ( int i = 0; i < worldModel->brush.nummodelsurfaces; ++i )
	{
		SurfaceHandle_t surfID = SurfaceHandleFromIndex( worldModel->brush.firstmodelsurface + i );
		if ( MSurf_Flags( surfID ) & ( SURFDRAW_NODRAW | SURFDRAW_SKY ) ||
		     SurfaceHasDispInfo( surfID ) )
			continue;
		mtexinfo_t *pTexInfo = MSurf_TexInfo( surfID );
		if ( !pTexInfo || bakedTexinfos.count( int( pTexInfo - world->texinfo ) ) )
			continue;
		const selfillum_emission::Emission *pEmission = builder.materials.Of( pTexInfo->material );
		if ( !pEmission )
			continue;
		const int nCount = MSurf_VertCount( surfID );
		if ( nCount < 3 )
			continue;
		const float width = float( MAX( pTexInfo->material->GetMappingWidth(), 1 ) );
		const float height = float( MAX( pTexInfo->material->GetMappingHeight(), 1 ) );
		positions.SetCount( nCount );
		uvs.SetCount( nCount );
		for ( int v = 0; v < nCount; ++v )
		{
			const Vector &p =
			    world->vertexes[world->vertindices[MSurf_FirstVertIndex( surfID ) + v]].position;
			positions[v] = p;
			uvs[v].x = ( DotProduct( p, pTexInfo->textureVecsTexelsPerWorldUnits[0].AsVector3D() ) +
			               pTexInfo->textureVecsTexelsPerWorldUnits[0][3] ) /
			           width;
			uvs[v].y = ( DotProduct( p, pTexInfo->textureVecsTexelsPerWorldUnits[1].AsVector3D() ) +
			               pTexInfo->textureVecsTexelsPerWorldUnits[1][3] ) /
			           height;
		}
		builder.AddPolygon( builder.NewGroup( pTexInfo->material ), *pEmission,
		    MSurf_Plane( surfID ).normal, positions.Base(), uvs.Base(), nCount );
	}
	// Overlays (the chamber icons, most signage).
	OverlayMgr()->EnumerateFragments( VisitOverlayFragment, &builder );

	for ( const emissive::Emitter &emitter : emissive::BuildEmitters( builder.triangles ) )
	{
		WorldEmitter out;
		out.light.rect = emitter.rect;
		for ( int k = 0; k < 3; ++k )
			out.light.radiance[k] = emitter.radiance[k];
		out.material = builder.groupMaterials[size_t( emitter.group )];
		out.material->IncrementReferenceCount();
		s.emitters.push_back( out );
	}
}

} // namespace

const std::vector<WorldEmitter> &WorldEmitters_Get()
{
	State &s = S();
	if ( s.map != g_nMapLoadCount )
	{
		s.map = g_nMapLoadCount;
		Build( s );
	}
	return s.emitters;
}
