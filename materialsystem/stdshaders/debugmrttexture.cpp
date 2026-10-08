//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"


BEGIN_VS_SHADER_FLAGS( DebugMRTTexture, "Help for DebugMRTTexture", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( MRTINDEX, SHADER_PARAM_TYPE_INTEGER, "", "" )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
//		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
//		{
//			return "UnlitGeneric_DX8";
//		}
		return 0;
	}

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}

			int numTexCoords = 2;
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, numTexCoords, 0, 0 );
		}
		Draw();
	}
END_SHADER

