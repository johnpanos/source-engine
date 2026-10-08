//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"


BEGIN_VS_SHADER( Sample4x4_Blend, "Help for Sample4x4_Blend" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( PIXSHADER, SHADER_PARAM_TYPE_STRING, "sample4x4_ps20", "Name of the pixel shader to use" )
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
			
			
			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				const char *szPixelShader = params[PIXSHADER]->GetStringValue();
				size_t iLength = Q_strlen( szPixelShader );

				if( (iLength > 5) && (Q_stricmp( &szPixelShader[iLength - 5], "_ps20" ) == 0) ) //detect if it's trying to load a ps20 shader
				{
					//replace it with the ps20b shader
					char *szNewName = (char *)stackalloc( sizeof( char ) * (iLength + 2) );
					memcpy( szNewName, szPixelShader, sizeof( char ) * iLength );
					szNewName[iLength] = 'b';
					szNewName[iLength + 1] = '\0';
				}
				else
				{
				}
			}
			else
			{
			}

			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA,
									  SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		}

		Draw();
	}
END_SHADER
