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


BEGIN_VS_SHADER( vr_distort_hud, "Help for hud warp" )
	BEGIN_SHADER_PARAMS
	
		SHADER_PARAM( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_gui", "" )
		SHADER_PARAM( DISTORTMAP, SHADER_PARAM_TYPE_TEXTURE, "vr_distort_map_left", "" )
		SHADER_PARAM( DISTORTBOUNDS, SHADER_PARAM_TYPE_VEC4, "[ 0 0 1 1 ]", "" )
		SHADER_PARAM( HUDTRANSLUCENT, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( HUDUNDISTORT, SHADER_PARAM_TYPE_INTEGER, "0", "" )

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
		LoadTexture( DISTORTMAP, TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_NODEBUGOVERRIDE | TEXTUREFLAGS_SINGLECOPY |
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT );
	}

	SHADER_DRAW
	{


		SHADOW_STATE
		{
			SetInitialShadowState( );

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			pShaderShadow->EnableSRGBWrite( true );
			pShaderShadow->EnableAlphaWrites( false );

			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.0f );

			if ( IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) )
			{
				pShaderShadow->EnableAlphaTest( true );
				pShaderShadow->EnableBlending( true );
				pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			}
			else
			{
				pShaderShadow->EnableAlphaTest( false );
				pShaderShadow->EnableBlending( false );
				pShaderShadow->BlendFunc( SHADER_BLEND_ONE, SHADER_BLEND_ZERO );
			}

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
