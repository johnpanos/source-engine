//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"
#include "common_hlsl_cpp_consts.h"

#include "Downsample_ps20.inc"
#include "Downsample_ps20b.inc"

BEGIN_VS_SHADER_FLAGS( sfm_downsample_shader, "Help for Downsample", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
	}
	
	SHADER_FALLBACK
	{
		// Requires DX9 + above
		if (!g_pHardwareConfig->SupportsVertexAndPixelShaders())
		{
			Assert( 0 );
			return "Wireframe";
		}
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableAlphaWrites( true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			pShaderShadow->SetVertexShader( "downsample_vs20", 0 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( downsample_ps20b );
				SET_STATIC_PIXEL_SHADER( downsample_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( downsample_ps20 );
				SET_STATIC_PIXEL_SHADER( downsample_ps20 );
			}
		}

		Draw();
	}
END_SHADER
