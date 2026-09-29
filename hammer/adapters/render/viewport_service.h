//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor's viewports on a render sequence (RFC 0016 decision
//			"threading": first the GTK main loop, then a render sequence that
//			owns the device). ViewportService owns a ViewportRenderer that
//			only ever runs on the render sequence the composition root passes
//			in, and replies on the host's sequence, as hammer::app::
//			MapBuildQueue does for builds.
//
//			The host posts immutable inputs: the render snapshot as a shared,
//			const value (the host makes a new one when the scene key changes)
//			and a ViewJob holding copies of the cameras, grid lines and tool
//			overlay. The render sequence stages the scene when the key moved,
//			records the view, and polls the device with delayed tasks (it
//			never blocks on the GPU); the pixels come back through the reply
//			runner. Jobs run in submission order.
//
//			The textured preview's material source (IMaterialTextures) and
//			model source (IModelSource; the same mount) are owned by the
//			service and used only on the render sequence; SetMaterialSource
//			replaces both there, and the next job restages the scene with
//			them. Without a material source the views are the flat preview;
//			without a model source model entities draw their markers.
//
//			Sharing: several services (documents) may use one device, but
//			all of them must be given that device's one render sequence: the
//			device port allows concurrent encoder recording only, not
//			resource creation or submission from several threads.
//
//			Lifetime: the device and both runners outlive the service. The
//			destructor stops replies at once and waits, on the render
//			sequence, for the renderer (and its frames) and the material
//			source to be gone, so the device can be destroyed right after it.
//			A job's reply never runs after the service is destroyed.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_VIEWPORT_SERVICE_H
#define HAMMER_ADAPTERS_RENDER_VIEWPORT_SERVICE_H

#include "platform/contracts/task_runner.h"
#include "viewport_renderer.h"

#include <functional>
#include <memory>
#include <optional>

namespace hammer::render_adapter
{

// One view to draw: the request's inputs by value.
struct ViewJob
{
	viewport::ViewKind kind = viewport::ViewKind::Camera3D;
	std::optional<viewport::Camera2D> camera2D;
	std::optional<viewport::Camera3D> camera3D;
	std::vector<viewport::GridLine> grid;
	tools::OverlayList overlay;
	std::uint32_t pixelWidth = 0;
	std::uint32_t pixelHeight = 0;
	bool external = false; // an exported frame (ViewRequest::external)
};

class ViewportService
{
public:
	using Result = foundation::Expected<ViewPixels, ViewportStatus>;
	using Done = std::function<void( Result )>;

	// Borrows the device and both runners; owns 'textures' and 'models' (each
	// may be null). The renderer is created on 'render' with the first job.
	ViewportService( ::render::device::IRenderDevice2 &device, platform::ITaskRunner &render,
	    platform::ISequencedTaskRunner &reply, std::unique_ptr<IMaterialTextures> textures = {},
	    std::unique_ptr<IModelSource> models = {} );
	~ViewportService();

	ViewportService( const ViewportService & ) = delete;
	ViewportService &operator=( const ViewportService & ) = delete;

	// Posts one view of 'scene' under 'key'; 'done' runs on the reply runner.
	// False when the render runner refused the job (shut down).
	[[nodiscard]] bool Submit( std::shared_ptr<const viewport::RenderSnapshot> scene,
	    std::uint64_t key, ViewJob job, Done done );

	// Replaces the material and model sources (null: the flat preview, marker
	// boxes) after the jobs already posted. False when the render runner
	// refused it.
	[[nodiscard]] bool SetMaterialSource(
	    std::unique_ptr<IMaterialTextures> textures, std::unique_ptr<IModelSource> models = {} );

	// The host no longer shows an external frame (ViewportRenderer::
	// ReturnFrame, on the render sequence). A lease of a renderer since
	// replaced is ignored. False when the render runner refused it.
	bool ReturnFrame( std::uint64_t lease );

	// Poll interval while a frame is on the GPU.
	static constexpr std::uint64_t kPollNanoseconds = 250'000;

	struct State;

private:
	std::shared_ptr<State> m_State;
};

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_VIEWPORT_SERVICE_H
