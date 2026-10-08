//========= Copyright Valve Corporation, All rights reserved. ============//

#include "BaseVSShader.h"
#include "shaderlib/cshader.h"

#include "debugtextureview_vs20.inc"
#include "debugtextureview_ps20.inc"
#include "debugtextureview_ps20b.inc"

DEFINE_FALLBACK_SHADER( DebugTextureView, DebugTextureView_dx9 )
BEGIN_VS_SHADER( DebugTextureView_dx9, "Help for DebugTextureView" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( SHOWALPHA, SHADER_PARAM_TYPE_BOOL, "0", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if ( params[BASETEXTURE]->IsDefined() )
		{
			LoadTexture( BASETEXTURE );
		}
	}
	
	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "UnlitGeneric";
		}
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableAlphaTest( true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			// Set stream format (note that this shader supports compression)
			unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			int nTexCoordCount = 1;
			int userDataSize = 0;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

			DECLARE_STATIC_VERTEX_SHADER( debugtextureview_vs20 );
			SET_STATIC_VERTEX_SHADER( debugtextureview_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( debugtextureview_ps20b );
				SET_STATIC_PIXEL_SHADER_COMBO( SHOWALPHA, params[SHOWALPHA]->GetIntValue() != 0 );
				SET_STATIC_PIXEL_SHADER( debugtextureview_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( debugtextureview_ps20 );
				SET_STATIC_PIXEL_SHADER_COMBO( SHOWALPHA, params[SHOWALPHA]->GetIntValue() != 0 );
				SET_STATIC_PIXEL_SHADER( debugtextureview_ps20 );
			}
		}

		Draw();
	}
END_SHADER
