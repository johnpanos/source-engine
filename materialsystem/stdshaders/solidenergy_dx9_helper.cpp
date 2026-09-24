//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SolidEnergy (Portal 2 fizzlers, light bridges, tractor beams and
//          laser fields). Ported from the CS:GO-era helper to this tree's shader
//          API: the semi-static command buffer becomes direct calls, and the
//          gpu_level convar (absent here) is treated as a PC's highest level.
//
//          c10 and c11 carry the combos (solidenergy_ps20b reads neither), so a
//          backend that reimplements the shader from its constants sees them:
//          c10 = ( ACTIVE, POWERUP, VORTEX1, VORTEX2 )
//          c11 = ( static flags, DETAIL1BLENDMODE, DETAIL2BLENDMODE, 0 )
//
//==========================================================================//

#include "BaseVSShader.h"
#include "solidenergy_dx9_helper.h"
#include "cpp_shader_constant_register_map.h"

// Auto generated inc files
#include "solidenergy_vs20.inc"
#include "solidenergy_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Bits of c11.x (see the file comment).
enum SolidEnergyStaticFlags_t
{
	SOLIDENERGY_ADDITIVE = 1,
	SOLIDENERGY_DETAIL1 = 2,
	SOLIDENERGY_DETAIL2 = 4,
	SOLIDENERGY_TANGENTTOPACITY = 8,
	SOLIDENERGY_TANGENTSOPACITY = 16,
	SOLIDENERGY_FRESNELOPACITY = 32,
	SOLIDENERGY_VERTEXCOLOR = 64,
	SOLIDENERGY_FLOWMAP = 128,
	SOLIDENERGY_MODELFORMAT = 256,
};

void InitParamsSolidEnergy( CBaseVSShader *pShader, IMaterialVar **params,
    const char *pMaterialName, SolidEnergyVars_t &info )
{
	// Set material parameter default values
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nDetail1Scale, kDefaultDetailScale );
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nDetail1Frame, kDefaultDetailFrame );
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nDetail1BlendMode, kDefaultDetailBlendMode );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nDetail2Scale, kDefaultDetailScale );
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nDetail2Frame, kDefaultDetailFrame );
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nDetail2BlendMode, kDefaultDetailBlendMode );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nDepthBlendScale, kDefaultDepthBlendScale );
	SET_PARAM_INT_IF_NOT_DEFINED(
	    info.m_nNeedsTangentT, IS_PARAM_DEFINED( info.m_nTangentTOpacityRanges ) );
	SET_PARAM_INT_IF_NOT_DEFINED(
	    info.m_nNeedsTangentS, IS_PARAM_DEFINED( info.m_nTangentSOpacityRanges ) );
	SET_PARAM_INT_IF_NOT_DEFINED(
	    info.m_nNeedsNormals, IS_PARAM_DEFINED( info.m_nFresnelOpacityRanges ) );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFlowWorldUVScale, kDefaultDetailScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFlowNormalUVScale, kDefaultDetailScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFlowTimeIntervalInSeconds, kDefaultTimescale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFlowUVScrollDistance, kDefaultScrollDist );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFlowNoiseScale, kDefaultNoiseScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nPowerUp, kDefaultPowerUpIntensity );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFlowColorIntensity, kDefaultIntensity );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nFlowVortexSize, kDefaultVortexSize );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nFlowColor, kDefaultFieldColor, 3 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nFlowVortexColor, kDefaultVortexColor, 3 );
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nFlowCheap, 0 );
	SET_PARAM_INT_IF_NOT_DEFINED( info.m_nModel, 0 );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nOutputIntensity, 1.0f );
}

void InitSolidEnergy( CBaseVSShader *pShader, IMaterialVar **params, SolidEnergyVars_t &info )
{
	// Load textures
	if ( ( info.m_nBaseTexture != -1 ) && params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
	}
	if ( ( info.m_nDetail1Texture != -1 ) && params[info.m_nDetail1Texture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDetail1Texture, TEXTUREFLAGS_SRGB );
	}
	if ( ( info.m_nDetail2Texture != -1 ) && params[info.m_nDetail2Texture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDetail2Texture, TEXTUREFLAGS_SRGB );
	}
	if ( ( info.m_nFlowMap != -1 ) && params[info.m_nFlowMap]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nFlowMap );
		if ( ( info.m_nFlowNoiseTexture != -1 ) && params[info.m_nFlowNoiseTexture]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nFlowNoiseTexture );
		}
		if ( ( info.m_nFlowBoundsTexture != -1 ) && params[info.m_nFlowBoundsTexture]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nFlowBoundsTexture );
		}
	}

	if ( ( info.m_nModel != -1 ) && ( params[info.m_nModel]->GetIntValue() != 0 ) )
	{
		SET_FLAGS( MATERIAL_VAR_MODEL );
	}

	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
}

void DrawSolidEnergy( CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI,
    IShaderShadow *pShaderShadow, SolidEnergyVars_t &info,
    VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr )
{
	bool bAlphaBlend = IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT );
	bool bDetail1 = ( info.m_nDetail1Texture != -1 ) && params[info.m_nDetail1Texture]->IsTexture();
	bool bDetail2 =
	    bDetail1 && ( info.m_nDetail2Texture != -1 ) && params[info.m_nDetail2Texture]->IsTexture();
	bool bHasFlowmap =
	    !bDetail1 && ( info.m_nFlowMap != -1 ) && params[info.m_nFlowMap]->IsTexture();
	bool bHasCheapFlow = bHasFlowmap && ( params[info.m_nFlowCheap]->GetIntValue() != 0 );

	bool bAdditiveBlend = IS_FLAG_SET( MATERIAL_VAR_ADDITIVE );
	bool bHasVertexColor = IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
	bool bHasVertexAlpha = IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA );
	bool bModel = IS_FLAG_SET( MATERIAL_VAR_MODEL );

	bool bTangentT =
	    ( info.m_nNeedsTangentT != -1 ) && params[info.m_nNeedsTangentT]->GetIntValue();
	bool bTangentS =
	    ( info.m_nNeedsTangentS != -1 ) && params[info.m_nNeedsTangentS]->GetIntValue();
	if ( bTangentS && bTangentT ) // If both on, T wins
	{
		bTangentS = false;
	}

	bool bFresnel = !bTangentS && !bTangentT && ( info.m_nNeedsNormals != -1 ) &&
	                params[info.m_nNeedsNormals]->GetIntValue();
	int nDetail1BlendMode = ( info.m_nDetail1BlendMode != -1 )
	                            ? params[info.m_nDetail1BlendMode]->GetIntValue()
	                            : kDefaultDetailBlendMode;
	nDetail1BlendMode = bDetail1 ? clamp( nDetail1BlendMode, 0, kMaxDetailBlendMode ) : 0;
	int nDetail2BlendMode = ( info.m_nDetail2BlendMode != -1 )
	                            ? params[info.m_nDetail2BlendMode]->GetIntValue()
	                            : kDefaultDetailBlendMode;
	nDetail2BlendMode = bDetail2 ? clamp( nDetail2BlendMode, 0, kMaxDetailBlendMode ) : 0;

	SHADOW_STATE
	{
		// Set stream format (note that this shader supports compression)
		int userDataSize = 0;
		unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
		if ( !bModel && ( bTangentS || bTangentT || bHasFlowmap ) )
		{
			flags |= VERTEX_TANGENT_S;
			flags |= VERTEX_TANGENT_T;
		}
		if ( bModel && ( bTangentS || bTangentT || bHasFlowmap ) )
		{
			flags |= VERTEX_USERDATA_SIZE( 4 );
			userDataSize = 4;
		}
		if ( bModel || bFresnel || bTangentS || bTangentT || bHasFlowmap )
		{
			flags |= VERTEX_NORMAL;
		}
		if ( bHasVertexColor || bHasVertexAlpha )
		{
			flags |= VERTEX_COLOR;
		}

		int nTexCoordCount = 1;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		DECLARE_STATIC_VERTEX_SHADER( solidenergy_vs20 );
		SET_STATIC_VERTEX_SHADER_COMBO( VERTEXCOLOR, bHasVertexColor || bHasVertexAlpha );
		SET_STATIC_VERTEX_SHADER_COMBO( DETAIL1, bDetail1 );
		SET_STATIC_VERTEX_SHADER_COMBO( DETAIL2, bDetail2 );
		SET_STATIC_VERTEX_SHADER_COMBO( TANGENTTOPACITY, bTangentT );
		SET_STATIC_VERTEX_SHADER_COMBO( TANGENTSOPACITY, bTangentS );
		SET_STATIC_VERTEX_SHADER_COMBO( FRESNELOPACITY, bFresnel );
		SET_STATIC_VERTEX_SHADER_COMBO( FLOWMAP, bHasFlowmap );
		SET_STATIC_VERTEX_SHADER_COMBO( MODELFORMAT, bModel );
		SET_STATIC_VERTEX_SHADER( solidenergy_vs20 );

		DECLARE_STATIC_PIXEL_SHADER( solidenergy_ps20b );
		SET_STATIC_PIXEL_SHADER_COMBO( ADDITIVE, bAdditiveBlend );
		SET_STATIC_PIXEL_SHADER_COMBO( DETAIL1, bDetail1 );
		SET_STATIC_PIXEL_SHADER_COMBO( DETAIL2, bDetail2 );
		SET_STATIC_PIXEL_SHADER_COMBO( DETAIL1BLENDMODE, nDetail1BlendMode );
		SET_STATIC_PIXEL_SHADER_COMBO( DETAIL2BLENDMODE, nDetail2BlendMode );
		SET_STATIC_PIXEL_SHADER_COMBO( VERTEXCOLOR, ( bHasVertexColor || bHasVertexAlpha ) );
		SET_STATIC_PIXEL_SHADER_COMBO( TANGENTTOPACITY, bTangentT );
		SET_STATIC_PIXEL_SHADER_COMBO( TANGENTSOPACITY, bTangentS );
		SET_STATIC_PIXEL_SHADER_COMBO( FRESNELOPACITY, bFresnel );
		SET_STATIC_PIXEL_SHADER_COMBO( DEPTHBLEND, 0 );
		SET_STATIC_PIXEL_SHADER_COMBO( FLOWMAP, bHasFlowmap );
		SET_STATIC_PIXEL_SHADER_COMBO( FLOW_CHEAP, bHasCheapFlow );
		SET_STATIC_PIXEL_SHADER( solidenergy_ps20b );

		// Textures
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); // [sRGB] Base
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

		if ( bDetail1 )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true ); // [sRGB] Detail1
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );
		}

		if ( bDetail2 )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER4, true ); // [sRGB] Detail2
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, true );
		}

		if ( bHasFlowmap )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER5, true ); // Flow map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER5, false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER6, true ); // Flow map noise
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER6, false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER7, true ); // Flow map bounds
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER7, false );
		}

		if ( bAlphaBlend )
		{
			if ( bAdditiveBlend )
			{
				pShader->EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
			}
			else
			{
				pShader->EnableAlphaBlending(
				    SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			}
			pShaderShadow->EnableAlphaWrites( false );
			pShaderShadow->EnableDepthWrites( !bAdditiveBlend );
		}
		else
		{
			pShader->DisableAlphaBlending();
			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableDepthWrites( true );
		}

		pShaderShadow->EnableSRGBWrite( true );
	}
	DYNAMIC_STATE
	{
		if ( pShaderAPI->InFlashlightMode() )
		{
			// Don't draw anything for the flashlight pass
			pShader->Draw( false );
			return;
		}

		// VS consts
		if ( info.m_nBaseTextureTransform != -1 )
			pShader->SetVertexShaderTextureTransform(
			    VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform ); // 0-1

		if ( IS_PARAM_DEFINED( info.m_nDetail1TextureTransform ) )
			pShader->SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2,
			    info.m_nDetail1TextureTransform, info.m_nDetail1Scale ); // 2-3
		else
			pShader->SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2,
			    info.m_nBaseTextureTransform, info.m_nDetail1Scale );

		if ( IS_PARAM_DEFINED( info.m_nDetail2TextureTransform ) )
			pShader->SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6,
			    info.m_nDetail2TextureTransform, info.m_nDetail2Scale ); // 6-7
		else
			pShader->SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6,
			    info.m_nBaseTextureTransform, info.m_nDetail2Scale );

		// Textures
		pShader->BindTexture( SHADER_SAMPLER0, info.m_nBaseTexture, -1 );
		if ( bDetail1 )
			pShader->BindTexture( SHADER_SAMPLER1, info.m_nDetail1Texture, info.m_nDetail1Frame );
		if ( bDetail2 )
			pShader->BindTexture( SHADER_SAMPLER4, info.m_nDetail2Texture, info.m_nDetail2Frame );
		if ( bHasFlowmap )
		{
			pShader->BindTexture( SHADER_SAMPLER5, info.m_nFlowMap, info.m_nFlowMapFrame );
			pShader->BindTexture( SHADER_SAMPLER6, info.m_nFlowNoiseTexture, -1 );
			pShader->BindTexture( SHADER_SAMPLER7, info.m_nFlowBoundsTexture, -1 );
		}

		// PS constants
		float flConsts[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		if ( IS_PARAM_DEFINED( info.m_nTangentTOpacityRanges ) )
			params[info.m_nTangentTOpacityRanges]->GetVecValue( flConsts, 4 );
		else
			memcpy( flConsts, kDefaultFalloffRanges, sizeof( kDefaultFalloffRanges ) );
		pShaderAPI->SetPixelShaderConstant( 0, flConsts, 1 );

		if ( IS_PARAM_DEFINED( info.m_nTangentSOpacityRanges ) )
			params[info.m_nTangentSOpacityRanges]->GetVecValue( flConsts, 4 );
		else
			memcpy( flConsts, kDefaultFalloffRanges, sizeof( kDefaultFalloffRanges ) );
		pShaderAPI->SetPixelShaderConstant( 1, flConsts, 1 );

		if ( IS_PARAM_DEFINED( info.m_nFresnelOpacityRanges ) )
			params[info.m_nFresnelOpacityRanges]->GetVecValue( flConsts, 4 );
		else
			memcpy( flConsts, kDefaultFalloffRanges, sizeof( kDefaultFalloffRanges ) );
		pShaderAPI->SetPixelShaderConstant( 2, flConsts, 1 );

		float flOutputIntensity = params[info.m_nOutputIntensity]->GetFloatValue();
		if ( bHasFlowmap )
		{
			float vFlowConst1[4] = {
			    params[info.m_nFlowWorldUVScale]->GetFloatValue(), 0.0f, 0.0f, flOutputIntensity };
			pShaderAPI->SetPixelShaderConstant( 6, vFlowConst1, 1 );

			float vFlowConst2[4] = { params[info.m_nFlowTimeIntervalInSeconds]->GetFloatValue(),
			    params[info.m_nFlowUVScrollDistance]->GetFloatValue(), 0.0f,
			    params[info.m_nFlowLerpExp]->GetFloatValue() };
			pShaderAPI->SetPixelShaderConstant( 7, vFlowConst2, 1 );

			float vFlowConst3[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			params[info.m_nFlowColor]->GetVecValue( vFlowConst3, 3 );
			pShaderAPI->SetPixelShaderConstant( 8, vFlowConst3, 1 );

			float vFlowConst4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			params[info.m_nFlowVortexColor]->GetVecValue( vFlowConst4, 3 );
			vFlowConst4[3] = params[info.m_nFlowVortexSize]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 9, vFlowConst4, 1 );
		}
		else
		{
			float vFlowConst1[4] = { 0.0f, 0.0f, 0.0f, flOutputIntensity };
			pShaderAPI->SetPixelShaderConstant( 6, vFlowConst1, 1 );
		}

		bool bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha() && !bAlphaBlend;
		float flPowerUp = params[info.m_nPowerUp]->GetFloatValue();
		float flIntensity = params[info.m_nFlowColorIntensity]->GetFloatValue();

		bool bActive = ( flIntensity > 0.0f );
		if ( bHasFlowmap && ( flPowerUp <= 0.0f ) )
		{
			bActive = false;
		}

		bool bPowerup = bActive && bHasFlowmap && ( flPowerUp > 0.0f && flPowerUp < 1.0f );
		bool bVortex1 = bActive && bHasFlowmap && ( info.m_nFlowVortex1 != -1 ) &&
		                ( params[info.m_nFlowVortex1]->GetIntValue() != 0 );
		bool bVortex2 = bActive && bHasFlowmap && ( info.m_nFlowVortex2 != -1 ) &&
		                ( params[info.m_nFlowVortex2]->GetIntValue() != 0 );

		DECLARE_DYNAMIC_VERTEX_SHADER( solidenergy_vs20 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( VORTEX1, bVortex1 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( VORTEX2, bVortex2 );
		SET_DYNAMIC_VERTEX_SHADER( solidenergy_vs20 );

		pShaderAPI->GetWorldSpaceCameraPosition( flConsts );
		flConsts[3] = 0.0f;
		pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, flConsts, 1 );

		if ( bHasFlowmap )
		{
			float vFlowConst1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			params[info.m_nFlowVortexPos1]->GetVecValue( vFlowConst1, 3 );
			vFlowConst1[3] = params[info.m_nFlowNoiseScale]->GetFloatValue();
			pShaderAPI->SetVertexShaderConstant(
			    VERTEX_SHADER_SHADER_SPECIFIC_CONST_9, vFlowConst1, 1 );

			float vFlowConst2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			params[info.m_nFlowVortexPos2]->GetVecValue( vFlowConst2, 3 );
			vFlowConst2[3] = params[info.m_nFlowNormalUVScale]->GetFloatValue();
			pShaderAPI->SetVertexShaderConstant(
			    VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, vFlowConst2, 1 );
		}

		DECLARE_DYNAMIC_PIXEL_SHADER( solidenergy_ps20b );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( ACTIVE, bActive );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( POWERUP, bPowerup );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( VORTEX1, bVortex1 );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( VORTEX2, bVortex2 );
		SET_DYNAMIC_PIXEL_SHADER( solidenergy_ps20b );

		flConsts[0] = bWriteDepthToAlpha ? 1.0f : 0.0f;
		flConsts[1] = pShaderAPI->CurrentTime();
		flConsts[2] = flPowerUp;
		flConsts[3] = flIntensity;
		pShaderAPI->SetPixelShaderConstant( 3, flConsts, 1 );

		// The combos, for backends that implement the shader from its constants.
		float vCombos[4] = { bActive ? 1.0f : 0.0f, bPowerup ? 1.0f : 0.0f, bVortex1 ? 1.0f : 0.0f,
		    bVortex2 ? 1.0f : 0.0f };
		pShaderAPI->SetPixelShaderConstant( 10, vCombos, 1 );
		int nStaticFlags =
		    ( bAdditiveBlend ? SOLIDENERGY_ADDITIVE : 0 ) | ( bDetail1 ? SOLIDENERGY_DETAIL1 : 0 ) |
		    ( bDetail2 ? SOLIDENERGY_DETAIL2 : 0 ) |
		    ( bTangentT ? SOLIDENERGY_TANGENTTOPACITY : 0 ) |
		    ( bTangentS ? SOLIDENERGY_TANGENTSOPACITY : 0 ) |
		    ( bFresnel ? SOLIDENERGY_FRESNELOPACITY : 0 ) |
		    ( ( bHasVertexColor || bHasVertexAlpha ) ? SOLIDENERGY_VERTEXCOLOR : 0 ) |
		    ( bHasFlowmap ? SOLIDENERGY_FLOWMAP : 0 ) | ( bModel ? SOLIDENERGY_MODELFORMAT : 0 );
		float vStatic[4] = { (float)nStaticFlags, (float)nDetail1BlendMode,
		    (float)nDetail2BlendMode, bHasCheapFlow ? 1.0f : 0.0f };
		pShaderAPI->SetPixelShaderConstant( 11, vStatic, 1 );
	}
	pShader->Draw();
}
