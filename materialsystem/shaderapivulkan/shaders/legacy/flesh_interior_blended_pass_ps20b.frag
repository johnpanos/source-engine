#version 450
// VertexLitGeneric's $interior pass (the flesh opening): a port of
// stdshaders/flesh_interior_blended_pass_ps2x.fxc (ps20b). Combos
// (fxctmp9/flesh_interior_blended_pass_ps20b.inc): static CONVERT_TO_SRGB (1,
// always 0 here).
// @legacy program=flesh_interior_blended_pass ps=flesh_interior_blended_pass_ps20b
//         vs=flesh_interior_blended_pass_vs20 vert=flesh_interior_blended_pass_vs20
//         samplers=0:2d,1:2d,2:2d,3:2d,4:2d,5:cube
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tBaseSampler;       // s0
layout( set = 0, binding = 1 ) uniform sampler2D g_tNoiseSampler;      // s1
layout( set = 0, binding = 2 ) uniform sampler2D g_tBorder1DSampler;   // s2
layout( set = 0, binding = 3 ) uniform sampler2D g_tNormalSampler;     // s3
layout( set = 0, binding = 4 ) uniform sampler2D g_tSubsurfaceSampler; // s4
layout( set = 0, binding = 5 ) uniform samplerCube g_tCubeSampler;     // s5

layout( location = 0 ) in vec2 vTexCoord0;
layout( location = 1 ) in vec2 flDistanceToEffectCenter_flFresnelEffect;
layout( location = 2 ) in vec4 vNoiseTexCoord;
layout( location = 3 ) in vec3 vTangentViewVector;
layout( location = 4 ) in vec3 cVertexLight;
layout( location = 5 ) in vec3 mTangentSpaceTranspose0;
layout( location = 6 ) in vec3 mTangentSpaceTranspose1;
layout( location = 7 ) in vec3 mTangentSpaceTranspose2;

#define g_cSubsurfaceTint ( PS_C( 0 ).rgb )
#define g_flBorderWidth ( PS_C( 1 ).xy )
#define g_flBorderSoftness ( PS_C( 2 ).x )
#define g_cBorderTint ( PS_C( 3 ).rgb )
#define g_flGlobalOpacity ( PS_C( 4 ).x )
#define g_flGlossBrightness ( PS_C( 5 ).x )

void main()
{
	// Color texture
	vec4 cBaseColor = tex2D( 0, g_tBaseSampler, vTexCoord0 );
	const float flFleshMaskFromTexture = cBaseColor.a;

	// Subsurface colors
	const vec4 cSubsurfaceColor = tex2D( 4, g_tSubsurfaceSampler, vTexCoord0 );
	cBaseColor.rgb += cBaseColor.rgb * cSubsurfaceColor.rgb * g_cSubsurfaceTint;

	// Scroll noise textures to ripple border of opening
	const float flNoise0 = tex2D( 1, g_tNoiseSampler, vNoiseTexCoord.xy ).g;
	const float flNoise1 = tex2D( 1, g_tNoiseSampler, vNoiseTexCoord.wz ).g;
	const float flNoise = ( flNoise0 + flNoise1 ) * 0.5;

	// Generate 0-1 mask from distance computed in the VS
	float flClampedInputMask = 1.0 - saturate( flDistanceToEffectCenter_flFresnelEffect.x );
	flClampedInputMask *= flDistanceToEffectCenter_flFresnelEffect.y;
	flClampedInputMask *= flFleshMaskFromTexture;

	// Noise mask - Only apply noise around border of sphere
	const float flBorderMask =
	    saturate( ( 1.0 - flClampedInputMask ) * g_flBorderWidth.x - g_flBorderWidth.y );
	const float flNoiseMask = 1.0 - abs( ( flBorderMask * 2.0 ) - 1.0 );

	// This is used to lerp in the 1D border texture over the flesh color
	const float flBorderMaskWithNoise =
	    ( 1.0 - smoothstep( flNoiseMask - g_flBorderSoftness, flNoiseMask + g_flBorderSoftness,
	                flNoise ) ) *
	    flNoiseMask;

	// Border color
	// pow( x, 4 ), which fxc compiles as two squarings (and pow( x, 2 ) and
	// pow( x, 8 ) below as one and three).
	const float flMask2 = flBorderMaskWithNoise * flBorderMaskWithNoise;
	const float vBorderUv = ( sign( flBorderMask - 0.5 ) * ( 1.0 - flMask2 * flMask2 ) * 0.5 ) + 0.5;
	vec4 cBorderColor = 2.0 * tex2D( 2, g_tBorder1DSampler, vec2( vBorderUv ) );
	cBorderColor.rgb *= g_cBorderTint;
	cBorderColor.rgb *= flNoise;

	// Normal map
	const vec4 vNormalMapValue = tex2D( 3, g_tNormalSampler, vTexCoord0 );
	vec3 vTangentNormal = ( vNormalMapValue.xyz * 2.0 ) - 1.0;
	vTangentNormal.xy += ( flNoise * 1.5 ) - 0.75; // NOTE: This will denormalize the normal.

	// Specular gloss layer
	const vec3 vTangentReflectionVector = reflect( vTangentViewVector, vTangentNormal );
	const vec3 vWorldReflectionVector = vec3( dot( mTangentSpaceTranspose0, vTangentReflectionVector ),
	    dot( mTangentSpaceTranspose1, vTangentReflectionVector ),
	    dot( mTangentSpaceTranspose2, vTangentReflectionVector ) );
	vec3 cGlossLayer = ENV_MAP_SCALE * texCUBE( 5, g_tCubeSampler, vWorldReflectionVector ).rgb;
	cGlossLayer *= g_flGlossBrightness;

	// Gloss mask is just hard-coded fresnel for now
	const float flGloss = saturate( dot( vTangentNormal, -vTangentViewVector ) );
	const float flGloss2 = flGloss * flGloss;
	const float flGloss4 = flGloss2 * flGloss2;
	const float flGlossMask = flGloss4 * flGloss4;

	// Opacity
	float flOpacity = max( flBorderMaskWithNoise, step( flBorderMask, 0.5 ) );
	flOpacity *= g_flGlobalOpacity;

	vec4 result;
	result.rgb = cBaseColor.rgb * cVertexLight;
	result.rgb += cGlossLayer * flGlossMask;
	const float flDarken = 1.0 - flBorderMaskWithNoise;
	result.rgb *= flDarken * flDarken; // Darken near border
	result.rgb = mix( result.rgb, cBorderColor.rgb, saturate( vBorderUv * 2.0 ) );
	result.a = flOpacity;

	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_LINEAR ) );
}
