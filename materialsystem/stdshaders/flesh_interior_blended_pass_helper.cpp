//========= Copyright Valve Corporation, All rights reserved. ============//

/* Example how to plug this into an existing shader:

		In the VMT:
			// Flesh Interior Pass
			"$FleshInteriorEnabled"      "1" // Enables effect
			"$FleshInteriorTexture"      "models/Alyx/alyx_flesh_color" // Mask in alpha
			"$FleshNormalTexture"		 "models/Alyx/alyx_flesh_normal"
			"$FleshBorderTexture1D"      "models/Alyx/alyx_flesh_border"
			"$FleshInteriorNoiseTexture" "Engine/noise-blur-256x256"
			"$FleshSubsurfaceTexture"	 "models/Alyx/alyx_flesh_subsurface"
			"$FleshBorderNoiseScale"     "1.5" // Flesh Noise UV scalar for border
			"$FleshBorderWidth"			 "0.3" // Width of flesh border
			"$FleshBorderSoftness"		 "0.42" // Border softness must be greater than 0.0 and up tp 0.5
			"$FleshBorderTint"			 "[1 1 1]" // Tint / brighten the border 1D texture
			"$FleshGlossBrightness"		 "0.66" // Change the brightness of the glossy layer
			"$FleshDebugForceFleshOn"	 "0" // DEBUG: This will force on full flesh for testing
			"$FleshScrollSpeed"			 "1.0"
			"Proxies"
			{
				"FleshInterior"
				{
				}
			}

		#include "flesh_interior_blended_pass_helper.h"

		In BEGIN_SHADER_PARAMS:
			// Flesh Interior Pass
			SHADER_PARAM( FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass" )
			SHADER_PARAM( FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture" )
			SHADER_PARAM( FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture" )
			SHADER_PARAM( FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture" )
			SHADER_PARAM( FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture" )
			SHADER_PARAM( FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture" )
			SHADER_PARAM( FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture" )
			SHADER_PARAM( FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border" )
			SHADER_PARAM( FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh" )
			SHADER_PARAM( FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
			SHADER_PARAM( FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
			SHADER_PARAM( FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
			SHADER_PARAM( FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
			SHADER_PARAM( FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color" )
			SHADER_PARAM( FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border" )
			SHADER_PARAM( FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)" )
			SHADER_PARAM( FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color" )
			SHADER_PARAM( FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity" )
			SHADER_PARAM( FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness" )
			SHADER_PARAM( FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed" )

		Add this above SHADER_INIT_PARAMS()
			// Flesh Interior Pass
			void SetupVarsFleshInteriorBlendedPass( FleshInteriorBlendedPassVars_t &info )
			{
				info.m_nFleshTexture = FLESHINTERIORTEXTURE;
				info.m_nFleshNoiseTexture = FLESHINTERIORNOISETEXTURE;
				info.m_nFleshBorderTexture1D = FLESHBORDERTEXTURE1D;
				info.m_nFleshNormalTexture = FLESHNORMALTEXTURE;
				info.m_nFleshSubsurfaceTexture = FLESHSUBSURFACETEXTURE;
				info.m_nFleshCubeTexture = FLESHCUBETEXTURE;

				info.m_nflBorderNoiseScale = FLESHBORDERNOISESCALE;
				info.m_nflDebugForceFleshOn = FLESHDEBUGFORCEFLESHON;
				info.m_nvEffectCenterRadius1 = FLESHEFFECTCENTERRADIUS1;
				info.m_nvEffectCenterRadius2 = FLESHEFFECTCENTERRADIUS2;
				info.m_nvEffectCenterRadius3 = FLESHEFFECTCENTERRADIUS3;
				info.m_nvEffectCenterRadius4 = FLESHEFFECTCENTERRADIUS4;

				info.m_ncSubsurfaceTint = FLESHSUBSURFACETINT;
				info.m_nflBorderWidth = FLESHBORDERWIDTH;
				info.m_nflBorderSoftness = FLESHBORDERSOFTNESS;
				info.m_ncBorderTint = FLESHBORDERTINT;
				info.m_nflGlobalOpacity = FLESHGLOBALOPACITY;
				info.m_nflGlossBrightness = FLESHGLOSSBRIGHTNESS;
				info.m_nflScrollSpeed = FLESHSCROLLSPEED;
			}

		In SHADER_INIT_PARAMS()
			// Flesh Interior Pass
			if ( !params[FLESHINTERIORENABLED]->IsDefined() )
			{
				params[FLESHINTERIORENABLED]->SetIntValue( 0 );
			}
			else if ( params[FLESHINTERIORENABLED]->GetIntValue() )
			{
				FleshInteriorBlendedPassVars_t info;
				SetupVarsFleshInteriorBlendedPass( info );
				InitParamsFleshInteriorBlendedPass( this, params, pMaterialName, info );
			}

		In SHADER_INIT
			// Flesh Interior Pass
			if ( params[FLESHINTERIORENABLED]->GetIntValue() )
			{
				FleshInteriorBlendedPassVars_t info;
				SetupVarsFleshInteriorBlendedPass( info );
				InitFleshInteriorBlendedPass( this, params, info );
			}

		At the very end of SHADER_DRAW
			// Flesh Interior Pass
			if ( params[FLESHINTERIORENABLED]->GetIntValue() )
			{
				// If ( snapshotting ) or ( we need to draw this frame )
				if ( ( pShaderShadow != NULL ) || ( true ) )
				{
					FleshInteriorBlendedPassVars_t info;
					SetupVarsFleshInteriorBlendedPass( info );
					DrawFleshInteriorBlendedPass( this, params, pShaderAPI, pShaderShadow, info );
				}
				else // We're not snapshotting and we don't need to draw this frame
				{
					// Skip this pass!
					Draw( false );
				}
			}

==================================================================================================== */

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "convar.h"
#include "flesh_interior_blended_pass_helper.h"

// Auto generated inc files
#include "flesh_interior_blended_pass_vs20.inc"
#include "flesh_interior_blended_pass_ps20.inc"
#include "flesh_interior_blended_pass_ps20b.inc"

void InitParamsFleshInteriorBlendedPass( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, FleshInteriorBlendedPassVars_t &info )
{
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );

	SET_PARAM_STRING_IF_NOT_DEFINED( info.m_nFleshCubeTexture, "env_cubemap" ); // Default to in-game env map
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nflBorderNoiseScale, kDefaultBorderNoiseScale );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nflDebugForceFleshOn, kDefaultDebugForceFleshOn );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nvEffectCenterRadius1, kDefaultEffectCenterRadius, 4 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nvEffectCenterRadius2, kDefaultEffectCenterRadius, 4 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nvEffectCenterRadius3, kDefaultEffectCenterRadius, 4 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_nvEffectCenterRadius4, kDefaultEffectCenterRadius, 4 );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_ncSubsurfaceTint, kDefaultSubsurfaceTint, 4 );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nflBorderWidth, kDefaultBorderWidth );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nflBorderSoftness, kDefaultBorderSoftness );
	SET_PARAM_VEC_IF_NOT_DEFINED( info.m_ncBorderTint, kDefaultBorderTint, 4 );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nflGlobalOpacity, kDefaultGlobalOpacity );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nflGlossBrightness, kDefaultGlossBrightness );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nflScrollSpeed, kDefaultScrollSpeed );
	SET_PARAM_FLOAT_IF_NOT_DEFINED( info.m_nTime, 0.0f );
}

void InitFleshInteriorBlendedPass( CBaseVSShader *pShader, IMaterialVar** params, FleshInteriorBlendedPassVars_t &info )
{
	// Load textures
	pShader->LoadTexture( info.m_nFleshTexture, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nFleshNoiseTexture );
	pShader->LoadTexture( info.m_nFleshBorderTexture1D, TEXTUREFLAGS_SRGB );
	pShader->LoadTexture( info.m_nFleshNormalTexture );
	pShader->LoadTexture( info.m_nFleshSubsurfaceTexture, TEXTUREFLAGS_SRGB );
	pShader->LoadCubeMap( info.m_nFleshCubeTexture, TEXTUREFLAGS_SRGB );
}

void DrawFleshInteriorBlendedPass( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
								  IShaderShadow* pShaderShadow, FleshInteriorBlendedPassVars_t &info, VertexCompressionType_t vertexCompression )
{
	SHADOW_STATE
	{
		// Reset shadow state manually since we're drawing from two materials
		pShader->SetInitialShadowState();

		// Set stream format (note that this shader supports compression)
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

		// Vertex Shader
		DECLARE_STATIC_VERTEX_SHADER( flesh_interior_blended_pass_vs20 );
		SET_STATIC_VERTEX_SHADER_COMBO( HALFLAMBERT, IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT ) );
		SET_STATIC_VERTEX_SHADER_COMBO( USE_STATIC_CONTROL_FLOW, bUseStaticControlFlow );
		SET_STATIC_VERTEX_SHADER( flesh_interior_blended_pass_vs20 );

		// Pixel Shader
		if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
		{
			DECLARE_STATIC_PIXEL_SHADER( flesh_interior_blended_pass_ps20b );
			SET_STATIC_PIXEL_SHADER( flesh_interior_blended_pass_ps20b );
		}
		else
		{
			DECLARE_STATIC_PIXEL_SHADER( flesh_interior_blended_pass_ps20 );
			SET_STATIC_PIXEL_SHADER( flesh_interior_blended_pass_ps20 );
		}

		// Textures
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false ); // Noise texture not sRGB
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, false ); // Normal texture not sRGB
		pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER5, true );
		pShaderShadow->EnableSRGBWrite( true );

		// Blending
		pShader->EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		pShaderShadow->EnableAlphaTest( true );
		pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.0f );
	}
	pShader->Draw();
}
