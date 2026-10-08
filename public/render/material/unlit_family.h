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
#include "render/sprite_card.h"

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
	bool energy = false; // SolidEnergy (energy_family.h)
	bool wireframe = false; // Wireframe: triangle edges as lines (device clause D38)
	bool ignoreDepth = false;
	bool depthBlend = false; // requires the view's copied scene depth in alpha
	bool baseSrgb = true;
	bool fogToBlack = false;
	// The parameter whose texture is the base (empty: $hdrbasetexture when
	// bound, else $basetexture).
	std::string baseParameter;
	// tint: $color and $alpha; flags.x $vertexcolor, .y $alphatest, .z its
	// reference; state.y 1 (gamma vertex colors); state.w $vertexalpha.
	SurfaceConstants constants;

	// The program's point for this claim on a vertex layout.
	SurfaceVariant Variant( SurfaceVertexLayout layout = SurfaceVertexLayout::kFlat ) const
	{
		SurfaceVariant variant{ blend, alphaWrite, kSurfaceUnlit, 0, layout, ignoreDepth };
		variant.cable = cable;
		variant.energy = energy;
		variant.wireframe = wireframe;
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
// SpriteCard's particle cards (RFC 0016 K8): the unlit point with the card
// terms (frame blend, $overbrightfactor, $addself, $mod2x, depth feathering)
// and spritecard.cpp's blend state. Its pixel combos beyond those are refused
// by name.
UnlitClaim ClaimSpriteCard( const ParameterBlock &block );
// The card constants render.sprite-card.v1 expands a claimed SpriteCard's
// records with: size limits, far fade, kind, orientation and frame blend. The
// caller supplies the matrices and the mesh's format facts.
sprite_card::Frame SpriteCardTerms( const ParameterBlock &block );
// DecalModulate: undecoded multiplicative texture, alpha > 0, fog to neutral.
// The caller supplies decal depth bias and captured culling; destination alpha stays intact.
UnlitClaim ClaimDecalModulate( const ParameterBlock &block );
// Shadow (shadow.cpp, shadow_ps2x): a render-to-texture blob shadow decal on
// the decal-modulate point: the mean alpha of five samples of the shadow page
// (the base and +-1 texel on each diagonal) less the vertex alpha, lerping
// white toward $color (linear), fogged to white, multiplied into the frame
// (ZERO, SRC_COLOR, drawn as the 2x blend of half the factor).
UnlitClaim ClaimBlobShadow( const ParameterBlock &block );
// ShadowBuild (shadowbuild_dx9.cpp, shadowbuildtexture_ps2x): white with the
// caster's base alpha times its alpha, added (ONE, ONE) into the shadow page
// without depth; without a base texture the coverage is 1.
UnlitClaim ClaimShadowBuild( const ParameterBlock &block );
// Modulate (modulate_dx9.cpp, modulate_ps2x): the decal-modulate point with
// saturate( base x $color/$alpha x vertex color ), its color lerped from the
// neutral 0.5 by its alpha, fog to the neutral grey. $mod2x blends
// DST_COLOR, SRC_COLOR; without it DST_COLOR, ZERO, drawn as the same 2x
// blend of half the factor. $writez is the captured draw state's. A
// material without $basetexture samples an unbound sampler and is refused,
// as is the cloak pass.
UnlitClaim ClaimModulate( const ParameterBlock &block );
// Model extension: an authored env map is resolved from the stage's native
// reflection probes and shaded beside the emissive base in the PBR point.
UnlitClaim ClaimUnlitMesh( const ParameterBlock &block );
// The Sky shader's faces (Sky_HDR_DX9, Sky_DX9; render.pass.sky): the base
// as its encoding gives it ($hdrcompressedtexture RGBS times 8, else
// $basetexture), times $color unconverted, depth ignored. The three-texture
// encoding ($hdrcompressedtexture0) and a lone $hdrbasetexture, whose
// conversion depends on the texture's format, are refused by name.
UnlitClaim ClaimSky( const ParameterBlock &block, bool hdr );
// The Black shader (black.cpp): an opaque surface of no parameters drawn
// black and fogged to the fog color, the unlit point with a zero tint.
UnlitClaim ClaimBlack( const ParameterBlock &block );
// Wireframe (wireframe_dx9.cpp; Eyeball, a dead shader, falls back to it):
// UnlitGeneric with fog off, drawn as its triangles' edges (the device's line
// fill, Capability::kFillModeLines, which the resolver requires).
UnlitClaim ClaimWireframe( const ParameterBlock &block );

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
