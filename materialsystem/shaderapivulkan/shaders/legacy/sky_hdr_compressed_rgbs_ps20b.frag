#version 450
// Sky_HDR_DX9's pixel stage for $hdrcompressedtexture (RGBS: color times
// alpha as the scale): a port of stdshaders/sky_hdr_compressed_rgbs_ps2x.fxc
// (ps20b), a manual bilinear filter of the four point samples the vertex
// stage placed, scaled by InputScale and LINEAR_LIGHT_SCALE. Combos
// (fxctmp9/sky_hdr_compressed_rgbs_ps20b.inc): static CONVERT_TO_SRGB (2,
// always 0 here); dynamic WRITE_DEPTH_TO_DESTALPHA (1).
// @legacy program=sky_hdr_compressed_rgbs ps=sky_hdr_compressed_rgbs_ps20b vs=sky_vs20
//         vert=sky_vs20 samplers=0:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D RGBSTextureSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord00;
layout( location = 1 ) in vec2 baseTexCoord01;
layout( location = 2 ) in vec2 baseTexCoord10;
layout( location = 3 ) in vec2 baseTexCoord11;
layout( location = 4 ) in vec2 baseTexCoord_In_Pixels;

#define InputScale PS_C( 0 )

void main()
{
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 1, 2 );

	vec4 s00 = tex2D( 0, RGBSTextureSampler, baseTexCoord00 );
	vec4 s10 = tex2D( 0, RGBSTextureSampler, baseTexCoord10 );
	vec4 s01 = tex2D( 0, RGBSTextureSampler, baseTexCoord01 );
	vec4 s11 = tex2D( 0, RGBSTextureSampler, baseTexCoord11 );

	const vec2 fracCoord = fract( baseTexCoord_In_Pixels );

	s00.rgb *= s00.a;
	s10.rgb *= s10.a;
	s00.xyz = mix( s00.xyz, s10.xyz, fracCoord.x );

	s01.rgb *= s01.a;
	s11.rgb *= s11.a;
	s01.xyz = mix( s01.xyz, s11.xyz, fracCoord.x );

	const vec3 result = mix( s00.xyz, s01.xyz, fracCoord.y );

	// This is never fogged.
	LegacyWrite( FinalOutput( vec4( InputScale.rgb * result, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE,
	    TONEMAP_SCALE_LINEAR, WRITE_DEPTH_TO_DESTALPHA != 0, 1e20 ) );
}
