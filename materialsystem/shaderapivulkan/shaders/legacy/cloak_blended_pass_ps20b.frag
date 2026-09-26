#version 450
// VertexLitGeneric's $cloakpassenabled pass: a port of
// stdshaders/cloak_blended_pass_ps2x.fxc (ps20b): the frame buffer refracted
// by the (bumped) normal, blurred and tinted, blended in by a fresnel cloak
// factor. Combos (fxctmp9/cloak_blended_pass_ps20b.inc): static
// CONVERT_TO_SRGB (1, always 0 here), BUMPMAP (2).
// @legacy program=cloak_blended_pass ps=cloak_blended_pass_ps20b
//         vs=cloak_blended_pass_vs20 vert=cloak_blended_pass_vs20 samplers=0:2d,1:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tRefractionSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D g_tBumpSampler;       // s1

layout( location = 0 ) in vec3 vWorldNormalIn;
layout( location = 1 ) in vec3 vProjPosForRefract;
layout( location = 2 ) in vec3 vWorldViewVector;
layout( location = 3 ) in vec3 mTangentSpaceTranspose0;
layout( location = 4 ) in vec3 mTangentSpaceTranspose1;
layout( location = 5 ) in vec3 mTangentSpaceTranspose2;
layout( location = 6 ) in vec2 vTexCoord0;

#define g_mViewProj0 PS_C( 0 )
#define g_mViewProj1 PS_C( 1 )
#define g_flCloakFactor ( PS_C( 6 ).x )
#define g_flRefractAmount ( PS_C( 6 ).y )
#define g_cCloakColorTint PS_C( 7 )

const vec4 g_vPoissonOffset[4] = vec4[4]( vec4( -0.0876, 0.9703, 0.5651, 0.4802 ),
    vec4( 0.1851, 0.1580, -0.0617, -0.2616 ), vec4( -0.5477, -0.6603, 0.0711, -0.5325 ),
    vec4( -0.0751, -0.8954, 0.4054, 0.6384 ) );

void main()
{
	const bool BUMPMAP = STATIC_PS_COMBO( 2, 2 ) != 0;

	vec3 vWorldNormal = normalize( vWorldNormalIn );
	if ( BUMPMAP )
	{
		const vec4 vBumpTexel = tex2D( 1, g_tBumpSampler, vTexCoord0 );
		const vec3 vTangentNormal = ( 2.0 * vBumpTexel.xyz ) - 1.0;
		vWorldNormal = vec3( dot( mTangentSpaceTranspose0, vTangentNormal ),
		    dot( mTangentSpaceTranspose1, vTangentNormal ),
		    dot( mTangentSpaceTranspose2, vTangentNormal ) );
	}

	const vec2 vProjNormal =
	    vec2( dot( vWorldNormal, g_mViewProj0.xyz ), dot( vWorldNormal, g_mViewProj1.xyz ) );

	const vec2 vRefractTexCoordNoWarp = vProjPosForRefract.xy / vProjPosForRefract.z;
	vec2 vRefractTexCoord = vProjNormal;
	const float scale = mix( g_flRefractAmount, 0.0, saturate( g_flCloakFactor ) );
	vRefractTexCoord *= scale;
	vRefractTexCoord += vRefractTexCoordNoWarp;

	const float flBlurAmount = mix( 0.05, 0.0, saturate( g_flCloakFactor ) );
	vec3 cRefract = tex2D( 0, g_tRefractionSampler, vRefractTexCoord ).rgb;
	for ( int i = 0; i < 4; ++i )
	{
		cRefract += tex2D( 0, g_tRefractionSampler,
		    vRefractTexCoord + ( g_vPoissonOffset[i].xy * flBlurAmount ) ).rgb;
		cRefract += tex2D( 0, g_tRefractionSampler,
		    vRefractTexCoord + ( g_vPoissonOffset[i].wz * flBlurAmount ) ).rgb;
	}
	cRefract /= 9.0;

	const float flFresnel = 1.0 - saturate( dot( vWorldNormalIn, normalize( -vWorldViewVector ) ) );
	float flCloakLerpFactor = saturate( mix( 1.0, flFresnel - 1.35, saturate( g_flCloakFactor ) ) );
	flCloakLerpFactor = 1.0 - smoothstep( 0.4, 0.425, flCloakLerpFactor );

	// A scalar in the range [0.8 1.2].
	cRefract *= mix( flFresnel * 0.4 + 0.8, 1.0, saturate( g_flCloakFactor ) * saturate( g_flCloakFactor ) );

	const float fColorTintStrength = saturate( ( saturate( g_flCloakFactor ) - 0.75 ) * 4.0 );
	cRefract *= mix( g_cCloakColorTint.rgb, vec3( 1.0 ), fColorTintStrength );

	LegacyWrite( FinalOutput(
	    vec4( cRefract, flCloakLerpFactor ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
