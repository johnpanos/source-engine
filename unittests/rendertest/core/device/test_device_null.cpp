//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 on render.device.null (RFC 0016 K1): the shared
//			suite in manual completion (tokens complete only when the suite
//			runs them) and in the products' on-poll mode, plus the recording
//			adapter's own clauses: the recorded stream is in submission order,
//			and a full upload ring defers instead of overwriting.
//
//=============================================================================//

#include "device_conformance.h"
#include "render/device/null/provider.h"

#include <cstdio>

namespace
{

using namespace render::device;

rendertest::DeviceDriver ManualDriver()
{
	rendertest::DeviceDriver driver;
	driver.name = "null-manual";
	driver.create = []() -> std::unique_ptr<IRenderDevice2>
	{
		null::NullOptions options;
		options.completion = null::CompletionMode::kManual;
		options.uploadRingBytes = 256 * 1024;
		auto device = null::Create( options );
		return device ? std::move( device ).Value() : nullptr;
	};
	driver.complete = []( IRenderDevice2 &device, CompletionToken token )
	{
		null::Control( device )->CompleteThrough( token.queue, token.value );
		return device.IsComplete( token );
	};
	driver.lose = []( IRenderDevice2 &device )
	{
		null::Control( device )->LoseDevice();
	};
	driver.holdsCompletion = true;
	return driver;
}

rendertest::DeviceDriver PollDriver()
{
	rendertest::DeviceDriver driver;
	driver.name = "null-poll";
	driver.create = []() -> std::unique_ptr<IRenderDevice2>
	{
		auto device = null::Describe().create( DeviceRequest{} );
		return device ? std::move( device ).Value() : nullptr;
	};
	driver.complete = []( IRenderDevice2 &device, CompletionToken token )
	{
		(void)device.Poll();
		return device.IsComplete( token );
	};
	driver.lose = []( IRenderDevice2 &device )
	{
		null::Control( device )->LoseDevice();
	};
	return driver;
}

void RecordingClauses( testing::Checks &checks )
{
	null::NullOptions options;
	options.completion = null::CompletionMode::kManual;
	options.uploadRingBytes = 4096;
	auto created = null::Create( options );
	checks.That( created.HasValue(), "null.create" );
	if ( !created )
		return;
	std::unique_ptr<IRenderDevice2> device = std::move( created ).Value();
	null::INullDeviceControl *control = null::Control( *device );
	checks.That( control != nullptr, "null.control" );
	BufferDesc desc;
	desc.size = 3000;
	desc.usages = { ResourceUsage::kCopyDestination };
	auto a = device->CreateBuffer( desc );
	auto b = device->CreateBuffer( desc );
	if ( !a || !b || !control )
		return;
	const std::vector<std::byte> bytes( 3000, std::byte{ 7 } );
	CompletionToken last;
	for ( BufferId buffer : { a.Value(), b.Value() } )
	{
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		encoder.Value().TransitionBuffer(
		    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.Value().WriteBuffer( buffer, 0, bytes );
		last = device->Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} ).Value();
	}
	checks.That( control->Recorded().empty(), "null.nothing-runs-before-completion" );
	checks.Equal(
	    control->DeferredUploads(), std::uint64_t( 1 ), "null.full-ring-defers-the-second-upload" );
	control->CompleteThrough( QueueKind::kGraphics, last.value );
	const auto recorded = control->Recorded();
	checks.Equal( recorded.size(), std::size_t( 4 ), "null.records-every-executed-command" );
	checks.That( recorded.size() == 4 && recorded[0].resource == a.Value().value &&
	                 recorded[1].op == null::RecordedOp::kWriteBuffer &&
	                 recorded[2].resource == b.Value().value &&
	                 recorded[3].submission == last.value,
	    "null.records-in-submission-order" );
	auto missing = null::Describe().create( { CapabilitySet{}, false } );
	checks.That( missing.HasValue(), "null.descriptor-creates" );
}

} // namespace

int main()
{
	testing::Checks checks;
	rendertest::RunDeviceConformance( checks, ManualDriver() );
	rendertest::RunDeviceConformance( checks, PollDriver() );
	RecordingClauses( checks );
	return checks.Report();
}
