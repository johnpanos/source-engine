//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "BaseVSShader.h"



// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( Teeth, Teeth_DX9 )

extern ConVar r_flashlight_version2;
BEGIN_VS_SHADER( Teeth_DX9, "Help for Teeth_DX9" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ILLUMFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "Amount to darken or brighten the teeth" )
		SHADER_PARAM( FORWARD, SHADER_PARAM_TYPE_VEC3, "[1 0 0]", "Forward direction vector for teeth lighting" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map" )
		SHADER_PARAM( PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "100", "phong exponent" )
		SHADER_PARAM( INTRO, SHADER_PARAM_TYPE_BOOL, "0", "is teeth in the ep1 intro" )
 	    SHADER_PARAM( ENTITYORIGIN, SHADER_PARAM_TYPE_VEC3,"0.0","center if the model in world space" )
 	    SHADER_PARAM( WARPPARAM, SHADER_PARAM_TYPE_FLOAT,"0.0","animation param between 0 and 1" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		if ( g_pHardwareConfig->SupportsBorderColor() )
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
		}
		else
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
		}

		SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );

		if( !params[INTRO]->IsDefined() )
		{
			params[INTRO]->SetIntValue( 0 );
		}
	}

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 || g_pConfig->bSoftwareLighting )
		{
			return "Teeth_dx8";
		}
		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB );
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );

		if( params[BUMPMAP]->IsDefined() )
		{
			LoadTexture( BUMPMAP );
		}
	}

	void DrawUsingVertexShader( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, VertexCompressionType_t vertexCompression )
	{
		bool hasBump = params[BUMPMAP]->IsTexture();

		BlendType_t nBlendType = EvaluateBlendRequirements( BASETEXTURE, true );
		bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !IS_FLAG_SET(MATERIAL_VAR_ALPHATEST); //dest alpha is free for special use
		
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );		// Base map

			int flags = VERTEX_POSITION | VERTEX_NORMAL;
			int nTexCoordCount = 1;
			int userDataSize = 0;

			if ( hasBump )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );	// Bump map
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false );
				pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );	// Normalization sampler for per-pixel lighting
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, false );
				userDataSize = 4;										// tangent S
			}

			// This shader supports compressed vertices, so OR in that flag:
			flags |= VERTEX_FORMAT_COMPRESSED;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

			if ( hasBump )
			{
				if ( !g_pHardwareConfig->HasFastVertexTextures() )
				{
					bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();


					// ps_2_b version which does phong
					if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
					{
					}
					else
					{
					}
				}
				else
				{
					// The vertex shader uses the vertex id stream
					SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );


				}
			}
			else
			{
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
					// The vertex shader uses the vertex id stream
					SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );


				}
			}

			// On DX9, do sRGB
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBWrite( true );

			FogToFogColor();

			pShaderShadow->EnableAlphaWrites( bFullyOpaque );
		}
		Draw();
	}

	void DrawFlashlight( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, VertexCompressionType_t vertexCompression )
	{
		SHADOW_STATE
		{
			// Be sure not to write to dest alpha
			pShaderShadow->EnableAlphaWrites( false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );		// Base map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );		// Flashlight spot
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );

			// Additive blend the teeth, lit by the flashlight
			s_pShaderShadow->EnableAlphaTest( false );
			s_pShaderShadow->BlendFunc( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			s_pShaderShadow->EnableBlending( true );

			// Set stream format (note that this shader supports compression)
			int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
			int nTexCoordCount = 1;
			int userDataSize = 0;
			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

			int nShadowFilterMode = 0;
			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );		// shadow depth map
				pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER2 );
				pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );		// shadow noise

				nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
			}

			if ( !g_pHardwareConfig->HasFastVertexTextures() )
			{

				if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				{
				}
				else
				{
				}
			}
			else
			{
				// The vertex shader uses the vertex id stream
				SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );


			}
			// On DX9, do sRGB
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBWrite( true );

			FogToFogColor();
		}
		Draw();
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
		}
		bool hasFlashlight = UsingFlashlight( params );
		if ( !hasFlashlight || ( false || r_flashlight_version2.GetInt() ) )
		{
			DrawUsingVertexShader( params, pShaderAPI, pShaderShadow, vertexCompression );
			SHADOW_STATE
			{
				SetInitialShadowState();
			}
		}
		if( hasFlashlight )
		{
			DrawFlashlight( params, pShaderAPI, pShaderShadow, vertexCompression );
		}
	}
END_SHADER
