//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 on render.device.d3d12 (RFC 0024 X1): the shared
//			suite on the Direct3D 12 adapter, in the default and a small-ring
//			configuration and under the debug layer, plus the adapter's own
//			clauses: facts, held submissions and a device created by its
//			descriptor (RFC 0024 X1-X2).
//
//			Built with MinGW-w64 and run under Wine with vkd3d-proton by
//			tools/render/d3d12_lane.py.
//
//=============================================================================//

#include "device_conformance.h"
#include "render/device/d3d12/provider.h"
#include "spv/device_fixtures_hlsl.h"

#include <chrono>
#include <cstdio>
#include <thread>

namespace
{

using namespace render::device;

struct Fixture
{
	const std::uint32_t *spirv;
	std::string_view hlsl;
};

// Each suite fixture's SPIR-V and its HLSL artifact.
const Fixture kFixtures[] = {
    { rendertest::shaders::kFullScreenVertex, rendertest::hlsl::kFullScreenVertex },
    { rendertest::shaders::kTopHalfVertex, rendertest::hlsl::kTopHalfVertex },
    { rendertest::shaders::kColorFragment, rendertest::hlsl::kColorFragment },
    { rendertest::shaders::kConstantFragment, rendertest::hlsl::kConstantFragment },
    { rendertest::shaders::kSpecializedFragment, rendertest::hlsl::kSpecializedFragment },
    { rendertest::shaders::kDoubleCompute, rendertest::hlsl::kDoubleCompute },
    { rendertest::shaders::kSampledFragment, rendertest::hlsl::kSampledFragment },
    { rendertest::shaders::kComparisonFragment, rendertest::hlsl::kComparisonFragment },
    { rendertest::shaders::kPositionVertex, rendertest::hlsl::kPositionVertex },
};

std::span<const std::byte> Artifact( std::span<const std::uint32_t> spirv )
{
	for ( const Fixture &fixture : kFixtures )
	{
		if ( fixture.spirv == spirv.data() )
			return std::as_bytes( std::span<const char>( fixture.hlsl ) );
	}
	return {};
}

rendertest::DeviceDriver Driver( std::string name, d3d12::D3d12AdapterOptions options )
{
	rendertest::DeviceDriver driver;
	driver.name = std::move( name );
	driver.create = [options]() -> std::unique_ptr<IRenderDevice2>
	{
		auto device = d3d12::Create( options );
		if ( !device )
			std::fprintf( stderr, "d3d12: device creation failed (status %d, native %d)\n",
			    int( device.Error().status ), int( device.Error().nativeCode ) );
		return device ? std::move( device ).Value() : nullptr;
	};
	driver.complete = []( IRenderDevice2 &device, CompletionToken token )
	{
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 10 );
		while ( !device.IsComplete( token ) )
		{
			if ( std::chrono::steady_clock::now() > deadline )
				return false;
			std::this_thread::sleep_for( std::chrono::microseconds( 200 ) );
		}
		(void)device.Poll();
		return true;
	};
	driver.lose = []( IRenderDevice2 &device )
	{
		d3d12::SimulateDeviceLoss( device );
	};
	driver.hold = []( IRenderDevice2 &device, bool held )
	{
		d3d12::HoldSubmissions( device, held );
	};
	driver.rasterizes = true;
	driver.doubleCompute = rendertest::shaders::kDoubleCompute;
	driver.artifact = Artifact;
	return driver;
}

void AdapterClauses( testing::Checks &checks )
{
	auto created = d3d12::Describe().create( DeviceRequest{} );
	checks.That( created.HasValue(), "d3d12.descriptor-creates" );
	if ( !created )
		return;
	const DeviceFacts &facts = created.Value()->Facts();
	std::printf( "d3d12 adapter: %.*s\n", int( facts.adapterName.size() ),
	    facts.adapterName.data() );
	checks.That( facts.artifactFormat == ArtifactFormat::kHlsl, "d3d12.facts.hlsl" );
	checks.That( facts.diagnosticBackend == "d3d12", "d3d12.facts.backend" );
	checks.That( facts.capabilities.Has( Capability::kCompute ) &&
	                 !facts.capabilities.Has( Capability::kIndirectFirstInstance ),
	    "d3d12.facts.claims" );
	// A capability the device has but the profile masks is not claimed.
	d3d12::D3d12AdapterOptions masked;
	masked.allowed = CapabilitySet::All().Remove( Capability::kTimestamps );
	auto device = d3d12::Create( masked );
	d3d12::D3d12AdapterOptions noLines;
	noLines.allowed = CapabilitySet::All().Remove( Capability::kFillModeLines );
	rendertest::RunDeviceConformance( checks, Driver( "d3d12-no-line-fill", noLines ) );
	checks.That( device.HasValue() &&
	                 !device.Value()->Facts().capabilities.Has( Capability::kTimestamps ),
	    "d3d12.facts.masked-capability" );
	auto missing = d3d12::Describe().create( { CapabilitySet{ Capability::kRayQuery }, false } );
	checks.That( !missing.HasValue() && missing.Error().status == DeviceStatus::kUnsupported,
	    "d3d12.descriptor-refuses-missing-required" );
}

} // namespace

int main()
{
	testing::Checks checks;
	rendertest::RunDeviceConformance( checks, Driver( "d3d12", {} ) );
	d3d12::D3d12AdapterOptions small;
	small.uploadRingBytes = 256 * 1024;
	rendertest::RunDeviceConformance( checks, Driver( "d3d12-small-ring", small ) );
	std::atomic<std::uint64_t> messages{ 0 };
	d3d12::D3d12AdapterOptions validated;
	validated.validation = true;
	validated.validationCounter = &messages;
	rendertest::RunDeviceConformance( checks, Driver( "d3d12-validation", validated ) );
	checks.Equal( messages.load(), std::uint64_t( 0 ), "d3d12.validation-silent" );
	AdapterClauses( checks );
	// Sensitivity: each bad adapter must fail the shared suite (its output
	// goes to a scratch stream, so only the verdict is reported here).
	auto caught = [&]( const char *name, d3d12::D3d12AdapterOptions bad )
	{
		std::FILE *sink = std::tmpfile();
		testing::Checks local( sink ? sink : stdout );
		rendertest::RunDeviceConformance( local, Driver( name, bad ) );
		if ( sink )
			std::fclose( sink );
		checks.That( local.Failures() > 0, ( std::string( "d3d12.sensitivity." ) + name ).c_str() );
	};
	d3d12::D3d12AdapterOptions reuse;
	reuse.uploadRingBytes = 256 * 1024; // D10 stages 96 KiB uploads
	reuse.sensitivity.unsafeUploadReuse = true;
	caught( "unsafe-upload-reuse", reuse );
	d3d12::D3d12AdapterOptions early;
	early.sensitivity.skipReleaseWait = true;
	caught( "release-before-token", early );
	return checks.Report();
}
