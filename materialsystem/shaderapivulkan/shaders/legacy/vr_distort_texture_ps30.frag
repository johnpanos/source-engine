#version 450
// vr_distort_texture's pixel stage: a port of
// stdshaders/vr_distort_texture_ps2x.fxc (ps30, which the shader selects with
// shader model 3): the frame read through the distortion map's per-channel
// coordinates, black outside the bounds (and along the eyes' seam without a
// render target). Combos (fxctmp9/vr_distort_texture_ps30.inc): dynamic
// CMBO_USERENDERTARGET (1).
// @legacy program=vr_distort_texture ps=vr_distort_texture_ps30 vs=vr_distort_texture_vs30
//         vert=vr_distort_texture_vs30 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;       // s0
layout( set = 0, binding = 1 ) uniform sampler2D DistortMapTextureSampler; // s1

layout( location = 0 ) in vec2 vBaseTexCoord;

void main()
{
	const bool CMBO_USERENDERTARGET = DYNAMIC_PS_COMBO( 1, 2 ) != 0;

	vec2 vOriginal = vBaseTexCoord.xy;

	vec4 vRead = tex2D( 1, DistortMapTextureSampler, vOriginal );

	vec2 vGreen;
	vGreen.r = ( vRead.x + vRead.z ) / 2.0;
	vGreen.g = ( vRead.y + vRead.w ) / 2.0;

	vec4 vFinal;
	vFinal.r = tex2D( 0, BaseTextureSampler, vRead.xy ).r;
	vFinal.ga = tex2D( 0, BaseTextureSampler, vGreen ).ga;
	vFinal.b = tex2D( 0, BaseTextureSampler, vRead.zw ).b;

	float fBoundsCheck;
	if ( CMBO_USERENDERTARGET )
	{
		fBoundsCheck = saturate( dot( vec2( lessThan( vGreen.xy, vec2( 0.01, 0.01 ) ) ),
		                             vec2( 1.0, 1.0 ) ) +
		                         dot( vec2( greaterThan( vGreen.xy, vec2( 0.99, 0.99 ) ) ),
		                             vec2( 1.0, 1.0 ) ) );
	}
	else
	{
		fBoundsCheck = saturate( dot( vec2( lessThan( vGreen.xy, vec2( 0.005, 0.005 ) ) ),
		                             vec2( 1.0, 1.0 ) ) +
		                         dot( vec2( greaterThan( vGreen.xy, vec2( 0.995, 0.995 ) ) ),
		                             vec2( 1.0, 1.0 ) ) +
		                         ( ( vGreen.x > 0.495 && vGreen.x < 0.505 ) ? 1.0 : 0.0 ) );
	}

	vFinal.xyz = mix( vFinal.xyz, vec3( 0.0, 0.0, 0.0 ), fBoundsCheck );

	LegacyWrite( vFinal );
}
