//========= Copyright Valve Corporation, All rights reserved. ============//
#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "portal_refract_helper.h"
#include "convar.h"

// Auto generated inc files
#include "portal_refract_vs20.inc"
#include "portal_refract_ps20.inc"
#include "portal_refract_ps20b.inc"

void InitParamsPortalRefract( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, PortalRefractVars_t &info )
{
	// Set material flags
	SET_FLAGS( MATERIAL_VAR_MODEL );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );

	SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE );

	// Set material parameter default values
	if ( ( info.m_nStage != -1 ) && ( !params[info.m_nStage]->IsDefined() ) )
	{
		params[info.m_nStage]->SetIntValue( 0 );
	}

	if ( ( info.m_nPortalOpenAmount != -1 ) && ( !params[info.m_nPortalOpenAmount]->IsDefined() ) )
	{
		params[info.m_nPortalOpenAmount]->SetFloatValue( kDefaultPortalOpenAmount );
	}

	if ( ( info.m_nPortalStatic != -1 ) && ( !params[info.m_nPortalStatic]->IsDefined() ) )
	{
		params[info.m_nPortalStatic]->SetFloatValue( kDefaultPortalStatic );
	}

	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nTime, 0.0f );
}

void InitPortalRefract( CBaseVSShader *pShader, IMaterialVar** params, PortalRefractVars_t &info )
{
	int nStage = IS_PARAM_DEFINED( info.m_nStage ) ? params[info.m_nStage]->GetIntValue() : 0;

	// Load textures
	if ( nStage == 2 ) // Only load textures for third stage
	{
		if ( (info.m_nPortalMaskTexture != -1) && params[info.m_nPortalMaskTexture]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nPortalMaskTexture );
		}

		if ( (info.m_nPortalColorTexture != -1) && params[info.m_nPortalColorTexture]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nPortalColorTexture, TEXTUREFLAGS_SRGB );
		}
	}
}

void DrawPortalRefract( CBaseVSShader *pShader, IMaterialVar** params, 
						   IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, PortalRefractVars_t &info )
{
	int nStage = IS_PARAM_DEFINED( info.m_nStage ) ? params[info.m_nStage]->GetIntValue() : 0;
		
	SHADOW_STATE
	{
		// Set stream format
		pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_NORMAL, 1, NULL, 4 );

		// Vertex Shader
		DECLARE_STATIC_VERTEX_SHADER( portal_refract_vs20 );
		SET_STATIC_VERTEX_SHADER_COMBO( STAGE, nStage );
		SET_STATIC_VERTEX_SHADER( portal_refract_vs20 );

		// On Leopard / 10.5.8, we can't do an sRGB read from a render target, so we must fake it in shader code
		bool bShaderSRGBRead = IsOSX() && !g_pHardwareConfig->CanDoSRGBReadFromRTs();
		
		// Pixel Shader
		if( g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders() ) // Always send OpenGL / Posix down this path
		{
			DECLARE_STATIC_PIXEL_SHADER( portal_refract_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( STAGE, nStage );
			SET_STATIC_PIXEL_SHADER_COMBO( SHADER_SRGB_READ, bShaderSRGBRead ? 1 : 0 );
			SET_STATIC_PIXEL_SHADER( portal_refract_ps20b );
		}
		else
		{
			DECLARE_STATIC_PIXEL_SHADER( portal_refract_ps20 );
			SET_STATIC_PIXEL_SHADER_COMBO( STAGE, nStage );
			SET_STATIC_PIXEL_SHADER( portal_refract_ps20 );
		}

		// Textures
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); // Refraction texture
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true ); // Noise
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false ); // Not sRGB
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true ); // Color
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true ); // sRGB
		pShaderShadow->EnableSRGBWrite( true ); //Stage 1 doesn't actually need srgb conversion, but it skips the software srgb solution anyway.

		// Enable alpha testing for all stages
		pShaderShadow->EnableAlphaTest( true );
		pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.5f );

		// Enable alpha blending for stage 2
		if ( nStage == 2 )
		{
			pShader->EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 1.0f/255.0f );
		}

		// Disable z-writes for all passes
		pShaderShadow->EnableDepthWrites( false );

		// Disable alpha-writes for all passes
		pShaderShadow->EnableAlphaWrites( false );
	}
	pShader->Draw();
}
