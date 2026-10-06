//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The BSP world drawn by the render core (RFC 0016 K5); see
// render_core_world_draw.h.
//
//=============================================================================//

#include "render_pch.h"
#include "render_core_world_draw.h"
#include "render_core_host.h"
#include "cmodel_engine.h"
#include "render_core_world.h"
#include "client.h"
#include "staticpropmgr.h"
#include "modelloader.h"
#include "ModelInfo.h"
#include "studio.h"
#include "indirect_light_host.h"
#include "render/composition/render_core_world.h"
#include "gl_matsysiface.h"
#include "gl_lightmap.h"
#include "gl_rmain.h"
#include "host.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/IShader.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/itexture.h"
#include "filesystem.h"
#include "networkstringtabledefs.h"
#include "sys_dll.h"
#include "tier1/KeyValues.h"
#include "tier1/fmtstr.h"
#include "tier1/convar.h"
#include "tier1/utldict.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"
#include "tier1/utlvector.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "render/legacy/material_flag_keys.h"
#include "render/material/vmt_matrix.h"
#include "tier0/memdbgon.h"

static void CoreWorldModeChanged( IConVar *variable, const char *, float previous )
{
	// Rebuild once on either transition: an unchanged light must not leave
	// a legacy contribution in core pages, or be missing when legacy resumes.
	ConVarRef mode( variable );
	if ( ( int( previous ) == 1 ) != ( mode.GetInt() == 1 ) )
		GL_RebuildLightmaps();
}

static ConVar r_core_world( "r_core_world", "0", FCVAR_CHEAT,
    "RFC 0016: core-only Forward+ shading (1). Legacy shader draws are rejected, and "
    "unsupported cohorts are reported rather than rendered by a fallback. 3 is the "
    "negative control: legacy skips them and the core does not draw them",
    CoreWorldModeChanged );

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
	// The r_core_runtime_direct the direct occlusion was last built under
	// (BuildDirectOcclusion), so a change of it rebuilds the occlusion.
	int occlusionRuntimeDirect = -1;
	bool viewActive = false;
	// The map's world is its WMSH, which the core holds as a world stage
	// (RFC 0016 K12): views name meshlets, and stageTakes says per WMSH batch
	// whether the core draws its material.
	bool stageWorld = false;
	bool stageView = false; // this view's WMSH world is the core's
	CUtlVector<unsigned char> stageTakes;
	// Views declined because the map's world is its WMSH, this level.
	unsigned long long worldMeshViews = 0;
	unsigned long long failuresSeen = 0;
	CUtlVector<unsigned char> takes; // per surface index
	// Textures the core's materials name that their shaders did not load,
	// held until the level ends.
	CUtlVector<ITexture *> heldTextures;
	// Per surface index: its entry in the core's world (only eligible
	// surfaces have one), or -1. Views name the core's entries.
	CUtlVector<int> entryOf;
	// Static meshes first (their prop indices are stable), then the Studio
	// models the client precache table supplies for posed draws.
	std::vector<const model_t *> registeredModels;
	// Per registered model: the client may draw it as a posed model (it is in
	// the precache table), so the core keeps its per-frame skinning copy. A
	// static prop the precache table never names is static-only and keeps none.
	std::vector<bool> registeredPosed;
	std::vector<unsigned long long> posedClaims; // per registered model, this level
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

struct NeutralMaterials
{
	IMaterial *For( const char *shader )
	{
		IRenderCoreWorld *world = RenderCoreHost_World();
		return world ? world->NeutralMaterial( shader ) : NULL;
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

void ReadVariables(
    IMaterial *pMaterial, NeutralMaterials &neutrals, MaterialVars &out, bool nativeMesh = false )
{
	out.material = pMaterial;
	IMaterialVar **ppParams = pMaterial->GetShaderParams();
	const int nParams = pMaterial->ShaderParamCount();
	IShader *pShader = FindShader( pMaterial->GetShaderName() );
	IMaterial *pNeutral = neutrals.For( pMaterial->GetShaderName() );
	CUtlVector<const char *> pendingKeys; // flag-like keys from texture properties
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
		// GetStringValue prints matrix columns; the core VMT reader takes rows.
		out.values.AddToTail( CUtlString(
		    pVar->GetType() == MATERIAL_VAR_TYPE_MATRIX
		        ? RenderMaterialVmt::MatrixValue( pVar->GetMatrixValue().Base() ).c_str()
		        : pVar->GetStringValue() ) );
		const bool declaredHere = pShader && i < pShader->GetNumParams() &&
		                          !V_stricmp( pShader->GetParamName( i ), pVar->GetName() );
		// A model or symbolic $envmap selects the stage's RPRB image light.
		// Importing its legacy cube can resolve a placeholder through the
		// current view, or turn a missing map-specific VTF into an error texture.
		const bool nativeProbe =
		    !V_stricmp( pVar->GetName(), "$envmap" ) &&
		    ( nativeMesh || !V_stricmp( out.values.Tail().String(), "env_cubemap" ) );
		ITexture *pTexture = !nativeProbe && pVar->GetType() == MATERIAL_VAR_TYPE_TEXTURE
		                         ? pVar->GetTextureValue()
		                         : NULL;
		// A texture parameter its shader did not load (a cubemap-patched
		// brush material's bump map, sp_a3_crazy_box): the texture it names,
		// held for the level, so the core samples what the material names.
		if ( !nativeProbe && !pTexture && declaredHere &&
		     pShader->GetParamType( i ) == SHADER_PARAM_TYPE_TEXTURE && pVar->GetStringValue() &&
		     pVar->GetStringValue()[0] )
		{
			ITexture *pFound =
			    materials->FindTexture( pVar->GetStringValue(), TEXTURE_GROUP_WORLD, false );
			if ( pFound && !pFound->IsError() )
			{
				pFound->IncrementReferenceCount();
				State().heldTextures.AddToTail( pFound );
				pTexture = pFound;
			}
		}
		// A texture no draw has used yet has not reached the renderer (Portal
		// 2 loads a patched material's bump map when first drawn): the core
		// imports by handle, so it is sent now.
		IRenderCoreWorld *pCore = RenderCoreHost_World();
		if ( pTexture && pCore && !pCore->TextureResident( pTexture ) )
			pTexture->Download();
		// A texture the content does not ship (sp_a3_crazy_box's patched
		// walls name metal/metalwall_bts_001a_normal, which no Portal 2
		// archive holds) never reaches the renderer: the variable is absent
		// for the core (its neutral value, a flat normal for a bump map),
		// named once, not a failure of the core.
		if ( pTexture && pCore && !pCore->TextureResident( pTexture ) &&
		     !g_pFileSystem->FileExists(
		         CFmtStr( "materials/%s.vtf", pVar->GetStringValue() ).Get(), "GAME" ) )
		{
			static CUtlDict<bool, int> s_Reported;
			if ( s_Reported.Find( pVar->GetStringValue() ) == s_Reported.InvalidIndex() )
			{
				s_Reported.Insert( pVar->GetStringValue(), true );
				Msg( "r_core_world: material %s names %s %s, which the content lacks; the core "
				     "draws it without\n",
				    pMaterial->GetName(), pVar->GetName(), pVar->GetStringValue() );
			}
			out.values.Tail() = CUtlString( "" );
			pTexture = NULL;
		}
		out.textures.AddToTail( pTexture );
		// A detail texture flagged as an ssbump selects the ssbump detail
		// modes (10, 11): a key the model does not read yet, so it is named.
		if ( pVar->GetType() == MATERIAL_VAR_TYPE_TEXTURE && pVar->GetTextureValue() &&
		     !V_stricmp( pVar->GetName(), "$detail" ) &&
		     ( pVar->GetTextureValue()->GetFlags() & TEXTUREFLAGS_SSBUMP ) )
		{
			pendingKeys.AddToTail( "$detail_ssbump" );
		}
		// The neutral material's value; else the shader's declared default
		// (the material's parameters are its shader's, in order).
		bool found = false;
		IMaterialVar *pNeutralVar =
		    pNeutral ? pNeutral->FindVar( pVar->GetName(), &found, false ) : NULL;
		const bool declared = pShader && i < pShader->GetNumParams() &&
		                      !V_stricmp( pShader->GetParamName( i ), pVar->GetName() );
		if ( found && pNeutralVar && pNeutralVar->IsDefined() )
			out.defaultValues.AddToTail( CUtlString(
			    pNeutralVar->GetType() == MATERIAL_VAR_TYPE_MATRIX
			        ? RenderMaterialVmt::MatrixValue( pNeutralVar->GetMatrixValue().Base() ).c_str()
			        : pNeutralVar->GetStringValue() ) );
		else
			out.defaultValues.AddToTail(
			    CUtlString( declared ? pShader->GetParamDefault( i ) : "" ) );
		out.hasDefault.AddToTail(
		    ( found && pNeutralVar && pNeutralVar->IsDefined() ) || declared );
	}
	for ( int i = 0; i < pendingKeys.Count(); ++i )
	{
		out.keys.AddToTail( CUtlString( pendingKeys[i] ) );
		out.values.AddToTail( CUtlString( "1" ) );
		out.textures.AddToTail( NULL );
		out.defaultValues.AddToTail( CUtlString( "" ) );
		out.hasDefault.AddToTail( false );
	}
	for ( const auto &flag : RenderLegacyMaterialFlags::Keys )
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
	// Water surfaces are eligible: the water family claims their materials
	// (render/material/water_family.h); its point draws them at the height
	// the view moves them to (RenderCoreWorldDraw_BeginView's waterZOffset).
	if ( MSurf_Flags( surfID ) & ( SURFDRAW_NODRAW | SURFDRAW_SKY ) )
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

static bool ReadStaticModelFile( const std::string &path, std::string &bytes )
{
	CUtlBuffer buffer;
	if ( !g_pFileSystem->ReadFile( path.c_str(), "GAME", buffer ) )
		return false;
	bytes.assign( static_cast<const char *>( buffer.Base() ), buffer.TellPut() );
	return true;
}

static void LevelInitModels( IRenderCoreWorld *pWorld )
{
	struct ModelSource
	{
		std::string name, mdl, vvd, vtx;
		std::vector<std::unique_ptr<MaterialVars>> variables;
		std::vector<RenderCoreWorldMaterial> materials;
	};
	const int staticModelCount = StaticPropMgr_CoreModelCount();
	CoreWorldState &state = State();
	state.registeredModels.clear();
	state.registeredPosed.clear();
	state.registeredModels.reserve( staticModelCount );
	state.registeredPosed.reserve( staticModelCount );
	for ( int i = 0; i < staticModelCount; ++i )
	{
		state.registeredModels.push_back( StaticPropMgr_CoreModel( i ) );
		state.registeredPosed.push_back( false );
	}
	// Loading every precached Studio model is a core-world composition step.
	// The default legacy path keeps its lazy model loading behavior.
	const int precached = r_core_world.GetInt() == 1 && cl.m_pModelPrecacheTable
	                          ? std::min( cl.m_pModelPrecacheTable->GetNumStrings(), MAX_MODELS )
	                          : 0;
	for ( int i = 2; i < precached; ++i )
	{
		const char *name = cl.m_pModelPrecacheTable->GetString( i );
		const std::size_t length = name ? std::strlen( name ) : 0;
		if ( length < 4 || V_stricmp( name + length - 4, ".mdl" ) )
			continue;
		const model_t *model = cl.GetModel( i );
		if ( !model || model->type != mod_studio )
			continue;
		const auto held =
		    std::find( state.registeredModels.begin(), state.registeredModels.end(), model );
		studiohdr_t *header = modelinfo->GetStudiomodel( model );
		if ( !header || header->numbones <= 0 || header->numbones > 255 ||
		     header->numflexdesc != 0 )
			continue;
		if ( held != state.registeredModels.end() )
		{
			// Already registered as a static prop: the precache table makes it
			// posable too, which the core needs to know before it drops the
			// model's per-frame skinning copy.
			state.registeredPosed[held - state.registeredModels.begin()] = true;
			continue;
		}
		state.registeredModels.push_back( model );
		state.registeredPosed.push_back( true );
	}
	const int modelCount = int( state.registeredModels.size() );
	state.posedClaims.assign( modelCount, 0 );
	// A model the precache table never names keeps no per-frame skinning copy
	// in the core (RFC 0016 model geometry residency), so a product boot can
	// report how much of the set that is.
	int staticOnly = 0;
	for ( const bool posed : state.registeredPosed )
		staticOnly += posed ? 0 : 1;
	Msg( "r_core_world: %d static models, %d candidate posed models from %d scanned precache "
	     "entries, %d static-only\n",
	    staticModelCount, modelCount - staticModelCount, precached, staticOnly );
	const int propCount = StaticPropMgr_CorePropCount();
	std::vector<ModelSource> sources( modelCount );
	std::vector<RenderCoreStaticModel> models( modelCount );
	NeutralMaterials neutrals;
	int missingModel = 0, missingMdl = 0, missingVvd = 0, missingVtx = 0;
	int missingMaterials = 0;
	for ( int i = 0; i < modelCount; ++i )
	{
		const model_t *model = state.registeredModels[i];
		if ( !model )
		{
			++missingModel;
			continue;
		}
		ModelSource &source = sources[i];
		source.name = modelloader->GetName( const_cast<model_t *>( model ) );
		if ( source.name.size() < 4 ||
		     V_stricmp( source.name.c_str() + source.name.size() - 4, ".mdl" ) )
			continue;
		const std::string stem = source.name.substr( 0, source.name.size() - 4 );
		if ( !ReadStaticModelFile( source.name, source.mdl ) )
		{
			Msg( "r_core_world: model %s missing GAME file %s\n", source.name.c_str(),
			    source.name.c_str() );
			++missingMdl;
			continue;
		}
		if ( !ReadStaticModelFile( stem + ".vvd", source.vvd ) )
		{
			Msg( "r_core_world: model %s missing GAME file %s.vvd\n", source.name.c_str(),
			    stem.c_str() );
			++missingVvd;
			continue;
		}
		for ( const char *suffix : { ".dx90.vtx", ".vtx", ".dx80.vtx", ".sw.vtx" } )
		{
			if ( ReadStaticModelFile( stem + suffix, source.vtx ) )
				break;
		}
		if ( source.vtx.empty() )
		{
			Msg( "r_core_world: model %s has no GAME VTX companion\n", source.name.c_str() );
			++missingVtx;
			continue;
		}
		// A Studio model's header owns the stable texture slots. Hardware data
		// resolves those slots independently for each LOD, including VTX
		// material replacements; the core descriptors use the same LOD-major
		// order as the imported geometry.
		studiohdr_t *header = modelinfo->GetStudiomodel( model );
		const int count = header ? header->numtextures : 0;
		studiohwdata_t *hardware = g_pMDLCache->GetHardwareData( model->studio );
		const int lodCount = hardware ? hardware->m_NumLODs : 0;
		if ( count <= 0 || lodCount <= 0 || !hardware->m_pLODs )
		{
			++missingMaterials;
			continue;
		}
		source.variables.reserve( std::size_t( count ) * lodCount );
		source.materials.reserve( std::size_t( count ) * lodCount );
		bool completeMaterials = true;
		for ( int lod = 0; lod < lodCount && completeMaterials; ++lod )
		{
			const studioloddata_t &lodData = hardware->m_pLODs[lod];
			if ( lodData.numMaterials != count || !lodData.ppMaterials )
			{
				completeMaterials = false;
				break;
			}
			for ( int slot = 0; slot < count; ++slot )
			{
				IMaterial *material = lodData.ppMaterials[slot];
				if ( !material )
				{
					completeMaterials = false;
					break;
				}
				source.variables.push_back( std::make_unique<MaterialVars>() );
				MaterialVars &vars = *source.variables.back();
				ReadVariables( material, neutrals, vars, true );
				RenderCoreWorldMaterial desc;
				desc.name = material->GetName();
				desc.shader = material->GetShaderName();
				desc.variableCount = vars.keys.Count();
				desc.keys = vars.keyPtrs.Base();
				desc.values = vars.valuePtrs.Base();
				desc.textures = vars.textures.Base();
				desc.defaults = vars.defaults.Base();
				desc.hasProxy = material->HasProxy();
				desc.translucent = material->IsTranslucent();
				source.materials.push_back( desc );
			}
		}
		if ( !completeMaterials || source.materials.size() != std::size_t( count ) * lodCount )
		{
			source.variables.clear();
			source.materials.clear();
			++missingMaterials;
			continue;
		}
		RenderCoreStaticModel &out = models[i];
		out.posed = state.registeredPosed[i];
		out.name = source.name.c_str();
		out.mdl = source.mdl.data();
		out.mdlBytes = source.mdl.size();
		out.vvd = source.vvd.data();
		out.vvdBytes = source.vvd.size();
		out.vtx = source.vtx.data();
		out.vtxBytes = source.vtx.size();
		out.materials = source.materials.data();
		out.materialCount = unsigned( count );
		out.materialLodCount = unsigned( lodCount );
	}
	Msg( "r_core_world: Studio model inputs: %d models, missing model %d, mdl %d, vvd %d, "
	     "vtx %d, materials %d\n",
	    modelCount, missingModel, missingMdl, missingVvd, missingVtx, missingMaterials );
	std::vector<RenderCoreStaticProp> props( propCount );
	for ( int i = 0; i < propCount; ++i )
	{
		const model_t *model = NULL;
		unsigned char alpha = 0;
		float modulation[3] = {};
		StaticPropMgr_CorePropInfo(
		    i, &model, props[i].world, &props[i].skin, &alpha, modulation, &props[i].castsShadow );
		studiohdr_t *header = model ? modelinfo->GetStudiomodel( model ) : NULL;
		if ( header && ( header->flags & STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS ) )
			props[i].castsShadow = false;
		props[i].model = ~0u;
		for ( int m = 0; m < staticModelCount; ++m )
		{
			if ( StaticPropMgr_CoreModel( m ) == model )
			{
				props[i].model = unsigned( m );
				break;
			}
		}
		if ( alpha != 255 || modulation[0] != 1.0f || modulation[1] != 1.0f ||
		     modulation[2] != 1.0f || !model || modelinfo->IsTranslucent( model ) ||
		     modelinfo->ModelHasMaterialProxy( model ) )
			props[i].skin = -1;
	}
	pWorld->SetStaticProps(
	    models.data(), unsigned( models.size() ), props.data(), unsigned( props.size() ) );
	int claimed = 0;
	int eligible = 0;
	for ( int i = 0; i < propCount; ++i )
	{
		eligible += props[i].skin >= 0 ? 1 : 0;
		if ( pWorld->DrawsStaticProp( unsigned( i ) ) )
			++claimed;
	}
	Msg( "r_core_world: static props: %d of %d instances in rendercore, %d eligible\n", claimed,
	    propCount, eligible );
}

// A BSP2 map's world is its WMSH (RFC 0016 K12): the core holds it as a
// world stage, its meshlets the surfaces and its batches' materials read as
// the BSP faces' are; the stage's lighting reached the core with the world
// mesh uploads (RenderCoreHost_WorldMeshUpload).
static void LevelInitStage( IRenderCoreWorld *pWorld, worldbrushdata_t *pBrush )
{
	CoreWorldState &state = State();
	CUtlVector<MaterialVars> materials;
	NeutralMaterials neutrals;
	CUtlVector<int> materialOfBatch;
	for ( unsigned int b = 0; b < pBrush->worldMeshBatchCount; ++b )
	{
		IMaterial *pMaterial = pBrush->pWorldMeshBatches[b].material;
		int material = -1;
		for ( int m = 0; m < materials.Count() && pMaterial; ++m )
		{
			if ( materials[m].material == pMaterial )
				material = m;
		}
		if ( material < 0 && pMaterial )
		{
			material = materials.AddToTail();
			ReadVariables( pMaterial, neutrals, materials[material] );
		}
		materialOfBatch.AddToTail( material );
	}
	// One surface per meshlet, in the mesh's meshlet order: a view names the
	// visible meshlets by their index. A batch without a material has no
	// surface the core can take.
	CUtlVector<RenderCoreWorldMeshlet> meshlets;
	meshlets.SetCount( pBrush->worldMeshClusterCount );
	for ( unsigned int m = 0; m < pBrush->worldMeshClusterCount; ++m )
	{
		meshlets[m].material = ~0u;
		meshlets[m].firstIndex = pBrush->pWorldMeshClusters[m].firstIndex;
		meshlets[m].indexCount = pBrush->pWorldMeshClusters[m].indexCount;
	}
	for ( unsigned int b = 0; b < pBrush->worldMeshBatchCount; ++b )
	{
		const worldmeshbatch_t &batch = pBrush->pWorldMeshBatches[b];
		for ( unsigned int m = batch.firstMeshlet;
		    m < batch.firstMeshlet + batch.meshletCount && m < pBrush->worldMeshClusterCount; ++m )
			meshlets[m].material = materialOfBatch[b] >= 0 ? unsigned( materialOfBatch[b] ) : ~0u;
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
		materialDescs.AddToTail( desc );
	}
	pWorld->SetWorldMesh( pBrush->pWorldMeshData, pBrush->worldMeshSize, meshlets.Base(),
	    meshlets.Count(), materialDescs.Base(), materialDescs.Count(), CM_EntityString() );
	LevelInitModels( pWorld );
	state.stageTakes.SetCount( pBrush->worldMeshBatchCount );
	int taken = 0;
	for ( unsigned int b = 0; b < pBrush->worldMeshBatchCount; ++b )
	{
		state.stageTakes[b] =
		    materialOfBatch[b] >= 0 && pWorld->Draws( unsigned( materialOfBatch[b] ) ) ? 1 : 0;
		taken += state.stageTakes[b];
	}
	state.takes.SetCount( pBrush->numsurfaces );
	for ( int i = 0; i < state.takes.Count(); ++i )
		state.takes[i] = 0;
	RenderCoreWorldStats stats;
	pWorld->GetStats( &stats );
	// The core holds no stage without the map's lightmap (it said why):
	// then its views are declined by name.
	state.stageWorld = stats.surfaces != 0;
	state.loaded = true;
	if ( r_core_world.GetBool() )
	{
		Msg( "r_core_world: world stage (WMSH): %d of %u batches, %u of %u materials in the "
		     "core's model\n",
		    taken, pBrush->worldMeshBatchCount, stats.claimedMaterials, stats.materials );
		if ( stats.gaps[0] )
			Msg( "r_core_world: materials outside the model yet:\n%s", stats.gaps );
	}
}

// The index ranges of the world mesh the render core does not light itself
// (the frozen backend's batches, or every batch without a stage drawn under
// runtime direct light): DirectOcclusion's surfaces (indirect_light_host.h).
static void BuildDirectOcclusion()
{
	CoreWorldState &state = State();
	worldbrushdata_t *pBrush = host_state.worldbrush;
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	RenderCoreWorldStats stats{};
	if ( pWorld )
		pWorld->GetStats( &stats );
	const bool coreLights =
	    pWorld && state.stageWorld && r_core_world.GetBool() && stats.stageRuntimeDirect != 0;
	static ConVarRef r_core_runtime_direct( "r_core_runtime_direct" );
	state.occlusionRuntimeDirect = r_core_runtime_direct.GetInt();
	if ( !coreLights || !pBrush || !pBrush->pWorldMeshBatches || !pBrush->pWorldMeshClusters )
	{
		IndirectLight_BuildDirectOcclusion( nullptr, 0, true );
		return;
	}
	CUtlVector<IndirectLightIndexRange> ranges;
	for ( unsigned int b = 0; b < pBrush->worldMeshBatchCount; ++b )
	{
		if ( b < unsigned( state.stageTakes.Count() ) && state.stageTakes[b] )
			continue;
		const worldmeshbatch_t &batch = pBrush->pWorldMeshBatches[b];
		for ( unsigned int m = batch.firstMeshlet;
		    m < batch.firstMeshlet + batch.meshletCount && m < pBrush->worldMeshClusterCount; ++m )
		{
			const IndirectLightIndexRange range = { pBrush->pWorldMeshClusters[m].firstIndex,
			    pBrush->pWorldMeshClusters[m].indexCount };
			if ( ranges.Count() && ranges.Tail().first + ranges.Tail().count == range.first )
				ranges.Tail().count += range.count;
			else
				ranges.AddToTail( range );
		}
	}
	IndirectLight_BuildDirectOcclusion( ranges.Base(), ranges.Count(), false );
}

static void LevelInitWorld();

void RenderCoreWorldDraw_LevelInit()
{
	LevelInitWorld();
	// Only now is it known which surfaces the core lights itself.
	BuildDirectOcclusion();
}

static void LevelInitWorld()
{
	CoreWorldState &state = State();
	state.loaded = false;
	state.stageWorld = false;
	state.stageView = false;
	state.stageTakes.RemoveAll();
	state.worldMeshViews = 0;
	state.takes.RemoveAll();
	state.entryOf.RemoveAll();
	state.registeredModels.clear();
	state.registeredPosed.clear();
	state.posedClaims.clear();
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( pWorld )
		state.failuresSeen = pWorld->Failures();
	worldbrushdata_t *pBrush = host_state.worldbrush;
	if ( !pWorld || !pBrush )
		return;
	if ( pBrush->pWorldMeshData && pBrush->pWorldMeshBatches && pBrush->worldMeshBatchCount &&
	     pBrush->pWorldMeshClusters )
	{
		LevelInitStage( pWorld, pBrush );
		return;
	}
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

		// The vertex as BuildMSurfaceVertexArrays builds the world's static
		// mesh: coordinates, the vertex normal, the tangent basis on tangent
		// space surfaces and the bumped pages' offset on bumped ones.
		SurfaceCtx_t ctx;
		SurfSetupSurfaceContext( ctx, surfID );
		Vector tVect( 0, 0, 0 );
		bool negate = false;
		const bool tangentSpace = ( MSurf_Flags( surfID ) & SURFDRAW_TANGENTSPACE ) != 0;
		if ( tangentSpace )
			negate = TangentSpaceSurfaceSetup( surfID, tVect );
		const float bumpOffset =
		    ( MSurf_Flags( surfID ) & SURFDRAW_BUMPLIGHT ) ? ctx.m_BumpSTexCoordOffset : 0.0f;
		const unsigned int firstVertex = vertices.Count();
		const int nVerts = MSurf_VertCount( surfID );
		for ( int v = 0; v < nVerts; ++v )
		{
			const int vertIndex = pBrush->vertindices[MSurf_FirstVertIndex( surfID ) + v];
			Vector &position = pBrush->vertexes[vertIndex].position;
			RenderCoreWorldVertex vertex = {};
			const Vector &normal =
			    pBrush->vertnormals[pBrush->vertnormalindices[MSurf_FirstVertNormal( surfID ) + v]];
			Vector tangentS( 1, 0, 0 ), tangentT( 0, 1, 0 );
			if ( tangentSpace )
				TangentSpaceComputeBasis( tangentS, tangentT, normal, tVect, negate );
			for ( int c = 0; c < 3; ++c )
			{
				vertex.normal[c] = normal[c];
				vertex.tangentS[c] = tangentS[c];
				vertex.tangentT[c] = tangentT[c];
			}
			vertex.lightmapOffset = bumpOffset;
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
			// BSP brush fans use Source's clockwise front face. Normalize to
			// the core world's counter-clockwise convention (also used by WMSH).
			indices.AddToTail( firstVertex );
			indices.AddToTail( firstVertex + t + 1 );
			indices.AddToTail( firstVertex + t );
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
	state.entryOf.Swap( surfaceOfIndex );
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
	state.registeredModels.clear();
	state.registeredPosed.clear();
	state.posedClaims.clear();
	for ( int i = 0; i < state.heldTextures.Count(); ++i )
		state.heldTextures[i]->DecrementReferenceCount();
	state.heldTextures.RemoveAll();
	state.loaded = false;
	state.viewActive = false;
	state.stageWorld = false;
	state.stageView = false;
	state.stageTakes.RemoveAll();
	state.takes.RemoveAll();
	state.entryOf.RemoveAll();
	if ( IRenderCoreWorld *pWorld = RenderCoreHost_World() )
	{
		CheckFailures( pWorld );
		pWorld->ClearWorld();
	}
}

bool RenderCoreWorldDraw_ViewEligible( unsigned long flags, bool bWorldMeshWorld )
{
	CoreWorldState &state = State();
	if ( !state.loaded || !r_core_world.GetBool() || RenderCoreWorld_ViewDepth() < 1 )
		return false;
	if ( flags & ( DRAWWORLDLISTS_DRAW_SHADOWDEPTH | DRAWWORLDLISTS_DRAW_SSAO |
	                 DRAWWORLDLISTS_DRAW_REFRACTION | DRAWWORLDLISTS_DRAW_REFLECTION ) )
		return false;
	if ( bWorldMeshWorld && !state.stageWorld )
	{
		if ( state.worldMeshViews++ == 0 )
			Msg( "r_core_world: this map's world is its WMSH, and the core holds no world "
			     "stage for it (the level-load line says why), so it declines the view and "
			     "the WMSH path draws the world\n" );
		return false;
	}
	// Only the back buffer is a slot target. The view's fog (range or height)
	// is a frame term, captured when the slot is marked.
	VMatrix view, projection;
	int viewport[4];
	return R_CurrentSceneView( view, projection, viewport );
}

bool RenderCoreWorldDraw_Takes( SurfaceHandle_t surfID )
{
	const CoreWorldState &state = State();
	const int index = MSurf_Index( surfID );
	return state.loaded && index >= 0 && index < state.takes.Count() && state.takes[index];
}

bool RenderCoreWorldDraw_OnlyCore()
{
	return r_core_world.GetInt() == 1 && State().loaded && RenderCoreHost_World();
}

bool RenderCoreWorldDraw_StageOwnsRuntimeLighting()
{
	return RenderCoreWorldDraw_OnlyCore() && State().stageWorld;
}

bool RenderCoreWorldDraw_OwnsLighting( SurfaceHandle_t surfID )
{
	return r_core_world.GetBool() && RenderCoreWorldDraw_Takes( surfID );
}

// Queues the core's surfaces (its entries) for the current view, with the
// view's transform and viewport, and marks its slot here in the stream.
static bool QueueCoreView( IRenderCoreWorld *pWorld, const unsigned int *pEntries, int nCount,
    float waterZOffset, const RenderCoreStaticPropDraw *pStaticProps = NULL, int nStaticProps = 0,
    const RenderCorePosedModel *pPosedModels = NULL, int nPosedModels = 0 )
{
	// The engine's view as pushed: the legacy context holds the same
	// matrices and viewport, and the core reads no legacy stream.
	VMatrix view, projection;
	int rect[4];
	if ( !R_CurrentSceneView( view, projection, rect ) )
		return false;
	const VMatrix worldToClip = projection * view;
	float toClip[16];
	for ( int r = 0; r < 4; ++r )
		for ( int c = 0; c < 4; ++c )
			toClip[r * 4 + c] = worldToClip.m[r][c];
	const float viewport[6] = {
	    float( rect[0] ), float( rect[1] ), float( rect[2] ), float( rect[3] ), 0.0f, 1.0f };
	// The view and projection alone too: a world stage clusters the frame's
	// lights for the view with them.
	float toView[16], projectionRows[16];
	for ( int r = 0; r < 4; ++r )
	{
		for ( int c = 0; c < 4; ++c )
		{
			toView[r * 4 + c] = view.m[r][c];
			projectionRows[r * 4 + c] = projection.m[r][c];
		}
	}
	return pWorld->DrawView( pEntries, unsigned( nCount ), toClip, viewport,
	    static_cast<unsigned long long>( host_framecount ) + 1, toView, projectionRows,
	    waterZOffset, pStaticProps, unsigned( nStaticProps ), pPosedModels,
	    unsigned( nPosedModels ) );
}

bool RenderCoreWorldDraw_CanTakePosedModel( const model_t *model )
{
	const CoreWorldState &state = State();
	return model && state.stageWorld && r_core_world.GetInt() == 1 &&
	       RenderCoreWorld_ViewDepth() >= 1 && RenderCoreHost_World() &&
	       std::find( state.registeredModels.begin(), state.registeredModels.end(), model ) !=
	           state.registeredModels.end();
}

bool RenderCoreWorldDraw_TakePosedModel( const model_t *model, int skin,
    const matrix3x4_t *boneToWorld, int boneCount, RenderCoreDrawPhase phase, int body, int lod,
    unsigned long long motionIdentity )
{
	CoreWorldState &state = State();
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( !RenderCoreWorldDraw_CanTakePosedModel( model ) || !pWorld || skin < 0 || !boneToWorld ||
	     boneCount <= 0 || lod < 0 )
		return false;
	const auto found =
	    std::find( state.registeredModels.begin(), state.registeredModels.end(), model );
	std::vector<float> palette;
	palette.reserve( std::size_t( boneCount ) * 12 );
	for ( int bone = 0; bone < boneCount; ++bone )
		for ( int row = 0; row < 3; ++row )
			for ( int col = 0; col < 4; ++col )
				palette.push_back( boneToWorld[bone].m_flMatVal[row][col] );
	RenderCorePosedModel posed = { unsigned( found - state.registeredModels.begin() ),
	    unsigned( skin ), palette.data(), unsigned( boneCount ), phase, body, unsigned( lod ),
	    motionIdentity };
	if ( !QueueCoreView( pWorld, NULL, 0, 0.0f, NULL, 0, &posed, 1 ) )
		return false;
	++state.posedClaims[posed.model];
	return true;
}

void RenderCoreWorldDraw_BeginView( const unsigned int *pSurfaces, int nCount, float waterZOffset )
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
	// The view names surface indices; the core's world holds only the
	// eligible surfaces, as entries (a displacement, a sky or water face has
	// none, and a taken surface always has one).
	CUtlVector<unsigned int> entries;
	entries.EnsureCapacity( nCount );
	for ( int i = 0; i < nCount; ++i )
	{
		const unsigned int index = pSurfaces[i];
		if ( index < unsigned( state.entryOf.Count() ) && state.entryOf[index] >= 0 )
			entries.AddToTail( unsigned( state.entryOf[index] ) );
	}
	if ( entries.Count() == 0 )
		return;
	state.viewActive = QueueCoreView( pWorld, entries.Base(), entries.Count(), waterZOffset );
}

void RenderCoreWorldDraw_BeginStageView()
{
	CoreWorldState &state = State();
	state.viewActive = false;
	state.stageView = false;
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( !pWorld || !state.stageWorld )
		return;
	CheckFailures( pWorld );
	state.stageView = true;
	// r_core_runtime_direct changes between frames (the stage keeps both
	// lightmap layers): the surfaces the core lights itself change with it.
	static ConVarRef r_core_runtime_direct( "r_core_runtime_direct" );
	if ( r_core_runtime_direct.GetInt() != state.occlusionRuntimeDirect )
		BuildDirectOcclusion();
}

bool RenderCoreWorldDraw_StageView()
{
	return State().stageView;
}

bool RenderCoreWorldDraw_StageTakesBatch( unsigned int batch )
{
	const CoreWorldState &state = State();
	return state.stageView && batch < unsigned( state.stageTakes.Count() ) &&
	       state.stageTakes[batch];
}

void RenderCoreWorldDraw_DrawStageView( const unsigned int *pMeshlets, int nCount )
{
	CoreWorldState &state = State();
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( !pWorld || !state.stageView || nCount <= 0 )
		return;
	if ( r_core_world.GetInt() == 3 )
		return; // negative control: taken and not drawn
	state.viewActive = QueueCoreView( pWorld, pMeshlets, nCount, 0.0f );
}

void RenderCoreWorldDraw_EndView()
{
	State().viewActive = false;
	State().stageView = false;
}

bool RenderCoreWorldDraw_Skips( SurfaceHandle_t surfID )
{
	const CoreWorldState &state = State();
	if ( state.loaded && r_core_world_isolate.GetBool() && !RenderCoreWorldDraw_Takes( surfID ) )
		return true; // the oracle's isolation: only the taken surfaces draw
	return state.viewActive && RenderCoreWorldDraw_Takes( surfID );
}

bool RenderCoreWorldDraw_ChainsOnly()
{
	const CoreWorldState &state = State();
	// A world stage's view goes through the WMSH path (RenderCoreWorldDraw_StageView).
	return !state.stageWorld &&
	       ( state.viewActive || ( state.loaded && r_core_world_isolate.GetBool() ) );
}

bool RenderCoreWorldDraw_TakeStaticProps(
    const unsigned int *props, const unsigned int *lods, int count )
{
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( !pWorld || !State().stageWorld || r_core_world.GetInt() != 1 || !props || !lods ||
	     count <= 0 )
		return false;
	std::vector<RenderCoreStaticPropDraw> draws;
	draws.reserve( count );
	for ( int i = 0; i < count; ++i )
	{
		if ( !pWorld->DrawsStaticProp( props[i], lods[i] ) )
			return false;
		draws.push_back( { props[i], lods[i] } );
	}
	return QueueCoreView( pWorld, NULL, 0, 0.0f, draws.data(), count );
}

bool RenderCoreWorldDraw_DrawsStaticProp( unsigned int prop, unsigned int lod )
{
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	return pWorld && State().stageWorld && r_core_world.GetInt() == 1 &&
	       pWorld->DrawsStaticProp( prop, lod );
}

namespace
{

// Msg has a bounded formatting buffer. Emit each census entry separately so
// a scene's named refusals survive even when the complete census exceeds it.
void PrintCoreCensus( const char *census )
{
	for ( const char *line = census; *line; )
	{
		const char *end = strchr( line, '\n' );
		const int length = end ? int( end - line ) : V_strlen( line );
		Msg( "%.*s\n", length, line );
		line = end ? end + 1 : line + length;
	}
}

} // namespace

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
	     "drawn %llu failed %llu skipped %llu surfaces drawn %llu static queued %llu drawn %llu "
	     "last failure '%s'\n",
	    stats.materials, stats.claimedMaterials, stats.surfaces, stats.claimedSurfaces,
	    stats.viewsQueued, stats.viewsDrawn, stats.viewsFailed, stats.viewsSkipped,
	    stats.surfacesDrawn, stats.staticInstancesQueued, stats.staticDrawsDrawn,
	    stats.lastFailure );
	Msg( "r_core_world_stats: posed models queued %llu draws %llu\n", stats.posedModelsQueued,
	    stats.posedDrawsDrawn );
	Msg( "r_core_world_stats: gpu submission views %llu indirect draws %llu fallbacks %llu "
	     "occlusion views %llu pyramids %llu prepass views %llu model views %llu\n",
	    stats.gpuViews, stats.gpuIndirectDraws, stats.gpuFallbacks, stats.gpuOcclusionViews,
	    stats.gpuPyramids, stats.gpuPrepassViews, stats.gpuModelViews );
	Msg( "r_core_world_stats: screen-pass prepass lists: %llu build(s), %llu reuse(s), %llu "
	     "rebuild(s) after an unready material or group\n",
	    stats.prepassListBuilds, stats.prepassListReuses, stats.prepassListRetries );
	Msg( "r_core_world_stats: dynamic draws %llu refused %llu last refusal '%s'\n",
	    stats.dynamicDrawsDrawn, stats.dynamicDrawsRefused, stats.lastRefusal );
	for ( std::size_t i = 0; i < State().posedClaims.size(); ++i )
	{
		if ( State().posedClaims[i] && State().registeredModels[i] )
			Msg( "r_core_world_stats: posed %llu %s\n", State().posedClaims[i],
			    modelloader->GetName( const_cast<model_t *>( State().registeredModels[i] ) ) );
	}
	if ( stats.gaps[0] )
	{
		Msg( "r_core_world_stats: gaps:\n" );
		PrintCoreCensus( stats.gaps );
	}
	if ( stats.claimed[0] )
	{
		Msg( "r_core_world_stats: drawn by the core:\n" );
		PrintCoreCensus( stats.claimed );
	}
	if ( State().stageWorld )
		Msg( "r_core_world_stats: world stage: %u runtime lights, %llu lit views, %llu lighting "
		     "builds, %llu view(s) sharing the frame's lighting\n",
		    stats.stageLights, stats.stageLitViews, stats.stageLightingBuilds,
		    stats.stageSharedViews );
	if ( State().stageWorld )
		Msg( "r_core_world_stats: medium %u (%llu views, %llu refused), %u projected lights "
		     "(%llu refused), cutout shadow draws %llu (%llu refused, %llu not resident)\n",
		    stats.volumetricMedium, stats.volumetricViews, stats.volumetricRefused,
		    stats.projectorsLit, stats.projectorsRefused, stats.cutoutShadowDraws,
		    stats.cutoutShadowRefused, stats.cutoutShadowNotResident );
	if ( State().stageWorld )
		Msg( "r_core_world_stats: screen-space reflections over %llu views (%llu refused)\n",
		    stats.ssrViews, stats.ssrRefused );
	if ( State().worldMeshViews )
		Msg( "r_core_world_stats: declined %llu view(s): the map's world is its WMSH\n",
		    State().worldMeshViews );
}

// RFC 0014: what the core claims, per program and material, and each named gap
// (the one report of claims; VK_DEBUG_LIGHTMAPPED's diagnostics are gone).
CON_COMMAND( cl_render_debug_claims,
    "RFC 0014: what the render core claims, per program and material, and why the rest is not" )
{
	IRenderCoreWorld *pWorld = RenderCoreHost_World();
	if ( !pWorld )
	{
		Msg( "cl_render_debug_claims: no render core\n" );
		return;
	}
	RenderCoreWorldStats stats;
	pWorld->GetStats( &stats );
	Msg( "cl_render_debug_claims: world: %u of %u materials, %u of %u surfaces claimed\n",
	    stats.claimedMaterials, stats.materials, stats.claimedSurfaces, stats.surfaces );
	Msg( "cl_render_debug_claims: debug slots: %llu hatched frames, %llu tinted frames, %llu "
	     "views drawn again over the tint\n",
	    stats.debugHatches, stats.debugTints, stats.debugViewsRedrawn );
	// "surfaces program material" lines, grouped by program.
	CUtlVector<CUtlString> programs;
	for ( const char *line = stats.claimed; *line; )
	{
		const char *end = strchr( line, '\n' );
		const int length = end ? int( end - line ) : V_strlen( line );
		char entry[512];
		V_strncpy( entry, line, MIN( length + 1, (int)sizeof( entry ) ) );
		char program[64] = {};
		unsigned int surfaces = 0;
		if ( sscanf( entry, "%u %63s", &surfaces, program ) == 2 )
		{
			bool known = false;
			for ( int i = 0; i < programs.Count(); ++i )
				known = known || V_strcmp( programs[i].Get(), program ) == 0;
			if ( !known )
				programs.AddToTail( CUtlString( program ) );
		}
		line = end ? end + 1 : line + length;
	}
	for ( int i = 0; i < programs.Count(); ++i )
	{
		Msg( "cl_render_debug_claims: program %s:\n", programs[i].Get() );
		for ( const char *line = stats.claimed; *line; )
		{
			const char *end = strchr( line, '\n' );
			const int length = end ? int( end - line ) : V_strlen( line );
			char entry[512];
			V_strncpy( entry, line, MIN( length + 1, (int)sizeof( entry ) ) );
			char program[64] = {};
			unsigned int surfaces = 0;
			int name = 0;
			if ( sscanf( entry, "%u %63s %n", &surfaces, program, &name ) == 2 &&
			     V_strcmp( program, programs[i].Get() ) == 0 )
				Msg( "  %u surfaces  %s\n", surfaces, entry + name );
			line = end ? end + 1 : line + length;
		}
	}
	if ( stats.gaps[0] )
	{
		Msg( "cl_render_debug_claims: not claimed (count, material, reason):\n" );
		PrintCoreCensus( stats.gaps );
	}
}
