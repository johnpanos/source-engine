//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "shadow_ps20.inc"
#include "shadow_ps20b.inc"
#include "shadow_vs20.inc"

BEGIN_VS_SHADER_FLAGS( Shadow, "Help for Shadow", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		/*
		The alpha blending state either must be:

		Src Color * Dst Color + Dst Color * 0	
		(src color = C*A + 1-A)

		or

		// Can't be this, doesn't work with fog
		Src Color * Dst Color + Dst Color * (1-Src Alpha)	
		(src color = C * A, Src Alpha = A)
		*/
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "Shadow_DX8";
		}
		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );


			// NOTE: This is deliberately this way round (instead of "DST_COLOR, ZERO"),
			//       since these two permutations produce *different* values on 360!!
			//       This was causing undue darkening of the framebuffer by these
			//       shadows, which was highly noticeable in very dark areas:
			EnableAlphaBlending( SHADER_BLEND_ZERO, SHADER_BLEND_SRC_COLOR );


			unsigned int flags = VERTEX_POSITION | VERTEX_COLOR;
			int numTexCoords = 1;
			pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, 0, 0 );

			DECLARE_STATIC_VERTEX_SHADER( shadow_vs20 );
			SET_STATIC_VERTEX_SHADER( shadow_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( shadow_ps20b );
				SET_STATIC_PIXEL_SHADER( shadow_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( shadow_ps20 );
				SET_STATIC_PIXEL_SHADER( shadow_ps20 );
			}

			pShaderShadow->EnableSRGBWrite( true );

			// We need to fog to *white* regardless of overbrighting...
			FogToWhite();
		}
		Draw( );
	}
END_SHADER
