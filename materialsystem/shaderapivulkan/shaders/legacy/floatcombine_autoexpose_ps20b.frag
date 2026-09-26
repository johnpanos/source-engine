#version 450
// floatcombine_autoexpose's pixel stage: a port of
// stdshaders/floatcombine_autoexpose_ps2x.fxc (ps20b, HDR_TYPE_FLOAT): the
// frame and its bloom with sharpening and vignette, scaled by the exposure
// texture's average luminance. Combos (fxctmp9/floatcombine_autoexpose_ps20b.inc):
// static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=floatcombine_autoexpose ps=floatcombine_autoexpose_ps20b
//         vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20 samplers=0:2d,1:2d,2:2d
//         flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FBSampler;        // s0
layout( set = 0, binding = 1 ) uniform sampler2D BloomSampler;     // s1
layout( set = 0, binding = 2 ) uniform sampler2D Exposure_Sampler; // s2

layout( location = 0 ) in vec2 texCoord;
layout( location = 1 ) in vec2 ZeroTexCoord;

// x sharpness, y woodcut, z bloom amount, w alpha sharpen factor
#define settings PS_C( 0 )
// x bloom exponent, y vignette min scale, z vignette power, w edge softness
#define settings2 PS_C( 1 )
// x autoexpose min, y autoexpose max
#define settings3 PS_C( 2 )

void main()
{
	vec4 fbSample = tex2D( 0, FBSampler, texCoord );
	vec4 bloom = tex2D( 1, BloomSampler, texCoord );
	vec4 exposure_data = tex2D( 2, Exposure_Sampler, ZeroTexCoord );
	float avg_lum = exposure_data.r;
	float tmscale = max( 0.18 / max( avg_lum, 0.0001 ), settings3.x );
	tmscale = min( tmscale, settings3.y );

	vec2 xofs = 2.0 * ( texCoord - vec2( 0.5, 0.5 ) );
	float dist = ( 1.0 / 2.0 ) * ( xofs.x * xofs.x + xofs.y * xofs.y );
	float vig = HlslPow( 1.0 - dist, settings2.z );

	fbSample = mix( fbSample, bloom, ( 1.0 - vig ) * settings2.w );
	fbSample = mix( bloom, fbSample, settings.x + settings.w * fbSample.a * settings2.w );

	bloom.xyz = min( bloom.xyz, 1.0 );
	float lum = 0.3 * bloom.x + 0.59 * bloom.y + 0.11 * bloom.z;
	lum = min( 1.0, lum );

	vec4 c_out = fbSample + settings.z * HlslPow( lum, settings2.x ) * bloom;
	c_out.xyz *= tmscale;
	LegacyWrite( FinalOutput( c_out, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
