//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy render providers through the render.contracts provider
//			contract (RFC 0001 rank 8). Runs the SAME shared render-backend suite
//			as the null fake against LegacyRenderBackendProvider over each real
//			linked legacy module that can enumerate adapters without a composed
//			material system, and over deliberately bad fake device managers. Also
//			covers adapter translation and legacy profile selection, including the
//			documented quirk table.
//
//			The D3D9 manager enumerates adapters only after a material-system
//			Connect, so its provider is exercised by the composed product boot and
//			material_pixel_conformance instead (see RFC/0001-render-seam-progress.md).
//
//=============================================================================//

#include "materialsystem/imaterialsystem.h"
#include "render/legacy_shader_provider.h"
#include "legacy_render_backend_provider.h"
#include "shader_device_facade.h"
#include "render/legacy/stream_device.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "tier1/KeyValues.h"
#include "conformance/render_backend_conformance.h"

#include <cstdio>
#include <string.h>

namespace
{

int g_Checks = 0;
int g_Failures = 0;

void Check( bool passed, const char *condition, int line )
{
	++g_Checks;
	if ( !passed )
	{
		++g_Failures;
		std::printf( "FAIL legacy render provider line %d: %s\n", line, condition );
	}
}

#define CHECK( condition ) Check( ( condition ), #condition, __LINE__ )

// A scriptable adapter source: the part of a legacy device manager the
// provider uses.
class CFakeDeviceMgr : public render::ILegacyAdapterSource
{
public:
	CFakeDeviceMgr() : m_nAdapters( 1 ), m_nCalls( 0 )
	{
		memset( &m_Info, 0, sizeof( m_Info ) );
		strcpy( m_Info.m_pDriverName, "Fixture GPU" );
		m_Info.m_VendorID = 0x1002;
		m_Info.m_DeviceID = 0x7300;
		m_Info.m_nDriverVersionHigh = 0x00010002;
		m_Info.m_nDriverVersionLow = 0x00030004;
	}

	int GetAdapterCount() const override { return m_nAdapters; }
	void GetAdapterInfo( int, MaterialAdapterInfo_t &info ) const override
	{
		++m_nCalls;
		info = m_Info;
	}

	int m_nAdapters;
	MaterialAdapterInfo_t m_Info;
	mutable int m_nCalls;
};

CFakeDeviceMgr g_FakeMgr;
const char *g_pFakeDriverApi = "opengl";
bool g_bFakeDescribeFails = false;
bool g_bFakeClaimsNever = false;

bool DescribeFake( int adapter, render::RenderAdapterInfo *info )
{
	if ( g_bFakeDescribeFails || adapter < 0 || adapter >= g_FakeMgr.m_nAdapters )
		return false;
	*info = render::RenderAdapterInfo();
	info->supportedFeatures.Add( render::RenderFeature::kSampledSrgb );
	if ( g_bFakeClaimsNever )
		info->supportedFeatures.Add( render::RenderFeature::kNeverSupported );
	strcpy( info->driverApi, g_pFakeDriverApi );
	info->deviceMemoryBytes = 256u << 20;
	// The manager owns identity; these must be ignored by the provider.
	strcpy( info->name, "ignored" );
	info->vendorId = 0xFFFF;
	return true;
}

const render::LegacyShaderProvider g_FakeProvider = { "fixture", "fixture_module", NULL, false };

render::LegacyRenderBackendProvider FakeProvider( bool withDescribe )
{
	return render::LegacyRenderBackendProvider(
	    g_FakeProvider, &g_FakeMgr, withDescribe ? DescribeFake : NULL );
}

bool SelectFake( const render::RenderProfileRequest &request, render::RenderFeatureProfile *profile,
    render::RenderProfileError *error )
{
	return render::SelectLegacyRenderProfile( FakeProvider( true ), 0, request, profile, error );
}

void ResetFake()
{
	g_FakeMgr = CFakeDeviceMgr();
	g_pFakeDriverApi = "opengl";
	g_bFakeDescribeFails = false;
	g_bFakeClaimsNever = false;
}

int RunSharedSuite( render::IRenderBackendProvider &provider, const char *label, bool expectPass )
{
	render::conformance::Report report;
	const bool passed = render::conformance::RunRenderBackendConformance( provider, report );
	for ( size_t i = 0; i < report.checks.size(); ++i )
	{
		if ( !report.checks[i].ok && expectPass )
			std::printf( "FAIL %s %s: %s\n", label, report.checks[i].id.c_str(),
			    report.checks[i].detail.c_str() );
	}
	std::printf( "%s: shared suite %d check(s), %d failure(s)\n", label, (int)report.checks.size(),
	    report.FailureCount() );
	CHECK( passed == expectPass );
	return (int)report.checks.size();
}

// The material system's composition (CMaterialSystem::BindShaderProvider):
// the backend's services with its manager behind the one device facade.
bool Compose( const render::LegacyShaderProvider *provider, render::LegacyShaderServices *services,
    CShaderDeviceFacade *facade )
{
	if ( !provider->create( services ) )
		return false;
	facade->Bind( *services );
	services->manager = facade;
	return true;
}

// A real linked legacy module through the provider contract.
void CheckRealProvider(
    const render::LegacyShaderProvider *provider, const char *driverApi, bool isSoftware )
{
	CHECK( provider && provider->create );
	if ( !provider || !provider->create )
		return;
	render::LegacyShaderServices services;
	CShaderDeviceFacade facade;
	CHECK( Compose( provider, &services, &facade ) );
	CHECK( services.describeAdapter != NULL );

	render::LegacyRenderBackendProvider backend( *provider, services );
	CHECK( backend.IsValid() );
	CHECK( backend.GetAdapterCount() >= 1 );
	render::RenderAdapterInfo info;
	CHECK( backend.GetAdapterInfo( 0, &info ) );
	CHECK( strcmp( info.driverApi, driverApi ) == 0 );
	CHECK( info.isSoftware == isSoftware );
	char expectedId[64];
	std::snprintf( expectedId, sizeof( expectedId ), "%s:0", provider->id );
	CHECK( strcmp( info.id, expectedId ) == 0 );
	CHECK( strcmp( backend.GetBackendId().id, provider->id ) == 0 );

	// The claims are narrow: no offscreen device, so nothing a presentation
	// bridge could present from.
	const render::RenderProviderCaps caps = backend.GetProviderCaps();
	CHECK( !caps.supportsOffscreenDevice );
	// Identity, adapters, structured failure and zero live devices.
	CHECK( RunSharedSuite( backend, provider->id, true ) >= 9 );

	render::RenderFeatureProfile profile;
	render::RenderProfileError error;
	CHECK( render::SelectLegacyRenderProfile(
	    *provider, services, 0, render::PreferAvailableRenderFeatures(), &profile, &error ) );
	CHECK( profile.enabled == info.supportedFeatures );
	CHECK( profile.appliedQuirkCount == 0 );
	CHECK( profile.workarounds.IsEmpty() );
}

void CheckNullProvider()
{
	const render::LegacyShaderProvider *provider = NullShaderBackend_Describe();
	CheckRealProvider( provider, "none", true );
	render::LegacyShaderServices services;
	CShaderDeviceFacade facade;
	CHECK( Compose( provider, &services, &facade ) );
	render::LegacyRenderBackendProvider backend( *provider, services );
	render::RenderAdapterInfo info;
	CHECK( backend.GetAdapterInfo( 0, &info ) );
	CHECK( strcmp( info.name, "Null (no GPU)" ) == 0 );
	MaterialAdapterInfo_t legacy;
	facade.GetAdapterInfo( 0, legacy );
	CHECK( legacy.m_nDXSupportLevel == 90 && legacy.m_nMaxDXSupportLevel == 0 );
	CHECK( facade.GetModeCount( 0 ) == 0 );
	CHECK( info.supportedFeatures.bits == 0 );

	// The null adapter offers no feature, so a required one fails structurally.
	render::RenderProfileRequest request;
	request.required.Add( render::RenderFeature::kSampledSrgb );
	render::RenderFeatureProfile profile;
	render::RenderProfileError error;
	CHECK(
	    !render::SelectLegacyRenderProfile( *provider, services, 0, request, &profile, &error ) );
	CHECK( error.status == render::RenderProfileStatus::kMissingRequiredFeature );
	CHECK( error.feature == render::RenderFeature::kSampledSrgb );
	CHECK( !render::SelectLegacyRenderProfile(
	    *provider, services, 1, render::PreferAvailableRenderFeatures(), &profile, &error ) );
	CHECK( error.status == render::RenderProfileStatus::kInvalidAdapter );
}

void CheckTranslation()
{
	ResetFake();
	render::LegacyRenderBackendProvider backend = FakeProvider( true );
	CHECK( backend.IsValid() );
	render::RenderAdapterInfo info;
	CHECK( backend.GetAdapterInfo( 0, &info ) );
	CHECK( strcmp( info.id, "fixture:0" ) == 0 );
	CHECK( strcmp( info.name, "Fixture GPU" ) == 0 );
	CHECK( info.vendorId == 0x1002 );
	CHECK( info.deviceId == 0x7300 );
	CHECK( info.driverVersion == 0x0001000200030004ull );
	CHECK( strcmp( info.driverApi, "opengl" ) == 0 );
	CHECK( info.deviceMemoryBytes == ( 256u << 20 ) );
	CHECK( info.supportedFeatures.Has( render::RenderFeature::kSampledSrgb ) );
	RunSharedSuite( backend, "fixture", true );

	// Descriptions are captured values: later manager changes do not leak in.
	const int calls = g_FakeMgr.m_nCalls;
	g_FakeMgr.m_Info.m_VendorID = 0x10DE;
	render::RenderAdapterInfo again;
	CHECK( backend.GetAdapterInfo( 0, &again ) );
	CHECK( again == info );
	CHECK( g_FakeMgr.m_nCalls == calls );

	// Without a describe hook an adapter claims no semantic feature or api.
	ResetFake();
	render::LegacyRenderBackendProvider bare = FakeProvider( false );
	CHECK( bare.IsValid() );
	CHECK( bare.GetAdapterInfo( 0, &info ) );
	CHECK( info.supportedFeatures.bits == 0 && info.driverApi[0] == '\0' );

	// Structured device-creation outcomes.
	render::RenderCreateError error;
	render::RenderDeviceRequest request;
	request.adapterIndex = 3;
	CHECK( backend.CreateDevice( request, &error ) == NULL );
	CHECK( error.status == render::RenderCreateStatus::kInvalidAdapter );
	request.adapterIndex = 0;
	request.requiredFeatures.Add( render::RenderFeature::kComputeShaders );
	CHECK( backend.CreateDevice( request, &error ) == NULL );
	CHECK( error.status == render::RenderCreateStatus::kUnsupportedRequiredFeature );
	CHECK( error.missingFeature == render::RenderFeature::kComputeShaders );
	request.requiredFeatures = render::RenderFeatureSet();
	CHECK( backend.CreateDevice( request, &error ) == NULL );
	CHECK( error.status == render::RenderCreateStatus::kNotAdvertised );
	CHECK( backend.GetLiveDeviceCount() == 0 );
}

// Deliberately bad legacy backends. Each is rejected, and the shared suite
// detects the resulting provider instead of certifying it.
void CheckBadProviders()
{
	struct BadCase
	{
		const char *label;
		void ( *arrange )();
	};
	const BadCase cases[] = {
	    { "describe rejects an enumerated adapter",
	        []
	        {
		        g_bFakeDescribeFails = true;
	        } },
	    { "describe claims the reserved feature",
	        []
	        {
		        g_bFakeClaimsNever = true;
	        } },
	    { "too many adapters",
	        []
	        {
		        g_FakeMgr.m_nAdapters = render::LegacyRenderBackendProvider::kMaxAdapters + 1;
	        } },
	    { "negative adapter count",
	        []
	        {
		        g_FakeMgr.m_nAdapters = -1;
	        } },
	    { "no adapters",
	        []
	        {
		        g_FakeMgr.m_nAdapters = 0;
	        } },
	};
	for ( size_t i = 0; i < sizeof( cases ) / sizeof( cases[0] ); ++i )
	{
		ResetFake();
		cases[i].arrange();
		render::LegacyRenderBackendProvider backend = FakeProvider( true );
		CHECK( backend.GetAdapterCount() == 0 );
		RunSharedSuite( backend, cases[i].label, false );
		render::RenderFeatureProfile profile;
		render::RenderProfileError error;
		CHECK( !SelectFake( render::PreferAvailableRenderFeatures(), &profile, &error ) );
		CHECK( error.status == render::RenderProfileStatus::kInvalidProvider ||
		       error.status == render::RenderProfileStatus::kInvalidAdapter );
		CHECK( error.message[0] != '\0' );
	}

	// A provider without an id or manager is invalid.
	render::LegacyShaderProvider unnamed = g_FakeProvider;
	unnamed.id = "";
	render::LegacyRenderBackendProvider noId( unnamed, &g_FakeMgr, DescribeFake );
	CHECK( !noId.IsValid() );
	render::LegacyRenderBackendProvider noSource( g_FakeProvider, NULL, DescribeFake );
	CHECK( !noSource.IsValid() );
	render::LegacyShaderServices noManager;
	render::LegacyRenderBackendProvider missing( g_FakeProvider, noManager );
	CHECK( !missing.IsValid() );
}

void CheckQuirkTable()
{
	size_t count = 0;
	const render::RenderQuirk *quirks = render::LegacyRenderQuirks( &count );
	CHECK( quirks != NULL && count >= 1 );
	for ( size_t i = 0; i < count; ++i )
		CHECK( render::IsValidRenderQuirk( quirks[i] ) );

	// The OpenGL translation quirk selects the float cubemap workaround...
	ResetFake();
	render::RenderFeatureProfile profile;
	render::RenderProfileError error;
	CHECK( SelectFake( render::PreferAvailableRenderFeatures(), &profile, &error ) );
	CHECK( profile.workarounds.Has( render::RenderWorkaround::kFloatNormalizationCubemaps ) );
	CHECK( profile.appliedQuirkCount == 1 &&
	       strcmp( profile.appliedQuirks[0]->id, "gl.float-normalization-cubemaps" ) == 0 );
	CHECK( profile.enabled.Has( render::RenderFeature::kSampledSrgb ) );

	// ...and nothing else does.
	const char *apis[] = { "d3d9", "vulkan", "none", "" };
	for ( size_t i = 0; i < sizeof( apis ) / sizeof( apis[0] ); ++i )
	{
		ResetFake();
		g_pFakeDriverApi = apis[i];
		CHECK( SelectFake( render::PreferAvailableRenderFeatures(), &profile, &error ) );
		CHECK( profile.workarounds.IsEmpty() && profile.appliedQuirkCount == 0 );
	}

	char description[256];
	ResetFake();
	SelectFake( render::PreferAvailableRenderFeatures(), &profile, &error );
	render::DescribeRenderProfile( profile, description, sizeof( description ) );
	CHECK( strcmp( description, "features=sampled-srgb quirks=gl.float-normalization-cubemaps" ) ==
	       0 );
	render::DescribeRenderProfile( render::RenderFeatureProfile(), description, 8 );
	CHECK( strlen( description ) < 8 );
}

// ---------------------------------------------------------------------------
// The device facade (RFC 0016 legacy device facade, F1)
// ---------------------------------------------------------------------------

// A backend's lifecycle and presentation callbacks: counts what the facades
// forward. No backend implements IShaderDeviceMgr (F4).
struct CForwardingMgr
{
	static void *ForwardedFactory( const char *, int * ) { return NULL; }

	void Fill( render::LegacyShaderServices *services )
	{
		services->lifecycle.context = this;
		services->lifecycle.connect = []( void *self, CreateInterfaceFn )
		{
			return ++static_cast<CForwardingMgr *>( self )->m_nConnects > 0;
		};
		services->lifecycle.disconnect = []( void *self )
		{
			++static_cast<CForwardingMgr *>( self )->m_nDisconnects;
		};
		services->lifecycle.init = []( void * )
		{
			return true;
		};
		services->lifecycle.shutdown = []( void *self )
		{
			++static_cast<CForwardingMgr *>( self )->m_nShutdowns;
		};
		services->lifecycle.setAdapter = []( void *, int adapter, int )
		{
			return adapter == 0;
		};
		services->lifecycle.setMode = []( void *self, void *, int, const ShaderDeviceInfo_t & )
		{
			++static_cast<CForwardingMgr *>( self )->m_nSetModes;
			return static_cast<CreateInterfaceFn>( ForwardedFactory );
		};
		services->presentation.context = this;
		services->presentation.addModeChangeCallback = []( void *self, void ( * )() )
		{
			++static_cast<CForwardingMgr *>( self )->m_nCallbacks;
		};
		services->presentation.removeModeChangeCallback = []( void *self, void ( * )() )
		{
			--static_cast<CForwardingMgr *>( self )->m_nCallbacks;
		};
	}

	int m_nConnects = 0, m_nDisconnects = 0, m_nShutdowns = 0, m_nSetModes = 0, m_nCallbacks = 0;
	int m_nPrepares = 0, m_nPreparedAtSetMode = -1, m_nReleases = 0, m_nReleasedAfterShutdowns = -1;
};

bool g_bBackendSoftware = false;
bool g_bBackendDescribes = true;

bool DescribeBackend( int adapter, render::RenderAdapterInfo *info )
{
	if ( !g_bBackendDescribes || adapter != 0 )
		return false;
	*info = render::RenderAdapterInfo();
	strcpy( info->name, "Backend GPU" );
	info->vendorId = 0x1234;
	info->deviceId = 0x5678;
	info->driverVersion = 0x0000000200000000ull;
	info->deviceMemoryBytes = 1u << 20;
	strcpy( info->driverApi, "vulkan" );
	info->isSoftware = g_bBackendSoftware;
	info->supportedFeatures.Add( render::RenderFeature::kComputeShaders );
	return true;
}

int g_nCoreAdapters = 1;

bool DescribeCore( void *context, int adapter, render::RenderAdapterInfo *info )
{
	if ( context != &g_nCoreAdapters || adapter < 0 || adapter >= g_nCoreAdapters )
		return false;
	*info = render::RenderAdapterInfo();
	strcpy( info->name, "Core GPU" );
	info->vendorId = 0x10DE;
	info->deviceId = 0x2484;
	info->driverVersion = 0x0000000300000001ull;
	info->deviceMemoryBytes = 8ull << 30;
	strcpy( info->driverApi, "ignored" );
	info->isSoftware = true; // semantic facts stay the backend's
	return true;
}

render::DisplayModeFacts g_Desktop;

bool FakeDesktop( void *, render::DisplayModeFacts *desktop )
{
	*desktop = g_Desktop;
	return desktop->width > 0;
}

void CheckDeviceFacade()
{
	// The null backend's hardware config and API stand in for the backend's.
	render::LegacyShaderServices services;
	CHECK( NullShaderBackend_Describe()->create( &services ) );
	CForwardingMgr backend;
	backend.Fill( &services );
	services.describeAdapter = DescribeBackend;
	g_bBackendSoftware = false;
	g_bBackendDescribes = true;
	g_nCoreAdapters = 1;
	g_Desktop = render::DisplayModeFacts();

	CShaderDeviceFacade facade;
	CHECK( facade.GetAdapterCount() == 0 ); // unbound
	CHECK( facade.Init() == INIT_FAILED );
	CHECK( facade.SetMode( NULL, 0, ShaderDeviceInfo_t() ) == NULL );
	facade.Bind( services );

	// Without the core, identity and facts are the backend's describe hook's;
	// the backend manager's own answers are never reported.
	CHECK( facade.GetAdapterCount() == 1 );
	MaterialAdapterInfo_t info;
	facade.GetAdapterInfo( 0, info );
	CHECK( strcmp( info.m_pDriverName, "Backend GPU" ) == 0 );
	CHECK( info.m_VendorID == 0x1234 && info.m_DeviceID == 0x5678 );
	CHECK( info.m_nDriverVersionHigh == 2 && info.m_nDriverVersionLow == 0 );
	CHECK( info.m_nDXSupportLevel == services.hardware->GetMaxDXSupportLevel() );
	CHECK( info.m_nMaxDXSupportLevel == services.hardware->GetMaxDXSupportLevel() );
	facade.GetAdapterInfo( 1, info );
	CHECK( info.m_pDriverName[0] == '\0' && info.m_VendorID == 0 );

	// With the core's source, identity is the core's; the backend keeps the
	// semantic facts (driverApi, software, features).
	render::LegacyShaderServices withCore = services;
	withCore.coreAdapter.context = &g_nCoreAdapters;
	withCore.coreAdapter.describe = DescribeCore;
	facade.Bind( withCore );
	CHECK( facade.GetAdapterCount() == 1 );
	facade.GetAdapterInfo( 0, info );
	CHECK( strcmp( info.m_pDriverName, "Core GPU" ) == 0 );
	CHECK( info.m_VendorID == 0x10DE && info.m_DeviceID == 0x2484 );
	CHECK( info.m_nDriverVersionHigh == 3 && info.m_nDriverVersionLow == 1 );
	render::LegacyShaderServices composed = withCore;
	composed.manager = &facade;
	render::LegacyRenderBackendProvider provider( g_FakeProvider, composed );
	render::RenderAdapterInfo described;
	CHECK( provider.IsValid() && provider.GetAdapterInfo( 0, &described ) );
	CHECK( strcmp( described.name, "Core GPU" ) == 0 );
	CHECK( strcmp( described.driverApi, "vulkan" ) == 0 );
	CHECK( !described.isSoftware );
	CHECK( described.supportedFeatures.Has( render::RenderFeature::kComputeShaders ) );

	// Bad sources: an adapter either side cannot describe is not enumerated.
	g_nCoreAdapters = 0;
	CHECK( facade.GetAdapterCount() == 0 );
	g_nCoreAdapters = 1;
	withCore.coreAdapter.context = NULL; // the source refuses a foreign context
	facade.Bind( withCore );
	CHECK( facade.GetAdapterCount() == 0 );
	facade.Bind( services );
	g_bBackendDescribes = false;
	CHECK( facade.GetAdapterCount() == 0 );
	g_bBackendDescribes = true;
	render::LegacyShaderServices noDescribe = services;
	noDescribe.describeAdapter = NULL;
	facade.Bind( noDescribe );
	CHECK( facade.GetAdapterCount() == 0 );
	facade.Bind( services );

	// Video modes come from the desktop display alone.
	CHECK( facade.GetModeCount( 0 ) == 0 );
	ShaderDisplayMode_t mode;
	facade.GetCurrentModeInfo( &mode, 0 );
	CHECK( mode.m_nWidth == 0 && mode.m_nRefreshRateDenominator == 0 );
	g_Desktop.width = 1920;
	g_Desktop.height = 1080;
	g_Desktop.refreshNumerator = 60;
	facade.SetDesktopSource( FakeDesktop, NULL );
	const int modes = facade.GetModeCount( 0 );
	CHECK( modes == (int)render::BuildBackBufferModeList( g_Desktop ).size() && modes > 0 );
	bool fit = true;
	for ( int i = 0; i < modes; ++i )
	{
		facade.GetModeInfo( &mode, 0, i );
		fit = fit && mode.m_nWidth <= 1920 && mode.m_nHeight <= 1080 && mode.m_nWidth > 0 &&
		      mode.m_nRefreshRateNumerator == 60;
	}
	CHECK( fit );
	facade.GetModeInfo( &mode, 0, modes );
	CHECK( mode.m_nWidth == 0 && mode.m_nRefreshRateDenominator == 0 );
	facade.GetCurrentModeInfo( &mode, 0 );
	CHECK( mode.m_nWidth == 1920 && mode.m_nHeight == 1080 && mode.m_nRefreshRateNumerator == 60 );

	// A software adapter has no recommended configuration to apply, no
	// display modes and no maximum level (the null manager's answers).
	g_bBackendSoftware = true;
	CHECK( facade.GetModeCount( 0 ) == 0 );
	facade.GetCurrentModeInfo( &mode, 0 );
	CHECK( mode.m_nWidth == 0 );
	facade.GetAdapterInfo( 0, info );
	CHECK( info.m_nMaxDXSupportLevel == 0 && info.m_nDXSupportLevel == services.hardware->GetMaxDXSupportLevel() );
	KeyValues *config = new KeyValues( "config" );
	CHECK( facade.GetRecommendedConfigurationInfo( 0, 0, config ) );
	CHECK( config->GetFirstSubKey() == NULL );
	CHECK( !facade.GetRecommendedConfigurationInfo( 1, 0, config ) );
	config->deleteThis();
	g_bBackendSoftware = false;

	// The core's device: created before the backend sets the mode, released
	// after it shut down; a refused creation fails the mode.
	render::LegacyShaderServices withDevice = services;
	withDevice.coreDevice.context = &backend;
	withDevice.coreDevice.prepare = []( void *context, void *window, char *error, size_t size )
	{
		CForwardingMgr *mgr = static_cast<CForwardingMgr *>( context );
		mgr->m_nPreparedAtSetMode = mgr->m_nSetModes;
		++mgr->m_nPrepares;
		if ( window == reinterpret_cast<void *>( 1 ) )
		{
			strcpy( error, "refused" );
			return false;
		}
		return true;
	};
	withDevice.coreDevice.release = []( void *context )
	{
		CForwardingMgr *mgr = static_cast<CForwardingMgr *>( context );
		mgr->m_nReleasedAfterShutdowns = mgr->m_nShutdowns;
		++mgr->m_nReleases;
	};
	{
		CShaderDeviceFacade device;
		device.Bind( withDevice );
		const int setModes = backend.m_nSetModes;
		CHECK(
		    device.SetMode( NULL, 0, ShaderDeviceInfo_t() ) == CForwardingMgr::ForwardedFactory );
		CHECK( backend.m_nPrepares == 1 && backend.m_nPreparedAtSetMode == setModes );
		CHECK( backend.m_nSetModes == setModes + 1 );
		CHECK( device.SetMode( reinterpret_cast<void *>( 1 ), 0, ShaderDeviceInfo_t() ) == NULL );
		CHECK( backend.m_nSetModes == setModes + 1 ); // the backend never saw the refused mode
		const int shutdowns = backend.m_nShutdowns;
		device.Shutdown();
		CHECK( backend.m_nReleases == 1 && backend.m_nReleasedAfterShutdowns == shutdowns + 1 );
		backend.m_nSetModes = setModes;
		backend.m_nShutdowns = shutdowns;
	}

	// The device bring-up and lifecycle still belong to the backend.
	CHECK( facade.Connect( CForwardingMgr::ForwardedFactory ) && backend.m_nConnects == 1 );
	CHECK( facade.Init() == INIT_OK );
	CHECK( facade.SetAdapter( 0, 0 ) && !facade.SetAdapter( 1, 0 ) );
	CHECK( facade.SetMode( NULL, 0, ShaderDeviceInfo_t() ) == CForwardingMgr::ForwardedFactory );
	CHECK( backend.m_nSetModes == 1 );
	facade.AddModeChangeCallback( NULL );
	CHECK( backend.m_nCallbacks == 1 );
	facade.RemoveModeChangeCallback( NULL );
	CHECK( backend.m_nCallbacks == 0 );
	CHECK( facade.QueryInterface( SHADER_DEVICE_MGR_INTERFACE_VERSION ) == &facade );
	CHECK( facade.QueryInterface( MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION ) ==
	       services.hardware );
	CHECK( facade.QueryInterface( "Backend001" ) == NULL );
	facade.Shutdown();
	facade.Disconnect();
	CHECK( backend.m_nShutdowns == 1 && backend.m_nDisconnects == 1 );

	// Unbinding forgets the backend.
	facade.Unbind();
	CHECK( !facade.IsBound() && facade.GetAdapterCount() == 0 );
}

// The device facade (F4): presentation answers, the stream forwarded.
class CFakeStream final : public render::legacy::ILegacyStreamDevice
{
public:
	void Present() override { ++m_nPresents; }
	void ReleaseResources() override { ++m_nReleases; }
	void ReacquireResources() override { --m_nReleases; }
	bool AddView( void * ) override { return true; }
	void RemoveView( void * ) override {}
	void SetView( void *hWnd ) override { m_pView = hWnd; }
	IMesh *CreateStaticMesh( VertexFormat_t, const char *, IMaterial * ) override
	{
		++m_nMeshes;
		return NULL;
	}
	void DestroyStaticMesh( IMesh * ) override { --m_nMeshes; }
	IVertexBuffer *CreateVertexBuffer(
	    ShaderBufferType_t, VertexFormat_t, int, const char * ) override
	{
		return NULL;
	}
	void DestroyVertexBuffer( IVertexBuffer * ) override {}
	IIndexBuffer *CreateIndexBuffer(
	    ShaderBufferType_t, MaterialIndexFormat_t, int, const char * ) override
	{
		return NULL;
	}
	void DestroyIndexBuffer( IIndexBuffer * ) override {}
	IVertexBuffer *GetDynamicVertexBuffer( int, VertexFormat_t, bool ) override { return NULL; }
	IIndexBuffer *GetDynamicIndexBuffer( MaterialIndexFormat_t, bool ) override { return NULL; }
	IShaderBuffer *CompileShader( const char *, size_t, const char * ) override { return NULL; }
	VertexShaderHandle_t CreateVertexShader( IShaderBuffer * ) override
	{
		return VERTEX_SHADER_HANDLE_INVALID;
	}
	void DestroyVertexShader( VertexShaderHandle_t ) override {}
	GeometryShaderHandle_t CreateGeometryShader( IShaderBuffer * ) override
	{
		return GEOMETRY_SHADER_HANDLE_INVALID;
	}
	void DestroyGeometryShader( GeometryShaderHandle_t ) override {}
	PixelShaderHandle_t CreatePixelShader( IShaderBuffer * ) override
	{
		return PIXEL_SHADER_HANDLE_INVALID;
	}
	void DestroyPixelShader( PixelShaderHandle_t ) override {}
	void EnableNonInteractiveMode(
	    MaterialNonInteractiveMode_t, ShaderNonInteractiveInfo_t * ) override
	{
	}
	void RefreshFrontBufferNonInteractive() override {}
	void HandleThreadEvent( uint32 ) override { ++m_nThreadEvents; }

	int m_nPresents = 0, m_nReleases = 0, m_nMeshes = 0, m_nThreadEvents = 0;
	void *m_pView = NULL;
};

render::LegacyPresentationFacts g_Facts;
float g_GammaSet = 0.0f;
bool g_FixedDisplay = false;

void CheckDeviceFacadeDevice()
{
	render::LegacyShaderServices services;
	CHECK( NullShaderBackend_Describe()->create( &services ) );
	CFakeStream stream;
	services.stream = &stream;

	// Without a presentation source: nothing presents (the null backend's
	// answers: a 1024 x 768 back buffer, no window, no stencil, no AA).
	CShaderDeviceFacadeDevice device;
	device.Bind( services );
	CHECK( !device.IsUsingGraphics() );
	int w = 0, h = 0;
	device.GetBackBufferDimensions( w, h );
	CHECK( w == 1024 && h == 768 );
	device.GetWindowSize( w, h );
	CHECK( w == 0 && h == 0 );
	CHECK( device.StencilBufferBits() == 0 && !device.IsAAEnabled() );
	CHECK( device.GetBackBufferFormat() == IMAGE_FORMAT_RGB888 && device.GetCurrentAdapter() == 0 );
	device.SetHardwareGammaRamp( 2.2f, 16.0f, 235.0f, 2.5f, false ); // no source: ignored

	// With one: the presented frame's facts, and the ramp forwarded.
	services.presentation.facts = []( void *, render::LegacyPresentationFacts *out )
	{
		*out = g_Facts;
	};
	services.presentation.setGammaRamp = []( void *, float gamma, float, float, float, bool )
	{
		g_GammaSet = gamma;
	};
	g_Facts = render::LegacyPresentationFacts();
	g_Facts.presenting = true;
	g_Facts.backBufferWidth = 1920;
	g_Facts.backBufferHeight = 1080;
	g_Facts.windowWidth = 2560;
	g_Facts.windowHeight = 1440;
	g_Facts.samples = 4;
	g_Facts.stencilBits = 8;
	device.Bind( services );
	CHECK( device.IsUsingGraphics() );
	device.GetBackBufferDimensions( w, h );
	CHECK( w == 1920 && h == 1080 );
	device.GetWindowSize( w, h );
	CHECK( w == 2560 && h == 1440 );
	CHECK( device.StencilBufferBits() == 8 && device.IsAAEnabled() );
	g_Facts.samples = 1;
	CHECK( !device.IsAAEnabled() ); // asked each time, never cached
	device.SetHardwareGammaRamp( 2.2f, 16.0f, 235.0f, 2.5f, false );
	CHECK( g_GammaSet == 2.2f );

	// The stream: forwarded.
	device.Present();
	device.Present();
	device.ReleaseResources();
	CHECK( device.CreateStaticMesh( 0, "test" ) == NULL );
	device.SetView( &stream );
	device.HandleThreadEvent( 1 );
	CHECK( stream.m_nPresents == 2 && stream.m_nReleases == 1 && stream.m_nMeshes == 1 );
	CHECK( stream.m_pView == &stream && stream.m_nThreadEvents == 1 );
	device.ReacquireResources();
	device.DestroyStaticMesh( NULL );
	CHECK( stream.m_nReleases == 0 && stream.m_nMeshes == 0 );

	// A fixed display (a handheld's screen) has exactly its own mode.
	CShaderDeviceFacade manager;
	services.presentation.fixedDisplay = []( void *, int *width, int *height, int *refreshHz )
	{
		*width = 400;
		*height = 240;
		*refreshHz = 60;
		return g_FixedDisplay;
	};
	g_FixedDisplay = true;
	manager.Bind( services );
	CHECK( manager.GetModeCount( 0 ) == 1 );
	ShaderDisplayMode_t mode;
	manager.GetModeInfo( &mode, 0, 0 );
	CHECK( mode.m_nWidth == 400 && mode.m_nHeight == 240 && mode.m_nRefreshRateNumerator == 60 );
	manager.GetCurrentModeInfo( &mode, 0 );
	CHECK( mode.m_nWidth == 400 && mode.m_nHeight == 240 );
	g_FixedDisplay = false; // the display not ready: no modes
	CHECK( manager.GetModeCount( 0 ) == 0 );
}

} // namespace

int main()
{
	CheckNullProvider();
	CheckTranslation();
	CheckBadProviders();
	CheckQuirkTable();
	CheckDeviceFacade();
	CheckDeviceFacadeDevice();
	std::printf( "CONFORMANCE %d %d\n", g_Checks, g_Failures );
	return g_Checks > 0 && g_Failures == 0 ? 0 : 1;
}
