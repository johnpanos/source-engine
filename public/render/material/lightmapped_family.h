//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `lightmapped` material family (RFC 0016 K4, K5's surface
//			model): its claim on a material and the packing of a parameter
//			block into the surface program's constants. The family's draws
//			are points of the one surface program (surface_program.h, RFC
//			0016 K11), which owns the bind groups and pipelines.
//
//			The family is the lit surface term. It claims LightmappedGeneric's
//			and WorldVertexTransition's base texture, $color, $alpha,
//			$vertexcolor, $vertexalpha, $alphatest with $alphatestreference,
//			$translucent, and these terms, each a specialization constant
//			(clause D20), so a term at its neutral value costs nothing:
//			- bump: $bumpmap with the three bumped lightmap pages (RNM),
//			  $ssbump's basis weights, $nodiffusebumplighting (the flat page
//			  under a bump map);
//			- env map: $envmap with $envmapmask, $basealphaenvmapmask or
//			  $normalmapalphaenvmapmask, $envmaptint, $envmapcontrast,
//			  $envmapsaturation and $fresnelreflection, as the port's fast
//			  and slow paths use them;
//			- detail: $detail with $detailscale, $detailtint,
//			  $detailblendfactor and the TextureCombine modes the port's
//			  combos allow (0 to 4 and 7 to 9 without a bump map, 0 and 1
//			  with one);
//			- self-illumination: $selfillum with $selfillumtint;
//			- Portal 2's $ssbumpmathfix and $envmaplightscale (with
//			  $envmaplightscaleminmax).
//			ClaimLightmapped names the first parameter outside these that a
//			material sets away from its default (a second base texture, a
//			texture transform, $additive, ...); such a material is not the
//			family's. $model, $nofog and $nocull are accepted: they select
//			vertex formats, view fog and cull state, which the caller owns.
//
//			The arithmetic is the lightmappedgeneric port's, in linear light.
//			The tint is $color times the lightmap scale (2 in gamma space,
//			2^2.2 linear), not gamma converted, as the port leaves it. The
//			bump and env map terms read the world vertex (normal, tangents,
//			the bumped pages' offset); the flat vertex draws the rest. The
//			draw group holds the draw's lightmap page, because surfaces of
//			one material sit on different pages.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H
#define RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/material/parameter_block.h"
#include "render/material/surface_program.h"
#include "render/shaderlib/debug_view.h"

#include <cstdint>
#include <span>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace render::material
{

struct LightmappedClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	bool alphaToCoverage = false; // only opaque cutouts on multisampled targets
	std::uint32_t terms = 0;      // kSurface* bits
	std::uint32_t detailMode = 0; // $detailblendmode, with kSurfaceDetail
	// WorldVertexTransition's second layer (SurfaceVariant::blendTexture2):
	// the caller binds $basetexture2 as the emission texture, $bumpmap2 as
	// MRAO and $blendmodulatetexture as the env map mask.
	bool blendTexture2 = false;
	SurfaceConstants constants;

	// The program's point for this claim on a vertex layout.
	SurfaceVariant Variant( SurfaceVertexLayout layout ) const
	{
		SurfaceVariant variant{ blend, alphaWrite, terms, detailMode, layout };
		variant.alphaToCoverage = alphaToCoverage;
		variant.blendTexture2 = blendTexture2;
		return variant;
	}
};

// Whether the family draws this block's material, and how. The block must be
// of the `lightmapped` family's schema (FamiliesFromMapping).
LightmappedClaim ClaimLightmapped( const ParameterBlock &block );

// kInvalidRequest: the claim's terms read the normal, and the layout is flat.
using LightmappedStatus = SurfaceStatus;

class LightmappedFamily : public SurfaceFamily
{
public:
	using SurfaceFamily::SurfaceFamily;
	// fragmentModule: a replacement fragment program (SPIR-V words) for the
	// debug suites' seeded programs; empty for the program's own.
	static foundation::Expected<std::unique_ptr<LightmappedFamily>, LightmappedStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1, std::span<const std::uint32_t> fragmentModule = {},
	    std::span<const std::uint32_t> shadowFragmentModule = {} )
	{
		return CreateSurfaceFamily<LightmappedFamily>(
		    device, colorFormat, depthFormat, sampleCount, fragmentModule, shadowFragmentModule );
	}

	// The pipeline for a claim on a vertex layout (SurfaceProgram::Pipeline).
	foundation::Expected<device::PipelineId, LightmappedStatus> Pipeline(
	    const LightmappedClaim &claim, SurfaceVertexLayout layout = SurfaceVertexLayout::kFlat,
	    const shaderlib::DebugSpecialization &debug = {} ) const
	{
		return Program().Pipeline( claim.Variant( layout ), debug );
	}
	// The claim as a MaterialPrograms request (SurfaceProgram::Request): the
	// base texture at 1, the env map at 3, its mask at 5, the bump map at 7
	// and the detail texture at 9, each with its sampler after it; an absent
	// one named empty. Each draw's lightmap page fills the draw group
	// (LightmapGroup).
	foundation::Expected<ProgramRequest, LightmappedStatus> Request( const LightmappedClaim &claim,
	    const SurfaceTextures &textures, SurfaceVertexLayout layout,
	    const device::SamplerDesc &sampler = {} ) const
	{
		return Program().Request( claim.Variant( layout ), claim.constants, textures, sampler );
	}
	// The flat vertex and the base texture alone.
	foundation::Expected<ProgramRequest, LightmappedStatus> Request( const LightmappedClaim &claim,
	    std::string baseTexture, const device::SamplerDesc &sampler = {} ) const;
	// A draw group for a lightmap page ('page', a TextureCache name staged as
	// sRGB), with neutral model lighting.
	GroupRequest LightmapGroup( std::string page, const device::SamplerDesc &sampler = {} ) const
	{
		return Program().DrawGroup( std::move( page ), {}, sampler );
	}
};

} // namespace render::material

#endif // RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H
