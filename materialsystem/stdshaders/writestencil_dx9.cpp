//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( WriteStencil, WriteStencil_DX9 )

BEGIN_VS_SHADER_FLAGS( WriteStencil_DX9, "Help for WriteStencil", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
			return "WriteStencil_DX8";

		return 0;
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableColorWrites( false );	//
			pShaderShadow->EnableAlphaWrites( false );	//	Write ONLY to stencil
			pShaderShadow->EnableDepthWrites( false );	//


			// No pixel shader on Direct3D, doubles fill rate
			if ( g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() )
			{
			}

			// Set stream format (note that this shader supports compression)
			unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			int nTexCoordCount = 1;
			int userDataSize = 0;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );
		}
		Draw();
	}
END_SHADER

