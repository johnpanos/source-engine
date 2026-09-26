#version 450
// MotionBlur_dx9's pixel stage: a port of stdshaders/motion_blur_ps2x.fxc
// (ps20b). Combos (fxctmp9/motion_blur_ps20b.inc): static CONVERT_TO_SRGB (4,
// always 0 here); dynamic QUALITY (1, 0..3: 1, 7, 11 or 15 samples).
// @legacy program=motion_blur ps=motion_blur_ps20b vs=motion_blur_vs20 vert=motion_blur_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tTexSampler; // s0

layout( location = 0 ) in vec2 vUv0;

#define g_flMaxMotionBlur ( PS_C( 0 ).x )
#define g_vConst5 PS_C( 1 )
#define g_vGlobalBlurVector ( g_vConst5.xy )
#define g_flFallingMotionIntensity ( g_vConst5.z )
#define g_flRollBlurIntensity ( g_vConst5.w )

void main()
{
	const int QUALITY = DYNAMIC_PS_COMBO( 1, 4 );
	const int kNumSamples = QUALITY == 0 ? 1 : QUALITY == 1 ? 7 : QUALITY == 2 ? 11 : 15;

	// Calculate blur vector
	vec2 vFallingMotionBlurVector = ( vUv0.xy * 2.0 ) - 1.0;
	// cross( float3( v, 0 ), float3( 0, 0, 1 ) ).xy
	vec2 vRollBlurVector = vec2( vFallingMotionBlurVector.y, -vFallingMotionBlurVector.x );
	vec2 vGlobalBlurVector = g_vGlobalBlurVector;
	vGlobalBlurVector.y = -vGlobalBlurVector.y;

	float flFallingMotionBlurIntensity = -abs( g_flFallingMotionIntensity );
	vFallingMotionBlurVector.xy *= dot( vFallingMotionBlurVector.xy, vFallingMotionBlurVector.xy );
	vFallingMotionBlurVector.xy *= flFallingMotionBlurIntensity;

	float flRollBlurIntensity = g_flRollBlurIntensity;
	vRollBlurVector.xy *= flRollBlurIntensity;

	vec2 vFinalBlurVector = vGlobalBlurVector.xy + vFallingMotionBlurVector.xy + vRollBlurVector.xy;

	// Clamp length of blur vector
	if ( length( vFinalBlurVector.xy ) > g_flMaxMotionBlur )
		vFinalBlurVector.xy = normalize( vFinalBlurVector.xy ) * g_flMaxMotionBlur;

	vec4 cColor = vec4( 0.0 );
	// At QUALITY 0 the offset is 0 / 0, which fxc folds away with its one
	// sample at x = 0; the sample reads vUv0 itself (not a NaN coordinate).
	vec2 vUvOffset = kNumSamples > 1 ? vFinalBlurVector.xy / float( kNumSamples - 1 ) : vec2( 0.0 );
	for ( int x = 0; x < kNumSamples; x++ )
	{
		vec2 vUvTmp = vUv0.xy + ( vUvOffset.xy * float( x ) );
		cColor += ( 1.0 / float( kNumSamples ) ) * tex2D( 0, g_tTexSampler, vUvTmp );
	}

	LegacyWrite( FinalOutput( vec4( cColor.rgb, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
