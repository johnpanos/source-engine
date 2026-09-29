//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The BSP world drawn by the render core (RFC 0016 K5); see
// render_core_world_draw.h.
//
//=============================================================================//

#include "render_pch.h"
#include "render_core_world_draw.h"
#include "render_core_host.h"
#include "render_core_world.h"
#include "render/composition/render_core_world.h"
#include "gl_matsysiface.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/itexture.h"
#include "tier1/convar.h"
#include "tier1/utldict.h"
#include "tier2/tier2.h"
#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar r_core_world( "r_core_world", "0", FCVAR_CHEAT,
    "RFC 0016 K5: the render core draws the BSP world surfaces its material model draws, at a "
    "core pass slot of the legacy stream (1); the legacy world chains skip them. 3 is the "
    "negative control: legacy skips them and the core does not draw them" );

static ConVar r_core_world_isolate( "r_core_world_isolate", "0", FCVAR_CHEAT,
    "RFC 0016 K5 pixel oracle: the legacy world chains draw only the surfaces the core's model "
    "takes (1), with r_core_world 0 or 1, so a frame compares exactly those surfaces" );

namespace
{

struct CoreWorldState
{
	bool loaded = false;
	bool viewActive = false;
	CUtlVector<unsigned char> takes; // per surface index
};

CoreWorldState &State()
{
	static CoreWorldState s_State;
	return s_State;
}

// One material's variables, kept alive for SetWorld.
struct MaterialVars
{
	IMaterial *material = NULL;
	CUtlVector<CUtlString> keys;
	CUtlVector<CUtlString> values;
	CUtlVector<const char *> keyPtrs;
	CUtlVector<const char *> valuePtrs;
	CUtlVector<ITexture *> textures;
};

void ReadVariables( IMaterial *pMaterial, MaterialVars &out )
{
	out.material = pMaterial;
	IMaterialVar **ppParams = pMaterial->GetShaderParams();
	const int nParams = pMaterial->ShaderParamCount();
	for ( int i = 0; i < nParams; ++i )
	{
		IMaterialVar *pVar = ppParams[i];
		if ( !pVar || !pVar->IsDefined() )
			continue;
		out.keys.AddToTail( CUtlString( pVar->GetName() ) );
		out.values.AddToTail( CUtlString( pVar->GetStringValue() ) );
		out.textures.AddToTail(
		    pVar->GetType() == MATERIAL_VAR_TYPE_TEXTURE ? pVar->GetTextureValue() : NULL );
	}
	for ( int i = 0; i < out.keys.Count(); ++i )
	{
		out.keyPtrs.AddToTail( out.keys[i].Get() );
		out.valuePtrs.AddToTail( out.values[i].Get() );
	}
}

bool SurfaceEligible( SurfaceHandle_t surfID )
{
	if ( SurfaceHasDispInfo( surfID ) || MSurf_VertCount( surfID ) < 3 )
		return false;
	if ( MSurf_Flags( surfID ) & ( SURFDRAW_NODRAW | SURFDRAW_SKY | SURFDRAW_WATERSURFACE ) )
		return false;
	mtexinfo_t *pTexInfo = MSurf_TexInfo( surfID );
	return pTexInfo && pTexInfo->material;
}

} // namespace

void RenderCoreWorldDraw_LevelInit()
{
	CoreWorldState &state = State();
	state.loaded = false;
	state.takes.RemoveAll();
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	worldbrushdata_t *pBrush = host_state.worldbrush;
	if ( !pWorld || !pBrush )
		return;
	CUtlVector<RenderCoreWorldVertex> vertices;
	CUtlVector<unsigned int> indices;
	CUtlVector<RenderCoreWorldSurface> surfaces;
	CUtlVector<MaterialVars> materials;
	CUtlVector<int> surfaceOfIndex; // per surface index: its surface entry, or -1
	surfaceOfIndex.SetCount( pBrush->numsurfaces );
	for ( int i = 0; i < pBrush->numsurfaces; ++i )
	{
		surfaceOfIndex[i] = -1;
		SurfaceHandle_t surfID = SurfaceHandleFromIndex( i, pBrush );
		if ( !SurfaceEligible( surfID ) )
			continue;
		IMaterial *pMaterial = MSurf_TexInfo( surfID )->material;
		int material = -1;
		for ( int m = 0; m < materials.Count(); ++m )
		{
			if ( materials[m].material == pMaterial )
			{
				material = m;
				break;
			}
		}
		if ( material < 0 )
		{
			material = materials.AddToTail();
			ReadVariables( pMaterial, materials[material] );
		}

		SurfaceCtx_t ctx;
		SurfSetupSurfaceContext( ctx, surfID );
		const unsigned int firstVertex = vertices.Count();
		const int nVerts = MSurf_VertCount( surfID );
		for ( int v = 0; v < nVerts; ++v )
		{
			const int vertIndex = pBrush->vertindices[MSurf_FirstVertIndex( surfID ) + v];
			Vector &position = pBrush->vertexes[vertIndex].position;
			RenderCoreWorldVertex vertex;
			vertex.position[0] = position.x;
			vertex.position[1] = position.y;
			vertex.position[2] = position.z;
			Vector2D uv;
			SurfComputeTextureCoordinate( ctx, surfID, position, uv );
			vertex.uv[0] = uv.x;
			vertex.uv[1] = uv.y;
			SurfComputeLightmapCoordinate( ctx, surfID, position, uv );
			vertex.lightmapUv[0] = uv.x;
			vertex.lightmapUv[1] = uv.y;
			vertex.color[0] = vertex.color[1] = vertex.color[2] = vertex.color[3] = 255;
			vertices.AddToTail( vertex );
		}
		RenderCoreWorldSurface surface;
		surface.material = material;
		surface.lightmapPage = SortInfoToLightmapPage( MSurf_MaterialSortID( surfID ) );
		surface.firstIndex = indices.Count();
		for ( int t = 1; t + 1 < nVerts; ++t )
		{
			indices.AddToTail( firstVertex );
			indices.AddToTail( firstVertex + t );
			indices.AddToTail( firstVertex + t + 1 );
		}
		surface.indexCount = indices.Count() - surface.firstIndex;
		surfaceOfIndex[i] = surfaces.AddToTail( surface );
	}

	CUtlVector<RenderCoreWorldMaterial> materialDescs;
	for ( int m = 0; m < materials.Count(); ++m )
	{
		RenderCoreWorldMaterial desc;
		desc.name = materials[m].material->GetName();
		desc.shader = materials[m].material->GetShaderName();
		desc.variableCount = materials[m].keys.Count();
		desc.keys = materials[m].keyPtrs.Base();
		desc.values = materials[m].valuePtrs.Base();
		desc.textures = materials[m].textures.Base();
		materialDescs.AddToTail( desc );
	}
	pWorld->SetWorld( vertices.Base(), vertices.Count(), indices.Base(), indices.Count(),
	    surfaces.Base(), surfaces.Count(), materialDescs.Base(), materialDescs.Count() );

	// A surface is the core's when the core draws its material.
	state.takes.SetCount( pBrush->numsurfaces );
	int nTaken = 0;
	for ( int i = 0; i < pBrush->numsurfaces; ++i )
	{
		const int entry = surfaceOfIndex[i];
		state.takes[i] = entry >= 0 && pWorld->Draws( surfaces[entry].material ) ? 1 : 0;
		nTaken += state.takes[i];
	}
	state.loaded = true;
	RenderCoreWorldStats stats;
	pWorld->GetStats( &stats );
	if ( r_core_world.GetBool() )
	{
		Msg( "r_core_world: %d of %d surfaces, %u of %u materials in the core's model\n", nTaken,
		    pBrush->numsurfaces, stats.claimedMaterials, stats.materials );
		if ( stats.gaps[0] )
			Msg( "r_core_world: materials outside the model yet:\n%s", stats.gaps );
	}
}

void RenderCoreWorldDraw_LevelShutdown()
{
	CoreWorldState &state = State();
	state.loaded = false;
	state.viewActive = false;
	state.takes.RemoveAll();
	if ( IRenderCoreWorld *pWorld = RenderCoreHost_World() )
		pWorld->ClearWorld();
}

bool RenderCoreWorldDraw_ViewEligible( unsigned long flags )
{
	const CoreWorldState &state = State();
	if ( !state.loaded || !r_core_world.GetBool() || RenderCoreWorld_ViewDepth() != 1 )
		return false;
	if ( flags & ( DRAWWORLDLISTS_DRAW_SHADOWDEPTH | DRAWWORLDLISTS_DRAW_SSAO |
	                 DRAWWORLDLISTS_DRAW_REFRACTION | DRAWWORLDLISTS_DRAW_REFLECTION ) )
		return false;
	CMatRenderContextPtr pRenderContext( materials );
	// Only the back buffer is a slot target, and the model has no fog term yet.
	return pRenderContext->GetRenderTarget() == NULL &&
	       pRenderContext->GetFogMode() == MATERIAL_FOG_NONE;
}

bool RenderCoreWorldDraw_Takes( SurfaceHandle_t surfID )
{
	const CoreWorldState &state = State();
	const int index = MSurf_Index( surfID );
	return state.loaded && index >= 0 && index < state.takes.Count() && state.takes[index];
}

void RenderCoreWorldDraw_BeginView( const unsigned int *pSurfaces, int nCount )
{
	CoreWorldState &state = State();
	state.viewActive = false;
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( !pWorld || nCount <= 0 )
		return;
	if ( r_core_world.GetInt() == 3 )
	{
		state.viewActive = true; // negative control: skipped and not drawn
		return;
	}
	CMatRenderContextPtr pRenderContext( materials );
	VMatrix view, projection;
	pRenderContext->GetMatrix( MATERIAL_VIEW, &view );
	pRenderContext->GetMatrix( MATERIAL_PROJECTION, &projection );
	const VMatrix worldToClip = projection * view;
	float toClip[16];
	for ( int r = 0; r < 4; ++r )
		for ( int c = 0; c < 4; ++c )
			toClip[r * 4 + c] = worldToClip.m[r][c];
	int x, y, width, height;
	pRenderContext->GetViewport( x, y, width, height );
	const float viewport[6] = {
	    float( x ), float( y ), float( width ), float( height ), 0.0f, 1.0f };
	state.viewActive = pWorld->DrawView(
	    reinterpret_cast<const unsigned int *>( pSurfaces ), nCount, toClip, viewport );
}

void RenderCoreWorldDraw_EndView()
{
	State().viewActive = false;
}

bool RenderCoreWorldDraw_Skips( SurfaceHandle_t surfID )
{
	const CoreWorldState &state = State();
	if ( state.loaded && r_core_world_isolate.GetBool() && !RenderCoreWorldDraw_Takes( surfID ) )
		return true; // the oracle's isolation: only the taken surfaces draw
	return state.viewActive && RenderCoreWorldDraw_Takes( surfID );
}

CON_COMMAND( r_core_world_stats, "RFC 0016 K5: the core world's surfaces, views and gaps" )
{
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( !pWorld )
	{
		Msg( "r_core_world_stats: no render core\n" );
		return;
	}
	RenderCoreWorldStats stats;
	pWorld->GetStats( &stats );
	Msg( "r_core_world_stats: materials %u claimed %u surfaces %u claimed %u views queued %llu "
	     "drawn %llu failed %llu surfaces drawn %llu last failure '%s'\n",
	    stats.materials, stats.claimedMaterials, stats.surfaces, stats.claimedSurfaces,
	    stats.viewsQueued, stats.viewsDrawn, stats.viewsFailed, stats.surfacesDrawn,
	    stats.lastFailure );
	if ( stats.gaps[0] )
		Msg( "r_core_world_stats: gaps:\n%s", stats.gaps );
	if ( stats.claimed[0] )
		Msg( "r_core_world_stats: drawn by the core:\n%s", stats.claimed );
}
