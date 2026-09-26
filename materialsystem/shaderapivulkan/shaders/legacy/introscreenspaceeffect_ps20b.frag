#version 450
// IntroScreenSpaceEffect's pixel stage: a port of
// stdshaders/IntroScreenSpaceEffect_ps2x.fxc (ps20b): ten ways of combining the
// frame buffer copies 0 (the scene) and 1 (the G-Man layer), with alpha
// g_Alpha. Combos (fxctmp9/IntroScreenSpaceEffect_ps20b.inc): static
// CONVERT_TO_SRGB (10, always 0 here), LINEAR_TO_SRGB (20; the shader selects
// it on OS X only); dynamic MODE (1, 0..9).
// @legacy program=introscreenspaceeffect ps=introscreenspaceeffect_ps20b
//         vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20 samplers=0:2d,1:2d
//         flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;  // s0
layout( set = 0, binding = 1 ) uniform sampler2D BaseTextureSampler2; // s1

layout( location = 0 ) in vec2 baseTexCoord;

#define g_Alpha ( PS_C( 0 ).x )

bool LinearToSrgbCombo()
{
	return STATIC_PS_COMBO( 20, 2 ) != 0;
}

vec3 RGBtoHSV( vec3 rgb )
{
	vec3 hsv;
	float fmin, fmax, delta;
	fmin = min( min( rgb.r, rgb.g ), rgb.b );
	fmax = max( max( rgb.r, rgb.g ), rgb.b );
	hsv.b = fmax; // v
	delta = fmax - fmin;
	if ( delta != 0.0 )
	{
		hsv.g = delta / fmax; // s
		if ( rgb.r == fmax )
			hsv.r = ( rgb.g - rgb.b ) / delta; // between yellow & magenta
		else if ( rgb.g == fmax )
			hsv.r = 2.0 + ( rgb.b - rgb.r ) / delta; // between cyan & yellow
		else
			hsv.r = 4.0 + ( rgb.r - rgb.g ) / delta; // between magenta & cyan
		hsv.r *= 60.0; // degrees
		if ( hsv.r < 0.0 )
			hsv.r += 360.0;
	}
	else
	{
		// r = g = b = 0: s = 0, v is undefined
		hsv.g = 0.0;
		hsv.r = -1.0;
	}
	return hsv;
}

vec3 HSVtoRGB( vec3 hsv )
{
	vec3 rgb;
	float h = hsv.r;
	float s = hsv.g;
	float v = hsv.b;
	if ( s == 0.0 )
	{
		// achromatic (grey)
		rgb = vec3( v );
	}
	else
	{
		h /= 60.0; // sector 0 to 5
		float i = floor( h );
		float f = h - i; // factorial part of h
		float p = v * ( 1.0 - s );
		float q = v * ( 1.0 - s * f );
		float t = v * ( 1.0 - s * ( 1.0 - f ) );
		if ( h < 1.0 )
			rgb = vec3( v, t, p );
		else if ( h >= 1.0 && h < 2.0 )
			rgb = vec3( q, v, p );
		else if ( h >= 2.0 && h < 3.0 )
			rgb = vec3( p, v, t );
		else if ( h >= 3.0 && h < 4.0 )
			rgb = vec3( p, q, v );
		else if ( h >= 4.0 && h < 5.0 )
			rgb = vec3( t, p, v );
		else // if ( h >= 5 )
			rgb = vec3( v, p, q );
	}
	return rgb;
}

// The input converter OS X needs when the rest of the shader expects sRGB
// values (LINEAR_TO_SRGB).
vec3 SampleTexture( int set, sampler2D texSampler, vec2 tc )
{
	vec3 c = tex2D( set, texSampler, tc ).xyz;
	if ( LinearToSrgbCombo() )
		c = LinearToGamma( c );
	return c;
}

// The output converter of LINEAR_TO_SRGB.
vec3 OutputColor( vec3 result )
{
	if ( LinearToSrgbCombo() )
		return GammaToLinear( result );
	return result;
}

vec4 Output( vec3 result )
{
	return FinalOutput( vec4( OutputColor( result ), g_Alpha ), 0.0, PIXEL_FOG_TYPE_NONE,
	    TONEMAP_SCALE_NONE );
}

vec4 Effect( int MODE )
{
	const float scale = 1.0 / 3.0;
	vec3 scene = SampleTexture( 0, BaseTextureSampler, baseTexCoord );
	vec3 gman = SampleTexture( 1, BaseTextureSampler2, baseTexCoord );

	if ( MODE == 0 )
	{
		// negative greyscale of scene * gman
		scene.xyz = vec3( dot( vec3( scale, scale, scale ), scene.xyz ) );
		scene = vec3( 1.0, 1.0, 1.0 ) - scene;
		return Output( scene * gman );
	}
	if ( MODE == 1 )
	{
		scene.xyz = vec3( dot( vec3( scale, scale, scale ), scene.xyz ) );
		float gmanLum = dot( vec3( scale, scale, scale ), gman );
		if ( gmanLum < 0.3 )
			return Output( vec3( 1.0, 1.0, 1.0 ) - gman );
		return Output( ( vec3( 1.0, 1.0, 1.0 ) - gman ) * scene );
	}
	if ( MODE == 2 )
	{
		float gmanLum = dot( vec3( scale, scale, scale ), gman );
		return Output( min( vec3( gmanLum ), scene ) );
	}
	if ( MODE == 3 || MODE == 4 )
	{
		float gmanLum = dot( vec3( scale, scale, scale ), gman );

		const float a = 0.0;
		const float b = 0.4;
		const float c = 0.7;
		const float d = 1.0;

		float blend;
		if ( gmanLum < b )
			blend = ( gmanLum - a ) / ( b - a );
		else if ( gmanLum > c )
			blend = 1.0 - ( ( gmanLum - c ) / ( d - c ) );
		else
			blend = 1.0;

		blend = saturate( blend );

		if ( MODE == 3 )
			return Output(
			    vec3( gmanLum ) * ( vec3( 1.0 ) - vec3( blend ) ) + scene * vec3( blend ) );
		return Output( gman * ( vec3( 1.0 ) - vec3( blend ) ) + scene * vec3( blend ) );
	}
	if ( MODE == 5 )
	{
		float sceneLum = scene.r;
		if ( sceneLum > 0.0 )
			return Output( scene );

		vec3 hsv = RGBtoHSV( gman );
		float blend = hsv.b - 0.5;
		hsv.b *= 1.0 + blend;
		hsv.g *= 1.0 - blend;
		return Output( HSVtoRGB( hsv ) );
	}
	if ( MODE == 6 )
		return Output( scene + gman );
	if ( MODE == 7 )
		return Output( scene );
	if ( MODE == 8 )
		return Output( gman );

	// MODE 9
	vec3 cLayer1 = scene;
	vec3 cLayer2 = gman;

	float flLayer1Brightness = saturate( dot( cLayer1.rgb, vec3( 0.333, 0.334, 0.333 ) ) );

	// Modify layer 1 to be more contrasty
	cLayer1.rgb = saturate( cLayer1.rgb * cLayer1.rgb * 2.0 );
	vec3 cLinearOverlayResult =
	    cLayer1.rgb + cLayer2.rgb * saturate( 1.0 - flLayer1Brightness * 2.0 );

	return Output( cLinearOverlayResult.rgb );
}

void main()
{
	LegacyWrite( Effect( DYNAMIC_PS_COMBO( 1, 10 ) ) );
}
