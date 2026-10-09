//========= Copyright Valve Corporation, All rights reserved. ============//

/* Based heavily on cloak_blended_pass, look at cloak_blended_pass_helper.cpp

==================================================================================================== */

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "weapon_sheen_pass_helper.h"
#include "convar.h"

// Auto generated inc files


void InitParamsWeaponSheenPass( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, WeaponSheenPassVars_t &info )
{
	// Set material flags
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS( MATERIAL_VAR_MODEL );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );

	// Set material parameter default values
	if ( ( info.m_nSheenMapMaskFrame != -1 ) && ( !params[info.m_nSheenMapMaskFrame]->IsDefined() ) )
	{
		params[info.m_nSheenMapMaskFrame]->SetFloatValue( 0 );
	}

	if ( ( info.m_nSheenMapTint != -1 ) && ( !params[info.m_nSheenMapTint]->IsDefined() ) )
	{
		params[info.m_nSheenMapTint]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	if ( ( info.m_nSheenMapMaskScaleX != -1 ) && ( !params[info.m_nSheenMapMaskScaleX]->IsDefined() ) )
	{
		params[info.m_nSheenMapMaskScaleX]->SetFloatValue( 1.0f );
	}
	if ( ( info.m_nSheenMapMaskScaleY != -1 ) && ( !params[info.m_nSheenMapMaskScaleY]->IsDefined() ) )
	{
		params[info.m_nSheenMapMaskScaleY]->SetFloatValue( 1.0f );
	}

	if ( ( info.m_nSheenMapMaskOffsetX != -1 ) && ( !params[info.m_nSheenMapMaskOffsetX]->IsDefined() ) )
	{
		params[info.m_nSheenMapMaskOffsetX]->SetFloatValue( 0 );
	}
	if ( ( info.m_nSheenMapMaskOffsetY != -1 ) && ( !params[info.m_nSheenMapMaskOffsetY]->IsDefined() ) )
	{
		params[info.m_nSheenMapMaskOffsetY]->SetFloatValue( 0 );
	}

	if ( ( info.m_nSheenMapMaskDirection != -1 ) && ( !params[info.m_nSheenMapMaskDirection]->IsDefined() ) )
	{
		params[info.m_nSheenMapMaskDirection]->SetFloatValue( 0 );
	}

	if( (info.m_nSheenIndex != -1 ) && !params[info.m_nSheenIndex]->IsDefined() )
	{
		params[info.m_nSheenIndex]->SetIntValue( 0 );
	}

	if( (info.m_nBumpFrame != -1 ) && !params[info.m_nBumpFrame]->IsDefined() )
	{
		params[info.m_nBumpFrame]->SetIntValue( 0 );
	}
}

void InitWeaponSheenPass( CBaseVSShader *pShader, IMaterialVar** params, WeaponSheenPassVars_t &info )
{
	// Load textures
	if ( g_pConfig->UseBumpmapping() )
	{
		if ( (info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nBumpmap );
		}
	}

	if ( (info.m_nSheenMap != -1) && params[info.m_nSheenMap]->IsDefined() )
	{
		pShader->LoadCubeMap( info.m_nSheenMap, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0  );
	}

	if ( (info.m_nSheenMapMask != -1) && params[info.m_nSheenMapMask]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nSheenMapMask );
	}
	
}

void DrawWeaponSheenPass( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
						  IShaderShadow* pShaderShadow, WeaponSheenPassVars_t &info, VertexCompressionType_t vertexCompression )
{
	bool bBumpMapping = ( !g_pConfig->UseBumpmapping() ) || ( info.m_nBumpmap == -1 ) || !params[info.m_nBumpmap]->IsTexture() ? 0 : 1;

	SHADOW_STATE
	{
		// Reset shadow state manually since we're drawing from two materials
		pShader->SetInitialShadowState( );

		// Set stream format (note that this shader supports compression)
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			// Vertex Shader

			// Pixel Shader
		}
		else
		{
			// The vertex shader uses the vertex id stream
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );

			// Vertex Shader

			// Pixel Shader
		}

		// Textures
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); // Refraction texture
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		if ( bBumpMapping )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true ); // Bump
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false ); // Not sRGB
		}
		pShaderShadow->EnableSRGBWrite( true );

		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
		if( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
		{
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true );
		}

		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );
		if( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
		{
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, true );
		}

		// Blending
		pShader->EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		pShaderShadow->EnableAlphaWrites( false );

		// !!! We need to turn this back on because EnableAlphaBlending() above disables it!
		pShaderShadow->EnableDepthWrites( true );
	}
	pShader->Draw();
}

bool ShouldDrawMaterialSheen ( IMaterialVar** params, WeaponSheenPassVars_t &info )
{
	// If the frame is zero we're not rendering
	if ( IS_PARAM_DEFINED( info.m_nSheenMapMaskFrame ) && params[info.m_nSheenMapMaskFrame]->GetIntValue() > 0 )
	{
		return true;
	}
	
	return false;
}