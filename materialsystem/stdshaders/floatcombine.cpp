//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "screenspaceeffect_vs20.inc"
#include "floatcombine_ps20.inc"
#include "floatcombine_ps20b.inc"

BEGIN_VS_SHADER_FLAGS( floatcombine, "Help for floatcombine", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BLOOMTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( SHARPNESS, SHADER_PARAM_TYPE_FLOAT, "1", "" )
	    SHADER_PARAM( WOODCUT, SHADER_PARAM_TYPE_FLOAT, "0", "" )
		SHADER_PARAM( VIGNETTE_MIN_BRIGHT,SHADER_PARAM_TYPE_FLOAT,"1","")
		SHADER_PARAM( VIGNETTE_POWER,SHADER_PARAM_TYPE_FLOAT,"4","")
		SHADER_PARAM( EDGE_SOFTNESS,SHADER_PARAM_TYPE_FLOAT,"0","")
	    SHADER_PARAM( BLOOMAMOUNT, SHADER_PARAM_TYPE_FLOAT, "1.0", "" )
		SHADER_PARAM( BLOOMEXPONENT, SHADER_PARAM_TYPE_FLOAT, "2.0", "" )
		SHADER_PARAM( ALPHASHARPENFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if( params[BASETEXTURE]->IsDefined() )
		{
			LoadTexture( BASETEXTURE );
		}
		if( params[BLOOMTEXTURE]->IsDefined() )
		{
			LoadTexture( BLOOMTEXTURE );
		}
	}
	
	SHADER_FALLBACK
	{
		// Requires DX9 + above
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
//			Assert( 0 );
			return "Wireframe";
		}
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			// convert from linear to gamma on write.
			pShaderShadow->EnableSRGBWrite( true );

			// Pre-cache shaders
			DECLARE_STATIC_VERTEX_SHADER( screenspaceeffect_vs20 );
			SET_STATIC_VERTEX_SHADER( screenspaceeffect_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( floatcombine_ps20b );
				SET_STATIC_PIXEL_SHADER( floatcombine_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( floatcombine_ps20 );
				SET_STATIC_PIXEL_SHADER( floatcombine_ps20 );
			}
		}

		Draw();
	}
END_SHADER
