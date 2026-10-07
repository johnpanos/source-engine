//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `vertexlit` material family (RFC 0016 K4): its claim on a
//			material and the packing of a parameter block into the surface
//			program's constants. The family's draws are the vertexlit point
//			of the one surface program (surface_program.h, RFC 0016 K11:
//			kSurfaceVertexLit on the model vertex), which owns the bind groups
//			and pipelines.
//
//			The family claims this subset of VertexLitGeneric: $basetexture,
//			$color, $alpha, $alphatest with $alphatestreference,
//			$translucent and $halflambert. ClaimVertexLit names the first
//			parameter outside the subset that a material sets away from its
//			default (bump map, env map, detail, self-illumination, $phong,
//			rim light, light warp, $additive, a texture transform, ...); such
//			a material is not the family's and stays on its legacy port.
//			$model, $nofog and $nocull are accepted: they select vertex
//			formats, view fog and cull state, which the caller owns.
//			$vertexcolor and $vertexalpha are accepted and their vertex data
//			ignored, as vertexlitgeneric_dx9_helper.cpp ignores it for
//			VertexLitGeneric; $vertexalpha still selects blending, as the
//			port blends.
//
//			The arithmetic is the vertexlit_and_unlit_generic port's lit,
//			non-bumped path (static control flow, common_vs_fxc.h
//			DoLighting): per vertex, each light's color times its cosine term
//			(Lambert, or half-Lambert squared) times its attenuation (distance
//			falloff, the spot cone, 1 for a directional light), plus the
//			ambient cube; per pixel, the base texture times $color (Source's
//			GammaToLinear) times that lighting, and $alpha times the base
//			alpha. In linear light: the base texture is sampled as sRGB. The
//			draw blends when $translucent, $vertexalpha or an $alpha below one
//			says so (EvaluateBlendRequirements), and the alpha-test reference
//			is quantized to eight bits, as the port's alpha test holds it. The
//			draw group holds the draw's model lighting (LightingGroup),
//			because each model instance has its own ambient cube and lights.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_VERTEXLIT_FAMILY_H
#define RENDER_MATERIAL_VERTEXLIT_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/draw_program.h"
#include "render/material/material_programs.h"
#include "render/shaderlib/debug_view.h"
#include "render/material/parameter_block.h"
#include "render/material/model_lighting.h"
#include "render/material/surface_program.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace render::material
{

struct VertexLitClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	bool halfLambert = false;
	// tint: $color (linear) and $alpha; flags.y $alphatest, .z its reference.
	SurfaceConstants constants;

	// The program's point for this claim: kSurfaceVertexLit on the model
	// vertex.
	SurfaceVariant Variant() const
	{
		return { blend, alphaWrite, kSurfaceVertexLit | ( halfLambert ? kSurfaceHalfLambert : 0u ),
		    0, SurfaceVertexLayout::kModel };
	}
};

// Whether the family draws this block's material, and how. The block must be
// of the `vertexlit` family's schema (FamiliesFromMapping).
VertexLitClaim ClaimVertexLit( const ParameterBlock &block );

// The modern mesh point for the supported VertexLitGeneric subset. It uses
// the surface PBR program with a dielectric MRAO constant until a material
// supplies modern values. Unsupported non-neutral VMT terms are refused.
//
// Self-illumination (RFC 0016 surface model, emission term) is visible
// emission only. $selfillum selects the emitting region by base alpha or
// $selfillummask; $selfillumtint (linear) colors it. $selfillumfresnel with
// $selfillumfresnelminmaxexp [min max exp] weights the region by the vertex
// normal's facing c = ( N.V )^exp: the region covers
// saturate( b + ( 1 - b ) c ) of the surface, b = min / max, at radiance
// max x tint x albedo, so a fully covered texel emits
// tint x albedo x ( min + ( max - min ) c ). VertexLitGeneric and its phong
// (skin) shader both define it so, with base alpha as the only mask. The
// translation publishes no area light: $selfillum gives no scene-unit
// radiance, so lighting the room needs an authored or reviewed source.
struct VertexLitMeshClaim
{
	bool claimed = false;
	std::string reason;
	bool normalMap = false;
	bool ssbump = false;
	bool selfIllum = false;
	bool selfIllumMask = false;
	bool selfIllumFresnel = false;
	bool phongExponentTexture = false;
	bool envmapMask = false;
	bool detail = false;
	bool lightwarp = false;
	bool phongWarp = false;
	std::uint32_t detailMode = 0;
	bool alphaTest = false;
	bool alphaToCoverage = false;
	bool halfLambert = false;
	bool ignoreDepth = false; // $ignorez
	device::BlendMode blend = device::BlendMode::kOpaque;
	SurfaceConstants constants;
	SurfaceVariant Variant() const
	{
		SurfaceVariant variant;
		variant.ignoreDepth = ignoreDepth;
		variant.terms = kSurfacePbr |
		                ( normalMap ? ( ssbump ? kSurfaceSsbump : kSurfaceBump ) : 0u ) |
		                ( selfIllum ? kSurfaceSelfIllum : 0u ) |
		                ( selfIllumMask ? kSurfaceSelfIllumMask : 0u ) |
		                ( phongExponentTexture ? kSurfacePhongExponentTexture : 0u ) |
		                ( detail ? kSurfaceDetail : 0u );
		variant.detailMode = detailMode;
		variant.treeSwayMode = std::uint32_t( constants.treeWind[3] );
		variant.alphaToCoverage = alphaToCoverage;
		variant.blend = blend;
		variant.alphaWrite = blend == device::BlendMode::kOpaque && !alphaTest;
		variant.layout = SurfaceVertexLayout::kWorld;
		return variant;
	}
};
VertexLitMeshClaim ClaimVertexLitMesh( const ParameterBlock &block );

// Teeth (teeth.cpp, teeth_vs20/ps2x without a bump map): the base texture
// under Source's model lighting, darkened by $illumfactor x saturate( N .
// $forward ), which studiorender sets per draw from the mouth's flex and
// bone (R_MouthSetupVertexShader); unset, the shader system leaves both at
// zero. It draws on VertexLitGeneric's mesh point with that factor
// (SurfaceConstants::teeth); $color and $alpha are not read. $intro (the
// episode intro warp) and an authored $bumpmap (teeth_bump's Phong) are
// refused by name.
struct TeethClaim
{
	bool claimed = false;
	std::string reason;
	float forward[3] = {};
	float illum = 0.0f;
};
TeethClaim ClaimTeeth( const ParameterBlock &block );

// Eyes (eyes_dx8_dx9_helper.cpp, Eyes_vs20/eyes_ps2x): the sclera with the
// iris lerped over it by iris alpha, the iris planar-projected from the
// world position by $irisu and $irisv, lit by Source's model lighting with
// the eyeball's normal (position less $eyeorigin, less half its $eyeup
// component), plus the glint projected by $glintu and $glintv and damped by
// the ambient cube's luminance. studiorender sets the vectors and the glint
// render target per draw (SetEyeMaterialVars, R_StudioEyeballGlint). It
// draws on VertexLitGeneric's mesh point (SurfaceConstants::eyes). $dilation
// is read by no DirectX 9 eye shader (its code is commented out); $intro is
// refused by name.
struct EyesClaim
{
	bool claimed = false;
	std::string reason;
	float origin[3] = {};
	float up[3] = {};
	float irisU[4] = {};
	float irisV[4] = {};
	float glintU[4] = {};
	float glintV[4] = {};
	bool glint = false; // $glint bound
};
EyesClaim ClaimEyes( const ParameterBlock &block );

using VertexLitStatus = SurfaceStatus;

class VertexLitFamily : public SurfaceFamily
{
public:
	using SurfaceFamily::SurfaceFamily;
	static foundation::Expected<std::unique_ptr<VertexLitFamily>, VertexLitStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat )
	{
		return CreateSurfaceFamily<VertexLitFamily>( device, colorFormat, depthFormat );
	}

	// The pipeline for a claim (created on first use).
	foundation::Expected<device::PipelineId, VertexLitStatus> Pipeline(
	    const VertexLitClaim &claim, const shaderlib::DebugSpecialization &debug = {} ) const
	{
		return Program().Pipeline( claim.Variant(), debug );
	}
	// The claim as a MaterialPrograms request with 'baseTexture', a
	// TextureCache name staged as an sRGB format. The draws bind their model
	// lighting as a draw group (LightingGroup) and a frame group.
	foundation::Expected<ProgramRequest, VertexLitStatus> Request( const VertexLitClaim &claim,
	    std::string baseTexture, const device::SamplerDesc &sampler = {} ) const
	{
		SurfaceTextures textures;
		textures.base = std::move( baseTexture );
		return Program().Request( claim.Variant(), claim.constants, textures, sampler );
	}
};

} // namespace render::material

#endif // RENDER_MATERIAL_VERTEXLIT_FAMILY_H
