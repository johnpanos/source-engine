// The RFC 0007 metal/roughness BRDF on the GPU: the one GLSL copy of
// public/render/pbr_brdf.h, included by every PBR stage (pbr_direct.frag,
// world_pbr.frag, world_pbr_glass.frag, model_pbr.frag). The
// render.pbr-brdf.glsl suite runs these functions in a compute program
// (pbr_brdf_check.comp) and compares them with the C++ on the same inputs.
// Roughness is perceptual (alpha = roughness^2) as in the C++; dot products
// are clamped by the caller.
#ifndef PBR_BRDF_GLSL
#define PBR_BRDF_GLSL

const float kPi = 3.14159265358979323846;

// Trowbridge-Reitz GGX normal distribution.
float PbrGgxDistribution( float normalDotHalf, float roughness )
{
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;
	float denominator = normalDotHalf * normalDotHalf * ( alphaSquared - 1.0 ) + 1.0;
	return alphaSquared / ( kPi * denominator * denominator );
}

// Height-correlated Smith G2 over 4 (N.V)(N.L).
float PbrSmithVisibility( float normalDotView, float normalDotLight, float roughness )
{
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;
	float lambdaView =
	    sqrt( alphaSquared + ( 1.0 - alphaSquared ) * normalDotView * normalDotView );
	float lambdaLight =
	    sqrt( alphaSquared + ( 1.0 - alphaSquared ) * normalDotLight * normalDotLight );
	float denominator = normalDotView * lambdaLight + normalDotLight * lambdaView;
	return denominator > 0.0 ? 0.5 / denominator : 0.0;
}

float PbrFresnelSchlick( float reflectanceAtNormal, float viewDotHalf )
{
	float grazing = 1.0 - viewDotHalf;
	float grazingSquared = grazing * grazing;
	return reflectanceAtNormal +
	       ( 1.0 - reflectanceAtNormal ) * grazingSquared * grazingSquared * grazing;
}

vec3 PbrFresnelSchlick( vec3 reflectanceAtNormal, float viewDotHalf )
{
	float grazing = 1.0 - viewDotHalf;
	float grazingSquared = grazing * grazing;
	return reflectanceAtNormal +
	       ( vec3( 1.0 ) - reflectanceAtNormal ) * grazingSquared * grazingSquared * grazing;
}

// The single-scatter specular BRDF (EvaluateSpecular), zero below either horizon.
vec3 PbrSpecular( vec3 reflectanceAtNormal, float normalDotView, float normalDotLight,
    float normalDotHalf, float viewDotHalf, float roughness )
{
	if ( normalDotView <= 0.0 || normalDotLight <= 0.0 )
		return vec3( 0.0 );
	return PbrFresnelSchlick( reflectanceAtNormal, viewDotHalf ) *
	       PbrGgxDistribution( normalDotHalf, roughness ) *
	       PbrSmithVisibility( normalDotView, normalDotLight, roughness );
}

// PbrSpecular for unit vectors toward the eye and the light.
vec3 PbrSpecular( vec3 normal, vec3 view, vec3 light, vec3 reflectanceAtNormal, float roughness )
{
	vec3 halfVector = normalize( view + light );
	return PbrSpecular( reflectanceAtNormal, max( dot( normal, view ), 0.0 ),
	    max( dot( normal, light ), 0.0 ), max( dot( normal, halfVector ), 0.0 ),
	    max( dot( view, halfVector ), 0.0 ), roughness );
}

// The split-sum table's (A, B) at (N.V, roughness): clamped linear sampling.
vec2 PbrSplitSum( sampler2D table, float normalDotView, float roughness )
{
	return texture( table, clamp( vec2( normalDotView, roughness ), vec2( 0.0 ), vec2( 1.0 ) ) )
	    .rg;
}

// SpecularEnergyCompensation: 1 + F0 (1 / (A + B) - 1).
vec3 PbrEnergyCompensation( vec3 reflectanceAtNormal, vec2 splitSum )
{
	float white = max( splitSum.x + splitSum.y, 1e-4 );
	return vec3( 1.0 ) + reflectanceAtNormal * ( 1.0 / white - 1.0 );
}

// SpecularDirectionalAlbedo: the multiple-scattering albedo, at most one.
vec3 PbrDirectionalAlbedo( vec3 reflectanceAtNormal, vec2 splitSum )
{
	return min( vec3( 1.0 ), ( reflectanceAtNormal * splitSum.x + vec3( splitSum.y ) ) *
	                             PbrEnergyCompensation( reflectanceAtNormal, splitSum ) );
}

// EvaluateClearCoat: the coat's specular BRDF (.x) and its Fresnel weight (.y),
// by which the layer beneath is attenuated. IOR 1.5, Kelemen visibility.
vec2 PbrClearCoat( float weight, float coatNormalDotHalf, float lightDotHalf, float roughness )
{
	roughness = max( roughness, 0.02 );
	float fresnel = PbrFresnelSchlick( 0.04, lightDotHalf ) * weight;
	float visibility = 0.25 / max( lightDotHalf * lightDotHalf, 1e-6 );
	return vec2( PbrGgxDistribution( coatNormalDotHalf, roughness ) * visibility * fresnel,
	    fresnel );
}

#endif // PBR_BRDF_GLSL
