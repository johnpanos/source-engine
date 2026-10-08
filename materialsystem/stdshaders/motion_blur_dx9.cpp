//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"
#include "motion_blur_vs20.inc"
#include "motion_blur_ps20.inc"
#include "motion_blur_ps20b.inc"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar mat_motion_blur_percent_of_screen_max( "mat_motion_blur_percent_of_screen_max", "4.0" );

DEFINE_FALLBACK_SHADER( MotionBlur, MotionBlur_dx9 )
BEGIN_VS_SHADER_FLAGS( MotionBlur_dx9, "Motion Blur", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( MOTIONBLURINTERNAL, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0]", "Internal motion blur value set by proxy" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "Wireframe";
		}

		return 0;
	}

	SHADER_INIT
	{
		if ( params[BASETEXTURE]->IsDefined() )
		{
			LoadTexture( BASETEXTURE, IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs() ? TEXTUREFLAGS_SRGB : 0 );
		}
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );

			// On OpenGL OSX, we must do sRGB reads and writes since these render targets are tagged as such
			bool bForceSRGBReadsAndWrites = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
			
			// NOTE: sRGB is disabled because of the NV8800 brokenness
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, bForceSRGBReadsAndWrites );
			pShaderShadow->EnableSRGBWrite( bForceSRGBReadsAndWrites );

			DECLARE_STATIC_VERTEX_SHADER( motion_blur_vs20 );
			SET_STATIC_VERTEX_SHADER( motion_blur_vs20 );

			if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( motion_blur_ps20b );
				SET_STATIC_PIXEL_SHADER( motion_blur_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( motion_blur_ps20 );
				SET_STATIC_PIXEL_SHADER( motion_blur_ps20 );
			}

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableAlphaWrites( false );
		}


		Draw();
	}
END_SHADER
