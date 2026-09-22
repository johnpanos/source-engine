//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the shared render-backend conformance suite. Each
//			obligation from RFC 0001 "Render backend conformance tests" is one or
//			more recorded checks with a stable id. See the header and
//			unittests/rendertest/contracts/render.backend.v1.md.
//
//=============================================================================//

#include "render_backend_conformance.h"

#include <string>

namespace render::conformance
{

namespace
{

// Picks a feature the given adapter does NOT advertise, so we can build an
// unsatisfiable required-feature request. kNeverSupported is reserved for this.
RenderFeature UnsupportedFeatureFor( const RenderAdapterInfo &adapter )
{
	const RenderFeature candidates[] = {
		RenderFeature::kNeverSupported,
		RenderFeature::kComputeShaders,
		RenderFeature::kMultiSample4x,
		RenderFeature::kRuntimeShaderCompile,
	};
	for ( RenderFeature f : candidates )
	{
		if ( !adapter.supportedFeatures.Has( f ) )
			return f;
	}
	return RenderFeature::kNeverSupported;
}

// ---------------------------------------------------------------------------
// Sections
// ---------------------------------------------------------------------------

void CheckIdentity( IRenderBackendProvider &provider, Report &report )
{
	const RenderBackendId id = provider.GetBackendId();
	report.Record( "identity.id", id.id != nullptr && id.id[0] != '\0',
		"backend id token must be a non-empty stable string" );
	report.Record( "identity.name", id.name != nullptr && id.name[0] != '\0',
		"backend name must be non-empty" );
}

// Returns true and fills 'adapter0' when at least one adapter enumerates.
bool CheckAdapterEnumeration( IRenderBackendProvider &provider, Report &report,
	RenderAdapterInfo &adapter0 )
{
	const int count = provider.GetAdapterCount();
	report.Record( "adapter.count", count >= 1,
		"a provider must advertise at least one adapter" );
	if ( count < 1 )
		return false;

	RenderAdapterInfo first;
	const bool got = provider.GetAdapterInfo( 0, &first );
	report.Record( "adapter.query_valid", got, "GetAdapterInfo(0) must succeed" );
	if ( !got )
		return false;

	// Stability / value semantics: a second query yields an equal value, and the
	// first copy is unchanged by it (no pointer into transient enumeration state).
	RenderAdapterInfo again;
	const bool got2 = provider.GetAdapterInfo( 0, &again );
	const bool stable = got2 && first == again;
	report.Record( "adapter.stable_description", stable,
		"repeated adapter descriptions must be equal values" );

	// Invalid indices are rejected and do not mutate the caller's buffer.
	RenderAdapterInfo probe;
	probe.vendorId = 0xDEADBEEFu; // sentinel a real adapter never reports here
	const RenderAdapterInfo untouchedSentinel = probe;
	const bool badHigh = provider.GetAdapterInfo( count, &probe );
	const bool badNeg = provider.GetAdapterInfo( -1, &probe );
	const bool untouched = probe == untouchedSentinel;
	report.Record( "adapter.invalid_index_rejected", !badHigh && !badNeg && untouched,
		"out-of-range adapter indices must return false and not write 'out'" );

	adapter0 = first;
	return true;
}

void CheckStructuredFailure( IRenderBackendProvider &provider, Report &report,
	const RenderAdapterInfo &adapter0 )
{
	// Invalid adapter index -> structured kInvalidAdapter, no device.
	{
		RenderDeviceRequest req;
		req.adapterIndex = provider.GetAdapterCount(); // one past the end
		RenderCreateError err;
		IRenderDevice *dev = provider.CreateDevice( req, &err );
		const bool ok = dev == nullptr && err.status == RenderCreateStatus::kInvalidAdapter;
		report.Record( "create.invalid_adapter", ok,
			"creating on an invalid adapter must fail with kInvalidAdapter and no device" );
		if ( dev )
			provider.DestroyDevice( dev );
	}

	// Unsatisfied required feature -> structured kUnsupportedRequiredFeature naming
	// the missing feature, no device.
	{
		const RenderFeature missing = UnsupportedFeatureFor( adapter0 );
		RenderDeviceRequest req;
		req.adapterIndex = 0;
		req.requiredFeatures.Add( missing );
		RenderCreateError err;
		IRenderDevice *dev = provider.CreateDevice( req, &err );
		const bool ok = dev == nullptr &&
			err.status == RenderCreateStatus::kUnsupportedRequiredFeature &&
			err.missingFeature == missing;
		report.Record( "create.unsupported_required_feature", ok,
			"an unsatisfiable required feature must fail structurally and name the feature" );
		if ( dev )
			provider.DestroyDevice( dev );
	}
}

// Creates an offscreen device on adapter 0 requesting its supported features.
// Returns nullptr (recording a failure) if creation was advertised but failed.
IRenderDevice *CreateOffscreenDevice( IRenderBackendProvider &provider, Report &report,
	const RenderAdapterInfo &adapter0 )
{
	RenderDeviceRequest req;
	req.adapterIndex = 0;
	req.requiredFeatures = adapter0.supportedFeatures;
	RenderCreateError err;
	IRenderDevice *dev = provider.CreateDevice( req, &err );
	report.Record( "create.offscreen_ok", dev != nullptr,
		std::string( "offscreen device creation must succeed for advertised features; status=" ) +
			std::to_string( static_cast<uint32_t>( err.status ) ) );
	return dev;
}

void CheckImmutableCaps( IRenderDevice &device, Report &report )
{
	const RenderDeviceCaps a = device.GetCapabilities();
	const RenderDeviceCaps b = device.GetCapabilities();
	report.Record( "caps.immutable", a == b,
		"device capabilities must not change between queries" );
}

void CheckResourceLifetimeAndCompletion( IRenderDevice &device, Report &report )
{
	const size_t base = device.GetLiveResourceCount();

	const RenderResourceHandle res = device.CreateResource( RenderResourceType::kBuffer );
	report.Record( "resource.create_live", res != kInvalidResource && device.IsResourceLive( res ),
		"a created resource must be a valid, live handle" );
	report.Record( "resource.count_incremented", device.GetLiveResourceCount() == base + 1,
		"creating a resource must increment the live-resource count" );

	// Record use of the resource in a submission and defer its destruction on the
	// resulting completion token.
	IRenderCommandContext *ctx = device.CreateCommandContext();
	report.Record( "command.context_created", ctx != nullptr,
		"CreateCommandContext must return a context" );
	if ( !ctx )
		return;
	ctx->RecordUse( res );

	IRenderCompletionToken *token = device.Submit( *ctx );
	report.Record( "submit.token_returned", token != nullptr,
		"Submit must return a provider-owned completion token" );
	if ( !token )
		return;
	report.Record( "submit.token_initially_incomplete", !token->IsComplete(),
		"a freshly submitted token must not already be complete" );

	device.DestroyResourceWhenComplete( res, *token );

	// The resource must stay live while the token is incomplete, even after a
	// collection pass -- a CPU-side event or frame advance is NOT completion.
	device.CollectCompletedDestructions();
	report.Record( "resource.deferred_until_complete",
		device.IsResourceLive( res ) && !token->IsComplete(),
		"a resource must stay live until its completion token reports complete" );

	// Drive GPU completion deterministically, then collect.
	device.PollCompletion();
	report.Record( "submit.token_complete_after_poll", token->IsComplete(),
		"PollCompletion must complete the outstanding submission" );

	device.CollectCompletedDestructions();
	report.Record( "resource.recycled_after_complete",
		!device.IsResourceLive( res ) && device.GetLiveResourceCount() == base,
		"a resource must be recycled once its completion token is complete" );
}

void CheckSubmissionOrdering( IRenderDevice &device, Report &report )
{
	IRenderCommandContext *c1 = device.CreateCommandContext();
	IRenderCommandContext *c2 = device.CreateCommandContext();
	if ( !c1 || !c2 )
	{
		report.Record( "submit.ordering", false, "could not create two command contexts" );
		return;
	}

	IRenderCompletionToken *t1 = device.Submit( *c1 );
	IRenderCompletionToken *t2 = device.Submit( *c2 );
	if ( !t1 || !t2 )
	{
		report.Record( "submit.ordering", false, "submission did not return two tokens" );
		return;
	}

	const uint64_t before = device.LastCompletedSubmission();

	device.PollCompletion(); // completes the first submission only
	const bool firstDone = t1->IsComplete() && !t2->IsComplete();
	report.Record( "submit.ordered_completion", firstDone,
		"submissions must complete in submission order (first before second)" );

	const uint64_t mid = device.LastCompletedSubmission();
	device.PollCompletion(); // completes the second submission
	const uint64_t after = device.LastCompletedSubmission();
	report.Record( "submit.monotonic_completion", mid > before && after > mid && t2->IsComplete(),
		"LastCompletedSubmission must advance monotonically as work completes" );
}

void CheckDeviceLoss( IRenderBackendProvider &provider, IRenderDevice &device, Report &report )
{
	const RenderProviderCaps caps = provider.GetProviderCaps();

	const bool lost = device.SimulateDeviceLoss();
	report.Record( "loss.enters_lost_state",
		lost && device.GetState() == RenderDeviceState::kDeviceLost,
		"SimulateDeviceLoss must transition the device to kDeviceLost" );

	const bool recovered = device.RecoverDevice();
	if ( caps.supportsDeviceLossRecovery )
	{
		report.Record( "loss.recovers_to_available",
			recovered && device.GetState() == RenderDeviceState::kAvailable,
			"a recovery-capable provider must return to kAvailable after RecoverDevice" );
	}
	else
	{
		report.Record( "loss.non_recoverable_is_fatal",
			!recovered && device.GetState() == RenderDeviceState::kFatal,
			"a provider without recovery must report kFatal after device loss" );
	}
}

// A fixed-extent surface for presentation tests. Extent is mutated by the caller
// to model surface/window resizes and orientation changes.
class TestSurface : public IRenderSurface
{
public:
	explicit TestSurface( RenderExtent extent ) : m_Extent( extent ) {}
	RenderExtent GetDrawableExtent() const override { return m_Extent; }
	void SetExtent( RenderExtent extent ) { m_Extent = extent; }

private:
	RenderExtent m_Extent;
};

void CheckPresentation( IRenderBackendProvider &provider, IRenderDevice &device, Report &report )
{
	IRenderDevice *const deviceBefore = &device;

	TestSurface surface( RenderExtent{ 800, 600 } );
	RenderPresentationConfig config;
	config.extent = RenderExtent{ 800, 600 };

	RenderCreateError err;
	IRenderPresentation *pres = device.CreatePresentation( surface, config, &err );
	report.Record( "present.create", pres != nullptr,
		"presentation creation must succeed where advertised" );
	if ( !pres )
		return;

	report.Record( "present.initial_extent", pres->GetExtent() == config.extent,
		"presentation must report its configured extent" );

	// Runtime resize must not recreate the logical device.
	const bool resized = pres->ResizeTo( RenderExtent{ 1024, 768 } );
	report.Record( "present.resize", resized && pres->GetExtent() == RenderExtent{ 1024, 768 },
		"runtime resize must update the drawable extent" );
	report.Record( "present.resize_keeps_device",
		&device == deviceBefore && device.GetState() == RenderDeviceState::kAvailable,
		"runtime resize must not recreate or lose the logical render device" );

	// Orientation-style aspect change.
	const bool rotated = pres->ResizeTo( RenderExtent{ 600, 800 } );
	report.Record( "present.orientation_change",
		rotated && pres->GetExtent() == RenderExtent{ 600, 800 } &&
			device.GetState() == RenderDeviceState::kAvailable,
		"an aspect/orientation change must resize without losing the device" );

	// Transient zero-sized state suspends non-fatally.
	pres->ResizeTo( RenderExtent{ 0, 0 } );
	const RenderPresentStatus suspended = pres->Present();
	report.Record( "present.zero_size_suspends",
		suspended == RenderPresentStatus::kSuspended &&
			device.GetState() != RenderDeviceState::kFatal,
		"a zero-sized surface must suspend presentation, not fail fatally" );

	// Resume when presentable again.
	pres->ResizeTo( RenderExtent{ 640, 480 } );
	const RenderPresentStatus resumed = pres->Present();
	report.Record( "present.resume_after_suspend", resumed == RenderPresentStatus::kOk,
		"presentation must resume once the surface is presentable again" );

	device.DestroyPresentation( pres );
	report.Record( "present.destroy_keeps_device",
		device.GetState() != RenderDeviceState::kFatal,
		"destroying a presentation must not fault the device" );

	// Multiple presentations, where advertised.
	const RenderProviderCaps caps = provider.GetProviderCaps();
	if ( caps.supportsMultiplePresentations )
	{
		std::vector<IRenderPresentation *> live;
		std::vector<TestSurface *> surfaces;
		bool allOk = true;
		for ( uint32_t i = 0; i < caps.maxPresentations; ++i )
		{
			TestSurface *s = new TestSurface( RenderExtent{ 320, 240 } );
			surfaces.push_back( s );
			RenderPresentationConfig cfg;
			cfg.extent = RenderExtent{ 320, 240 };
			IRenderPresentation *p = device.CreatePresentation( *s, cfg, nullptr );
			if ( !p )
				allOk = false;
			else
				live.push_back( p );
		}
		report.Record( "present.multiple_surfaces", allOk && live.size() == caps.maxPresentations,
			"a provider must support up to maxPresentations concurrent surfaces" );

		// One beyond the limit must fail structurally.
		TestSurface extra( RenderExtent{ 320, 240 } );
		RenderPresentationConfig cfg;
		cfg.extent = RenderExtent{ 320, 240 };
		RenderCreateError overErr;
		IRenderPresentation *over = device.CreatePresentation( extra, cfg, &overErr );
		report.Record( "present.over_limit_rejected",
			over == nullptr && overErr.status == RenderCreateStatus::kTooManyPresentations,
			"exceeding maxPresentations must fail with kTooManyPresentations" );
		if ( over )
			device.DestroyPresentation( over );

		for ( IRenderPresentation *p : live )
			device.DestroyPresentation( p );
		for ( TestSurface *s : surfaces )
			delete s;
	}
}

} // namespace

bool RunRenderBackendConformance( IRenderBackendProvider &provider, Report &report )
{
	const RenderProviderCaps caps = provider.GetProviderCaps();

	CheckIdentity( provider, report );

	RenderAdapterInfo adapter0;
	if ( !CheckAdapterEnumeration( provider, report, adapter0 ) )
		return report.Passed(); // cannot proceed without an adapter

	CheckStructuredFailure( provider, report, adapter0 );

	if ( caps.supportsOffscreenDevice )
	{
		IRenderDevice *device = CreateOffscreenDevice( provider, report, adapter0 );
		if ( device )
		{
			CheckImmutableCaps( *device, report );
			CheckResourceLifetimeAndCompletion( *device, report );
			CheckSubmissionOrdering( *device, report );

			if ( caps.supportsPresentation )
				CheckPresentation( provider, *device, report );

			// Device loss last: it may transition the device to a terminal state.
			CheckDeviceLoss( provider, *device, report );

			provider.DestroyDevice( device );
		}
	}

	// Complete destruction: after tearing down every device, the provider owns no
	// live devices (proves teardown before a backend module would unload).
	report.Record( "lifetime.no_leaked_devices", provider.GetLiveDeviceCount() == 0,
		"a provider must own zero live devices after all are destroyed" );

	return report.Passed();
}

} // namespace render::conformance
