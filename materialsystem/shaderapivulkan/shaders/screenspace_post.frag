#version 450
// The engine's bloom and color-correction passes (CViewRender's
// DoEnginePostProcessing), ported from their ps20b pixel shaders:
//   mode 1  Downsample_nohdr_ps2x.fxc   (dev/downsample_non_hdr)
//   mode 2  BlurFilter_ps2x.fxc         (dev/blurfilterx_nohdr, _y_nohdr)
//   mode 3  Engine_Post_ps2x.fxc        (dev/engine_post: bloom add, software
//           anti-aliasing and up to four color-correction volume lookups)
// Each ends in FinalOutput( ..., PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ), an
// identity with the sRGB conversion done by the hardware (none of these passes
// reads or writes sRGB off OS X).
//
// The uniform block holds the pixel shader registers c0..c5 as the material's
// dynamic state wrote them, the vertex shader's SHADER_SPECIFIC_CONST_0..3 in
// c8..c11, and in c16 the pixel shader's static and dynamic combo indices.
// params: y the mode, z kColor* flags (1 sRGB sampler 0, 2 sRGB sampler 1, 4
// sRGB output).
layout( location = 0 ) in vec2 vUv;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 1, binding = 0 ) uniform sampler2D FBTextureSampler;   // s1
layout( set = 2, binding = 0 ) uniform sampler3D ColorCorrectionVolumeTexture0; // s2
layout( set = 3, binding = 0 ) uniform sampler3D ColorCorrectionVolumeTexture1; // s3
layout( set = 4, binding = 0 ) uniform sampler3D ColorCorrectionVolumeTexture2; // s4
layout( set = 5, binding = 0 ) uniform sampler3D ColorCorrectionVolumeTexture3; // s5
layout( set = 6, binding = 0 ) uniform PixelShaderConstants
{
	vec4 c[32];
}
ps;
layout( push_constant ) uniform Constants
{
	mat4 unusedMatrix;
	vec4 unused0;
	vec4 unused1;
	vec4 unused2;
	vec4 params;
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;

const int kModeDownsample = 1;
const int kModeBlur = 2;
const int kModeEnginePost = 3;

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}
// common_ps_fxc.h's GammaToLinear / LinearToGamma (the 2.2 power pair).
vec3 GammaToLinear( vec3 c )
{
	return pow( max( c, vec3( 0.0 ) ), vec3( 2.2 ) );
}
vec3 LinearToGamma( vec3 c )
{
	return pow( max( c, vec3( 0.0 ) ), vec3( 1.0 / 2.2 ) );
}

vec4 Sample0( vec2 uv )
{
	vec4 texel = texture( BaseTextureSampler, uv );
	if ( ( int( consts.params.z ) & 1 ) != 0 )
		texel.rgb = SrgbToLinear( texel.rgb );
	return texel;
}
vec4 SampleFB( vec2 uv )
{
	vec4 texel = texture( FBTextureSampler, uv );
	if ( ( int( consts.params.z ) & 2 ) != 0 )
		texel.rgb = SrgbToLinear( texel.rgb );
	return texel;
}

// Downsample_nohdr_ps2x.fxc (not CSTRIKE): shape each tap by its luminance
// against c0.rgb, raised to c0.w, and average four taps at TEXCOORD0 plus
// SHADER_SPECIFIC_CONST_0..3.
vec4 Shape( vec2 uv, bool srgbAdapter )
{
	vec4 pixel = Sample0( uv );
	if ( srgbAdapter )
		pixel.rgb = LinearToGamma( pixel.rgb );
	const float lum = dot( pixel.xyz, ps.c[0].xyz );
	pixel.xyz = pow( pixel.xyz, vec3( ps.c[0].w ) ) * lum;
	if ( srgbAdapter )
		pixel.rgb = GammaToLinear( pixel.rgb );
	return pixel;
}
// Its CSTRIKE variant.
vec3 ShapeCstrike( vec2 uv )
{
	const vec3 s = GammaToLinear( Sample0( uv ).rgb );
	const float lum = 0.3 * s.x + 0.59 * s.y + 0.11 * s.z;
	return lum * s;
}
vec4 Downsample( int staticIndex )
{
	const bool srgbAdapter = ( staticIndex % 2 ) != 0;
	const bool cstrike = ( ( staticIndex / 4 ) % 2 ) != 0;
	const vec2 taps[4] = vec2[4]( vUv + ps.c[8].xy, vUv + ps.c[9].xy, vUv + ps.c[10].xy,
	    vUv + ps.c[11].xy );
	if ( cstrike )
	{
		const vec3 sum = ShapeCstrike( taps[0] ) + ShapeCstrike( taps[1] ) +
		                 ShapeCstrike( taps[2] ) + ShapeCstrike( taps[3] );
		return vec4( sum * 0.25, 1.0 );
	}
	return ( Shape( taps[0], srgbAdapter ) + Shape( taps[1], srgbAdapter ) +
	           Shape( taps[2], srgbAdapter ) + Shape( taps[3], srgbAdapter ) ) *
	       0.25;
}

// BlurFilter_ps2x.fxc: a 13-tap separable Gaussian, taps 1..3 from the vertex
// stage's offsets (c8..c10) and 4..6 from the pixel shader's (c0..c2), scaled by c3.
vec4 BlurTap( vec2 uv, bool approxSrgbAdapter )
{
	vec4 cSample = Sample0( uv );
	if ( approxSrgbAdapter )
	{
		cSample.rgb = max( cSample.rgb, vec3( 0.00001 ) );
		cSample.rgb *= inversesqrt( cSample.rgb );
	}
	return cSample;
}
vec4 Blur( int staticIndex )
{
	const bool approx = ( ( staticIndex / 2 ) % 2 ) != 0;
	vec4 color = BlurTap( vUv, approx ) * 0.2013;
	color += ( BlurTap( vUv + ps.c[8].xy, approx ) + BlurTap( vUv - ps.c[8].xy, approx ) ) * 0.2185;
	color += ( BlurTap( vUv + ps.c[9].xy, approx ) + BlurTap( vUv - ps.c[9].xy, approx ) ) * 0.0821;
	color += ( BlurTap( vUv + ps.c[10].xy, approx ) + BlurTap( vUv - ps.c[10].xy, approx ) ) * 0.0461;
	color += ( BlurTap( vUv + ps.c[0].xy, approx ) + BlurTap( vUv - ps.c[0].xy, approx ) ) * 0.0262;
	color += ( BlurTap( vUv + ps.c[1].xy, approx ) + BlurTap( vUv - ps.c[1].xy, approx ) ) * 0.0162;
	color += ( BlurTap( vUv + ps.c[2].xy, approx ) + BlurTap( vUv - ps.c[2].xy, approx ) ) * 0.0102;
	color.xyz *= ps.c[3].xyz;
	if ( approx )
		color.xyz *= color.xyz;
	return color;
}

// Engine_Post_ps2x.fxc PerformAA (AA_DEBUG_MODE 0, FAST_DELTAS 1). psTapOffs_Packed
// is c0 ( +dX, 0, +dY, -dX ), tweakables c1.
vec3 PerformAA( vec3 baseColor, vec2 fbTexCoord, bool quality1, bool reduceOnePixelLineBlur )
{
	const float COLOUR_DELTA_BASE = quality1 ? 0.65 : 0.5;
	const float COLOUR_DELTA_CONTRAST = 100.0;
	const float DELTA_SCALE = quality1 ? 1.0 : 0.75;
	const float MAX_LERP_FACTOR = 0.66;
	const float onePixelLineBlurReduction = ps.c[1].y;
	vec4 texelDelta = ps.c[0] * ps.c[1].w;
	const vec3 a = SampleFB( fbTexCoord + texelDelta.yz ).rgb; // ( 0,+1)
	const vec3 b = SampleFB( fbTexCoord + texelDelta.xy ).rgb; // (+1, 0)
	const vec3 c = SampleFB( fbTexCoord - texelDelta.yz ).rgb; // ( 0,-1)
	const vec3 d = SampleFB( fbTexCoord - texelDelta.xy ).rgb; // (-1, 0)
	vec3 e = vec3( 0.0 ), f = vec3( 0.0 ), g = vec3( 0.0 ), h = vec3( 0.0 );
	if ( quality1 )
	{
		e = SampleFB( fbTexCoord + texelDelta.wz ).rgb; // (-1,+1)
		f = SampleFB( fbTexCoord - texelDelta.wz ).rgb; // (+1,-1)
		texelDelta.y = texelDelta.z;
		g = SampleFB( fbTexCoord + texelDelta.xy ).rgb; // (+1,+1)
		h = SampleFB( fbTexCoord - texelDelta.xy ).rgb; // (-1,-1)
	}
	vec4 weights = vec4( dot( abs( a - baseColor ), vec3( 1.0 ) ),
	                   dot( abs( b - baseColor ), vec3( 1.0 ) ),
	                   dot( abs( c - baseColor ), vec3( 1.0 ) ),
	                   dot( abs( d - baseColor ), vec3( 1.0 ) ) ) *
	               DELTA_SCALE;
	vec4 weights2 = vec4( dot( abs( e - baseColor ), vec3( 1.0 ) ),
	                    dot( abs( f - baseColor ), vec3( 1.0 ) ),
	                    dot( abs( g - baseColor ), vec3( 1.0 ) ),
	                    dot( abs( h - baseColor ), vec3( 1.0 ) ) ) *
	                DELTA_SCALE;
	vec4 lumS = vec4( dot( a, a ), dot( b, b ), dot( c, c ), dot( d, d ) );
	lumS.xy = max( lumS.xy, lumS.wz );
	lumS.x = max( lumS.x, lumS.y );
	float maxLumS = max( lumS.x, dot( baseColor, baseColor ) );
	if ( quality1 )
	{
		lumS = vec4( dot( e, e ), dot( f, f ), dot( g, g ), dot( h, h ) );
		lumS.xy = max( lumS.xy, lumS.wz );
		lumS.x = max( lumS.x, lumS.y );
		maxLumS = max( lumS.x, maxLumS );
	}
	const float lumScale = 1.0 / sqrt( maxLumS );
	weights *= lumScale;
	weights2 *= lumScale;
	const float colourDeltaBase = ps.c[1].z * COLOUR_DELTA_BASE;
	weights = clamp( colourDeltaBase + COLOUR_DELTA_CONTRAST * ( weights - colourDeltaBase ), 0.0, 1.0 );
	weights2 =
	    clamp( colourDeltaBase + COLOUR_DELTA_CONTRAST * ( weights2 - colourDeltaBase ), 0.0, 1.0 );
	float unlikeSum = dot( weights, vec4( 1.0 ) );
	vec3 unlike = weights.x * a + weights.y * b + weights.z * c + weights.w * d;
	if ( quality1 )
	{
		unlikeSum += dot( weights2, vec4( 1.0 ) );
		unlike += weights2.x * e + weights2.y * f + weights2.z * g + weights2.w * h;
	}
	// Division by zero leaves lerpFactor 0, as in the original.
	unlike = unlike / unlikeSum;
	if ( reduceOnePixelLineBlur )
	{
		const float ONE_PIXEL_LINE_BIAS_BASE = 0.4;
		const float ONE_PIXEL_LINE_BIAS_CONTRAST = 16.0;
		vec2 unlikeCentroid = vec2( 0.0 );
		unlikeCentroid.x += dot( 1.0 - weights, vec4( 0.0, 1.0, 0.0, -1.0 ) );
		unlikeCentroid.y += dot( 1.0 - weights, vec4( 1.0, 0.0, -1.0, 0.0 ) );
		if ( !quality1 )
		{
			unlikeCentroid /= 4.0 - unlikeSum;
		}
		else
		{
			unlikeCentroid.x += dot( 1.0 - weights2, vec4( -1.0, 1.0, 1.0, -1.0 ) );
			unlikeCentroid.y += dot( 1.0 - weights2, vec4( 1.0, -1.0, 1.0, -1.0 ) );
			unlikeCentroid /= 8.0 - unlikeSum;
		}
		float onePixelLineBias = 1.0 - clamp( length( unlikeCentroid ), 0.0, 1.0 );
		onePixelLineBias = onePixelLineBlurReduction *
		                   clamp( ONE_PIXEL_LINE_BIAS_BASE +
		                              ONE_PIXEL_LINE_BIAS_CONTRAST *
		                                  ( onePixelLineBias - ONE_PIXEL_LINE_BIAS_BASE ),
		                       0.0, 1.0 );
		if ( !quality1 )
			unlikeSum -= 2.0 * onePixelLineBias * 0.4 * clamp( 3.0 - unlikeSum, 0.0, 1.0 );
		else
			unlikeSum -= 2.0 * onePixelLineBias * 1.9 * clamp( 7.0 - unlikeSum, 0.0, 1.0 );
	}
	const float strengthMultiplier = ps.c[1].x;
	float lerpFactor = quality1
	                       ? clamp( strengthMultiplier * DELTA_SCALE * ( ( unlikeSum - 3.0 ) / 3.0 ), 0.0, 1.0 )
	                       : clamp( strengthMultiplier * DELTA_SCALE * ( ( unlikeSum - 1.0 ) / 3.0 ), 0.0, 1.0 );
	lerpFactor = min( lerpFactor, MAX_LERP_FACTOR );
	return mix( baseColor, unlike, lerpFactor );
}

// Engine_Post_ps2x.fxc main (LINEAR_INPUT and LINEAR_OUTPUT are OS X only).
vec4 EnginePost( int dynamicIndex )
{
	const bool aaEnable = ( dynamicIndex % 2 ) != 0;
	const bool aaQuality1 = ( ( dynamicIndex / 2 ) % 2 ) != 0;
	const bool aaReduce = ( ( dynamicIndex / 4 ) % 2 ) != 0;
	const int lookups = ( dynamicIndex / 8 ) % 5;
	// PerformUVTransform: bloomUVs * uvTransform.wz + uvTransform.xy.
	const vec2 fbTexCoord = vUv * ps.c[2].wz + ps.c[2].xy;
	vec4 outColor = vec4( SampleFB( fbTexCoord ).rgb, 1.0 );
	if ( aaEnable )
		outColor.rgb = PerformAA( outColor.rgb, fbTexCoord, aaQuality1, aaReduce );
	const vec4 bloomColor = ps.c[5].x * Sample0( vUv );
	outColor.rgb += bloomColor.rgb;
	if ( lookups > 0 )
	{
		// A 32-texel volume: (0,0,0) reads at 0.5 / 32 and (1,1,1) at 31.5 / 32.
		const vec3 offsetOutColor = outColor.rgb * ( 31.0 / 32.0 ) + ( 0.5 / 32.0 );
		vec3 corrected = outColor.rgb * ps.c[3].x;
		corrected += texture( ColorCorrectionVolumeTexture0, offsetOutColor ).rgb * ps.c[4].x;
		if ( lookups > 1 )
			corrected += texture( ColorCorrectionVolumeTexture1, offsetOutColor ).rgb * ps.c[4].y;
		if ( lookups > 2 )
			corrected += texture( ColorCorrectionVolumeTexture2, offsetOutColor ).rgb * ps.c[4].z;
		if ( lookups > 3 )
			corrected += texture( ColorCorrectionVolumeTexture3, offsetOutColor ).rgb * ps.c[4].w;
		outColor.rgb = corrected;
	}
	return outColor;
}

void main()
{
	const int mode = int( consts.params.y );
	const int staticIndex = int( ps.c[16].x );
	const int dynamicIndex = int( ps.c[16].y );
	vec4 result = vec4( 0.0 );
	if ( mode == kModeDownsample )
		result = Downsample( staticIndex );
	else if ( mode == kModeBlur )
		result = Blur( staticIndex );
	else if ( mode == kModeEnginePost )
		result = EnginePost( dynamicIndex );
	if ( ( int( consts.params.z ) & 4 ) != 0 )
		result.rgb = LinearToSrgb( result.rgb );
	outColor = result;
}
