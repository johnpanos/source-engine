//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `pbr` material family's program (RFC 0016 K4): PBRMetalRough
//			(RFC 0007, which owns its semantics and the BRDF) on meshes: its
//			claim on a material, the packing of a parameter block into the
//			family's constants, its bind groups and its pipelines on
//			render.device.v2.
//
//			The family claims this subset of the RFC 0007 schema:
//			$basetexture, $mraotexture, $bumpmap, $emissiontexture with
//			$emissionscale ($fallbackmaterial names another profile's
//			material and is accepted). ClaimPbr names the first parameter
//			outside the subset that a material sets away from its default
//			($envmap, $alphatest, $translucent, glass, clear coat); such a
//			material stays on the native stages until the family claims it.
//
//			The arithmetic is the model port's (model_pbr.frag) without map
//			probes, environment maps, the probe volume or clear coat: the
//			layered BRDF of render/shaders/common/pbr_brdf.glsl under
//			Source's model lighting, with the ambient cube as the specular
//			image light. Base and emission are sampled as sRGB, MRAO and the
//			normal map as linear data; the target is sRGB.
//
//			Bind groups: the frame group (role kFrame) holds the split-sum
//			table and its sampler (PbrSplitSumTable); the view group (role
//			kView) holds the model lighting (PbrModelLighting); the material
//			group (role kMaterial) holds the constants and four texture and
//			sampler pairs. The draw constants are the FamilyDrawConstants
//			prefix: world-to-clip and object-to-world.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_PBR_FAMILY_H
#define RENDER_MATERIAL_PBR_FAMILY_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/parameter_block.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace render::material
{

// The family's material constants (std140, the Material block of pbr.frag).
struct PbrConstants
{
	float flags[4] = {}; // normal map, emission, $emissionscale, unused
};
static_assert( sizeof( PbrConstants ) == 16 );

// The draw constants (pbr.vert), row-major with column vectors.
struct PbrDrawConstants
{
	float toClip[16] = {};
	float world[16] = {};
};
static_assert( sizeof( PbrDrawConstants ) == 128 );

// The vertex the family draws: position, normal, tangent (w: the bitangent's
// sign) and uv0.
struct PbrVertex
{
	float position[3] = {};
	float normal[3] = {};
	float tangent[4] = {};
	float uv[2] = {};
};
static_assert( sizeof( PbrVertex ) == 48 );

// A Source model light, as the legacy frontend describes it (LightDesc_t).
enum class PbrLightType : std::uint8_t
{
	kPoint,
	kDirectional,
	kSpot
};

struct PbrLightDesc
{
	PbrLightType type = PbrLightType::kPoint;
	float color[3] = {};
	float position[3] = {}; // a directional light's too: the vertex term reads it
	float direction[3] = { 0.0f, 0.0f, 1.0f };
	float attenuation[3] = { 1.0f, 0.0f, 0.0f }; // constant, linear, quadratic
	float theta = 0.0f;                          // spot inner cone angle (radians)
	float phi = 0.0f;                            // spot outer cone angle (radians)
	float falloff = 0.0f;                        // spot exponent
};

// The view group's model lighting (std140, pbr_lighting.glsl).
struct PbrModelLighting
{
	struct Light
	{
		float color[4] = {};
		float direction[4] = {};
		float position[4] = {};
		float spot[4] = {};
		float attenuation[4] = {};
	};
	float eye[4] = {}; // w: the number of lights
	float cube[6][4] = {};
	Light lights[4];
};
static_assert( sizeof( PbrModelLighting ) == 432 );

inline constexpr std::size_t kPbrMaxLights = 4;

// Packs the eye, the ambient cube (+x, -x, +y, -y, +z, -z) and up to four
// lights as the legacy shader API does: lights sorted spot, point,
// directional (SortLights, stable), each as CShaderAPIDx8::SetLight builds
// cLightInfo. Lights past the fourth are dropped, as the port drops them.
PbrModelLighting PackSourceModelLighting(
    const float eye[3], const float cube[6][3], std::span<const PbrLightDesc> lights );

// The frame group's split-sum table (RFC 0007, pbr_split_sum_table.h): the
// texels for a kRGBA32Float texture sampled linearly with clamped addressing.
struct PbrSplitSumTable
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	device::Format format = device::Format::kRGBA32Float;
	std::vector<float> texels; // RGBA per texel
};
PbrSplitSumTable SplitSumTable();

struct PbrClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	bool normalMap = false;
	bool emission = false;
	PbrConstants constants;
};

// Whether the family draws this block's material, and how. The block must be
// of the `pbr` family's schema (FamiliesFromMapping). An unused texture slot
// (no normal map or emission) still needs a texture bound; its contents are
// not read.
PbrClaim ClaimPbr( const ParameterBlock &block );

enum class PbrStatus : std::uint8_t
{
	kDevice = 1 // a layout or pipeline was refused
};

class PbrFamily
{
public:
	static foundation::Expected<std::unique_ptr<PbrFamily>, PbrStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat );
	~PbrFamily();
	PbrFamily( const PbrFamily & ) = delete;
	PbrFamily &operator=( const PbrFamily & ) = delete;

	// Binding 0 the split-sum table, 1 its sampler.
	device::BindGroupLayoutId FrameLayout() const { return m_FrameLayout; }
	// Binding 0 the model lighting.
	device::BindGroupLayoutId ViewLayout() const { return m_ViewLayout; }
	// Binding 0 the constants; 1-2 base, 3-4 MRAO, 5-6 normal map, 7-8
	// emission (texture, sampler).
	device::BindGroupLayoutId MaterialLayout() const { return m_MaterialLayout; }
	// The opaque pipeline (created on first use).
	foundation::Expected<device::PipelineId, PbrStatus> Pipeline( const PbrClaim &claim );

private:
	explicit PbrFamily( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	device::Format m_DepthFormat = device::Format::kUnknown;
	device::BindGroupLayoutId m_FrameLayout;
	device::BindGroupLayoutId m_ViewLayout;
	device::BindGroupLayoutId m_MaterialLayout;
	device::PipelineId m_Pipeline;
};

} // namespace render::material

#endif // RENDER_MATERIAL_PBR_FAMILY_H
