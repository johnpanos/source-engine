//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "BaseVSShader.h"


#include "../materialsystem_global.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"







static const float kAllZeros[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };


BEGIN_VS_SHADER( vr_distort_texture, "Help for warp" )
	BEGIN_SHADER_PARAMS
	
		SHADER_PARAM( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( DISTORTMAP, SHADER_PARAM_TYPE_TEXTURE, "vr_distort_map", "" )
		SHADER_PARAM( USERENDERTARGET, SHADER_PARAM_TYPE_INTEGER, "0", "" )

	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
		LoadTexture( DISTORTMAP, TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_NODEBUGOVERRIDE |
			TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT );
	}

	SHADER_DRAW
	{


		SHADOW_STATE
		{
			SetInitialShadowState( );

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );

			pShaderShadow->EnableBlending( false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			pShaderShadow->EnableSRGBWrite( true );
			pShaderShadow->EnableAlphaWrites( false );
			pShaderShadow->EnableAlphaTest( false );

			DefaultFog();

			int nFormat = 0;
			nFormat |= VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( nFormat, 2, 0, 0 );

			if ( !g_pHardwareConfig->SupportsShaderModel_3_0() )
			{

				if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				{
				}
				else
				{
				}
			}
			else
			{

			}
		}

		Draw();
	}
END_SHADER
