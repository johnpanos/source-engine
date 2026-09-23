//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for render feature profiles and quirks
//			(render.profile.v1, RFC 0001 "Capabilities, profiles, and quirks").
//			Positive selections, inclusive quirk ranges, table order, and one
//			negative fixture per structured failure, including invalid quirk
//			entries that do not even match the adapter.
//
//			Build/run: python3 tools/quality/conformance.py check --suite render.profile
//
//=============================================================================//

#include "render/render_profile.h"

#include <cstdio>
#include <cstring>

namespace
{

using render::RenderAdapterInfo;
using render::RenderFeature;
using render::RenderFeatureProfile;
using render::RenderFeatureSet;
using render::RenderProfileError;
using render::RenderProfileRequest;
using render::RenderProfileStatus;
using render::RenderQuirk;
using render::RenderWorkaround;
using render::RenderWorkaroundSet;

int g_Checks = 0;
int g_Failures = 0;

void Check( bool passed, const char *condition, int line )
{
	++g_Checks;
	if ( !passed )
	{
		++g_Failures;
		std::printf( "FAIL line %d: %s\n", line, condition );
	}
}

#define CHECK( condition ) Check( ( condition ), #condition, __LINE__ )

const uint32_t kFloatCubemaps =
    RenderWorkaroundSet::Bit( RenderWorkaround::kFloatNormalizationCubemaps );

uint32_t Bits( RenderFeature a )
{
	return RenderFeatureSet::Bit( a );
}

RenderAdapterInfo Adapter(
    const char *driverApi, uint32_t vendor, uint32_t device, uint64_t driver, uint32_t features )
{
	RenderAdapterInfo info;
	std::snprintf( info.id, sizeof( info.id ), "fixture:0" );
	std::snprintf( info.name, sizeof( info.name ), "Fixture adapter" );
	std::snprintf( info.driverApi, sizeof( info.driverApi ), "%s", driverApi );
	info.vendorId = vendor;
	info.deviceId = device;
	info.driverVersion = driver;
	info.supportedFeatures.bits = features;
	return info;
}

RenderQuirk Quirk( const char *id, const char *backend, const char *api, uint32_t vendor,
    uint32_t removed, uint32_t workarounds )
{
	RenderQuirk quirk = { id, "fixture reason", backend, api, vendor, 0,
	    render::kRenderAnyDeviceIdMax, 0, render::kRenderAnyDriverVersionMax, removed,
	    workarounds };
	return quirk;
}

RenderProfileRequest Request( uint32_t required, uint32_t preferred )
{
	RenderProfileRequest request;
	request.required.bits = required;
	request.preferred.bits = preferred;
	return request;
}

// A profile the selection must leave untouched when it fails.
RenderFeatureProfile Sentinel()
{
	RenderFeatureProfile profile;
	profile.enabled.bits = 0xA5A5A5A5u;
	profile.workarounds.bits = 0x5Au;
	profile.appliedQuirkCount = 3;
	return profile;
}

bool SameProfile( const RenderFeatureProfile &a, const RenderFeatureProfile &b )
{
	if ( a.enabled != b.enabled || a.workarounds != b.workarounds ||
	     a.appliedQuirkCount != b.appliedQuirkCount )
		return false;
	for ( uint32_t i = 0; i < a.appliedQuirkCount; ++i )
	{
		if ( a.appliedQuirks[i] != b.appliedQuirks[i] )
			return false;
	}
	return true;
}

void CheckPositiveSelection()
{
	const uint32_t srgb = Bits( RenderFeature::kSampledSrgb );
	const uint32_t offscreen = Bits( RenderFeature::kOffscreenRender );
	const uint32_t compute = Bits( RenderFeature::kComputeShaders );
	const RenderAdapterInfo adapter = Adapter( "d3d9", 0x10DE, 0x1000, 100, srgb | offscreen );

	// Prefer-everything with no quirks enables exactly what the adapter supports.
	{
		RenderFeatureProfile profile;
		RenderProfileError error;
		error.status = RenderProfileStatus::kInvalidQuirk;
		const bool ok = render::SelectRenderFeatureProfile( "legacy", adapter,
		    render::PreferAvailableRenderFeatures(), nullptr, 0, &profile, &error );
		CHECK( ok );
		CHECK( profile.enabled.bits == ( srgb | offscreen ) );
		CHECK( profile.workarounds.IsEmpty() );
		CHECK( profile.appliedQuirkCount == 0 );
		CHECK( error.status == RenderProfileStatus::kOk ); // success resets the error
	}

	// The default request requires nothing and prefers every bit.
	{
		const RenderProfileRequest request = render::PreferAvailableRenderFeatures();
		CHECK( request.required.bits == 0 );
		CHECK( request.preferred.bits == 0xFFFFFFFFu );
	}

	// Preferred features the adapter lacks are simply absent; required ones enable.
	{
		RenderFeatureProfile profile;
		const bool ok = render::SelectRenderFeatureProfile( "legacy", adapter,
		    Request( srgb, compute | offscreen ), nullptr, 0, &profile, nullptr );
		CHECK( ok );
		CHECK( profile.enabled.bits == ( srgb | offscreen ) );
	}

	// Nothing requested, nothing enabled.
	{
		RenderFeatureProfile profile;
		CHECK( render::SelectRenderFeatureProfile(
		    "legacy", adapter, Request( 0, 0 ), nullptr, 0, &profile, nullptr ) );
		CHECK( profile.enabled.bits == 0 );
	}

	// Selection is a pure function of its inputs.
	{
		RenderFeatureProfile a;
		RenderFeatureProfile b;
		const RenderQuirk quirks[] = {
		    Quirk( "q.api", nullptr, "d3d9", 0, offscreen, kFloatCubemaps ) };
		CHECK( render::SelectRenderFeatureProfile(
		    "legacy", adapter, render::PreferAvailableRenderFeatures(), quirks, 1, &a, nullptr ) );
		CHECK( render::SelectRenderFeatureProfile(
		    "legacy", adapter, render::PreferAvailableRenderFeatures(), quirks, 1, &b, nullptr ) );
		CHECK( SameProfile( a, b ) );
	}
}

void CheckQuirkApplication()
{
	const uint32_t srgb = Bits( RenderFeature::kSampledSrgb );
	const uint32_t offscreen = Bits( RenderFeature::kOffscreenRender );
	const RenderAdapterInfo gl =
	    Adapter( "opengl", 0x1002, 0x7300, 0x0001000200030004ull, srgb | offscreen );

	// A driverApi quirk enables its workaround, removes a preferred feature and is
	// recorded by pointer into the table.
	{
		const RenderQuirk quirks[] = {
		    Quirk( "q.other-api", nullptr, "vulkan", 0, srgb, kFloatCubemaps ),
		    Quirk( "q.gl", nullptr, "opengl", 0, srgb, kFloatCubemaps ),
		};
		RenderFeatureProfile profile;
		CHECK( render::SelectRenderFeatureProfile(
		    "legacy", gl, render::PreferAvailableRenderFeatures(), quirks, 2, &profile, nullptr ) );
		CHECK( profile.workarounds.Has( RenderWorkaround::kFloatNormalizationCubemaps ) );
		CHECK( profile.enabled.bits == offscreen );
		CHECK( profile.appliedQuirkCount == 1 );
		CHECK( profile.appliedQuirks[0] == &quirks[1] );
	}

	// Backend, vendor, device and driver selectors: each mismatch prevents the match.
	struct SelectorCase
	{
		const char *label;
		RenderQuirk quirk;
		bool matches;
	};
	RenderQuirk backend = Quirk( "q.backend", "legacy", nullptr, 0, 0, kFloatCubemaps );
	RenderQuirk otherBackend =
	    Quirk( "q.backend-other", "native-vulkan", nullptr, 0, 0, kFloatCubemaps );
	RenderQuirk vendor = Quirk( "q.vendor", nullptr, nullptr, 0x1002, 0, kFloatCubemaps );
	RenderQuirk otherVendor =
	    Quirk( "q.vendor-other", nullptr, nullptr, 0x10DE, 0, kFloatCubemaps );
	RenderQuirk deviceLow = Quirk( "q.device-min", nullptr, nullptr, 0x1002, 0, kFloatCubemaps );
	deviceLow.deviceIdMin = 0x7300;
	deviceLow.deviceIdMax = 0x73FF;
	RenderQuirk deviceHigh = deviceLow;
	deviceHigh.id = "q.device-max";
	deviceHigh.deviceIdMin = 0x7200;
	deviceHigh.deviceIdMax = 0x7300;
	RenderQuirk deviceOutside = deviceLow;
	deviceOutside.id = "q.device-outside";
	deviceOutside.deviceIdMin = 0x7301;
	RenderQuirk driverExact = Quirk( "q.driver-exact", nullptr, "opengl", 0, 0, kFloatCubemaps );
	driverExact.driverVersionMin = 0x0001000200030004ull;
	driverExact.driverVersionMax = 0x0001000200030004ull;
	RenderQuirk driverBelow = driverExact;
	driverBelow.id = "q.driver-below";
	driverBelow.driverVersionMin = 0;
	driverBelow.driverVersionMax = 0x0001000200030003ull;
	RenderQuirk emptyApi = Quirk( "q.empty-api", nullptr, "", 0, 0, kFloatCubemaps );

	const SelectorCase cases[] = {
	    { "backend", backend, true },
	    { "other backend", otherBackend, false },
	    { "vendor", vendor, true },
	    { "other vendor", otherVendor, false },
	    { "device range starts at the device", deviceLow, true },
	    { "device range ends at the device", deviceHigh, true },
	    { "device range excludes the device", deviceOutside, false },
	    { "exact driver version", driverExact, true },
	    { "driver range below", driverBelow, false },
	    { "empty driverApi selector matches only an empty api", emptyApi, false },
	};
	for ( size_t i = 0; i < sizeof( cases ) / sizeof( cases[0] ); ++i )
	{
		RenderFeatureProfile profile;
		const bool ok = render::SelectRenderFeatureProfile( "legacy", gl,
		    render::PreferAvailableRenderFeatures(), &cases[i].quirk, 1, &profile, nullptr );
		const bool matched = profile.appliedQuirkCount == 1;
		if ( !ok || matched != cases[i].matches )
			std::printf( "FAIL selector case '%s'\n", cases[i].label );
		CHECK( ok && matched == cases[i].matches );
		CHECK( render::RenderQuirkMatches( cases[i].quirk, "legacy", gl ) == cases[i].matches );
	}

	// Several matching quirks apply in table order and their workarounds combine.
	{
		const RenderQuirk quirks[] = {
		    Quirk( "q.first", "legacy", nullptr, 0, srgb, 0 ),
		    Quirk( "q.second", nullptr, "opengl", 0, 0, kFloatCubemaps ),
		};
		RenderFeatureProfile profile;
		CHECK( render::SelectRenderFeatureProfile(
		    "legacy", gl, render::PreferAvailableRenderFeatures(), quirks, 2, &profile, nullptr ) );
		CHECK( profile.appliedQuirkCount == 2 );
		CHECK( profile.appliedQuirks[0] == &quirks[0] && profile.appliedQuirks[1] == &quirks[1] );
		CHECK( profile.enabled.bits == offscreen );
		CHECK( profile.workarounds.bits == kFloatCubemaps );
	}
}

struct FailureCase
{
	const char *label;
	RenderProfileStatus status;
};

void ExpectFailure( const FailureCase &expected, const char *backendId,
    const RenderAdapterInfo &adapter, const RenderProfileRequest &request,
    const RenderQuirk *quirks, size_t count, RenderFeature feature, const RenderQuirk *quirk )
{
	RenderFeatureProfile profile = Sentinel();
	RenderProfileError error;
	const bool ok = render::SelectRenderFeatureProfile(
	    backendId, adapter, request, quirks, count, &profile, &error );
	const bool statusOk = error.status == expected.status;
	if ( ok || !statusOk )
		std::printf( "FAIL failure case '%s': ok=%d status=%u\n", expected.label, ok ? 1 : 0,
		    static_cast<unsigned int>( error.status ) );
	CHECK( !ok && statusOk );
	CHECK( SameProfile( profile, Sentinel() ) );
	CHECK( error.message[0] != '\0' );
	if ( expected.status == RenderProfileStatus::kMissingRequiredFeature ||
	     expected.status == RenderProfileStatus::kRemovedByQuirk )
		CHECK( error.feature == feature );
	if ( expected.status == RenderProfileStatus::kInvalidQuirk ||
	     expected.status == RenderProfileStatus::kRemovedByQuirk )
		CHECK( error.quirk == quirk );
	// A null error pointer is permitted on every failure path.
	RenderFeatureProfile again = Sentinel();
	CHECK( !render::SelectRenderFeatureProfile(
	    backendId, adapter, request, quirks, count, &again, nullptr ) );
}

void CheckFailures()
{
	const uint32_t srgb = Bits( RenderFeature::kSampledSrgb );
	const uint32_t offscreen = Bits( RenderFeature::kOffscreenRender );
	const uint32_t compute = Bits( RenderFeature::kComputeShaders );
	const uint32_t msaa = Bits( RenderFeature::kMultiSample4x );
	const RenderAdapterInfo adapter = Adapter( "d3d9", 0x8086, 0x3E92, 7, srgb | offscreen );
	const RenderProfileRequest prefer = render::PreferAvailableRenderFeatures();

	// Invalid quirk entries fail every selection, even entries that do not match
	// this adapter, and the error points at the offending entry.
	RenderQuirk valid = Quirk( "q.valid", nullptr, "vulkan", 0, 0, kFloatCubemaps );
	RenderQuirk invalid[9];
	for ( int i = 0; i < 9; ++i )
		invalid[i] = valid;
	invalid[0].id = nullptr;
	invalid[1].id = "";
	invalid[2].reason = nullptr;
	invalid[3].reason = "";
	invalid[4].driverApi = nullptr; // no selector at all
	invalid[5].deviceIdMin = 2;
	invalid[5].deviceIdMax = 1;
	invalid[6].driverVersionMin = 9;
	invalid[6].driverVersionMax = 8;
	invalid[7].enabledWorkaroundBits = 0;        // no effect
	invalid[8].enabledWorkaroundBits = 1u << 30; // unknown workaround
	for ( int i = 0; i < 9; ++i )
	{
		CHECK( !render::IsValidRenderQuirk( invalid[i] ) );
		const RenderQuirk table[] = { valid, invalid[i] };
		char label[64];
		std::snprintf( label, sizeof( label ), "invalid quirk %d", i );
		ExpectFailure( { label, RenderProfileStatus::kInvalidQuirk }, "legacy", adapter, prefer,
		    table, 2, RenderFeature::kNeverSupported, &table[1] );
	}
	CHECK( render::IsValidRenderQuirk( valid ) );

	// A required feature the adapter lacks is named (the lowest missing bit).
	ExpectFailure( { "missing required", RenderProfileStatus::kMissingRequiredFeature }, "legacy",
	    adapter, Request( srgb | msaa | compute, 0 ), nullptr, 0, RenderFeature::kComputeShaders,
	    nullptr );

	// A quirk that removes a required feature fails with both the feature and quirk.
	{
		const RenderQuirk table[] = { Quirk( "q.removes-srgb", nullptr, "d3d9", 0, srgb, 0 ) };
		ExpectFailure( { "removed by quirk", RenderProfileStatus::kRemovedByQuirk }, "legacy",
		    adapter, Request( srgb, offscreen ), table, 1, RenderFeature::kSampledSrgb, &table[0] );
	}

	// More matching quirks than a profile can record.
	{
		RenderQuirk table[render::kRenderMaxAppliedQuirks + 1];
		for ( uint32_t i = 0; i <= render::kRenderMaxAppliedQuirks; ++i )
			table[i] = Quirk( "q.many", nullptr, "d3d9", 0, 0, kFloatCubemaps );
		ExpectFailure( { "too many quirks", RenderProfileStatus::kTooManyQuirks }, "legacy",
		    adapter, prefer, table, render::kRenderMaxAppliedQuirks + 1,
		    RenderFeature::kNeverSupported, nullptr );
		// Exactly the capacity is still accepted.
		RenderFeatureProfile profile;
		CHECK( render::SelectRenderFeatureProfile( "legacy", adapter, prefer, table,
		    render::kRenderMaxAppliedQuirks, &profile, nullptr ) );
		CHECK( profile.appliedQuirkCount == render::kRenderMaxAppliedQuirks );
	}

	// Malformed calls.
	ExpectFailure( { "null backend id", RenderProfileStatus::kInvalidProvider }, nullptr, adapter,
	    prefer, nullptr, 0, RenderFeature::kNeverSupported, nullptr );
	ExpectFailure( { "empty backend id", RenderProfileStatus::kInvalidProvider }, "", adapter,
	    prefer, nullptr, 0, RenderFeature::kNeverSupported, nullptr );
	ExpectFailure( { "count without table", RenderProfileStatus::kInvalidProvider }, "legacy",
	    adapter, prefer, nullptr, 1, RenderFeature::kNeverSupported, nullptr );
	{
		RenderProfileError error;
		CHECK( !render::SelectRenderFeatureProfile(
		    "legacy", adapter, prefer, nullptr, 0, nullptr, &error ) );
		CHECK( error.status == RenderProfileStatus::kInvalidProvider );
	}
}

// The adapter description's new driver facts take part in value equality, so a
// captured description cannot silently diverge from a re-query.
void CheckAdapterValueEquality()
{
	const RenderAdapterInfo base = Adapter( "d3d9", 1, 2, 3, 0 );
	RenderAdapterInfo changed = base;
	CHECK( changed == base );
	changed.deviceId = 9;
	CHECK( changed != base );
	changed = base;
	changed.driverVersion = 9;
	CHECK( changed != base );
	changed = base;
	std::snprintf( changed.driverApi, sizeof( changed.driverApi ), "vulkan" );
	CHECK( changed != base );
}

} // namespace

int main()
{
	CheckPositiveSelection();
	CheckQuirkApplication();
	CheckFailures();
	CheckAdapterValueEquality();
	std::printf( "CONFORMANCE %d %d\n", g_Checks, g_Failures );
	return g_Checks > 0 && g_Failures == 0 ? 0 : 1;
}
