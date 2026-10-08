//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "worldvertextransition_dx8_helper.h"
#include "BaseVSShader.h"



// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"




void DrawWorldVertexTransitionEditor_DX8( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, WorldVertexTransitionEditor_DX8_Vars_t &info )
{
	SHADOW_STATE
	{
		// This is the dx8 worldcraft version (non-bumped always.. too bad)
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );

		int fmt = VERTEX_POSITION | VERTEX_COLOR;
		pShaderShadow->VertexShaderVertexFormat( fmt, 2, 0, 0 );

	
		pShader->FogToFogColor();
	}
	pShader->Draw();
}
