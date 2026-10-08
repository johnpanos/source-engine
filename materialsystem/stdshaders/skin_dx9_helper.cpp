//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "BaseVSShader.h"
#include "skin_dx9_helper.h"
#include "convar.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT );
static ConVar r_lightwarpidentity( "r_lightwarpidentity", "0", FCVAR_CHEAT );
static ConVar r_rimlight( "r_rimlight", "1", FCVAR_CHEAT );

// Textures may be bound to the following samplers:
//	SHADER_SAMPLER0	 Base (Albedo) / Gloss in alpha
//	SHADER_SAMPLER1	 Specular warp (including iridescence)
//	SHADER_SAMPLER2	 Diffuse Lighting warp texture
//	SHADER_SAMPLER3	 Normal Map
//	SHADER_SAMPLER4	 Flashlight Shadow Depth Map
//	SHADER_SAMPLER5	 Normalization cube map
//	SHADER_SAMPLER6	 Flashlight Cookie
//	SHADER_SAMPLER7	 Specular exponent
//	SHADER_SAMPLER8	 Cubic environment map
//  SHADER_SAMPLER9  Compressed wrinklemap
//  SHADER_SAMPLER10 Stretched wrinklemap
//  SHADER_SAMPLER11 Compressed wrinkle normal map
//  SHADER_SAMPLER12 Stretched wrinkle normal map
//  SHADER_SAMPLER13 Detail texture


//-----------------------------------------------------------------------------
// Initialize shader parameters
//-----------------------------------------------------------------------------
void InitParamsSkin_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, VertexLitGeneric_DX9_Vars_t &info )
{	
	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	Assert( info.m_nFlashlightTexture >= 0 );

	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	}
	
	// Write over $basetexture with $info.m_nBumpmap if we are going to be using diffuse normal mapping.
	if( info.m_nAlbedo != -1 && g_pConfig->UseBumpmapping() && info.m_nBumpmap != -1 && params[info.m_nBumpmap]->IsDefined() && params[info.m_nAlbedo]->IsDefined() &&
		params[info.m_nBaseTexture]->IsDefined() )
	{
		params[info.m_nBaseTexture]->SetStringValue( params[info.m_nAlbedo]->GetStringValue() );
	}

	// This shader can be used with hw skinning
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	// No texture means no env mask in base alpha
	if ( !params[info.m_nBaseTexture]->IsDefined() )
	{
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
	}

	// Lots of reasons to want tangent space, since we bind a flat normal map in many cases where we don't have a bump map
	bool bBump = (info.m_nBumpmap != -1) && g_pConfig->UseBumpmapping() && params[info.m_nBumpmap]->IsDefined();
	bool bEnvMap = (info.m_nEnvmap != -1) && params[info.m_nEnvmap]->IsDefined();
	bool bDiffuseWarp = (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined();
	bool bPhong = (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined();
	if( bBump || bEnvMap || bDiffuseWarp || bPhong )
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	}
	else
	{
		CLEAR_FLAGS( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	}

	if ( ( info.m_nSelfIllumFresnel != -1 ) && ( !params[info.m_nSelfIllumFresnel]->IsDefined() ) )
	{
		params[info.m_nSelfIllumFresnel]->SetIntValue( 0 );
	}

	if ( ( info.m_nSelfIllumFresnelMinMaxExp != -1 ) && ( !params[info.m_nSelfIllumFresnelMinMaxExp]->IsDefined() ) )
	{
		params[info.m_nSelfIllumFresnelMinMaxExp]->SetVecValue( 0.0f, 1.0f, 1.0f );
	}

	if ( ( info.m_nBaseMapAlphaPhongMask != -1 ) && ( !params[info.m_nBaseMapAlphaPhongMask]->IsDefined() ) )
	{
		params[info.m_nBaseMapAlphaPhongMask]->SetIntValue( 0 );
	}

	if ( ( info.m_nEnvmapFresnel != -1 ) && ( !params[info.m_nEnvmapFresnel]->IsDefined() ) )
	{
		params[info.m_nEnvmapFresnel]->SetFloatValue( 0 );
	}
}

//-----------------------------------------------------------------------------
// Initialize shader
//-----------------------------------------------------------------------------
void InitSkin_DX9( CBaseVSShader *pShader, IMaterialVar** params, VertexLitGeneric_DX9_Vars_t &info )
{
	Assert( info.m_nFlashlightTexture >= 0 );
	pShader->LoadTexture( info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB );
	
	bool bIsBaseTextureTranslucent = false;
	if ( params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
		
		if ( params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent() )
		{
			bIsBaseTextureTranslucent = true;
		}

		if ( ( info.m_nWrinkle != -1 ) && ( info.m_nStretch != -1 ) &&
			params[info.m_nWrinkle]->IsDefined() && params[info.m_nStretch]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nWrinkle, TEXTUREFLAGS_SRGB );
			pShader->LoadTexture( info.m_nStretch, TEXTUREFLAGS_SRGB );
		}
	}

	bool bHasSelfIllumMask = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && (info.m_nSelfIllumMask != -1) && params[info.m_nSelfIllumMask]->IsDefined();

	// No alpha channel in any of the textures? No self illum or envmapmask
	if ( !bIsBaseTextureTranslucent )
	{
		bool bHasSelfIllumFresnel = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 );

		// Can still be self illum with no base alpha if using one of these alternate modes
		if ( !bHasSelfIllumFresnel && !bHasSelfIllumMask )
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
		}

		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	if ( (info.m_nPhongExponentTexture != -1) && params[info.m_nPhongExponentTexture]->IsDefined() &&
		 (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nPhongExponentTexture );
	}

	if ( (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined() &&
		 (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDiffuseWarpTexture );
	}

	if ( (info.m_nPhongWarpTexture != -1) && params[info.m_nPhongWarpTexture]->IsDefined() &&
		 (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nPhongWarpTexture );
	}

	if ( info.m_nDetail != -1 && params[info.m_nDetail]->IsDefined() )
	{
		int nDetailBlendMode = ( info.m_nDetailTextureCombineMode == -1 ) ? 0 : params[info.m_nDetailTextureCombineMode]->GetIntValue();
		if ( nDetailBlendMode == 0 ) // Mod2X
			pShader->LoadTexture( info.m_nDetail );
		else
			pShader->LoadTexture( info.m_nDetail, TEXTUREFLAGS_SRGB );
	}

	if ( g_pConfig->UseBumpmapping() )
	{
		if ( (info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsDefined() )
		{
			pShader->LoadBumpMap( info.m_nBumpmap );
			SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );

			if ( ( info.m_nNormalWrinkle != -1 ) && ( info.m_nNormalStretch != -1 ) &&
				params[info.m_nNormalWrinkle]->IsDefined() && params[info.m_nNormalStretch]->IsDefined() )
			{
				pShader->LoadTexture( info.m_nNormalWrinkle );
				pShader->LoadTexture( info.m_nNormalStretch );
			}
		}
	}
	
	if ( params[info.m_nEnvmap]->IsDefined() )
	{
		pShader->LoadCubeMap( info.m_nEnvmap, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0  );
	}

	if ( bHasSelfIllumMask )
	{
		pShader->LoadTexture( info.m_nSelfIllumMask );
	}
}

class CSkin_DX9_Context : public CBasePerMaterialContextData
{
public:
	bool m_bFastPath;

};

//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
void DrawSkin_DX9_Internal( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
	bool bHasFlashlight, VertexLitGeneric_DX9_Vars_t &info, VertexCompressionType_t vertexCompression,
							CBasePerMaterialContextData **pContextDataPtr )
{
	bool bHasBaseTexture = (info.m_nBaseTexture != -1) && params[info.m_nBaseTexture]->IsTexture();
	bool bHasBump = (info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsTexture();

	bool bHasBaseTextureWrinkle = bHasBaseTexture && 
		(info.m_nWrinkle != -1) && params[info.m_nWrinkle]->IsTexture() &&
		(info.m_nStretch != -1) && params[info.m_nStretch]->IsTexture();

	bool bHasBumpWrinkle = bHasBump && 
		(info.m_nNormalWrinkle != -1) && params[info.m_nNormalWrinkle]->IsTexture() &&
		(info.m_nNormalStretch != -1) && params[info.m_nNormalStretch]->IsTexture();

	bool bHasVertexColor = IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
	bool bHasVertexAlpha = IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA );
	bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;
	bool bHasSelfIllum = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) != 0;
	bool bHasSelfIllumFresnel = ( bHasSelfIllum ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 );
	bool bHasSelfIllumMask = ( bHasSelfIllum ) && (info.m_nSelfIllumMask != -1) && params[info.m_nSelfIllumMask]->IsTexture();

	// Tie these to specular
	bool bHasPhong = (info.m_nPhong != -1) && ( params[info.m_nPhong]->GetIntValue() != 0 );
	bool bHasSpecularExponentTexture = (info.m_nPhongExponentTexture != -1) && params[info.m_nPhongExponentTexture]->IsTexture();
	bool bHasPhongTintMap = bHasSpecularExponentTexture && (info.m_nPhongAlbedoTint != -1) && ( params[info.m_nPhongAlbedoTint]->GetIntValue() != 0 );
	bool bHasDiffuseWarp = (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsTexture();
	bool bHasPhongWarp = (info.m_nPhongWarpTexture != -1) && params[info.m_nPhongWarpTexture]->IsTexture();
	bool bHasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );

	bool bIsDecal = IS_FLAG_SET( MATERIAL_VAR_DECAL );

	// Rimlight must be set to non-zero to trigger rim light combo (also requires Phong)
	bool bHasRimLight = r_rimlight.GetBool() && bHasPhong && (info.m_nRimLight != -1) && ( params[info.m_nRimLight]->GetIntValue() != 0 );
	bool bHasRimMaskMap = bHasSpecularExponentTexture && bHasRimLight && (info.m_nRimMask != -1) && ( params[info.m_nRimMask]->GetIntValue() != 0 );

	float fBlendFactor=( info.m_nDetailTextureBlendFactor == -1 )? 1 : params[info.m_nDetailTextureBlendFactor]->GetFloatValue();
	bool hasDetailTexture = ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture();
	int nDetailBlendMode = ( hasDetailTexture && info.m_nDetailTextureCombineMode != -1 ) ? params[info.m_nDetailTextureCombineMode]->GetIntValue() : 0;

	bool bBlendTintByBaseAlpha = IsBoolSet( info.m_nBlendTintByBaseAlpha, params ) && !bHasSelfIllum;	// Pixel shader can't do both BLENDTINTBYBASEALPHA and SELFILLUM, so let selfillum win

	float flTintReplacementAmount = GetFloatParam( info.m_nTintReplacesBaseColor, params );

	BlendType_t nBlendType= pShader->EvaluateBlendRequirements( bBlendTintByBaseAlpha ? -1 : info.m_nBaseTexture, true );

	bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested && !bHasFlashlight; //dest alpha is free for special use

	CSkin_DX9_Context *pContextData = reinterpret_cast< CSkin_DX9_Context *> ( *pContextDataPtr );
	if ( ! pContextData )
	{
		pContextData = new CSkin_DX9_Context;
		*pContextDataPtr = pContextData;
	}

	if( pShader->IsSnapshotting() )
	{
		// look at color and alphamod stuff.
		// Unlit generic never uses the flashlight
		bool bHasEnvmap = !bHasFlashlight && params[info.m_nEnvmap]->IsTexture();
		bool bHasNormal = params[info.m_nBumpmap]->IsTexture();
		bool bCanUseBaseAlphaPhongMaskFastPath = (info.m_nBaseMapAlphaPhongMask != -1) && ( params[info.m_nBaseMapAlphaPhongMask]->GetIntValue() != 0 );

		if ( ! ( params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent() ) )
			bCanUseBaseAlphaPhongMaskFastPath = true;
		
		pContextData->m_bFastPath =
			(! bHasBump ) && 
			(! bHasSpecularExponentTexture ) &&
			(! bHasPhongTintMap ) &&
			(! bHasPhongWarp ) && 
			(! bHasRimLight ) && 
			(! hasDetailTexture ) &&
			bCanUseBaseAlphaPhongMaskFastPath &&
			(! bHasSelfIllum ) &&
			(! bBlendTintByBaseAlpha );
		
		// Alpha test: FIXME: shouldn't this be handled in CBaseVSShader::SetInitialShadowState
		pShaderShadow->EnableAlphaTest( bIsAlphaTested );

		if( info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f )
		{
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue() );
		}

		int nShadowFilterMode = 0;
		if( bHasFlashlight )
		{
			if (params[info.m_nBaseTexture]->IsTexture())
			{
				pShader->SetAdditiveBlendingShadowState( info.m_nBaseTexture, true );
			}

			if( bIsAlphaTested )
			{
				// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to 
				// be the same on both the regular pass and the flashlight pass.
				pShaderShadow->EnableAlphaTest( false );
				pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
			}
			pShaderShadow->EnableBlending( true );
			pShaderShadow->EnableDepthWrites( false );

			// Be sure not to write to dest alpha
			pShaderShadow->EnableAlphaWrites( false );

			nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
		}
		else // not flashlight pass
		{
			if (params[info.m_nBaseTexture]->IsTexture())
			{
				pShader->SetDefaultBlendingShadowState( info.m_nBaseTexture, true );
			}

			if ( bHasEnvmap )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );	// Cubic environment map
				if( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
				{
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER8, true );
				}
			}
		}
		
		unsigned int flags = VERTEX_POSITION;
		if( bHasNormal )
		{
			flags |= VERTEX_NORMAL;
		}

		int userDataSize = 0;

		// Always enable...will bind white if nothing specified...
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );		// Base (albedo) map
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

		if ( bHasBaseTextureWrinkle || bHasBumpWrinkle )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER9, true );	// Base (albedo) compression map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER9, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER10, true );	// Base (albedo) expansion map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER10, true );
		}

		if( bHasDiffuseWarp )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );	// Diffuse warp texture
		}

		if( bHasPhongWarp )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );	// Specular warp texture
		}

		// Specular exponent map or dummy
		pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );	// Specular exponent map

		if( bHasFlashlight )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );	// Shadow depth map
			pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER4 );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, false );
			pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );	// Noise map
			pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );	// Flashlight cookie
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER6, true );
			userDataSize = 4; // tangent S
		}

		// Always enable, since flat normal will be bound
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );		// Normal map
		userDataSize = 4; // tangent S
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );		// Normalizing cube map

		if ( bHasBaseTextureWrinkle || bHasBumpWrinkle )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER11, true );	// Normal compression map
			pShaderShadow->EnableTexture( SHADER_SAMPLER12, true );	// Normal expansion map
		}

		if ( hasDetailTexture )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER13, true );
			if ( nDetailBlendMode != 0 ) //Not Mod2X
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER13, true );
		}

		if ( bHasSelfIllum )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER14, true );
		}

		if( bHasVertexColor || bHasVertexAlpha )
		{
			flags |= VERTEX_COLOR;
		}

		pShaderShadow->EnableSRGBWrite( true );
		
		// texcoord0 : base texcoord, texcoord2 : decal hw morph delta
		int pTexCoordDim[3] = { 2, 0, 3 };
		int nTexCoordCount = 1;

		// Special morphed decal information 
		if ( bIsDecal && g_pHardwareConfig->HasFastVertexTextures() )
		{
			nTexCoordCount = 3;
		}

		// This shader supports compressed vertices, so OR in that flag:
		flags |= VERTEX_FORMAT_COMPRESSED;

		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, pTexCoordDim, userDataSize );


		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();


			// Assume we're only going to get in here if we support 2b
		}
		else
		{
			// The vertex shader uses the vertex id stream
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );


		}

		if( bHasFlashlight )
		{
			pShader->FogToBlack();
		}
		else
		{
			pShader->DefaultFog();
		}

		// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
		pShaderShadow->EnableAlphaWrites( bFullyOpaque );
	}
	pShader->Draw();
}


//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
extern ConVar r_flashlight_version2;
void DrawSkin_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
				   VertexLitGeneric_DX9_Vars_t &info, VertexCompressionType_t vertexCompression,
				   CBasePerMaterialContextData **pContextDataPtr )

{
	bool bHasFlashlight = pShader->UsingFlashlight( params );
	if ( bHasFlashlight && ( false || r_flashlight_version2.GetInt() ) )
	{
		DrawSkin_DX9_Internal( pShader, params, pShaderAPI,
			pShaderShadow, false, info, vertexCompression, pContextDataPtr++ );
		if ( pShaderShadow )
		{
			pShader->SetInitialShadowState( );
		}
	}
	DrawSkin_DX9_Internal( pShader, params, pShaderAPI,
		pShaderShadow, bHasFlashlight, info, vertexCompression, pContextDataPtr );
}
