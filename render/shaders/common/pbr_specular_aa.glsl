// Geometric specular antialiasing (render.pbr-specular-aa.v1, RFC 0012 A2):
// the GPU copy of public/render/pbr_specular_aa.h. The caller passes the
// final shading normal's screen derivatives (dFdx/dFdy in a fragment, given
// values in render_lab's check kernel). Zero derivatives return the input.
#ifndef PBR_SPECULAR_AA_GLSL
#define PBR_SPECULAR_AA_GLSL

const float kSpecularAaScreenVariance = 0.15;
const float kSpecularAaThreshold = 0.2;

float SpecularAaRoughness( float perceptualRoughness, vec3 normalDx, vec3 normalDy )
{
#if defined( SEEDED_SPECULAR_AA_DISABLED )
	return perceptualRoughness;
#elif defined( SEEDED_SPECULAR_AA_BIASED )
	return min( perceptualRoughness + 0.05, 1.0 );
#else
	const float variance =
	    kSpecularAaScreenVariance * ( dot( normalDx, normalDx ) + dot( normalDy, normalDy ) );
#if defined( SEEDED_SPECULAR_AA_NO_THRESHOLD )
	const float kernel = 2.0 * variance;
#else
	const float kernel = min( 2.0 * variance, kSpecularAaThreshold );
#endif
	if ( !( kernel > 0.0 ) )
		return perceptualRoughness;
	const float alpha = perceptualRoughness * perceptualRoughness;
	return sqrt( sqrt( min( alpha * alpha + kernel, 1.0 ) ) );
#endif
}

#endif
