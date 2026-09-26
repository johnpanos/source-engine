//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal 2 paint on world surfaces. The engine (engine/paint.cpp)
//          draws each painted brush surface again with a LightmappedPaint
//          material whose $paintmap is the surface's paint page: per lightmap
//          luxel the gel color and its coverage.
//
// Ported from the CS:GO-era LightmappedGeneric paint pass
// (lightmappedpaint_dx9_helper.cpp), which drew the surface's own material
// with the paint shader; here the pass is its own shader. The pixel shader is
// lightmappedpaint_ps2x.fxc (native Vulkan: shaders/lightmappedpaint.frag),
// after lightmappedgeneric_vs20 with bumped lightmap coordinates.
//
//=============================================================================//

#include "BaseVSShader.h"
#include "convar.h"
#include "cpp_shader_constant_register_map.h"
#include "lightmappedgeneric_vs20.inc"
#include "lightmappedpaint_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER( LightmappedPaint, "Portal 2 paint on lightmapped world surfaces" )
BEGIN_SHADER_PARAMS
SHADER_PARAM( PAINTMAP, SHADER_PARAM_TYPE_TEXTURE, "", "the surface's paint page (engine)" )
SHADER_PARAM( PAINTSPLATNORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "paint/splatnormal_default",
    "paint splat normal map; alpha is the splat height" )
SHADER_PARAM( PAINTSPLATBUBBLELAYOUT, SHADER_PARAM_TYPE_TEXTURE, "paint/bubblelayout",
    "layout of the bubbles in the thick paint" )
SHADER_PARAM( PAINTSPLATBUBBLE, SHADER_PARAM_TYPE_TEXTURE, "paint/bubble", "bubble normal map" )
SHADER_PARAM( PAINTENVMAP, SHADER_PARAM_TYPE_TEXTURE, "paint/paint_envmap_hdr", "paint reflection" )
SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "paint reflection tint" )
SHADER_PARAM( THICKPAINT, SHADER_PARAM_TYPE_INTEGER, "-1",
    "1 bubbles and bumped splats, 0 flat paint, -1 by gpu_level (thick above 1), as retail" )
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP );
	// Retail's textures (lightmappedgeneric_dx9_helper.cpp's paint defaults).
	if ( !params[PAINTSPLATNORMALMAP]->IsDefined() )
		params[PAINTSPLATNORMALMAP]->SetStringValue( "paint/splatnormal_default" );
	if ( !params[PAINTSPLATBUBBLELAYOUT]->IsDefined() )
		params[PAINTSPLATBUBBLELAYOUT]->SetStringValue( "paint/bubblelayout" );
	if ( !params[PAINTSPLATBUBBLE]->IsDefined() )
		params[PAINTSPLATBUBBLE]->SetStringValue( "paint/bubble" );
	if ( !params[PAINTENVMAP]->IsDefined() )
		params[PAINTENVMAP]->SetStringValue( "paint/paint_envmap_hdr" );
	if ( !params[ENVMAPTINT]->IsDefined() )
		params[ENVMAPTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
	if ( !params[THICKPAINT]->IsDefined() )
		params[THICKPAINT]->SetIntValue( -1 );
}

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT
{
	if ( params[PAINTMAP]->IsDefined() )
		LoadTexture( PAINTMAP );
	if ( params[PAINTSPLATNORMALMAP]->IsDefined() )
		LoadBumpMap( PAINTSPLATNORMALMAP );
	if ( params[PAINTSPLATBUBBLELAYOUT]->IsDefined() )
		LoadTexture( PAINTSPLATBUBBLELAYOUT );
	if ( params[PAINTSPLATBUBBLE]->IsDefined() )
		LoadTexture( PAINTSPLATBUBBLE );
	if ( params[PAINTENVMAP]->IsDefined() )
		LoadCubeMap( PAINTENVMAP );
}

bool ThickPaint( IMaterialVar **params ) const
{
	int nThick = params[THICKPAINT]->GetIntValue();
	if ( nThick >= 0 )
		return nThick != 0;
	static ConVarRef gpu_level( "gpu_level", true );
	return !gpu_level.IsValid() || gpu_level.GetInt() > 1;
}

SHADER_DRAW
{
	const bool bHDR = g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE;
	const bool bHasEnvmap = params[PAINTENVMAP]->IsTexture();

	SHADOW_STATE
	{
		SetInitialShadowState();

		// Over the surface already drawn: blended, at its depth.
		EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		pShaderShadow->EnableDepthWrites( false );
		pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL );

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); // paint page
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true ); // lightmap
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, !bHDR );
		if ( bHasEnvmap )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, !bHDR );
		}
		pShaderShadow->EnableTexture( SHADER_SAMPLER4, true ); // bubble layout
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true ); // bubble normals
		pShaderShadow->EnableTexture( SHADER_SAMPLER7, true ); // splat normals

		// texcoord0 base, texcoord1 lightmap, texcoord2 bumped lightmap offset.
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_S | VERTEX_TANGENT_T;
		pShaderShadow->VertexShaderVertexFormat( flags, 3, 0, 0 );

		DECLARE_STATIC_VERTEX_SHADER( lightmappedgeneric_vs20 );
		SET_STATIC_VERTEX_SHADER_COMBO( ENVMAP_MASK, false );
		SET_STATIC_VERTEX_SHADER_COMBO( TANGENTSPACE, true );
		SET_STATIC_VERTEX_SHADER_COMBO( BUMPMAP, true );
		SET_STATIC_VERTEX_SHADER_COMBO( DIFFUSEBUMPMAP, true );
		SET_STATIC_VERTEX_SHADER_COMBO( VERTEXCOLOR, false );
		SET_STATIC_VERTEX_SHADER_COMBO( VERTEXALPHATEXBLENDFACTOR, false );
		SET_STATIC_VERTEX_SHADER_COMBO( BUMPMASK, false );
		SET_STATIC_VERTEX_SHADER_COMBO( RELIEF_MAPPING, false );
		SET_STATIC_VERTEX_SHADER_COMBO( SEAMLESS, false );
		SET_STATIC_VERTEX_SHADER( lightmappedgeneric_vs20 );

		DECLARE_STATIC_PIXEL_SHADER( lightmappedpaint_ps20b );
		SET_STATIC_PIXEL_SHADER_COMBO( BUMPMAP, 1 );
		SET_STATIC_PIXEL_SHADER_COMBO( CUBEMAP, bHasEnvmap ? 1 : 0 );
		SET_STATIC_PIXEL_SHADER_COMBO( SEAMLESS, false );
		SET_STATIC_PIXEL_SHADER_COMBO( THICKPAINT, ThickPaint( params ) );
		SET_STATIC_PIXEL_SHADER( lightmappedpaint_ps20b );

		pShaderShadow->EnableAlphaWrites( false );
		pShaderShadow->EnableSRGBWrite( true );
		DefaultFog();
	}
	DYNAMIC_STATE
	{
		BindTexture( SHADER_SAMPLER0, PAINTMAP );
		pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_LIGHTMAP );
		if ( bHasEnvmap )
			BindTexture( SHADER_SAMPLER2, PAINTENVMAP );
		if ( params[PAINTSPLATBUBBLELAYOUT]->IsTexture() )
			BindTexture( SHADER_SAMPLER4, PAINTSPLATBUBBLELAYOUT );
		else
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER4, TEXTURE_BLACK );
		if ( params[PAINTSPLATBUBBLE]->IsTexture() )
			BindTexture( SHADER_SAMPLER5, PAINTSPLATBUBBLE );
		else
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER5, TEXTURE_NORMALMAP_FLAT );
		if ( params[PAINTSPLATNORMALMAP]->IsTexture() )
			BindTexture( SHADER_SAMPLER7, PAINTSPLATNORMALMAP );
		else
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER7, TEXTURE_NORMALMAP_FLAT );

		// c0 the reflection tint; c10 the eye; c11 fog; c12 (and the vertex
		// modulation) the tint times the lightmap scale, $alpha2 in w; c21 the
		// tint alone.
		SetEnvMapTintPixelShaderDynamicState( 0, ENVMAPTINT, -1 );
		float vEyePos[4];
		pShaderAPI->GetWorldSpaceCameraPosition( vEyePos );
		vEyePos[3] = 0.0f;
		pShaderAPI->SetPixelShaderConstant( 10, vEyePos );
		pShaderAPI->SetPixelShaderFogParams( 11 );

		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		pShaderAPI->SetPixelShaderConstant( 21, color );
		const float flLScale = pShaderAPI->GetLightMapScaleFactor();
		color[0] *= flLScale;
		color[1] *= flLScale;
		color[2] *= flLScale;
		pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_MODULATION_COLOR, color );
		pShaderAPI->SetPixelShaderConstant( 12, color );

		// The camera's right and forward on the horizontal plane (c22, c23):
		// the thick paint's bubble refraction offsets.
		VMatrix mView;
		pShaderAPI->GetMatrix( MATERIAL_VIEW, mView.m[0] );
		mView = mView.Transpose3x3();
		Vector4D vCameraRight( mView.m[0][0], mView.m[0][1], 0.0f, 0.0f );
		vCameraRight.AsVector3D().NormalizeInPlace();
		Vector4D vCameraForward( 0.0f, 0.0f, 0.0f, 0.0f );
		CrossProduct( Vector( 0.0f, 0.0f, 1.0f ), vCameraRight.AsVector3D(), vCameraForward.AsVector3D() );
		pShaderAPI->SetPixelShaderConstant( 22, vCameraRight.Base() );
		pShaderAPI->SetPixelShaderConstant( 23, vCameraForward.Base() );

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		DECLARE_DYNAMIC_VERTEX_SHADER( lightmappedgeneric_vs20 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( FASTPATH, true );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( LIGHTING_PREVIEW, 0 );
		SET_DYNAMIC_VERTEX_SHADER( lightmappedgeneric_vs20 );

		DECLARE_DYNAMIC_PIXEL_SHADER( lightmappedpaint_ps20b );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( FASTPATH, true );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( FASTPATHENVMAPCONTRAST, false );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
		SET_DYNAMIC_PIXEL_SHADER( lightmappedpaint_ps20b );
	}
	Draw();
}
END_SHADER
