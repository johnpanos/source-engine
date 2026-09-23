//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless window system, headless presentation bridge and harness.
//			See headless_presentation.h.
//
//=============================================================================//

#include "headless_presentation.h"

#include <cstdio>

namespace rendertest
{

using namespace render;

// ---------------------------------------------------------------------------
// Window system
// ---------------------------------------------------------------------------

class HeadlessSurface : public IRenderSurface
{
public:
	explicit HeadlessSurface( RenderExtent extent ) : m_Extent( extent ) {}

	RenderExtent GetDrawableExtent() const override
	{
		if ( !m_Visible || m_Status != RenderSurfaceStatus::kAvailable )
			return RenderExtent();
		return m_Extent;
	}
	RenderSurfaceStatus GetStatus() const override { return m_Status; }
	uint64_t GetGeneration() const override { return m_Generation; }

	bool AttachListener( IRenderSurfaceListener &listener ) override
	{
		if ( m_Listener != nullptr )
			return false;
		m_Listener = &listener;
		return true;
	}
	void DetachListener( IRenderSurfaceListener &listener ) override
	{
		if ( m_Listener == &listener )
			m_Listener = nullptr;
	}

	void NotifyReleasing()
	{
		if ( m_Listener )
			m_Listener->OnNativeSurfaceReleasing();
	}

	RenderExtent m_Extent;
	bool m_Visible = true;
	bool m_NativeBound = false;
	RenderSurfaceStatus m_Status = RenderSurfaceStatus::kAvailable;
	uint64_t m_Generation = 1;
	IRenderSurfaceListener *m_Listener = nullptr;
};

HeadlessSurfaces::HeadlessSurfaces() : m_Violations( 0 )
{
}

HeadlessSurfaces::~HeadlessSurfaces()
{
	for ( HeadlessSurface *s : m_Surfaces )
		delete s;
}

IRenderSurface *HeadlessSurfaces::Create( RenderExtent extent )
{
	HeadlessSurface *s = new HeadlessSurface( extent );
	m_Surfaces.push_back( s );
	return s;
}

HeadlessSurface *HeadlessSurfaces::Find( const IRenderSurface &surface ) const
{
	for ( HeadlessSurface *s : m_Surfaces )
		if ( s == &surface )
			return s;
	return nullptr;
}

void HeadlessSurfaces::Resize( IRenderSurface &surface, RenderExtent extent )
{
	if ( HeadlessSurface *s = Find( surface ) )
		s->m_Extent = extent;
}

void HeadlessSurfaces::SetVisible( IRenderSurface &surface, bool visible )
{
	if ( HeadlessSurface *s = Find( surface ) )
		s->m_Visible = visible;
}

void HeadlessSurfaces::ReleaseNativeSurface( IRenderSurface &surface )
{
	HeadlessSurface *s = Find( surface );
	if ( !s || s->m_Status != RenderSurfaceStatus::kAvailable )
		return;
	s->NotifyReleasing();
	if ( s->m_NativeBound )
		++m_Violations;
	s->m_Status = RenderSurfaceStatus::kUnavailable;
}

void HeadlessSurfaces::RestoreNativeSurface( IRenderSurface &surface )
{
	HeadlessSurface *s = Find( surface );
	if ( !s || s->m_Status != RenderSurfaceStatus::kUnavailable )
		return;
	++s->m_Generation;
	s->m_Status = RenderSurfaceStatus::kAvailable;
}

void HeadlessSurfaces::DestroyWindow( IRenderSurface &surface )
{
	HeadlessSurface *s = Find( surface );
	if ( !s || s->m_Status == RenderSurfaceStatus::kDestroyed )
		return;
	if ( s->m_Status == RenderSurfaceStatus::kAvailable )
		s->NotifyReleasing();
	if ( s->m_NativeBound )
		++m_Violations;
	s->m_Status = RenderSurfaceStatus::kDestroyed;
}

void HeadlessSurfaces::Destroy( IRenderSurface *surface )
{
	for ( size_t i = 0; i < m_Surfaces.size(); ++i )
	{
		if ( m_Surfaces[i] == surface )
		{
			delete m_Surfaces[i];
			m_Surfaces.erase( m_Surfaces.begin() + static_cast<std::ptrdiff_t>( i ) );
			return;
		}
	}
}

bool HeadlessSurfaces::Owns( const IRenderSurface &surface ) const
{
	return Find( surface ) != nullptr;
}

void HeadlessSurfaces::SetNativeBinding( const IRenderSurface &surface, bool bound )
{
	if ( HeadlessSurface *s = Find( surface ) )
		s->m_NativeBound = bound;
}

// ---------------------------------------------------------------------------
// Bridge
// ---------------------------------------------------------------------------

namespace
{

// Stands in for "a CPU event was treated as GPU completion" (a defect).
class AlreadyComplete : public IRenderCompletionToken
{
public:
	bool IsComplete() const override { return true; }
};

AlreadyComplete g_AlreadyComplete;

// Defect support: a registration the bridge forgets to remove. It is a live
// object (not the deleted presentation), so the leak is observable, not a crash.
class StaleListener : public IRenderSurfaceListener
{
public:
	void OnNativeSurfaceReleasing() override {}
};

StaleListener g_StaleListener;

class HeadlessBridge;

class HeadlessPresentation : public IRenderPresentation, public IRenderSurfaceListener
{
public:
	HeadlessPresentation( HeadlessBridge &bridge, IRenderDevice &device, IRenderSurface &surface,
	    RenderExtent extent )
	    : m_Bridge( bridge ), m_Device( device ), m_Surface( surface ), m_Extent( extent )
	{
	}

	RenderExtent GetExtent() const override { return m_Extent; }
	bool ResizeTo( RenderExtent extent ) override;
	RenderPresentStatus BeginFrame( RenderResourceHandle *outBackBuffer ) override;
	RenderPresentStatus Present() override;
	RenderPresentStatus CancelFrame() override
	{
		if ( !m_FrameOpen )
			return RenderPresentStatus::kInvalidSequence;
		m_FrameOpen = false;
		return RenderPresentStatus::kOk;
	}

	size_t GetPendingRetirementCount() const override { return m_Retiring.size(); }
	void CollectRetired() override
	{
		// Back buffers are device resources: the device frees them when their
		// tokens complete. The presentation only forgets the ones already freed.
		std::vector<RenderResourceHandle> still;
		for ( RenderResourceHandle h : m_Retiring )
			if ( m_Device.IsResourceLive( h ) )
				still.push_back( h );
		m_Retiring.swap( still );
	}

	void OnNativeSurfaceReleasing() override;

	// Retires every resource behind an ordering submission and detaches.
	void Shutdown( bool leak );

	void SetExtentFromBridge( RenderExtent extent ) { m_Extent = extent; }
	IRenderSurface &Surface() { return m_Surface; }
	IRenderDevice &Device() { return m_Device; }

private:
	void Retire( RenderResourceHandle handle, bool immediately );
	void UnbindNative();

	HeadlessBridge &m_Bridge;
	IRenderDevice &m_Device;
	IRenderSurface &m_Surface;
	RenderExtent m_Extent;
	RenderResourceHandle m_BackBuffer = kInvalidResource;
	RenderExtent m_BackBufferExtent;
	std::vector<RenderResourceHandle> m_Retiring;
	bool m_FrameOpen = false;
	bool m_NativeBound = false;
	uint64_t m_BoundGeneration = 0;
};

class HeadlessBridge : public IRenderPresentationBridgeFactory
{
public:
	HeadlessBridge( IRenderBackendProvider &provider, HeadlessSurfaces &surfaces, uint32_t max,
	    const HeadlessBridgeDefects &defects )
	    : m_Provider( provider ), m_Surfaces( surfaces ), m_Max( max ), m_Defects( defects )
	{
	}

	~HeadlessBridge() override
	{
		for ( HeadlessPresentation *p : m_Live )
			delete p;
	}

	RenderPresentationPairId GetPairId() const override
	{
		RenderPresentationPairId pair;
		pair.windowSystem = "headless";
		pair.renderBackend = m_Provider.GetBackendId().id;
		return pair;
	}

	uint32_t GetMaxPresentations() const override { return m_Max; }

	IRenderPresentation *CreatePresentation( IRenderDevice &device, IRenderSurface &surface,
	    const RenderPresentationConfig &config, RenderCreateError *error ) override
	{
		RenderCreateError local;
		RenderCreateError &err = error ? *error : local;
		err = RenderCreateError();
		if ( ( !m_Defects.acceptForeignDevice && !m_Provider.OwnsDevice( device ) ) ||
		     !m_Surfaces.Owns( surface ) )
			return Fail( err, RenderCreateStatus::kForeignObject, "device or surface is foreign" );
		if ( surface.GetStatus() == RenderSurfaceStatus::kDestroyed )
			return Fail( err, RenderCreateStatus::kSurfaceLost, "the window is destroyed" );
		const RenderDeviceState state = device.GetState();
		if ( state == RenderDeviceState::kDeviceLost || state == RenderDeviceState::kFatal )
			return Fail( err, RenderCreateStatus::kDeviceUnavailable, "the device is lost" );
		if ( !m_Defects.noLimit && m_Live.size() >= m_Max )
			return Fail(
			    err, RenderCreateStatus::kTooManyPresentations, "presentation limit reached" );

		const RenderExtent extent = ( config.extent.width == 0 && config.extent.height == 0 )
		                                ? surface.GetDrawableExtent()
		                                : config.extent;
		HeadlessPresentation *p = new HeadlessPresentation( *this, device, surface, extent );
		if ( !surface.AttachListener( *p ) && !m_Defects.allowSecondPresentation )
		{
			delete p;
			return Fail(
			    err, RenderCreateStatus::kSurfaceBusy, "the surface is already presented" );
		}
		m_Live.push_back( p );
		return p;
	}

	void DestroyPresentation( IRenderPresentation *presentation ) override
	{
		for ( size_t i = 0; i < m_Live.size(); ++i )
		{
			if ( m_Live[i] == presentation )
			{
				m_Live[i]->Shutdown( m_Defects.leakOnDestroy );
				IRenderSurface &surface = m_Live[i]->Surface();
				surface.DetachListener( *m_Live[i] );
				if ( m_Defects.keepListenerAttached )
					surface.AttachListener( g_StaleListener );
				delete m_Live[i];
				m_Live.erase( m_Live.begin() + static_cast<std::ptrdiff_t>( i ) );
				return;
			}
		}
	}

	size_t GetLivePresentationCount() const override { return m_Live.size(); }

	bool ReleaseDevice( IRenderDevice &device ) override
	{
		for ( HeadlessPresentation *p : m_Live )
			if ( &p->Device() == &device && !m_Defects.releaseDeviceWhileLive )
				return false;
		// No native objects: back buffers are device resources the device frees.
		return true;
	}

	// Defect support: one resize moves every presentation.
	void BroadcastExtent( RenderExtent extent )
	{
		for ( HeadlessPresentation *p : m_Live )
			p->SetExtentFromBridge( extent );
	}

	HeadlessSurfaces &Surfaces() { return m_Surfaces; }
	const HeadlessBridgeDefects &Defects() const { return m_Defects; }

private:
	static IRenderPresentation *Fail(
	    RenderCreateError &err, RenderCreateStatus status, const char *message )
	{
		err.status = status;
		std::snprintf( err.message, sizeof( err.message ), "%s", message );
		return nullptr;
	}

	IRenderBackendProvider &m_Provider;
	HeadlessSurfaces &m_Surfaces;
	uint32_t m_Max;
	HeadlessBridgeDefects m_Defects;
	std::vector<HeadlessPresentation *> m_Live;
};

void HeadlessPresentation::Retire( RenderResourceHandle handle, bool immediately )
{
	if ( handle == kInvalidResource )
		return;
	if ( immediately )
	{
		m_Device.DestroyResourceWhenComplete( handle, g_AlreadyComplete );
		m_Device.CollectCompletedDestructions();
		return;
	}
	// An empty submission orders after every earlier use of the resource, and
	// submissions complete in order, so its token completing proves the GPU is
	// done with the resource.
	IRenderCommandContext *ordering = m_Device.CreateCommandContext();
	IRenderCompletionToken *token = ordering ? m_Device.Submit( *ordering ) : nullptr;
	if ( token )
		m_Device.DestroyResourceWhenComplete( handle, *token );
	m_Retiring.push_back( handle );
}

bool HeadlessPresentation::ResizeTo( RenderExtent extent )
{
	const HeadlessBridgeDefects &defects = m_Bridge.Defects();
	if ( m_FrameOpen && !defects.resizeDuringFrame )
		return false;
	CollectRetired();
	if ( defects.sharedExtent )
		m_Bridge.BroadcastExtent( extent );
	m_Extent = extent;
	if ( m_BackBuffer != kInvalidResource && m_BackBufferExtent != extent )
	{
		Retire( m_BackBuffer, defects.recycleOnResize );
		m_BackBuffer = kInvalidResource;
	}
	if ( defects.resizeLosesDevice )
		m_Device.SimulateDeviceLoss();
	return true;
}

void HeadlessPresentation::UnbindNative()
{
	m_NativeBound = false;
	m_Bridge.Surfaces().SetNativeBinding( m_Surface, false );
}

void HeadlessPresentation::OnNativeSurfaceReleasing()
{
	if ( !m_Bridge.Defects().keepNativeAfterRelease )
		UnbindNative();
}

RenderPresentStatus HeadlessPresentation::BeginFrame( RenderResourceHandle *outBackBuffer )
{
	if ( outBackBuffer )
		*outBackBuffer = kInvalidResource;
	if ( m_FrameOpen )
		return RenderPresentStatus::kInvalidSequence;
	CollectRetired();
	const HeadlessBridgeDefects &defects = m_Bridge.Defects();

	const RenderDeviceState state = m_Device.GetState();
	if ( state == RenderDeviceState::kDeviceLost || state == RenderDeviceState::kFatal )
		return RenderPresentStatus::kLost;
	const RenderSurfaceStatus surface = m_Surface.GetStatus();
	if ( surface == RenderSurfaceStatus::kDestroyed && !defects.ignoreSurfaceDestroyed )
		return RenderPresentStatus::kLost;
	if ( surface == RenderSurfaceStatus::kUnavailable && !defects.ignoreSurfaceUnavailable )
		return RenderPresentStatus::kSuspended;
	if ( !m_Extent.IsPresentable() )
		return defects.fatalOnZeroSize ? RenderPresentStatus::kLost
		                               : RenderPresentStatus::kSuspended;
	if ( !m_Surface.GetDrawableExtent().IsPresentable() &&
	     surface == RenderSurfaceStatus::kAvailable )
		return RenderPresentStatus::kSuspended;

	// (Re)build the simulated native binding for the current native surface.
	if ( surface == RenderSurfaceStatus::kAvailable &&
	     ( !m_NativeBound || m_BoundGeneration != m_Surface.GetGeneration() ) )
	{
		m_NativeBound = true;
		m_BoundGeneration = m_Surface.GetGeneration();
		m_Bridge.Surfaces().SetNativeBinding( m_Surface, true );
	}

	if ( m_BackBuffer == kInvalidResource )
	{
		m_BackBuffer = m_Device.CreateResource( RenderResourceType::kTexture );
		m_BackBufferExtent = m_Extent;
		if ( m_BackBuffer == kInvalidResource )
			return RenderPresentStatus::kRecoverable;
	}
	m_FrameOpen = true;
	if ( outBackBuffer )
		*outBackBuffer = m_BackBuffer;
	return RenderPresentStatus::kOk;
}

RenderPresentStatus HeadlessPresentation::Present()
{
	if ( !m_FrameOpen )
		return RenderPresentStatus::kInvalidSequence;
	m_FrameOpen = false;
	// The present itself: a submission that reads the back buffer.
	IRenderCommandContext *context = m_Device.CreateCommandContext();
	if ( !context )
		return RenderPresentStatus::kRecoverable;
	context->RecordUse( m_BackBuffer );
	m_Device.Submit( *context );
	if ( m_Surface.GetStatus() == RenderSurfaceStatus::kDestroyed )
		return RenderPresentStatus::kLost;
	return RenderPresentStatus::kOk;
}

void HeadlessPresentation::Shutdown( bool leak )
{
	if ( !leak )
	{
		Retire( m_BackBuffer, false );
		m_BackBuffer = kInvalidResource;
	}
	UnbindNative();
}

} // namespace

std::unique_ptr<IRenderPresentationBridgeFactory> MakeHeadlessPresentationBridge(
    IRenderBackendProvider &provider, HeadlessSurfaces &surfaces, uint32_t maxPresentations,
    HeadlessBridgeDefects defects )
{
	return std::unique_ptr<IRenderPresentationBridgeFactory>(
	    new HeadlessBridge( provider, surfaces, maxPresentations, defects ) );
}

// ---------------------------------------------------------------------------
// Harness
// ---------------------------------------------------------------------------

IRenderSurface *HeadlessHarness::CreateSurface( RenderExtent extent )
{
	return m_Surfaces.Create( extent );
}

bool HeadlessHarness::ResizeSurface( IRenderSurface &surface, RenderExtent extent )
{
	m_Surfaces.Resize( surface, extent );
	return true;
}

void HeadlessHarness::SetSurfaceVisible( IRenderSurface &surface, bool visible )
{
	m_Surfaces.SetVisible( surface, visible );
}

void HeadlessHarness::ReleaseNativeSurface( IRenderSurface &surface )
{
	m_Surfaces.ReleaseNativeSurface( surface );
}

void HeadlessHarness::RestoreNativeSurface( IRenderSurface &surface )
{
	m_Surfaces.RestoreNativeSurface( surface );
}

void HeadlessHarness::DestroyWindow( IRenderSurface &surface )
{
	m_Surfaces.DestroyWindow( surface );
}

void HeadlessHarness::DestroySurface( IRenderSurface *surface )
{
	m_Surfaces.Destroy( surface );
}

uint32_t HeadlessHarness::GetNativeLifetimeViolations() const
{
	return m_Surfaces.GetNativeLifetimeViolations();
}

bool HeadlessHarness::HoldGpuCompletion( IRenderDevice &device )
{
	m_Held = true;
	m_HeldAt = device.LastCompletedSubmission();
	return true;
}

void HeadlessHarness::ReleaseGpuCompletion( IRenderDevice & )
{
	m_Held = false;
}

bool HeadlessHarness::IsGpuWorkPending( IRenderDevice &device )
{
	// The null device completes work only when polled; the suite never polls
	// while held, so held work is pending exactly while nothing new completed.
	return m_Held && device.LastCompletedSubmission() == m_HeldAt;
}

} // namespace rendertest
