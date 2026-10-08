//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// This is what all vs/ps (dx8+) shaders inherit from.
//===========================================================================//

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "mathlib/bumpvects.h"
#include "cpp_shader_constant_register_map.h"
#include "convar.h"

#ifndef GAME_SHADER_DLL
#ifdef HDR
#endif

#if SUPPORT_DX8
#endif

#include "lightmappedgeneric_flashlight_vs20.inc"
#include "flashlight_ps20.inc"
#include "flashlight_ps20b.inc"
#include "vertexlitgeneric_flashlight_vs11.inc"
#endif // GAME_SHADER_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright( "mat_fullbright","0", FCVAR_CHEAT );

// These functions are to be called from the shaders.

//-----------------------------------------------------------------------------
// Pixel and vertex shader constants....
//-----------------------------------------------------------------------------
void CBaseVSShader::SetPixelShaderConstant( int pixelReg, int constantVar, int constantVar2 )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1) || (constantVar2 == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );
	IMaterialVar* pPixelVar2 = s_ppParams[constantVar2];
	Assert( pPixelVar2 );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pPixelVar->GetVecValue( val, 3 );
	}
	else
	{
		val[0] = val[1] = val[2] = pPixelVar->GetFloatValue();
	}

	val[3] = pPixelVar2->GetFloatValue();
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstantGammaToLinear( int pixelReg, int constantVar, int constantVar2 )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1) || (constantVar2 == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );
	IMaterialVar* pPixelVar2 = s_ppParams[constantVar2];
	Assert( pPixelVar2 );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pPixelVar->GetVecValue( val, 3 );
	}
	else
	{
		val[0] = val[1] = val[2] = pPixelVar->GetFloatValue();
	}

	val[3] = pPixelVar2->GetFloatValue();
	val[0] = val[0] > 1.0f ? val[0] : GammaToLinear( val[0] );
	val[1] = val[1] > 1.0f ? val[1] : GammaToLinear( val[1] );
	val[2] = val[2] > 1.0f ? val[2] : GammaToLinear( val[2] );

	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstant_W( int pixelReg, int constantVar, float fWValue )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pPixelVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue();
	val[3]=fWValue;
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstant( int pixelReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pPixelVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue();
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstantGammaToLinear( int pixelReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pPixelVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue();

	val[0] = val[0] > 1.0f ? val[0] : GammaToLinear( val[0] );
	val[1] = val[1] > 1.0f ? val[1] : GammaToLinear( val[1] );
	val[2] = val[2] > 1.0f ? val[2] : GammaToLinear( val[2] );

	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstantGammaToLinear( int var, float const* pVec, int numConst, bool bForce )
{
	int i;
	for( i = 0; i < numConst; i++ )
	{
		float vec[4];
		vec[0] = pVec[i*4+0] > 1.0f ? pVec[i*4+0] : GammaToLinear( pVec[i*4+0] );
		vec[1] = pVec[i*4+1] > 1.0f ? pVec[i*4+1] : GammaToLinear( pVec[i*4+1] );
		vec[2] = pVec[i*4+2] > 1.0f ? pVec[i*4+2] : GammaToLinear( pVec[i*4+2] );

		vec[3] = pVec[i*4+3];

		s_pShaderAPI->SetPixelShaderConstant( var + i, vec, 1, bForce );
	}
}

void CBaseVSShader::SetVertexShaderConstant( int vertexReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pVertexVar = s_ppParams[constantVar];
	Assert( pVertexVar );

	float val[4];
	if (pVertexVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pVertexVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pVertexVar->GetFloatValue();
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, val );	
}

void CBaseVSShader::SetVertexShaderTextureTransform( int vertexReg, int transformVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
		transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
	}
	else
	{
		transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
		transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
	}
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
}

void CBaseVSShader::SetVertexShaderTextureScaledTransform( int vertexReg, int transformVar, int scaleVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
		transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
	}
	else
	{
		transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
		transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
	}

	Vector2D scale( 1, 1 );
	IMaterialVar* pScaleVar = s_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue( scale.Base(), 2 );
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}

	// Apply the scaling
	transformation[0][0] *= scale[0];
	transformation[0][1] *= scale[1];
	transformation[1][0] *= scale[0];
	transformation[1][1] *= scale[1];
	transformation[0][3] *= scale[0];
	transformation[1][3] *= scale[1];
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
}


void CBaseVSShader::SetPixelShaderTextureTransform( int pixelReg, int transformVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
		transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
	}
	else
	{
		transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
		transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
	}
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, transformation[0].Base(), 2 ); 
}

void CBaseVSShader::SetModulationPixelShaderDynamicState( int modulationVar )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	s_pShaderAPI->SetPixelShaderConstant( modulationVar, color );
}

void CBaseVSShader::SetModulationPixelShaderDynamicState_LinearColorSpace( int modulationVar )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	color[0] = color[0] > 1.0f ? color[0] : GammaToLinear( color[0] );
	color[1] = color[1] > 1.0f ? color[1] : GammaToLinear( color[1] );
	color[2] = color[2] > 1.0f ? color[2] : GammaToLinear( color[2] );

	s_pShaderAPI->SetPixelShaderConstant( modulationVar, color );
}

void CBaseVSShader::SetModulationPixelShaderDynamicState_LinearColorSpace_LinearScale( int modulationVar, float flScale )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	color[0] = ( color[0] > 1.0f ? color[0] : GammaToLinear( color[0] ) ) * flScale;
	color[1] = ( color[1] > 1.0f ? color[1] : GammaToLinear( color[1] ) ) * flScale;
	color[2] = ( color[2] > 1.0f ? color[2] : GammaToLinear( color[2] ) ) * flScale;

	s_pShaderAPI->SetPixelShaderConstant( modulationVar, color );
}


#ifdef _DEBUG
ConVar mat_envmaptintoverride( "mat_envmaptintoverride", "-1" );
ConVar mat_envmaptintscale( "mat_envmaptintscale", "-1" );
#endif

//-----------------------------------------------------------------------------
// Helpers for dealing with envmap tint
//-----------------------------------------------------------------------------
// set alphaVar to -1 to ignore it.
void CBaseVSShader::SetEnvMapTintPixelShaderDynamicState( int pixelReg, int tintVar, int alphaVar, bool bConvertFromGammaToLinear )
{
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if( g_pConfig->bShowSpecular && mat_fullbright.GetInt() != 2 )
	{
		IMaterialVar* pAlphaVar = NULL;
		if( alphaVar >= 0 )
		{
			pAlphaVar = s_ppParams[alphaVar];
		}
		if( pAlphaVar )
		{
			color[3] = pAlphaVar->GetFloatValue();
		}

		IMaterialVar* pTintVar = s_ppParams[tintVar];
#ifdef _DEBUG
		pTintVar->GetVecValue( color, 3 );

		float envmapTintOverride = mat_envmaptintoverride.GetFloat();
		float envmapTintScaleOverride = mat_envmaptintscale.GetFloat();

		if( envmapTintOverride != -1.0f )
		{
			color[0] = color[1] = color[2] = envmapTintOverride;
		}
		if( envmapTintScaleOverride != -1.0f )
		{
			color[0] *= envmapTintScaleOverride;
			color[1] *= envmapTintScaleOverride;
			color[2] *= envmapTintScaleOverride;
		}

		if( bConvertFromGammaToLinear )
		{
			color[0] = color[0] > 1.0f ? color[0] : GammaToLinear( color[0] );
			color[1] = color[1] > 1.0f ? color[1] : GammaToLinear( color[1] );
			color[2] = color[2] > 1.0f ? color[2] : GammaToLinear( color[2] );
		}
#else
		if( bConvertFromGammaToLinear )
		{
			pTintVar->GetLinearVecValue( color, 3 );
		}
		else
		{
			pTintVar->GetVecValue( color, 3 );
		}
#endif
	}
	else
	{
		color[0] = color[1] = color[2] = color[3] = 0.0f;
	}
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, color, 1 );
}

void CBaseVSShader::SetAmbientCubeDynamicStateVertexShader( )
{
	s_pShaderAPI->SetVertexShaderStateAmbientLightCube();
}

float CBaseVSShader::GetAmbientLightCubeLuminance( )
{
	return s_pShaderAPI->GetAmbientLightCubeLuminance();
}

#ifndef GAME_SHADER_DLL
//-----------------------------------------------------------------------------
// Sets up hw morphing state for the vertex shader
//-----------------------------------------------------------------------------
void CBaseVSShader::SetHWMorphVertexShaderState( int nDimConst, int nSubrectConst, VertexTextureSampler_t morphSampler )
{
	if ( !s_pShaderAPI->IsHWMorphingEnabled() )
		return;

	int nMorphWidth, nMorphHeight;
	s_pShaderAPI->GetStandardTextureDimensions( &nMorphWidth, &nMorphHeight, TEXTURE_MORPH_ACCUMULATOR );

	int nDim = s_pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_MORPH_ACCUMULATOR_4TUPLE_COUNT );
	float pMorphAccumSize[4] = { (float)nMorphWidth, (float)nMorphHeight, (float)nDim, 0.0f };
	s_pShaderAPI->SetVertexShaderConstant( nDimConst, pMorphAccumSize );

	int nXOffset = s_pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_MORPH_ACCUMULATOR_X_OFFSET );
	int nYOffset = s_pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_MORPH_ACCUMULATOR_Y_OFFSET );
	int nWidth = s_pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_MORPH_ACCUMULATOR_SUBRECT_WIDTH );
	int nHeight = s_pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_MORPH_ACCUMULATOR_SUBRECT_HEIGHT );
	float pMorphAccumSubrect[4] = { (float)nXOffset, (float)nYOffset, (float)nWidth, (float)nHeight };
	s_pShaderAPI->SetVertexShaderConstant( nSubrectConst, pMorphAccumSubrect );

	s_pShaderAPI->BindStandardVertexTexture( morphSampler, TEXTURE_MORPH_ACCUMULATOR );
}


//#define USE_DEST_ALPHA
#define USE_NORMALMAP_ALPHA

#endif // GAME_SHADER_DLL


//-----------------------------------------------------------------------------
// GR - translucency query
//-----------------------------------------------------------------------------
BlendType_t CBaseVSShader::EvaluateBlendRequirements( int textureVar, bool isBaseTexture,
													  int detailTextureVar )
{
	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha (for blending or alpha test)
	isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) &&
		                               !(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	if ( ( detailTextureVar != -1 ) && ( ! isTranslucent ) )
	{
		isTranslucent = TextureIsTranslucent( detailTextureVar, isBaseTexture );
	}

	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE )
	{	
		return isTranslucent ? BT_BLENDADD : BT_ADD;	// Additive
	}
	else
	{
		return isTranslucent ? BT_BLEND : BT_NONE;		// Normal blending
	}
}

#ifndef GAME_SHADER_DLL

#if SUPPORT_DX8
#endif // support_dx8

void CBaseVSShader::DrawFlashlight_dx90( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, 
										IShaderShadow* pShaderShadow, DrawFlashlight_dx90_Vars_t &vars )
{
	// FLASHLIGHTFIXME: hack . . need to fix the vertex shader so that it can deal with and without bumps for vertexlitgeneric
	if( !vars.m_bLightmappedGeneric )
	{
		vars.m_bBump = false;
	}
	bool bBump2 = vars.m_bWorldVertexTransition && vars.m_bBump && vars.m_nBumpmap2Var != -1 && params[vars.m_nBumpmap2Var]->IsTexture();
	bool bSeamless = vars.m_fSeamlessScale != 0.0;
	bool bDetail = vars.m_bLightmappedGeneric && (vars.m_nDetailVar != -1) && params[vars.m_nDetailVar]->IsDefined() && (vars.m_nDetailScale != -1);

	int nDetailBlendMode = 0;
	if ( bDetail )
	{
		nDetailBlendMode = GetIntParam( vars.m_nDetailTextureCombineMode, params );
		nDetailBlendMode = nDetailBlendMode > 1 ? 1 : nDetailBlendMode;
	}

	if( pShaderShadow )
	{
		SetInitialShadowState();
		pShaderShadow->EnableDepthWrites( false );
		pShaderShadow->EnableAlphaWrites( false );

		// Alpha blend
		SetAdditiveBlendingShadowState( BASETEXTURE, true );

		// Alpha test
		pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );
		if ( vars.m_nAlphaTestReference != -1 && params[vars.m_nAlphaTestReference]->GetFloatValue() > 0.0f )
		{
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[vars.m_nAlphaTestReference]->GetFloatValue() );
		}

		// Spot sampler
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

		// Base sampler
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );

		// Normalizing cubemap sampler
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );

		// Normalizing cubemap sampler2 or normal map sampler
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );

		// RandomRotation sampler
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );

		// Flashlight depth sampler
		pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );
		pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER7 );

		if( vars.m_bWorldVertexTransition )
		{
			// $basetexture2
			pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, true );
		}
		if( bBump2 )
		{
			// Normalmap2 sampler
			pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );
		}
		if( bDetail )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );				// detail sampler
			if ( nDetailBlendMode != 0 ) //Not Mod2X
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER8, true );
		}
		
		pShaderShadow->EnableSRGBWrite( true );

		if( vars.m_bLightmappedGeneric )
		{
			lightmappedgeneric_flashlight_vs20_Static_Index	vshIndex;
			vshIndex.SetWORLDVERTEXTRANSITION( vars.m_bWorldVertexTransition );
			vshIndex.SetNORMALMAP( vars.m_bBump );
			vshIndex.SetSEAMLESS( bSeamless );
			vshIndex.SetDETAIL( bDetail );
			pShaderShadow->SetVertexShader( "lightmappedgeneric_flashlight_vs20", vshIndex.GetIndex() );

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			if( vars.m_bBump )
			{
				flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
			}
			int numTexCoords = 1;
			if( vars.m_bWorldVertexTransition )
			{
				flags |= VERTEX_COLOR;
				numTexCoords = 2; // need lightmap texcoords to get alpha.
			}
			pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, 0, 0 );
		}
		else
		{
			vertexlitgeneric_flashlight_vs11_Static_Index vshIndex;
			vshIndex.SetTEETH( vars.m_bTeeth );
			pShaderShadow->SetVertexShader( "vertexlitgeneric_flashlight_vs11", vshIndex.GetIndex() );

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			int numTexCoords = 1;
			pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, 0, vars.m_bBump ? 4 : 0 );
		}

		int nBumpMapVariant = 0;
		if ( vars.m_bBump )
		{
			nBumpMapVariant = ( vars.m_bSSBump ) ? 2 : 1;
		}
		if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
		{
			int nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();

			flashlight_ps20b_Static_Index	pshIndex;
			pshIndex.SetNORMALMAP( nBumpMapVariant );
			pshIndex.SetNORMALMAP2( bBump2 );
			pshIndex.SetWORLDVERTEXTRANSITION( vars.m_bWorldVertexTransition );
			pshIndex.SetSEAMLESS( bSeamless );
			pshIndex.SetDETAILTEXTURE( bDetail );
			pshIndex.SetDETAIL_BLEND_MODE( nDetailBlendMode );
			pshIndex.SetFLASHLIGHTDEPTHFILTERMODE( nShadowFilterMode );
			pShaderShadow->SetPixelShader( "flashlight_ps20b", pshIndex.GetIndex() );
		}
		else
		{
			flashlight_ps20_Static_Index	pshIndex;
			pshIndex.SetNORMALMAP( nBumpMapVariant );
			pshIndex.SetNORMALMAP2( bBump2 );
			pshIndex.SetWORLDVERTEXTRANSITION( vars.m_bWorldVertexTransition );
			pshIndex.SetSEAMLESS( bSeamless );
			pshIndex.SetDETAILTEXTURE( bDetail );
			pshIndex.SetDETAIL_BLEND_MODE( nDetailBlendMode );
			pShaderShadow->SetPixelShader( "flashlight_ps20", pshIndex.GetIndex() );
		}
		FogToBlack();
	}
	Draw();
}


#endif // GAME_SHADER_DLL



// Take 0..1 seed and map to (u, v) coordinate to be used in shadow filter jittering...
void CBaseVSShader::HashShadow2DJitter( const float fJitterSeed, float *fU, float* fV )
{
	const int nTexRes = 32;
	int nSeed = fmod (fJitterSeed, 1.0f) * nTexRes * nTexRes;

	int nRow = nSeed / nTexRes;
	int nCol = nSeed % nTexRes;

	// Div and mod to get an individual texel in the fTexRes x fTexRes grid
	*fU = nRow / (float) nTexRes;	// Row
	*fV = nCol / (float) nTexRes;	// Column
}


void CBaseVSShader::DrawEqualDepthToDestAlpha( void )
{
	if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
	{
		bool bMakeActualDrawCall = false;
		if( s_pShaderShadow )
		{
			s_pShaderShadow->EnableColorWrites( false );
			s_pShaderShadow->EnableAlphaWrites( true );
			s_pShaderShadow->EnableDepthWrites( false );
			s_pShaderShadow->EnableAlphaTest( false );
			s_pShaderShadow->EnableBlending( false );

			s_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );

			s_pShaderShadow->SetVertexShader( "depthtodestalpha_vs20", 0 );
			s_pShaderShadow->SetPixelShader( "depthtodestalpha_ps20b", 0 );
		}
		if( s_pShaderAPI )
		{
			s_pShaderAPI->SetVertexShaderIndex( 0 );
			s_pShaderAPI->SetPixelShaderIndex( 0 );

			bMakeActualDrawCall = s_pShaderAPI->ShouldWriteDepthToDestAlpha();
		}
		Draw( bMakeActualDrawCall );
	}
}
