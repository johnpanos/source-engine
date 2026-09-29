// RFC 0014 render.debug-views.v1 on the RFC 0016 core: every debug view's
// formula, in one place. A core program includes this file, fills a
// DebugInputs with the terms it has (the mask names them) and, when
// DebugViewActive(), writes DebugViewOutput() at its single output point
// instead of its shading. The view and its parameters are specialization
// constants (public/render/shaderlib/debug_view.h owns the ids and numbers):
// at their neutral values the branch is dead and the program is the shipped
// one.
//
// Outputs are linear values, written through the target's encoding like any
// color (the lab reads a linear float target). A view whose inputs the
// program lacks draws the not-applicable hatch: an 8-pixel diagonal of 25%
// and 50% grey, never the program's normal shading.
#ifndef DEBUG_VIEW_GLSL
#define DEBUG_VIEW_GLSL

layout( constant_id = 100 ) const int kDebugView = 0;
layout( constant_id = 101 ) const int kDebugBrdf = 0;
layout( constant_id = 102 ) const int kDebugTermsOff = 0;
layout( constant_id = 103 ) const int kDebugFlags = 0;
layout( constant_id = 104 ) const float kDebugScale = 1.0;
layout( constant_id = 105 ) const float kDebugRange = 4096.0;
layout( constant_id = 106 ) const float kDebugThreshold = 1.0;
layout( constant_id = 107 ) const float kDebugForceRoughness = -1.0;
layout( constant_id = 108 ) const float kDebugForceMetalness = -1.0;

// shaderlib::DebugInput
const int kDebugHasAlbedo = 1 << 0;
const int kDebugHasNormal = 1 << 1;
const int kDebugHasNormalMap = 1 << 2;
const int kDebugHasRoughness = 1 << 3;
const int kDebugHasFilteredRoughness = 1 << 4;
const int kDebugHasMetalness = 1 << 5;
const int kDebugHasAo = 1 << 6;
const int kDebugHasBaked = 1 << 7;
const int kDebugHasDirect = 1 << 8;
const int kDebugHasImageSpecular = 1 << 9;
const int kDebugHasSsr = 1 << 10;
const int kDebugHasEmission = 1 << 11;
const int kDebugHasUv0 = 1 << 12;
const int kDebugHasVertexColor = 1 << 13;

// shaderlib::DebugTerm
const int kDebugTermClustered = 1 << 0;
const int kDebugTermSun = 1 << 1;
const int kDebugTermArea = 1 << 2;
const int kDebugTermProjected = 1 << 3;
const int kDebugTermBaked = 1 << 4;
const int kDebugTermProbes = 1 << 5;
const int kDebugTermIbl = 1 << 6;
const int kDebugTermSsr = 1 << 7;
const int kDebugTermAo = 1 << 8;
const int kDebugTermSpecularOcclusion = 1 << 9;
const int kDebugTermEmission = 1 << 10;
const int kDebugTermVolumetric = 1 << 11;

// shaderlib::DebugBrdf
const int kDebugBrdfFull = 0;
const int kDebugBrdfDiffuseOnly = 1;
const int kDebugBrdfSpecularOnly = 2;
const int kDebugBrdfNoEnergyCompensation = 3;
const int kDebugBrdfSplitSumSample = 4;

const int kDebugFlagFurnace = 1 << 0;
const int kDebugFlagFilteredOut = 1 << 1;

struct DebugInputs
{
	int mask;              // kDebugHas* bits
	vec3 albedo;           // linear base color after texture and modulation
	vec3 normal;           // the shading normal, world space, unit length
	vec3 normalMap;        // the decoded tangent-space sample
	float roughness;       // perceptual
	float filteredRoughness;
	float metalness;
	float ao;              // material AO times screen-space AO
	vec3 baked;            // baked irradiance with albedo 1, times the lightmap scale
	vec3 direct;           // the direct-light terms with their visibility
	vec3 imageSpecular;    // the probe / env map specular term
	vec4 ssr;              // the SSR contribution, confidence in alpha
	vec3 emission;
	vec2 uv0;
	vec4 vertexColor;
	vec3 final;            // the color before tone mapping and output encoding
};

DebugInputs DebugInputsNone()
{
	DebugInputs inputs;
	inputs.mask = 0;
	inputs.albedo = vec3( 0.0 );
	inputs.normal = vec3( 0.0, 0.0, 1.0 );
	inputs.normalMap = vec3( 0.0, 0.0, 1.0 );
	inputs.roughness = 0.0;
	inputs.filteredRoughness = 0.0;
	inputs.metalness = 0.0;
	inputs.ao = 1.0;
	inputs.baked = vec3( 0.0 );
	inputs.direct = vec3( 0.0 );
	inputs.imageSpecular = vec3( 0.0 );
	inputs.ssr = vec4( 0.0 );
	inputs.emission = vec3( 0.0 );
	inputs.uv0 = vec2( 0.0 );
	inputs.vertexColor = vec4( 1.0 );
	inputs.final = vec3( 0.0 );
	return inputs;
}

bool DebugViewActive()
{
	return kDebugView != 0 || ( kDebugFlags & kDebugFlagFilteredOut ) != 0;
}

// Whether the frame keeps a lighting-model term (cl_render_debug_term).
bool DebugTermOn( int term )
{
	return ( kDebugTermsOff & term ) == 0;
}

bool DebugFurnace()
{
	return ( kDebugFlags & kDebugFlagFurnace ) != 0;
}

// The not-applicable pattern: period 8 along the diagonal, 25% and 50% grey.
vec3 DebugHatch()
{
	const ivec2 pixel = ivec2( gl_FragCoord.xy );
	return ( ( pixel.x + pixel.y ) & 7 ) < 4 ? vec3( 0.25 ) : vec3( 0.5 );
}

float DebugLuminance( vec3 color )
{
	return dot( color, vec3( 0.2126, 0.7152, 0.0722 ) );
}

vec4 DebugViewOutput( DebugInputs inputs )
{
	if ( ( kDebugFlags & kDebugFlagFilteredOut ) != 0 )
		return vec4( vec3( 0.18 ), 1.0 );
#ifdef SEEDED_DEBUG_NO_HATCH
	const vec3 hatch = inputs.final; // normal shading where the view does not apply
#else
	const vec3 hatch = DebugHatch();
#endif
	vec3 result = hatch;
	if ( kDebugView == 1 )
		result = ( inputs.mask & kDebugHasAlbedo ) != 0 ? inputs.albedo : hatch;
	else if ( kDebugView == 2 )
	{
#ifdef SEEDED_DEBUG_SWAPPED_NORMAL
		result = ( inputs.mask & kDebugHasNormal ) != 0 ? inputs.normal.zyx * 0.5 + 0.5 : hatch;
#else
		result = ( inputs.mask & kDebugHasNormal ) != 0 ? inputs.normal * 0.5 + 0.5 : hatch;
#endif
	}
	else if ( kDebugView == 3 )
		result = ( inputs.mask & kDebugHasNormalMap ) != 0 ? inputs.normalMap * 0.5 + 0.5 : hatch;
	else if ( kDebugView == 4 )
		result = ( inputs.mask & kDebugHasRoughness ) != 0 ? vec3( inputs.roughness ) : hatch;
	else if ( kDebugView == 5 )
		result = ( inputs.mask & kDebugHasFilteredRoughness ) != 0
		             ? vec3( inputs.filteredRoughness )
		             : hatch;
	else if ( kDebugView == 6 )
		result = ( inputs.mask & kDebugHasMetalness ) != 0 ? vec3( inputs.metalness ) : hatch;
	else if ( kDebugView == 7 )
		result = ( inputs.mask & kDebugHasAo ) != 0 ? vec3( inputs.ao ) : hatch;
	else if ( kDebugView == 8 )
		result = ( inputs.mask & kDebugHasBaked ) != 0 ? inputs.baked * kDebugScale : hatch;
	else if ( kDebugView == 9 )
		result = ( inputs.mask & kDebugHasDirect ) != 0 ? inputs.direct * kDebugScale : hatch;
	else if ( kDebugView == 10 )
		result = ( inputs.mask & kDebugHasImageSpecular ) != 0 ? inputs.imageSpecular * kDebugScale
		                                                        : hatch;
	else if ( kDebugView == 11 )
	{
		if ( ( inputs.mask & kDebugHasSsr ) != 0 )
			return vec4( inputs.ssr.rgb * kDebugScale, inputs.ssr.a );
	}
	else if ( kDebugView == 12 )
		result = ( inputs.mask & kDebugHasEmission ) != 0 ? inputs.emission * kDebugScale : hatch;
	else if ( kDebugView == 13 )
	{
		if ( ( inputs.mask & kDebugHasUv0 ) != 0 )
		{
			const vec2 cell = floor( 8.0 * inputs.uv0 );
			result = vec3( mod( cell.x + cell.y, 2.0 ) );
		}
	}
	else if ( kDebugView == 14 )
		result = ( inputs.mask & kDebugHasVertexColor ) != 0 ? inputs.vertexColor.rgb : hatch;
	else if ( kDebugView == 15 )
	{
		// gl_FragCoord.w is 1 / clip w, and clip w is the view-space depth.
		result = vec3( clamp( ( 1.0 / gl_FragCoord.w ) / kDebugRange, 0.0, 1.0 ) );
	}
	else if ( kDebugView == 16 )
	{
		const vec3 c = inputs.final;
#ifdef SEEDED_DEBUG_MISS_NAN
		if ( any( isinf( c ) ) )
#else
		if ( any( isnan( c ) ) )
			result = vec3( 1.0, 0.0, 1.0 );
		else if ( any( isinf( c ) ) )
#endif
			result = vec3( 0.0, 1.0, 1.0 );
		else if ( any( lessThan( c, vec3( 0.0 ) ) ) )
			result = vec3( 1.0, 1.0, 0.0 );
		else
			result = vec3( DebugLuminance( c ) * 0.5 );
	}
	else if ( kDebugView == 17 )
	{
		const float luminance = DebugLuminance( inputs.final );
		result = luminance > kDebugThreshold ? vec3( 1.0, 0.0, 0.0 ) : vec3( luminance * 0.5 );
	}
#ifdef SEEDED_DEBUG_TONE_MAPS
	result = result / ( 1.0 + result );
#endif
	return vec4( result, 1.0 );
}

#endif // DEBUG_VIEW_GLSL
