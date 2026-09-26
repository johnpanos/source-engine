// Pixel-stage color helpers of common_fxc.h and common_ps_fxc.h that the
// screen-space and post-processing ports share: the scalar 2.2 gamma curve
// (legacy_common.glsl has the vec3 one), the sRGB curve in shader math,
// RGB/HSL conversions and MAX_HDR_OVERBRIGHT.
#ifndef LEGACY_COLOR_GLSL
#define LEGACY_COLOR_GLSL

#define MAX_HDR_OVERBRIGHT 16.0

// D3D9's pow instruction raises the magnitude of its base.
float HlslPow( float x, float y )
{
	return pow( abs( x ), y );
}
vec3 HlslPow( vec3 x, float y )
{
	return pow( abs( x ), vec3( y ) );
}
vec4 HlslPow( vec4 x, float y )
{
	return pow( abs( x ), vec4( y ) );
}

float LinearToGamma( float f1linear )
{
	return HlslPow( f1linear, 1.0 / 2.2 );
}
float GammaToLinear( float gamma )
{
	return HlslPow( gamma, 2.2 );
}

float SrgbGammaToLinear( float flSrgbGammaValue )
{
	float x = saturate( flSrgbGammaValue );
	return ( x <= 0.04045 ) ? ( x / 12.92 ) : HlslPow( ( x + 0.055 ) / 1.055, 2.4 );
}
float SrgbLinearToGamma( float flLinearValue )
{
	float x = saturate( flLinearValue );
	return ( x <= 0.0031308 ) ? ( x * 12.92 ) : ( 1.055 * HlslPow( x, 1.0 / 2.4 ) ) - 0.055;
}

// common_ps_fxc.h's RGBtoHSL: ( hue 0..1, saturation, lightness, 1 ).
vec4 RGBtoHSL( vec4 inColor )
{
	float h, s;
	float flMax = max( inColor.r, max( inColor.g, inColor.b ) );
	float flMin = min( inColor.r, min( inColor.g, inColor.b ) );

	float l = ( flMax + flMin ) / 2.0;

	if ( flMax == flMin ) // achromatic case
	{
		s = h = 0.0;
	}
	else // chromatic case
	{
		float delta = flMax - flMin;

		if ( l < 0.5 )
			s = delta / ( flMax + flMin );
		else
			s = delta / ( 2.0 - flMax - flMin );

		if ( inColor.r == flMax )
			h = ( inColor.g - inColor.b ) / delta;
		else if ( inColor.g == flMax )
			h = 2.0 + ( inColor.b - inColor.r ) / delta;
		else
			h = 4.0 + ( inColor.r - inColor.g ) / delta;

		h *= 60.0;
		if ( h < 0.0 )
			h += 360.0;
		h /= 360.0;
	}
	return vec4( h, s, l, 1.0 );
}

// HLSL's fmod: the remainder with the sign of x.
float HlslFmod( float x, float y )
{
	return x - y * trunc( x / y );
}

float HueToRGB( float v1, float v2, float vH )
{
	float fResult = v1;

	vH = HlslFmod( vH + 1.0, 1.0 );

	if ( ( 6.0 * vH ) < 1.0 )
		fResult = ( v1 + ( v2 - v1 ) * 6.0 * vH );
	else if ( ( 2.0 * vH ) < 1.0 )
		fResult = ( v2 );
	else if ( ( 3.0 * vH ) < 2.0 )
		fResult = ( v1 + ( v2 - v1 ) * ( ( 2.0 / 3.0 ) - vH ) * 6.0 );

	return fResult;
}

// common_ps_fxc.h's HSLtoRGB.
vec4 HSLtoRGB( vec4 hsl )
{
	float r, g, b;
	float h = hsl.x;
	float s = hsl.y;
	float l = hsl.z;

	if ( s == 0.0 )
	{
		r = g = b = l;
	}
	else
	{
		float v1, v2;
		if ( l < 0.5 )
			v2 = l * ( 1.0 + s );
		else
			v2 = ( l + s ) - ( s * l );

		v1 = 2.0 * l - v2;

		r = HueToRGB( v1, v2, h + ( 1.0 / 3.0 ) );
		g = HueToRGB( v1, v2, h );
		b = HueToRGB( v1, v2, h - ( 1.0 / 3.0 ) );
	}
	return vec4( r, g, b, 1.0 );
}

#endif // LEGACY_COLOR_GLSL
