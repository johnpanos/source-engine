//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor's viewports on the RFC 0016 render core (RFC 0002
//			hammer.adapters.render; RFC 0016 "Editor viewports"). One
//			ViewportRenderer draws any number of views of one document through
//			render.pass.lines on the device the composition root passes in
//			(RenderCore_Create's binding->device), offscreen, into RGBA8
//			pixels the host shows (a GdkMemoryTexture now, a dmabuf later).
//
//			Scene geometry (scene_geometry.h) is resident: SetScene restages
//			the faces and edges only when the caller's key changes (the
//			snapshot revision and selection), so camera moves, hover and tool
//			feedback upload only the per-view list (grid and overlay).
//
//			A view renders as one graph: a lines pass for the grid (2D views,
//			clearing the target), then one for the scene and the tool
//			overlay, then a copy into a readback buffer. The 3D view draws the
//			shaded faces and biased edges depth-tested; 2D views draw the
//			edges over the grid. Nothing blocks: Render submits and returns a
//			ticket, and Take returns the pixels once the device completes it
//			(RenderAndWait polls for offscreen hosts and tests).
//
//			Lifetime: the renderer borrows the device and must be destroyed
//			before it; its destructor waits for its own frames to complete.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_VIEWPORT_RENDERER_H
#define HAMMER_ADAPTERS_RENDER_VIEWPORT_RENDERER_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/pass/lines/lines.h"
#include "render/resources/mesh_cache.h"
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
};

struct ViewPixels
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> rgba; // row 0 at the top, straight RGBA8
};

enum class ViewportStatus : std::uint8_t
{
	kDevice = 1,   // the device refused a resource, submission or readback
	kGraph,        // the view's graph did not compile or run
	kPass,         // render.pass.lines refused the view
	kInvalidView,  // no camera for the view's kind, or a zero size
	kUnknownTicket // Take for a ticket this renderer does not hold
};

struct SceneStats
{
	std::uint64_t key = 0;
	std::uint32_t triangles = 0;
	std::uint32_t faceVertices = 0;
	std::uint32_t edgeVertices = 0;
	std::uint32_t stagings = 0; // times the scene was restaged
};

class ViewportRenderer
{
public:
	using Ticket = std::uint64_t;

	static foundation::Expected<std::unique_ptr<ViewportRenderer>, ViewportStatus> Create(
	    ::render::device::IRenderDevice2 &device );
	~ViewportRenderer();

	ViewportRenderer( const ViewportRenderer & ) = delete;
	ViewportRenderer &operator=( const ViewportRenderer & ) = delete;

	// Restages the scene when 'key' differs from the resident one.
	foundation::Expected<void, ViewportStatus> SetScene(
	    const viewport::RenderSnapshot &snapshot, std::uint64_t key );
	const SceneStats &Scene() const { return m_Stats; }

	// Records and submits one view.
	foundation::Expected<Ticket, ViewportStatus> Render( const ViewRequest &request );
	// The ticket's pixels once the device completed it; nothing while pending.
	foundation::Expected<std::optional<ViewPixels>, ViewportStatus> Take( Ticket ticket );
	// Render, then poll until the pixels arrive (offscreen hosts and tests).
	foundation::Expected<ViewPixels, ViewportStatus> RenderAndWait( const ViewRequest &request );

	std::size_t PendingCount() const { return m_Pending.size(); }

private:
	struct Pending
	{
		::render::device::CompletionToken token;
		::render::device::TextureId color;
		::render::device::BufferId readback;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};

	explicit ViewportRenderer( ::render::device::IRenderDevice2 &device )
	    : m_Device( device ), m_Meshes( device )
	{
	}
	void Release( const Pending &pending );

	::render::device::IRenderDevice2 &m_Device;
	::render::resources::MeshCache m_Meshes;
	std::unique_ptr<::render::pass::lines::LinesRenderer> m_Lines;
	std::optional<::render::resources::MeshEntry> m_Faces;
	std::optional<::render::resources::MeshEntry> m_Edges;
	std::optional<scene::Box> m_Bounds;
	SceneStats m_Stats;
	bool m_HaveScene = false;
	std::map<Ticket, Pending> m_Pending;
	Ticket m_NextTicket = 1;
	::render::device::CompletionToken m_LastToken;
};

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_VIEWPORT_RENDERER_H
