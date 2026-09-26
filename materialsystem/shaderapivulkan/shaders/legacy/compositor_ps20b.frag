#version 450
// Compositor's pixel stage: a port of stdshaders/compositor_ps2x.fxc (ps20b),
// one stage of the texture compositor: up to four sRGB inputs with Photoshop
// style level adjustment, multiplied, added, lerped, blended (sticker over the
// previous result, specular from the third input) or turned into a selector
// mask. Combos (fxctmp9/compositor_ps20b.inc): static COMBINE_MODE (2, 0..6;
// the ps20 legacy lerp passes 4 and 5 are skipped); dynamic DEBUG_MODE (1,
// COMBINE_MODE 6 only).
// @legacy program=compositor ps=compositor_ps20b vs=compositor_vs20 vert=compositor_vs20
//         samplers=0:2d,1:2d,2:2d,3:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D InSampler0; // s0
layout( set = 0, binding = 1 ) uniform sampler2D InSampler1; // s1
layout( set = 0, binding = 2 ) uniform sampler2D InSampler2; // s2
layout( set = 0, binding = 3 ) uniform sampler2D InSampler3; // s3

layout( location = 0 ) in vec4 texCoord01;
layout( location = 1 ) in vec4 texCoord23;

#define COMBINE_MODE_MULTIPLY 0
#define COMBINE_MODE_ADD 1
#define COMBINE_MODE_LERP 2
#define COMBINE_MODE_SELECTOR 3
#define COMBINE_MODE_LERP_TEX_FIRST 4
#define COMBINE_MODE_LERP_TEX_SECOND 5
#define COMBINE_MODE_BLEND 6

// cAdjustInLevel[n]: x black point, y white point, z gamma.
#define cAdjustInLevel( n ) PS_C( 2 + ( n ) )
// Declared int; the shader compares the float register.
#define cNumTextures ( PS_C( 6 ).x )
#define cSelectValues( n ) PS_C( 7 + ( n ) )

float invlerp( float x, float y, float r )
{
	return ( r - x ) / ( y - x );
}

vec4 invlerp( float x, float y, vec4 r )
{
	return ( r - x ) / ( y - x );
}

vec4 ConvertLinearTosRGB( vec4 lin )
{
	vec3 col_lin = lin.xyz;
	vec3 col_srgb;
	for ( int i = 0; i < 3; ++i )
	{
		if ( col_lin[i] <= 0.0031308 )
			col_srgb[i] = 12.92 * col_lin[i];
		else
			col_srgb[i] = 1.055 * HlslPow( col_lin[i], 1.0 / 2.4 ) - 0.055;
	}
	return vec4( col_srgb.xyz, lin.a );
}

vec4 ConvertsRGBToLinear( vec4 srgb )
{
	vec3 col_srgb = srgb.xyz;
	vec3 col_lin;
	for ( int i = 0; i < 3; ++i )
	{
		if ( col_srgb[i] <= 0.04045 )
			col_lin[i] = col_srgb[i] / 12.92;
		else
			col_lin[i] = HlslPow( ( col_srgb[i] + 0.055 ) / 1.055, 2.4 );
	}
	return vec4( col_lin.xyz, srgb.a );
}

// Photoshop's level adjustment, in sRGB space as Photoshop does it.
vec4 AdjustLevels( vec4 inSrc, float inBlackPoint, float inWhitePoint, float inGammaValue )
{
	if ( inBlackPoint == 0.0 && inWhitePoint == 1.0 && inGammaValue == 1.0 )
		return inSrc;

	inSrc = ConvertLinearTosRGB( inSrc );

	vec4 pcg = saturate( invlerp( inBlackPoint, inWhitePoint, inSrc ) );
	vec4 gammaAdjusted = HlslPow( pcg, inGammaValue );

	gammaAdjusted = ConvertsRGBToLinear( gammaAdjusted );

	return saturate( gammaAdjusted );
}

vec4 AdjustLevels( vec4 inSrc, int n )
{
	return AdjustLevels(
	    inSrc, cAdjustInLevel( n ).x, cAdjustInLevel( n ).y, cAdjustInLevel( n ).z );
}

vec4 main_simple( int COMBINE_MODE, bool DEBUG_MODE )
{
	const vec4 cSafeColor = COMBINE_MODE == COMBINE_MODE_MULTIPLY ? vec4( 1.0 ) : vec4( 0.0 );

	vec4 color0 = cNumTextures > 0.0 ? tex2D( 0, InSampler0, texCoord01.xy ) : cSafeColor;
	vec4 color1 = cNumTextures > 1.0 ? tex2D( 1, InSampler1, texCoord01.zw ) : cSafeColor;
	vec4 color2 = cNumTextures > 2.0 ? tex2D( 2, InSampler2, texCoord23.xy ) : cSafeColor;
	vec4 color3 = cNumTextures > 3.0 ? tex2D( 3, InSampler3, texCoord23.zw ) : cSafeColor;

	color0 = cNumTextures > 0.0 ? AdjustLevels( color0, 0 ) : cSafeColor;
	color1 = cNumTextures > 1.0 ? AdjustLevels( color1, 1 ) : cSafeColor;
	color2 = cNumTextures > 2.0 ? AdjustLevels( color2, 2 ) : cSafeColor;
	color3 = cNumTextures > 3.0 ? AdjustLevels( color3, 3 ) : cSafeColor;

	if ( COMBINE_MODE == COMBINE_MODE_MULTIPLY )
		return color0 * color1 * color2 * color3;
	if ( COMBINE_MODE == COMBINE_MODE_ADD )
		return color0 + color1 + color2 + color3;

	// COMBINE_MODE_BLEND: color0 is the previous result, color1 the sticker
	// (alpha its transparency), color2's red the specular written to alpha.
	if ( !DEBUG_MODE )
	{
		float srcSpecular = color2.r;

		vec3 tmpColor = ( 1.0 - color1.a ) * color0.xyz + ( color1.a ) * color1.xyz;

		float tmpSpecular = ( 1.0 - color1.a ) * color0.w + ( color1.a ) * srcSpecular;

		return vec4( tmpColor.xyz, tmpSpecular );
	}
	vec2 texCoord1 = texCoord01.zw;
	if ( texCoord1.x < 0.0 || texCoord1.y < 0.0 || texCoord1.x > 1.0 || texCoord1.y > 1.0 )
		return color0;
	return vec4( texCoord1.xy, 0.0, color0.a );
}

vec4 main_lerp()
{
	if ( cNumTextures == 3.0 )
	{
		vec4 color0 = tex2D( 0, InSampler0, texCoord01.xy );
		vec4 color1 = tex2D( 1, InSampler1, texCoord01.zw );
		vec4 colSel = tex2D( 2, InSampler2, texCoord23.xy );

		color0 = AdjustLevels( color0, 0 );
		color1 = AdjustLevels( color1, 1 );
		colSel = AdjustLevels( colSel, 2 );

		return mix( color0, color1, colSel.xxxx );
	}
	return vec4( 1.0, 0.0, 0.0, 1.0 );
}

// HLSL's round, as fxc compiles it: floor( x + 0.5 ).
float HlslRound( float x )
{
	return floor( x + 0.5 );
}

vec4 main_selector()
{
	if ( cNumTextures == 1.0 )
	{
		float fNormalizedColor = tex2D( 0, InSampler0, texCoord01.xy ).x;
		float fTestColor = HlslRound( fNormalizedColor * 255.0 / 16.0 );

		bool bAny = false;
		for ( int i = 0; i < 4; ++i )
		{
			for ( int k = 0; k < 4; ++k )
			{
				const float select = cSelectValues( i )[k];
				if ( select != 0.0 && HlslRound( select ) == fTestColor )
					bAny = true;
			}
		}
		return bAny ? vec4( 1.0, 1.0, 1.0, 1.0 ) : vec4( 0.0, 0.0, 0.0, 0.0 );
	}
	return vec4( 1.0, 1.0, 0.0, 1.0 );
}

void main()
{
	const int COMBINE_MODE = STATIC_PS_COMBO( 2, 7 );
	const bool DEBUG_MODE = DYNAMIC_PS_COMBO( 1, 2 ) != 0;

	vec4 result;
	if ( COMBINE_MODE == COMBINE_MODE_LERP )
		result = main_lerp();
	else if ( COMBINE_MODE == COMBINE_MODE_SELECTOR )
		result = main_selector();
	else
		result = main_simple( COMBINE_MODE, DEBUG_MODE );
	LegacyWrite( result );
}
