//========= Copyright Valve Corporation, All rights reserved. ============//

#include "BaseVSShader.h"
#include "shaderlib/cshader.h"


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


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}
		}

		Draw();
	}
END_SHADER
