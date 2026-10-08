//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "depthwrite_ps20.inc"
#include "depthwrite_ps20b.inc"
#include "depthwrite_vs20.inc"

#include "depthwrite_ps30.inc"
#include "depthwrite_vs30.inc"

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
				DECLARE_STATIC_VERTEX_SHADER( depthwrite_vs20 );
				SET_STATIC_VERTEX_SHADER_COMBO( ONLY_PROJECT_POSITION, !bAlphaClip && false && !nColorDepth ); //360 needs to know if it *shouldn't* output texture coordinates to avoid shader patches
				SET_STATIC_VERTEX_SHADER_COMBO( COLOR_DEPTH, nColorDepth );
				SET_STATIC_VERTEX_SHADER( depthwrite_vs20 );
				
				if ( bAlphaClip || g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() || nColorDepth )
				{
					if( bAlphaClip )
					{
						pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
						pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
					}

					if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
					{
						DECLARE_STATIC_PIXEL_SHADER( depthwrite_ps20b );
						SET_STATIC_PIXEL_SHADER_COMBO( COLOR_DEPTH, nColorDepth );
						SET_STATIC_PIXEL_SHADER( depthwrite_ps20b );
					}
					else
					{
						DECLARE_STATIC_PIXEL_SHADER( depthwrite_ps20 );
						SET_STATIC_PIXEL_SHADER_COMBO( COLOR_DEPTH, nColorDepth );
						SET_STATIC_PIXEL_SHADER( depthwrite_ps20 );
					}
				}
			}
			else
			{
				SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );

				DECLARE_STATIC_VERTEX_SHADER( depthwrite_vs30 );
				SET_STATIC_VERTEX_SHADER_COMBO( ONLY_PROJECT_POSITION, 0 ); //360 only combo, and this is a PC path
				SET_STATIC_VERTEX_SHADER_COMBO( COLOR_DEPTH, nColorDepth );
				SET_STATIC_VERTEX_SHADER( depthwrite_vs30 );

				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

				DECLARE_STATIC_PIXEL_SHADER( depthwrite_ps30 );
				SET_STATIC_PIXEL_SHADER_COMBO( COLOR_DEPTH, nColorDepth );
				SET_STATIC_PIXEL_SHADER( depthwrite_ps30 );
			}
		}
	// DYNAMIC_STATE

		Draw( );
	}
END_SHADER
