//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor's viewports on the RFC 0016 render core (RFC 0002
//			hammer.adapters.render; RFC 0016 "Editor viewports"). One
//			ViewportRenderer draws any number of views of one document on the
//			device the composition root passes in (RenderCore_Create's
//			binding->device), offscreen, into sRGB RGBA8 pixels the host shows
//			(a GdkMemoryTexture now, a dmabuf later).
//
//			Solids draw through the material families (RFC 0016 K4): each
//			face batch (scene_geometry.h) is a mesh and one instance of the
//			renderer's own render.scene, drawn by render.pass.opaque with the
//			unlit family. With a material source (IMaterialTextures) the
//			camera view is textured: a material's base texture is fetched once
//			on the render sequence, staged in a TextureCache and drawn through
//			its own program; a face whose texture is missing draws untextured.
//			Without a source every face is untextured (the flat preview).
//			Edges, the grid and tool overlays draw through render.pass.lines.
//
//			Scene geometry is resident: SetScene restages the batches and
//			edges only when the caller's key changes (the snapshot revision
//			and selection), so camera moves, hover and tool feedback upload
//			only the per-view list (grid and overlay).
//
//			A view renders as one graph: in the camera view the opaque pass
//			(clearing), then the lines pass for the depth-tested, biased edges
//			and the overlay; in 2D views the grid lines pass (clearing), then
//			the edges and overlay; then a copy into a readback buffer. Nothing
//			blocks: Render submits and returns a ticket, and Take returns the
//			pixels once the device completes it (RenderAndWait polls for
//			offscreen hosts and tests).
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
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/material/unlit_family.h"
#include "render/pass/lines/lines.h"
#include "render/pass/opaque/opaque.h"
#include "render/resources/mesh_cache.h"
#include "render/resources/texture_cache.h"
#include "render/scene/scene.h"
#include "scene_geometry.h"

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
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

struct SceneStats
{
	std::uint64_t key = 0;
	std::uint32_t triangles = 0;
	std::uint32_t faceVertices = 0;
	std::uint32_t edgeVertices = 0;
	std::uint32_t stagings = 0; // times the scene was restaged
	std::uint32_t batches = 0;  // face batches (instances), the untextured one included
	std::uint32_t texturedBatches = 0;
	std::uint32_t textures = 0;        // materials whose base texture is resident
	std::uint32_t missingTextures = 0; // materials the source had no texture for
};

// What the last Render drew (its opaque pass).
struct ViewStats
{
	std::uint32_t drawn = 0;
	std::uint32_t unresolved = 0;
};

class ViewportRenderer
{
public:
	using Ticket = std::uint64_t;

	// 'textures' null: the flat preview.
	static foundation::Expected<std::unique_ptr<ViewportRenderer>, ViewportStatus> Create(
	    ::render::device::IRenderDevice2 &device, IMaterialTextures *textures = nullptr );
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

	ViewportRenderer( ::render::device::IRenderDevice2 &device, IMaterialTextures *textures );
	void Release( const Pending &pending );
	foundation::Expected<int, ViewportStatus> ExternalSlotFor(
	    std::uint32_t width, std::uint32_t height );
	foundation::Expected<void, ViewportStatus> ResolveMaterials(
	    const viewport::RenderSnapshot &snapshot );
	foundation::Expected<std::uint64_t, ViewportStatus> AddProgram(
	    std::uint64_t id, const std::string &texture );

	::render::device::IRenderDevice2 &m_Device;
	IMaterialTextures *m_Source = nullptr;
	::render::resources::MeshCache m_MeshCache;
	::render::resources::TextureCache m_Textures;
	::render::material::MaterialPrograms m_Programs;
	std::unique_ptr<::render::material::UnlitFamily> m_Unlit;
	std::unique_ptr<::render::pass::lines::LinesRenderer> m_Lines;
	std::unique_ptr<::render::scene::IRenderScene> m_Scene;
	std::vector<::render::scene::InstanceId> m_Instances;
	Meshes m_FaceMeshes; // by material id (one batch per material)
	std::map<std::string, Material> m_Materials;
	std::uint64_t m_NextMaterial = 2; // 1 is the untextured batch
	std::optional<::render::resources::MeshEntry> m_Edges;
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
