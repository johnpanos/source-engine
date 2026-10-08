//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"
#include "convar.h"
#include "cpp_shader_constant_register_map.h"

BEGIN_VS_SHADER( PortalStaticOverlay, 
				"Help for PortalStaticOverlay shader" )

				BEGIN_SHADER_PARAMS
				SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE )
				SHADER_PARAM_OVERRIDE( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE )
				SHADER_PARAM( STATICAMOUNT, SHADER_PARAM_TYPE_FLOAT, "0.0", "Amount of the static blend texture to blend into the base texture" )
				SHADER_PARAM( STATICBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "When adding static, this is the texture that gets blended in" )
				SHADER_PARAM( STATICBLENDTEXTUREFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
				SHADER_PARAM( ALPHAMASKTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "An alpha mask for odd shaped portals" )
				SHADER_PARAM( ALPHAMASKTEXTUREFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
				SHADER_PARAM( NOCOLORWRITE, SHADER_PARAM_TYPE_INTEGER, "0", "" )
				SHADER_PARAM( GHOSTOVERLAY, SHADER_PARAM_TYPE_INTEGER, "0", "Portal 2: draw the portal where it is hidden (1 tinted by the vertex color, 2 the texture's color)" )
				END_SHADER_PARAMS


SHADER_INIT_PARAMS()
{
	SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
}

SHADER_FALLBACK
{
	if( !g_pHardwareConfig->SupportsVertexAndPixelShaders() )
		return "PortalStaticOverlay_DX60";

	return 0;
}

SHADER_INIT
{
	if( params[STATICBLENDTEXTURE]->IsDefined() )
		LoadTexture( STATICBLENDTEXTURE );
	if( params[ALPHAMASKTEXTURE]->IsDefined() )
		LoadTexture( ALPHAMASKTEXTURE );

	if( !params[STATICAMOUNT]->IsDefined() )
		params[STATICAMOUNT]->SetFloatValue( 0.0f );

	if( !params[STATICBLENDTEXTURE]->IsDefined() )
		params[STATICBLENDTEXTURE]->SetIntValue( 0 );
	if( !params[STATICBLENDTEXTUREFRAME]->IsDefined() )
		params[STATICBLENDTEXTUREFRAME]->SetIntValue( 0 );

	if( !params[ALPHAMASKTEXTURE]->IsDefined() )
		params[ALPHAMASKTEXTURE]->SetIntValue( 0 );
	if( !params[ALPHAMASKTEXTUREFRAME]->IsDefined() )
		params[ALPHAMASKTEXTUREFRAME]->SetIntValue( 0 );

	if( !params[NOCOLORWRITE]->IsDefined() )
		params[NOCOLORWRITE]->SetIntValue( 0 );

	if( !params[GHOSTOVERLAY]->IsDefined() )
		params[GHOSTOVERLAY]->SetIntValue( 0 );
}

SHADER_DRAW
{
	bool bStaticBlendTexture = params[STATICBLENDTEXTURE]->IsTexture();
	bool bAlphaMaskTexture = params[ALPHAMASKTEXTURE]->IsTexture(); //must support 2 texture stages to use a mask

	bool bIsModel = IS_FLAG_SET( MATERIAL_VAR_MODEL );
	bool bColorWrites = params[NOCOLORWRITE]->GetIntValue() == 0;
	int nGhostOverlay = clamp( params[GHOSTOVERLAY]->GetIntValue(), 0, 2 );
	bool bGhostOverlay = nGhostOverlay != 0;

	SHADOW_STATE
	{
		SetInitialShadowState();
		FogToFogColor();

		//pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL ); //a portal is effectively a decal on top of a wall
		pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_NEAREROREQUAL );

		pShaderShadow->EnableDepthWrites( true );

		if( g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE )
		{
			pShaderShadow->EnableSRGBWrite( true );
		}

		pShaderShadow->EnableBlending( true );
		if( bGhostOverlay )
		{
			// Portal 2's ghost: a reverse z-test draws only the pixels where the
			// portal is hidden, and one / inverse-source-alpha keeps it visible
			// on bright surfaces in front of it.
			pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_FARTHER );
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->BlendFunc( SHADER_BLEND_ONE, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		}
		else
		{
			pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );

			pShaderShadow->EnableAlphaTest( true );
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.0f );
		}

		pShaderShadow->EnableColorWrites( bColorWrites );

		if( g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE )
			pShaderShadow->EnableSRGBWrite( true );

		if( bStaticBlendTexture || bAlphaMaskTexture )
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		if( bGhostOverlay && bStaticBlendTexture )
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		if( bStaticBlendTexture && bAlphaMaskTexture )
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

		int fmt = VERTEX_POSITION | VERTEX_NORMAL;
		if( bGhostOverlay )
			fmt |= VERTEX_COLOR;
		int userDataSize = 0;
		if( bIsModel )
		{
			userDataSize = 4;
		}
		else
		{
			fmt |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
		}
		pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, userDataSize );


		// Avoid setting a pixel shader when only doing depth/stencil operations, as recommended by PIX
		if( bColorWrites || bAlphaMaskTexture || g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() )
		{
			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}
		}
	}

	Draw();
}

END_SHADER


