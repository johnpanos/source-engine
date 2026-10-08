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
	pShader->Draw();
}
