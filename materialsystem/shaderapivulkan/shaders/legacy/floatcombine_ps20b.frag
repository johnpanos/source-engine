#version 450
// floatcombine's pixel stage: a port of stdshaders/floatcombine_ps2x.fxc (ps20b,
// HDR_TYPE_FLOAT): the frame and its bloom with sharpening, woodcut and
// vignette. Combos (fxctmp9/floatcombine_ps20b.inc): static CONVERT_TO_SRGB (1,
// always 0 here).
// @legacy program=floatcombine ps=floatcombine_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FBSampler;    // s0
layout( set = 0, binding = 1 ) uniform sampler2D BloomSampler; // s1

layout( location = 0 ) in vec2 texCoord;
layout( location = 2 ) in vec2 bloomTexCoord;

// x sharpness, y woodcut, z bloom amount, w alpha sharpen factor
#define settings PS_C( 0 )
// x bloom exponent, y vignette min scale, z vignette power, w edge softness
#define settings2 PS_C( 1 )

void main()
{
	vec4 fbSample = tex2D( 0, FBSampler, texCoord );
	vec4 bloom = tex2D( 1, BloomSampler, bloomTexCoord );

	vec2 xofs = 2.0 * ( texCoord - vec2( 0.5, 0.5 ) );
	float dist = ( 1.0 / 2.0 ) * ( xofs.x * xofs.x + xofs.y * xofs.y );
	float vig = HlslPow( 1.0 - dist, settings2.z );

	fbSample = mix( fbSample, bloom, ( 1.0 - vig ) * settings2.w );
	fbSample = mix( bloom, fbSample, settings.x + settings.w * fbSample.a * settings2.w );

	vec3 woodcut;
	woodcut.x = bloom.x < fbSample.x ? 1.0 : 0.0;
	woodcut.y = bloom.y < fbSample.y ? 1.0 : 0.0;
	woodcut.z = bloom.z < fbSample.z ? 1.0 : 0.0;
	fbSample.xyz = mix( fbSample.xyz, woodcut.xyz, settings.y );

	bloom.xyz *= sqrt( LINEAR_LIGHT_SCALE );
	bloom.xyz = min( bloom.xyz, 1.0 );
	float lum = 0.3 * bloom.x + 0.59 * bloom.y + 0.11 * bloom.z;
	lum = min( 1.0, lum );

	vec4 c_out = vec4( cLightScale.xyz, 1.0 ) * fbSample +
	             settings.z * HlslPow( lum, settings2.x ) * bloom;

	// the vignette (lens brightness falloff near the edges)
	c_out.xyz *= max( settings2.y, vig );
	LegacyWrite( FinalOutput( c_out, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
