//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"
#include "BlurFilter_vs20.inc"
#include "BlurFilter_ps20.inc"
#include "BlurFilter_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER_FLAGS( BlurFilterX, "Help for BlurFilterX", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if( params[BASETEXTURE]->IsDefined() )
		{
			LoadTexture( BASETEXTURE );
		}
	}
	
	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "BlurFilterX_DX80";
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
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );

			// Render targets are pegged as sRGB on POSIX, so just force these reads and writes
			bool bForceSRGBReadAndWrite = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, bForceSRGBReadAndWrite );
			pShaderShadow->EnableSRGBWrite( bForceSRGBReadAndWrite );

			// Pre-cache shaders
			blurfilter_vs20_Static_Index vshIndex;
			pShaderShadow->SetVertexShader( "BlurFilter_vs20", vshIndex.GetIndex() );
			
			if( g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders() )
			{
				DECLARE_STATIC_PIXEL_SHADER( blurfilter_ps20b );
				SET_STATIC_PIXEL_SHADER_COMBO( APPROX_SRGB_ADAPTER, bForceSRGBReadAndWrite );
				SET_STATIC_PIXEL_SHADER( blurfilter_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( blurfilter_ps20 );
				SET_STATIC_PIXEL_SHADER( blurfilter_ps20 );
			}

			if ( IS_FLAG_SET( MATERIAL_VAR_ADDITIVE ) )
				EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
		}

		Draw();
	}
END_SHADER
