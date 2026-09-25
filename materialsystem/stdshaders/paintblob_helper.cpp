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
#include "paintblob_vs20.inc"
#include "paintblob_ps20b.inc"

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

void InitParamsPaintBlob(
    CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, PaintBlobVars_t &info )
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
		DECLARE_STATIC_VERTEX_SHADER( paintblob_vs20 );
		SET_STATIC_VERTEX_SHADER_COMBO( FLATTEN_STATIC_CONTROL_FLOW, bFlattenStaticControlFlow );
		SET_STATIC_VERTEX_SHADER( paintblob_vs20 );

		// Pixel Shader
		DECLARE_STATIC_PIXEL_SHADER( paintblob_ps20b );
		SET_STATIC_PIXEL_SHADER_COMBO( BACK_SURFACE, bBackSurface );
		SET_STATIC_PIXEL_SHADER_COMBO( LIGHT_WARP, bLightWarp );
		SET_STATIC_PIXEL_SHADER_COMBO( FRESNEL_WARP, bFresnelWarp );
		SET_STATIC_PIXEL_SHADER_COMBO( OPACITY_TEXTURE, bOpacityTexture );
		SET_STATIC_PIXEL_SHADER_COMBO( INTERIOR_LAYER, bInteriorLayer );
		SET_STATIC_PIXEL_SHADER_COMBO(
		    HIGH_PRECISION_DEPTH, ( g_pHardwareConfig->GetHDRType() == HDR_TYPE_FLOAT ) );
		SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
		SET_STATIC_PIXEL_SHADER_COMBO( CONTACT_SHADOW, bContactShadows );
		SET_STATIC_PIXEL_SHADER( paintblob_ps20b );

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

	DYNAMIC_STATE
	{
		///////////////////////
		// VERTEX SHADER SETUP
		///////////////////////

		LightState_t lightState = { 0, false, false };
		pShaderAPI->GetDX9LightState( &lightState );

		// Set Vertex Shader Combos
		DECLARE_DYNAMIC_VERTEX_SHADER( paintblob_vs20 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
		SET_DYNAMIC_VERTEX_SHADER_COMBO(
		    NUM_LIGHTS, bFlattenStaticControlFlow ? lightState.m_nNumLights : 0 );
		SET_DYNAMIC_VERTEX_SHADER( paintblob_vs20 );

		// Ambient cube (vertex shader) and local lights (pixel shader); the
		// CS:GO helper recorded these in its per-instance command buffer.
		pShader->SetAmbientCubeDynamicStateVertexShader();
		pShaderAPI->SetPixelShaderStateAmbientLightCube( PSREG_AMBIENT_CUBE );
		pShaderAPI->CommitPixelShaderLighting( PSREG_LIGHT_INFO_ARRAY );

		// VS constants
		float flConsts[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		flConsts[0] = IS_PARAM_DEFINED( info.m_nUVScale ) ? params[info.m_nUVScale]->GetFloatValue()
		                                                  : kDefaultUVScale;
		pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, flConsts );

		if ( IS_PARAM_DEFINED( info.m_nUVProjOffset ) )
			params[info.m_nUVProjOffset]->GetVecValue( flConsts, 3 );
		else
			memcpy( flConsts, kDefaultUVProjOffset, sizeof( kDefaultUVProjOffset ) );
		pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, flConsts );

		if ( IS_PARAM_DEFINED( info.m_nBBMin ) )
			params[info.m_nBBMin]->GetVecValue( flConsts, 3 );
		else
			memcpy( flConsts, kDefaultBB, sizeof( kDefaultBB ) );
		pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, flConsts );

		if ( IS_PARAM_DEFINED( info.m_nBBMax ) )
			params[info.m_nBBMax]->GetVecValue( flConsts, 3 );
		else
			memcpy( flConsts, kDefaultBB, sizeof( kDefaultBB ) );
		pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_3, flConsts );

		//////////////////////
		// PIXEL SHADER SETUP
		//////////////////////

		// Bind textures
		pShader->BindTexture( SHADER_SAMPLER0, BASETEXTURE );
		pShader->BindTexture( SHADER_SAMPLER1, info.m_nNormalMap, info.m_nBumpFrame );
		pShaderAPI->BindStandardTexture(
		    SHADER_SAMPLER2, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0 ); // Refraction Map

		if ( bSpecMap )
		{
			pShader->BindTexture( SHADER_SAMPLER3, info.m_nSpecMap );
		}
		else
		{
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER3, TEXTURE_WHITE );
		}

		if ( bLightWarp )
		{
			pShader->BindTexture( SHADER_SAMPLER4, info.m_nLightWarpTexture );
		}

		if ( bFresnelWarp )
		{
			pShader->BindTexture( SHADER_SAMPLER5, info.m_nFresnelWarpTexture );
		}

		if ( bOpacityTexture )
		{
			pShader->BindTexture( SHADER_SAMPLER6, info.m_nOpacityTexture );
		}

		if ( bEnvMap )
		{
			pShader->BindTexture( SHADER_SAMPLER7, info.m_nEnvMap );
		}
		else
		{
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER7, TEXTURE_BLACK );
		}

		bool bFlashlightShadows = false;
		if ( bHasFlashlight )
		{
			Assert( info.m_nFlashlightTexture >= 0 && info.m_nFlashlightTextureFrame >= 0 );
			pShader->BindTexture(
			    SHADER_SAMPLER10, info.m_nFlashlightTexture, info.m_nFlashlightTextureFrame );
			VMatrix worldToTexture;
			ITexture *pFlashlightDepthTexture;
			FlashlightState_t state =
			    pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );
			bFlashlightShadows = state.m_bEnableShadows;

			SetFlashLightColorFromState( state, pShaderAPI, PSREG_FLASHLIGHT_COLOR );

			if ( pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() &&
			     state.m_bEnableShadows )
			{
				pShader->BindTexture( SHADER_SAMPLER8, pFlashlightDepthTexture );
				pShaderAPI->BindStandardTexture( SHADER_SAMPLER9, TEXTURE_SHADOW_NOISE_2D );
			}

			float atten[4], pos[4], tweaks[4];

			atten[0] = state.m_fConstantAtten; // Set the flashlight attenuation factors
			atten[1] = state.m_fLinearAtten;
			atten[2] = state.m_fQuadraticAtten;
			atten[3] = state.m_FarZ;
			pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_ATTENUATION, atten, 1 );

			pos[0] = state.m_vecLightOrigin[0]; // Set the flashlight origin
			pos[1] = state.m_vecLightOrigin[1];
			pos[2] = state.m_vecLightOrigin[2];
			pos[3] = state.m_FarZ;
			pShaderAPI->SetPixelShaderConstant(
			    PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1 ); // steps on rim boost

			pShaderAPI->SetPixelShaderConstant(
			    PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4 );

			// Tweaks associated with a given flashlight
			tweaks[0] = ShadowFilterFromState( state );
			tweaks[1] = ShadowAttenFromState( state );
			pShader->HashShadow2DJitter( state.m_flShadowJitterSeed, &tweaks[2], &tweaks[3] );
			pShaderAPI->SetPixelShaderConstant( PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1 );

			// Dimensions of screen, used for screen-space noise map sampling
			float vScreenScale[4] = { 1280.0f / 32.0f, 720.0f / 32.0f, 0, 0 };
			int nWidth, nHeight;
			pShaderAPI->GetBackBufferDimensions( nWidth, nHeight );

			int nTexWidth, nTexHeight;
			pShaderAPI->GetStandardTextureDimensions(
			    &nTexWidth, &nTexHeight, TEXTURE_SHADOW_NOISE_2D );

			vScreenScale[0] = (float)nWidth / nTexWidth;
			vScreenScale[1] = (float)nHeight / nTexHeight;
			vScreenScale[2] = 1.0f / state.m_flShadowMapResolution;
			vScreenScale[3] = 2.0f / state.m_flShadowMapResolution;

			pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_SCREEN_SCALE, vScreenScale, 1 );
		}

		flConsts[0] = IS_PARAM_DEFINED( info.m_nBumpStrength )
		                  ? params[info.m_nBumpStrength]->GetFloatValue()
		                  : kDefaultBumpStrength;
		// destalpha depth scale factor
		flConsts[1] = ( g_pHardwareConfig->GetHDRType() == HDR_TYPE_FLOAT ) ? 8192.0f : 192.0f;
		flConsts[2] = IS_PARAM_DEFINED( info.m_nInteriorFogStrength )
		                  ? params[info.m_nInteriorFogStrength]->GetFloatValue()
		                  : kDefaultInteriorFogStrength;
		flConsts[3] = IS_PARAM_DEFINED( info.m_nInteriorRefractStrength )
		                  ? params[info.m_nInteriorRefractStrength]->GetFloatValue()
		                  : kDefaultInteriorRefractStrength;
		pShaderAPI->SetPixelShaderConstant( 0, flConsts, 1 );

		Assert( IS_PARAM_DEFINED( info.m_nFresnelParams ) );
		if ( IS_PARAM_DEFINED( info.m_nFresnelParams ) )
			params[info.m_nFresnelParams]->GetVecValue( flConsts, 3 );
		else
			memcpy( flConsts, kDefaultFresnelParams, sizeof( kDefaultFresnelParams ) );
		flConsts[3] = params[info.m_nInteriorBackgroundBoost]->GetFloatValue();
		pShaderAPI->SetPixelShaderConstant( 1, flConsts, 1 );

		flConsts[0] = IS_PARAM_DEFINED( info.m_nRimLightExp )
		                  ? params[info.m_nRimLightExp]->GetFloatValue()
		                  : kDefaultRimLightExp;
		flConsts[1] = IS_PARAM_DEFINED( info.m_nRimLightScale )
		                  ? params[info.m_nRimLightScale]->GetFloatValue()
		                  : kDefaultRimLightScale;
		flConsts[2] = IS_PARAM_DEFINED( info.m_nSpecScale )
		                  ? params[info.m_nSpecScale]->GetFloatValue()
		                  : kDefaultSpecScale;
		flConsts[3] = IS_PARAM_DEFINED( info.m_nSpecExp2 ) ? params[info.m_nSpecExp2]->GetFloatValue()
		                                                   : kDefaultSpecExp;
		pShaderAPI->SetPixelShaderConstant( 3, flConsts, 1 );

		flConsts[0] = IS_PARAM_DEFINED( info.m_nSpecScale2 )
		                  ? params[info.m_nSpecScale2]->GetFloatValue()
		                  : kDefaultSpecScale;
		flConsts[1] = IS_PARAM_DEFINED( info.m_nFresnelBumpStrength )
		                  ? params[info.m_nFresnelBumpStrength]->GetFloatValue()
		                  : kDefaultFresnelBumpStrength;
		flConsts[2] = IS_PARAM_DEFINED( info.m_nDiffuseScale )
		                  ? params[info.m_nDiffuseScale]->GetFloatValue()
		                  : kDefaultDiffuseScale;
		flConsts[3] = IS_PARAM_DEFINED( info.m_nInteriorAmbientScale )
		                  ? params[info.m_nInteriorAmbientScale]->GetFloatValue()
		                  : kDefaultInteriorAmbientScale;
		pShaderAPI->SetPixelShaderConstant( 10, flConsts, 1 );

		pShaderAPI->GetWorldSpaceCameraPosition( flConsts );
		flConsts[3] = IS_PARAM_DEFINED( info.m_nSpecExp ) ? params[info.m_nSpecExp]->GetFloatValue()
		                                                  : kDefaultSpecExp;
		pShaderAPI->SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, flConsts, 1 );

		// Depth alpha [ TODO: support fog ]
		bool bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
		flConsts[0] = bWriteDepthToAlpha ? 1.0f : 0.0f;
		flConsts[1] = flConsts[2] = flConsts[3] = 0.0f;
		pShaderAPI->SetPixelShaderConstant( PSREG_FOG_PARAMS, flConsts, 1 );

		if ( IS_PARAM_DEFINED( info.m_nBaseColorTint ) )
			params[info.m_nBaseColorTint]->GetVecValue( flConsts, 3 );
		else
			memcpy( flConsts, kDefaultBaseColorTint, sizeof( kDefaultBaseColorTint ) );
		flConsts[3] = IS_PARAM_DEFINED( info.m_nInteriorBackLightScale )
		                  ? params[info.m_nInteriorBackLightScale]->GetFloatValue()
		                  : kDefaultInteriorBackLightScale;
		pShaderAPI->SetPixelShaderConstant( 19, flConsts, 1 );

		// Combos for a backend that reimplements the shader (see the file comment).
		int nStaticFlags = ( bBackSurface ? PAINTBLOB_BACK_SURFACE : 0 ) |
		                   ( bLightWarp ? PAINTBLOB_LIGHT_WARP : 0 ) |
		                   ( bFresnelWarp ? PAINTBLOB_FRESNEL_WARP : 0 ) |
		                   ( bOpacityTexture ? PAINTBLOB_OPACITY_TEXTURE : 0 ) |
		                   ( bInteriorLayer ? PAINTBLOB_INTERIOR_LAYER : 0 ) |
		                   ( bContactShadows ? PAINTBLOB_CONTACT_SHADOW : 0 ) |
		                   ( bSpecMap ? PAINTBLOB_SPEC_MASK : 0 ) | ( bEnvMap ? PAINTBLOB_ENVMAP : 0 );
		float vCombos[4] = { (float)nStaticFlags, (float)lightState.m_nNumLights,
		    bHasFlashlight ? 1.0f : 0.0f, 0.0f };
		pShaderAPI->SetPixelShaderConstant( PAINTBLOB_PSREG_COMBOS, vCombos, 1 );

		// Set Pixel Shader Combos
		DECLARE_DYNAMIC_PIXEL_SHADER( paintblob_ps20b );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHT, bHasFlashlight );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, bFlashlightShadows );
		SET_DYNAMIC_PIXEL_SHADER( paintblob_ps20b );
	}

	pShader->Draw();
}
