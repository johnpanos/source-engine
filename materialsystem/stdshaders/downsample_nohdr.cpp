//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//============================================================================//

#include "BaseVSShader.h"
#include "convar.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static ConVar r_bloomtintr( "r_bloomtintr", "0.3" );
static ConVar r_bloomtintg( "r_bloomtintg", "0.59" );
static ConVar r_bloomtintb( "r_bloomtintb", "0.11" );
static ConVar r_bloomtintexponent( "r_bloomtintexponent", "2.2" );

BEGIN_VS_SHADER_FLAGS( Downsample_nohdr, "Help for Downsample_nohdr", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BLOOMTINTENABLE, SHADER_PARAM_TYPE_INTEGER, "1", "" )
		SHADER_PARAM( CSTRIKE, SHADER_PARAM_TYPE_INTEGER, "0", "" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		if ( !params[ BLOOMTINTENABLE ]->IsDefined() )
		{
			params[ BLOOMTINTENABLE ]->SetIntValue( 1 );
		}
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "Downsample_nohdr_DX80";
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

			// Render targets are pegged as sRGB on OSX, so just force these reads and writes
			bool bForceSRGBReadAndWrite = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, bForceSRGBReadAndWrite );
			pShaderShadow->EnableSRGBWrite( bForceSRGBReadAndWrite );

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );

			
			if( g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders() )
			{
			}
			else
			{
			}
		}

		Draw();
	}
END_SHADER
