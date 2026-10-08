//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"



BEGIN_VS_SHADER_FLAGS( DepthWrite, "Help for Depth Write", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "", "Alpha reference value" )
		SHADER_PARAM( COLOR_DEPTH, SHADER_PARAM_TYPE_BOOL, "0", "Write depth as color")
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "Wireframe";
		}
		return 0;
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		bool bAlphaClip = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST );
		int nColorDepth = GetIntParam( COLOR_DEPTH, params, 0 );

		SHADOW_STATE
		{
			// Set stream format (note that this shader supports compression)
			unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			int nTexCoordCount = 1;
			int userDataSize = 0;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

			if ( nColorDepth == 0 )
			{
				// Bias primitives when rendering into shadow map so we get slope-scaled depth bias
				// rather than having to apply a constant bias in the filtering shader later
				pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_SHADOW_BIAS );
			}

			// Turn off writes to color buffer since we always sample shadows from the DEPTH texture later
			// This gives us double-speed fill when rendering INTO the shadow map
			pShaderShadow->EnableColorWrites( ( nColorDepth == 1 ) );
			pShaderShadow->EnableAlphaWrites( false );

			// Don't backface cull unless alpha clipping, since this can cause artifacts when the
			// geometry is clipped by the flashlight near plane
			// If a material was already marked nocull, don't cull it
			pShaderShadow->EnableCulling( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) && !IS_FLAG_SET(MATERIAL_VAR_NOCULL) );

			if ( !g_pHardwareConfig->HasFastVertexTextures() )
			{
 //360 needs to know if it *shouldn't* output texture coordinates to avoid shader patches
				
				if ( bAlphaClip || g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() || nColorDepth )
				{
					if( bAlphaClip )
					{
						pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
						pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
					}

					if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
					{
					}
					else
					{
					}
				}
			}
			else
			{
				SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );

 //360 only combo, and this is a PC path

				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			}
		}
	// DYNAMIC_STATE

		Draw( );
	}
END_SHADER
