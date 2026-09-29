//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor's viewports on the RFC 0016 render core (RFC 0002
//			hammer.adapters.render; RFC 0016 "Editor viewports"). One
//			ViewportRenderer draws any number of views of one document on the
//			device the composition root passes in (RenderCore_Create's
//			binding->device), offscreen, into sRGB RGBA8 pixels the host shows
//			(a GdkMemoryTexture now, a dmabuf later).
//
//			Solids draw through the render core's editor preview (RFC 0016
//			K5, render::material::ProgramResolver::ResolvePreview): each face
//			batch (scene_geometry.h) of a chunk is a mesh and one instance of
//			the renderer's own render.scene, drawn by render.pass.opaque. With
//			a material source (IMaterialTextures) the camera view is
//			textured: a material is fetched once on the render sequence as
//			its imported MaterialDesc and decoded textures, resolved with
//			ResolvePreview (the base texture times $color and $alpha, the
//			material's blend and alpha test, the vertex color on, lighting
//			one), and every texture the program samples is staged by the
//			program's name with its mip chain (BuildMipChain, linear light) and
//			sampled trilinearly. The program's draw group gets a neutral
//			lightmap page (white; lighting one reads none) and its frame
//			group the LDR terms for this sRGB target. A material the preview
//			approximates counts each variable it ignores
//			(SceneStats::ignored); one that does not import or resolve is a
//			named failure (SceneStats::failures); both, and a material whose
//			texture is missing, draw untextured. Untextured faces, markers and
//			the flat preview (no source) draw through the same path: a
//			neutral material whose base texture is white. Selection tint and
//			shading stay in the vertex colors. Blended materials ($translucent,
//			$vertexalpha, $additive) write no depth and draw after the opaque
//			ones. Edges, the grid and tool overlays draw through
//			render.pass.lines.
//
//			Studio models (R17 follow-up): with a model source
//			(IModelSource) a point entity whose model is a studio model is
//			drawn as the model. Its file is read once per path; its meshes
//			are staged once per (path, skin, tint) as indexed per-material
//			batches in model space (BuildModelBatches) and drawn as scene
//			instances with the entity's world matrix (ModelWorld), textured
//			through the same material programs. The tint is the selection
//			fill when selected, else its render color times the instance
//			tint for instance content. Its box (ModelWorldBox) is drawn in
//			2D views, and in the camera view only when selected. A model
//			that is missing, fails to parse or has no triangles draws its
//			marker box instead and is counted (missingModels).
//
//			Instances: each InstanceDraw is one chunk of its own (keyed by
//			the func_instance's id, apart from the id chunks), staged like a
//			document chunk with the instance tint (kInstanceTint on faces
//			and content models, kInstanceEdgeColor on edges) unless the
//			instance is selected.
//
//			Scene geometry is resident per chunk (ChunkOf: an object's id
//			over 64): a chunk has one mesh per material and one edge mesh.
//			SetScene does nothing when the caller's key (the snapshot
//			revision and selection) is unchanged; otherwise it compares each
//			chunk's objects with the ones it staged and rebuilds and uploads
//			only the chunks that differ, so an edit of one solid restages
//			its chunk's meshes and every other buffer stays. Camera moves,
//			hover and tool feedback upload only the per-view list (grid and
//			overlay).
//
//			A view renders as one graph: in the camera view the opaque pass
//			(clearing) draws the opaque and alpha-tested batches (draw list
//			of view bit 0), a second pass loading the targets draws the
//			blended batches (view bit 1) back to front, then the lines pass
//			draws the depth-tested, biased edges and the overlay; in 2D views
//			the grid lines pass (clearing), then the edges and overlay; then
//			a copy into a readback buffer. Nothing blocks: Render submits and
//			returns a ticket, and Take returns the pixels once the device
//			completes it (RenderAndWait polls for offscreen hosts and
//			tests).
//
//			Lifetime: the renderer borrows the device and the material source
//			and must be destroyed before them; its destructor waits for its
//			own frames to complete. It is used on one sequence.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_VIEWPORT_RENDERER_H
#define HAMMER_ADAPTERS_RENDER_VIEWPORT_RENDERER_H

#include "foundation/expected.h"
#include "material_textures.h"
#include "model_source.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/material/program_resolver.h"
#include "render/pass/lines/lines.h"
#include "render/pass/opaque/opaque.h"
#include "render/resources/mesh_cache.h"
#include "render/resources/texture_cache.h"
#include "render/scene/scene.h"
#include "scene_geometry.h"

#include <cstdint>
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace hammer::render_adapter
{

struct ViewRequest
{
	viewport::ViewKind kind = viewport::ViewKind::Camera3D;
	const viewport::Camera2D *camera2D = nullptr; // for Top, Front and Side
	const viewport::Camera3D *camera3D = nullptr; // for Camera3D
	std::vector<viewport::GridLine> grid;         // 2D views
	tools::OverlayList overlay;
	// Framebuffer pixels (the camera's logical size times the display scale).
	std::uint32_t pixelWidth = 0;
	std::uint32_t pixelHeight = 0;
	// Render into an exported image (clause D18) the host shows without a
	// copy, instead of reading the pixels back. Needs CanExport().
	bool external = false;
};

// An exported frame: one plane of a dmabuf (render.device external_images.h).
// The handle belongs to the renderer and stays valid until ReturnFrame(lease)
// or the renderer's destruction; a host that keeps the memory longer takes
// its own copy of the handle. The image is not drawn into again before its
// lease is returned.
struct ExternalFrame
{
	std::int64_t handle = -1;
	std::uint32_t fourcc = 0;
	std::uint64_t modifier = 0;
	std::uint32_t offset = 0;
	std::uint32_t stride = 0;
	std::uint64_t lease = 0;
};

struct ViewPixels
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> rgba; // row 0 at the top, straight RGBA8; empty for an external frame
	std::optional<ExternalFrame> external;
};

enum class ViewportStatus : std::uint8_t
{
	kDevice = 1,    // the device refused a resource, submission or readback
	kGraph,         // the view's graph did not compile or run
	kPass,          // a pass or the material family refused the view
	kInvalidView,   // no camera for the view's kind, or a zero size
	kUnknownTicket, // Take for a ticket this renderer does not hold
	kUnsupported    // an external frame on a device that does not export images
};

// The vertex the preview program draws (ResolvePreview: the lightmapped
// program with lighting one): position, base uv, lightmap uv (unused) and
// color. The renderer refuses a program whose stride differs.
struct PreviewVertex
{
	float position[3] = {};
	float uv[2] = {};
	float lightmapUv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};
static_assert( sizeof( PreviewVertex ) == 32 );

struct SceneStats
{
	std::uint64_t key = 0;
	std::uint32_t triangles = 0;
	std::uint32_t faceVertices = 0;
	std::uint32_t edgeVertices = 0;
	std::uint32_t stagings = 0; // times the scene was restaged
	std::uint32_t batches = 0;  // face batches (instances) of every chunk, untextured ones included
	std::uint32_t texturedBatches = 0;
	std::uint32_t blendedBatches = 0;  // drawn after the opaque ones ($translucent, $additive)
	std::uint32_t textures = 0;        // materials whose base texture is resident
	std::uint32_t missingTextures = 0; // materials drawn untextured (a failure, or no texture)
	std::uint32_t failedMaterials = 0; // of those, the ones in 'failures'
	std::uint32_t approximatedMaterials = 0; // drawn with variables the preview ignores
	std::uint32_t chunks = 0;                // resident chunks
	std::uint32_t stagedChunks = 0;          // chunks the last restage rebuilt
	std::uint32_t stagedMeshes = 0;          // face and edge meshes the last restage uploaded
	std::uint32_t models = 0;                // model files read (parsed or not)
	std::uint32_t modelVariants = 0;         // staged (model, skin, tint) mesh sets
	std::uint32_t modelEntities = 0;         // entities drawn as models
	std::uint32_t missingModels = 0;         // model entities drawn as markers instead
	std::uint32_t instanceChunks = 0;        // resident func_instance content chunks
	// Materials that did not import or resolve, with the reason (drawn untextured).
	std::map<std::string, std::string> failures;
	// Each variable the preview ignored, with the number of materials that set it.
	std::map<std::string, std::uint32_t> ignored;
};

// What the last Render drew (its opaque and blended passes).
struct ViewStats
{
	std::uint32_t drawn = 0;
	std::uint32_t unresolved = 0;
	std::uint32_t blended = 0; // of drawn, in the blended pass
};

class ViewportRenderer
{
public:
	using Ticket = std::uint64_t;

	// 'textures' null: the flat preview. 'models' null: model entities draw
	// their markers.
	static foundation::Expected<std::unique_ptr<ViewportRenderer>, ViewportStatus> Create(
	    ::render::device::IRenderDevice2 &device, IMaterialTextures *textures = nullptr,
	    IModelSource *models = nullptr );
	~ViewportRenderer();

	ViewportRenderer( const ViewportRenderer & ) = delete;
	ViewportRenderer &operator=( const ViewportRenderer & ) = delete;

	// Restages the scene when 'key' differs from the resident one.
	foundation::Expected<void, ViewportStatus> SetScene(
	    const viewport::RenderSnapshot &snapshot, std::uint64_t key );
	const SceneStats &Scene() const { return m_Stats; }
	const ViewStats &LastView() const { return m_LastView; }

	// Records and submits one view.
	foundation::Expected<Ticket, ViewportStatus> Render( const ViewRequest &request );
	// The ticket's pixels once the device completed it; nothing while pending.
	foundation::Expected<std::optional<ViewPixels>, ViewportStatus> Take( Ticket ticket );
	// Render, then poll until the pixels arrive (offscreen hosts and tests).
	foundation::Expected<ViewPixels, ViewportStatus> RenderAndWait( const ViewRequest &request );

	std::size_t PendingCount() const { return m_Pending.size(); }

	// Whether views may render into exported images (the device claims
	// kExternalImages).
	bool CanExport() const { return m_Device.ExternalImages() != nullptr; }
	// The host no longer shows the frame of 'lease'; its image may be drawn
	// into again. Unknown leases are ignored.
	void ReturnFrame( std::uint64_t lease );
	// Exported images the renderer holds (free and leased).
	std::size_t ExternalImageCount() const { return m_External.size(); }

private:
	struct Pending
	{
		::render::device::CompletionToken token;
		::render::device::TextureId color;
		::render::device::BufferId readback;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		int external = -1; // the exported image's slot, or -1
	};

	struct ExternalSlot
	{
		::render::device::ExternalImage image;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		bool busy = false;       // rendering, or leased to the host
		std::uint64_t lease = 0; // while leased
	};

	struct Material
	{
		std::uint64_t id = 0;
		std::optional<TextureSize> size; // nothing: no texture, drawn untextured
		bool blended = false;            // drawn in the blended pass
		std::uint64_t drawGroup = 0;     // its program's draw group (DrawGroups id)
	};

	// What resolving a material's preview gave.
	struct Preview
	{
		std::optional<Material> material; // nothing: drawn untextured
		bool missingTexture = false;      // a texture the program samples is missing
		std::string failure;              // why it did not resolve (not a missing texture)
		std::vector<std::string> ignored;
	};

	// A chunk's objects as staged and its resident meshes.
	struct Chunk
	{
		std::vector<viewport::SolidDraw> solids;
		std::vector<viewport::EntityDraw> entities;
		bool instanceContent = false;                  // a func_instance's content (tinted)
		std::map<std::uint64_t, std::uint64_t> meshes; // material id -> mesh id
		std::vector<::render::scene::InstanceId> instances;
		std::optional<::render::resources::MeshEntry> edges;
		std::optional<::render::resources::MeshEntry> edges2D; // drawn in 2D views only
		std::uint32_t modelEntities = 0;
		std::uint32_t missingModels = 0;
		std::uint32_t triangles = 0;
		std::uint32_t faceVertices = 0;
		std::uint32_t edgeVertices = 0;
		std::uint32_t texturedBatches = 0;
		std::uint32_t blendedBatches = 0;
	};

	// One staged batch of a model variant.
	struct ModelMesh
	{
		std::uint64_t material = 0; // program id
		std::uint64_t mesh = 0;
		std::uint64_t drawGroup = 0;
		bool blended = false;
		::render::math::Aabb bounds; // model space
	};
	// A model file and its staged variants, keyed by (skin, tint r, g, b).
	struct ModelEntry
	{
		std::optional<ModelAsset> asset; // nothing: missing, malformed or without triangles
		std::map<std::array<int, 4>, std::vector<ModelMesh>> variants;
	};

	class Meshes final : public ::render::pass::opaque::IMeshResolver
	{
	public:
		std::map<std::uint64_t, ::render::resources::MeshEntry> entries;
		const ::render::resources::MeshEntry *Mesh( std::uint64_t mesh ) const override
		{
			auto found = entries.find( mesh );
			return found == entries.end() ? nullptr : &found->second;
		}
	};

	ViewportRenderer( ::render::device::IRenderDevice2 &device, IMaterialTextures *textures,
	    IModelSource *models );
	void Release( const Pending &pending );
	foundation::Expected<int, ViewportStatus> ExternalSlotFor(
	    std::uint32_t width, std::uint32_t height );
	foundation::Expected<void, ViewportStatus> ResolveMaterials(
	    const std::vector<viewport::SolidDraw> &solids );
	foundation::Expected<void, ViewportStatus> ResolveMaterialNames(
	    const std::set<std::string> &names );
	// The model an entity names, read on first use; null when the entity
	// draws its marker (no model source, not a studio model, or unusable).
	ModelEntry *ModelFor( const viewport::EntityDraw &entity );
	foundation::Expected<const std::vector<ModelMesh> *, ViewportStatus> VariantFor(
	    ModelEntry &entry, std::int32_t skin, const scene::Rgb &tint );
	// Resolves a material's preview, stages the textures its program samples
	// and its draw and frame groups, and sets the program under 'id'.
	foundation::Expected<Preview, ViewportStatus> AddPreview(
	    std::uint64_t id, const SourceMaterial &source );
	// The neutral draw group (a white lightmap page) and the frame group of a
	// resolved program's layouts, made on first use.
	foundation::Expected<std::uint64_t, ViewportStatus> GroupsFor(
	    const ::render::material::ResolvedProgram &program, std::string &failure );
	// The batch's vertices as the program reads them.
	static std::vector<PreviewVertex> ToPreview( const std::vector<FaceVertex> &vertices );
	// Rebuilds one chunk from 'staged' (its objects now; empty: the chunk is
	// gone) into 'changes'.
	foundation::Expected<void, ViewportStatus> StageChunk(
	    std::uint64_t chunk, Chunk staged, ::render::scene::ChangeSet &changes );
	void DropChunk( Chunk &chunk, ::render::scene::ChangeSet &changes );

	::render::device::IRenderDevice2 &m_Device;
	IMaterialTextures *m_Source = nullptr;
	IModelSource *m_ModelSource = nullptr;
	::render::resources::MeshCache m_MeshCache;
	::render::resources::TextureCache m_Textures;
	::render::material::MaterialPrograms m_Programs;
	::render::material::DrawGroups m_Groups; // neutral lightmap pages and frame groups
	std::unique_ptr<::render::material::ProgramResolver> m_Resolver;
	std::unique_ptr<::render::pass::lines::LinesRenderer> m_Lines;
	std::unique_ptr<::render::scene::IRenderScene> m_Scene;
	Meshes m_FaceMeshes; // by mesh id (one per material per chunk)
	std::map<std::string, Material> m_Materials;
	Material m_Untextured;                              // the neutral material
	std::set<std::string> m_StagedTextures;             // by the programs' names
	std::map<std::uint64_t, std::uint64_t> m_DrawPages; // draw layout -> group id
	std::map<std::uint64_t, std::uint64_t> m_Frames;    // frame layout -> group id
	std::uint64_t m_NextGroup = 1;
	std::map<std::string, ModelEntry> m_Models; // by canonical model path
	std::uint64_t m_NextMaterial = 2;           // 1 is the untextured batch
	std::uint64_t m_NextMesh = 1;
	std::map<std::uint64_t, Chunk> m_Chunks;
	std::optional<scene::Box> m_Bounds;
	SceneStats m_Stats;
	ViewStats m_LastView;
	bool m_HaveScene = false;
	std::map<Ticket, Pending> m_Pending;
	std::vector<ExternalSlot> m_External;
	std::uint64_t m_NextLease = 1;
	Ticket m_NextTicket = 1;
	::render::device::CompletionToken m_LastToken;
};

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_VIEWPORT_RENDERER_H
