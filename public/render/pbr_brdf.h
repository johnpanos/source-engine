//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The scalar metal/roughness specular model for RFC 0007. Inputs are
//          unit-vector dot products in [0, 1], and roughness is in (0, 1].
//          A zero-roughness delta lobe has no finite BRDF value and is handled
//          by the material's declared minimum roughness before calling here.
//          These functions return linear radiometric values; they do not encode
//          color or apply exposure.
//
//===========================================================================//

#ifndef RENDER_PBR_BRDF_H
#define RENDER_PBR_BRDF_H

#include <cmath>

namespace render::pbr
{
constexpr float kPi = 3.14159265358979323846f;

struct Color
{
	float red;
	float green;
	float blue;
};

// Trowbridge-Reitz GGX normal distribution. alpha = perceptualRoughness^2.
[[nodiscard]] inline float GgxDistribution( float normalDotHalf, float perceptualRoughness )
{
	const float alpha = perceptualRoughness * perceptualRoughness;
	const float alphaSquared = alpha * alpha;
	const float denominator = normalDotHalf * normalDotHalf * ( alphaSquared - 1.0f ) + 1.0f;
	return alphaSquared / ( kPi * denominator * denominator );
}

// Height-correlated Smith G2 divided by 4 (N.V)(N.L). Keeping the visibility
// term factored avoids a 0/0 at the horizon when the BRDF is multiplied by N.L.
[[nodiscard]] inline float SmithVisibility(
    float normalDotView, float normalDotLight, float perceptualRoughness )
{
	const float alpha = perceptualRoughness * perceptualRoughness;
	const float alphaSquared = alpha * alpha;
	const float lambdaView =
	    std::sqrt( alphaSquared + ( 1.0f - alphaSquared ) * normalDotView * normalDotView );
	const float lambdaLight =
	    std::sqrt( alphaSquared + ( 1.0f - alphaSquared ) * normalDotLight * normalDotLight );
	const float denominator = normalDotView * lambdaLight + normalDotLight * lambdaView;
	return denominator > 0.0f ? 0.5f / denominator : 0.0f;
}

[[nodiscard]] inline float FresnelSchlick( float reflectanceAtNormal, float viewDotHalf )
{
	const float grazing = 1.0f - viewDotHalf;
	const float grazingSquared = grazing * grazing;
	return reflectanceAtNormal +
	       ( 1.0f - reflectanceAtNormal ) * grazingSquared * grazingSquared * grazing;
}

[[nodiscard]] inline Color EvaluateSpecular( Color reflectanceAtNormal, float normalDotView,
    float normalDotLight, float normalDotHalf, float viewDotHalf, float perceptualRoughness )
{
	if ( normalDotView <= 0.0f || normalDotLight <= 0.0f )
	{
		return { 0.0f, 0.0f, 0.0f };
	}
	const float distribution = GgxDistribution( normalDotHalf, perceptualRoughness );
	const float visibility = SmithVisibility( normalDotView, normalDotLight, perceptualRoughness );
	return { FresnelSchlick( reflectanceAtNormal.red, viewDotHalf ) * distribution * visibility,
	    FresnelSchlick( reflectanceAtNormal.green, viewDotHalf ) * distribution * visibility,
	    FresnelSchlick( reflectanceAtNormal.blue, viewDotHalf ) * distribution * visibility };
}
} // namespace render::pbr

#endif // RENDER_PBR_BRDF_H
