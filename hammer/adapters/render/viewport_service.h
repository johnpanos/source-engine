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
//			Lifetime: the device and both runners outlive the service. The
//			destructor stops replies at once and waits, on the render
//			sequence, for the renderer (and its frames) to be gone, so the
//			device can be destroyed right after it. A job's reply never runs
//			after the service is destroyed.
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
};

class ViewportService
{
public:
	using Result = foundation::Expected<ViewPixels, ViewportStatus>;
	using Done = std::function<void( Result )>;

	// Borrows the device and both runners. The renderer is created on
	// 'render' with the first job.
	ViewportService( ::render::device::IRenderDevice2 &device, platform::ITaskRunner &render,
	    platform::ISequencedTaskRunner &reply );
	~ViewportService();

	ViewportService( const ViewportService & ) = delete;
	ViewportService &operator=( const ViewportService & ) = delete;

	// Posts one view of 'scene' under 'key'; 'done' runs on the reply runner.
	// False when the render runner refused the job (shut down).
	[[nodiscard]] bool Submit( std::shared_ptr<const viewport::RenderSnapshot> scene,
	    std::uint64_t key, ViewJob job, Done done );

	// Poll interval while a frame is on the GPU.
	static constexpr std::uint64_t kPollNanoseconds = 250'000;

	struct State;

private:
	std::shared_ptr<State> m_State;
};

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_VIEWPORT_SERVICE_H
