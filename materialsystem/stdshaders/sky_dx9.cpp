//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#include "BaseVSShader.h"
#include "sky_vs20.inc"
#include "sky_ps20.inc"
#include "sky_ps20b.inc"

#include "convar.h"

BEGIN_VS_SHADER( Sky_DX9, "Help for Sky_DX9 shader" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_VEC3, "[ 1 1 1]", "color multiplier", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "sky_dx6";
		}
		return 0;
	}

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS( MATERIAL_VAR_NOFOG );
		SET_FLAGS( MATERIAL_VAR_IGNOREZ );
	}
	SHADER_INIT
	{
		if (params[BASETEXTURE]->IsDefined())
		{
			ImageFormat fmt = params[BASETEXTURE]->GetTextureValue()->GetImageFormat();
			LoadTexture( BASETEXTURE, (fmt==IMAGE_FORMAT_RGBA16161616F) || (fmt==IMAGE_FORMAT_RGBA16161616) ? 0 : TEXTUREFLAGS_SRGB );
		}
	}
	SHADER_DRAW
	{
		SHADOW_STATE
		{
			SetInitialShadowState();

//			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			ITexture *txtr=params[BASETEXTURE]->GetTextureValue();
			ImageFormat fmt=txtr->GetImageFormat();
			if ((fmt==IMAGE_FORMAT_RGBA16161616F) || (fmt==IMAGE_FORMAT_RGBA16161616))
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0,false);
			else
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0,true);

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, NULL, 0 );

			DECLARE_STATIC_VERTEX_SHADER( sky_vs20 );
			SET_STATIC_VERTEX_SHADER( sky_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( sky_ps20b );
				SET_STATIC_PIXEL_SHADER( sky_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( sky_ps20 );
				SET_STATIC_PIXEL_SHADER( sky_ps20 );
			}
			// we are writing linear values from this shader.
			pShaderShadow->EnableSRGBWrite( true );

			pShaderShadow->EnableAlphaWrites( true );
		}

		Draw( );
	}

END_SHADER

