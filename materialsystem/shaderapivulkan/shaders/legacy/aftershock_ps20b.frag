#version 450
// Aftershock_dx9's pixel stage: a port of stdshaders/aftershock_ps2x.fxc
// (ps20b). Combos (fxctmp9/aftershock_ps20b.inc): static CONVERT_TO_SRGB (1,
// always 0 here). The frame copy refracted through two scrolling bump layers,
// blurred, tinted and outlined by the silhouette mask.
// @legacy program=aftershock ps=aftershock_ps20b vs=aftershock_vs20 vert=aftershock_vs20
//         samplers=0:2d,1:2d
#include "legacy_ps.glsl"
#include "legacy_ps_lighting.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tRefractionSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D g_tBumpSampler;       // s1

layout( location = 0 ) in vec3 vWorldNormalIn;
layout( location = 1 ) in vec3 vWorldTangent;
layout( location = 2 ) in vec3 vWorldBinormal;
layout( location = 3 ) in vec3 vProjPosForRefract;
layout( location = 4 ) in vec3 vWorldViewVector;
layout( location = 5 ) in vec4 vUv0;
layout( location = 6 ) in vec4 vUv1;

#define g_mViewProj0 PS_C( 0 ) // 1st row of matrix
#define g_mViewProj1 PS_C( 1 ) // 2nd row of matrix
#define g_vPackedConst6 PS_C( 6 )
#define g_flBlurAmount ( g_vPackedConst6.x )
#define g_flRefractAmount ( g_vPackedConst6.y )
#define g_cColorTint PS_C( 7 )
#define g_vPackedConst8 PS_C( 8 )
#define g_cSilhouetteColor g_vPackedConst8
#define g_flSilhouetteThickness ( g_vPackedConst8.w )
#define g_vGroundMinMax ( PS_C( 9 ).xy )

// 8 2D Poisson offsets (designed to use .xy and .wz swizzles (not .zw)
const vec4 g_vPoissonOffset[4] = vec4[4]( vec4( -0.0876, 0.9703, 0.5651, 0.4802 ),
    vec4( 0.1851, 0.1580, -0.0617, -0.2616 ), vec4( -0.5477, -0.6603, 0.0711, -0.5325 ),
    vec4( -0.0751, -0.8954, 0.4054, 0.6384 ) );

void main()
{
	// Bump layer 0
	vec4 vBumpTexel0 = tex2D( 1, g_tBumpSampler, vUv0.wz );
	vBumpTexel0 = tex2D( 1, g_tBumpSampler, vUv0.xy + ( vBumpTexel0.xy * 0.03 ) );

	// Bump layer 1
	vec4 vBumpTexel1 = tex2D( 1, g_tBumpSampler, vUv1.wz );
	vBumpTexel1 = tex2D( 1, g_tBumpSampler, vUv1.xy + ( vBumpTexel1.xy * 0.03 ) );

	// Combine bump layers into tangetn normal
	vec3 vTangentNormal = ( vBumpTexel0.xyz * 2.0 ) - 1.0;
	vTangentNormal.xyz += ( vBumpTexel1.xyz * 2.0 - 1.0 ) * 0.5;

	// Transform into world space
	vec3 vWorldNormal = Vec3TangentToWorld( vTangentNormal.xyz, vWorldNormalIn, vWorldTangent, vWorldBinormal );

	// Effect mask
	float flEffectMask = saturate( dot( -vWorldViewVector.xyz, vWorldNormalIn.xyz ) * ( ( 2.0 - g_flSilhouetteThickness ) ) );

	// Simulate ground intersection
	flEffectMask *= smoothstep( g_vGroundMinMax.x, g_vGroundMinMax.y, vWorldNormalIn.z );

	// Soften mask by squaring term
	flEffectMask *= flEffectMask;

	// Silhouette mask
	float flSilhouetteHighlightMask = saturate( flEffectMask * ( 1.0 - flEffectMask ) * 4.0 );
	flSilhouetteHighlightMask *= flSilhouetteHighlightMask * flSilhouetteHighlightMask;

	// Transform world space normal into clip space and project
	vec2 vProjNormal;
	vProjNormal.x = dot( vWorldNormal.xyz, g_mViewProj0.xyz ); // 1st row
	vProjNormal.y = dot( vWorldNormal.xyz, g_mViewProj1.xyz ); // 2nd row

	// Compute coordinates for sampling refraction
	vec2 vRefractTexCoordNoWarp = vProjPosForRefract.xy / vProjPosForRefract.z;
	vec2 vRefractTexCoord = vProjNormal.xy;
	float scale = mix( 0.0, g_flRefractAmount, flEffectMask );
	vRefractTexCoord.xy *= scale;
	vRefractTexCoord.xy += vRefractTexCoordNoWarp.xy;

	// Blur by scalable Poisson filter
	float flBlurAmount = g_flBlurAmount * flEffectMask;
	vec3 cRefract = tex2D( 0, g_tRefractionSampler, vRefractTexCoord.xy ).rgb;
	for ( int n = 0; n < 4; ++n )
	{
		cRefract += tex2D( 0, g_tRefractionSampler, vRefractTexCoord.xy + ( g_vPoissonOffset[n].xy * flBlurAmount ) ).rgb;
		cRefract += tex2D( 0, g_tRefractionSampler, vRefractTexCoord.xy + ( g_vPoissonOffset[n].wz * flBlurAmount ) ).rgb;
	}
	cRefract /= 9.0;

	// Undo tone mapping
	cRefract /= LINEAR_LIGHT_SCALE;

	// Refract color tint
	float fColorTintStrength = 1.0 - flEffectMask;
	vec3 cRefractColorTint = mix( g_cColorTint.rgb, vec3( 1.0 ), fColorTintStrength );

	// Combine terms
	vec4 result;
	result.rgb = cRefract.rgb * cRefractColorTint.rgb;
	result.rgb += result.rgb * ( flSilhouetteHighlightMask * g_cSilhouetteColor.rgb );

	// Set alpha to...
	result.a = flEffectMask;

	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_LINEAR ) );
}
