#version 450
// A port of stdshaders/flesh_interior_blended_pass_vs20.fxc (VertexLitGeneric's
// $interior pass). Combos (fxctmp9/flesh_interior_blended_pass_vs20.inc):
// static HALFLAMBERT (96), USE_STATIC_CONTROL_FLOW (192); dynamic
// COMPRESSED_VERTS (1), SKINNING (2), DOWATERFOG (4), DYNAMIC_LIGHT (8),
// STATIC_LIGHT (16), NUM_LIGHTS (32, 0..2). The vertex record arrives skinned
// and flexed; its TANGENT is the user data stream, which the pass's format
// may leave out (as D3D9's declaration then does).
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"

layout( location = 0 ) out vec2 vTexCoord0;
layout( location = 1 ) out vec2 flDistanceToEffectCenter_flFresnelEffect;
layout( location = 2 ) out vec4 vNoiseTexCoord;
layout( location = 3 ) out vec3 vTangentViewVector;
layout( location = 4 ) out vec3 cVertexLight;
layout( location = 5 ) out vec3 mTangentSpaceTranspose0;
layout( location = 6 ) out vec3 mTangentSpaceTranspose1;
layout( location = 7 ) out vec3 mTangentSpaceTranspose2;

#define g_flNoiseUvScroll ( VS_C( 48 ).y )
#define g_flBorderNoiseScale ( VS_C( 48 ).z )
#define g_flDebugForceFleshOn ( VS_C( 48 ).w )
#define g_vEffectCenterOoRadius1 VS_C( 49 )
#define g_vEffectCenterOoRadius2 VS_C( 50 )
#define g_vEffectCenterOoRadius3 VS_C( 51 )
#define g_vEffectCenterOoRadius4 VS_C( 52 )

void main()
{
	const bool g_bHalfLambert = STATIC_VS_COMBO( 96, 2 ) != 0;
	const bool USE_STATIC_CONTROL_FLOW = STATIC_VS_COMBO( 192, 2 ) != 0;
	const bool bDynamicLight = DYNAMIC_VS_COMBO( 8, 2 ) != 0;
	const bool bStaticLight = DYNAMIC_VS_COMBO( 16, 2 ) != 0;
	const int NUM_LIGHTS = DYNAMIC_VS_COMBO( 32, 3 );

	const vec3 vWorldPosition = inWorldPos;
	const vec3 vWorldNormal = inWorldNormal;
	const vec3 vWorldTangent = inWorldTangentS.xyz;
	const vec3 vWorldBinormal = cross( vWorldNormal, vWorldTangent ) * LegacyTangentSign();

	gl_Position = LegacyProject( vWorldPosition );
	vTexCoord0 = inTexCoord0;

	// The closest effect intensity.
	float flDistance = 9999.0; // A very large distance
	flDistance = min( flDistance, length( vWorldPosition - g_vEffectCenterOoRadius1.xyz ) *
	                                  g_vEffectCenterOoRadius1.w );
	flDistance = min( flDistance, length( vWorldPosition - g_vEffectCenterOoRadius2.xyz ) *
	                                  g_vEffectCenterOoRadius2.w );
	flDistance = min( flDistance, length( vWorldPosition - g_vEffectCenterOoRadius3.xyz ) *
	                                  g_vEffectCenterOoRadius3.w );
	flDistance = min( flDistance, length( vWorldPosition - g_vEffectCenterOoRadius4.xyz ) *
	                                  g_vEffectCenterOoRadius4.w );
	if ( g_flDebugForceFleshOn != 0.0 )
		flDistance = 0.0;

	// Fresnel mask
	const vec3 vWorldViewVector = normalize( vWorldPosition - cEyePos );
	flDistanceToEffectCenter_flFresnelEffect =
	    vec2( flDistance, pow( saturate( dot( -vWorldViewVector, vWorldNormal ) ), 1.5 ) );

	// Noise UV
	vNoiseTexCoord.xy = vTexCoord0 * g_flBorderNoiseScale + g_flNoiseUvScroll;
	vNoiseTexCoord.zw = vTexCoord0 * g_flBorderNoiseScale - g_flNoiseUvScroll;

	// Tangent view vector (Vec3WorldToTangentNormalized)
	vTangentViewVector = normalize( vec3( dot( vWorldViewVector, vWorldTangent ),
	    dot( vWorldViewVector, vWorldBinormal ), dot( vWorldViewVector, vWorldNormal ) ) );

	if ( USE_STATIC_CONTROL_FLOW )
		cVertexLight = DoLighting(
		    vWorldPosition, vWorldNormal, vec3( 0.0 ), bStaticLight, bDynamicLight, g_bHalfLambert );
	else
		cVertexLight = DoLightingUnrolled( vWorldPosition, vWorldNormal, vec3( 0.0 ), bStaticLight,
		    bDynamicLight, g_bHalfLambert, NUM_LIGHTS );

	// Tangent space transform
	mTangentSpaceTranspose0 = vec3( vWorldTangent.x, vWorldBinormal.x, vWorldNormal.x );
	mTangentSpaceTranspose1 = vec3( vWorldTangent.y, vWorldBinormal.y, vWorldNormal.y );
	mTangentSpaceTranspose2 = vec3( vWorldTangent.z, vWorldBinormal.z, vWorldNormal.z );
}
