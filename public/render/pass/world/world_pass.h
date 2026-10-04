//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5, the K5 plan's step 4): the BSP
//			world's surfaces drawn by the core, at a core-pass slot of the
//			legacy stream (render/legacy/core_passes.h).
//
//			At level load the engine hands the pass the world's vertices,
//			triangle indices, surfaces (an index range, a material and a
//			lightmap page each) and materials (shader name, variables, and
//			the material system handles of their textures). The pass names no
//			material family: it maps each material (MapVariables) and asks the
//			one resolver (render/material/program_resolver.h) whether the
//			model draws it, opaque, from the world's vertex; a surface of such
//			a material is the pass's, and the engine stops drawing it through
//			the legacy stream while the pass is on. A material the model does
//			not draw yet stays legacy, with the reason in WorldStats.
//
//			The view's fog (range or height) is a frame term captured when
//			the slot is marked, so fogged views are the pass's too.
//
//			Per view, the engine queues the visible surfaces the pass draws
//			and the view's world-to-clip (QueueView); the returned tag names
//			the slot to mark at that point of the stream. When the scene pass
//			records the slot, Record draws the view's surfaces into the slot's
//			target: the back buffer (its sRGB view, or its unorm view with the
//			shader encoding sRGB) and its depth, loaded and
//			stored, with the resolved programs, and the materials' textures
//			and the lightmap pages imported from the backend (IWorldTextures)
//			with the backend's samplers.
//			Device objects are made on the render sequence at the first Record
//			after SetWorld, one set per target format, and the previous
//			world's are released at a slot of a later frame, behind the
//			submissions of the frames that used them.
//
//			Nothing is dropped silently, and nothing the pass claimed goes
//			back to legacy: a view or a claimed material the pass fails to draw
//			(a texture that does not import, a refused pipeline, a slot with no
//			queued view) counts in WorldStats::viewsFailed with the reason.
//			The composition root decides what a failure costs (the engine's
//			r_core_world_strict makes it fatal). Only a material the model does
//			not draw is legacy's, decided at SetWorld.
//
//			Threads: SetWorld, ClearWorld, Draws and QueueView on the main
//			thread; Record on the render sequence. The queue between them is
//			locked.
//
//=============================================================================//

#ifndef RENDER_PASS_WORLD_WORLD_PASS_H
#define RENDER_PASS_WORLD_WORLD_PASS_H

#include "render/draw_phase.h"

#include "render/device/device.h"
#include "render/frame/debug_controls.h"
#include "render/material/surface_program.h"
#include "render/resources/mip_feedback.h"
#include "render/shadow_tile.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace render::pass::world
{

// The surface vertex (material::SurfaceWorldVertex): the programs'
// bump and env map terms read its normal, tangents and bumped pages' offset.
using WorldVertex = material::SurfaceWorldVertex;

struct WorldMaterial
{
	std::string name;
	std::string shader;
	bool mesh = false; // object-space static model; resolves through the PBR mesh point
	bool translucent = false; // Studio's two-pass material classification
	std::vector<std::pair<std::string, std::string>> variables; // "$key", value
	// The material system handle of each texture variable ("$key", handle).
	std::vector<std::pair<std::string, int>> textures;
	// The legacy shader's declared default of each parameter ("$key",
	// default): a variable the model does not read must hold it.
	std::vector<std::pair<std::string, std::string>> defaults;
	bool hasProxy = false;
};

struct WorldSurface
{
	std::uint32_t material = 0; // into WorldData::materials
	int lightmapPage = 0;       // the material system handle of its lightmap page
	std::uint32_t firstIndex = 0;
	std::uint32_t indexCount = 0;
};

// One LMAP layer as the lightmap basis reads it (RFC 0008's LMAP;
// tools/quality/lightmap_directional.py writes the directional form). A flat
// page holds the diffuse light (irradiance / pi) at every texel. A
// directional page is twice as wide as it is tall: its left half is that flat
// light E0, baked on the smooth normal N, and its right half holds at the same
// texel the signed world-space luminance gradient beta of E(n) = a + g . n,
// relative to E0. The world mesh's lightmap coordinates span the flat half,
// so the halves become two pages of the same size.
struct LightmapPages
{
	std::uint32_t width = 0; // of each page
	std::uint32_t height = 0;
	std::vector<std::byte> flat;     // RGBA16F texels, row 0 at the top
	std::vector<std::byte> gradient; // RGBA16F beta; empty for a flat page

	bool Directional() const { return !gradient.empty(); }
};

// Splits one layer of `width` x `height` RGBA16F texels; a page whose width
// is twice its height is directional. No page (flat empty) when `layer` does
// not hold exactly that many texels.
LightmapPages SplitLightmapLayer(
    std::span<const std::byte> layer, std::uint32_t width, std::uint32_t height );

// The probe volume as render/shaders/common/probe_volume.glsl reads it (RFC
// 0011 render.probe-volume.v1): the atlas and the grid table rows
// (mapcontainer::WriteProbeGridTable, then the moving occluders' rows).
struct StageProbeVolume
{
	std::uint32_t atlasWidth = 0;
	std::uint32_t atlasHeight = 0;
	std::vector<std::byte> atlas;  // RGBA16F, rows top first
	std::uint32_t tableTexels = 0; // per row
	std::uint32_t rows = 0;
	std::vector<float> table; // RGBA32F texels
};

// A world stage (RFC 0016 K12, "Draw what the lab draws"): a BSP2 map's world
// mesh (WMSH), drawn as the lab draws it. Its surfaces are the mesh's
// meshlets; its materials resolve with world pbr (the pbr point on the world
// vertex, ProgramResolver::SetWorldPbr) and its lighting comes from the
// map's own data: the lightmap pages (LMAP, linear light), the baked probe
// volume (PRBV), the reflection probes (RPRB), and the indirect-light host's
// change volume (RFC 0011 BakedPlusDelta), added to every surface the volume
// covers (kSurfaceProbeBounce). A surface's lightmapPage is not read.
struct WorldStage
{
	LightmapPages lightmap;          // the baked (total) layer's pages
	std::vector<std::byte> indirect; // the indirect layer's flat page; empty without one
	// The indirect layer's gradient page (its own directional half); empty
	// when the bake wrote none.
	std::vector<std::byte> indirectGradient;
	// Runtime direct light (material::kSurfaceRuntimeDirect): the surfaces'
	// basis is the indirect layer and every light's direct light is drawn at
	// runtime. Needs the indirect layer.
	bool runtimeDirect = false;
	std::optional<StageProbeVolume> probes;
	std::uint32_t reflectionWidth = 0;
	std::uint32_t reflectionHeight = 0;
	std::vector<std::byte> reflectionProbes; // RGBA16F (WriteReflectionProbeTexture); empty without
};

struct WorldData
{
	std::vector<WorldVertex> vertices;
	std::vector<std::uint32_t> indices; // triangle lists, into vertices
	std::vector<WorldSurface> surfaces;
	std::vector<WorldMaterial> materials;
	struct StaticMesh
	{
		std::vector<material::SurfaceModelVertex> vertices;
		std::vector<std::uint32_t> indices;
		std::vector<WorldSurface> surfaces; // material indexes into WorldData::materials
		// One material per surface for each studio skin; geometry and GPU buffers
		// remain shared across instances and skins. Empty means skin zero only.
		std::vector<std::vector<std::uint32_t>> skinMaterials;
	};
	struct StaticInstance
	{
		std::uint32_t mesh = 0;
		std::uint32_t skin = 0;
		float world[16] = {}; // object to world, row-major
		// Null selects every surface; an explicit empty list is a blank model.
		// A subset contains unique, increasing indices into StaticMesh::surfaces.
		std::optional<std::vector<std::uint32_t>> surfaceSelection;
	};
	std::vector<StaticMesh> staticMeshes;
	std::vector<StaticInstance> staticInstances;
	// Set for a world stage (a BSP2 map's WMSH); null for the BSP surfaces.
	std::shared_ptr<const WorldStage> stage;
};

// The backend's textures (render/legacy/core_passes.h ICoreTextures).
class IWorldTextures
{
public:
	struct MipInfo
	{
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		std::uint32_t levels = 0;
	};
	virtual device::TextureId Import( int handle, bool srgb ) = 0;
	virtual device::SamplerDesc Sampler( int handle ) = 0;
	virtual std::optional<MipInfo> MipDescription( int ) { return std::nullopt; }

protected:
	~IWorldTextures() = default;
};

struct StageViewLights;

struct WorldSceneColor
{
	device::TextureId texture;
	device::TextureDesc desc;
};

// The composition owns capture storage until the encoder's submission is
// complete. The pass only borrows the sampled snapshot for this view.
class IWorldSceneColorCapture
{
public:
	virtual ~IWorldSceneColorCapture() = default;
	virtual std::optional<WorldSceneColor> Capture( device::IRenderDevice2 &device,
	    device::CommandEncoder &encoder, device::TextureId source,
	    const device::TextureDesc &sourceDesc, std::uint64_t frame ) = 0;
};

struct WorldTarget
{
	material::SurfaceDrawState drawState;
	float clipPlanes[6][4] = {};
	bool overrideDepthRange = false;
	float minDepth = 0.0f;
	float maxDepth = 1.0f;
	// Optional capture observer, called synchronously on the render sequence
	// with the producer's semantic view and the viewport actually used.
	std::function<void( std::uint64_t, device::Viewport )> temporalViewport;
	// Replaces the queued view's lights when set: a stage view's clustered
	// lights, made when its slot records rather than when it was queued.
	std::shared_ptr<const StageViewLights> lights;
	// Draw the opaque surfaces' depth into the target first
	// (material::kSurfaceDepthOnly), so the lit pass shades each pixel once.
	bool depthPrepass = false;
	// The device the slot records on (the legacy backend's); the pass's
	// device objects live on it.
	device::IRenderDevice2 *device = nullptr;
	// Optional frame-owned CPU feedback collector; requests are generated only
	// for successfully recorded material surfaces.
	resources::MipFeedbackFrame *mipFeedback = nullptr;
	// The color target (home kColorAttachment): its sRGB view, or, when it has
	// none, its unorm view with the shader encoding sRGB (encodeOutput).
	device::TextureId color;
	device::TextureId motion;      // optional RG16F MRT, home kColorAttachment
	device::TextureId motionDepth; // paired R32F depth of the surface writing motion
	device::Format colorFormat = device::Format::kUnknown;
	bool colorCopySource = false; // the imported target supports kCopySource
	IWorldSceneColorCapture *sceneColorCapture = nullptr;
	bool encodeOutput = false;
	device::TextureId depth; // home kDepthWrite
	device::Format depthFormat = device::Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t samples = 1;
	IWorldTextures *textures = nullptr;
	// A token no earlier than every submission made so far; at a slot of
	// frame F it covers every frame before F. Objects a frame used are
	// released behind it at a slot of a later frame.
	device::CompletionToken submitted;
	// The serial of the frame being recorded (rises with each frame's
	// submission; a frame recorded again keeps or raises it). 0 when the
	// target does not know: retired objects then wait for ReleaseDevice.
	std::uint64_t frame = 0;
	std::uint64_t streamEpoch = 0; // CPU replay lifetime, separate from GPU completion
	// The frame's light terms at the slot (material::FrameTerms).
	float lightmapScale = 1.0f;
	float outputScale = 1.0f;
	// The eye's world position, ENV_MAP_SCALE, and whether specular shows
	// (material::FrameTerms).
	float eye[3] = { 0.0f, 0.0f, 0.0f };
	float envmapScale = 1.0f;
	bool specular = true;
	bool ssbumpNormalized = false; // the game scales every ssbump (Portal 2)
	// The view's fog at the slot (material::FrameTerms, legacy::CorePassFog).
	float fogType = -1.0f;
	float fogColor[3] = { 0.0f, 0.0f, 0.0f };
	float fogParams[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float fogEyeZ = 0.0f;
	// The water point's terms at the slot (material::FrameTerms): the
	// shaders' time in seconds and the reflection tint's scale (4 in integer
	// HDR, where the client draws the water views at a quarter of the tone-map
	// scale).
	float time = 0.0f;
	float foliage[2][4] = {};
	bool foliageAvailable = false;
	float waterReflectTintScale = 1.0f;
	// The shadow atlas the composition drew for this slot's view (a world
	// stage's lights with shadow tiles), in kSampled; invalid without one.
	device::TextureId shadowAtlas;
	device::TextureDesc shadowAtlasDesc;
	// A world stage view's screen passes (RFC 0016 K12, render_lab's order:
	// prepass, then the ambient occlusion the lit pass reads). With them the
	// pass draws the view's depth and normal prepass into its own targets
	// (single-sample, the view's size) and calls screenPasses, which records
	// the occlusion into `ambientOcclusion` (the composition's, in kSampled
	// before and after) and says whether it did. Without them, or when it
	// did not, the occlusion's neutral value (one) applies.
	struct Prepass
	{
		device::TextureId depth;           // D32, kSampled
		device::TextureId normalRoughness; // RGBA16F, kSampled
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};
	std::function<bool( device::CommandEncoder &, const Prepass & )> screenPasses;
	device::TextureId ambientOcclusion;
	device::TextureDesc ambientOcclusionDesc;
};

// A world stage's view lights (RFC 0016 K12): the frame's runtime lights
// clustered for the view (render.pass.lights' layout, as the surface
// program's view group reads it), each packed with whether its diffuse light
// is in the bake. The composition root clusters them; the pass binds them.
// Publish as an immutable snapshot: cohorts in one frame/recording may share
// its uploaded bindings. Changed inputs publish a different snapshot, never
// mutate an alias of an already queued or recorded frame's snapshot.
struct StageViewLights
{
	material::SurfaceViewGpu view;
	device::BufferId gpuFroxels;
	device::BufferId gpuIndices;
	std::vector<std::byte> froxels; // unassigned neutral fixtures only
	std::vector<std::byte> indices; // ClusterIndexHeader, then indices
	std::vector<material::SurfaceLightGpu> lights;
	// The view's shadow tiles (render.shadows.v1), which the lights index;
	// their atlas is the slot's (WorldTarget::shadowAtlas).
	std::vector<ShadowTileGpu> shadowTiles;
	// The frame terms the view's lights set (material::FrameTerms): its
	// area lights (with their shadow tiles) and the sun.
	std::vector<material::SurfaceAreaLight> areas;
	float sunDirection[4] = {};
	float sunColor[4] = {};
	float sunShadow[4] = { -1.0f, 0.0f, 0.0f, 0.0f };
};

// Opaque composition-owned lighting snapshot. Its lifetime follows the view
// through queueing, recording and capture replay.
struct StageLightingInputs
{
	virtual ~StageLightingInputs() = default;
};

struct WorldView
{
	std::vector<std::uint32_t> surfaces; // into WorldData::surfaces
	struct StaticInstance
	{
		std::uint32_t instance = 0; // into WorldData::staticInstances
		// Captured per-view geometry selection, including the selected LOD.
		// Absence inherits the world's instance selection when queued.
		std::optional<std::vector<std::uint32_t>> surfaceSelection;
		StaticInstance() = default;
		StaticInstance( std::uint32_t value ) : instance( value ) {}
	};
	std::vector<StaticInstance> staticInstances;
	// One animated model at the pose captured for this view. Geometry and
	// material skins are shared with WorldData::staticMeshes; these vertices
	// are already in world space, so the model draw uses an identity transform.
	// The value copy survives queued rendering and a later animation update.
	struct PosedModel
	{
		std::uint32_t mesh = 0;
		std::uint32_t skin = 0;
		RenderCoreDrawPhase phase = RenderCoreDrawPhase::kAll;
		std::vector<material::SurfaceModelVertex> vertices;
		std::vector<material::SurfaceModelVertex> previousVertices;
		// Captured geometry selection; later body-group changes cannot mutate it.
		std::optional<std::vector<std::uint32_t>> surfaceSelection;
	};
	std::vector<PosedModel> posedModels;
	// Dynamic geometry captured at its ordered stream slot. The material and
	// vertices are values: proxy changes and mesh reuse cannot change a queued draw.
	struct DynamicDraw
	{
		WorldMaterial material;
		float modelToWorld[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		std::vector<WorldVertex> vertices;
		std::vector<std::uint32_t> indices;
		int lightmapPage = 0; // IWorldTextures handle; 0 binds the neutral page
		// Native page/UV pair captured by the frontend; never reinterpret these
		// coordinates as the compiled stage atlas's coordinates.
		bool capturedLightmap = false;
	};
	std::vector<DynamicDraw> dynamicDraws;
	// Captured ordered depth-copy input for soft particles; one-based importer
	// handle. Required when a draw sets $depthblend, never the live attachment.
	int depthAlphaHandle = 0;
	float depthAlphaRange = 0.0f;

	float toClip[16] = {};               // world to clip, row-major, D3D9 conventions
	float motionToClip[16] = {};         // unjittered camera transform
	float previousToClip[16] = {};
	bool previousViewValid = false;
	std::uint64_t temporalView = 0; // producer identity, never inferred from matrices
	device::Viewport viewport;
	// The host frame that queued the view (views of one frame share it; 0
	// when unknown). A view whose slot never records is skipped, not failed,
	// only when no slot of its frame recorded: the backend never recorded
	// that frame (a resize, a lost surface, a dropped queued frame).
	std::uint64_t hostFrame = 0;
	std::shared_ptr<const StageLightingInputs> stageLighting;
	// The frame's debug controls (RFC 0014), as the renderer applied them
	// when the view was queued: the render sequence draws with the main
	// thread's frame value, in either queued mode.
	frame::DebugControls debug;
	// A world stage's view: its clustered lights, area lights and sun. A
	// view without a stage reads its area lights alone. Null draws the
	// program's neutral view (no runtime light).
	std::shared_ptr<const StageViewLights> lights;
	// The camera's right in the water plane, normalized (the view's x axis
	// with its z dropped): the water point offsets its reflection along it.
	float viewRight[2] = { 1.0f, 0.0f };
	// The height the view moves water surfaces by (the client's waterZAdjust:
	// an eye within r_eyewaterepsilon of a water plane moves the plane off
	// it); the water point's draws are translated by it.
	float waterZOffset = 0.0f;
};

struct WorldStats
{
	std::uint32_t materials = 0;
	std::uint32_t claimedMaterials = 0; // opaque materials the model draws
	std::uint32_t surfaces = 0;
	std::uint32_t claimedSurfaces = 0;
	std::uint64_t viewsQueued = 0;
	std::uint64_t viewsDrawn = 0;
	std::uint64_t viewsFailed = 0;  // claimed work not drawn: never legacy's
	std::uint64_t viewsSkipped = 0; // views of a host frame the backend never recorded
	std::uint64_t surfacesDrawn = 0;
	std::uint64_t staticInstancesQueued = 0;
	std::uint64_t staticDrawsDrawn = 0;
	std::uint64_t posedModelsQueued = 0;
	std::uint64_t posedDrawsDrawn = 0;
	std::uint64_t dynamicDrawsDrawn = 0;
	std::uint64_t dynamicDrawsRefused = 0; // unsupported input, never claimed or queued
	std::string lastRefusal;
	std::string lastFailure;
	// Unclaimed material names, reasons and counts, most frequent first.
	std::vector<std::pair<std::string, std::uint32_t>> gaps;
	// The materials the pass draws ("program material"), and their surface
	// counts.
	std::vector<std::pair<std::string, std::uint32_t>> claimed;
};

// A world tag: the high bit set, then the view's serial in the low 28 bits
// (bits 30 and 29 are the core-pass slots' kCorePassLegacyOff and
// kCorePassFrameEnd; bit 28 marks render.pass.panels' tags, 0x90000000 |
// serial, and bit 27 marks temporal reconstruction tags, 0x88000000 |
// serial, so these never overlap).
// Serial zero is reserved for a composition frame-start marker; QueueView
// allocates serials 1..kWorldSerialMask and never emits it.
inline constexpr std::uint32_t kWorldTag = 0x80000000u;
inline constexpr std::uint32_t kWorldSerialMask = 0x07ffffffu;
inline bool IsWorldTag( std::uint32_t tag )
{
	return ( tag & kWorldTag ) != 0 && ( tag & ~( kWorldTag | kWorldSerialMask ) ) == 0;
}

class WorldPass
{
public:
	WorldPass();
	~WorldPass();
	WorldPass( const WorldPass & ) = delete;
	WorldPass &operator=( const WorldPass & ) = delete;

	// Main thread.
	void SetWorld( WorldData data );
	void ClearWorld();
	// render_lab's sensitivity runs: a replacement fragment module for the
	// surface program (SPIR-V words the caller keeps alive), used by the
	// color resolvers the pass creates after the call. Products never set it.
	void SetSurfaceFragmentModule( std::span<const std::uint32_t> module );
	// A world stage's lighting as it changes (main thread): the total page
	// recomposed (moving objects blocking baked direct light), and the probe
	// volume's change from the bake (the atlas's size and layout; empty for
	// none) with its grid table (the occluders' rows change with it). Each is
	// uploaded in place at the next slot; a size that differs from the
	// stage's is a failure.
	void SetStageLightmap( LightmapPages pages );
	void SetStageChange( std::vector<std::byte> change, StageProbeVolume table );
	// Publish both current probe atlases and the grid table together, so a
	// render sequence never sees a new atlas with the previous table.
	void SetStageProbeVolume(
	    std::vector<std::byte> atlas, std::vector<std::byte> change, StageProbeVolume table );
	// A sparse probe publication: the current atlas and optional change
	// atlas use the same rectangles, with RGBA16F texels packed region after
	// region, rows top first. The grid table is whole. Applied at the next
	// slot as one committed update.
	struct StageRegion
	{
		std::uint32_t x = 0;
		std::uint32_t y = 0;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};
	void SetStageProbeRegions( std::vector<StageRegion> regions, std::vector<std::byte> atlasTexels,
	    std::vector<std::byte> changeTexels, StageProbeVolume table );
	// Rectangles of the total page's flat light recomposed (moving objects
	// blocking baked direct light, RFC 0011), their texels packed rectangle
	// after rectangle, rows top first: updated in place at the next slot of
	// each resource set, on the page as SetStageLightmap or the stage last
	// set it.
	void SetStageLightmapRegions( std::vector<StageRegion> regions, std::vector<std::byte> texels );
	// Whether the pass draws the material's surfaces (valid after SetWorld).
	bool Draws( std::uint32_t material ) const;
	bool DrawsStaticInstance( std::uint32_t instance ) const;
	bool DrawsPosedModel( std::uint32_t mesh, std::uint32_t skin,
	    RenderCoreDrawPhase phase = RenderCoreDrawPhase::kAll,
	    const std::optional<std::vector<std::uint32_t>> &surfaceSelection = std::nullopt ) const;
	// The tag of the slot to mark for the view; 0 when there is nothing to draw.
	std::uint32_t QueueView( WorldView view );
	// Render sequence: largest compatible opaque prefix (at least one for
	// nonempty input). The caller supplies only slots with identical target
	// state and no intervening observable commands. A later world cohort is
	// a boundary; model cohorts retain their original color-draw ordering.
	std::size_t OpaqueBatchSize(
	    std::span<const std::uint32_t> tags, std::uint64_t streamEpoch ) const;
	void RecordBatch( std::span<const std::uint32_t> tags, device::CommandEncoder &encoder,
	    const WorldTarget &target );
	// Add the legacy culler's accepted static props before the queued view's
	// slot records. The array is either accepted whole or refused whole.
	WorldStats Stats() const;
	// Returns the view-owned snapshot for the slot being recorded or replayed.
	std::shared_ptr<const StageLightingInputs> LightingInputs(
	    std::uint32_t tag, std::uint64_t streamEpoch ) const;
	// The view's queued temporal choice, including replay of the same stream.
	bool TemporalView( std::uint32_t tag, std::uint64_t streamEpoch ) const;
	// WorldStats::viewsFailed alone (cheap, for a per-view policy check).
	std::uint64_t Failures() const;

	// Render sequence: draws the view a slot's tag names.
	void Record( std::uint32_t tag, device::CommandEncoder &encoder, const WorldTarget &target );
	// The device is about to go (after an idle wait): its objects are released.
	// A pass destroyed without it drops its handles without calling a device
	// (which may be gone by then).
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace render::pass::world

#endif // RENDER_PASS_WORLD_WORLD_PASS_H
