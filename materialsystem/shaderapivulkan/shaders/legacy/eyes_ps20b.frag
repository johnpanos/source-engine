#version 450
// Eyes_dx9's pixel stage: a port of stdshaders/eyes_ps2x.fxc (ps20b). Combos
// (fxctmp9/eyes_ps20b.inc): dynamic WRITE_DEPTH_TO_DESTALPHA (1),
// PIXELFOGTYPE (2).
// @legacy program=eyes ps=eyes_ps20b vs=eyes_vs20 vert=eyes_vs20
//         samplers=0:2d,1:2d,2:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D IrisSampler;        // s1
layout( set = 0, binding = 2 ) uniform sampler2D GlintSampler;       // s2

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 1 ) in vec2 irisTexCoord;
layout( location = 2 ) in vec2 glintTexCoord;
layout( location = 3 ) in vec3 vertAtten;
layout( location = 7 ) in vec4 worldPos_projPosZ;

#define cEyeScalars PS_C( 0 ) // { Dilation, ambient, x, x }
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )
#define fGlintDamping ( cEyeScalars.y )

void main()
{
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 1, 2 );
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );

	vec4 baseSample = tex2D( 0, BaseTextureSampler, baseTexCoord );
	vec4 glintSample = tex2D( 2, GlintSampler, glintTexCoord );
	vec4 irisSample = tex2D( 1, IrisSampler, irisTexCoord );

	vec4 result;
	result.rgb = mix( baseSample.rgb, irisSample.rgb, irisSample.a );
	result.rgb *= vertAtten;
	result.rgb += glintSample.rgb * fGlintDamping;
	result.a = baseSample.a;

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( result, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR,
	    WRITE_DEPTH_TO_DESTALPHA != 0, worldPos_projPosZ.w ) );
}
