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
		results.That( latest.passes.size() == 2, "cost.parent-and-child-recorded" );
		if ( latest.passes.size() == 2 )
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
