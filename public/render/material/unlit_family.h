//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `unlit` material family (RFC 0016 K4): its claim on a
//			material and the packing of a parameter block into the surface
//			program's constants. The family's draws are the unlit point of
//			the one surface program (surface_program.h, RFC 0016 K11:
//			kSurfaceUnlit), which owns the bind groups and pipelines.
//
//			The family claims this subset of UnlitGeneric: $basetexture,
//			$color, $alpha, $vertexcolor, $vertexalpha, $alphatest with
//			$alphatestreference, and $translucent or $additive (not both:
//			the port blends that pair src-alpha/one, which the device port
//			has no mode for). ClaimUnlit names the first parameter outside
//			the subset that a material sets away from its default (detail,
//			env map, ...); such a material is not the family's and stays on
//			its legacy port. $model, $nofog and $nocull are accepted: they
//			select vertex formats, view fog and cull state, which the caller
//			owns.
//
//			$depthblend with a positive $depthblendscale fades unlit particles
//			against the view's ordered depth-alpha copy (SurfaceScreenInputs).
//			The view range must match the copy's producer. WorldPass refuses a
//			missing input before claiming a slot; a lost import fails that slot.
//
//			The arithmetic is the vertexlit_and_unlit_generic port's without
//			lighting, in linear light: the base texture times $color (Source's
//			GammaToLinear), times the vertex color (decoded per vertex) with
//			$vertexcolor; $alpha times the base alpha, times the vertex alpha
//			with $vertexalpha. The base texture is sampled as sRGB.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_UNLIT_FAMILY_H
#define RENDER_MATERIAL_UNLIT_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/shaderlib/debug_view.h"
#include "render/material/parameter_block.h"
#include "render/material/surface_program.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace render::material
{

struct UnlitClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	bool nativeProbe = false; // mesh-only authored $envmap resolved through RPRB
	bool twoTexture = false;
	bool decalModulate = false;
	bool cable = false; // expanded ribbon: normal UV0, color UV1, linear vertex lighting
	bool ignoreDepth = false;
	bool depthBlend = false; // requires the view's copied scene depth in alpha
	bool baseSrgb = true;
	bool fogToBlack = false;
	// tint: $color and $alpha; flags.x $vertexcolor, .y $alphatest, .z its
	// reference; state.y 1 (gamma vertex colors); state.w $vertexalpha.
	SurfaceConstants constants;

	// The program's point for this claim on a vertex layout.
	SurfaceVariant Variant( SurfaceVertexLayout layout = SurfaceVertexLayout::kFlat ) const
	{
		SurfaceVariant variant{ blend, alphaWrite, kSurfaceUnlit, 0, layout, ignoreDepth };
		variant.cable = cable;
		variant.decalModulate = decalModulate;
		return variant;
	}
};

// Whether the family draws this block's material, and how. The block must be
// of the `unlit` family's schema (FamiliesFromMapping).
UnlitClaim ClaimUnlit( const ParameterBlock &block );
// Already-expanded Sprite_DX9 quads. The frontend owns orientation and glow
// visibility; this point owns render-mode blending, color and depth behavior.
UnlitClaim ClaimSprite( const ParameterBlock &block );
// DecalModulate: undecoded multiplicative texture, alpha > 0, fog to neutral.
// The caller supplies decal depth bias and captured culling; destination alpha stays intact.
UnlitClaim ClaimDecalModulate( const ParameterBlock &block );
// Model extension: an authored env map is resolved from the stage's native
// reflection probes and shaded beside the emissive base in the PBR point.
UnlitClaim ClaimUnlitMesh( const ParameterBlock &block );

using UnlitStatus = SurfaceStatus;

class UnlitFamily : public SurfaceFamily
{
public:
	using SurfaceFamily::SurfaceFamily;
	static foundation::Expected<std::unique_ptr<UnlitFamily>, UnlitStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1 )
	{
		return CreateSurfaceFamily<UnlitFamily>( device, colorFormat, depthFormat, sampleCount );
	}

	// The pipeline for a claim on a vertex layout (created on first use).
	foundation::Expected<device::PipelineId, UnlitStatus> Pipeline( const UnlitClaim &claim,
	    SurfaceVertexLayout layout = SurfaceVertexLayout::kFlat,
	    const shaderlib::DebugSpecialization &debug = {} ) const
	{
		return Program().Pipeline( claim.Variant( layout ), debug );
	}
	// The claim as a MaterialPrograms request with 'baseTexture', a
	// TextureCache name staged as an sRGB format. The draws bind a draw group
	// (a neutral one: DrawGroup(), which reads no page) and a frame group.
	foundation::Expected<ProgramRequest, UnlitStatus> Request( const UnlitClaim &claim,
	    std::string baseTexture, SurfaceVertexLayout layout = SurfaceVertexLayout::kFlat,
	    const device::SamplerDesc &sampler = {} ) const
	{
		SurfaceTextures textures;
		textures.base = std::move( baseTexture );
		textures.baseSrgb = claim.baseSrgb;
		return Program().Request( claim.Variant( layout ), claim.constants, textures, sampler );
	}
	// The draw group an unlit draw binds: no page, neutral lighting.
	GroupRequest DrawGroup() const { return Program().DrawGroup( {} ); }
};

} // namespace render::material

#endif // RENDER_MATERIAL_UNLIT_FAMILY_H
