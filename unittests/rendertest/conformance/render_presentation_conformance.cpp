//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the shared presentation-bridge conformance suite.
//			Each obligation of render.presentation.v1 is one or more checks with a
//			stable id. See the header and
//			unittests/rendertest/contracts/render.presentation.v1.md.
//
//=============================================================================//

#include "render_presentation_conformance.h"

#include <string>
#include <vector>

namespace render
{
namespace conformance
{

namespace
{

RenderExtent Extent( uint32_t width, uint32_t height )
{
	RenderExtent extent;
	extent.width = width;
	extent.height = height;
	return extent;
}

std::string ExtentText( const RenderExtent &e )
{
	return std::to_string( e.width ) + "x" + std::to_string( e.height );
}

// A device no provider created, so a bridge must reject it structurally.
class ForeignDevice : public IRenderDevice
{
public:
	const RenderDeviceCaps &GetCapabilities() const override { return m_Caps; }
	RenderDeviceState GetState() const override { return RenderDeviceState::kAvailable; }
	RenderResourceHandle CreateResource( RenderResourceType ) override { return kInvalidResource; }
	bool IsResourceLive( RenderResourceHandle ) const override { return false; }
	size_t GetLiveResourceCount() const override { return 0; }
	void DestroyResourceWhenComplete( RenderResourceHandle, IRenderCompletionToken & ) override {}
	void CollectCompletedDestructions() override {}
	IRenderCommandContext *CreateCommandContext() override { return nullptr; }
	IRenderCompletionToken *Submit( IRenderCommandContext & ) override { return nullptr; }
	uint32_t PollCompletion() override { return 0; }
	uint64_t LastCompletedSubmission() const override { return 0; }
	bool SimulateDeviceLoss() override { return false; }
	bool RecoverDevice() override { return false; }

private:
	RenderDeviceCaps m_Caps;
};

// A surface no window system created.
class ForeignSurface : public IRenderSurface
{
public:
	RenderExtent GetDrawableExtent() const override { return Extent( 64, 64 ); }
	RenderSurfaceStatus GetStatus() const override { return RenderSurfaceStatus::kAvailable; }
	uint64_t GetGeneration() const override { return 1; }
	bool AttachListener( IRenderSurfaceListener & ) override { return true; }
	void DetachListener( IRenderSurfaceListener & ) override {}
};

class ProbeListener : public IRenderSurfaceListener
{
public:
	void OnNativeSurfaceReleasing() override {}
};

// Observes every outstanding submission, then applies deferred destructions.
// Never call while GPU completion is held: a real provider waits for the GPU.
void Drain( IRenderDevice &device, IRenderPresentation *presentation )
{
	while ( device.PollCompletion() > 0 )
	{
	}
	device.CollectCompletedDestructions();
	if ( presentation )
		presentation->CollectRetired();
	device.CollectCompletedDestructions();
}

// Submits the harness's frame content for 'backBuffer' on the device.
bool SubmitFrameWork( IRenderDevice &device, IPresentationHarness &harness,
	RenderResourceHandle backBuffer, uint32_t frameIndex )
{
	IRenderCommandContext *context = device.CreateCommandContext();
	if ( !context )
		return false;
	harness.DrawFrame( device, *context, backBuffer, frameIndex );
	return device.Submit( *context ) != nullptr;
}

// One complete frame. Returns the first non-kOk status, or kOk.
RenderPresentStatus RunFrame( IRenderDevice &device, IRenderPresentation &presentation,
	IPresentationHarness &harness, uint32_t frameIndex, RenderResourceHandle *outBackBuffer )
{
	RenderResourceHandle backBuffer = kInvalidResource;
	const RenderPresentStatus begin = presentation.BeginFrame( &backBuffer );
	if ( outBackBuffer )
		*outBackBuffer = backBuffer;
	if ( begin != RenderPresentStatus::kOk )
		return begin;
	if ( !SubmitFrameWork( device, harness, backBuffer, frameIndex ) )
		return RenderPresentStatus::kLost;
	return presentation.Present();
}

// Frames until one presents, allowing a bounded number of kRecoverable results
// while the presentation rebuilds after a native change.
RenderPresentStatus RunFrameAllowingRebuild( IRenderDevice &device,
	IRenderPresentation &presentation, IPresentationHarness &harness, uint32_t frameIndex )
{
	RenderPresentStatus status = RenderPresentStatus::kRecoverable;
	for ( int attempt = 0; attempt < 3 && status == RenderPresentStatus::kRecoverable; ++attempt )
		status = RunFrame( device, presentation, harness, frameIndex, nullptr );
	return status;
}

IRenderDevice *CreateDevice( IRenderBackendProvider &provider )
{
	RenderDeviceRequest request;
	request.adapterIndex = 0;
	return provider.CreateDevice( request, nullptr );
}

// ---------------------------------------------------------------------------
// Sections
// ---------------------------------------------------------------------------

void CheckPairSelection( IRenderBackendProvider &provider, IRenderPresentationBridgeFactory &bridge,
	Report &report )
{
	const RenderPresentationPairId pair = bridge.GetPairId();
	const RenderBackendId backend = provider.GetBackendId();
	const bool named = pair.windowSystem && pair.renderBackend && pair.windowSystem[0] != '\0';
	report.Record( "pair.identity",
		named && backend.id && contract_detail::EqualStrings( pair.renderBackend, backend.id, 64 ),
		"a bridge names its window system and the backend id of its render provider" );
	if ( !named )
		return;

	IRenderPresentationBridgeFactory *const catalog[] = { &bridge };
	RenderCreateError error;
	report.Record( "pair.select_supported",
		SelectPresentationBridge( catalog, 1, pair.windowSystem, pair.renderBackend, &error ) ==
			&bridge,
		"composition selects the bridge for exactly its pair" );
	RenderCreateError unsupported;
	IRenderPresentationBridgeFactory *other = SelectPresentationBridge(
		catalog, 1, "no-such-window-system", pair.renderBackend, &unsupported );
	report.Record( "pair.select_unsupported",
		other == nullptr && unsupported.status == RenderCreateStatus::kUnsupportedPair,
		"an unsupported pair fails composition with kUnsupportedPair, never a fallback" );
}

void CheckCreation( IRenderDevice &device, IRenderPresentationBridgeFactory &bridge,
	IPresentationHarness &harness, Report &report )
{
	IRenderSurface *surface = harness.CreateSurface( Extent( 320, 240 ) );
	report.Record( "create.surface", surface != nullptr, "the harness opens a window" );
	if ( !surface )
		return;

	RenderPresentationConfig config; // zero extent: follow the surface
	RenderCreateError error;
	IRenderPresentation *presentation = bridge.CreatePresentation( device, *surface, config, &error );
	report.Record( "create.ok",
		presentation != nullptr && error.status == RenderCreateStatus::kOk,
		std::string( "creation succeeds for the bridge's own device and surface: " ) +
			error.message );
	if ( presentation )
	{
		report.Record( "create.extent_from_surface",
			presentation->GetExtent() == surface->GetDrawableExtent(),
			"a zero configured extent takes the surface's drawable extent (" +
				ExtentText( presentation->GetExtent() ) + " vs " +
				ExtentText( surface->GetDrawableExtent() ) + ")" );
		report.Record( "create.live_count", bridge.GetLivePresentationCount() == 1,
			"the bridge counts its live presentations" );

		RenderCreateError busy;
		IRenderPresentation *second = bridge.CreatePresentation( device, *surface, config, &busy );
		report.Record( "create.surface_busy",
			second == nullptr && busy.status == RenderCreateStatus::kSurfaceBusy,
			"a second presentation on one surface fails with kSurfaceBusy" );
		if ( second )
			bridge.DestroyPresentation( second );

		bridge.DestroyPresentation( presentation );
		Drain( device, nullptr );
	}

	ForeignDevice foreignDevice;
	RenderCreateError foreignDeviceError;
	IRenderPresentation *foreign =
		bridge.CreatePresentation( foreignDevice, *surface, config, &foreignDeviceError );
	report.Record( "create.foreign_device",
		foreign == nullptr && foreignDeviceError.status == RenderCreateStatus::kForeignObject,
		"a device from another provider fails with kForeignObject" );
	if ( foreign )
		bridge.DestroyPresentation( foreign );

	ForeignSurface foreignSurface;
	RenderCreateError foreignSurfaceError;
	foreign = bridge.CreatePresentation( device, foreignSurface, config, &foreignSurfaceError );
	report.Record( "create.foreign_surface",
		foreign == nullptr && foreignSurfaceError.status == RenderCreateStatus::kForeignObject,
		"a surface from another window system fails with kForeignObject" );
	if ( foreign )
		bridge.DestroyPresentation( foreign );

	RenderPresentationConfig explicitConfig;
	explicitConfig.extent = Extent( 200, 100 );
	presentation = bridge.CreatePresentation( device, *surface, explicitConfig, nullptr );
	report.Record( "create.explicit_extent",
		presentation != nullptr && presentation->GetExtent() == explicitConfig.extent,
		"an explicit back-buffer extent is honored independently of the window size" );
	if ( presentation )
		bridge.DestroyPresentation( presentation );
	Drain( device, nullptr );

	ProbeListener probe;
	const bool attachable = surface->AttachListener( probe );
	if ( attachable )
		surface->DetachListener( probe );
	report.Record( "lifetime.surface_detached", attachable,
		"destroying a presentation detaches it from its surface" );

	harness.DestroyWindow( *surface );
	harness.DestroySurface( surface );
}

void CheckFrames( IRenderDevice &device, IRenderPresentationBridgeFactory &bridge,
	IPresentationHarness &harness, Report &report )
{
	IRenderSurface *surface = harness.CreateSurface( Extent( 320, 240 ) );
	if ( !surface )
	{
		report.Record( "frame.surface", false, "the harness could not open a window" );
		return;
	}
	const size_t baseResources = device.GetLiveResourceCount();
	RenderPresentationConfig config;
	IRenderPresentation *p = bridge.CreatePresentation( device, *surface, config, nullptr );
	if ( !p )
	{
		report.Record( "frame.create", false, "presentation creation failed" );
		harness.DestroyWindow( *surface );
		harness.DestroySurface( surface );
		return;
	}
	IRenderDevice *const deviceBefore = &device;

	report.Record( "frame.present_without_begin",
		p->Present() == RenderPresentStatus::kInvalidSequence,
		"Present with no open frame reports kInvalidSequence" );

	RenderResourceHandle backBuffer = kInvalidResource;
	const RenderPresentStatus begin = p->BeginFrame( &backBuffer );
	report.Record( "frame.begin",
		begin == RenderPresentStatus::kOk && device.IsResourceLive( backBuffer ),
		"BeginFrame opens a frame and hands out a live back-buffer resource" );
	RenderResourceHandle again = kInvalidResource;
	report.Record( "frame.begin_twice",
		p->BeginFrame( &again ) == RenderPresentStatus::kInvalidSequence,
		"BeginFrame with a frame already open reports kInvalidSequence" );

	const RenderExtent before = p->GetExtent();
	const bool resizedDuringFrame = p->ResizeTo( Extent( 500, 400 ) );
	report.Record( "resize.rejected_during_frame",
		!resizedDuringFrame && p->GetExtent() == before && device.IsResourceLive( backBuffer ),
		"ResizeTo during an open frame is rejected and changes nothing" );

	SubmitFrameWork( device, harness, backBuffer, 0 );
	report.Record( "frame.present", p->Present() == RenderPresentStatus::kOk,
		"presenting the open frame succeeds" );

	// Repeated runtime resize, including orientation changes, with frames between.
	const RenderExtent sizes[] = { Extent( 640, 480 ), Extent( 800, 600 ), Extent( 480, 640 ),
		Extent( 1024, 576 ), Extent( 576, 1024 ), Extent( 333, 177 ), Extent( 1280, 720 ),
		Extent( 320, 240 ) };
	bool allResized = true, allPresented = true, deviceKept = true;
	std::string firstFailure;
	uint32_t frame = 1;
	for ( const RenderExtent &size : sizes )
	{
		if ( !p->ResizeTo( size ) || p->GetExtent() != size )
		{
			allResized = false;
			if ( firstFailure.empty() )
				firstFailure = "resize to " + ExtentText( size );
		}
		for ( int i = 0; i < 2; ++i )
		{
			const RenderPresentStatus status = RunFrameAllowingRebuild( device, *p, harness, frame++ );
			if ( status != RenderPresentStatus::kOk )
			{
				allPresented = false;
				if ( firstFailure.empty() )
					firstFailure = "frame at " + ExtentText( size );
			}
		}
		if ( &device != deviceBefore || device.GetState() != RenderDeviceState::kAvailable )
			deviceKept = false;
	}
	report.Record( "resize.repeated", allResized && allPresented, firstFailure );
	report.Record( "resize.keeps_device", deviceKept,
		"resizes and orientation changes never lose or recreate the device" );

	// The window itself changes size (the swapchain follows the drawable).
	const bool windowResized = harness.ResizeSurface( *surface, Extent( 400, 300 ) );
	RenderPresentStatus afterWindow = RenderPresentStatus::kLost;
	for ( int i = 0; i < 3; ++i )
		afterWindow = RunFrameAllowingRebuild( device, *p, harness, frame++ );
	const bool aspectResized = harness.ResizeSurface( *surface, Extent( 240, 400 ) );
	RenderPresentStatus afterAspect = RenderPresentStatus::kLost;
	for ( int i = 0; i < 3; ++i )
		afterAspect = RunFrameAllowingRebuild( device, *p, harness, frame++ );
	report.Record( "resize.window_driven",
		windowResized && aspectResized && afterWindow == RenderPresentStatus::kOk &&
			afterAspect == RenderPresentStatus::kOk &&
			device.GetState() == RenderDeviceState::kAvailable,
		"presentation follows window resizes and aspect changes without losing the device" );

	// Zero-area back buffer: suspend without opening a frame, then resume.
	p->ResizeTo( Extent( 0, 0 ) );
	RenderResourceHandle none = kInvalidResource;
	const RenderPresentStatus zero = p->BeginFrame( &none );
	report.Record( "zero.backbuffer_suspends",
		zero == RenderPresentStatus::kSuspended &&
			p->Present() == RenderPresentStatus::kInvalidSequence &&
			device.GetState() != RenderDeviceState::kFatal,
		"a zero-area back buffer suspends presentation non-fatally without opening a frame" );
	p->ResizeTo( Extent( 640, 480 ) );
	report.Record( "zero.resume_after_backbuffer",
		RunFrameAllowingRebuild( device, *p, harness, frame++ ) == RenderPresentStatus::kOk,
		"presentation resumes once the back buffer is presentable" );

	// Hidden/minimized window: zero drawable extent.
	harness.SetSurfaceVisible( *surface, false );
	const RenderPresentStatus hidden = p->BeginFrame( &none );
	if ( hidden == RenderPresentStatus::kOk )
	{
		SubmitFrameWork( device, harness, none, frame++ );
		p->Present();
	}
	report.Record( "zero.surface_suspends",
		surface->GetDrawableExtent().IsPresentable() ? hidden == RenderPresentStatus::kOk
												 : hidden == RenderPresentStatus::kSuspended,
		"a zero-area (hidden or minimized) surface suspends presentation" );
	harness.SetSurfaceVisible( *surface, true );
	RenderPresentStatus shown = RenderPresentStatus::kSuspended;
	for ( int i = 0; i < 3 && shown != RenderPresentStatus::kOk; ++i )
		shown = RunFrameAllowingRebuild( device, *p, harness, frame++ );
	report.Record( "zero.resume_after_surface", shown == RenderPresentStatus::kOk,
		"presentation resumes when the surface is shown again" );

	// The platform takes the native surface away and later provides a new one.
	const uint64_t generation = surface->GetGeneration();
	harness.ReleaseNativeSurface( *surface );
	const RenderPresentStatus away = p->BeginFrame( &none );
	report.Record( "surface.unavailable_suspends",
		surface->GetStatus() == RenderSurfaceStatus::kUnavailable &&
			away == RenderPresentStatus::kSuspended &&
			device.GetState() == RenderDeviceState::kAvailable,
		"an unavailable native surface suspends presentation and keeps the device" );
	harness.RestoreNativeSurface( *surface );
	const RenderPresentStatus back = RunFrameAllowingRebuild( device, *p, harness, frame++ );
	report.Record( "surface.replaced_recovers",
		surface->GetGeneration() != generation && back == RenderPresentStatus::kOk,
		"presentation rebuilds against a replacement native surface" );

	bridge.DestroyPresentation( p );
	Drain( device, nullptr );
	report.Record( "lifetime.device_resources_baseline",
		device.GetLiveResourceCount() == baseResources && bridge.GetLivePresentationCount() == 0,
		"after destruction and GPU completion the presentation holds no device resources (" +
			std::to_string( device.GetLiveResourceCount() ) + " vs " +
			std::to_string( baseResources ) + ")" );

	harness.DestroyWindow( *surface );
	harness.DestroySurface( surface );
}

// Storage replaced or released while the GPU still uses it must stay live until
// the device's completion token for that use reports complete.
void CheckDelayedCompletion( IRenderDevice &device, IRenderPresentationBridgeFactory &bridge,
	IPresentationHarness &harness, Report &report )
{
	IRenderSurface *surface = harness.CreateSurface( Extent( 320, 240 ) );
	if ( !surface )
	{
		report.Record( "completion.surface", false, "the harness could not open a window" );
		return;
	}
	Drain( device, nullptr );
	const size_t baseResources = device.GetLiveResourceCount();
	RenderPresentationConfig config;
	IRenderPresentation *p = bridge.CreatePresentation( device, *surface, config, nullptr );
	if ( !p || RunFrame( device, *p, harness, 0, nullptr ) != RenderPresentStatus::kOk )
	{
		report.Record( "completion.setup", false, "could not present an initial frame" );
		if ( p )
			bridge.DestroyPresentation( p );
		Drain( device, nullptr );
		harness.DestroyWindow( *surface );
		harness.DestroySurface( surface );
		return;
	}
	Drain( device, p );

	const bool held = harness.HoldGpuCompletion( device );
	report.Record( "completion.hold", held,
		"the pair can hold GPU completion (required to test early reuse)" );
	if ( !held )
	{
		bridge.DestroyPresentation( p );
		Drain( device, nullptr );
		harness.DestroyWindow( *surface );
		harness.DestroySurface( surface );
		return;
	}

	// A frame whose GPU work cannot complete yet, then a resize that replaces its
	// back buffer (and, natively, its swapchain).
	RenderResourceHandle oldBackBuffer = kInvalidResource;
	const RenderPresentStatus heldFrame = RunFrame( device, *p, harness, 1, &oldBackBuffer );
	const bool resized = p->ResizeTo( Extent( 400, 200 ) );
	device.CollectCompletedDestructions();
	p->CollectRetired();
	device.CollectCompletedDestructions();
	const bool gpuPending = harness.IsGpuWorkPending( device );
	report.Record( "completion.retired_until_complete",
		heldFrame == RenderPresentStatus::kOk && resized && gpuPending &&
			device.IsResourceLive( oldBackBuffer ) && p->GetPendingRetirementCount() > 0,
		"a back buffer replaced while its GPU work is incomplete stays live and pending" );

	// Destroying the presentation with incomplete work defers its storage too.
	RenderResourceHandle newBackBuffer = kInvalidResource;
	( void )newBackBuffer;
	bridge.DestroyPresentation( p );
	p = nullptr;
	device.CollectCompletedDestructions();
	report.Record( "completion.destroy_defers",
		harness.IsGpuWorkPending( device ) && device.IsResourceLive( oldBackBuffer ) &&
			bridge.GetLivePresentationCount() == 0,
		"destroying a presentation never frees storage the GPU still uses" );

	harness.ReleaseGpuCompletion( device );
	Drain( device, nullptr );
	report.Record( "completion.released_after_complete",
		!device.IsResourceLive( oldBackBuffer ) && device.GetLiveResourceCount() == baseResources,
		"once the GPU completes, retired storage is released and resources return to baseline" );

	harness.DestroyWindow( *surface );
	harness.DestroySurface( surface );
}

void CheckMultipleSurfaces( IRenderDevice &device, IRenderPresentationBridgeFactory &bridge,
	IPresentationHarness &harness, Report &report )
{
	const uint32_t limit = bridge.GetMaxPresentations();
	const uint32_t count = limit < harness.GetMaxSurfaces() ? limit : harness.GetMaxSurfaces();
	if ( count < 2 )
	{
		report.Record( "multi.available", false,
			"multi-window coverage needs a bridge and harness supporting two surfaces" );
		return;
	}

	std::vector<IRenderSurface *> surfaces;
	std::vector<IRenderPresentation *> live;
	bool allCreated = true;
	for ( uint32_t i = 0; i < count; ++i )
	{
		IRenderSurface *s = harness.CreateSurface( Extent( 200 + 40 * i, 150 + 20 * i ) );
		if ( !s )
		{
			allCreated = false;
			break;
		}
		surfaces.push_back( s );
		RenderPresentationConfig config;
		IRenderPresentation *p = bridge.CreatePresentation( device, *s, config, nullptr );
		if ( !p )
			allCreated = false;
		live.push_back( p );
	}
	report.Record( "multi.create", allCreated && bridge.GetLivePresentationCount() == count,
		"one presentation per window up to the bridge limit" );
	if ( !allCreated )
	{
		for ( IRenderPresentation *p : live )
			if ( p )
				bridge.DestroyPresentation( p );
		Drain( device, nullptr );
		for ( IRenderSurface *s : surfaces )
		{
			harness.DestroyWindow( *s );
			harness.DestroySurface( s );
		}
		return;
	}

	if ( count == limit && harness.GetMaxSurfaces() > limit )
	{
		IRenderSurface *extra = harness.CreateSurface( Extent( 64, 64 ) );
		RenderCreateError error;
		IRenderPresentation *over =
			extra ? bridge.CreatePresentation( device, *extra, RenderPresentationConfig(), &error )
				  : nullptr;
		report.Record( "multi.over_limit",
			extra && over == nullptr && error.status == RenderCreateStatus::kTooManyPresentations,
			"a presentation beyond the bridge limit fails with kTooManyPresentations" );
		if ( over )
			bridge.DestroyPresentation( over );
		if ( extra )
		{
			harness.DestroyWindow( *extra );
			harness.DestroySurface( extra );
		}
	}

	// Interleaved frames; resizing one presentation leaves the others alone.
	std::vector<RenderExtent> before;
	for ( IRenderPresentation *p : live )
		before.push_back( p->GetExtent() );
	live[0]->ResizeTo( Extent( 512, 256 ) );
	bool othersKept = live[0]->GetExtent() == Extent( 512, 256 );
	for ( size_t i = 1; i < live.size(); ++i )
		othersKept = othersKept && live[i]->GetExtent() == before[i];
	report.Record( "multi.independent_resize", othersKept,
		"resizing one presentation does not change another's extent" );

	bool interleaved = true;
	for ( uint32_t frame = 0; frame < 3; ++frame )
		for ( IRenderPresentation *p : live )
			interleaved = interleaved &&
						  RunFrameAllowingRebuild( device, *p, harness, frame ) ==
							  RenderPresentStatus::kOk;
	report.Record( "multi.interleaved_frames", interleaved,
		"frames interleave across presentations on one device" );

	// Destroy in a non-creation order: the middle window first, while others run.
	const size_t middle = live.size() / 2;
	bridge.DestroyPresentation( live[middle] );
	live[middle] = nullptr;
	harness.DestroyWindow( *surfaces[middle] );
	bool survivorsPresent = true;
	for ( IRenderPresentation *p : live )
		if ( p )
			survivorsPresent = survivorsPresent &&
							   RunFrameAllowingRebuild( device, *p, harness, 9 ) ==
								   RenderPresentStatus::kOk;
	report.Record( "multi.destroy_order", survivorsPresent,
		"destroying one presentation and its window leaves the others presenting" );

	// A window destroyed before its presentation: the presentation reports lost
	// and can still be destroyed; others continue.
	const size_t last = live.size() - 1;
	RenderPresentStatus lost = RenderPresentStatus::kOk;
	if ( last != middle )
	{
		harness.DestroyWindow( *surfaces[last] );
		RenderResourceHandle none = kInvalidResource;
		lost = live[last]->BeginFrame( &none );
		RenderCreateError error;
		IRenderPresentation *onDestroyed = bridge.CreatePresentation( device, *surfaces[last],
			RenderPresentationConfig(), &error );
		report.Record( "loss.create_on_destroyed",
			onDestroyed == nullptr && error.status == RenderCreateStatus::kSurfaceLost,
			"creating a presentation on a destroyed window fails with kSurfaceLost" );
		if ( onDestroyed )
			bridge.DestroyPresentation( onDestroyed );
	}
	report.Record( "loss.window_destroyed",
		lost == RenderPresentStatus::kLost && device.GetState() == RenderDeviceState::kAvailable,
		"a presentation whose window is destroyed reports kLost without losing the device" );
	bool othersAfterLoss = true;
	for ( size_t i = 0; i < live.size(); ++i )
		if ( live[i] && i != last )
			othersAfterLoss = othersAfterLoss &&
							  RunFrameAllowingRebuild( device, *live[i], harness, 10 ) ==
								  RenderPresentStatus::kOk;
	report.Record( "loss.others_unaffected", othersAfterLoss,
		"losing one window's surface leaves other presentations working" );

	for ( size_t i = live.size(); i-- > 0; )
		if ( live[i] )
			bridge.DestroyPresentation( live[i] );
	Drain( device, nullptr );
	for ( size_t i = 0; i < surfaces.size(); ++i )
	{
		if ( i != middle && i != last )
			harness.DestroyWindow( *surfaces[i] );
		harness.DestroySurface( surfaces[i] );
	}
	report.Record( "lifetime.no_leaked_presentations", bridge.GetLivePresentationCount() == 0,
		"the bridge owns no presentations after all are destroyed" );
}

void CheckDeviceLoss( IRenderBackendProvider &provider, IRenderPresentationBridgeFactory &bridge,
	IPresentationHarness &harness, Report &report )
{
	if ( !provider.GetProviderCaps().supportsDeviceLossRecovery )
		return;
	IRenderDevice *device = CreateDevice( provider );
	IRenderSurface *surface = harness.CreateSurface( Extent( 256, 256 ) );
	if ( !device || !surface )
	{
		report.Record( "loss.device_setup", false, "could not create a device and window" );
		if ( device )
			provider.DestroyDevice( device );
		if ( surface )
		{
			harness.DestroyWindow( *surface );
			harness.DestroySurface( surface );
		}
		return;
	}
	IRenderPresentation *p =
		bridge.CreatePresentation( *device, *surface, RenderPresentationConfig(), nullptr );
	const bool presented =
		p && RunFrame( *device, *p, harness, 0, nullptr ) == RenderPresentStatus::kOk;
	Drain( *device, p );

	const bool simulated = device->SimulateDeviceLoss();
	RenderResourceHandle none = kInvalidResource;
	const RenderPresentStatus lost = p ? p->BeginFrame( &none ) : RenderPresentStatus::kOk;
	report.Record( "loss.device_lost",
		presented && simulated && lost == RenderPresentStatus::kLost,
		"presentation reports kLost while its device is lost" );
	if ( p )
		bridge.DestroyPresentation( p );

	RenderCreateError error;
	IRenderPresentation *during =
		bridge.CreatePresentation( *device, *surface, RenderPresentationConfig(), &error );
	report.Record( "loss.create_during_loss",
		during == nullptr && error.status == RenderCreateStatus::kDeviceUnavailable,
		"a presentation cannot be created on a lost device" );
	if ( during )
		bridge.DestroyPresentation( during );

	const bool recovered = device->RecoverDevice();
	IRenderPresentation *after =
		recovered ? bridge.CreatePresentation( *device, *surface, RenderPresentationConfig(), nullptr )
				  : nullptr;
	report.Record( "loss.recreate_after_recovery",
		after != nullptr &&
			RunFrame( *device, *after, harness, 1, nullptr ) == RenderPresentStatus::kOk,
		"after recovery a new presentation presents on the same surface" );
	if ( after )
		bridge.DestroyPresentation( after );
	Drain( *device, nullptr );
	harness.DestroyWindow( *surface );
	harness.DestroySurface( surface );
	bridge.ReleaseDevice( *device );
	provider.DestroyDevice( device );
}

} // namespace

bool RunPresentationConformance( IRenderBackendProvider &provider,
	IRenderPresentationBridgeFactory &bridge, IPresentationHarness &harness, Report &report )
{
	CheckPairSelection( provider, bridge, report );

	IRenderDevice *device = CreateDevice( provider );
	report.Record( "setup.device", device != nullptr, "the provider creates a device" );
	if ( !device )
		return report.Passed();

	CheckCreation( *device, bridge, harness, report );
	CheckFrames( *device, bridge, harness, report );
	CheckDelayedCompletion( *device, bridge, harness, report );
	CheckMultipleSurfaces( *device, bridge, harness, report );
	Drain( *device, nullptr );

	// Destruction order: a live presentation pins the device; once none remain,
	// the bridge releases what it still holds and the device can go.
	IRenderSurface *pin = harness.CreateSurface( Extent( 128, 128 ) );
	IRenderPresentation *pinned =
		pin ? bridge.CreatePresentation( *device, *pin, RenderPresentationConfig(), nullptr )
			: nullptr;
	if ( pinned )
		RunFrame( *device, *pinned, harness, 0, nullptr );
	const bool refused = pinned != nullptr && !bridge.ReleaseDevice( *device );
	if ( pinned )
		bridge.DestroyPresentation( pinned );
	const bool released = bridge.ReleaseDevice( *device );
	report.Record( "lifetime.release_device_order", refused && released,
		"ReleaseDevice refuses while a presentation uses the device, then succeeds" );
	Drain( *device, nullptr );
	if ( pin )
	{
		harness.DestroyWindow( *pin );
		harness.DestroySurface( pin );
	}
	provider.DestroyDevice( device );

	CheckDeviceLoss( provider, bridge, harness, report );

	report.Record( "lifetime.native_released_before_window",
		harness.GetNativeLifetimeViolations() == 0,
		"no native presentation object outlives the window it was built on (" +
			std::to_string( harness.GetNativeLifetimeViolations() ) + " violation(s))" );
	report.Record( "lifetime.no_leaked_devices", provider.GetLiveDeviceCount() == 0,
		"every device the suite created is destroyed after its presentations" );
	return report.Passed();
}

} // namespace conformance
} // namespace render
