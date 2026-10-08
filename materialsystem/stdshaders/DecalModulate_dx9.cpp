//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"



// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( DecalModulate, DecalModulate_DX9 )


BEGIN_VS_SHADER( DecalModulate_dx9, 
			  "Help for DecalModulate_dx9" )
			  
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS
	
	SHADER_FALLBACK
	{
		if (g_pHardwareConfig->GetDXSupportLevel() < 90)
			return "DecalModulate_DX6";
		return 0;
	}

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );

		if ( g_pHardwareConfig->HasFastVertexTextures() )
		{
			// The vertex shader uses the vertex id stream
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );
			SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
		}
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableAlphaTest( true );
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.0f );
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL );
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			// Be sure not to write to dest alpha
			pShaderShadow->EnableAlphaWrites( false );

			// SRGB conversions hose the blend on some hardware, so keep we everything in gamma space
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, false );
			pShaderShadow->EnableSRGBWrite( false );

			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR );
			pShaderShadow->DisableFogGammaCorrection( true ); //fog should stay exactly middle grey
			FogToGrey();

			if ( !g_pHardwareConfig->HasFastVertexTextures() )
			{
				bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();


				if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				{
				}
				else
				{
				}
			}
			else
			{

			}

			// Set stream format (note that this shader supports compression)
			unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			// The VS30 shader offsets decals along the normal (for morphed geom)
			flags |= g_pHardwareConfig->HasFastVertexTextures() ? VERTEX_NORMAL : 0;
			int pTexCoordDim[3] = { 2, 0, 3 };
			int nTexCoordCount = 1;
			int userDataSize = 0;

			if ( g_pHardwareConfig->HasFastVertexTextures() )
			{
				nTexCoordCount = 3;
			}

			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, pTexCoordDim, userDataSize );
		}
		Draw( );
	}
END_SHADER
