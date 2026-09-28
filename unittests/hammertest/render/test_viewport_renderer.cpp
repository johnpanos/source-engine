//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.viewport.null (RFC 0002
//			hammer.adapters.render; RFC 0016 "Editor viewports"; contract
//			render_adapter.viewport-geometry.v1): the editor's ViewportRenderer
//			on render.device.null, which validates every transition and
//			records the executed commands.
//
//			V1 the scene is staged once per key: the same key restages
//			   nothing, a new key restages;
//			V2 a 2D view with a grid runs two render passes (the grid clears,
//			   the scene loads) and draws the edges and the overlay; the 3D
//			   view runs one and draws faces, edges and the overlay;
//			V3 nothing blocks: Take returns nothing while the device has not
//			   completed the frame and the pixels (width x height RGBA) once
//			   it has;
//			V4 a view without its camera, a zero size and an unknown ticket
//			   are refused;
//			V5 the renderer releases everything it made: after it is gone the
//			   device holds what it held before.
//
//=============================================================================//

#include "hammer/adapters/render/viewport_renderer.h"
#include "render/device/null/provider.h"
#include "testing/checks.h"
#include "viewport_fixture.h"

#include <vector>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::ViewportRenderer;
using hammer::render_adapter::ViewportStatus;
using hammer::render_adapter::ViewRequest;
using hammer::viewport::ViewKind;
namespace nulldev = render::device::null;

struct Trace
{
	std::uint32_t passes = 0;
	std::vector<std::uint64_t> draws;
};

Trace Recorded( render::device::IRenderDevice2 &device )
{
	Trace trace;
	for ( const nulldev::RecordedCommand &command : nulldev::Control( device )->Recorded() )
	{
		if ( command.op == nulldev::RecordedOp::kBeginRendering )
			++trace.passes;
		if ( command.op == nulldev::RecordedOp::kDraw ||
		     command.op == nulldev::RecordedOp::kDrawIndexed )
			trace.draws.push_back( command.count );
	}
	return trace;
}

} // namespace

int main()
{
	testing::Checks checks;
	nulldev::NullOptions options;
	options.completion = nulldev::CompletionMode::kManual;
	auto device = nulldev::Create( options ).Value();
	nulldev::INullDeviceControl *control = nulldev::Control( *device );
	const std::size_t baseline = device->LiveResourceCount();
	Document d;
	Build( d );
	{
		auto made = ViewportRenderer::Create( *device );
		if ( !checks.That( made.HasValue(), "setup.renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();

		// V1.
		checks.That( renderer.SetScene( Snapshot( d ), 1 ).HasValue() &&
		                 renderer.Scene().stagings == 1 && renderer.Scene().triangles == 24,
		    "V1.the-scene-is-staged" );
		checks.That( renderer.SetScene( Snapshot( d ), 1 ).HasValue() &&
		                 renderer.Scene().stagings == 1,
		    "V1.the-same-key-restages-nothing" );
		checks.That( renderer.SetScene( Snapshot( d, false ), 2 ).HasValue() &&
		                 renderer.Scene().stagings == 2,
		    "V1.a-new-key-restages" );
		control->CompleteAll();

		// V2, V3.
		const hammer::viewport::Camera2D top = TopCamera();
		ViewRequest request2D;
		request2D.kind = ViewKind::Top;
		request2D.camera2D = &top;
		request2D.grid = { { hammer::viewport::GridLineKind::Minor,
		    hammer::viewport::GridLineOrientation::Vertical, 0.0, 10.0 } };
		request2D.overlay.Box(
		    Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ), hammer::tools::OverlayRole::Pending );
		request2D.pixelWidth = 256;
		request2D.pixelHeight = 192;
		control->ClearRecorded();
		auto ticket = renderer.Render( request2D );
		checks.That( ticket.HasValue(), "V2.the-2d-view-renders" );
		if ( ticket )
		{
			auto early = renderer.Take( ticket.Value() );
			checks.That( early.HasValue() && !early.Value(), "V3.nothing-before-completion" );
			control->CompleteAll();
			const Trace trace = Recorded( *device );
			checks.Equal( trace.passes, 2u, "V2.the-grid-and-the-scene-are-two-passes" );
			checks.Equal( trace.draws,
			    std::vector<std::uint64_t>{ 2, renderer.Scene().edgeVertices, 24 },
			    "V2.the-2d-view-draws-grid-edges-and-overlay" );
			auto taken = renderer.Take( ticket.Value() );
			checks.That( taken.HasValue() && taken.Value() && taken.Value()->width == 256 &&
			                 taken.Value()->rgba.size() == std::size_t( 256 * 192 * 4 ),
			    "V3.the-pixels-arrive-after-completion" );
		}
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request3D;
		request3D.kind = ViewKind::Camera3D;
		request3D.camera3D = &eye;
		request3D.pixelWidth = 128;
		request3D.pixelHeight = 96;
		control->ClearRecorded();
		auto ticket3D = renderer.Render( request3D );
		control->CompleteAll();
		const Trace trace3D = Recorded( *device );
		checks.That( ticket3D.HasValue() && trace3D.passes == 1, "V2.the-3d-view-is-one-pass" );
		checks.Equal( trace3D.draws,
		    std::vector<std::uint64_t>{ renderer.Scene().faceVertices, renderer.Scene().edgeVertices },
		    "V2.the-3d-view-draws-faces-then-edges" );
		if ( ticket3D )
		{
			auto taken = renderer.Take( ticket3D.Value() );
			checks.That( taken.HasValue() && taken.Value(), "V3.the-3d-pixels-arrive" );
		}

		// V4.
		ViewRequest noCamera;
		noCamera.kind = ViewKind::Front;
		noCamera.pixelWidth = noCamera.pixelHeight = 8;
		auto refused = renderer.Render( noCamera );
		checks.That( !refused && refused.Error() == ViewportStatus::kInvalidView,
		    "V4.a-view-without-its-camera-is-refused" );
		ViewRequest empty = request3D;
		empty.pixelWidth = 0;
		auto zero = renderer.Render( empty );
		checks.That( !zero && zero.Error() == ViewportStatus::kInvalidView,
		    "V4.a-zero-size-is-refused" );
		auto unknown = renderer.Take( 999 );
		checks.That( !unknown && unknown.Error() == ViewportStatus::kUnknownTicket,
		    "V4.an-unknown-ticket-is-refused" );

		// A pending frame at destruction (V5).
		auto left = renderer.Render( request2D );
		checks.That( left.HasValue() && renderer.PendingCount() == 1, "V5.a-frame-is-pending" );
		control->CompleteAll();
	}
	(void)device->Poll();
	control->CompleteAll();
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "V5.everything-is-released" );
	return checks.Report();
}
