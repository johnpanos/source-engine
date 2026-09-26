#version 450
// A port of stdshaders/volume_clouds_vs20.fxc (VolumeClouds_dx9's vertex
// stage). Combos (fxctmp9/volume_clouds_vs20.inc): dynamic COMPRESSED_VERTS
// (1), SKINNING (2), both applied by the shader API's vertex record. Three
// texture layers rotating about the center by g_vTime, each with the
// tangent-space view vector rotated the same way. The binormal is
// cross( N, T ) * TANGENT.w of the unnormalized world normal and tangent.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec4 v2DTangentViewVector01;
layout( location = 1 ) out vec4 vUv01;
layout( location = 2 ) out vec4 v2DTangentViewVector2_vUv2;

#define g_vTime VS_C( 48 )
#define g_flTime1x ( g_vTime.x )
#define g_flTime2x ( g_vTime.y )
#define g_flTime4x ( g_vTime.z )

vec2 Rotate( vec2 v, vec4 mRotate )
{
	return vec2( dot( v, mRotate.xy ), dot( v, mRotate.zw ) );
}

vec4 RotationFor( float angle )
{
	vec4 mRotate;
	mRotate.x = cos( angle );
	mRotate.y = -sin( angle );
	mRotate.z = -mRotate.y;
	mRotate.w = mRotate.x;
	return mRotate;
}

void main()
{
	const vec3 vWorldPosition = inWorldPos;
	const vec3 worldNormal = inWorldNormal;
	const vec3 worldTangent = inWorldTangentS.xyz;
	const vec3 worldBinormal = cross( worldNormal, worldTangent ) * LegacyTangentSign();
	const vec3 vWorldNormal = normalize( worldNormal );
	const vec3 vWorldTangent = normalize( worldTangent );
	const vec3 vWorldBinormal = normalize( worldBinormal );

	gl_Position = LegacyProject( vWorldPosition );

	// View vector
	const vec3 vWorldViewVector = normalize( vWorldPosition - cEyePos );
	const vec3 vTangentViewVector = normalize( vec3( dot( vWorldViewVector, vWorldTangent ),
	    dot( vWorldViewVector, vWorldBinormal ), dot( vWorldViewVector, vWorldNormal ) ) );

	const vec2 vBaseUv = inTexCoord0;

	// Inner layer
	vec4 mRotate = RotationFor( g_flTime4x );
	vUv01.xy = Rotate( vBaseUv - 0.5, mRotate ) + 0.5;
	v2DTangentViewVector01.xy = Rotate( vTangentViewVector.xy, mRotate );

	// Middle layer
	mRotate = RotationFor( g_flTime2x );
	vUv01.wz = Rotate( vBaseUv - 0.5, mRotate ) + 0.5;
	v2DTangentViewVector01.wz = Rotate( vTangentViewVector.xy, mRotate );

	// Outer layer
	mRotate = RotationFor( g_flTime1x );
	v2DTangentViewVector2_vUv2.wz = Rotate( vBaseUv - 0.5, mRotate ) + 0.5;
	v2DTangentViewVector2_vUv2.xy = Rotate( vTangentViewVector.xy, mRotate );
}
