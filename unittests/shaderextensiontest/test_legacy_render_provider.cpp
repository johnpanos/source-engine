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

// A real linked legacy module through the provider contract.
void CheckRealProvider(
    const render::LegacyShaderProvider *provider, const char *driverApi, bool isSoftware )
{
	CHECK( provider && provider->create );
	if ( !provider || !provider->create )
		return;
	render::LegacyShaderServices services;
	CHECK( provider->create( &services ) );
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
	provider->create( &services );
	render::LegacyRenderBackendProvider backend( *provider, services );
	render::RenderAdapterInfo info;
	CHECK( backend.GetAdapterInfo( 0, &info ) );
	CHECK( strcmp( info.name, "Null (no GPU)" ) == 0 );
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

} // namespace

int main()
{
	CheckNullProvider();
#if defined( LINKED_NATIVE_VULKAN_BACKEND )
	// Its manager enumerates its single adapter without a composed material system.
	CheckRealProvider( NativeVulkanShaderBackend_Describe(), "vulkan", false );
#endif
	CheckTranslation();
	CheckBadProviders();
	CheckQuirkTable();
	std::printf( "CONFORMANCE %d %d\n", g_Checks, g_Failures );
	return g_Checks > 0 && g_Failures == 0 ? 0 : 1;
}
