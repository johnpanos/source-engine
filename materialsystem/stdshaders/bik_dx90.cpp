//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"


BEGIN_VS_SHADER( Bik, "Help for Bik" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( YTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Y Bink Texture" )
		// re-enable this if we want alpha blending
//		SHADER_PARAM( ATEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "A Bink Texture" )
		SHADER_PARAM( CRTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Cr Bink Texture" )
		SHADER_PARAM( CBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Cb Bink Texture" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "bik_dx81";
		}
		return 0;
	}

	SHADER_INIT
	{
		if ( params[YTEXTURE]->IsDefined() )
		{
			LoadTexture( YTEXTURE );
		}
//		if ( params[ATEXTURE]->IsDefined() )
//		{
//			LoadTexture( ATEXTURE );
//		}
		if ( params[CRTEXTURE]->IsDefined() )
		{
			LoadTexture( CRTEXTURE );
		}
		if ( params[CBTEXTURE]->IsDefined() )
		{
			LoadTexture( CBTEXTURE );
		}
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
//			pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );

			unsigned int flags = VERTEX_POSITION;
			int numTexCoords = 1;
			pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, 0, 0 );


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}

			// The 360 needs an sRGB write, but NOT an sRGB read!
			pShaderShadow->EnableSRGBWrite( false );

//			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		}
		Draw( );
	}
END_SHADER
