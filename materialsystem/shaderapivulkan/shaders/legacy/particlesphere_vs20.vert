#version 450
// A port of stdshaders/particlesphere_vs20.fxc (ParticleSphere_DX9's vertex
// stage). Combos (fxctmp9/particlesphere_vs20.inc): dynamic FOGTYPE (1), which
// only feeds the unused fixed-function fog. The position is the POSITION
// stream (the particle system draws in view space with identity model and
// view matrices), projected by cModelViewProj.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vBumpTexCoord;
layout( location = 1 ) out vec3 vTangentSpaceLightDir;
layout( location = 2 ) out vec3 vAmbientColor;
layout( location = 3 ) out vec4 vScreenPos;
layout( location = 7 ) out vec4 worldPos_projPosZ;
layout( location = 8 ) out vec4 vDirLightScale; // COLOR0

#define g_vLightPosition VS_C( 48 )
#define g_vLightColor VS_C( 49 )             // range 0-1
#define g_flLightIntensity ( VS_C( 50 ).x ) // scales g_vLightColor

void main()
{
	vec4 vPos = vec4( inWorldPos, 1.0 );

	// Transform the input position: mul( v.vPos, cModelViewProj ).
	vec4 projPos = vec4( dot( vPos, VS_C( 4 ) ), dot( vPos, VS_C( 5 ) ), dot( vPos, VS_C( 6 ) ),
	    dot( vPos, VS_C( 7 ) ) );
	gl_Position = projPos;
	gl_ClipDistance[0] = dot( pc.clipPlanes[0], projPos );
	gl_ClipDistance[1] = dot( pc.clipPlanes[1], projPos );
	projPos.z = dot( vPos, VS_C( 12 ) ); // cModelViewProjZ

	vScreenPos.x = projPos.x;
	vScreenPos.y = -projPos.y; // invert Y
	vScreenPos.xy = ( vScreenPos.xy + projPos.w ) * 0.5;
	vScreenPos.z = projPos.z;
	vScreenPos.w = projPos.w;

	worldPos_projPosZ = vec4( vPos.xyz, projPos.z );

	// Copy texcoords over.
	vBumpTexCoord = inTexCoord0;

	// Copy the vertex color over.
	vAmbientColor = inColor;

	// This basis wants Z positive going into the screen so flip it here.
	vec4 vForward = normalize( vec4( vPos.x, vPos.y, -vPos.z, 1.0 ) );

	// This is the same as CrossProduct( vForward, Vector( 1, 0, 0 ) )
	vec4 vUp = normalize( vec4( 0.0, vForward.z, -vForward.y, vForward.w ) );

	// vRight = CrossProduct( vUp, vForward )
	vec4 vRight = vUp.yzxw * vForward.zxyw;
	vRight += -vUp.zxyw * vForward.yzxw;

	// Put the light in tangent space.
	vec4 vToLight = g_vLightPosition - vPos;
	vec4 vTangentSpaceLight = vRight * vToLight.x + vUp * vToLight.y + vForward * vToLight.z;

	// Output texcoord 1 holds the normalized transformed light direction.
	vTangentSpaceLightDir = normalize( vTangentSpaceLight ).xyz * 0.5 + 0.5; // make it 0-1 for the pixel shader

	// Handle oversaturation here.
	float flTransposedLenSqr = dot( vTangentSpaceLight, vTangentSpaceLight );
	float flScaledIntensity = g_flLightIntensity / flTransposedLenSqr;
	if ( flScaledIntensity > 1.0 )
		vDirLightScale.xyz = g_vLightColor.xyz;
	else
		vDirLightScale.xyz = g_vLightColor.xyz * flScaledIntensity;

	// Alpha comes right from the vertex color.
	vDirLightScale.a = inAlpha;
}
