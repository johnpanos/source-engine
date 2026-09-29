//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `lightmapped` material family's program (RFC 0016 K4): its
//			claim on a material, the packing of a parameter block into the
//			family's shader constants, its bind groups and its pipelines on
//			render.device.v2.
//
//			The family claims this subset of LightmappedGeneric and
//			WorldVertexTransition: $basetexture, $color, $alpha, $vertexcolor,
//			$vertexalpha, $alphatest with $alphatestreference, $translucent.
//			ClaimLightmapped names the first parameter outside the subset
//			that a material sets away from its default (bump map, second
//			base texture, detail, env map, $additive, ...); such a material
//			is not the family's and stays on its legacy port. $model, $nofog
//			and $nocull are accepted: they select vertex formats, view fog
//			and cull state, which the caller owns.
//
//			The arithmetic is the lightmappedgeneric port's non-bumped path,
//			in linear light: the base texture and the lightmap are sampled as
//			sRGB and the target is sRGB, as the port draws. The tint is $color
//			times the lightmap scale (2 in gamma space, 2^2.2 linear); $color
//			is not gamma converted, as the port leaves it. Vertex colors are
//			used unconverted. The port's vertex fast path holds for every
//			claimed material (no texture transform, no detail): with
//			$vertexcolor the vertex alpha replaces the modulation alpha, and
//			otherwise $alpha applies twice.
//
//			Bind groups: the material group (role kMaterial) holds the
//			constants, the base texture and its sampler; the draw group (role
//			kDraw) holds the draw's lightmap page and its sampler, because
//			surfaces of one material sit on different pages. The draw
//			constants carry the draw's world-to-clip matrix.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H
#define RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/material/parameter_block.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
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
};
static_assert( sizeof( LightmappedConstants ) == 32 );

// The frame's terms (std140, the Frame block of lightmapped.frag): one
// lightmap term whose scale depends on how the pages encode light, and the
// output's linear scale. LDR pages hold gamma light at half overbright
// (scale 2^2.2 after sRGB decode); integer-HDR pages hold linear light / 16
// (scale 16); the output scale is the frame's linear tone-mapping scale
// (1 without HDR). The defaults are LDR's, where the family's pixel cases sit.
struct LightmappedFrame
{
	// lightmap scale, output scale, 1 to encode sRGB in the shader (a target
	// without an sRGB view), unused
	float light[4] = { kLightmapScaleLinear, 1.0f, 0.0f, 0.0f };
};
static_assert( sizeof( LightmappedFrame ) == 16 );

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

struct LightmappedClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	LightmappedConstants constants;
};

// Whether the family draws this block's material, and how. The block must be
// of the `lightmapped` family's schema (FamiliesFromMapping).
LightmappedClaim ClaimLightmapped( const ParameterBlock &block );

enum class LightmappedStatus : std::uint8_t
{
	kDevice = 1 // a layout or pipeline was refused
};

class LightmappedFamily
{
public:
	static foundation::Expected<std::unique_ptr<LightmappedFamily>, LightmappedStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1 );
	~LightmappedFamily();
	LightmappedFamily( const LightmappedFamily & ) = delete;
	LightmappedFamily &operator=( const LightmappedFamily & ) = delete;

	device::BindGroupLayoutId MaterialLayout() const { return m_MaterialLayout; }
	// The draw group's layout: binding 0 the lightmap page, 1 its sampler.
	device::BindGroupLayoutId DrawLayout() const { return m_DrawLayout; }
	// The frame group's layout: binding 0 the LightmappedFrame block.
	device::BindGroupLayoutId FrameLayout() const { return m_FrameLayout; }
	// The pipeline for a claim's blend mode and alpha write (created on first use).
	foundation::Expected<device::PipelineId, LightmappedStatus> Pipeline(
	    const LightmappedClaim &claim );
	// The claim as a MaterialPrograms request: the pipeline, the material
	// group (constants at binding 0, 'baseTexture', a TextureCache name, at 1
	// with its sampler at 2) and the draw layout, which each draw's lightmap
	// page fills (LightmapGroup). Stage the base texture as sRGB.
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
	std::map<std::pair<device::BlendMode, bool>, device::PipelineId> m_Pipelines;
};

} // namespace render::material

#endif // RENDER_MATERIAL_LIGHTMAPPED_FAMILY_H
