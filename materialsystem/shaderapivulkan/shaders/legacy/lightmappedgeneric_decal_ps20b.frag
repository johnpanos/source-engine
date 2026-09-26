#version 450
// DecalBaseTimesLightmapAlphaBlendSelfIllum's first pass: a port of
// stdshaders/lightmappedgeneric_decal_ps2x.fxc (ps20b). The three bumped
// lightmap pages weighted by the squared basis z (c0..c2), times the decal
// texture, the modulation color and the vertex color. Combos
// (fxctmp9/lightmappedgeneric_decal_ps20b.inc): static CONVERT_TO_SRGB (2,
// always 0 here); dynamic PIXELFOGTYPE (1).
// @legacy program=lightmappedgeneric_decal ps=lightmappedgeneric_decal_ps20b
//         vs=lightmappedgeneric_decal_vs20 vert=lightmappedgeneric_decal_vs20
//         samplers=0:2d,1:2d,2:2d,3:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D LightMap0Sampler;   // s1
layout( set = 0, binding = 2 ) uniform sampler2D LightMap1Sampler;   // s2
layout( set = 0, binding = 3 ) uniform sampler2D LightMap2Sampler;   // s3

layout( location = 0 ) in vec2 vTexCoord0;
layout( location = 1 ) in vec2 vTexCoord1;
layout( location = 2 ) in vec2 vTexCoord2;
layout( location = 3 ) in vec2 vTexCoord3;
layout( location = 4 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec4 vColor;

#define g_LightMap0Color PS_C( 0 )
#define g_LightMap1Color PS_C( 1 )
#define g_LightMap2Color PS_C( 2 )
#define g_ModulationColor PS_C( 3 )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	// output = lightmapColor[0] * ( ( N dot basis[0] )^2 ) +
	//	       lightmapColor[1] * ( ( N dot basis[1] )^2 ) +
	//	       lightmapColor[2] * ( ( N dot basis[2] )^2 ) +
	vec4 resultColor = tex2D( 1, LightMap0Sampler, vTexCoord1 ) * g_LightMap0Color;
	resultColor = ( tex2D( 2, LightMap1Sampler, vTexCoord2 ) * g_LightMap1Color ) + resultColor;
	resultColor = ( tex2D( 3, LightMap2Sampler, vTexCoord3 ) * g_LightMap2Color ) + resultColor;

	// Modulate by decal texture
	const vec4 decalColor = tex2D( 0, BaseTextureSampler, vTexCoord0 );
	resultColor.rgb = resultColor.rgb * decalColor.rgb;
	resultColor.a = decalColor.a;

	// Modulate by constant color
	resultColor = resultColor * g_ModulationColor;

	// Modulate by per-vertex factor
	resultColor = resultColor * vColor;

	const float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( resultColor, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
