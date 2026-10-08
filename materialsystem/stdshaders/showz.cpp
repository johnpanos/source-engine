//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Visualize shadow z buffers.  Designed to be used when drawing a screen-aligned
//          quad with a floating-point z-buffer so that the large z-range is divided down
//          into visual range of grayscale colors.
//
// $NoKeywords: $
//=============================================================================//

#include "convar.h"
#include "BaseVSShader.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar r_showz_power( "r_showz_power", "1.0f", FCVAR_CHEAT );

BEGIN_VS_SHADER_FLAGS( showz, "Help for ShowZ", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALPHADEPTH, SHADER_PARAM_TYPE_INTEGER, "0", "Depth is stored in alpha channel" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		if ( !params[ALPHADEPTH]->IsDefined() )
		{
			params[ALPHADEPTH]->SetIntValue( 0 );
		}
	}

	SHADER_FALLBACK
	{
//		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
//			return "Wireframe";
		return 0;
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );

			pShaderShadow->EnableSRGBWrite( true );  // The back buffer is sRGB, we should always set this true!
		}
		Draw();
	}
END_SHADER

