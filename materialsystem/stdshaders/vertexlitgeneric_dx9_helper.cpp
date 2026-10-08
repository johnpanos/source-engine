//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "BaseVSShader.h"
#include "vertexlitgeneric_dx9_helper.h"
#include "skin_dx9_helper.h"




#include "commandbuilder.h"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright( "mat_fullbright","0", FCVAR_CHEAT );
static ConVar r_lightwarpidentity( "r_lightwarpidentity","0", FCVAR_CHEAT );


static inline bool WantsSkinShader( IMaterialVar** params, const VertexLitGeneric_DX9_Vars_t &info )
{
	if ( info.m_nPhong == -1)								// Don't use skin without Phong
		return false;
	
	if ( params[info.m_nPhong]->GetIntValue() == 0 )		// Don't use skin without Phong turned on
		return false;

	if ( ( info.m_nDiffuseWarpTexture != -1 ) && params[info.m_nDiffuseWarpTexture]->IsTexture() ) // If there's Phong and diffuse warp do skin
		return true;

	if ( ( info.m_nBaseMapAlphaPhongMask != -1 ) && params[info.m_nBaseMapAlphaPhongMask]->GetIntValue() != 1 )
	{
		if ( info.m_nBumpmap == -1 )						// Don't use without a bump map
			return false;

		if ( !params[info.m_nBumpmap]->IsTexture() )		// Don't use if the texture isn't specified
			return false;
	}

	return true;
}

int g_nSnapShots;

//-----------------------------------------------------------------------------
// Initialize shader parameters
//-----------------------------------------------------------------------------
void InitParamsVertexLitGeneric_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, bool bVertexLitGeneric, VertexLitGeneric_DX9_Vars_t &info )
{	
	InitIntParam( info.m_nPhong, params, 0 );

	InitFloatParam( info.m_nAlphaTestReference, params, 0.0f );
	InitIntParam( info.m_nVertexAlphaTest, params, 0 );

	InitIntParam( info.m_nFlashlightNoLambert, params, 0 );

	if ( info.m_nDetailTint != -1 && !params[info.m_nDetailTint]->IsDefined() )
	{
		params[info.m_nDetailTint]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	if ( info.m_nEnvmapTint != -1 && !params[info.m_nEnvmapTint]->IsDefined() )
	{
		params[info.m_nEnvmapTint]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	InitIntParam( info.m_nEnvmapFrame, params, 0 );
	InitIntParam( info.m_nBumpFrame, params, 0 );
	InitFloatParam( info.m_nDetailTextureBlendFactor, params, 1.0 );
	InitIntParam( info.m_nReceiveFlashlight, params, 0 );

	InitFloatParam( info.m_nDetailScale, params, 4.0f );

	if ( (info.m_nBlendTintByBaseAlpha != -1) && (!params[info.m_nBlendTintByBaseAlpha]->IsDefined()) )
	{
		params[info.m_nBlendTintByBaseAlpha]->SetIntValue( 0 );
	}

	InitFloatParam( info.m_nTintReplacesBaseColor, params, 0 );

	if ( (info.m_nSelfIllumTint != -1) && (!params[info.m_nSelfIllumTint]->IsDefined()) )
	{
		params[info.m_nSelfIllumTint]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}


	if ( WantsSkinShader( params, info ) )
	{
		if ( !g_pHardwareConfig->SupportsPixelShaders_2_b() || !g_pConfig->UsePhong() )
		{
			params[info.m_nPhong]->SetIntValue( 0 );
		}
		else
		{
			InitParamsSkin_DX9( pShader, params, pMaterialName, info );
			return;
		}
	}
	
	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	if ( info.m_nFlashlightTexture != -1 )
	{
		if ( g_pHardwareConfig->SupportsBorderColor() )
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
		}
		else
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
		}
	}
	
	// Write over $basetexture with $info.m_nBumpmap if we are going to be using diffuse normal mapping.
	if ( info.m_nAlbedo != -1 && g_pConfig->UseBumpmapping() && info.m_nBumpmap != -1 && params[info.m_nBumpmap]->IsDefined() && params[info.m_nAlbedo]->IsDefined() &&
		params[info.m_nBaseTexture]->IsDefined() )
	{
		params[info.m_nBaseTexture]->SetStringValue( params[info.m_nAlbedo]->GetStringValue() );
	}

	// This shader can be used with hw skinning
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );

	if ( bVertexLitGeneric )
	{
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
	}
	else
	{
		CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
	}

	InitIntParam( info.m_nEnvmapMaskFrame, params, 0 );
	InitFloatParam( info.m_nEnvmapContrast, params, 0.0 );
	InitFloatParam( info.m_nEnvmapSaturation, params, 1.0f );
	InitFloatParam( info.m_nSeamlessScale, params, 0.0 );

	// handle line art parms
	InitFloatParam( info.m_nEdgeSoftnessStart, params, 0.5 );
	InitFloatParam( info.m_nEdgeSoftnessEnd, params, 0.5 );
	InitFloatParam( info.m_nGlowAlpha, params, 1.0 );
	InitFloatParam( info.m_nOutlineAlpha, params, 1.0 );

	// No texture means no self-illum or env mask in base alpha
	if ( info.m_nBaseTexture != -1 && !params[info.m_nBaseTexture]->IsDefined() )
	{
		CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
	}

	if( ( (info.m_nBumpmap != -1) && g_pConfig->UseBumpmapping() && params[info.m_nBumpmap]->IsDefined() ) 
		// we don't need a tangent space if we have envmap without bumpmap
		//		|| ( info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined() ) 
		)
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	}
	else if ( (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined() ) // diffuse warp goes down bump path...
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	}
	else // no tangent space needed
	{
		CLEAR_FLAGS( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	}

	bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	if ( hasNormalMapAlphaEnvmapMask )
	{
		params[info.m_nEnvmapMask]->SetUndefined();
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	if ( IS_FLAG_SET( MATERIAL_VAR_BASEALPHAENVMAPMASK ) && info.m_nBumpmap != -1 && 
		params[info.m_nBumpmap]->IsDefined() && !hasNormalMapAlphaEnvmapMask )
	{
		Warning( "material %s has a normal map and $basealphaenvmapmask.  Must use $normalmapalphaenvmapmask to get specular.\n\n", pMaterialName );
		params[info.m_nEnvmap]->SetUndefined();
	}
	
	if ( info.m_nEnvmapMask != -1 && params[info.m_nEnvmapMask]->IsDefined() && info.m_nBumpmap != -1 && params[info.m_nBumpmap]->IsDefined() )
	{
		params[info.m_nEnvmapMask]->SetUndefined();
		if ( !hasNormalMapAlphaEnvmapMask )
		{
			Warning( "material %s has a normal map and an envmapmask.  Must use $normalmapalphaenvmapmask.\n\n", pMaterialName );
			params[info.m_nEnvmap]->SetUndefined();
		}
	}

	// If mat_specular 0, then get rid of envmap
	if ( !g_pConfig->UseSpecular() && info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined() && params[info.m_nBaseTexture]->IsDefined() )
	{
		params[info.m_nEnvmap]->SetUndefined();
	}

	InitFloatParam( info.m_nHDRColorScale, params, 1.0f );

	InitIntParam( info.m_nLinearWrite, params, 0 );
	InitIntParam( info.m_nGammaColorRead, params, 0 );

	InitIntParam( info.m_nDepthBlend, params, 0 );
	InitFloatParam( info.m_nDepthBlendScale, params, 50.0f );
}


//-----------------------------------------------------------------------------
// Initialize shader
//-----------------------------------------------------------------------------

void InitVertexLitGeneric_DX9( CBaseVSShader *pShader, IMaterialVar** params, bool bVertexLitGeneric, VertexLitGeneric_DX9_Vars_t &info )
{
	// both detailed and bumped = needs skin shader (for now)
	bool bNeedsSkinBecauseOfDetail = false;
	
	//bool bHasBump = ( info.m_nBumpmap != -1 ) && params[info.m_nBumpmap]->IsTexture();
	//if ( bHasBump )
	//{
	//	if (  ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsDefined() )
	//		bNeedsSkinBecauseOfDetail = true;
	//}

	if ( bNeedsSkinBecauseOfDetail || 
		 ( info.m_nPhong != -1 && 
		   params[info.m_nPhong]->GetIntValue() && 
		 g_pHardwareConfig->SupportsPixelShaders_2_b() ) )
	{
		InitSkin_DX9( pShader, params, info );
		return;
	}

	if ( info.m_nFlashlightTexture != -1 )
	{
		pShader->LoadTexture( info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB );
	}
	
	bool bIsBaseTextureTranslucent = false;
	if ( info.m_nBaseTexture != -1 && params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, ( info.m_nGammaColorRead != -1 ) && ( params[info.m_nGammaColorRead]->GetIntValue() == 1 ) ? 0 : TEXTUREFLAGS_SRGB );
		
		if ( params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent() )
		{
			bIsBaseTextureTranslucent = true;
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

	if ( info.m_nDetail != -1 && params[info.m_nDetail]->IsDefined() )
	{
		int nDetailBlendMode = ( info.m_nDetailTextureCombineMode == -1 ) ? 0 : params[info.m_nDetailTextureCombineMode]->GetIntValue();
		if ( nDetailBlendMode == 0 ) //Mod2X
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
		}
		else if ( (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined() )
		{
			SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );
		}
	}

	// Don't alpha test if the alpha channel is used for other purposes
	if ( IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) )
	{
		CLEAR_FLAGS( MATERIAL_VAR_ALPHATEST );
	}
	
	if ( info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined() )
	{
		if ( !IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) )
		{
			pShader->LoadCubeMap( info.m_nEnvmap, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0 );
		}
		else
		{
			pShader->LoadTexture( info.m_nEnvmap, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0 );
		}
		
		if ( !g_pHardwareConfig->SupportsCubeMaps() )
		{
			SET_FLAGS( MATERIAL_VAR_ENVMAPSPHERE );
		}
	}
	if ( info.m_nEnvmapMask != -1 && params[info.m_nEnvmapMask]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nEnvmapMask );
	}

	if ( (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDiffuseWarpTexture );
	}

	if ( bHasSelfIllumMask )
	{
		pShader->LoadTexture( info.m_nSelfIllumMask );
	}
}

class CVertexLitGeneric_DX9_Context : public CBasePerMaterialContextData
{
public:
	CCommandBufferBuilder< CFixedCommandStorageBuffer< 800 > > m_SemiStaticCmdsOut;

};


//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
static void DrawVertexLitGeneric_DX9_Internal( CBaseVSShader *pShader, IMaterialVar** params,
											   IShaderDynamicAPI *pShaderAPI,
											   IShaderShadow* pShaderShadow,
											   bool bVertexLitGeneric, bool bHasFlashlight, 
											   VertexLitGeneric_DX9_Vars_t &info,
											   VertexCompressionType_t vertexCompression,
											   CBasePerMaterialContextData **pContextDataPtr ) 

{
	CVertexLitGeneric_DX9_Context *pContextData = reinterpret_cast< CVertexLitGeneric_DX9_Context *> ( *pContextDataPtr );

/*^*/ // 	printf("\t\t>DrawVertexLitGeneric_DX9_Internal\n");

	bool bHasBump = IsTextureSet( info.m_nBumpmap, params );
	bool bIsDecal = IS_FLAG_SET( MATERIAL_VAR_DECAL );

	bool hasDiffuseLighting = bVertexLitGeneric;
/*^*/ // 	printf("\t\t[%d] bVertexLitGeneric\n",(int)bVertexLitGeneric);

	if ( IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) )
	{
		bHasFlashlight = false;
	}

	bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;
	bool bHasDiffuseWarp = (!bHasFlashlight || false ) && hasDiffuseLighting && (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsTexture();


	//bool bNoCull = IS_FLAG_SET( MATERIAL_VAR_NOCULL );
	bool bFlashlightNoLambert = false;
	if ( ( info.m_nFlashlightNoLambert != -1 ) && params[info.m_nFlashlightNoLambert]->GetIntValue() )
	{
		bFlashlightNoLambert = true;
	}

	bool bAmbientOnly = IsBoolSet( info.m_nAmbientOnly, params );

	float fBlendFactor = GetFloatParam( info.m_nDetailTextureBlendFactor, params, 1.0 );
	bool bHasDetailTexture = IsTextureSet( info.m_nDetail, params );
	int nDetailBlendMode = bHasDetailTexture ? GetIntParam( info.m_nDetailTextureCombineMode, params ) : 0;
	int nDetailTranslucencyTexture = -1;

	if ( bHasDetailTexture )
	{
		if ( ( nDetailBlendMode == 6 ) && ( ! (g_pHardwareConfig->SupportsPixelShaders_2_b() ) ) )
		{
			nDetailBlendMode = 5;								// skip fancy threshold blending if ps2.0
		}
		if ( ( nDetailBlendMode == 3 ) || ( nDetailBlendMode == 8 ) || ( nDetailBlendMode == 9 ) )
			nDetailTranslucencyTexture = info.m_nDetail;
	}

	bool bBlendTintByBaseAlpha = IsBoolSet( info.m_nBlendTintByBaseAlpha, params );
	float fTintReplaceFactor = GetFloatParam( info.m_nTintReplacesBaseColor, params, 0.0 );

	BlendType_t nBlendType;
	bool bHasBaseTexture = IsTextureSet( info.m_nBaseTexture, params );
	if ( bHasBaseTexture )
	{
		// if base alpha is used for tinting, ignore the base texture for computing translucency
		nBlendType = pShader->EvaluateBlendRequirements( bBlendTintByBaseAlpha ? -1 : info.m_nBaseTexture, true, nDetailTranslucencyTexture );
	}
	else
	{
		nBlendType = pShader->EvaluateBlendRequirements( info.m_nEnvmapMask, false );
	}
	bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested && (!bHasFlashlight || false ); //dest alpha is free for special use

	bool bHasEnvmap = (!bHasFlashlight || false ) && info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsTexture();

	
	bool bHasVertexColor = bVertexLitGeneric ? false : IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
	bool bHasVertexAlpha = bVertexLitGeneric ? false : IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA );
/*^*/ // 	printf("\t\t[%d] bHasVertexColor\n",(int)bHasVertexColor);
/*^*/ // 	printf("\t\t[%d] bHasVertexAlpha\n",(int)bHasVertexAlpha);
	
	if ( pShader->IsSnapshotting() || (! pContextData ) || ( pContextData->m_bMaterialVarsChanged ) )
	{
/*^*/ // 	printf("\t\t[1] snapshotting=%d  pContextData=%08x  pContextData->m_bMaterialVarsChanged=%d \n",(int)pShader->IsSnapshotting(), (int)pContextData, pContextData ? (int)pContextData->m_bMaterialVarsChanged : -1 );
		bool bSeamlessBase = IsBoolSet( info.m_nSeamlessBase, params );
		bool bSeamlessDetail = IsBoolSet( info.m_nSeamlessDetail, params );
		bool bDistanceAlpha = IsBoolSet( info.m_nDistanceAlpha, params );
		bool bHasSelfIllum = (!bHasFlashlight || false ) && IS_FLAG_SET( MATERIAL_VAR_SELFILLUM );
		bool bHasEnvmapMask = (!bHasFlashlight || false ) && info.m_nEnvmapMask != -1 && params[info.m_nEnvmapMask]->IsTexture();
		bool bHasSelfIllumFresnel = ( !IsTextureSet( info.m_nDetail, params ) ) && ( bHasSelfIllum ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 );

		bool bHasSelfIllumMask = bHasSelfIllum && IsTextureSet( info.m_nSelfIllumMask, params );
		bool hasSelfIllumInEnvMapMask =
			( info.m_nSelfIllumEnvMapMask_Alpha != -1 ) &&
			( params[info.m_nSelfIllumEnvMapMask_Alpha]->GetFloatValue() != 0.0 ) ;

		if  ( pShader->IsSnapshotting() )
		{
/*^*/ // 		printf("\t\t[2] snapshotting...\n");

			bool hasBaseAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_BASEALPHAENVMAPMASK );
			bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
			
			
			if ( info.m_nVertexAlphaTest != -1 && params[info.m_nVertexAlphaTest]->GetIntValue() > 0 )
			{
				bHasVertexAlpha = true;
			}

			// look at color and alphamod stuff.
			// Unlit generic never uses the flashlight
			if ( bHasSelfIllumFresnel )
			{
				CLEAR_FLAGS( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
				hasNormalMapAlphaEnvmapMask = false;
			}

			bool bHasEnvmap = (!bHasFlashlight || false ) && ( info.m_nEnvmap != -1 ) && params[info.m_nEnvmap]->IsTexture();
			bool bHasLegacyEnvSphereMap = bHasEnvmap && IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE);
			bool bHasNormal = bVertexLitGeneric || bHasEnvmap || bHasFlashlight || bSeamlessBase || bSeamlessDetail;
			if ( IsPC() )
			{
				// On PC, LIGHTING_PREVIEW requires normals (they won't use much memory - unlitgeneric isn't used on many models)
				bHasNormal = true;
			}

			bool bHalfLambert = IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT );
			// Alpha test: FIXME: shouldn't this be handled in CBaseVSShader::SetInitialShadowState
			pShaderShadow->EnableAlphaTest( bIsAlphaTested );

			if ( info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f )
			{
				pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue() );
			}

			int nShadowFilterMode = 0;
			if ( bHasFlashlight )
			{
				if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				{
					nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
				}

				{
					if (params[info.m_nBaseTexture]->IsTexture())
					{
						pShader->SetAdditiveBlendingShadowState( info.m_nBaseTexture, true );
					}
					else
					{
						pShader->SetAdditiveBlendingShadowState( info.m_nEnvmapMask, false );
					}

					if ( bIsAlphaTested )
					{
						// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to 
						// be the same on both the regular pass and the flashlight pass.
						pShaderShadow->EnableAlphaTest( false );
						pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
					}

					// Be sure not to write to dest alpha
					pShaderShadow->EnableAlphaWrites( false );

					pShaderShadow->EnableBlending( true );
					pShaderShadow->EnableDepthWrites( false );
				}
			}
			else
			{
				pShader->SetBlendingShadowState( nBlendType );
			}
		
			unsigned int flags = VERTEX_POSITION;
			if ( bHasNormal )
			{
				flags |= VERTEX_NORMAL;
			}
/*^*/ // 	printf("\t\t[%1d] VERTEX_NORMAL\n",(flags&VERTEX_NORMAL)!=0);

			int userDataSize = 0;
			bool bSRGBInputAdapter = false;

			// basetexture
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			if ( bHasBaseTexture )
			{
				if ( ( info.m_nGammaColorRead != -1 ) && ( params[info.m_nGammaColorRead]->GetIntValue() == 1 ) )
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, false );
				else
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

				// If we're on OSX GL on a crappy OS which can't do sRGB from render targets, check to see if we're reading from one...
				if ( IsOSX() && !g_pHardwareConfig->CanDoSRGBReadFromRTs() )
				{
					ITexture *pBaseTexture = params[info.m_nBaseTexture]->GetTextureValue();
					if ( pBaseTexture && pBaseTexture->IsRenderTarget() )
					{
						bSRGBInputAdapter = true;
					}
				}
			}

			if ( bHasEnvmap )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
				if( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
				{
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );
				}
			}
			if ( bHasFlashlight )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );	// Depth texture
				pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER8 );
				pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );	// Noise map
				pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );	// Flashlight cookie
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER7, true );
				userDataSize = 4; // tangent S
			}

			if ( bHasDetailTexture )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
				if ( nDetailBlendMode != 0 ) //Not Mod2X
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true );
			}

			if ( bHasBump || bHasDiffuseWarp )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );
				userDataSize = 4; // tangent S
				// Normalizing cube map
				pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );
			}
			if ( bHasEnvmapMask )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );
			}

			if ( bHasVertexColor || bHasVertexAlpha )
			{
				flags |= VERTEX_COLOR;
			}
/*^*/ // 	printf("\t\t[%1d] VERTEX_COLOR\n",(flags&VERTEX_COLOR)!=0);
/*^*/ // 	printf("\t\t[%1d] VERTEX_COLOR_STREAM_1\n",(flags&VERTEX_COLOR_STREAM_1)!=0);


			if ( bHasDiffuseWarp && (!bHasFlashlight || false ) && !bHasSelfIllumFresnel )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER9, true );	// Diffuse warp texture
			}

			if ( (info.m_nDepthBlend != -1) && (params[info.m_nDepthBlend]->GetIntValue()) )
			{
				if( bHasBump )
					Warning( "DEPTHBLEND not supported by bump mapped variations of vertexlitgeneric to avoid shader bloat. Either remove the bump map or convince a graphics programmer that it's worth it.\n"  );
			
				pShaderShadow->EnableTexture( SHADER_SAMPLER10, true );
			}

			if( bHasSelfIllum )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER11, true );	// self illum mask
			}

			bool bSRGBWrite = true;
			if( (info.m_nLinearWrite != -1) && (params[info.m_nLinearWrite]->GetIntValue() == 1) )
			{
				bSRGBWrite = false;
			}

			pShaderShadow->EnableSRGBWrite( bSRGBWrite );
		
			// texcoord0 : base texcoord
			int pTexCoordDim[3] = { 2, 2, 3 };
			int nTexCoordCount = 1;
		
			if ( IsBoolSet( info.m_nSeparateDetailUVs, params ) )
			{
				++nTexCoordCount;
			}
			else
			{
				pTexCoordDim[1] = 0;
			}

			// Special morphed decal information 
			if ( bIsDecal && g_pHardwareConfig->HasFastVertexTextures() )
			{
				nTexCoordCount = 3;
			}

			// This shader supports compressed vertices, so OR in that flag:
			flags |= VERTEX_FORMAT_COMPRESSED;
/*^*/ // 		printf("\t\t[%1d] VERTEX_FORMAT_COMPRESSED\n",(flags&VERTEX_FORMAT_COMPRESSED)!=0);

/*^*/ // 		printf("\t\t      -> CShaderShadowDX8::VertexShaderVertexFormat( flags=%08x, texcount=%d )\n",flags,nTexCoordCount);


			pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, pTexCoordDim, userDataSize );

			if ( bHasBump || bHasDiffuseWarp )
			{
				if ( !g_pHardwareConfig->HasFastVertexTextures() )
				{
					bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

				
					if ( g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders() ) // Always send GL this way
					{
					}
					else // ps_2_0
					{
					}
				}
				else
				{
					// The vertex shader uses the vertex id stream
					SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );


				}
			}
			else // !(bHasBump || bHasDiffuseWarp)
			{
				bool bDistanceAlphaFromDetail = false;
				bool bSoftMask = false;
				bool bGlow = false;
				bool bOutline = false;

				static ConVarRef mat_reduceparticles( "mat_reduceparticles" );
				bool bDoDepthBlend = IsBoolSet( info.m_nDepthBlend, params ) && !mat_reduceparticles.GetBool();

				if ( bDistanceAlpha )
				{
					bDistanceAlphaFromDetail = IsBoolSet( info.m_nDistanceAlphaFromDetail, params );
					bSoftMask = IsBoolSet( info.m_nSoftEdges, params );
					bGlow = IsBoolSet( info.m_nGlow, params );
					bOutline = IsBoolSet( info.m_nOutline, params );
				}

				if ( !g_pHardwareConfig->HasFastVertexTextures() )
				{
					bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();


					if ( g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders() ) // Always send Gl this way
					{
					}
					else // ps_2_0
					{
					}
				}
				else
				{
					// The vertex shader uses the vertex id stream
					SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );


				}
			}

			if ( bHasFlashlight )
			{
				pShader->FogToBlack();
			}
			else
			{
				pShader->DefaultFog();
			}

			// HACK HACK HACK - enable alpha writes all the time so that we have them for
			// underwater stuff and the loadout and character select screens.
			pShaderShadow->EnableAlphaWrites( bFullyOpaque );
		}

	}
	pShader->Draw();

/*^*/ // 	printf("\t\t<DrawVertexLitGeneric_DX9_Internal\n");
}


void DrawVertexLitGeneric_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
	IShaderShadow* pShaderShadow, bool bVertexLitGeneric, VertexLitGeneric_DX9_Vars_t &info, VertexCompressionType_t vertexCompression,
								CBasePerMaterialContextData **pContextDataPtr )
{
	if ( WantsSkinShader( params, info ) && g_pHardwareConfig->SupportsPixelShaders_2_b() && g_pConfig->UseBumpmapping() && g_pConfig->UsePhong() )
	{
		DrawSkin_DX9( pShader, params, pShaderAPI, pShaderShadow, info, vertexCompression, pContextDataPtr );
		return;
	}
	
	bool bReceiveFlashlight = bVertexLitGeneric;
	bool bNewFlashlight = false;
	if ( bNewFlashlight )
	{
		bReceiveFlashlight = bReceiveFlashlight || ( GetIntParam( info.m_nReceiveFlashlight, params ) != 0 );
	}
	bool bHasFlashlight = bReceiveFlashlight && pShader->UsingFlashlight( params );

	DrawVertexLitGeneric_DX9_Internal( pShader, params, pShaderAPI,
		pShaderShadow, bVertexLitGeneric, bHasFlashlight, info, vertexCompression, pContextDataPtr );
}
