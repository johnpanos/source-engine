//========= Copyright Valve Corporation, All rights reserved. ============//
// Native measurement oracle for the VGUI cost overlay (RFC 0014).
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/graph/pass_timers.h"

#include <atomic>
#include <chrono>
#include <thread>

namespace render::lab
{
namespace
{
std::optional<std::string> RunOnce(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counted{ 0 };
	std::unique_ptr<device::IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counted, device ) )
		return why;
	{
		const auto initial = device->ReadResourceActivity();
		results.That( initial.supported, "resources.native-provider-supported" );
		device::BufferDesc bufferDesc;
		bufferDesc.size = 128;
		bufferDesc.usages = { device::ResourceUsage::kCopyDestination };
		auto allocated = device->CreateBuffer( bufferDesc );
		if ( !allocated )
			return "resource fixture buffer refused";
		const auto created =
		    device::ResourceActivitySince( initial, device->ReadResourceActivity() );
		results.That( created.Created() == 1 && created.bufferBytes == 128 &&
		                  created.live == initial.live + 1,
		    "resources.creation-and-buffer-bytes" );
		results.That( !device->CreateBuffer( {} ), "resources.invalid-allocation-refused" );
		results.That( device->ReadResourceActivity().Created() == initial.Created() + 1,
		    "resources.failed-allocation-not-counted" );
		results.That(
		    bool( device->Release( allocated.Value(), {} ) ), "resources.release-accepted" );
		const auto retired =
		    device::ResourceActivitySince( initial, device->ReadResourceActivity() );
		results.That( retired.releaseRequests == 1 && retired.Destroyed() == 0 &&
		                  retired.pending == initial.pending + 1 &&
		                  retired.live == initial.live + 1,
		    "resources.release-is-not-destruction" );
		results.That(
		    !device->Release( allocated.Value(), {} ), "resources.double-release-refused" );
		device->Poll();
		const auto freed = device::ResourceActivitySince( initial, device->ReadResourceActivity() );
		results.That( freed.Destroyed() == 1 && freed.releaseRequests == 1 &&
		                  freed.pending == initial.pending && freed.live == initial.live,
		    "resources.completed-retirement" );
		auto otherEpoch = initial;
		++otherEpoch.epoch;
		results.That( !device::ResourceActivitySince( initial, otherEpoch ).supported,
		    "resources.recovery-is-discontinuity" );
		graph::GpuPassTimers timers( *device );
		if ( !timers.Supported() )
			return "native timestamp device required";
		graph::GraphBuilder builder;
		device::TextureDesc desc;
		desc.width = desc.height = 64;
		desc.format = device::Format::kRGBA8Unorm;
		desc.usages = { device::ResourceUsage::kColorAttachment };
		const auto target = builder.CreateTexture( "cost fixture", desc );
		builder.AddPass( "cost fixture", graph::PassKind::kRender )
		    .Write( target, device::ResourceUsage::kColorAttachment )
		    .SideEffect()
		    .Execute(
		        [target]( graph::RecordContext &context )
		        {
			        auto &encoder = context.Encoder();
			        encoder.BeginLabel( "clear" );
			        // Known CPU-only work: it must occur in CPU recording time.
			        std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
			        const device::ColorAttachment colors[] = { { context.Texture( target ),
			            device::LoadOp::kClear, device::StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
			        device::RenderingDesc rendering;
			        rendering.colors = colors;
			        rendering.width = rendering.height = 64;
			        encoder.BeginRendering( rendering );
			        // More than one timestamp chunk inside rendering, as the
			        // detailed world-family scopes do. Validation catches illegal
			        // query-buffer transitions at a chunk boundary.
			        for ( int i = 0; i < 12; ++i )
			        {
				        encoder.BeginLabel( "cohort" );
				        encoder.EndLabel();
			        }
			        encoder.EndRendering();
			        encoder.EndLabel();
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( !compiled )
			return "cost fixture graph did not compile";
		graph::SerialGraphExecutor executor;
		executor.SetLabelObserver( &timers );
		timers.BeginFrame( 42, {} );
		auto executed = executor.Execute( compiled.Value(), *device );
		if ( !executed )
			return "cost fixture did not execute";
		results.That( timers.Latest().frames == 0, "cost.no-sample-before-submission-token" );
		timers.EndFrame( executed.Value().token );
		if ( !device->WaitIdle() )
			return "cost fixture did not complete";
		const auto latest = timers.Latest();
		results.That(
		    latest.frames == 1 && latest.lastFrame == 42, "cost.completed-frame-identity" );
		results.That( latest.passes.size() == 3, "cost.parent-child-and-cohort-recorded" );
		if ( latest.passes.size() == 3 )
		{
			const auto &parent = latest.passes[0];
			const auto &child = latest.passes[1];
			results.That(
			    parent.cpuMilliseconds >= child.cpuMilliseconds && child.cpuMilliseconds >= 1,
			    "cost.cpu-only-work-and-inclusive-parent" );
			results.That( parent.milliseconds >= child.milliseconds && child.milliseconds >= 0,
			    "cost.gpu-nested-timestamps" );
			results.That( child.depth == 1 && parent.depth == 0, "cost.nesting-depth" );
		}
		results.That( latest.resources.supported && latest.resources.Created() > 0,
		    "cost.frame-resource-interval" );
		results.That( latest.passes.size() == 3 && latest.passes[2].count == 12 &&
		                  latest.passes[2].depth == 2 && latest.overflowed == 0,
		    "cost.repeated-cohort-across-timestamp-chunks" );
		results.That( timers.Recent().size() == 1 && timers.Recent()[0].lastFrame == 42,
		    "cost.completed-resource-history" );
		results.That( timers.Take().passes.size() == latest.passes.size(),
		    "cost.snapshot-does-not-drain-console" );
		results.That( timers.Take().frames == 0 && timers.Latest().lastFrame == 42,
		    "cost.console-does-not-drain-snapshot" );
	}
	device.reset();
	messages = counted.load();
	return std::nullopt;
}
} // namespace
int RunCostOverlaySuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "cost-overlay", {}, RunOnce );
}
} // namespace render::lab
