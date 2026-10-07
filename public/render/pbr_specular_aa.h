//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Geometric specular antialiasing for the PBR lobe
//          (render.pbr-specular-aa.v1, RFC 0012 A2). The screen-space change
//          of the final shading normal (after normal mapping) widens GGX
//          roughness: its variance, scaled by the kernel's screen variance,
//          is added in alpha^2 space, clamped by a threshold, and converted
//          back to perceptual roughness (Kaplanyan et al. 2016; Tokuyoshi
//          and Kaplanyan 2019, in Filament's form). Zero derivatives return
//          the input exactly, so constant-normal surfaces are unchanged.
//          render/shaders/common/pbr_specular_aa.glsl is the GPU copy; the
//          render.lab specular-aa suite evaluates both on the same inputs.
//
//===========================================================================//

#ifndef RENDER_PBR_SPECULAR_AA_H
#define RENDER_PBR_SPECULAR_AA_H

#include <cmath>

namespace render::pbr
{
// The contract's version: a change of either constant or of the formula
// increments it.
inline constexpr int kSpecularAaVersion = 1;
// The filter kernel's screen-space variance (Filament's default).
inline constexpr float kSpecularAaScreenVariance = 0.15f;
// The largest alpha^2 the kernel may add.
inline constexpr float kSpecularAaThreshold = 0.2f;

// The filtered perceptual roughness for `perceptualRoughness` whose shading
// normal changes by (dxX, dxY, dxZ) and (dyX, dyY, dyZ) per pixel.
[[nodiscard]] inline float SpecularAaRoughness(
    float perceptualRoughness, float dxX, float dxY, float dxZ, float dyX, float dyY, float dyZ )
{
	const float variance = kSpecularAaScreenVariance * ( dxX * dxX + dxY * dxY + dxZ * dxZ +
	                                                       dyX * dyX + dyY * dyY + dyZ * dyZ );
	const float kernel = std::fmin( 2.0f * variance, kSpecularAaThreshold );
	if ( !( kernel > 0.0f ) )
		return perceptualRoughness;
	const float alpha = perceptualRoughness * perceptualRoughness;
	const float filtered = std::fmin( alpha * alpha + kernel, 1.0f );
	return std::sqrt( std::sqrt( filtered ) );
}
} // namespace render::pbr

#endif // RENDER_PBR_SPECULAR_AA_H
