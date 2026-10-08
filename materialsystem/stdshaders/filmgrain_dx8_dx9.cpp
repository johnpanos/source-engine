//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "BaseVSShader.h"


#include "../materialsystem_global.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( FilmGrain, FilmGrain_dx9 )
BEGIN_VS_SHADER_FLAGS( FilmGrain_dx9, "Help for FilmGrain", SHADER_NOT_EDITABLE )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( GRAIN_TEXTURE,  SHADER_PARAM_TYPE_TEXTURE, "0", "Film grain texture" )
		SHADER_PARAM( NOISESCALE,     SHADER_PARAM_TYPE_VEC4, "", "Strength of film grain" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( GRAIN_TEXTURE );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_ONE, SHADER_BLEND_SRC_ALPHA );
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );



		}
		Draw();
	}
END_SHADER
