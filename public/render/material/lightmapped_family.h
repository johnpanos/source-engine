//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `lightmapped` material family's program (RFC 0016 K4, K5's
//			surface model): its claim on a material, the packing of a
//			parameter block into the family's shader constants, its bind
//			groups and its pipelines on render.device.v2.
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
//			bump and env map terms read the surface vertex (normal, tangents,
//			the bumped pages' offset); the flat vertex draws the rest.
//
//			Bind groups: the material group (role kMaterial) holds the
//			constants and the base, env map (a cube), env map mask, bump map
//			and detail textures with their samplers (an absent input takes a
//			neutral texture); the draw group (role kDraw) holds the draw's
//			lightmap page and its sampler, because surfaces of one material
//			sit on different pages. The draw constants carry the draw's
//			world-to-clip matrix.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H
#define RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/material/parameter_block.h"
#include "render/shaderlib/debug_view.h"

#include <cstdint>
#include <span>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

namespace render::material
{

// The lightmap scale in linear light: the port's 2.0 overbright, gamma to
// linear (pow 2.2).
inline constexpr float kLightmapScaleLinear = 4.5947938f;

// The family's shader constants (std140, the Material block of lightmapped.frag).
struct LightmappedConstants
{
	float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // $color, $alpha
	// vertexcolor, alphatest, reference, 1 when lighting is one (an unlit
	// material drawn as this term's degenerate case)
	float flags[4] = {};
	// x: 1 when the material is fully opaque (no blend, no alpha test), where
	// height fog writes its factor to the output alpha (the port's
	// WRITEWATERFOGTODESTALPHA); set by Request from the claim. y: 1 when the
	// vertex color is gamma-encoded and decoded per vertex (pow 2.2, as
	// UnlitGeneric's port reads it); LightmappedGeneric's is used unconverted.
	// z: the ssbump weights' scale (0.57735 with $ssbumpmathfix, else 1).
	float state[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float envTint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };       // $envmaptint, $fresnelreflection
	float envContrast[4] = { 0.0f, 0.0f, 0.0f, 0.0f };   // in effect; a: 1 - $fresnelreflection
	float envSaturation[4] = { 1.0f, 1.0f, 1.0f, 0.0f }; // in effect
	float selfIllumTint[4] = { 1.0f, 1.0f, 1.0f, 0.0f }; // in effect
	float detailTint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };    // $detailtint, $detailblendfactor
	float detailScale[4] = { 4.0f, 4.0f, 0.0f, 0.0f };   // $detailscale
	// Portal 2's $envmaplightscale: the cube map darkened where the diffuse
	// light is dark. x: the min of $envmaplightscaleminmax, y: min + max (as
	// the Portal 2 helper packs them), z: $envmaplightscale (0 off).
	float envLightScale[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
};
static_assert( sizeof( LightmappedConstants ) == 160 );

// The frame's terms (std140, the Frame block of lightmapped.frag): one
// lightmap term whose scale depends on how the pages encode light, and the
// output's linear scale. LDR pages hold gamma light at half overbright
// (scale 2^2.2 after sRGB decode); integer-HDR pages hold linear light / 16
// (scale 16); the output scale is the frame's linear tone-mapping scale
// (1 without HDR). The defaults are LDR's, where the family's pixel cases sit.
//
// The view's fog is a frame term too (legacy::CorePassFog): its color (linear,
// tone-scaled in integer HDR) with its type in w (-1 none, 0 range, 1 height),
// its parameters, and the eye's world z. The default is no fog.
struct LightmappedFrame
{
	// lightmap scale, output scale, 1 to encode sRGB in the shader (a target
	// without an sRGB view), 1 when specular shows
	float light[4] = { kLightmapScaleLinear, 1.0f, 0.0f, 1.0f };
	float fogColor[4] = { 0.0f, 0.0f, 0.0f, -1.0f };
	float fogParams[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	// x: the eye's world z; y: 1 when the running game's shaders scale every
	// ssbump's basis weights by 1/sqrt(3) (Portal 2's do; this SDK's only
	// with $ssbumpmathfix: the backend's SsbumpBasisNormalized owns it).
	float fogMisc[4] = {};
	// xyz: the eye's world position (the env map's reflection); w:
	// ENV_MAP_SCALE, 16 in integer HDR (cube maps hold light / 16), else 1.
	float eye[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};
static_assert( sizeof( LightmappedFrame ) == 80 );

// The draw constants (lightmapped.vert): row-major with column vectors.
struct LightmappedDrawConstants
{
	float toClip[16] = {};
};
static_assert( sizeof( LightmappedDrawConstants ) == 64 );

// The vertex the family draws: position, base and lightmap coordinates, and
// color as UNORM8x4 (RGBA).
struct LightmappedVertex
{
	float position[3] = {};
	float uv[2] = {};
	float lightmapUv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};
static_assert( sizeof( LightmappedVertex ) == 32 );

// The surface vertex: the flat vertex, then the world normal, tangents S and
// T, and the bumped lightmap pages' offset in the page (lightmappedgeneric_vs20's
// TEXCOORD2.x: the flat page's width, as a coordinate), read with tangent T as
// one four-component attribute.
struct LightmappedSurfaceVertex
{
	float position[3] = {};
	float uv[2] = {};
	float lightmapUv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
	float normal[3] = { 0.0f, 0.0f, 1.0f };
	float tangentS[3] = { 1.0f, 0.0f, 0.0f };
	float tangentT[3] = { 0.0f, 1.0f, 0.0f };
	float lightmapOffset = 0.0f;
};
static_assert( sizeof( LightmappedSurfaceVertex ) == 72 );

enum class LightmappedVertexLayout : std::uint8_t
{
	kFlat,   // LightmappedVertex: the terms that read no normal
	kSurface // LightmappedSurfaceVertex: every term
};

// The terms a claim draws (lightmapped.frag's kTerms bits, the port's static
// combo bits where they exist).
inline constexpr std::uint32_t kLightmappedDetail = 2;
inline constexpr std::uint32_t kLightmappedBump = 4;
inline constexpr std::uint32_t kLightmappedSsbump = 8;
inline constexpr std::uint32_t kLightmappedEnvmap = 32;
inline constexpr std::uint32_t kLightmappedEnvmapMask = 64;
inline constexpr std::uint32_t kLightmappedBaseAlphaEnvmapMask = 128;
inline constexpr std::uint32_t kLightmappedSelfIllum = 256;
inline constexpr std::uint32_t kLightmappedNormalMapAlphaEnvmapMask = 512;
inline constexpr std::uint32_t kLightmappedDiffuseBump = 1024;
// The terms that read the surface vertex.
inline constexpr std::uint32_t kLightmappedSurfaceTerms =
    kLightmappedBump | kLightmappedSsbump | kLightmappedEnvmap;

// The textures a claim reads, by TextureCache name (empty when absent).
struct LightmappedTextures
{
	std::string base;
	std::string envmap; // a cube map
	std::string envmapMask;
	std::string bump;
	std::string detail;
};

struct LightmappedClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	std::uint32_t terms = 0;      // kLightmapped* bits
	std::uint32_t detailMode = 0; // $detailblendmode, with kLightmappedDetail
	LightmappedConstants constants;
};

// Whether the family draws this block's material, and how. The block must be
// of the `lightmapped` family's schema (FamiliesFromMapping).
LightmappedClaim ClaimLightmapped( const ParameterBlock &block );

enum class LightmappedStatus : std::uint8_t
{
	kDevice = 1,    // a layout or pipeline was refused
	kInvalidRequest // the claim's terms read the surface vertex, and the layout is flat
};

class LightmappedFamily
{
public:
	// fragmentModule: a replacement fragment program (SPIR-V words) for the
	// debug suites' seeded programs; empty for the family's own.
	static foundation::Expected<std::unique_ptr<LightmappedFamily>, LightmappedStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1, std::span<const std::uint32_t> fragmentModule = {} );
	~LightmappedFamily();
	LightmappedFamily( const LightmappedFamily & ) = delete;
	LightmappedFamily &operator=( const LightmappedFamily & ) = delete;

	device::BindGroupLayoutId MaterialLayout() const { return m_MaterialLayout; }
	// The draw group's layout: binding 0 the lightmap page, 1 its sampler.
	device::BindGroupLayoutId DrawLayout() const { return m_DrawLayout; }
	// The frame group's layout: binding 0 the LightmappedFrame block.
	device::BindGroupLayoutId FrameLayout() const { return m_FrameLayout; }
	// The pipeline for a claim's blend mode, alpha write and terms, on a
	// vertex layout (created on first use). kInvalidRequest when the claim's
	// terms read the surface vertex and the layout is flat.
	// With a debug specialization (RFC 0014) that is not neutral, the same
	// program with the debug constants.
	foundation::Expected<device::PipelineId, LightmappedStatus> Pipeline(
	    const LightmappedClaim &claim,
	    LightmappedVertexLayout layout = LightmappedVertexLayout::kFlat,
	    const shaderlib::DebugSpecialization &debug = {} );
	// The debug variant of a pipeline this family made (Pipeline with a
	// neutral specialization); kInvalidRequest when the family did not make
	// it. A neutral specialization returns the pipeline itself.
	foundation::Expected<device::PipelineId, LightmappedStatus> DebugPipeline(
	    device::PipelineId shipped, const shaderlib::DebugSpecialization &debug );
	// The claim as a MaterialPrograms request: the pipeline, the material
	// group (constants at binding 0; the base texture at 1, the env map at 3,
	// its mask at 5, the bump map at 7 and the detail texture at 9, each with
	// its sampler after it; an absent one named empty) and the draw layout,
	// which each draw's lightmap page fills (LightmapGroup).
	foundation::Expected<ProgramRequest, LightmappedStatus> Request( const LightmappedClaim &claim,
	    const LightmappedTextures &textures, LightmappedVertexLayout layout,
	    const device::SamplerDesc &sampler = {} );
	// The flat vertex and the base texture alone.
	foundation::Expected<ProgramRequest, LightmappedStatus> Request( const LightmappedClaim &claim,
	    std::string baseTexture, const device::SamplerDesc &sampler = {} );
	// A draw group for a lightmap page ('page', a TextureCache name staged as
	// sRGB, at binding 0 with its sampler at 1), for DrawGroups.
	GroupRequest LightmapGroup( std::string page, const device::SamplerDesc &sampler = {} ) const;
	// The frame group for these terms (role kFrame).
	GroupRequest FrameGroup( const LightmappedFrame &frame ) const;

private:
	explicit LightmappedFamily( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	device::Format m_DepthFormat = device::Format::kUnknown;
	std::uint32_t m_SampleCount = 1;
	device::BindGroupLayoutId m_MaterialLayout;
	device::BindGroupLayoutId m_DrawLayout;
	device::BindGroupLayoutId m_FrameLayout;
	std::span<const std::uint32_t> m_FragmentModule;
	// By blend, alpha write, terms, detail mode, vertex layout and debug
	// specialization.
	using PipelineKey = std::tuple<device::BlendMode, bool, std::uint32_t, std::uint32_t,
	    LightmappedVertexLayout, shaderlib::DebugSpecialization>;
	std::map<PipelineKey, device::PipelineId> m_Pipelines;
	// The claim behind each shipped (neutral) pipeline, for DebugPipeline.
	std::map<std::uint64_t, std::pair<LightmappedClaim, LightmappedVertexLayout>> m_Shipped;
};

} // namespace render::material

#endif // RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H
