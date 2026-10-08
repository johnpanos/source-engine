//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal 2 paint blob surface. Ported from the CS:GO-era helper to
//          this tree's shader API: the per-instance command buffer (ambient
//          cube and local lights) becomes direct dynamic-state calls, and the
//          texture bind flags become the samplers' shadow-state sRGB reads.
//
//          c27 carries the combos (paintblob_ps20b does not read it), so a
//          backend that reimplements the shader from its constants sees them:
//          c27 = ( static flags, NUM_LIGHTS, FLASHLIGHT, 0 )
//
//==========================================================================//

#include "BaseVSShader.h"
#include "paintblob_helper.h"
#include "cpp_shader_constant_register_map.h"

// Auto generated inc files

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Bits of c27.x (see the file comment).
enum PaintBlobStaticFlags_t
{
	PAINTBLOB_BACK_SURFACE = 1,
	PAINTBLOB_LIGHT_WARP = 2,
	PAINTBLOB_FRESNEL_WARP = 4,
	PAINTBLOB_OPACITY_TEXTURE = 8,
	PAINTBLOB_INTERIOR_LAYER = 16,
	PAINTBLOB_CONTACT_SHADOW = 32,
	PAINTBLOB_SPEC_MASK = 64,
	PAINTBLOB_ENVMAP = 128,
};

static const int PAINTBLOB_PSREG_COMBOS = 27;

void InitParamsPaintBlob( CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName,
    PaintBlobVars_t &info )
{
	// Set material parameter default values
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nBackSurface, kDefaultBackSurface );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nUVScale, kDefaultUVScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nBumpStrength, kDefaultBumpStrength );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFresnelBumpStrength, kDefaultFresnelBumpStrength );

	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nInteriorEnable, kDefaultInteriorEnable );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nInteriorFogStrength, kDefaultInteriorFogStrength );
	SET_PARAM_FLOAT_IF_NOT_DEFINED(
	    info.m_nInteriorBackgroundBoost, kDefaultInteriorBackgroundBoost );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nInteriorAmbientScale, kDefaultInteriorAmbientScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED(
	    info.m_nInteriorBackLightScale, kDefaultInteriorBackLightScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED(
	    info.m_nInteriorRefractStrength, kDefaultInteriorRefractStrength );

	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nFresnelParams, kDefaultFresnelParams, 3 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nBaseColorTint, kDefaultBaseColorTint, 3 );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nDiffuseScale, kDefaultDiffuseScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nSpecExp, kDefaultSpecExp );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nSpecScale, kDefaultSpecScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nSpecExp2, kDefaultSpecExp );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nSpecScale2, kDefaultSpecScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nRimLightExp, kDefaultRimLightExp );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nRimLightScale, kDefaultRimLightScale );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nUVProjOffset, kDefaultUVProjOffset, 3 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nBBMin, kDefaultBB, 3 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nBBMax, kDefaultBB, 3 );

	Assert( info.m_nFlashlightTexture >= 0 );
	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[info.m_nFlashlightTexture]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[info.m_nFlashlightTexture]->SetStringValue( "effects/flashlight001" );
	}
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nFlashlightTextureFrame, 0 );

	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nBumpFrame, kDefaultBumpFrame );

	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nContactShadows, kDefaultContactShadows );

	// Set material flags
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	if ( params[info.m_nInteriorEnable]->IsDefined() &&
	     params[info.m_nInteriorEnable]->GetIntValue() != 0 )
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );
	}
}

void InitPaintBlob( CBaseVSShader *pShader, IMaterialVar **params, PaintBlobVars_t &info )
{
	// Load textures
	if ( ( info.m_nBaseTexture != -1 ) && params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
	}

	if ( ( info.m_nNormalMap != -1 ) && params[info.m_nNormalMap]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nNormalMap );
	}

	if ( ( info.m_nSpecMap != -1 ) && params[info.m_nSpecMap]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nSpecMap );
	}

	if ( ( info.m_nLightWarpTexture != -1 ) && params[info.m_nLightWarpTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nLightWarpTexture, TEXTUREFLAGS_SRGB );
	}

	if ( ( info.m_nFresnelWarpTexture != -1 ) && params[info.m_nFresnelWarpTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nFresnelWarpTexture );
	}

	if ( ( info.m_nOpacityTexture != -1 ) && params[info.m_nOpacityTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nOpacityTexture );
	}

	if ( ( info.m_nEnvMap != -1 ) && params[info.m_nEnvMap]->IsDefined() )
	{
		pShader->LoadCubeMap( info.m_nEnvMap, TEXTUREFLAGS_SRGB );
	}

	if ( ( info.m_nFlashlightTexture != -1 ) && params[info.m_nFlashlightTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB );
	}
}

void DrawPaintBlob( CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI,
    IShaderShadow *pShaderShadow, PaintBlobVars_t &info, VertexCompressionType_t vertexCompression )
{
	bool bHasFlashlight = pShader->UsingFlashlight( params );
	bool bBackSurface =
	    ( info.m_nBackSurface != -1 ) && ( params[info.m_nBackSurface]->GetIntValue() > 0 );
	bool bLightWarp =
	    ( info.m_nLightWarpTexture != -1 ) && params[info.m_nLightWarpTexture]->IsDefined();
	bool bFresnelWarp =
	    ( info.m_nFresnelWarpTexture != -1 ) && params[info.m_nFresnelWarpTexture]->IsDefined();
	bool bOpacityTexture =
	    ( info.m_nOpacityTexture != -1 ) && params[info.m_nOpacityTexture]->IsDefined();
	bool bInteriorLayer =
	    ( info.m_nInteriorEnable != -1 ) && ( params[info.m_nInteriorEnable]->GetIntValue() > 0 );
	bool bContactShadows =
	    ( info.m_nContactShadows != -1 ) && ( params[info.m_nContactShadows]->GetIntValue() > 0 );
	bool bSpecMap = ( info.m_nSpecMap != -1 ) && params[info.m_nSpecMap]->IsDefined();
	bool bEnvMap = ( info.m_nEnvMap != -1 ) && params[info.m_nEnvMap]->IsDefined();
	bool bFlattenStaticControlFlow = !g_pHardwareConfig->SupportsStaticControlFlow();

	SHADOW_STATE
	{
		// Position, normal and one 4D texcoord (the blobulator's closest-surface
		// direction, read by CONTACT_SHADOW). Texture coordinates are projected
		// from world space in the vertex shader.
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		int texCoordDims[4] = { 4, 4, 4, 4 };
		pShaderShadow->VertexShaderVertexFormat(
		    flags, nTexCoordCount, texCoordDims, userDataSize );

		int nShadowFilterMode = 0;
		if ( bHasFlashlight )
		{
			nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();
		}

		// Vertex Shader

		// Pixel Shader

		// Textures
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); //[sRGB] Base
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true ); //		 Bump
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false );
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true ); //[sRGB] Backbuffer
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true ); //       Spec mask
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, false );
		pShaderShadow->EnableTexture( SHADER_SAMPLER4, true ); //[sRGB] Light warp
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true ); //		 Fresnel warp
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER5, false );
		pShaderShadow->EnableTexture( SHADER_SAMPLER6, true ); //		 Opacity
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER6, false );
		pShaderShadow->EnableTexture( SHADER_SAMPLER7, true ); //[sRGB] Envmap
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER7, true );

		if ( bHasFlashlight )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER8, true ); //		 Shadow depth map
			pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER8 );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER8, false );
			pShaderShadow->EnableTexture( SHADER_SAMPLER9, true ); //		 Noise map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER9, false );
			pShaderShadow->EnableTexture( SHADER_SAMPLER10, true ); //[sRGB] Flashlight cookie
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER10, true );
		}

		pShaderShadow->EnableSRGBWrite( true );
		pShaderShadow->EnableAlphaWrites( true );
	}


	pShader->Draw();
}
