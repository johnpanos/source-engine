#version 450
// Portal_DX90's pixel stage: a port of stdshaders/portal_ps2x.fxc (ps20b).
// The portal (or frame buffer) texture at the projected screen position,
// darkened by the static amount and mixed with the static texture (or gray),
// with the alpha mask's alpha. Combos (fxctmp9/portal_ps20b.inc): static
// CONVERT_TO_SRGB (8, always 0 here), HASALPHAMASK (16), HASSTATICTEXTURE
// (32); dynamic ADDSTATIC (1), HDRENABLED (2, unused by the shader),
// PIXELFOGTYPE (4).
// @legacy program=portal ps=portal_ps20b vs=portal_vs20 vert=portal_vs20
//         samplers=0:2d,1:2d,2:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D PortalSampler;    // s0
layout( set = 0, binding = 1 ) uniform sampler2D SecondarySampler; // s1
layout( set = 0, binding = 2 ) uniform sampler2D TertiarySampler;  // s2

layout( location = 0 ) in vec3 vPortalTexCoord;
layout( location = 1 ) in vec2 vSecondaryTexCoord;
layout( location = 2 ) in vec2 vTertiaryTexCoord;
layout( location = 7 ) in vec4 worldPos_projPosZ;

#define g_StaticAmount PS_C( 0 ) // x is static, y is 1.0 - static
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const bool HASALPHAMASK = STATIC_PS_COMBO( 16, 2 ) != 0;
	const bool HASSTATICTEXTURE = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool ADDSTATIC = DYNAMIC_PS_COMBO( 1, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 4, 2 );

	vec4 result;
	result.rgb = tex2D( 0, PortalSampler, vPortalTexCoord.xy / vPortalTexCoord.z ).rgb;

	// mix in static
	if ( ADDSTATIC )
	{
		result.rgb *= g_StaticAmount.y; // inverse static on original colors
		if ( HASSTATICTEXTURE )
		{
			if ( HASALPHAMASK )
				result.rgb += tex2D( 2, TertiarySampler, vTertiaryTexCoord ).rgb * g_StaticAmount.x;
			else
				result.rgb += tex2D( 1, SecondarySampler, vSecondaryTexCoord ).rgb * g_StaticAmount.x;
		}
		else
		{
			result.rgb += g_StaticAmount.x * 0.25; // mix in gray
		}
	}

	// alpha mask
	if ( HASALPHAMASK )
		result.a = tex2D( 1, SecondarySampler, vSecondaryTexCoord ).a;
	else
		result.a = 1.0;

	const float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( result, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
