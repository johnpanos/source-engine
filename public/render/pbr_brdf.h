//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The metal/roughness direct BRDF for RFC 0007. Inputs are unit-vector
//          dot products in [0, 1]. The layered evaluator clamps roughness to
//          0.02 before evaluating the finite GGX lobe, compensates it for
//          multiple scattering, and weights Lambertian diffuse by what the
//          split-sum specular directional albedo leaves. Results are linear
//          radiometric values without color encoding or exposure.
//          shaders/pbr_brdf.glsl is the GPU copy of these functions; the
//          render.pbr-brdf.glsl suite evaluates both on the same inputs.
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

// Multiple-scattering energy compensation (Kulla and Conty 2017, in the form
// of Filament's surface_shading_lobes: 1 + F0 (1 / E(1) - 1)). The GGX lobe
// above is single-scatter: light that bounces between microfacets more than
// once is lost, up to 1 - ln(2) of it for a rough white metal. E(1) = A + B is
// the table's single-scatter albedo for F0 = 1, so a white conductor's
// compensated albedo is exactly one; other F0 are scaled proportionally.
[[nodiscard]] inline float SpecularEnergyCompensation(
    float reflectanceAtNormal, SplitSumCoefficients albedo )
{
	const float white = std::fmax( albedo.a + albedo.b, 1e-4f );
	return 1.0f + reflectanceAtNormal * ( 1.0f / white - 1.0f );
}

// The multiple-scattering directional specular albedo, F0 A + B times the
// compensation, clamped to one. Image light is weighted by it, and the
// Lambertian layer beneath receives what it leaves.
[[nodiscard]] inline float SpecularDirectionalAlbedo(
    float reflectanceAtNormal, SplitSumCoefficients albedo )
{
	return std::fmin( 1.0f, ( reflectanceAtNormal * albedo.a + albedo.b ) *
	                            SpecularEnergyCompensation( reflectanceAtNormal, albedo ) );
}

// EvaluateSpecular times the energy compensation at N.V.
[[nodiscard]] inline Color EvaluateSpecularMultiScatter( Color reflectanceAtNormal,
    float normalDotView, float normalDotLight, float normalDotHalf, float viewDotHalf,
    float perceptualRoughness )
{
	const Color single = EvaluateSpecular( reflectanceAtNormal, normalDotView, normalDotLight,
	    normalDotHalf, viewDotHalf, perceptualRoughness );
	const SplitSumCoefficients albedo = SampleSplitSum( normalDotView, perceptualRoughness );
	return { single.red * SpecularEnergyCompensation( reflectanceAtNormal.red, albedo ),
	    single.green * SpecularEnergyCompensation( reflectanceAtNormal.green, albedo ),
	    single.blue * SpecularEnergyCompensation( reflectanceAtNormal.blue, albedo ) };
}

// Direct BRDF, before multiplying incident radiance and N.L: the compensated
// specular lobe, and the diffuse layer weighted by what the view-direction
// specular albedo (the same split-sum table as IBL) leaves.
[[nodiscard]] inline Color EvaluateLayeredDirect( Color base, float metalness, float normalDotView,
    float normalDotLight, float normalDotHalf, float viewDotHalf, float perceptualRoughness )
{
	if ( normalDotView <= 0.0f || normalDotLight <= 0.0f )
		return { 0.0f, 0.0f, 0.0f };
	const float roughness = std::fmax( perceptualRoughness, 0.02f );
	const Color f0 = { 0.04f * ( 1.0f - metalness ) + base.red * metalness,
	    0.04f * ( 1.0f - metalness ) + base.green * metalness,
	    0.04f * ( 1.0f - metalness ) + base.blue * metalness };
	const Color specular = EvaluateSpecularMultiScatter(
	    f0, normalDotView, normalDotLight, normalDotHalf, viewDotHalf, roughness );
	const SplitSumCoefficients albedo = SampleSplitSum( normalDotView, roughness );
	const float diffuseScale = ( 1.0f - metalness ) / kPi;
	return { specular.red +
	             base.red * diffuseScale * ( 1.0f - SpecularDirectionalAlbedo( f0.red, albedo ) ),
	    specular.green +
	        base.green * diffuseScale * ( 1.0f - SpecularDirectionalAlbedo( f0.green, albedo ) ),
	    specular.blue +
	        base.blue * diffuseScale * ( 1.0f - SpecularDirectionalAlbedo( f0.blue, albedo ) ) };
}

// Clear coat, after Filament's standard model (Apache-2.0, google/filament
// shaders/src/surface_shading_model_standard.fs): a dielectric layer of IOR
// 1.5 (F0 0.04) with a GGX lobe and Kelemen visibility 1 / (4 (L.H)^2), shaded
// with its own normal. `weight` is $clearcoat in [0, 1]. The coat's specular
// BRDF (before incident radiance and N.L) is `specular`; the layer beneath is
// attenuated by 1 - `fresnel`.
struct ClearCoat
{
	float specular;
	float fresnel;
};

[[nodiscard]] inline ClearCoat EvaluateClearCoat(
    float weight, float coatNormalDotHalf, float lightDotHalf, float perceptualRoughness )
{
	const float roughness = std::fmax( perceptualRoughness, 0.02f );
	const float fresnel = FresnelSchlick( 0.04f, lightDotHalf ) * weight;
	const float visibility = 0.25f / std::fmax( lightDotHalf * lightDotHalf, 1e-6f );
	return { GgxDistribution( coatNormalDotHalf, roughness ) * visibility * fresnel, fresnel };
}
} // namespace render::pbr

#endif // RENDER_PBR_BRDF_H
