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
#include "host.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/IShader.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/itexture.h"
#include "tier1/KeyValues.h"
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

// No escape hatch (user direction, 2026-09-28): what the core claimed, legacy
// never draws. Only a material the model cannot draw is legacy's, decided at
// level load and named in r_core_world_stats.
static ConVar r_core_world_strict( "r_core_world_strict", "1", 0,
    "RFC 0016 K5: a view or claimed material the render core fails to draw is fatal (1, the "
    "default). 0 reports each failure and leaves the surfaces undrawn; legacy never draws what "
    "the core claimed" );

static ConVar r_core_world_seed_failure( "r_core_world_seed_failure", "", FCVAR_CHEAT,
    "RFC 0016 K5 negative control: the named material reaches the core without its texture "
    "handles, so the core claims it and fails it at its first view (fatal under "
    "r_core_world_strict). Read at level load" );

namespace
{

struct CoreWorldState
{
	bool loaded = false;
	bool viewActive = false;
	unsigned long long failuresSeen = 0;
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
	// Each variable's neutral value (its shader's, see NeutralMaterial), or
	// an empty string with a null pointer where there is none.
	CUtlVector<CUtlString> defaultValues;
	CUtlVector<bool> hasDefault;
	CUtlVector<const char *> defaults;
};

// The material flags by their VMT keys (CShaderSystem::ShaderStateString's
// names): the material system folds them into $flags, and the core reads
// them as keys.
const struct
{
	MaterialVarFlags_t flag;
	const char *key;
} s_FlagKeys[] = {
    { MATERIAL_VAR_NO_DRAW, "$no_draw" },
    { MATERIAL_VAR_VERTEXCOLOR, "$vertexcolor" },
    { MATERIAL_VAR_VERTEXALPHA, "$vertexalpha" },
    { MATERIAL_VAR_SELFILLUM, "$selfillum" },
    { MATERIAL_VAR_ADDITIVE, "$additive" },
    { MATERIAL_VAR_ALPHATEST, "$alphatest" },
    { MATERIAL_VAR_MULTIPASS, "$multipass" },
    { MATERIAL_VAR_ZNEARER, "$znearer" },
    { MATERIAL_VAR_MODEL, "$model" },
    { MATERIAL_VAR_FLAT, "$flat" },
    { MATERIAL_VAR_NOCULL, "$nocull" },
    { MATERIAL_VAR_NOFOG, "$nofog" },
    { MATERIAL_VAR_IGNOREZ, "$ignorez" },
    { MATERIAL_VAR_DECAL, "$decal" },
    { MATERIAL_VAR_ENVMAPSPHERE, "$envmapsphere" },
    { MATERIAL_VAR_NOALPHAMOD, "$noalphamod" },
    { MATERIAL_VAR_ENVMAPCAMERASPACE, "$envmapcameraspace" },
    { MATERIAL_VAR_BASEALPHAENVMAPMASK, "$basealphaenvmapmask" },
    { MATERIAL_VAR_TRANSLUCENT, "$translucent" },
    { MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK, "$normalmapalphaenvmapmask" },
    { MATERIAL_VAR_ENVMAPMODE, "$envmapmode" },
    { MATERIAL_VAR_HALFLAMBERT, "$halflambert" },
    { MATERIAL_VAR_WIREFRAME, "$wireframe" },
    { MATERIAL_VAR_ALLOWALPHATOCOVERAGE, "$allowalphatocoverage" },
    { MATERIAL_VAR_IGNORE_ALPHA_MODULATION, "$ignore_alpha_modulation" },
};

// Legacy's neutral values for a shader's parameters: a material of that
// shader with no VMT parameters, initialized by the shader (its InitParams
// sets what an absent parameter means). One per shader, for one level load.
struct NeutralMaterials
{
	CUtlDict<IMaterial *, int> byShader;

	IMaterial *For( const char *pShader )
	{
		const int found = byShader.Find( pShader );
		if ( found != byShader.InvalidIndex() )
			return byShader[found];
		char name[256];
		V_snprintf( name, sizeof( name ), "__render_core_neutral/%s", pShader );
		IMaterial *pMaterial = materials->CreateMaterial( name, new KeyValues( pShader ) );
		byShader.Insert( pShader, pMaterial );
		return pMaterial;
	}
	~NeutralMaterials()
	{
		for ( int i = byShader.First(); i != byShader.InvalidIndex(); i = byShader.Next( i ) )
		{
			if ( IMaterial *pMaterial = byShader[i] )
			{
				pMaterial->DecrementReferenceCount();
				pMaterial->DeleteIfUnreferenced();
			}
		}
	}
};

// The shader a material runs (after fallback), by name.
IShader *FindShader( const char *pName )
{
	static CUtlVector<IShader *> s_Shaders;
	if ( s_Shaders.Count() != materials->ShaderCount() )
	{
		s_Shaders.SetCount( materials->ShaderCount() );
		s_Shaders.SetCount( materials->GetShaders( 0, s_Shaders.Count(), s_Shaders.Base() ) );
	}
	for ( int i = 0; i < s_Shaders.Count(); ++i )
	{
		if ( s_Shaders[i] && !V_stricmp( s_Shaders[i]->GetName(), pName ) )
			return s_Shaders[i];
	}
	return NULL;
}

void ReadVariables( IMaterial *pMaterial, NeutralMaterials &neutrals, MaterialVars &out )
{
	out.material = pMaterial;
	IMaterialVar **ppParams = pMaterial->GetShaderParams();
	const int nParams = pMaterial->ShaderParamCount();
	IShader *pShader = FindShader( pMaterial->GetShaderName() );
	IMaterial *pNeutral = neutrals.For( pMaterial->GetShaderName() );
	for ( int i = 0; i < nParams; ++i )
	{
		IMaterialVar *pVar = ppParams[i];
		if ( !pVar || !pVar->IsDefined() )
			continue;
		// The flag words: their flags follow as keys.
		if ( !V_stricmp( pVar->GetName(), "$flags" ) ||
		     !V_stricmp( pVar->GetName(), "$flags_defined" ) ||
		     !V_stricmp( pVar->GetName(), "$flags2" ) ||
		     !V_stricmp( pVar->GetName(), "$flags_defined2" ) )
			continue;
		out.keys.AddToTail( CUtlString( pVar->GetName() ) );
		out.values.AddToTail( CUtlString( pVar->GetStringValue() ) );
		out.textures.AddToTail(
		    pVar->GetType() == MATERIAL_VAR_TYPE_TEXTURE ? pVar->GetTextureValue() : NULL );
		// The neutral material's value; else the shader's declared default
		// (the material's parameters are its shader's, in order).
		bool found = false;
		IMaterialVar *pNeutralVar =
		    pNeutral ? pNeutral->FindVar( pVar->GetName(), &found, false ) : NULL;
		const bool declared = pShader && i < pShader->GetNumParams() &&
		                      !V_stricmp( pShader->GetParamName( i ), pVar->GetName() );
		if ( found && pNeutralVar && pNeutralVar->IsDefined() )
			out.defaultValues.AddToTail( CUtlString( pNeutralVar->GetStringValue() ) );
		else
			out.defaultValues.AddToTail(
			    CUtlString( declared ? pShader->GetParamDefault( i ) : "" ) );
		out.hasDefault.AddToTail(
		    ( found && pNeutralVar && pNeutralVar->IsDefined() ) || declared );
	}
	for ( const auto &flag : s_FlagKeys )
	{
		if ( !pMaterial->GetMaterialVarFlag( flag.flag ) )
			continue;
		out.keys.AddToTail( CUtlString( flag.key ) );
		out.values.AddToTail( CUtlString( "1" ) );
		out.textures.AddToTail( NULL );
		out.defaultValues.AddToTail(
		    CUtlString( pNeutral && pNeutral->GetMaterialVarFlag( flag.flag ) ? "1" : "0" ) );
		out.hasDefault.AddToTail( true );
	}
	for ( int i = 0; i < out.keys.Count(); ++i )
	{
		out.keyPtrs.AddToTail( out.keys[i].Get() );
		out.valuePtrs.AddToTail( out.values[i].Get() );
		out.defaults.AddToTail( out.hasDefault[i] ? out.defaultValues[i].Get() : NULL );
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

// The core's failures since the last check, under r_core_world_strict (the
// render sequence records a frame behind the main thread in queued mode, so
// a failure surfaces here within a frame or two).
void CheckFailures( IRenderCoreWorld *pWorld )
{
	CoreWorldState &state = State();
	const unsigned long long failures = pWorld->Failures();
	if ( failures == state.failuresSeen )
		return;
	RenderCoreWorldStats stats;
	pWorld->GetStats( &stats );
	const unsigned long long newFailures = failures - state.failuresSeen;
	state.failuresSeen = failures;
	if ( r_core_world_strict.GetBool() )
	{
		Sys_Error( "r_core_world: the render core failed %llu claimed view(s): %s "
		           "(r_core_world_strict 0 reports failures instead; legacy never draws what the "
		           "core claimed)\n",
		    newFailures, stats.lastFailure );
	}
	Warning( "r_core_world: the render core failed %llu claimed view(s), surfaces left undrawn: "
	         "%s\n",
	    newFailures, stats.lastFailure );
}

} // namespace

void RenderCoreWorldDraw_LevelInit()
{
	CoreWorldState &state = State();
	state.loaded = false;
	state.takes.RemoveAll();
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( pWorld )
		state.failuresSeen = pWorld->Failures();
	worldbrushdata_t *pBrush = host_state.worldbrush;
	if ( !pWorld || !pBrush )
		return;
	CUtlVector<RenderCoreWorldVertex> vertices;
	CUtlVector<unsigned int> indices;
	CUtlVector<RenderCoreWorldSurface> surfaces;
	CUtlVector<MaterialVars> materials;
	NeutralMaterials neutrals;
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
			ReadVariables( pMaterial, neutrals, materials[material] );
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
		desc.defaults = materials[m].defaults.Base();
		if ( r_core_world_seed_failure.GetString()[0] &&
		     !V_stricmp( desc.name, r_core_world_seed_failure.GetString() ) )
		{
			for ( int t = 0; t < materials[m].textures.Count(); ++t )
				materials[m].textures[t] = NULL;
		}
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
	{
		CheckFailures( pWorld );
		pWorld->ClearWorld();
	}
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
	if ( !pWorld )
		return;
	CheckFailures( pWorld );
	if ( nCount <= 0 )
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
	state.viewActive = pWorld->DrawView( reinterpret_cast<const unsigned int *>( pSurfaces ),
	    nCount, toClip, viewport, static_cast<unsigned long long>( host_framecount ) + 1 );
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
	     "drawn %llu failed %llu skipped %llu surfaces drawn %llu last failure '%s'\n",
	    stats.materials, stats.claimedMaterials, stats.surfaces, stats.claimedSurfaces,
	    stats.viewsQueued, stats.viewsDrawn, stats.viewsFailed, stats.viewsSkipped,
	    stats.surfacesDrawn, stats.lastFailure );
	if ( stats.gaps[0] )
		Msg( "r_core_world_stats: gaps:\n%s", stats.gaps );
	if ( stats.claimed[0] )
		Msg( "r_core_world_stats: drawn by the core:\n%s", stats.claimed );
}
