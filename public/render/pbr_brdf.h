//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The metal/roughness direct BRDF for RFC 0007. Inputs are unit-vector
//          dot products in [0, 1]. The layered evaluator clamps roughness to
//          0.02 before evaluating the finite GGX lobe and weights Lambertian
//          diffuse by the split-sum specular directional albedo. Results are
//          linear radiometric values without color encoding or exposure.
//
//===========================================================================//

#ifndef RENDER_PBR_BRDF_H
#define RENDER_PBR_BRDF_H

#include "render/pbr_split_sum_table.h"

#include <algorithm>
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

[[nodiscard]] inline SplitSumCoefficients SampleSplitSum( float normalDotView, float roughness )
{
	const float view = std::fmax( 0.0f, std::fmin( normalDotView, 1.0f ) );
	const float perceptualRoughness = std::fmax( 0.0f, std::fmin( roughness, 1.0f ) );
	const float x = view * kSplitSumSize - 0.5f;
	const float y = perceptualRoughness * kSplitSumSize - 0.5f;
	const int lowX =
	    std::max( 0, std::min( static_cast<int>( std::floor( x ) ), kSplitSumSize - 1 ) );
	const int lowY =
	    std::max( 0, std::min( static_cast<int>( std::floor( y ) ), kSplitSumSize - 1 ) );
	const int highX = std::min( lowX + 1, kSplitSumSize - 1 );
	const int highY = std::min( lowY + 1, kSplitSumSize - 1 );
	const float fractionX = std::fmax( 0.0f, std::fmin( x - lowX, 1.0f ) );
	const float fractionY = std::fmax( 0.0f, std::fmin( y - lowY, 1.0f ) );
	const SplitSumCoefficients &bottomLeft = kSplitSumTable[lowY * kSplitSumSize + lowX];
	const SplitSumCoefficients &bottomRight = kSplitSumTable[lowY * kSplitSumSize + highX];
	const SplitSumCoefficients &topLeft = kSplitSumTable[highY * kSplitSumSize + lowX];
	const SplitSumCoefficients &topRight = kSplitSumTable[highY * kSplitSumSize + highX];
	const float bottomA = bottomLeft.a + ( bottomRight.a - bottomLeft.a ) * fractionX;
	const float bottomB = bottomLeft.b + ( bottomRight.b - bottomLeft.b ) * fractionX;
	const float topA = topLeft.a + ( topRight.a - topLeft.a ) * fractionX;
	const float topB = topLeft.b + ( topRight.b - topLeft.b ) * fractionX;
	return { bottomA + ( topA - bottomA ) * fractionY, bottomB + ( topB - bottomB ) * fractionY };
}

// Direct BRDF, before multiplying incident radiance and N.L. The diffuse layer
// uses the view-direction specular albedo from the same split-sum table as IBL.
[[nodiscard]] inline Color EvaluateLayeredDirect( Color base, float metalness, float normalDotView,
    float normalDotLight, float normalDotHalf, float viewDotHalf, float perceptualRoughness )
{
	if ( normalDotView <= 0.0f || normalDotLight <= 0.0f )
		return { 0.0f, 0.0f, 0.0f };
	const float roughness = std::fmax( perceptualRoughness, 0.02f );
	const Color f0 = { 0.04f * ( 1.0f - metalness ) + base.red * metalness,
	    0.04f * ( 1.0f - metalness ) + base.green * metalness,
	    0.04f * ( 1.0f - metalness ) + base.blue * metalness };
	const Color specular = EvaluateSpecular(
	    f0, normalDotView, normalDotLight, normalDotHalf, viewDotHalf, roughness );
	const SplitSumCoefficients albedo = SampleSplitSum( normalDotView, roughness );
	const float diffuseScale = ( 1.0f - metalness ) / kPi;
	return { specular.red + base.red * diffuseScale *
	                            ( 1.0f - std::fmin( 1.0f, f0.red * albedo.a + albedo.b ) ),
	    specular.green + base.green * diffuseScale *
	                         ( 1.0f - std::fmin( 1.0f, f0.green * albedo.a + albedo.b ) ),
	    specular.blue + base.blue * diffuseScale *
	                        ( 1.0f - std::fmin( 1.0f, f0.blue * albedo.a + albedo.b ) ) };
}
} // namespace render::pbr

#endif // RENDER_PBR_BRDF_H
