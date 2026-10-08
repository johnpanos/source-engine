//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A shader that builds the shadow using render-to-texture
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( ShadowBuild, ShadowBuild_DX9 )

BEGIN_VS_SHADER_FLAGS( ShadowBuild_DX9, "Help for ShadowBuild", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( TRANSLUCENT_MATERIAL, SHADER_PARAM_TYPE_MATERIAL, "", "Points to a material to grab translucency from" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
	}

	SHADER_FALLBACK
	{
		if ( !g_pHardwareConfig->SupportsVertexAndPixelShaders() )
			return "ShadowBuild_DX6";

		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
			return "ShadowBuild_DX8";

		return 0;
	}

	SHADER_INIT
	{
		if ( params[BASETEXTURE]->IsDefined() )
		{
			LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
		}
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			// Add the alphas into the frame buffer
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );

			// base texture.  We just use this for alpha, but enable SRGB read to make everything consistent.
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			pShaderShadow->EnableSRGBWrite( true );

			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_ALWAYS );

			// Specify vertex format (note that this shader supports compression)
			unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			unsigned int nTexCoordCount = 1;
			unsigned int userDataSize = 0;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}
		}
		Draw( );
	}
END_SHADER
