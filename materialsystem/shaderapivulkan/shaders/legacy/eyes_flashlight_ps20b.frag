#version 450
// Eyes_dx9's flashlight pass: a port of stdshaders/eyes_flashlight_ps2x.fxc
// (eyes_flashlight_inc.fxc, ps20b). Combos (fxctmp9/eyes_flashlight_ps20b.inc):
// static FLASHLIGHTDEPTHFILTERMODE (4); dynamic PIXELFOGTYPE (1),
// FLASHLIGHTSHADOWS (2). FLASHLIGHTSHADOWS is never selected here (the backend
// has no shadow depth textures) and FLASHLIGHTDEPTHFILTERMODE only filters
// those shadows.
// @legacy program=eyes_flashlight ps=eyes_flashlight_ps20b vs=eyes_flashlight_vs20 vert=eyes_flashlight_vs20
//         samplers=0:2d,1:2d,3:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D SpotSampler;        // s0
layout( set = 0, binding = 1 ) uniform sampler2D BaseTextureSampler; // s1
layout( set = 0, binding = 2 ) uniform sampler2D IrisSampler;        // s3

layout( location = 0 ) in vec4 spotTexCoord;
layout( location = 1 ) in vec2 baseTexCoord;
layout( location = 3 ) in vec2 irisTexCoord;
layout( location = 4 ) in vec3 vertAtten;
layout( location = 5 ) in vec3 worldPos;
layout( location = 7 ) in vec3 projPos;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	vec3 vProjCoords = spotTexCoord.xyz / spotTexCoord.w;
	vec3 spotColor = tex2D( 0, SpotSampler, vProjCoords.xy ).rgb * cFlashlightColor.rgb;

	vec4 baseSample = tex2D( 1, BaseTextureSampler, baseTexCoord );
	vec4 irisSample = tex2D( 2, IrisSampler, irisTexCoord );

	vec3 outcolor = vec3( 1.0 );
	if ( spotTexCoord.w <= 0.0 )
		outcolor = vec3( 0.0 );

	// Composite the iris and sclera together
	vec3 albedo = mix( baseSample.xyz, irisSample.xyz * 0.5, irisSample.a ); // dim down the iris in HDR

	outcolor *= spotColor * albedo;

	// NOTE!!  This has to be last to avoid loss of range.
	outcolor *= vertAtten;

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z, worldPos.z, projPos.z );
	LegacyWrite( FinalOutput( vec4( outcolor, 1.0 ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
