#version 450
// Teeth_DX9's flashlight pass: a port of stdshaders/teeth_flashlight_ps2x.fxc
// (ps20b). Combos (fxctmp9/teeth_flashlight_ps20b.inc): static CONVERT_TO_SRGB
// (4, always 0 here), FLASHLIGHTDEPTHFILTERMODE (8); dynamic PIXELFOGTYPE (1),
// FLASHLIGHTSHADOWS (2). FLASHLIGHTSHADOWS is never selected here (the backend
// has no shadow depth textures) and FLASHLIGHTDEPTHFILTERMODE only filters
// those shadows.
// @legacy program=teeth_flashlight ps=teeth_flashlight_ps20b vs=teeth_flashlight_vs20 vert=teeth_flashlight_vs20
//         samplers=0:2d,1:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D SpotSampler;        // s1

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 1 ) in vec4 spotTexCoord;
layout( location = 2 ) in vec3 vertAtten;
layout( location = 3 ) in vec4 projPos;
layout( location = 4 ) in vec3 worldPos;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos ( PS_C( PSREG_EYEPOS_SPEC_EXPONENT ).xyz )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	vec3 vProjCoords = spotTexCoord.xyz / spotTexCoord.w;
	vec3 result = tex2D( 1, SpotSampler, vProjCoords.xy ).rgb;

	result *= cFlashlightColor.rgb;
	result *= 0.35; // Without this, unshadowed teeth always seem to glow

	result *= vertAtten; // Distance atten, NdotL and forward vector

	vec4 baseSample = tex2D( 0, BaseTextureSampler, baseTexCoord );
	result *= baseSample.rgb; // Multiply by base map and diffuse

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos.z, worldPos.z, projPos.z );
	LegacyWrite( FinalOutput( vec4( result, baseSample.a ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
