#version 450
// ParticleSphere_DX9's pixel stage: a port of stdshaders/particlesphere_ps2x.fxc
// (ps20b). Combos (fxctmp9/particlesphere_ps20b.inc): static CONVERT_TO_SRGB
// (2, always 0 here), DEPTHBLEND (4); dynamic PIXELFOGTYPE (1).
// @legacy program=particlesphere ps=particlesphere_ps20b vs=particlesphere_vs20 vert=particlesphere_vs20
//         samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BumpmapSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D DepthSampler;   // s1

layout( location = 0 ) in vec2 vBumpTexCoord;
layout( location = 1 ) in vec3 vTangentSpaceLightDirIn;
layout( location = 2 ) in vec3 vAmbientColor;
layout( location = 3 ) in vec4 vScreenPos;
layout( location = 7 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec4 vDirLightScale; // COLOR0

#define g_DepthFeatheringConstants PS_C( 0 )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

// common_ps_fxc.h's DepthFeathering (ps_2_b): the scene depth from the frame
// copy's destination alpha.
float DepthFeathering( vec2 vScreenPosIn, float fProjZ, float fProjW, vec4 vDepthBlendConstants )
{
	float flSceneDepth = tex2D( 1, DepthSampler, vScreenPosIn ).a;
	float flSpriteDepth = fProjZ * OO_DESTALPHA_DEPTH_RANGE; // SoftParticleDepth
	float flFeatheredAlpha = abs( flSceneDepth - flSpriteDepth ) * vDepthBlendConstants.x;
	flFeatheredAlpha = max( smoothstep( 0.75, 1.0, flSceneDepth ), flFeatheredAlpha );
	return saturate( flFeatheredAlpha );
}

void main()
{
	const bool DEPTHBLEND = STATIC_PS_COMBO( 4, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	vec4 baseColor = tex2D( 0, BumpmapSampler, vBumpTexCoord );

	// Dot the bump normal and the light vector.
	vec4 vBumpMapNormal = ( baseColor - 0.5 ); // -0.5 to 0.5.

	vec3 vTangentSpaceLightDir = ( vTangentSpaceLightDirIn - 0.5 ) * 2.0; // This is -1 to 1
	vec4 vOutput = vec4( dot( vBumpMapNormal.xyz, vTangentSpaceLightDir ) + 0.5 );

	// Scale by the light color outputted by the vertex shader (ie: based on distance).
	vOutput *= vDirLightScale;

	// Add ambient.
	vOutput += vec4( vAmbientColor.x, vAmbientColor.y, vAmbientColor.z, 0.0 );

	// Alpha = normal map alpha * vertex alpha
	vOutput.a = baseColor.a * vDirLightScale.a;

	// Soft Particles FTW
	if ( DEPTHBLEND )
		vOutput.a *= DepthFeathering( vScreenPos.xy / vScreenPos.w, vScreenPos.z, vScreenPos.w,
		    g_DepthFeatheringConstants );

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( vOutput, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
