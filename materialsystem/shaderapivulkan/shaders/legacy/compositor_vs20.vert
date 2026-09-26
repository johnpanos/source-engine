#version 450
// A port of stdshaders/compositor_vs20.fxc (Compositor's vertex stage): the
// position passes through; TEXCOORD0 and TEXCOORD1 pack the four input
// textures' coordinates, TEXCOORD0 through the 3x2 transforms of c2..c9.
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec4 texCoord01;
layout( location = 1 ) out vec4 texCoord23;

// cXformTexCoord0..3: float4x2 each, its two columns in consecutive registers.
vec2 XformTexCoord( vec3 uv1, int reg )
{
	return vec2( dot( uv1, VS_C( reg ).xyz ), dot( uv1, VS_C( reg + 1 ).xyz ) );
}

void main()
{
	gl_Position = LegacyClipSpace( vec4( inWorldPos, 1.0 ) );

	const vec3 uv1 = vec3( inTexCoord0, 1.0 );
	texCoord01.xy = XformTexCoord( uv1, 2 );
	texCoord01.zw = XformTexCoord( uv1, 4 );
	texCoord23.xy = XformTexCoord( uv1, 6 );
	texCoord23.zw = XformTexCoord( uv1, 8 );
}
