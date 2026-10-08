//========= Copyright Valve Corporation, All rights reserved. ============//

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "eye_refract_helper.h"

#include "cpp_shader_constant_register_map.h"




#include "convar.h"

static ConVar r_lightwarpidentity( "r_lightwarpidentity","0", FCVAR_CHEAT );

void InitParams_Eyes_Refract( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, Eye_Refract_Vars_t &info )
{
	// FLASHLIGHTFIXME

	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	}


	// Set material flags
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	// Set material parameter default values
	if ( ( info.m_nIntro >= 0 ) && ( !params[info.m_nIntro]->IsDefined() ) )
	{
		params[info.m_nIntro]->SetIntValue( kDefaultIntro );
	}

	if ( ( info.m_nDilation >= 0 ) && ( !params[info.m_nDilation]->IsDefined() ) )
	{
		params[info.m_nDilation]->SetFloatValue( kDefaultDilation );
	}

	if ( ( info.m_nGlossiness >= 0 ) && ( !params[info.m_nGlossiness]->IsDefined() ) )
	{
		params[info.m_nGlossiness]->SetFloatValue( kDefaultGlossiness );
	}

	if ( ( info.m_nSphereTexKillCombo >= 0 ) && ( !params[info.m_nSphereTexKillCombo]->IsDefined() ) )
	{
		params[info.m_nSphereTexKillCombo]->SetIntValue( kDefaultSphereTexKillCombo );
	}

	if ( ( info.m_nRaytraceSphere >= 0 ) && ( !params[info.m_nRaytraceSphere]->IsDefined() ) )
	{
		params[info.m_nRaytraceSphere]->SetIntValue( kDefaultRaytraceSphere );
	}

	if ( ( info.m_nAmbientOcclColor >= 0 ) && ( !params[info.m_nAmbientOcclColor]->IsDefined() ) )
	{
		params[info.m_nAmbientOcclColor]->SetVecValue( kDefaultAmbientOcclColor, 4 );
	}

	if ( ( info.m_nEyeballRadius >= 0 ) && ( !params[info.m_nEyeballRadius]->IsDefined() ) )
	{
		params[info.m_nEyeballRadius]->SetFloatValue( kDefaultEyeballRadius );
	}

	if ( ( info.m_nParallaxStrength >= 0 ) && ( !params[info.m_nParallaxStrength]->IsDefined() ) )
	{
		params[info.m_nParallaxStrength]->SetFloatValue( kDefaultParallaxStrength );
	}

	if ( ( info.m_nCorneaBumpStrength >= 0 ) && ( !params[info.m_nCorneaBumpStrength]->IsDefined() ) )
	{
		params[info.m_nCorneaBumpStrength]->SetFloatValue( kDefaultCorneaBumpStrength );
	}
}

void Init_Eyes_Refract( CBaseVSShader *pShader, IMaterialVar** params, Eye_Refract_Vars_t &info )
{
	pShader->LoadTexture( info.m_nCorneaTexture );								// SHADER_SAMPLER0  (this is a normal, hence not sRGB)
	pShader->LoadTexture( info.m_nIris, TEXTUREFLAGS_SRGB );					// SHADER_SAMPLER1
	pShader->LoadCubeMap( info.m_nEnvmap, TEXTUREFLAGS_SRGB );					// SHADER_SAMPLER2
	pShader->LoadTexture( info.m_nAmbientOcclTexture, TEXTUREFLAGS_SRGB );		// SHADER_SAMPLER3

	if ( IS_PARAM_DEFINED( info.m_nDiffuseWarpTexture ) )
	{
		pShader->LoadTexture( info.m_nDiffuseWarpTexture );						// SHADER_SAMPLER4
	}

	pShader->LoadTexture( FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB );				// SHADER_SAMPLER5
}

void Draw_Eyes_Refract_Internal( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
	IShaderShadow* pShaderShadow, bool bDrawFlashlightAdditivePass, Eye_Refract_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	bool bDiffuseWarp = IS_PARAM_DEFINED( info.m_nDiffuseWarpTexture );
	bool bIntro = IS_PARAM_DEFINED( info.m_nIntro ) ? ( params[info.m_nIntro]->GetIntValue() ? true : false ) : false;

	SHADOW_STATE
	{
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );	// Cornea normal
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );	// Iris
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );	// Cube reflection
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );	// Ambient occlusion

		// Set stream format (note that this shader supports compression)
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		if ( bDiffuseWarp )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );	// Light warp
		}

		int nShadowFilterMode = 0;
		if ( bDrawFlashlightAdditivePass == true )
		{
			if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
			}

			pShaderShadow->EnableDepthWrites( false );
			pShader->EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE ); // Write over the eyes that were already there 
			pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );	// Flashlight cookie
		}

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{

			if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				bool bSphereTexKillCombo = IS_PARAM_DEFINED( info.m_nSphereTexKillCombo ) ? ( params[info.m_nSphereTexKillCombo]->GetIntValue() ? true : false ) : ( kDefaultSphereTexKillCombo ? true : false );
				bool bRayTraceSphere = IS_PARAM_DEFINED( info.m_nRaytraceSphere ) ? ( params[info.m_nRaytraceSphere]->GetIntValue() ? true : false ) : ( kDefaultRaytraceSphere ? true : false );


				if ( bDrawFlashlightAdditivePass == true )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );	// Shadow depth map
					pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER6 );
					pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );	// Noise map
				}
			}
			else
			{
			}
		}
		else
		{
			// The vertex shader uses the vertex id stream
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );


			bool bSphereTexKillCombo = IS_PARAM_DEFINED( info.m_nSphereTexKillCombo ) ? ( params[info.m_nSphereTexKillCombo]->GetIntValue() ? true : false ) : ( kDefaultSphereTexKillCombo ? true : false );
			bool bRayTraceSphere = IS_PARAM_DEFINED( info.m_nRaytraceSphere ) ? ( params[info.m_nRaytraceSphere]->GetIntValue() ? true : false ) : ( kDefaultRaytraceSphere ? true : false );


			if ( bDrawFlashlightAdditivePass == true )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );	// Shadow depth map
				pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );	// Noise map
			}
		}

		// On DX9, get the gamma read and write correct
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );			// Iris
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true );			// Cube map reflection
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, true );			// Ambient occlusion
		pShaderShadow->EnableSRGBWrite( true );

		if ( bDrawFlashlightAdditivePass == true )
		{
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER5, true );		// Flashlight cookie
		}

		// Fog
		if ( bDrawFlashlightAdditivePass == true )
		{
			pShader->FogToBlack();
		}
		else
		{
			pShader->FogToFogColor();
		}
	}
	pShader->Draw();
}


extern ConVar r_flashlight_version2;
void Draw_Eyes_Refract( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
	IShaderShadow* pShaderShadow, Eye_Refract_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	bool bHasFlashlight = pShader->UsingFlashlight( params );
	if ( bHasFlashlight && ( false || r_flashlight_version2.GetInt() ) )
	{
		Draw_Eyes_Refract_Internal( pShader, params, pShaderAPI, pShaderShadow, false, info, vertexCompression );
		if ( pShaderShadow )
		{
			pShader->SetInitialShadowState( );
		}
	}
	Draw_Eyes_Refract_Internal( pShader, params, pShaderAPI, pShaderShadow, bHasFlashlight, info, vertexCompression );
}
