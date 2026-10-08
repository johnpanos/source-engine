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
			// On DX9, get the gamma read and write correct
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );			// Base
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );			// White
			pShaderShadow->EnableSRGBWrite( true );
		}
		else
		{

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


