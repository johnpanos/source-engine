#version 450
// A port of stdshaders/lpreview1_ps2x.fxc (ps20b), which screenspace_general
// draws (editor/addlight*): one spot light of the lighting preview, from the
// albedo, normal and position buffers, added to the accumulation buffer.
// Combos (fxctmp9/lpreview1_ps20b.inc): static CONVERT_TO_SRGB (1, always 0
// here).
// @legacy program=lpreview1 ps=lpreview1_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d,1:2d,2:2d,3:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D AlbedoSampler;   // s0
layout( set = 0, binding = 1 ) uniform sampler2D NormalSampler;   // s1
layout( set = 0, binding = 2 ) uniform sampler2D PositionSampler; // s2
layout( set = 0, binding = 3 ) uniform sampler2D AccBuf_In;       // s3

layout( location = 0 ) in vec2 texCoord;

#define Light_origin PS_C( 0 )
#define INNER_COS ( Light_origin.w )
#define OUTER_COS ( Light_dir.w )
#define Light_dir PS_C( 1 )
#define Light_attn PS_C( 2 )
#define QUADRATIC_ATTN ( Light_attn.x )
#define LINEAR_ATTN ( Light_attn.y )
#define CONSTANT_ATTN ( Light_attn.z )
#define SCALE_FACTOR ( Light_attn.w )
#define Light_color ( PS_C( 3 ).xyz )

float Lerp5( float f1, float f2, float i1, float i2, float x )
{
	return f1 + ( f2 - f1 ) * ( x - i1 ) / ( i2 - i1 );
}

void main()
{
	vec4 normal = tex2D( 1, NormalSampler, texCoord );
	vec4 albedo = tex2D( 0, AlbedoSampler, texCoord );
	vec4 pos = tex2D( 2, PositionSampler, texCoord );
	vec3 old_acc = tex2D( 3, AccBuf_In, texCoord ).rgb;

	vec3 ldir = Light_origin.xyz - pos.xyz;
	float dist = sqrt( dot( ldir, ldir ) );
	ldir = normalize( ldir );
	float spot_dot = dot( ldir, -Light_dir.xyz );
	vec3 ret = Light_color * 0.09 * albedo.xyz; // ambient
	float dist_falloff =
	    ( SCALE_FACTOR / ( QUADRATIC_ATTN * dist * dist + LINEAR_ATTN * dist + CONSTANT_ATTN ) );
	if ( spot_dot > OUTER_COS )
	{
		float falloff = 1.0;
		if ( spot_dot < INNER_COS )
			falloff = Lerp5( 1.0, 0.0, INNER_COS, OUTER_COS, spot_dot );
		float dotprod = max( 0.0, dot( ldir.xyz, normal.xyz ) );
		ret += dotprod * falloff * ( Light_color * albedo.xyz );
	}
	else
	{
		dist_falloff = min( 1.0, dist_falloff );
	}
	ret *= dist_falloff;
	LegacyWrite( FinalOutput( vec4( ret + old_acc, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE,
	    TONEMAP_SCALE_NONE ) );
}
