//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#include "BaseVSShader.h"

#include "convar.h"

static ConVar mat_use_compressed_hdr_textures( "mat_use_compressed_hdr_textures", "1" );

DEFINE_FALLBACK_SHADER( Sky, Sky_HDR_DX9 )

BEGIN_VS_SHADER( Sky_HDR_DX9, "Help for Sky_HDR_DX9 shader" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( HDRBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "base texture when running with HDR enabled" )
		SHADER_PARAM( HDRCOMPRESSEDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "base texture (compressed) for hdr compression method A" )
		SHADER_PARAM( HDRCOMPRESSEDTEXTURE0, SHADER_PARAM_TYPE_TEXTURE, "", "compressed base texture0 for hdr compression method B" )
		SHADER_PARAM( HDRCOMPRESSEDTEXTURE1, SHADER_PARAM_TYPE_TEXTURE, "", "compressed base texture1 for hdr compression method B" )
		SHADER_PARAM( HDRCOMPRESSEDTEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "compressed base texture2 for hdr compression method B" )
		SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_VEC3, "[ 1 1 1]", "color multiplier", SHADER_PARAM_NOT_EDITABLE )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 || g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
		{
			return "Sky_DX9";
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
		// First figure out if sampler zero wants to be sRGB
		int nSamplerZeroFlags = 0;
		if ( (params[HDRCOMPRESSEDTEXTURE]->IsDefined()) &&	mat_use_compressed_hdr_textures.GetBool() )
		{
			nSamplerZeroFlags = 0;
		}
		else
		{
			if (params[HDRCOMPRESSEDTEXTURE0]->IsDefined())
			{
				nSamplerZeroFlags = 0;
			}
			else
			{
				nSamplerZeroFlags = TEXTUREFLAGS_SRGB;

				if ( params[HDRBASETEXTURE]->IsDefined() && params[HDRBASETEXTURE]->IsTexture() )
				{
					ITexture *txtr=params[HDRBASETEXTURE]->GetTextureValue();
					ImageFormat fmt=txtr->GetImageFormat();
					if ( ( fmt == IMAGE_FORMAT_RGBA16161616F ) || ( fmt == IMAGE_FORMAT_RGBA16161616 ) )
					{
						nSamplerZeroFlags = 0;
					}
				}
			}
		}

		// Next, figure out which texture will be on sampler zero
		int nSampler0 = HDRCOMPRESSEDTEXTURE;
		if ( params[HDRCOMPRESSEDTEXTURE]->IsDefined() && mat_use_compressed_hdr_textures.GetBool() )
		{
			nSampler0 = HDRCOMPRESSEDTEXTURE;
		}
		else
		{
			if ( params[HDRCOMPRESSEDTEXTURE0]->IsDefined() )
			{
				nSampler0 = HDRCOMPRESSEDTEXTURE0;
			}
			else
			{
				nSampler0 = HDRBASETEXTURE;
			}
		}

		// Load the appropriate textures, making sure that the texture set on sampler 0 is sRGB if necessary
		if ( params[HDRCOMPRESSEDTEXTURE]->IsDefined() && (mat_use_compressed_hdr_textures.GetBool() ) )
		{
			LoadTexture( HDRCOMPRESSEDTEXTURE, HDRCOMPRESSEDTEXTURE == nSampler0 ? nSamplerZeroFlags : 0 );
		}
		else
		{
			if (params[HDRCOMPRESSEDTEXTURE0]->IsDefined())
			{
				LoadTexture( HDRCOMPRESSEDTEXTURE0, HDRCOMPRESSEDTEXTURE0 == nSampler0 ? nSamplerZeroFlags : 0 );
				if ( params[HDRCOMPRESSEDTEXTURE1]->IsDefined() )
				{
					LoadTexture( HDRCOMPRESSEDTEXTURE1, HDRCOMPRESSEDTEXTURE1 == nSampler0 ? nSamplerZeroFlags : 0 );
				}
				if ( params[HDRCOMPRESSEDTEXTURE2]->IsDefined())
				{
					LoadTexture( HDRCOMPRESSEDTEXTURE2, HDRCOMPRESSEDTEXTURE2 == nSampler0 ? nSamplerZeroFlags : 0 );
				}
			}
			else
			{
				if ( params[HDRBASETEXTURE]->IsDefined() )
				{
					LoadTexture( HDRBASETEXTURE, HDRBASETEXTURE == nSampler0 ? nSamplerZeroFlags : 0 );
				}
			}
		}
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			SetInitialShadowState();

//			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, NULL, 0 );


			if ( (params[HDRCOMPRESSEDTEXTURE]->IsDefined()) &&
				 mat_use_compressed_hdr_textures.GetBool() )
			{
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0,false);
				if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				{
				}
				else
				{
				}
			}
			else
			{
				if (params[HDRCOMPRESSEDTEXTURE0]->IsDefined())
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
					pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
					
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0,false);
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1,false);
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2,false);
					if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
					{
					}
					else
					{
					}
				}
				else
				{
					ITexture *txtr=params[HDRBASETEXTURE]->GetTextureValue();
					ImageFormat fmt=txtr->GetImageFormat();
					if ((fmt==IMAGE_FORMAT_RGBA16161616F) || (fmt==IMAGE_FORMAT_RGBA16161616))
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0,false);
					else
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0,true);
					
					if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
					{
					}
					else
					{
					}
				}
			}
			// we are writing linear values from this shader.
			pShaderShadow->EnableSRGBWrite( true );

			pShaderShadow->EnableAlphaWrites( true );
		}

		Draw( );
	}
END_SHADER
