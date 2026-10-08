//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "BaseVSShader.h"
#include "mathlib/bumpvects.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( DecalBaseTimesLightmapAlphaBlendSelfIllum, DecalBaseTimesLightmapAlphaBlendSelfIllum_DX9 )

extern ConVar r_flashlight_version2;

BEGIN_VS_SHADER( DecalBaseTimesLightmapAlphaBlendSelfIllum_DX9, "" )
			  
	BEGIN_SHADER_PARAMS
		SHADER_PARAM_OVERRIDE( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "decals/decalporthole001b", "decal base texture", 0 )
		SHADER_PARAM( SELFILLUMTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "decals/decalporthole001b_mask", "self-illum texture" )
		SHADER_PARAM( SELFILLUMTEXTUREFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "self-illum texture frame" )
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

		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
		SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "DecalBaseTimesLightmapAlphaBlendSelfIllum_DX8";
		}
		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB );
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
		LoadTexture( SELFILLUMTEXTURE );
	}

	void DrawDecal( IMaterialVar **params, IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow )
	{
		if( IsSnapshotting() )
		{
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL );
			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			pShaderShadow->EnableSRGBWrite( true );

			// Base Texture
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			// Lightmaps
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE );
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE );
			pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE );

			int pTexCoords[3] = { 2, 2, 1 };
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_COLOR, 3, pTexCoords, 0 );


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}

			FogToFogColor();
		}
		Draw();

		if( IsSnapshotting() )
		{
			SetInitialShadowState( );
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL );
			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			
			// Base texture
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			pShaderShadow->EnableSRGBWrite( true );

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}

			FogToFogColor();
		}
		Draw();
	}

	void DrawPass( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bUsingFlashlight )
	{
		if( bUsingFlashlight )
		{
			DrawFlashlight_dx90_Vars_t flashlightVars;
			flashlightVars.m_bBump = false;
			flashlightVars.m_nBumpmapVar = -1;
			flashlightVars.m_nBumpmapFrame = -1;
			flashlightVars.m_nBumpTransform = -1;
			flashlightVars.m_nFlashlightTextureVar = FLASHLIGHTTEXTURE;
			flashlightVars.m_nFlashlightTextureFrameVar = FLASHLIGHTTEXTUREFRAME;
			flashlightVars.m_bLightmappedGeneric = true;
			flashlightVars.m_bWorldVertexTransition = false;
			// int nWorldVertexTransitionPassID = 0
			flashlightVars.m_nBaseTexture2Var = -1;
			flashlightVars.m_nBaseTexture2FrameVar = -1;
			flashlightVars.m_bTeeth = false;
			flashlightVars.m_nTeethForwardVar = 0;
			flashlightVars.m_nTeethIllumFactorVar = 0;

			DrawFlashlight_dx90( params, pShaderAPI, pShaderShadow, flashlightVars );
		}
		else
		{
			DrawDecal( params, pShaderAPI, pShaderShadow );
		}
	}

	SHADER_DRAW
	{
		bool bUsingFlashlight = UsingFlashlight( params );
		if ( bUsingFlashlight && ( false || r_flashlight_version2.GetInt() ) )
		{
			DrawPass( params, pShaderAPI, pShaderShadow, false );
			if ( pShaderShadow )
			{
				SetInitialShadowState( );
			}
		}
		DrawPass(  params, pShaderAPI, pShaderShadow, bUsingFlashlight );
	}

END_SHADER
