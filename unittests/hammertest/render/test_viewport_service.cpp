//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.service (RFC 0002 hammer.adapters.render;
//			RFC 0016 decision "threading"; contract
//			render_adapter.viewport-geometry.v1, clauses S1-S5):
//			ViewportService on a real render thread (ThreadTaskRunner) over
//			render.device.null, with the reply runner drained by the test as
//			the GTK main loop would be.
//
//			S1 a job's pixels arrive only through the reply runner, never on
//			   the render thread, with the requested size;
//			S2 jobs reply in submission order;
//			S3 a view without its camera and a missing scene reply with
//			   kInvalidView;
//			S4 destroying the service with jobs in flight: no reply runs
//			   afterwards, and once the destructor has returned and the device
//			   has polled, it holds nothing the renderer made;
//			S5 a shut-down render runner refuses jobs, and destroying the
//			   service then does not wait.
//
//=============================================================================//

#include "hammer/adapters/render/viewport_service.h"
#include "platform/runners/manual_task_runner.h"
#include "platform/runners/thread_task_runner.h"
#include "render/device/null/provider.h"
#include "testing/checks.h"
#include "viewport_fixture.h"

#include <chrono>
#include <thread>
#include <vector>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::ViewJob;
using hammer::render_adapter::ViewportService;
using hammer::render_adapter::ViewportStatus;
using hammer::viewport::ViewKind;

ViewJob TopJob( std::uint32_t width = 64, std::uint32_t height = 48 )
{
	ViewJob job;
	job.kind = ViewKind::Top;
	job.camera2D = TopCamera( int( width ), int( height ) );
	job.pixelWidth = width;
	job.pixelHeight = height;
	return job;
}

// Drains the reply runner until 'ready' holds or two seconds pass.
template <typename Ready> bool DrainUntil( platform::ManualTaskRunner &reply, Ready ready )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 2 );
	while ( !ready() )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		reply.RunUntilIdle();
		std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );
	}
	return true;
}

} // namespace

int main()
{
	testing::Checks checks;
	auto device = render::device::null::Create( {} ).Value();
	const std::size_t baseline = device->LiveResourceCount();
	platform::VirtualClock clock;
	platform::ManualTaskRunner reply( clock );
	Document d;
	Build( d );
	auto scene = std::make_shared<const hammer::viewport::RenderSnapshot>( Snapshot( d ) );

	{
		platform::ThreadTaskRunner render( "viewport-test-render" );
		{
			ViewportService service( *device, render, reply );

			// S1.
			bool arrived = false;
			bool onReply = false;
			std::uint32_t width = 0;
			const std::thread::id test = std::this_thread::get_id();
			checks.That( service.Submit( scene, 1, TopJob(),
			                 [&]( ViewportService::Result result )
			                 {
				                 arrived = true;
				                 onReply = std::this_thread::get_id() == test;
				                 width = result ? result.Value().width : 0;
			                 } ),
			    "S1.a-job-is-accepted" );
			checks.That( DrainUntil( reply,
			                 [&]
			                 {
				                 return arrived;
			                 } ),
			    "S1.the-pixels-arrive" );
			checks.That(
			    onReply && width == 64, "S1.through-the-reply-runner-at-the-requested-size" );

			// S2.
			std::vector<int> order;
			for ( int i = 0; i < 5; ++i )
			{
				(void)service.Submit( scene, 1, TopJob( 32 + i, 24 ),
				    [&order, i]( ViewportService::Result )
				    {
					    order.push_back( i );
				    } );
			}
			checks.That( DrainUntil( reply,
			                 [&]
			                 {
				                 return order.size() == 5;
			                 } ) &&
			                 order == std::vector<int>{ 0, 1, 2, 3, 4 },
			    "S2.jobs-reply-in-submission-order" );

			// S3.
			std::optional<ViewportStatus> noCamera;
			std::optional<ViewportStatus> noScene;
			ViewJob bare = TopJob();
			bare.camera2D.reset();
			(void)service.Submit( scene, 1, bare,
			    [&]( ViewportService::Result result )
			    {
				    noCamera = result ? std::nullopt : std::optional( result.Error() );
			    } );
			(void)service.Submit( nullptr, 1, TopJob(),
			    [&]( ViewportService::Result result )
			    {
				    noScene = result ? std::nullopt : std::optional( result.Error() );
			    } );
			checks.That( DrainUntil( reply,
			                 [&]
			                 {
				                 return noCamera && noScene;
			                 } ) &&
			                 *noCamera == ViewportStatus::kInvalidView &&
			                 *noScene == ViewportStatus::kInvalidView,
			    "S3.invalid-jobs-reply-with-kInvalidView" );

			// S4: jobs in flight at destruction.
			for ( int i = 0; i < 8; ++i )
			{
				(void)service.Submit( scene, 2 + i, TopJob(),
				    [&checks]( ViewportService::Result )
				    {
					    checks.That( false, "S4.no-reply-after-destruction" );
				    } );
			}
		}
		reply.RunUntilIdle();
		// Resources released behind completed tokens go at the device's next poll.
		(void)device->Poll();
		checks.Equal(
		    device->LiveResourceCount(), baseline, "S4.the-renderer-is-gone-with-the-service" );

		// S5.
		render.Shutdown();
		ViewportService late( *device, render, reply );
		checks.That( !late.Submit( scene, 1, TopJob(), {} ), "S5.a-shut-down-runner-refuses-jobs" );
	}
	checks.That( true, "S5.destruction-after-shutdown-does-not-wait" );
	return checks.Report();
}
