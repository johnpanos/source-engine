//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"



// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar gl_amd_occlusion_workaround( "gl_amd_occlusion_workaround", "1" );

DEFINE_FALLBACK_SHADER( Occlusion, Occlusion_DX9 )

BEGIN_VS_SHADER_FLAGS( Occlusion_DX9, "Help for Occlusion", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "Occlusion_DX8";
		}
		return 0;
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( false );
			pShaderShadow->EnableDepthWrites( false );


			// No pixel shader on Direct3D, doubles fill rate
			if ( g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() )
			{

				// Workaround for weird AMD bug - if sRGB write isn't enabled here then sRGB write enable in subsequent world rendering passes will randomly not take effect (even though we're enabling it) in the driver.
				if ( ( IsLinux() || IsWindows() || IsBSD() ) && gl_amd_occlusion_workaround.GetBool() )
				{
					pShaderShadow->EnableSRGBWrite( true );
				}
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

