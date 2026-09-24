//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Canonical PBRMetalRough material binding for native Vulkan worlds.
//          The shader API owns the WMSH PBR pipeline; this material owns its
//          Source parameters, texture loading and snapshot sampler contract.
//
//===========================================================================//

#include "BaseVSShader.h"

BEGIN_VS_SHADER( PBRMetalRough, "Metalness/roughness world material" )
BEGIN_SHADER_PARAMS
SHADER_PARAM( MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Linear metal/rough/AO map" )
SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Linear tangent-space normal map" )
SHADER_PARAM( EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emissive color map" )
SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_ENVMAP, "", "Reflection probe" )
SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.7", "Alpha cutoff" )
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
}

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT
{
	if ( !params[BASETEXTURE]->IsDefined() || !params[MRAOTEXTURE]->IsDefined() )
		Warning(
		    "PBRMetalRough material %s requires $basetexture and $mraotexture\n", pMaterialName );
	if ( params[EMISSIONTEXTURE]->IsDefined() || params[ENVMAP]->IsDefined() ||
	     IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) )
		Warning( "PBRMetalRough material %s requests an unsupported native PBR feature\n",
		    pMaterialName );
	if ( params[BASETEXTURE]->IsDefined() )
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
	if ( params[MRAOTEXTURE]->IsDefined() )
		LoadTexture( MRAOTEXTURE, 0 );
	if ( params[BUMPMAP]->IsDefined() )
		LoadBumpMap( BUMPMAP );
}

SHADER_DRAW
{
	// Missing required textures cannot borrow a previous material's GPU
	// binding. The shader API also validates formats before it queues a draw.
	const bool supported = params[BASETEXTURE]->IsTexture() && params[MRAOTEXTURE]->IsTexture() &&
	                       !params[EMISSIONTEXTURE]->IsDefined() && !params[ENVMAP]->IsDefined() &&
	                       !IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT );
	SHADOW_STATE
	{
		SetDefaultBlendingShadowState( BASETEXTURE, true );
		const bool alphaTest = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST );
		pShaderShadow->EnableAlphaTest( alphaTest );
		if ( alphaTest )
			pShaderShadow->AlphaFunc(
			    SHADER_ALPHAFUNC_GEQUAL, params[ALPHATESTREFERENCE]->GetFloatValue() );
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER10, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER10, false );
		if ( params[BUMPMAP]->IsTexture() )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false );
		}
		pShaderShadow->EnableSRGBWrite( true );
		pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_NORMAL, 3, 0, 0 );
		pShaderShadow->SetVertexShader( "pbr_metalrough_world_vs", 0 );
		pShaderShadow->SetPixelShader( "pbr_metalrough_world_ps", 0 );
	}
	// Source needs a snapshot even for rejected materials; without Draw() in
	// that phase it reports "No render states in shader" and loses the map.
	if ( !supported && !pShaderShadow )
		return;
	DYNAMIC_STATE
	{
		BindTexture( SHADER_SAMPLER0, BASETEXTURE, FRAME );
		BindTexture( SHADER_SAMPLER10, MRAOTEXTURE, 0 );
		if ( params[BUMPMAP]->IsTexture() )
			BindTexture( SHADER_SAMPLER1, BUMPMAP, 0 );
	}
	Draw();
}
END_SHADER
