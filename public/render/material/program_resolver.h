//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One resolver from a material to the program that draws it (RFC
//			0016 K5, "One model; legacy cases are degenerate").
//
//			Passes (world, props, models) never name a family. They map a
//			material's variables (MapVariables), hand the result here, and
//			get back a ProgramRequest: the pipeline for the pass's target
//			formats, the material group's constants and textures (by the
//			importer's normalized names, which the caller resolves to
//			textures), the vertex stride and draw constants the program
//			reads, and the per-draw inputs its draw group takes by name
//			("lightmap": the draw's lightmap page).
//
//			This is the one place that knows the families. Today it
//			dispatches to the narrow ones (lightmapped, unlit); as they fold
//			into the general surface model the dispatch shrinks, and callers
//			do not change. A material it cannot draw is refused with the
//			reason, which names the gap in the model.
//
//			Render sequence: pipelines are made on first use.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_PROGRAM_RESOLVER_H
#define RENDER_MATERIAL_PROGRAM_RESOLVER_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/material/surface_program.h"
#include "render/material/vmt_import.h"
#include "render/shaderlib/debug_view.h"
#include "render/sprite_card.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace render::material
{

// The frame's terms every program reads a subset of (one owner): how the
// lightmap pages encode light and the output's linear (tone-mapping) scale.
struct FrameTerms
{
	float motionCurrentToClip[16] = {};
	float motionPreviousToClip[16] = {};
	float motionExtent[4] = {};
	float lightmapScale = 4.5947938f; // LDR gamma pages at half overbright (2^2.2)
	float outputScale = 1.0f;
	// The target has no sRGB view: the shader encodes its output (the
	// resolver's color format is then the target's unorm format).
	bool encodeOutput = false;
	// The view's fog (legacy::CorePassFog's terms): type -1 none, 0 range,
	// 1 height; color linear and tone-scaled; parameters; the eye's world z.
	float fogType = -1.0f;
	float fogColor[3] = { 0.0f, 0.0f, 0.0f };
	float fogParams[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float fogEyeZ = 0.0f;
	float clipPlanes[6][4] = {};
	// The eye's world position (the env map's reflection), ENV_MAP_SCALE (16
	// in integer HDR, else 1), and whether specular shows (mat_fastspecular):
	// when not, env map tints are zero.
	float eye[3] = { 0.0f, 0.0f, 0.0f };
	float envmapScale = 1.0f;
	bool specular = true;
	// The running game's shaders scale every ssbump by 1/sqrt(3) (Portal 2).
	bool ssbumpNormalized = false;
	// The pbr point's frame inputs (world pbr, SetWorldPbr): the split-sum
	// and LTC tables by TextureCache name (SplitSumTable(), LtcTable()), the
	// map's probe textures, and the frame's area lights (at most
	// kSurfaceMaxAreaLights, PackAreaLight).
	std::string splitSumTable;
	std::string ltcTable;
	SurfaceMapTextures map;
	std::vector<SurfaceAreaLight> areas;
	// The sun (SurfaceFrame::sunDirection, sunColor, sunShadow).
	float sunDirection[4] = {};
	float sunColor[4] = {};
	float sunShadow[4] = { -1.0f, 0.0f, 0.0f, 0.0f };
	// The water point's (SurfaceFrame::water, viewport): the shaders' time
	// in seconds, its reflection tint's scale (4 in integer HDR), the
	// camera's right in the water plane (normalized) and the view's viewport
	// (x, y, 1 / width, 1 / height).
	float time = 0.0f;
	float foliage[2][4] = {};
	bool foliageAvailable = false;
	float waterReflectTintScale = 1.0f;
	float viewRight[2] = { 1.0f, 0.0f };
	float viewport[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
};

// The vertex a resolver's programs read: the flat vertex (position, base and
// lightmap coordinates, color), or the surface vertex, which adds the normal,
// tangents and the bumped lightmap pages' offset that bump and env map terms
// read (material::SurfaceWorldVertex). A flat resolver refuses those
// terms by name.
enum class VertexLayout : std::uint8_t
{
	kFlat,
	kSurface,
	kModel // object-space mesh; the draw constants hold its transform
};

struct ResolvedProgram
{
	// The program's name, as cl_render_debug_view_program selects it (one
	// of ProgramResolver::ProgramNames()).
	std::string name;
	ProgramRequest request;
	device::BlendMode blend = device::BlendMode::kOpaque;
	bool sceneColor = false; // the view group needs a snapshot before this draw
	bool depthBlend = false; // requires the view's copied depth-alpha input
	bool fogToBlack = false; // additive sprite modes fog their radiance to zero
	bool foliage = false; // $treesway needs captured animation time and wind
	bool twoSided = false;   // authored $nocull; mesh winding belongs to its draw owner
	// The per-draw inputs the draw group takes, in binding order.
	std::vector<std::string> drawInputs;
	// The view's render targets the program reads through its view group's
	// screen inputs, by the importer's texture names (the water point's
	// planar reflection, SurfaceScreenInputs::planarReflection): the pass
	// imports the target each view, as the stream drew it before the slot.
	std::vector<std::string> viewInputs;
};

// Missing/nonfinite required frame inputs are refused even when the pass
// already wrote another material's group of the same layout.
std::optional<std::string> FrameInputError( const ResolvedProgram &program, const FrameTerms &terms );

// Replacement program modules (SPIR-V words): the debug suites' seeded
// programs, borrowed for the resolver's lifetime. Color and shadow entry
// points have separate interfaces; empty spans keep the families' own.
struct ProgramModules
{
	std::span<const std::uint32_t> lightmappedFragment;
	std::span<const std::uint32_t> shadowFragment = {};
};

class ProgramResolver
{
public:
	// Programs for targets of these formats (color as the pass writes it) and
	// sample count.
	static foundation::Expected<std::unique_ptr<ProgramResolver>, std::string> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1, VertexLayout layout = VertexLayout::kFlat,
	    const ProgramModules &modules = {} );
	// The names of the programs the resolver serves (ResolvedProgram::name).
	static std::span<const std::string_view> ProgramNames();
	~ProgramResolver();
	ProgramResolver( const ProgramResolver & ) = delete;
	ProgramResolver &operator=( const ProgramResolver & ) = delete;

	// The program for a mapped material, or why it has none.
	foundation::Expected<ResolvedProgram, std::string> Resolve( const MaterialDesc &material );
	// World pbr (RFC 0016 K11 "World PBR materials"): with it, a surface
	// resolver claims PBRMetalRough materials as the surface program's pbr
	// point on the world vertex, with kSurfaceBakedLightmap and these scene
	// terms (kSurfaceDirectionalLightmap, kSurfaceMapProbeTerms,
	// kSurfaceClustered and the other scene terms of surface_program.h),
	// taking the draw inputs "lightmap", "lightmap-gradient",
	// "lightmap-indirect" and "lightmap-shadow-mask" (the static lights'
	// baked shadow masks, LSMK) ("lightmap-indirect-gradient" in place of the
	// second with kSurfaceRuntimeDirect among the terms). Without it
	// (the default, and the product until K12) a pbr material is refused by
	// name. Call before resolving; programs already resolved keep theirs.
	void SetWorldPbr( bool enabled, std::uint32_t sceneTerms = 0 );
	// The caller has a linear scene-color snapshot for a later transmission draw.
	// The default refuses every such material until the view owns that input.
	void SetSceneColorAvailable( bool available );
	// A PBRMetalRough or supported VertexLitGeneric mesh on the world or model vertex:
	// probe-volume indirect light and clustered, shadowed direct light.
	foundation::Expected<ResolvedProgram, std::string> ResolveMesh( const MaterialDesc &material );
	// A resolved program's pipeline with terms added and removed (a pass's
	// variant of it: the prepass's kSurfaceDepthNormal, the SSR targets).
	foundation::Expected<device::PipelineId, std::string> VariantPipeline(
	    const ResolvedProgram &program, std::uint32_t add, std::uint32_t remove );
	// A resolved program's static vertex light variant (a static prop's baked
	// per-vertex lighting, SurfaceVariant::staticVertexLight).
	foundation::Expected<device::PipelineId, std::string> StaticVertexLightPipeline(
	    const ResolvedProgram &program );
	// The one surface program every point is drawn through (its layouts for
	// the view group, SurfaceProgram::ViewGroup).
	SurfaceProgram &Program() const;
	// The draw group a resolved program reads, with its inputs' textures by
	// name (in drawInputs order); nullopt when the program reads none. A mesh
	// point without inputs takes `lighting` (Source's model lighting at the
	// draw) when given, else the neutral block.
	std::optional<GroupRequest> DrawGroup( const ResolvedProgram &program,
	    const std::vector<std::string> &inputTextures,
	    const ModelLighting *lighting = nullptr ) const;
	// The program's pipeline under a debug specialization (RFC 0014): the
	// shipped pipeline when it is neutral, else its debug variant (made on
	// first use and kept); the reason when the program has no variant.
	foundation::Expected<device::PipelineId, std::string> DebugPipeline(
	    const ResolvedProgram &program, const shaderlib::DebugSpecialization &debug );
	// The frame group a resolved program reads, for these terms; nullopt when
	// it reads none.
	std::optional<GroupRequest> FrameGroup(
	    const ResolvedProgram &program, const FrameTerms &terms ) const;
	// FrameGroup's constants alone (its GroupRequest::constants), for a
	// frame group already built whose terms change: no texture names or
	// storage bytes are copied.
	std::optional<std::vector<std::byte>> FrameConstants(
	    const ResolvedProgram &program, const FrameTerms &terms ) const;

	// The editor's preview of any material (Hammer's textured view). By
	// contract an approximation, never a claim, and never used by the game's
	// passes: the base texture times $color and $alpha, with the material's
	// blend (translucent, additive), alpha test and reference, the vertex
	// color on and the lighting fixed at one. Every variable it does not
	// read is named in `ignored`, so nothing is silent.
	struct Preview
	{
		ResolvedProgram program;
		std::vector<std::string> ignored; // lower case, as the VMT names them
	};
	foundation::Expected<Preview, std::string> ResolvePreview( const MaterialDesc &material );

private:
	struct State;
	explicit ProgramResolver( std::unique_ptr<State> state );
	std::unique_ptr<State> m_State;
};

// Whether a material's variables claim no more than the model draws, without
// a device (the pass's main-thread decision to take its surfaces). The same
// rules as Resolve, minus pipelines; worldPbr as SetWorldPbr (a pbr material
// is claimed only with it). Optional requiresDepthAlpha reports the claimed
// point’s ordered depth input, so callers never interpret a family’s variables.
// nativeReflectionProbes: the world stage carries reflection probes (RPRB),
// which a LightmappedGeneric $envmap env_cubemap reads in place of the
// view's legacy cube map.
foundation::Expected<device::BlendMode, std::string> ClaimForDrawing(
    const MaterialDesc &material, bool worldPbr = false, bool *requiresDepthAlpha = nullptr,
    bool nativeReflectionProbes = false );
// Additional ordering requirements for an already claimed opaque material:
// no scene-color dependency, animated frame time or depth/stencil-only effect.
// Evaluated when the material snapshot is claimed, not during view recording.
bool SupportsOpaqueBatch( const MaterialDesc &material, bool mesh );

// The mesh point's exact-variable claim without making a pipeline.
foundation::Expected<device::BlendMode, std::string> ClaimForMesh( const MaterialDesc &material,
    bool nativeReflectionProbes = false, bool sceneColorAvailable = false );

// A SpriteCard material's card constants (render.sprite-card.v1), which the
// unlit point's claim owns; nullopt for any other material or one whose
// values do not fit the schema.
std::optional<sprite_card::Frame> SpriteCardTermsFor( const MaterialDesc &material );

} // namespace render::material

#endif // RENDER_MATERIAL_PROGRAM_RESOLVER_H
