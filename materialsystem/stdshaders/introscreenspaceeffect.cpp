//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"


BEGIN_VS_SHADER_FLAGS( IntroScreenSpaceEffect, "Help for IntroScreenSpaceEffect", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( MODE, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( ENABLESRGB, SHADER_PARAM_TYPE_BOOL, "0", "" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );

		if ( !params[ENABLESRGB]->IsDefined() )
		{
			params[ENABLESRGB]->SetIntValue( 0 );
		}
	}

	SHADER_INIT
	{
	}
	
	SHADER_FALLBACK
	{
		// Requires DX9 + above
		if ( IsWindows() && ( g_pHardwareConfig->GetDXSupportLevel() < 90 || g_pHardwareConfig->PreferReducedFillrate() ) )
			return "IntroScreenSpaceEffect_dx80";
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			// On OpenGL OSX, we MUST do sRGB reads from the bloom and full framebuffer textures AND sRGB writes on the way out to the framebuffer.
			if ( params[ENABLESRGB]->GetIntValue() || IsOSX() )
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );
				pShaderShadow->EnableSRGBWrite( true );
			}

			// Only need the adapter if the shader expects sRGB values and we're forced to do an sRGB read by the API/Hardware
			bool bNeedsSRGBAdapter = ( params[ENABLESRGB]->GetIntValue() == 0 ) && IsOSX() && !g_pHardwareConfig->FakeSRGBWrite() && g_pHardwareConfig->CanDoSRGBReadFromRTs();

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );

			
			if ( g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders() ) // GL always goes the ps2b way for this shader, even on "ps20" parts
			{
			}
			else
			{
			}

			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
		}
		Draw();
	}
END_SHADER
