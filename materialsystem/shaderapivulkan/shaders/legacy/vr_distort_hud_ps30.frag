#version 450
// vr_distort_hud's pixel stage: a port of stdshaders/vr_distort_hud_ps2x.fxc
// (ps30, which the shader selects with shader model 3): the HUD texture mapped
// into DistortBounds (through the distortion map's per-channel coordinates with
// CMBO_HUDUNDISTORT), its edges smoothed to transparent black. Combos
// (fxctmp9/vr_distort_hud_ps30.inc): dynamic CMBO_HUDUNDISTORT (1).
// @legacy program=vr_distort_hud ps=vr_distort_hud_ps30 vs=vr_distort_hud_vs30
//         vert=vr_distort_hud_vs30 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;       // s0
layout( set = 0, binding = 1 ) uniform sampler2D DistortMapTextureSampler; // s1

layout( location = 0 ) in vec2 vBaseTexCoord;

#define DistortBounds PS_C( 0 )
// Declared int; the shader reads the float register.
#define bHudTranslucent ( PS_C( 1 ).x )

void main()
{
	const bool CMBO_HUDUNDISTORT = DYNAMIC_PS_COMBO( 1, 2 ) != 0;

	vec2 vOriginal = vBaseTexCoord.xy;

	// The base texture's 0..1 is mapped onto minUV..maxUV of the undistorted
	// frame, overlaying a higher-resolution inset.
	vec2 minUV = DistortBounds.xy;
	vec2 maxUV = DistortBounds.zw;
	vec2 scaleUV = 1.0 / ( maxUV - minUV );

	vec2 vGreen;
	vec4 vFinal;

	if ( CMBO_HUDUNDISTORT )
	{
		vec4 vRead = tex2D( 1, DistortMapTextureSampler, vOriginal );

		vec2 vRed = vRead.xy;
		vec2 vBlue = vRead.zw;

		vGreen = ( vRed + vBlue ) / 2.0;

		vRed = ( vRed - minUV ) * scaleUV;
		vGreen = ( vGreen - minUV ) * scaleUV;
		vBlue = ( vBlue - minUV ) * scaleUV;

		vFinal.r = tex2D( 0, BaseTextureSampler, vRed ).r;
		vFinal.ga = tex2D( 0, BaseTextureSampler, vGreen ).ga;
		vFinal.b = tex2D( 0, BaseTextureSampler, vBlue ).b;
	}
	else
	{
		vGreen = ( vOriginal - minUV ) * scaleUV;
		vFinal = tex2D( 0, BaseTextureSampler, vGreen );
	}

	// An opaque HUD's occasional non-unit alphas are fixed up.
	vFinal.a = mix( 1.0, vFinal.a, bHudTranslucent );

	// Smooth off the edges of the quad (and give ( 0, 0, 0, 0 ) outside).
	const float edgeRampFrac = 0.005;
	vec2 uvEdgeRamp = smoothstep( vec2( -edgeRampFrac, -edgeRampFrac ),
	                      vec2( edgeRampFrac, edgeRampFrac ), vGreen ) *
	                  ( 1.0 - smoothstep( vec2( 1.0 - edgeRampFrac, 1.0 - edgeRampFrac ),
	                              vec2( 1.0 + edgeRampFrac, 1.0 + edgeRampFrac ), vGreen ) );

	float edgeRamp = uvEdgeRamp.x * uvEdgeRamp.y;

	vFinal *= edgeRamp;

	LegacyWrite( vFinal );
}
