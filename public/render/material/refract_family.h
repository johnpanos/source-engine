//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Refract model-surface claim on the shared surface program.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_REFRACT_FAMILY_H
#define RENDER_MATERIAL_REFRACT_FAMILY_H

#include "render/material/parameter_block.h"
#include "render/material/surface_program.h"

#include <string>

namespace render::material
{

struct RefractClaim
{
	bool claimed = false;
	std::string reason;
	bool envmap = false;
	bool nativeProbe = false;
	bool sceneColor = false;
	bool baseTexture = false;
	// $refracttinttexture: the refracted color's tint is 2 x $refracttint x
	// the texture at the normal map's coordinates (refract_ps2x.fxc), bound
	// through the material's emission binding (sRGB, unused by Refract).
	bool tintTexture = false;
	// Portal 2's $localrefract: the base texture refracted in texture space
	// (refract_ps2x LOCALREFRACT). No scene color; opaque unless $translucent.
	bool local = false;
	bool translucent = false;
	SurfaceConstants constants;

	device::BlendMode Blend() const
	{
		if ( local )
			return translucent ? device::BlendMode::kAlpha : device::BlendMode::kOpaque;
		return envmap ? device::BlendMode::kOpaque : device::BlendMode::kAlpha;
	}

	SurfaceVariant Variant() const
	{
		SurfaceVariant variant;
		variant.layout = SurfaceVertexLayout::kModel;
		variant.terms = kSurfacePbr | kSurfaceTransmission | kSurfaceBump;
		variant.blend = Blend();
		variant.alphaWrite = false;
		return variant;
	}
};

// Only the simple Refract_DX90 model point: one normal map, screen displacement,
// tint, optional blur and an optional native reflection probe. The caller must
// own a linear scene-color snapshot behind this draw.
RefractClaim ClaimRefract( const ParameterBlock &block, bool sceneColorAvailable,
    bool usesNativeProbe, bool nativeReflectionProbes );

} // namespace render::material

#endif // RENDER_MATERIAL_REFRACT_FAMILY_H
