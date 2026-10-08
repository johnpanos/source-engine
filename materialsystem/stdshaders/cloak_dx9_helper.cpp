//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"
#include "cloak_dx9_helper.h"
#include "convar.h"


static ConVar r_lightwarpidentity( "r_lightwarpidentity", "0", FCVAR_CHEAT );

// FIXME: doesn't support fresnel!
void InitParamsCloak_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, Cloak_DX9_Vars_t &info )
{
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );

	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	if( !params[info.m_nFresnelReflection]->IsDefined() )
	{
		params[info.m_nFresnelReflection]->SetFloatValue( 1.0f );
	}
	if( !params[info.m_nMasked]->IsDefined() )
	{
		params[info.m_nMasked]->SetIntValue( 0 );
	}
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE );
}

void InitCloak_DX9( CBaseVSShader *pShader, IMaterialVar** params, Cloak_DX9_Vars_t &info )
{
	if (params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
	}

	if (params[info.m_nNormalMap]->IsDefined() )
	{
		pShader->LoadBumpMap( info.m_nNormalMap );
	}

	if ( (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDiffuseWarpTexture );
	}

	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
}

void DrawCloak_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
				    IShaderShadow* pShaderShadow, Cloak_DX9_Vars_t &info, VertexCompressionType_t vertexCompression )
{
	bool bIsModel = IS_FLAG_SET( MATERIAL_VAR_MODEL );
	bool bMasked = (params[info.m_nMasked]->GetIntValue() != 0);
	bool hasDiffuseWarp = (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsTexture();
	bool hasPhongExponentTexture = (info.m_nPhongExponentTexture != -1) && params[info.m_nPhongExponentTexture]->IsTexture();
	bool hasPhongTintMap = hasPhongExponentTexture && (info.m_nPhongAlbedoTint != -1) && ( params[info.m_nPhongAlbedoTint]->GetIntValue() != 0 );
	bool bHasRimLight = (info.m_nRimLight != -1) && ( params[info.m_nRimLight]->GetIntValue() != 0 );
	bool bHasRimMaskMap = hasPhongExponentTexture && bHasRimLight && (info.m_nRimMask != -1) && ( params[info.m_nRimMask]->GetIntValue() != 0 );

	SHADOW_STATE
	{
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

		pShader->SetInitialShadowState( );

		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );	// Always SRGB read on base map
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true );	// Refraction map sampler...

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );

		pShaderShadow->EnableSRGBWrite( true );

		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		if( bIsModel )
		{
			userDataSize = 4;
		}
		else
		{
			flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
		}

		// This shader supports compressed vertices, so OR in that flag:
		flags |= VERTEX_FORMAT_COMPRESSED;

		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();


			// Bind ps_2_b shader so we can get Phong terms
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


			// Bind ps_2_b shader so we can get Phong terms
		}

		pShader->DefaultFog();

		if( bMasked )
		{
			pShader->EnableAlphaBlending( SHADER_BLEND_ONE_MINUS_SRC_ALPHA, SHADER_BLEND_SRC_ALPHA );
		}
	}
	pShader->Draw();
}
