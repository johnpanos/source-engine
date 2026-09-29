//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `unlit` material family's program (RFC 0016 K4): its claim on a
//			material, the packing of a parameter block into the family's
//			shader constants, its material bind group and its pipelines on
//			render.device.v2.
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
//			The arithmetic is the vertexlit_and_unlit_generic port's without
//			lighting, in linear light: the base texture is sampled as sRGB
//			and the target is sRGB, as the port draws (tests compare the two
//			within the family's tolerance). The draw constants carry the
//			draw's world-to-clip matrix; the material bind group (role
//			kMaterial) holds the constants, the base texture and its sampler.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_UNLIT_FAMILY_H
#define RENDER_MATERIAL_UNLIT_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/shaderlib/debug_view.h"
#include "render/material/parameter_block.h"

#include <cstdint>
#include <tuple>
#include <optional>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace render::material
{

// The family's shader constants (std140, the Material block of unlit.frag).
struct UnlitConstants
{
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // rgb: $color, a: $alpha
	float flags[4] = {};                         // vertexcolor, vertexalpha, alphatest, reference
};
static_assert( sizeof( UnlitConstants ) == 32 );

// The draw constants (unlit.vert): row-major with column vectors.
struct UnlitDrawConstants
{
	float toClip[16] = {};
};
static_assert( sizeof( UnlitDrawConstants ) == 64 );

// The vertex the family draws: position, uv0, and color as UNORM8x4 (RGBA).
struct UnlitVertex
{
	float position[3] = {};
	float uv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};
static_assert( sizeof( UnlitVertex ) == 24 );

struct UnlitClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	UnlitConstants constants;
};

// Whether the family draws this block's material, and how. The block must be
// of the `unlit` family's schema (FamiliesFromMapping).
UnlitClaim ClaimUnlit( const ParameterBlock &block );

enum class UnlitStatus : std::uint8_t
{
	kDevice = 1 // a layout, sampler or pipeline was refused
};

class UnlitFamily
{
public:
	static foundation::Expected<std::unique_ptr<UnlitFamily>, UnlitStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1 );
	~UnlitFamily();
	UnlitFamily( const UnlitFamily & ) = delete;
	UnlitFamily &operator=( const UnlitFamily & ) = delete;

	device::BindGroupLayoutId MaterialLayout() const { return m_MaterialLayout; }
	// The pipeline for a claim's blend mode and alpha write (created on first use).
	// With a debug specialization (RFC 0014) that is not neutral, the same
	// program with the debug constants.
	foundation::Expected<device::PipelineId, UnlitStatus> Pipeline(
	    const UnlitClaim &claim, const shaderlib::DebugSpecialization &debug = {} );
	// The debug variant of a pipeline this family made; the pipeline itself
	// for a neutral specialization; nullopt when the family did not make it.
	std::optional<device::PipelineId> DebugPipeline(
	    device::PipelineId shipped, const shaderlib::DebugSpecialization &debug );
	// The claim as a MaterialPrograms request: the pipeline, the material
	// layout, the packed constants (binding 0) and 'baseTexture', a
	// TextureCache name, at binding 1 with its sampler at binding 2. Stage the
	// base texture as an sRGB format, as the family samples it.
	foundation::Expected<ProgramRequest, UnlitStatus> Request(
	    const UnlitClaim &claim, std::string baseTexture, const device::SamplerDesc &sampler = {} );

private:
	explicit UnlitFamily( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	device::Format m_DepthFormat = device::Format::kUnknown;
	std::uint32_t m_SampleCount = 1;
	device::BindGroupLayoutId m_MaterialLayout;
	std::map<std::tuple<device::BlendMode, bool, shaderlib::DebugSpecialization>,
	    device::PipelineId>
	    m_Pipelines;
	std::map<std::uint64_t, UnlitClaim> m_Shipped; // the claim behind each shipped pipeline
};

} // namespace render::material

#endif // RENDER_MATERIAL_UNLIT_FAMILY_H
