#version 450
// color_projection's pixel stage: a port of stdshaders/color_projection_ps2x.fxc
// (ps20; the shader selects the ps20 build on every device): color blindness
// simulation of the frame (http://kaioa.com/node/91). Combos
// (fxctmp9/color_projection_ps20.inc): dynamic NEED_BLINDMK (1),
// NEED_ANOMYLIZE (2), NEED_MONOCHROME (4).
// @legacy program=color_projection ps=color_projection_ps20 vs=color_projection_vs20
//         vert=color_projection_vs20 samplers=4:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FrameSampler; // s4

layout( location = 0 ) in vec2 vScreenUV;

#define g_vColorParms PS_C( 1 )
#define cpu ( g_vColorParms.x )
#define cpv ( g_vColorParms.y )
#define am ( g_vColorParms.z )
#define ayi ( g_vColorParms.w )

vec3 rgb_from_xyz( vec3 vNum )
{
	vec3 vResult;
	vResult.r = dot( vNum, vec3( 3.063218, -1.393325, -0.475802 ) );
	vResult.g = dot( vNum, vec3( -0.969243, 1.875966, 0.041555 ) );
	vResult.b = dot( vNum, vec3( 0.067871, -0.228834, 1.069251 ) );
	return vResult;
}

vec3 xyz_from_rgb( vec3 vNum )
{
	vec3 vResult;
	vResult.x = dot( vNum, vec3( 0.430574, 0.341550, 0.178325 ) );
	vResult.y = dot( vNum, vec3( 0.222015, 0.706655, 0.071330 ) );
	vResult.z = dot( vNum, vec3( 0.020183, 0.129553, 0.939180 ) );
	return vResult;
}

vec3 anomylize( vec3 a, vec3 b )
{
	return ( ( 1.75 * b ) + a ) / 2.75;
}

vec3 monochrome( vec3 r )
{
	return vec3( dot( r, vec3( 0.299, 0.587, 0.114 ) ) );
}

const vec3 w_xyz = vec3( 0.312713, 0.329016, 0.358271 );

vec3 blindMK( vec3 vColor )
{
	vec3 c_xyz = xyz_from_rgb( vColor );

	float sum_xyz = c_xyz.x + c_xyz.y + c_xyz.z;

	vec2 c_uv = vec2( 0.0 );
	if ( sum_xyz != 0.0 )
		c_uv = c_xyz.xy / sum_xyz;

	vec2 n_xz = w_xyz.xz * c_xyz.y / w_xyz.y;

	float clm;
	if ( c_uv.x < cpu )
		clm = ( cpv - c_uv.y ) / ( cpu - c_uv.x );
	else
		clm = ( c_uv.y - cpv ) / ( c_uv.x - cpu );

	float clyi = c_uv.y - c_uv.x * clm;
	vec2 d_uv;
	d_uv.x = ( ayi - clyi ) / ( clm - am );
	d_uv.y = ( clm * d_uv.x ) + clyi;

	vec3 s_xyz;
	s_xyz.x = d_uv.x * c_xyz.y / d_uv.y;
	s_xyz.y = c_xyz.y;
	s_xyz.z = ( 1.0 - ( d_uv.x + d_uv.y ) ) * c_xyz.y / d_uv.y;

	vec3 s_rgb = rgb_from_xyz( s_xyz );

	vec3 d_xyz = vec3( 0.0 );
	d_xyz.xz = n_xz - s_xyz.xz;

	vec3 d_rgb = rgb_from_xyz( d_xyz );

	// ( s_rgb < 0.0f ? 0.0f : 1.0f ): fxc compiled the comparison of s_rgb.r
	// alone, for all three channels.
	const float sStep = s_rgb.r < 0.0 ? 0.0 : 1.0;
	vec3 adj_rgb;
	for ( int i = 0; i < 3; ++i )
	{
		adj_rgb[i] = d_rgb[i] != 0.0 ? sStep - s_rgb[i] / d_rgb[i] : 0.0;
		adj_rgb[i] = adj_rgb[i] < 0.0 ? 0.0 : adj_rgb[i] > 1.0 ? 0.0 : adj_rgb[i];
	}
	float adjust = max( max( adj_rgb.r, adj_rgb.g ), adj_rgb.b );

	s_rgb = s_rgb + ( adjust * d_rgb );

	return s_rgb;
}

void main()
{
	const bool NEED_BLINDMK = DYNAMIC_PS_COMBO( 1, 2 ) != 0;
	const bool NEED_ANOMYLIZE = DYNAMIC_PS_COMBO( 2, 2 ) != 0;
	const bool NEED_MONOCHROME = DYNAMIC_PS_COMBO( 4, 2 ) != 0;

	vec4 vDiffuse = tex2D( 0, FrameSampler, vScreenUV );

	vec4 vResult = vDiffuse;

	if ( NEED_BLINDMK )
		vResult.rgb = blindMK( vResult.rgb );

	if ( NEED_MONOCHROME )
		vResult.rgb = monochrome( vResult.rgb );

	if ( NEED_ANOMYLIZE )
		vResult.rgb = anomylize( vDiffuse.rgb, vResult.rgb );

	LegacyWrite( vResult );
}
