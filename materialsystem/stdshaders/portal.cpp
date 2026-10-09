//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"
#include "convar.h"


DEFINE_FALLBACK_SHADER( Portal, Portal_DX90 )

BEGIN_VS_SHADER( Portal_DX90, 
				"Help for Portal shader" )

				BEGIN_SHADER_PARAMS
				SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE )
				SHADER_PARAM_OVERRIDE( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE )
				SHADER_PARAM( STATICAMOUNT, SHADER_PARAM_TYPE_FLOAT, "0.0", "Amount of the static blend texture to blend into the base texture" )
				SHADER_PARAM( STATICBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "When adding static, this is the texture that gets blended in" )
				SHADER_PARAM( STATICBLENDTEXTUREFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
				SHADER_PARAM( ALPHAMASKTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "An alpha mask for odd shaped portals" )
				SHADER_PARAM( ALPHAMASKTEXTUREFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
				SHADER_PARAM( RENDERFIXZ, SHADER_PARAM_TYPE_INTEGER, "0", "Special depth handling, intended for rendering bug workarounds for extremely close polygons" )
				SHADER_PARAM( USEALTERNATEVIEWMATRIX, SHADER_PARAM_TYPE_INTEGER, "1", "Use the alternate view matrix instead of the current view matrix" )
				SHADER_PARAM( ALTERNATEVIEWMATRIX, SHADER_PARAM_TYPE_MATRIX, "0", "The alternate view matrix to use when $usealternateviewmatrix is enabled" )
				END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
		if( !params[BASETEXTURE]->IsDefined() )
		{
			SET_FLAGS2( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE );
		}
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
			return "Portal_DX80";

		return 0;
	}

	SHADER_INIT
	{
		if ( params[BASETEXTURE]->IsDefined() )
		{
			
			LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
		}

		if ( params[STATICBLENDTEXTURE]->IsDefined() )
			LoadTexture( STATICBLENDTEXTURE );	
		if ( params[ALPHAMASKTEXTURE]->IsDefined() )
			LoadTexture( ALPHAMASKTEXTURE );

		if ( !params[STATICAMOUNT]->IsDefined() )
			params[STATICAMOUNT]->SetFloatValue( 0.0f );

		if ( !params[STATICAMOUNT]->IsDefined() )
			params[STATICAMOUNT]->SetFloatValue( 0.0f );

		if ( !params[STATICBLENDTEXTURE]->IsDefined() )
			params[STATICBLENDTEXTURE]->SetIntValue( 0 );
		if ( !params[STATICBLENDTEXTUREFRAME]->IsDefined() )
			params[STATICBLENDTEXTUREFRAME]->SetIntValue( 0 );

		if ( !params[ALPHAMASKTEXTURE]->IsDefined() )
			params[ALPHAMASKTEXTURE]->SetIntValue( 0 );
		if ( !params[ALPHAMASKTEXTUREFRAME]->IsDefined() )
			params[ALPHAMASKTEXTUREFRAME]->SetIntValue( 0 );

		if ( !params[RENDERFIXZ]->IsDefined() )
			params[RENDERFIXZ]->SetIntValue( 0 );

		if ( !params[USEALTERNATEVIEWMATRIX]->IsDefined() )
			params[USEALTERNATEVIEWMATRIX]->SetIntValue( 0 );

		if ( !params[ALTERNATEVIEWMATRIX]->IsDefined() )
		{
			VMatrix matIdentity;
			matIdentity.Identity();
			params[ALTERNATEVIEWMATRIX]->SetMatrixValue( matIdentity );
		}
	}

	SHADER_DRAW
	{
		bool bStaticBlendTexture = params[STATICBLENDTEXTURE]->IsTexture();
		bool bAlphaMaskTexture = ( params[ALPHAMASKTEXTURE]->IsTexture()? 1 : 0 );
		
		float fStaticAmount = params[STATICAMOUNT]->GetFloatValue();
		
		SHADOW_STATE
		{
			SetInitialShadowState();
			FogToFogColor();

			if( params[RENDERFIXZ]->GetIntValue() == 0 )
			{
				//pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL ); //a portal is effectively a decal on top of a wall
				pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_NEAREROREQUAL );
			}
			else
			{
				pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DISABLE );
				pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_ALWAYS );
				pShaderShadow->EnableDepthTest( false );
				pShaderShadow->EnableDepthWrites( false );
			}

			pShaderShadow->EnableAlphaTest( true );

			if( bAlphaMaskTexture )
			{
				pShaderShadow->EnableBlending( true );
				pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			}
			else
			{
				pShaderShadow->EnableBlending( false );
			}

			pShaderShadow->EnableSRGBWrite( true );

			int fmt = VERTEX_POSITION | VERTEX_NORMAL;
			int userDataSize = 0;
			int	iTexCoords = 1;
			if( IS_FLAG_SET( MATERIAL_VAR_MODEL ) )
			{
				userDataSize = 4;				
			}
			else
			{
				fmt |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
			}
			pShaderShadow->VertexShaderVertexFormat( fmt, iTexCoords, NULL, userDataSize );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

			if( bStaticBlendTexture || bAlphaMaskTexture )
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			if( bStaticBlendTexture && bAlphaMaskTexture )
				pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
			


		}

		Draw();		
	}

END_SHADER


