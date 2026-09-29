//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `vertexlit` material family's program (RFC 0016 K4): its
//			claim on a material, the packing of a parameter block into the
//			family's shader constants, its bind groups and its pipelines on
//			render.device.v2.
//
//			The family claims this subset of VertexLitGeneric: $basetexture,
//			$color, $alpha, $alphatest with $alphatestreference,
//			$translucent and $halflambert. ClaimVertexLit names the first
//			parameter outside the subset that a material sets away from its
//			default (bump map, env map, detail, self-illumination, $phong,
//			rim light, light warp, $additive, a texture transform, ...); such
//			a material is not the family's and stays on its legacy port.
//			$model, $nofog and $nocull are accepted: they select vertex
//			formats, view fog and cull state, which the caller owns. $vertexcolor and $vertexalpha
//			are accepted and their vertex data ignored, as
//			vertexlitgeneric_dx9_helper.cpp ignores it for VertexLitGeneric;
//			$vertexalpha still selects blending, as the port blends.
//
//			The arithmetic is the vertexlit_and_unlit_generic port's lit,
//			non-bumped path (static control flow, common_vs_fxc.h
//			DoLighting): per vertex, each light's color times its cosine term
//			(Lambert, or half-Lambert squared) times its attenuation (distance
//			falloff, the spot cone, 1 for a directional light), plus the
//			ambient cube; per pixel, the base texture times $color (Source's
//			GammaToLinear) times that lighting, and $alpha times the base
//			alpha. In linear light: the base texture is sampled as sRGB and
//			the target is sRGB, as the port draws. The draw blends when
//			$translucent, $vertexalpha or an $alpha below one says so
//			(EvaluateBlendRequirements), and the alpha-test reference is
//			quantized to eight bits, as the port's alpha test holds it.
//
//			Bind groups: the material group (role kMaterial) holds the
//			constants, the base texture and its sampler. The draw group
//			(role kDraw) holds the draw's lighting (VertexLitLighting, a
//			uniform buffer), because each model instance has its own ambient
//			cube and lights. The draw constants are the FamilyDrawConstants
//			prefix: object-to-clip and object-to-world; positions and normals
//			are moved into the lights' (world) space by object-to-world.
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

#include <cstdint>
#include <tuple>
#include <optional>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace render::material
{

// The family's shader constants (std140, the Material block of vertexlit.vert
// and vertexlit.frag).
struct VertexLitConstants
{
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // rgb: $color, linear; a: $alpha
	float flags[4] = {};                         // halflambert, alphatest, reference, unused
};
static_assert( sizeof( VertexLitConstants ) == 32 );

// The draw constants (vertexlit.vert): object-to-clip and object-to-world.
using VertexLitDrawConstants = FamilyDrawConstants;

// The vertex the family draws: position, normal and uv0.
struct VertexLitVertex
{
	float position[3] = {};
	float normal[3] = {};
	float uv[2] = {};
};
static_assert( sizeof( VertexLitVertex ) == 32 );

// The draw group's lighting (std140, the Lighting block of vertexlit.vert):
// Source's model lighting as PackSourceModelLighting packs it (model_lighting.h,
// the one owner of that packing): eye (w: the light count), the ambient cube
// (+x, -x, +y, -y, +z, -z) and up to four lights sorted spot, point,
// directional, each as CShaderAPIDx8::SetLight builds cLightInfo. The family
// reads the count, the cube and the lights, in world space.
using VertexLitLighting = ModelLighting;

struct VertexLitClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	device::BlendMode blend = device::BlendMode::kOpaque;
	// Whether the draw writes destination alpha: the port leaves it for
	// translucent and alpha-tested draws (write mask, clause D17).
	bool alphaWrite = true;
	VertexLitConstants constants;
};

// Whether the family draws this block's material, and how. The block must be
// of the `vertexlit` family's schema (FamiliesFromMapping).
VertexLitClaim ClaimVertexLit( const ParameterBlock &block );

enum class VertexLitStatus : std::uint8_t
{
	kDevice = 1 // a layout or pipeline was refused
};

class VertexLitFamily
{
public:
	static foundation::Expected<std::unique_ptr<VertexLitFamily>, VertexLitStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat );
	~VertexLitFamily();
	VertexLitFamily( const VertexLitFamily & ) = delete;
	VertexLitFamily &operator=( const VertexLitFamily & ) = delete;

	// Binding 0 the constants, 1 the base texture, 2 its sampler.
	device::BindGroupLayoutId MaterialLayout() const { return m_MaterialLayout; }
	// The draw group's layout: binding 0 the lighting (VertexLitLighting).
	device::BindGroupLayoutId DrawLayout() const { return m_DrawLayout; }
	// The pipeline for a claim's blend mode and alpha write (created on first use).
	// With a debug specialization (RFC 0014) that is not neutral, the same
	// program with the debug constants.
	foundation::Expected<device::PipelineId, VertexLitStatus> Pipeline(
	    const VertexLitClaim &claim, const shaderlib::DebugSpecialization &debug = {} );
	// The debug variant of a pipeline this family made; the pipeline itself
	// for a neutral specialization; nullopt when the family did not make it.
	std::optional<device::PipelineId> DebugPipeline(
	    device::PipelineId shipped, const shaderlib::DebugSpecialization &debug );
	// The claim as a MaterialPrograms request: the pipeline, the material
	// layout with the packed constants (binding 0) and 'baseTexture', a
	// TextureCache name, at binding 1 with its sampler at binding 2, and the
	// draw layout the program reads. Stage the base texture as an sRGB format,
	// as the family samples it.
	foundation::Expected<ProgramRequest, VertexLitStatus> Request( const VertexLitClaim &claim,
	    std::string baseTexture, const device::SamplerDesc &sampler = {} );
	// A draw's lighting as a DrawGroups request: the draw layout and the
	// packed lighting at binding 0. A scene instance names the group's id.
	GroupRequest LightingGroup( const VertexLitLighting &lighting ) const;

private:
	explicit VertexLitFamily( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	device::Format m_DepthFormat = device::Format::kUnknown;
	device::BindGroupLayoutId m_MaterialLayout;
	device::BindGroupLayoutId m_DrawLayout;
	std::map<std::tuple<device::BlendMode, bool, shaderlib::DebugSpecialization>,
	    device::PipelineId>
	    m_Pipelines;
	std::map<std::uint64_t, VertexLitClaim> m_Shipped; // the claim behind each shipped pipeline
};

} // namespace render::material

#endif // RENDER_MATERIAL_VERTEXLIT_FAMILY_H
