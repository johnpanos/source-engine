#version 450
// warp's pixel stage: a port of stdshaders/warp_ps2x.fxc (ps30, which the shader
// selects with shader model 3): the HMD lens warp of a side-by-side stereo
// frame, each eye's half distorted about its centre with per-channel
// coefficients from the HMD warp rendering parameters. Combos
// (fxctmp9/warp_ps30.inc): dynamic DISTORT_TYPE (1, 0..2: none, 1 / poly,
// poly).
// @legacy program=warp ps=warp_ps30 vs=warp_vs30 vert=warp_vs30 samplers=0:2d
//         flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec2 vBaseTexCoord;

#define g_vWarpParms0 PS_C( 0 )
#define stereo_distortion_ScaleX ( g_vWarpParms0.x )
#define stereo_distortion_OffsetX ( g_vWarpParms0.y )
#define stereo_distortion_ScaleY ( g_vWarpParms0.z )
#define stereo_distortion_OffsetY ( g_vWarpParms0.w )

#define g_vWarpParms1 PS_C( 1 )
#define distortion_l_centre ( g_vWarpParms1.xy )
#define distortion_r_centre ( g_vWarpParms1.zw )

#define g_vWarpParms2 PS_C( 2 )
#define distortion_l_coeff0 ( g_vWarpParms2.x )
#define distortion_l_coeff1 ( g_vWarpParms2.y )
#define distortion_l_coeff2 ( g_vWarpParms2.z )
#define distortion_red_coeff0delta ( g_vWarpParms2.w )

#define g_vWarpParms3 PS_C( 3 )
#define distortion_r_coeff0 ( g_vWarpParms3.x )
#define distortion_r_coeff1 ( g_vWarpParms3.y )
#define distortion_r_coeff2 ( g_vWarpParms3.z )
#define distortion_blue_coeff0delta ( g_vWarpParms3.w )

#define g_vWarpParms4 PS_C( 4 )
#define aspect_height_over_width_t2 ( g_vWarpParms4.x )

void main()
{
	const int DISTORT_TYPE = DYNAMIC_PS_COMBO( 1, 3 );

	vec2 vOriginal = vBaseTexCoord.xy;
	float BaseX;

	vec2 centre;
	float dcoeff0;
	float dcoeff1;
	float dcoeff2;
	float stereo_distortion_OffsetX_Corrected;

	if ( vOriginal.x < 0.5 )
	{
		BaseX = 0.25;
		stereo_distortion_OffsetX_Corrected = stereo_distortion_OffsetX;
		centre = distortion_l_centre;
		dcoeff0 = distortion_l_coeff0;
		dcoeff1 = distortion_l_coeff1;
		dcoeff2 = distortion_l_coeff2;
	}
	else
	{
		BaseX = 0.75;
		stereo_distortion_OffsetX_Corrected = -stereo_distortion_OffsetX;
		centre = distortion_r_centre;
		dcoeff0 = distortion_r_coeff0;
		dcoeff1 = distortion_r_coeff1;
		dcoeff2 = distortion_r_coeff2;
	}

	float fLocalAspectHeightOverWidthTimes2 = aspect_height_over_width_t2;

	// Delta runs from -1 to +1 (NDC) across the eye's half of the target;
	// vertically it has the same scale in pixels.
	vec2 Delta;
	Delta.x = ( vOriginal.x - BaseX ) * 4.0;
	Delta.y = ( vOriginal.y - 0.5 ) * fLocalAspectHeightOverWidthTimes2;

	// Offset by the calibration center.
	Delta -= centre;

	float r2 = Delta.x * Delta.x + Delta.y * Delta.y;

	float rdistortion_r = 1.0;
	float rdistortion_g = 1.0;
	float rdistortion_b = 1.0;
	if ( DISTORT_TYPE == 1 )
	{
		rdistortion_r = 1.0 / ( 1.0 + r2 * ( ( dcoeff0 + distortion_red_coeff0delta ) +
		                                     r2 * ( dcoeff1 + r2 * dcoeff2 ) ) );
		rdistortion_g = 1.0 / ( 1.0 + r2 * ( dcoeff0 + r2 * ( dcoeff1 + r2 * dcoeff2 ) ) );
		rdistortion_b = 1.0 / ( 1.0 + r2 * ( ( dcoeff0 + distortion_blue_coeff0delta ) +
		                                     r2 * ( dcoeff1 + r2 * dcoeff2 ) ) );
	}
	else if ( DISTORT_TYPE == 2 )
	{
		rdistortion_r = ( 1.0 + r2 * ( ( dcoeff0 + distortion_red_coeff0delta ) +
		                               r2 * ( dcoeff1 + r2 * dcoeff2 ) ) );
		rdistortion_g = ( 1.0 + r2 * ( dcoeff0 + r2 * ( dcoeff1 + r2 * dcoeff2 ) ) );
		rdistortion_b = ( 1.0 + r2 * ( ( dcoeff0 + distortion_blue_coeff0delta ) +
		                               r2 * ( dcoeff1 + r2 * dcoeff2 ) ) );
	}

	vec2 DeltaR = Delta * rdistortion_r;
	vec2 DeltaG = Delta * rdistortion_g;
	vec2 DeltaB = Delta * rdistortion_b;

	// Offset back from the calibration center.
	DeltaR += centre;
	DeltaG += centre;
	DeltaB += centre;

	float inv_aspect = 2.0 / fLocalAspectHeightOverWidthTimes2;

	float stereo_distortion_ScaleY_corrected = stereo_distortion_ScaleY * inv_aspect;

	DeltaR.x = DeltaR.x * stereo_distortion_ScaleX + stereo_distortion_OffsetX_Corrected;
	DeltaR.y = DeltaR.y * stereo_distortion_ScaleY_corrected + stereo_distortion_OffsetY;
	DeltaG.x = DeltaG.x * stereo_distortion_ScaleX + stereo_distortion_OffsetX_Corrected;
	DeltaG.y = DeltaG.y * stereo_distortion_ScaleY_corrected + stereo_distortion_OffsetY;
	DeltaB.x = DeltaB.x * stereo_distortion_ScaleX + stereo_distortion_OffsetX_Corrected;
	DeltaB.y = DeltaB.y * stereo_distortion_ScaleY_corrected + stereo_distortion_OffsetY;

	// Back to texture coordinates.
	vec2 PixPosR, PixPosG, PixPosB;
	PixPosR.x = DeltaR.x * 0.25 + BaseX;
	PixPosR.y = DeltaR.y * 0.5 + 0.5;
	PixPosG.x = DeltaG.x * 0.25 + BaseX;
	PixPosG.y = DeltaG.y * 0.5 + 0.5;
	PixPosB.x = DeltaB.x * 0.25 + BaseX;
	PixPosB.y = DeltaB.y * 0.5 + 0.5;

	vec4 vFinal = vec4( 0.0 );
	if ( ( DeltaG.x > -1.0 ) && ( DeltaG.y > -1.0 ) && ( DeltaG.x < 1.0 ) && ( DeltaG.y < 1.0 ) )
	{
		vFinal.r = tex2D( 0, BaseTextureSampler, PixPosR ).r;
		vFinal.g = tex2D( 0, BaseTextureSampler, PixPosG ).g;
		vFinal.b = tex2D( 0, BaseTextureSampler, PixPosB ).b;
	}

	LegacyWrite( vFinal );
}
