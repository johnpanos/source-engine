//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.service.vulkan (RFC 0002 R17;
//			hammer.adapters.render; contract render_adapter.viewport-geometry.v1,
//			clauses SV1-SV4; linux-native-vulkan-gpu): ViewportService on
//			render.device.vulkan, as the GTK shell composes it, judged against
//			frames a lone renderer draws of the same inputs.
//
//			SV1 sharing: two documents, each with its own service and render
//			    thread, on one device; their four views each, submitted
//			    interleaved, equal what a lone renderer draws of the same view;
//			SV2 restoration after a remount: a replaced material source (the
//			    same textures) gives the frame drawn before it;
//			SV3 restoration after a resize: a view drawn smaller and then at
//			    its size again equals its first frame;
//			SV4 teardown: a service destroyed with jobs in flight, exported
//			    frames still leased and a source swap queued leaves the device
//			    with what it held before the service existed.
//
//=============================================================================//

#include "hammer/adapters/render/viewport_service.h"
#include "platform/runners/manual_task_runner.h"
#include "platform/runners/thread_task_runner.h"
#include "render/device/vulkan/provider.h"
#include "testing/checks.h"
#include "viewport_fixture.h"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::IMaterialTextures;
using hammer::render_adapter::MaterialImage;
using hammer::render_adapter::ViewJob;
using hammer::render_adapter::ViewPixels;
using hammer::render_adapter::ViewportRenderer;
using hammer::render_adapter::ViewportService;
using hammer::render_adapter::ViewRequest;
using hammer::viewport::ViewKind;

constexpr std::uint32_t kWidth = 256;
constexpr std::uint32_t kHeight = 192;

// A flat 16x16 texture for every material.
class FlatTextures final : public IMaterialTextures
{
public:
	std::optional<MaterialImage> BaseTexture( const std::string & ) override
	{
		MaterialImage image;
		image.width = image.height = 16;
		for ( int i = 0; i < 16 * 16; ++i )
			image.rgba.insert( image.rgba.end(), { 190, 120, 60, 255 } );
		return image;
	}
};

struct View
{
	ViewKind kind;
	hammer::viewport::Camera2D camera2D;
	hammer::viewport::Camera3D camera3D;
};

std::vector<View> Views( std::uint32_t width, std::uint32_t height )
{
	std::vector<View> views;
	for ( ViewKind kind : { ViewKind::Camera3D, ViewKind::Top, ViewKind::Front, ViewKind::Side } )
	{
		View view{ kind, TopCamera( int( width ), int( height ) ),
		    EyeCamera( int( width ), int( height ) ) };
		view.camera2D.SetKind( kind == ViewKind::Camera3D ? ViewKind::Top : kind );
		views.push_back( view );
	}
	return views;
}

ViewJob JobOf( const View &view, std::uint32_t width, std::uint32_t height, bool external = false )
{
	ViewJob job;
	job.kind = view.kind;
	if ( view.kind == ViewKind::Camera3D )
		job.camera3D = view.camera3D;
	else
		job.camera2D = view.camera2D;
	job.pixelWidth = width;
	job.pixelHeight = height;
	job.external = external;
	return job;
}

// What a lone renderer draws of the view.
std::vector<std::uint8_t> Solo( render::device::IRenderDevice2 &device,
    const hammer::viewport::RenderSnapshot &scene, const View &view, IMaterialTextures *textures )
{
	auto renderer = ViewportRenderer::Create( device, textures );
	if ( !renderer || !renderer.Value()->SetScene( scene, 1 ) )
		return {};
	ViewRequest request;
	request.kind = view.kind;
	request.camera2D = &view.camera2D;
	request.camera3D = &view.camera3D;
	request.pixelWidth = kWidth;
	request.pixelHeight = kHeight;
	auto pixels = renderer.Value()->RenderAndWait( request );
	return pixels ? pixels.Value().rgba : std::vector<std::uint8_t>();
}

template <typename Ready> bool DrainUntil( platform::ManualTaskRunner &reply, Ready ready )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !ready() )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		reply.RunUntilIdle();
		std::this_thread::sleep_for( std::chrono::microseconds( 200 ) );
	}
	return true;
}

// Submits one job and waits for its reply on 'reply'.
std::optional<ViewPixels> Frame( ViewportService &service, platform::ManualTaskRunner &reply,
    std::shared_ptr<const hammer::viewport::RenderSnapshot> scene, ViewJob job )
{
	std::optional<ViewPixels> out;
	bool done = false;
	if ( !service.Submit( std::move( scene ), 1, std::move( job ),
	         [&]( ViewportService::Result result )
	         {
		         done = true;
		         if ( result )
			         out = std::move( result ).Value();
	         } ) )
		return std::nullopt;
	if ( !DrainUntil( reply,
	         [&]
	         {
		         return done;
	         } ) )
		return std::nullopt;
	return out;
}

} // namespace

int main()
{
	testing::Checks checks;
	namespace vulkan = render::device::vulkan;
	vulkan::VulkanAdapterOptions options;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	auto created = vulkan::Create( options );
	if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
		return checks.Report();
	std::unique_ptr<render::device::IRenderDevice2> device = std::move( created ).Value();
	(void)device->Poll();
	const std::size_t baseline = device->LiveResourceCount();

	Document first;
	Build( first );
	Document second;
	Build( second );
	{
		// The second document is another map: one more box.
		hammer::scene::DocumentEdit edit( second.doc );
		hammer::scene::FaceTexture texture;
		texture.material = "DEV/DEV_MEASUREGENERIC01B";
		(void)edit.Add( hammer::scene::MakeBoxSolid(
		    hammer::scene::Box{ Vec3d( 32, 96, 0 ), Vec3d( 96, 160, 96 ) }, texture ) );
		hammer::scene::CommitEdit( second.doc, edit );
	}
	auto sceneA = std::make_shared<const hammer::viewport::RenderSnapshot>( Snapshot( first ) );
	auto sceneB =
	    std::make_shared<const hammer::viewport::RenderSnapshot>( Snapshot( second, false ) );
	const std::vector<View> views = Views( kWidth, kHeight );
	FlatTextures soloTextures;
	platform::VirtualClock clock;
	platform::ManualTaskRunner reply( clock );

	// SV1.
	{
		platform::ThreadTaskRunner renderA( "viewport-a" );
		platform::ThreadTaskRunner renderB( "viewport-b" );
		ViewportService a( *device, renderA, reply, std::make_unique<FlatTextures>() );
		ViewportService b( *device, renderB, reply );
		std::vector<std::optional<ViewPixels>> framesA( views.size() );
		std::vector<std::optional<ViewPixels>> framesB( views.size() );
		std::size_t arrived = 0;
		for ( std::size_t i = 0; i < views.size(); ++i )
		{
			(void)a.Submit( sceneA, 1, JobOf( views[i], kWidth, kHeight ),
			    [&, i]( ViewportService::Result result )
			    {
				    ++arrived;
				    if ( result )
					    framesA[i] = std::move( result ).Value();
			    } );
			(void)b.Submit( sceneB, 1, JobOf( views[i], kWidth, kHeight ),
			    [&, i]( ViewportService::Result result )
			    {
				    ++arrived;
				    if ( result )
					    framesB[i] = std::move( result ).Value();
			    } );
		}
		checks.That( DrainUntil( reply,
		                 [&]
		                 {
			                 return arrived == 2 * views.size();
		                 } ),
		    "SV1.every-view-of-both-documents-arrives" );
		bool equalA = true;
		bool equalB = true;
		bool differ = false;
		for ( std::size_t i = 0; i < views.size(); ++i )
		{
			const auto soloA = Solo( *device, *sceneA, views[i], &soloTextures );
			const auto soloB = Solo( *device, *sceneB, views[i], nullptr );
			equalA = equalA && framesA[i] && !soloA.empty() && framesA[i]->rgba == soloA;
			equalB = equalB && framesB[i] && !soloB.empty() && framesB[i]->rgba == soloB;
			differ = differ || ( framesA[i] && framesB[i] && framesA[i]->rgba != framesB[i]->rgba );
		}
		checks.That( equalA, "SV1.the-first-documents-views-equal-a-lone-renderers" );
		checks.That( equalB, "SV1.the-second-documents-views-equal-a-lone-renderers" );
		checks.That( differ, "SV1.the-documents-show-different-content" );

		// SV2.
		const auto before = Frame( a, reply, sceneA, JobOf( views[0], kWidth, kHeight ) );
		checks.That( a.SetMaterialSource( std::make_unique<FlatTextures>() ), "SV2.remounted" );
		const auto after = Frame( a, reply, sceneA, JobOf( views[0], kWidth, kHeight ) );
		checks.That( before && after && before->rgba == after->rgba,
		    "SV2.a-remount-restores-the-frame-drawn-before" );

		// SV3.
		const auto small = Frame( a, reply, sceneA, JobOf( views[1], kWidth / 2, kHeight / 2 ) );
		const auto again = Frame( a, reply, sceneA, JobOf( views[1], kWidth, kHeight ) );
		checks.That( small && small->width == kWidth / 2 && again && framesA[1] &&
		                 again->rgba == framesA[1]->rgba,
		    "SV3.a-view-resized-and-back-equals-its-first-frame" );
	}
	(void)device->WaitIdle();
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "SV1.both-services-leave-nothing-behind" );

	// SV4.
	{
		platform::ThreadTaskRunner render( "viewport-teardown" );
		auto service = std::make_unique<ViewportService>(
		    *device, render, reply, std::make_unique<FlatTextures>() );
		const bool exports = device->ExternalImages() != nullptr;
		std::vector<std::uint64_t> leases;
		for ( int i = 0; i < 3; ++i )
		{
			(void)service->Submit( sceneA, 1, JobOf( views[0], kWidth, kHeight, exports ),
			    [&]( ViewportService::Result result )
			    {
				    if ( result && result.Value().external )
					    leases.push_back( result.Value().external->lease ); // never returned
			    } );
		}
		(void)DrainUntil( reply,
		    [&]
		    {
			    return !exports || leases.size() == 3;
		    } );
		for ( std::size_t i = 0; i < views.size(); ++i ) // in flight at destruction
			(void)service->Submit(
			    sceneB, 2, JobOf( views[i], kWidth, kHeight, exports ), []( auto ) {} );
		(void)service->SetMaterialSource( std::make_unique<FlatTextures>() );
		service.reset();
		reply.RunUntilIdle();
		checks.That( !exports || leases.size() == 3, "SV4.exported-frames-were-leased" );
	}
	(void)device->WaitIdle();
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline,
	    "SV4.teardown-with-frames-in-flight-and-leased-leaves-nothing" );
	return checks.Report();
}
