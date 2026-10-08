//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "BaseVSShader.h"
#include "tier1/convar.h"
#include "mathlib/vmatrix.h"
#include "eyes_dx8_dx9_helper.h"
#include "cpp_shader_constant_register_map.h"
#include "Eyes.inc"
#include "eyes_flashlight_vs11.inc"
#include "eyes_flashlight_ps11.inc"


#include "eyes_vs20.inc"
#include "eyes_ps20.inc"
#include "eyes_ps20b.inc"
#include "eyes_flashlight_vs20.inc"
#include "eyes_flashlight_ps20.inc"
#include "eyes_flashlight_ps20b.inc"

#include "eyes_vs30.inc"
#include "eyes_ps30.inc"
#include "eyes_flashlight_vs30.inc"
#include "eyes_flashlight_ps30.inc"


ConVar r_flashlight_version2( "r_flashlight_version2", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

void InitParamsEyes_DX8_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, 
							Eyes_DX8_DX9_Vars_t &info )
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
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	Assert( info.m_nIntro != -1 );
	if( info.m_nIntro != -1 && !params[info.m_nIntro]->IsDefined() )
	{
		params[info.m_nIntro]->SetIntValue( 0 );
	}
}

void InitEyes_DX8_DX9( CBaseVSShader *pShader, IMaterialVar** params, Eyes_DX8_DX9_Vars_t &info )
{
	pShader->LoadTexture( FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nIris, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nGlint );

	// Be sure dilation is zeroed if undefined
	if( !params[info.m_nDilation]->IsDefined() )
	{
		params[info.m_nDilation]->SetFloatValue( 0.0f );
	}
}



static void DrawFlashlight( bool bDX9, CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, 
						   IShaderShadow* pShaderShadow, Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	if( pShaderShadow )
	{
		pShaderShadow->EnableDepthWrites( false );

		pShader->EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );	// Write over the eyes that were already there 

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );			// Spot
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );			// Base
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );			// Normalizing cubemap
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );			// Iris

		// Set stream format (note that this shader supports compression)
		int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		// Be sure not to write to dest alpha
		pShaderShadow->EnableAlphaWrites( false );

		if ( bDX9 )
		{
			int nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
			if ( !g_pHardwareConfig->HasFastVertexTextures() )
			{
				DECLARE_STATIC_VERTEX_SHADER( eyes_flashlight_vs20 );
				SET_STATIC_VERTEX_SHADER( eyes_flashlight_vs20 );

				if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				{
					DECLARE_STATIC_PIXEL_SHADER( eyes_flashlight_ps20b );
					SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
					SET_STATIC_PIXEL_SHADER( eyes_flashlight_ps20b );
				}
				else
				{
					DECLARE_STATIC_PIXEL_SHADER( eyes_flashlight_ps20 );
					SET_STATIC_PIXEL_SHADER( eyes_flashlight_ps20 );
				}
			}
			else
			{
				// The vertex shader uses the vertex id stream
				SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );

				DECLARE_STATIC_VERTEX_SHADER( eyes_flashlight_vs30 );
				SET_STATIC_VERTEX_SHADER( eyes_flashlight_vs30 );

				DECLARE_STATIC_PIXEL_SHADER( eyes_flashlight_ps30 );
				SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
				SET_STATIC_PIXEL_SHADER( eyes_flashlight_ps30 );
			}

			// On DX9, get the gamma read and write correct
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );			// Spot
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );			// Base
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, true );			// Iris
			pShaderShadow->EnableSRGBWrite( true );

			if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );			// Shadow depth map
				pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER4 );
				pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );			// Shadow noise rotation map
			}
		}
		else
		{
			// DX8 uses old asm shaders
			eyes_flashlight_vs11_Static_Index	vshIndex;
			pShaderShadow->SetVertexShader( "eyes_flashlight_vs11", vshIndex.GetIndex() );

			eyes_flashlight_ps11_Static_Index	pshIndex;
			pShaderShadow->SetPixelShader( "eyes_flashlight_ps11", pshIndex.GetIndex() );
		}
		
		pShader->FogToBlack();
	}
	pShader->Draw();
}

static void DrawUsingVertexShader( bool bDX9, CBaseVSShader *pShader, IMaterialVar** params, 
								  IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
								  Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	SHADOW_STATE
	{
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );	// Base
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );	// Iris
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );	// Glint

		// Set stream format (note that this shader supports compression)
		int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		pShaderShadow->EnableAlphaWrites( true ); //we end up hijacking destination alpha for opaques most of the time.
		
		if ( bDX9 )
		{
			if ( !g_pHardwareConfig->HasFastVertexTextures() )
			{
				bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

				DECLARE_STATIC_VERTEX_SHADER( eyes_vs20 );
				SET_STATIC_VERTEX_SHADER_COMBO( HALFLAMBERT, IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT ) );
				SET_STATIC_VERTEX_SHADER_COMBO( INTRO, params[info.m_nIntro]->GetIntValue() ? 1 : 0 );
				SET_STATIC_VERTEX_SHADER_COMBO( USE_STATIC_CONTROL_FLOW, bUseStaticControlFlow );
				SET_STATIC_VERTEX_SHADER( eyes_vs20 );

				if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				{
					DECLARE_STATIC_PIXEL_SHADER( eyes_ps20b );
					SET_STATIC_PIXEL_SHADER( eyes_ps20b );
				}
				else
				{
					DECLARE_STATIC_PIXEL_SHADER( eyes_ps20 );
					SET_STATIC_PIXEL_SHADER( eyes_ps20 );
				}
			}
			else
			{
				// The vertex shader uses the vertex id stream
				SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );

				DECLARE_STATIC_VERTEX_SHADER( eyes_vs30 );
				SET_STATIC_VERTEX_SHADER_COMBO( HALFLAMBERT, IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT ) );
				SET_STATIC_VERTEX_SHADER_COMBO( INTRO, params[info.m_nIntro]->GetIntValue() ? 1 : 0 );
				SET_STATIC_VERTEX_SHADER( eyes_vs30 );

				DECLARE_STATIC_PIXEL_SHADER( eyes_ps30 );
				SET_STATIC_PIXEL_SHADER( eyes_ps30 );
			}
			// On DX9, get the gamma read and write correct
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );			// Base
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );			// White
			pShaderShadow->EnableSRGBWrite( true );
		}
		else
		{
			eyes_Static_Index vshIndex;
			vshIndex.SetHALF_LAMBERT( IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT ) );
			pShaderShadow->SetVertexShader( "Eyes", vshIndex.GetIndex() );

			pShaderShadow->SetPixelShader( "Eyes_Overbright2" );
		}

		pShader->FogToFogColor();
	}
	pShader->Draw();
}

static void DrawEyes_DX8_DX9_Internal( bool bDX9, CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
	IShaderShadow* pShaderShadow, bool bHasFlashlight, Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	if( !bHasFlashlight )
	{
		DrawUsingVertexShader( bDX9, pShader, params, pShaderAPI, pShaderShadow, info, vertexCompression );
	}
	else
	{
		DrawFlashlight( bDX9, pShader, params, pShaderAPI, pShaderShadow, info, vertexCompression );
	}
}

extern ConVar r_flashlight_version2;
void DrawEyes_DX8_DX9( bool bDX9, CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
					  IShaderShadow* pShaderShadow, Eyes_DX8_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	SHADOW_STATE
	{
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
	}
	bool bHasFlashlight = pShader->UsingFlashlight( params );
	if ( bHasFlashlight && ( false || r_flashlight_version2.GetInt() ) )
	{
		DrawEyes_DX8_DX9_Internal( bDX9, pShader, params, pShaderAPI, pShaderShadow, false, info, vertexCompression );
		if ( pShaderShadow )
		{
			pShader->SetInitialShadowState( );
		}
	}
	DrawEyes_DX8_DX9_Internal( bDX9, pShader, params, pShaderAPI, pShaderShadow, bHasFlashlight, info, vertexCompression );
}


