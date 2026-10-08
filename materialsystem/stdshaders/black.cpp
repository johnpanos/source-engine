//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Black: an opaque surface drawn black and fogged to the fog color.
//          Portal 2's materials use it for distant set dressing that only
//          reads as a fogged silhouette (sp_a1_wakeup's destroyed chamber
//          pieces, models/props_hub/glados_chamber_dest01). Without it the
//          material system replaces the unknown shader with the wireframe
//          error shader. Ported from the CS:GO-era shader (fog to fog color,
//          no parameters) onto this engine's pixel fog: the pixel stage blends
//          black to the fog color by the squared range fog factor, which is
//          what the original's vertex stage computed.
//
//=============================================================================//

#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER( Black, "Help for Black" )
BEGIN_SHADER_PARAMS
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
}

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT
{
}

SHADER_DRAW
{
	SHADOW_STATE
	{
		// Positions only (the shader supports compression). One texture
		// coordinate although none is read: a narrower vertex format is
		// padded with a warning.
		unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, NULL, userDataSize );


		if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
		{
		}
		else
		{
		}

		pShaderShadow->EnableSRGBWrite( true );

		FogToFogColor();
	}
	Draw();
}
END_SHADER
