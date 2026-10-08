//========= Copyright Valve Corporation, All rights reserved. ============//

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "aftershock_helper.h"
#include "convar.h"

// Auto generated inc files


void InitParamsAftershock( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, AftershockVars_t &info )
{
	// Set material flags
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );

	SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE );

	// Set material parameter default values
	if ( ( info.m_nRefractAmount != -1 ) && ( !params[info.m_nRefractAmount]->IsDefined() ) )
	{
		params[info.m_nRefractAmount]->SetFloatValue( kDefaultRefractAmount );
	}

	if ( ( info.m_nColorTint != -1 ) && ( !params[info.m_nColorTint]->IsDefined() ) )
	{
		params[info.m_nColorTint]->SetVecValue( kDefaultColorTint[0], kDefaultColorTint[1], kDefaultColorTint[2], kDefaultColorTint[3] );
	}

	if( (info.m_nBumpFrame != -1 ) && !params[info.m_nBumpFrame]->IsDefined() )
	{
		params[info.m_nBumpFrame]->SetIntValue( 0 );
	}

	if ( ( info.m_nSilhouetteThickness != -1 ) && ( !params[info.m_nSilhouetteThickness]->IsDefined() ) )
	{
		params[info.m_nSilhouetteThickness]->SetFloatValue( kDefaultSilhouetteThickness );
	}

	if ( ( info.m_nSilhouetteColor != -1 ) && ( !params[info.m_nSilhouetteColor]->IsDefined() ) )
	{
		params[info.m_nSilhouetteColor]->SetVecValue( kDefaultSilhouetteColor[0], kDefaultSilhouetteColor[1], kDefaultSilhouetteColor[2], kDefaultSilhouetteColor[3] );
	}

	if ( ( info.m_nGroundMin != -1 ) && ( !params[info.m_nGroundMin]->IsDefined() ) )
	{
		params[info.m_nGroundMin]->SetFloatValue( kDefaultGroundMin );
	}

	if ( ( info.m_nGroundMax != -1 ) && ( !params[info.m_nGroundMax]->IsDefined() ) )
	{
		params[info.m_nGroundMax]->SetFloatValue( kDefaultGroundMax );
	}

	if ( ( info.m_nBlurAmount != -1 ) && ( !params[info.m_nBlurAmount]->IsDefined() ) )
	{
		params[info.m_nBlurAmount]->SetFloatValue( kDefaultBlurAmount );
	}

	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nTime, 0.0f );
}

void InitAftershock( CBaseVSShader *pShader, IMaterialVar** params, AftershockVars_t &info )
{
	// Load textures
	if ( (info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBumpmap );
	}
}

void DrawAftershock( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
					IShaderShadow* pShaderShadow, AftershockVars_t &info, VertexCompressionType_t vertexCompression )
{
	bool bBumpMapping = ( info.m_nBumpmap == -1 ) || !params[info.m_nBumpmap]->IsTexture() ? 0 : 1;

	SHADOW_STATE
	{
		// Set stream format (note that this shader supports compression)
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		// Vertex Shader
	
		// Pixel Shader
		if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
		{
		}
		else
		{
		}

		// Textures
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); // Refraction texture
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true ); // Bump
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false ); // Not sRGB
		pShaderShadow->EnableSRGBWrite( true );

		// Blending
		pShader->EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		pShaderShadow->EnableAlphaWrites( false );

		// !!! We need to turn this back on because EnableAlphaBlending() above disables it!
		//pShaderShadow->EnableDepthWrites( true );
	}
	pShader->Draw();
}
