//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A wet version of base * lightmap
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"

#include "cpp_shader_constant_register_map.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar mat_fullbright;

DEFINE_FALLBACK_SHADER( Cable, Cable_DX9 )

// Portal 2's cables (materials/cable/*.vmt) name SplineRope, whose vertex
// shader expands a Catmull-Rom spline from four control points per vertex.
// This client's C_RopeKeyframe builds camera-facing rope geometry on the CPU
// in Cable's vertex format, and SplineRope's pixel shader is Cable's: a
// half-Lambert normal-map term times base texture and vertex color. So the
// SplineRope name draws with Cable; without it every rope material is the
// error material and no cable is drawn.
DEFINE_FALLBACK_SHADER( SplineRope, Cable_DX9 )

// Keep the declarative shader macro intact: clang-format parses BEGIN_VS_SHADER
// as an unterminated call and cannot determine this generated class's indentation.
// clang-format off
BEGIN_VS_SHADER( Cable_DX9, 
			  "Help for Cable shader" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "cable/cablenormalmap", "bumpmap texture" )
		SHADER_PARAM( MINLIGHT, SHADER_PARAM_TYPE_FLOAT, "0.1", "Minimum amount of light (0-1 value)" )
		SHADER_PARAM( MAXLIGHT, SHADER_PARAM_TYPE_FLOAT, "0.3", "Maximum amount of light" )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if ( !(g_pHardwareConfig->SupportsPixelShaders_2_0() && g_pHardwareConfig->SupportsVertexShaders_2_0()) ||
				(g_pHardwareConfig->GetDXSupportLevel() < 90) )
		{
			return "Cable_DX8";
		}
		return 0;
	}

	SHADER_INIT
	{
		// Declared texture defaults are descriptive; InitShaderParameters does
		// not assign them. Populate Cable's normal input before LoadBumpMap.
		if ( !params[BUMPMAP]->IsDefined() )
			params[BUMPMAP]->SetStringValue( GetParamDefault( BUMPMAP ) );
		LoadBumpMap( BUMPMAP );
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
	}

	SHADER_DRAW
	{
		BlendType_t nBlendType = EvaluateBlendRequirements( BASETEXTURE, true );
		bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !IS_FLAG_SET(MATERIAL_VAR_ALPHATEST); //dest alpha is free for special use

		SHADOW_STATE
		{
			// Enable blending?
			if ( IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) )
			{
				pShaderShadow->EnableDepthWrites( false );
				pShaderShadow->EnableBlending( true );
				pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			}

			pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			if ( g_pHardwareConfig->GetDXSupportLevel() >= 90)
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );
			}
			
			int tCoordDimensions[] = {2,2};
			pShaderShadow->VertexShaderVertexFormat( 
				VERTEX_POSITION | VERTEX_COLOR | VERTEX_TANGENT_S | VERTEX_TANGENT_T, 
				2, tCoordDimensions, 0 );


			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
			}
			else
			{
			}

			// we are writing linear values from this shader.
			// This is kinda wrong.  We are writing linear or gamma depending on "IsHDREnabled" below.
			// The COLOR really decides if we are gamma or linear.  
			pShaderShadow->EnableSRGBWrite( true );

			FogToFogColor();

			pShaderShadow->EnableAlphaWrites( bFullyOpaque );
		}
		Draw();
	}
END_SHADER
// clang-format on
