// The lightmap basis (RFC 0016 render.lighting.v1, "Indirect diffuse, static
// surfaces"): how a world surface's baked diffuse light is read at its
// shading normal. The one GLSL copy; its C++ oracle is
// render/lab/lightmap_basis_suite.cpp (render.lab.lightmap-basis), which
// shares no code with it. Values are the pages' diffuse light (irradiance /
// pi, RFC 0007); the includer applies its lightmap scale and the albedo.
//
// Three encodings:
// - flat: one page, the light on the surface's smooth normal;
// - directional (an LMAP page twice as wide as it is tall; RFC 0008,
//   tools/quality/lightmap_directional.py): the flat light E0 on the smooth
//   normal N, and at the same texel the signed world-space luminance
//   gradient beta of the fitted irradiance E(n) = a + g . n relative to E0,
//   staged as two pages of the same size. The gradient page stores beta *
//   0.5 + 0.5 (LMAP v3's BC7 page, or its RGBA16F split), so
//   LightmapGradientSample returns beta = g * 2 - 1; its alpha is the sun's
//   baked visibility. A normal n receives E0 * clamp( 1 + beta . ( n - N ),
//   0, 4 ), so n = N is the flat light bitwise (and a zero gradient, 0.5
//   stored, on an RGBA16F page; BC7 stores it as 128 / 255);
// - RNM (legacy bumped lightmaps, lightmappedgeneric_ps2_3_x.h): three pages
//   beside the flat one at the page offset (TEXCOORD2.x), one per Source
//   bump basis direction, weighted by the tangent-space normal's squared
//   clamped cosines, or by an ssbump's own texel.
//
// The sampled forms read level 0 with linear filtering (textureLod, so the
// functions serve compute as well as fragment stages); lightmap pages carry
// one level, where this equals texture().

// common_fxc.h's bump basis (tangent space).
const float kLightmapOoSqrt3 = 0.57735025882720947;
const vec3 kLightmapBumpBasis[3] = vec3[3]( vec3( 0.81649661064147949, 0.0, kLightmapOoSqrt3 ),
    vec3( -0.40824833512306213, 0.70710676908493042, kLightmapOoSqrt3 ),
    vec3( -0.40824821591377258, -0.7071068286895752, kLightmapOoSqrt3 ) );

// lightmap_directional.py GAIN_MAX: |n - N| <= 2 and |beta| <= 2 bound the
// fit's gain; the clamp keeps a mapped normal's light finite and positive.
const float kLightmapDirectionalGainMax = 4.0;

vec3 LightmapPageSample( texture2D page, sampler pageSampler, vec2 uv )
{
	return textureLod( sampler2D( page, pageSampler ), uv, 0.0 ).rgb;
}

// A gradient page's beta (stored beta * 0.5 + 0.5).
vec3 LightmapGradientSample( texture2D page, sampler pageSampler, vec2 uv )
{
#ifdef SEEDED_LIGHTMAP_GRADIENT_UNBIASED
	return textureLod( sampler2D( page, pageSampler ), uv, 0.0 ).rgb;
#else
	return textureLod( sampler2D( page, pageSampler ), uv, 0.0 ).rgb * 2.0 - 1.0;
#endif
}

// Directional: the flat light at the smooth normal carried to `normal`.
vec3 LightmapDirectional( vec3 flatLight, vec3 gradient, vec3 normal, vec3 smoothNormal )
{
#ifdef SEEDED_LIGHTMAP_NO_SMOOTH_NORMAL
	const float gain = 1.0 + dot( gradient, normal );
#else
	const float gain = 1.0 + dot( gradient, normal - smoothNormal );
#endif
#ifdef SEEDED_LIGHTMAP_NO_GAIN_CLAMP
	return flatLight * gain;
#else
	return flatLight * clamp( gain, 0.0, kLightmapDirectionalGainMax );
#endif
}

vec3 LightmapDirectionalSample( texture2D flatPage, texture2D gradientPage, sampler pageSampler,
    vec2 uv, vec3 normal, vec3 smoothNormal )
{
	return LightmapDirectional( LightmapPageSample( flatPage, pageSampler, uv ),
	    LightmapGradientSample( gradientPage, pageSampler, uv ), normal, smoothNormal );
}

// RNM: bumped page k (1 to 3) sits k page offsets right of the flat page.
vec2 LightmapRnmCoordinate( vec2 uv, float pageOffset, int page )
{
#ifdef SEEDED_LIGHTMAP_RNM_OFFSET_FROM_ZERO
	return uv + float( page - 1 ) * vec2( pageOffset, 0.0 );
#else
	return uv + float( page ) * vec2( pageOffset, 0.0 );
#endif
}

// A normal map's weights: each basis direction's clamped cosine, squared.
// The light is the weighted sum over their sum (LightmapRnm).
vec3 LightmapRnmWeights( vec3 tangentNormal )
{
	vec3 weights;
	weights.x = clamp( dot( tangentNormal, kLightmapBumpBasis[0] ), 0.0, 1.0 );
	weights.y = clamp( dot( tangentNormal, kLightmapBumpBasis[1] ), 0.0, 1.0 );
	weights.z = clamp( dot( tangentNormal, kLightmapBumpBasis[2] ), 0.0, 1.0 );
#ifndef SEEDED_LIGHTMAP_RNM_UNSQUARED
	weights *= weights;
#endif
	return weights;
}

// The weighted sum of the three bumped pages' light. A normal map's weights
// (LightmapRnmWeights) then divide by their sum; an ssbump's texel weights
// scale by its weight scale instead (1/sqrt(3) with $ssbumpmathfix).
vec3 LightmapRnmSum( vec3 weights, vec3 light1, vec3 light2, vec3 light3 )
{
	return weights.x * light1 + weights.y * light2 + weights.z * light3;
}

// A normal-mapped surface's RNM light (weights over their sum).
vec3 LightmapRnmSample(
    texture2D page, sampler pageSampler, vec2 uv, float pageOffset, vec3 tangentNormal )
{
	const vec3 weights = LightmapRnmWeights( tangentNormal );
	vec3 light[3];
	for ( int k = 0; k < 3; ++k )
	{
		light[k] =
		    LightmapPageSample( page, pageSampler, LightmapRnmCoordinate( uv, pageOffset, k + 1 ) );
	}
	return LightmapRnmSum( weights, light[0], light[1], light[2] ) / dot( weights, vec3( 1.0 ) );
}
