#version 450
// DebugTextureView_dx9's pixel stage: a port of stdshaders/DebugTextureView_ps2x.fxc
// (ps20b): a 2D texture (its alpha with SHOWALPHA, scaled for HDR formats), or
// a cube map unfolded into a cross. Combos (fxctmp9/debugtextureview_ps20b.inc):
// static CONVERT_TO_SRGB (2, always 0 here), SHOWALPHA (4); dynamic ISCUBEMAP
// (1). Sampler 0 is read as a 2D texture or, with ISCUBEMAP, as a cube map.
// @legacy program=debugtextureview ps=debugtextureview_ps20b vs=debugtextureview_vs20
//         vert=debugtextureview_vs20 samplers=0:2d,0:cube flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tSampler;       // s0
layout( set = 0, binding = 1 ) uniform samplerCube g_tSamplerCube; // s0, ISCUBEMAP

layout( location = 0 ) in vec2 texCoord;

#define g_vConst0 PS_C( 0 )
#define g_flIsHdrCube ( g_vConst0.x )
#define g_flIsHdr2D ( g_vConst0.y )
#define MAX_HDR_OVERBRIGHT 16.0

void main()
{
	const bool SHOWALPHA = STATIC_PS_COMBO( 4, 2 ) != 0;
	const bool ISCUBEMAP = DYNAMIC_PS_COMBO( 1, 2 ) != 0;

	vec4 result = vec4( 0.0, 0.0, 0.0, 1.0 );
	if ( !ISCUBEMAP )
	{
		vec4 texel = tex2D( 0, g_tSampler, texCoord );
		result.rgb = texel.rgb;
		if ( SHOWALPHA )
			result.rgb = vec3( texel.a );

		if ( g_flIsHdr2D != 0.0 )
			result.rgb *= MAX_HDR_OVERBRIGHT;
	}
	else
	{
		bool bNoDataForThisPixel = false;
		vec3 vec = vec3( 0.0, 0.0, 0.0 );
		float x = texCoord.x;
		float y = texCoord.y;
		float x2 = fract( texCoord.x * 3.0 ) * 2.0 - 1.0;
		float y2 = fract( texCoord.y * 4.0 ) * 2.0 - 1.0;
		if ( ( x >= 0.3333 ) && ( x <= 0.6666 ) ) // Center row
		{
			if ( y >= 0.75 )
				vec = vec3( x2, 1.0, y2 );
			else if ( y >= 0.5 )
				vec = vec3( x2, y2, -1.0 );
			else if ( y >= 0.25 )
				vec = vec3( x2, -1.0, -y2 );
			else if ( y >= 0.0 )
				vec = vec3( x2, -y2, 1.0 );
		}
		else if ( ( y >= 0.25 ) && ( y <= 0.5 ) )
		{
			if ( x <= 0.3333 )
				vec = vec3( -1.0, -x2, -y2 );
			else if ( x >= 0.6666 )
				vec = vec3( 1.0, x2, -y2 );
			else
				bNoDataForThisPixel = true;
		}
		else
		{
			bNoDataForThisPixel = true;
		}

		vec4 cBase = texCUBE( 1, g_tSamplerCube, vec );
		if ( SHOWALPHA )
			cBase.rgb = vec3( cBase.a );

		if ( g_flIsHdrCube != 0.0 )
			cBase.rgb *= ENV_MAP_SCALE;

		if ( bNoDataForThisPixel )
			cBase.rgb = vec3( 0.9, 0.4, 0.15 );

		result.rgb = cBase.rgb;
		result.a = 1.0;
	}

	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
