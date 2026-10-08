//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 on render.device.webgpu (RFC 0029 W3): the
//			shared suite on the WebGPU adapter, plainly and with every WebGPU
//			validation error counted (there must be none), plus the adapter's
//			own clauses: facts, held submissions and a device created by its
//			descriptor; and the suite's verdict on two deliberately bad
//			configurations (sensitivity).
//
//			Built against the pinned Dawn (quality/toolchain/webgpu.json) and
//			run natively, or with the pinned Emscripten against emdawnwebgpu
//			and run in a headless browser, by tools/render/webgpu_lane.py.
//
//=============================================================================//

#include "device_conformance.h"
#include "render/device/webgpu/provider.h"
#include "spv/device_fixtures_wgsl.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

#if defined( __EMSCRIPTEN__ )
#include <emscripten/emscripten.h>
#endif

namespace
{

using namespace render::device;

struct Fixture
{
	const std::uint32_t *spirv;
	std::string_view wgsl;
};

// Each suite fixture's SPIR-V and its WGSL artifact.
const Fixture kFixtures[] = {
    { rendertest::shaders::kFullScreenVertex, rendertest::wgsl::kFullScreenVertex },
    { rendertest::shaders::kTopHalfVertex, rendertest::wgsl::kTopHalfVertex },
    { rendertest::shaders::kColorFragment, rendertest::wgsl::kColorFragment },
    { rendertest::shaders::kConstantFragment, rendertest::wgsl::kConstantFragment },
    { rendertest::shaders::kSpecializedFragment, rendertest::wgsl::kSpecializedFragment },
    { rendertest::shaders::kDoubleCompute, rendertest::wgsl::kDoubleCompute },
    { rendertest::shaders::kSampledFragment, rendertest::wgsl::kSampledFragment },
    { rendertest::shaders::kComparisonFragment, rendertest::wgsl::kComparisonFragment },
    { rendertest::shaders::kPositionVertex, rendertest::wgsl::kPositionVertex },
};

std::span<const std::byte> Artifact( std::span<const std::uint32_t> spirv )
{
	for ( const Fixture &fixture : kFixtures )
	{
		if ( fixture.spirv == spirv.data() )
			return std::as_bytes( std::span<const char>( fixture.wgsl ) );
	}
	return {};
}

rendertest::DeviceDriver Driver( std::string name, webgpu::WebGpuAdapterOptions options )
{
	rendertest::DeviceDriver driver;
	driver.name = std::move( name );
	driver.create = [options]() -> std::unique_ptr<IRenderDevice2>
	{
		auto device = webgpu::Create( options );
		if ( !device )
			std::fprintf( stderr, "webgpu: device creation failed (status %d, native %d)\n",
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
#if defined( __EMSCRIPTEN__ )
			// The browser delivers WebGPU's callbacks from its event loop:
			// yield to it (JSPI) instead of sleeping.
			emscripten_sleep( 1 );
#else
			std::this_thread::sleep_for( std::chrono::microseconds( 200 ) );
#endif
		}
		(void)device.Poll();
		return true;
	};
	driver.lose = []( IRenderDevice2 &device )
	{
		webgpu::SimulateDeviceLoss( device );
	};
	driver.hold = []( IRenderDevice2 &device, bool held )
	{
		webgpu::HoldSubmissions( device, held );
	};
	driver.rasterizes = true;
	driver.doubleCompute = rendertest::shaders::kDoubleCompute;
	driver.artifact = Artifact;
	return driver;
}

void AdapterClauses( testing::Checks &checks )
{
	auto created = webgpu::Describe().create( DeviceRequest{} );
	checks.That( created.HasValue(), "webgpu.descriptor-creates" );
	if ( !created )
		return;
	const DeviceFacts &facts = created.Value()->Facts();
	std::printf(
	    "webgpu adapter: %.*s\n", int( facts.adapterName.size() ), facts.adapterName.data() );
	std::string claimed;
	for ( std::uint32_t bit = 0; bit < std::uint32_t( Capability::kCount ); ++bit )
	{
		if ( facts.capabilities.Has( Capability( bit ) ) )
			claimed +=
			    std::string( claimed.empty() ? "" : ", " ) + CapabilityName( Capability( bit ) );
	}
	std::printf( "webgpu capabilities: %s\n", claimed.c_str() );
	checks.That( facts.artifactFormat == ArtifactFormat::kWgsl, "webgpu.facts.wgsl" );
	checks.That( facts.diagnosticBackend == "webgpu", "webgpu.facts.backend" );
	checks.That( facts.capabilities.Has( Capability::kCompute ) &&
	                 !facts.capabilities.Has( Capability::kParallelRecording ) &&
	                 !facts.capabilities.Has( Capability::kTimestamps ),
	    "webgpu.facts.claims" );
	checks.That( facts.limits.sampleCounts == ( 1u | 4u ), "webgpu.facts.sample-counts" );
	// A capability the device has but the profile masks is not claimed.
	webgpu::WebGpuAdapterOptions masked;
	masked.allowed = CapabilitySet::All().Remove( Capability::kCompute );
	auto device = webgpu::Create( masked );
	checks.That(
	    device.HasValue() && !device.Value()->Facts().capabilities.Has( Capability::kCompute ),
	    "webgpu.facts.masked-capability" );
	auto missing = webgpu::Describe().create( { CapabilitySet{ Capability::kRayQuery }, false } );
	checks.That( !missing.HasValue() && missing.Error().status == DeviceStatus::kUnsupported,
	    "webgpu.descriptor-refuses-missing-required" );
}

} // namespace

int main()
{
	testing::Checks checks;
	rendertest::RunDeviceConformance( checks, Driver( "webgpu", {} ) );
	std::atomic<std::uint64_t> messages{ 0 };
	webgpu::WebGpuAdapterOptions validated;
	validated.validation = true;
	validated.validationCounter = &messages;
	rendertest::RunDeviceConformance( checks, Driver( "webgpu-validation", validated ) );
	checks.Equal( messages.load(), std::uint64_t( 0 ), "webgpu.validation-silent" );
	AdapterClauses( checks );
	// Sensitivity: each bad configuration must fail the shared suite (its
	// output goes to a scratch stream, so only the verdict is reported here).
	auto caught = [&]( const char *name, webgpu::WebGpuAdapterOptions bad )
	{
		std::FILE *sink = std::tmpfile();
		testing::Checks local( sink ? sink : stdout );
		rendertest::RunDeviceConformance( local, Driver( name, bad ) );
		if ( sink )
			std::fclose( sink );
		checks.That(
		    local.Failures() > 0, ( std::string( "webgpu.sensitivity." ) + name ).c_str() );
	};
	webgpu::WebGpuAdapterOptions early;
	early.sensitivity.skipReleaseWait = true;
	caught( "release-before-token", early );
	webgpu::WebGpuAdapterOptions eager;
	eager.sensitivity.completeOnSubmit = true;
	caught( "complete-on-submit", eager );
	return checks.Report();
}
