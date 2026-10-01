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
	SurfaceConstants constants;

	SurfaceVariant Variant() const
	{
		SurfaceVariant variant;
		variant.layout = SurfaceVertexLayout::kModel;
		variant.terms = kSurfacePbr | kSurfaceTransmission | kSurfaceBump;
		variant.blend = envmap ? device::BlendMode::kOpaque : device::BlendMode::kAlpha;
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
