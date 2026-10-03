//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `pbr` material family (RFC 0016 K4): PBRMetalRough (RFC 0007,
//			which owns its semantics and the BRDF) on meshes: its claim on a
//			material and the packing of a parameter block into the surface
//			program's constants. The family's draws are the pbr point of the
//			one surface program (surface_program.h, RFC 0016 K11: kSurfacePbr
//			on the model vertex), which owns the bind groups and pipelines.
//
//			The family claims this subset of the RFC 0007 schema:
	//			$basetexture, $mraotexture, $bumpmap, $emissiontexture with
//			$emissionscale, $alphatest, $alphatestreference and $translucent;
//			thin $transmission with $ior when the caller supplies
//			linear scene color
//			($fallbackmaterial names another profile's
//			material and is accepted). ClaimPbr names the first parameter
//			outside the subset that a material sets away from its default
//			($envmap, thick glass, clear coat); such a
//			material stays on the native stages until the family claims it.
//
//			The arithmetic is the model port's (model_pbr.frag) without map
//			probes, environment maps, the probe volume or clear coat: the
//			layered BRDF of render/shaders/common/pbr_brdf.glsl under
//			Source's model lighting, with the ambient cube as the specular
//			image light. The frame group names the split-sum table and holds
//			the eye; the draw group holds the draw's model lighting
//			(model_lighting.h).
//
//=============================================================================//

#ifndef RENDER_MATERIAL_PBR_FAMILY_H
#define RENDER_MATERIAL_PBR_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/shaderlib/debug_view.h"
#include "render/material/parameter_block.h"
#include "render/material/surface_program.h"

#include <cstdint>
#include <memory>
#include <string>

namespace render::material
{

struct PbrClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	bool normalMap = false;
	bool emission = false;
	bool alphaTest = false;
	bool translucent = false;
	bool transmission = false;
	SurfaceConstants constants; // emission.x: $emissionscale

	// The program's point for this claim: kSurfacePbr on the model vertex.
	SurfaceVariant Variant() const
	{
		SurfaceVariant variant;
		variant.terms = kSurfacePbr | kSurfaceMraoTexture | ( normalMap ? kSurfaceBump : 0u ) |
		                ( emission ? kSurfaceEmissionTexture : 0u ) |
		                ( transmission ? kSurfaceTransmission : 0u );
		variant.layout = SurfaceVertexLayout::kModel;
		variant.blend = translucent ? device::BlendMode::kAlpha : device::BlendMode::kOpaque;
		variant.alphaWrite = !alphaTest && !translucent;
		return variant;
	}
};

// Whether the family draws this block's material, and how. The block must be
// of the `pbr` family's schema (FamiliesFromMapping). An unused texture slot
// (no normal map or emission) still needs a texture bound; its contents are
// not read.
// A transmission claim needs the frame's linear scene-color input. The
// product's current resolver passes false until its translucent stage binds
// that input; render_lab can prove the point with true.
PbrClaim ClaimPbr( const ParameterBlock &block, bool sceneColorAvailable = false );

using PbrStatus = SurfaceStatus;

class PbrFamily : public SurfaceFamily
{
public:
	using SurfaceFamily::SurfaceFamily;
	static foundation::Expected<std::unique_ptr<PbrFamily>, PbrStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat )
	{
		return CreateSurfaceFamily<PbrFamily>( device, colorFormat, depthFormat );
	}

	// The claim's pipeline (SurfaceProgram::Pipeline).
	foundation::Expected<device::PipelineId, PbrStatus> Pipeline(
	    const PbrClaim &claim, const shaderlib::DebugSpecialization &debug = {} ) const
	{
		return Program().Pipeline( claim.Variant(), debug );
	}
	// The claim as a MaterialPrograms request. 'textures' names base, bump
	// (the normal map), MRAO and emission; stage base and emission as sRGB,
	// MRAO and the normal map as linear data. A slot the claim does not read
	// takes the neutral texture. The frame group names the split-sum table
	// (FrameGroup with a texture holding SplitSumTable()); the draw group
	// holds the draw's model lighting (LightingGroup).
	foundation::Expected<ProgramRequest, PbrStatus> Request( const PbrClaim &claim,
	    SurfaceTextures textures, const device::SamplerDesc &sampler = {} ) const;
};

} // namespace render::material

#endif // RENDER_MATERIAL_PBR_FAMILY_H
