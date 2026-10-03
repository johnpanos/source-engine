//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 (RFC 0016 K2, first slice) on render.device.null:
//
//			G1 culling keeps side effects and imported outputs, drops the rest;
//			G2 transitions of a hand-built graph are exactly as specified;
//			G3-G5 bad graphs are rejected with their status;
//			G6 execution records passes in order and releases transients behind
//			   the submission token;
//			G7 on seeded random graphs, the compiler agrees with an independent
//			   reference model written here (fixpoint culling, resource-major
//			   transitions), and the null device, which validates every
//			   transition against its own state, accepts every execution.
//
//			The Python model and the pooled executor of RFC 0016 K2 are not
//			part of this slice.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/pass_timers.h"
#include "render/graph/scene_color.h"
#include "render/graph/validate.h"
#include "graph_fixtures.h"
#include "jobsystem/parallel_executor.h"
#include "testing/checks.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <random>
#include <tuple>

namespace
{

namespace fixtures = rendertest::graph;
using namespace render;
using namespace render::graph;
using device::ResourceUsage;

std::unique_ptr<device::IRenderDevice2> ManualDevice()
{
	device::null::NullOptions options;
	options.completion = device::null::CompletionMode::kManual;
	return device::null::Create( options ).Value();
}

device::TextureDesc Color( std::uint32_t size = 16 )
{
	device::TextureDesc desc;
	desc.format = device::Format::kRGBA8Unorm;
	desc.width = size;
	desc.height = size;
	return desc;
}

void Noop( RecordContext & )
{
}

void Culling( testing::Checks &checks )
{
	GraphBuilder builder;
	const ResourceRef unused = builder.CreateTexture( "unused", Color() );
	const ResourceRef scene = builder.CreateTexture( "scene", Color() );
	device::TextureDesc backDesc = Color();
	const ResourceRef back = builder.ImportTexture( "back", device::TextureId{ 99 }, backDesc,
	    ResourceUsage::kUndefined, ResourceUsage::kPresent );
	builder.AddPass( "dead", PassKind::kRender )
	    .Write( unused, ResourceUsage::kColorAttachment )
	    .Execute( Noop );
	builder.AddPass( "scene", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .Execute( Noop );
	builder.AddPass( "post", PassKind::kRender )
	    .Read( scene, ResourceUsage::kSampled )
	    .Write( back, ResourceUsage::kColorAttachment )
	    .Execute( Noop );
	builder.AddPass( "capture", PassKind::kCopy )
	    .Read( scene, ResourceUsage::kCopySource )
	    .SideEffect()
	    .Execute( Noop );
	auto graph = CompileGraph( std::move( builder ) );
	checks.That( graph.HasValue(), "G1.compiles" );
	if ( !graph )
		return;
	checks.That( graph.Value().trace.kept == std::vector<std::uint32_t>{ 1, 2, 3 },
	    "G1.keeps-writers-of-needed-and-side-effect-passes" );
	checks.That( graph.Value().trace.culled == std::vector<std::uint32_t>{ 0 },
	    "G1.culls-a-pass-nothing-needs" );
	checks.Equal(
	    graph.Value().transients.size(), std::size_t( 1 ), "G1.culled-transients-are-not-created" );
}

void Transitions( testing::Checks &checks )
{
	GraphBuilder builder;
	const ResourceRef scene = builder.CreateTexture( "scene", Color() );
	const ResourceRef back = builder.ImportTexture(
	    "back", device::TextureId{ 7 }, Color(), ResourceUsage::kPresent, ResourceUsage::kPresent );
	builder.AddPass( "opaque", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .Execute( Noop );
	builder.AddPass( "translucent", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .Execute( Noop );
	builder.AddPass( "present", PassKind::kRender )
	    .Read( scene, ResourceUsage::kSampled )
	    .Write( back, ResourceUsage::kColorAttachment )
	    .Execute( Noop );
	auto graph = CompileGraph( std::move( builder ) );
	if ( !checks.That( graph.HasValue(), "G2.compiles" ) )
		return;
	const std::vector<TraceTransition> expected = {
	    { 0, 0, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment },
	    { 1, 0, ResourceUsage::kColorAttachment, ResourceUsage::kColorAttachment },
	    { 2, 0, ResourceUsage::kColorAttachment, ResourceUsage::kSampled },
	    { 2, 1, ResourceUsage::kPresent, ResourceUsage::kColorAttachment },
	    { UINT32_MAX, 1, ResourceUsage::kColorAttachment, ResourceUsage::kPresent } };
	checks.That( graph.Value().trace.transitions == expected,
	    "G2.transitions-discard-first-order-writes-and-restore-imports" );
	if ( graph.Value().trace.transitions != expected )
		std::printf( "%s", graph.Value().trace.ToString().c_str() );
	checks.That(
	    graph.Value().transients.size() == 1 &&
	        graph.Value().transients[0].usages ==
	            device::UsageSet{ ResourceUsage::kColorAttachment, ResourceUsage::kSampled },
	    "G2.transient-usages-are-the-union-of-its-accesses" );
}

void BadGraphs( testing::Checks &checks )
{
	{
		GraphBuilder builder;
		const ResourceRef t = builder.CreateTexture( "t", Color() );
		builder.AddPass( "reads", PassKind::kRender )
		    .Read( t, ResourceUsage::kSampled )
		    .SideEffect()
		    .Execute( Noop );
		auto graph = CompileGraph( std::move( builder ) );
		checks.That( !graph && graph.Error().status == GraphStatus::kReadBeforeWrite,
		    "G3.reading-an-unwritten-transient-fails" );
	}
	{
		GraphBuilder builder;
		const ResourceRef t = builder.CreateTexture( "t", Color() );
		builder.AddPass( "both", PassKind::kRender )
		    .Write( t, ResourceUsage::kColorAttachment )
		    .Write( t, ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute( Noop );
		auto graph = CompileGraph( std::move( builder ) );
		checks.That( !graph && graph.Error().status == GraphStatus::kConflictingAccess,
		    "G4.two-usages-of-one-resource-in-a-pass-fail" );
	}
	{
		GraphBuilder builder;
		const ResourceRef t = builder.CreateTexture( "t", Color() );
		builder.AddPass( "lies", PassKind::kRender )
		    .Read( t, ResourceUsage::kColorAttachment )
		    .SideEffect()
		    .Execute( Noop );
		auto graph = CompileGraph( std::move( builder ) );
		checks.That( !graph && graph.Error().status == GraphStatus::kUsageKindMismatch,
		    "G5.a-write-usage-declared-as-a-read-fails" );
	}
	{
		GraphBuilder builder;
		builder.AddPass( "empty", PassKind::kRender ).SideEffect();
		auto graph = CompileGraph( std::move( builder ) );
		checks.That( !graph && graph.Error().status == GraphStatus::kNoExecute,
		    "G5.a-kept-pass-without-execute-fails" );
	}
}

void Execution( testing::Checks &checks )
{
	auto device = ManualDevice();
	auto *control = device::null::Control( *device );
	device::TextureDesc backDesc = Color();
	backDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kPresent };
	const device::TextureId backId = device->CreateTexture( backDesc ).Value();
	const std::size_t baseline = device->LiveResourceCount();

	GraphBuilder builder;
	const ResourceRef scene = builder.CreateTexture( "scene", Color() );
	const ResourceRef back = builder.ImportTexture(
	    "back", backId, backDesc, ResourceUsage::kUndefined, ResourceUsage::kPresent );
	std::vector<std::string> order;
	builder.AddPass( "scene", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .Execute(
	        [&]( RecordContext &context )
	        {
		        order.push_back( "scene" );
		        device::ColorAttachment color[1];
		        color[0].texture = context.Texture( scene );
		        device::RenderingDesc rendering;
		        rendering.colors = color;
		        rendering.width = 16;
		        rendering.height = 16;
		        context.Encoder().BeginRendering( rendering );
		        context.Encoder().EndRendering();
	        } );
	builder.AddPass( "present", PassKind::kRender )
	    .Read( scene, ResourceUsage::kSampled )
	    .Write( back, ResourceUsage::kColorAttachment )
	    .Execute(
	        [&]( RecordContext &context )
	        {
		        order.push_back( "present" );
		        checks.That( context.Texture( back ) == backId, "G6.imports-keep-their-handle" );
	        } );
	auto graph = CompileGraph( std::move( builder ) );
	if ( !checks.That( graph.HasValue(), "G6.compiles" ) )
		return;
	SerialGraphExecutor executor;
	auto result = executor.Execute( graph.Value(), *device );
	if ( !checks.That( result.HasValue(), "G6.the-device-accepts-the-execution" ) )
		return;
	checks.That(
	    order == std::vector<std::string>{ "scene", "present" }, "G6.passes-record-in-order" );
	checks.Equal( result.Value().transients, std::uint32_t( 1 ), "G6.one-transient-created" );
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline + 1,
	    "G6.transients-stay-live-until-the-token-completes" );
	control->CompleteThrough( device::QueueKind::kGraphics, result.Value().token.value );
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "G6.transients-are-released-after" );
	std::size_t transitions = 0;
	for ( const auto &command : control->Recorded() )
		transitions += command.op == device::null::RecordedOp::kTransitionTexture;
	checks.Equal( transitions, std::size_t( result.Value().transitions ),
	    "G6.every-compiled-transition-is-recorded" );
}

void SceneColorCapture( testing::Checks &checks )
{
	GraphBuilder invalid;
	const std::size_t before = invalid.Passes().size();
	checks.That( !CaptureSceneColor( invalid, {} ) && invalid.Passes().size() == before,
	    "G11.invalid-scene-color-does-not-mutate-graph" );
	device::TextureDesc multi = Color( 4 );
	multi.sampleCount = 3;
	const ResourceRef invalidSamples = invalid.CreateTexture( "bad samples", multi );
	checks.That( !CaptureSceneColor( invalid, invalidSamples ) && invalid.Passes().size() == before,
	    "G11.invalid-sample-count-does-not-mutate-graph" );
	GraphBuilder multisampled;
	multi.sampleCount = 4;
	const ResourceRef msaa = multisampled.CreateTexture( "msaa", multi );
	multisampled.AddPass( "opaque", PassKind::kRender )
	    .Write( msaa, ResourceUsage::kColorAttachment )
	    .Execute( Noop );
	const auto resolved = CaptureSceneColor( multisampled, msaa );
	if ( !checks.That(
	         resolved && multisampled.Passes().size() == 4 &&
	             multisampled.Passes()[1].accesses[1].usage == ResourceUsage::kResolveDestination,
	         "G11.multisampled-scene-declares-resolve" ) )
		return;
	multisampled.AddPass( "sample", PassKind::kRender )
	    .Read( *resolved, ResourceUsage::kSampled )
	    .SideEffect()
	    .Execute( Noop );
	checks.That(
	    CompileGraph( std::move( multisampled ) ).HasValue(), "G11.multisampled-capture-compiles" );

	auto device = ManualDevice();
	auto *control = device::null::Control( *device );
	device::BufferDesc readbackDesc;
	readbackDesc.size = 4 * 4 * 4;
	readbackDesc.memory = device::MemoryKind::kReadback;
	readbackDesc.usages = { ResourceUsage::kCopyDestination };
	const auto readbackResult = device->CreateBuffer( readbackDesc );
	if ( !checks.That( readbackResult.HasValue(), "G11.readback-created" ) )
		return;
	const device::BufferId readbackId = readbackResult.Value();
	GraphBuilder builder;
	const ResourceRef scene = builder.CreateTexture( "scene", Color( 4 ) );
	const ResourceRef readback = builder.ImportBuffer( "readback", readbackId, readbackDesc,
	    ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	builder.AddPass( "opaque", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .Execute(
	        [scene]( RecordContext &context )
	        {
		        device::ColorAttachment color;
		        color.texture = context.Texture( scene );
		        color.load = device::LoadOp::kClear;
		        color.clear = { 0.2f, 0.4f, 0.6f, 1.0f };
		        device::RenderingDesc rendering;
		        rendering.colors = std::span( &color, 1 );
		        rendering.width = 4;
		        rendering.height = 4;
		        context.Encoder().BeginRendering( rendering );
		        context.Encoder().EndRendering();
	        } );
	const auto snapshot = CaptureSceneColor( builder, scene );
	if ( !checks.That( snapshot.has_value(), "G11.scene-color-captured" ) )
		return;
	builder.AddPass( "transmission-consumer", PassKind::kRender )
	    .Read( *snapshot, ResourceUsage::kSampled )
	    .SideEffect()
	    .Execute( Noop );
	builder.AddPass( "read-snapshot", PassKind::kCopy )
	    .Read( *snapshot, ResourceUsage::kCopySource )
	    .Write( readback, ResourceUsage::kCopyDestination )
	    .Execute(
	        [snapshot, readback]( RecordContext &context )
	        {
		        context.Encoder().CopyTextureToBuffer(
		            context.Texture( *snapshot ), context.Buffer( readback ), { 0, 0, 0, 4, 4 } );
	        } );
	auto graph = CompileGraph( std::move( builder ) );
	if ( !checks.That( graph.HasValue(), "G11.capture-compiles" ) )
		return;
	checks.Equal( graph.Value().trace.kept.size(), std::size_t( 5 ),
	    "G11.graph-keeps-opaque-capture-stage-consumer" );
	SerialGraphExecutor executor;
	auto result = executor.Execute( graph.Value(), *device );
	if ( !checks.That( result.HasValue(), "G11.capture-executes" ) )
		return;
	control->CompleteThrough( device::QueueKind::kGraphics, result.Value().token.value );
	(void)device->Poll();
	std::array<std::byte, 64> pixels{};
	checks.That( device->ReadBuffer( readbackId, 0, pixels ).HasValue(), "G11.capture-readback" );
	for ( std::size_t pixel = 0; pixel < 16; ++pixel )
	{
		const std::array<std::byte, 4> expected = {
		    std::byte{ 51 }, std::byte{ 102 }, std::byte{ 153 }, std::byte{ 255 } };
		checks.That( std::equal( expected.begin(), expected.end(), pixels.begin() + pixel * 4 ),
		    "G11.snapshot-preserves-scene-pixels" );
	}
	(void)device->Release( readbackId, result.Value().token );
}

// --- RFC 0014 D4: GPU pass timers --------------------------------------------

// Every pass's label is timed, nested labels at their depth; nothing is read
// before the frame's token completes; timestamps do not decrease, and the
// passes' times sum within the frame's span; a device without timestamps
// records none.
void PassTimers( testing::Checks &checks )
{
	auto device = ManualDevice();
	auto *control = device::null::Control( *device );
	GraphBuilder builder;
	const ResourceRef scene = builder.CreateTexture( "scene", Color() );
	auto clear = [scene]( RecordContext &context )
	{
		device::ColorAttachment color[1];
		color[0].texture = context.Texture( scene );
		color[0].load = device::LoadOp::kClear;
		device::RenderingDesc rendering;
		rendering.colors = color;
		rendering.width = 16;
		rendering.height = 16;
		context.Encoder().BeginRendering( rendering );
		context.Encoder().EndRendering();
	};
	builder.AddPass( "first", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .Execute( clear );
	builder.AddPass( "second", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .Execute(
	        [clear]( RecordContext &context )
	        {
		        context.Encoder().BeginLabel( "inner" );
		        clear( context );
		        context.Encoder().EndLabel();
	        } );
	builder.AddPass( "third", PassKind::kRender )
	    .Write( scene, ResourceUsage::kColorAttachment )
	    .SideEffect()
	    .Execute( clear );
	auto graph = CompileGraph( std::move( builder ) );
	if ( !checks.That( graph.HasValue(), "D4.compiles" ) )
		return;
	GpuPassTimers timers( *device );
	checks.That( timers.Supported(), "D4.the-null-device-has-timestamps" );
	SerialGraphExecutor executor;
	executor.SetLabelObserver( &timers );
	timers.BeginFrame( 1, device::CompletionToken() );
	auto result = executor.Execute( graph.Value(), *device );
	if ( !checks.That( result.HasValue(), "D4.a-timed-execution-submits" ) )
		return;
	timers.EndFrame( result.Value().token );
	checks.Equal(
	    timers.Take().frames, std::uint32_t( 0 ), "D4.nothing-is-read-before-completion" );
	checks.Equal( timers.Latest().frames, std::uint32_t( 0 ),
	    "cost.no-snapshot-before-completion" );
	control->CompleteThrough( device::QueueKind::kGraphics, result.Value().token.value );
	const PassTimerReport snapshot = timers.Latest();
	checks.Equal( snapshot.lastFrame, std::uint64_t( 1 ), "cost.snapshot-frame-identity" );
	const PassTimerReport report = timers.Take();
	checks.Equal( snapshot.passes.size(), report.passes.size(),
	    "cost.overlay-does-not-consume-console-report" );
	checks.Equal( timers.Latest().lastFrame, std::uint64_t( 1 ),
	    "cost.console-does-not-consume-overlay-sample" );
	checks.Equal( report.frames, std::uint32_t( 1 ), "D4.the-frame-is-read-after-completion" );
	auto time = [&]( const char *name, std::uint32_t depth ) -> const PassTime *
	{
		for ( const PassTime &pass : report.passes )
		{
			if ( pass.name == name && pass.depth == depth )
				return &pass;
		}
		return nullptr;
	};
	const PassTime *first = time( "first", 0 );
	const PassTime *second = time( "second", 0 );
	const PassTime *inner = time( "inner", 1 );
	const PassTime *third = time( "third", 0 );
	checks.That( first && second && inner && third && report.passes.size() == 4,
	    "D4.every-label-is-timed-at-its-depth" );
	checks.That( first && second && inner && third && first->milliseconds > 0.0 &&
	                 inner->milliseconds > 0.0 && inner->milliseconds <= second->milliseconds,
	    "D4.times-are-positive-and-a-nested-label-within-its-parent" );
	checks.That( first && second && inner && third && first->cpuMilliseconds >= 0 &&
	                 second->cpuMilliseconds >= inner->cpuMilliseconds && inner->cpuMilliseconds > 0,
	    "cost.cpu-recording-has-nested-inclusive-times" );
	std::vector<std::uint64_t> ticks;
	for ( const auto &command : control->Recorded() )
	{
		if ( command.op == device::null::RecordedOp::kWriteTimestamp )
			ticks.push_back( command.count );
	}
	checks.That( ticks.size() == 8 && std::is_sorted( ticks.begin(), ticks.end() ),
	    "D4.timers-are-monotonic" );
	const double span =
	    ticks.empty() ? 0.0 : double( ticks.back() - ticks.front() ) * 1e-6; // 1 ns ticks
	const double sum = ( first ? first->milliseconds : 0.0 ) +
	                   ( second ? second->milliseconds : 0.0 ) +
	                   ( third ? third->milliseconds : 0.0 );
	checks.That( sum > 0.0 && sum <= span, "D4.pass-times-sum-within-the-frame" );

	// Product frames attach many short encoders (one per material/model cohort).
	// A short encoder must not consume the old 64-timestamp allocation: eight
	// two-timestamp sections fit in this bounded 128-timestamp storage budget.
	GpuPassTimers cohorts( *device, 128 );
	cohorts.BeginFrame( 2, result.Value().token );
	device::CompletionToken last;
	for ( int i = 0; i < 8; ++i )
	{
		auto encoder = device->BeginEncoder( device::QueueKind::kGraphics );
		if ( !checks.That( encoder.HasValue(), "D4.short-encoder-begins" ) )
			return;
		cohorts.Attach( encoder.Value() );
		encoder.Value().BeginLabel( "short cohort" );
		encoder.Value().EndLabel();
		cohorts.Detach( encoder.Value() );
		auto submitted = device->Submit( device::QueueKind::kGraphics,
		    std::span<device::CommandEncoder>( &encoder.Value(), 1 ), {} );
		if ( !checks.That( submitted.HasValue(), "D4.short-encoder-submits" ) )
			return;
		last = submitted.Value();
	}
	cohorts.EndFrame( last );
	control->CompleteAll();
	const PassTimerReport cohortReport = cohorts.Take();
	checks.That( cohortReport.frames == 1 && cohortReport.overflowed == 0 &&
	                 cohortReport.passes.size() == 1 && cohortReport.passes[0].count == 8,
	    "D4.short-encoders-fit-the-timestamp-storage-budget" );

	device::null::NullOptions without;
	without.capabilities.Remove( device::Capability::kTimestamps );
	auto plain = device::null::Create( without ).Value();
	GpuPassTimers none( *plain );
	SerialGraphExecutor plainExecutor;
	plainExecutor.SetLabelObserver( &none );
	GraphBuilder again;
	const ResourceRef target = again.CreateTexture( "scene", Color() );
	again.AddPass( "only", PassKind::kRender )
	    .Write( target, ResourceUsage::kColorAttachment )
	    .SideEffect()
	    .Execute( Noop );
	auto plainGraph = CompileGraph( std::move( again ) );
	none.BeginFrame( 1, device::CompletionToken() );
	const bool ran = plainGraph && plainExecutor.Execute( plainGraph.Value(), *plain ).HasValue();
	device::null::Control( *plain )->CompleteAll();
	bool anyTimestamp = false;
	for ( const auto &command : device::null::Control( *plain )->Recorded() )
		anyTimestamp |= command.op == device::null::RecordedOp::kWriteTimestamp;
	checks.That( !none.Supported() && ran && !anyTimestamp && none.Take().frames == 0,
	    "D4.a-device-without-timestamps-records-none" );
}

// --- G7-G10: seeded random graphs ------------------------------------------

std::vector<device::null::RecordedCommand> Stream( device::IRenderDevice2 &device )
{
	auto *control = device::null::Control( device );
	control->CompleteAll();
	const auto recorded = control->Recorded();
	return { recorded.begin(), recorded.end() };
}

void RandomGraphs( testing::Checks &checks, int count )
{
	int agreements = 0;
	int aliased = 0;
	int valid = 0;
	int accepted = 0;
	int releasedAll = 0;
	int sameStream = 0;
	int pooledAccepted = 0;
	jobsystem::ParallelExecutor jobs( 4 );
	for ( int seed = 0; seed < count; ++seed )
	{
		auto device = ManualDevice();
		fixtures::RandomGraph random =
		    fixtures::MakeRandomGraph( static_cast<std::uint32_t>( seed ), *device );
		const std::size_t baseline = device->LiveResourceCount();
		auto graph = CompileGraph( std::move( random.builder ) );
		if ( !graph )
		{
			std::printf( "seed %d: compile failed (%s)\n", seed,
			    DescribeGraphStatus( graph.Error().status ) );
			continue;
		}
		const fixtures::Model model = fixtures::Reference( random.resources, random.passes );
		const bool agrees = model.kept == graph.Value().trace.kept &&
		                    model.transitions == fixtures::SortedTransitions( graph.Value() ) &&
		                    model.aliasSets == graph.Value().trace.aliasSets;
		agreements += agrees;
		if ( !agrees && agreements + 5 > seed )
			std::printf(
			    "seed %d: model disagrees\n%s", seed, graph.Value().trace.ToString().c_str() );
		for ( const auto &set : graph.Value().trace.aliasSets )
			aliased += set.size() > 1;
		valid += ValidateCompiledGraph( graph.Value() ).empty();

		SerialGraphExecutor serial;
		auto result = serial.Execute( graph.Value(), *device );
		const auto serialStream = Stream( *device );
		if ( result )
		{
			++accepted;
			(void)device->Poll();
			releasedAll += device->LiveResourceCount() == baseline;
		}

		// The same graph, recorded on the pool, on a device in the same state.
		auto pooledDevice = ManualDevice();
		fixtures::RandomGraph again =
		    fixtures::MakeRandomGraph( static_cast<std::uint32_t>( seed ), *pooledDevice );
		auto pooledGraph = CompileGraph( std::move( again.builder ) );
		if ( !pooledGraph )
			continue;
		PooledGraphExecutor pooled( jobs );
		auto pooledResult = pooled.Execute( pooledGraph.Value(), *pooledDevice );
		pooledAccepted += pooledResult.HasValue();
		sameStream += pooledResult && Stream( *pooledDevice ) == serialStream;
	}
	checks.Equal( agreements, count, "G7.compiler-agrees-with-the-reference-model" );
	std::printf(
	    "INFO graph: %d of %d random graphs share a physical transient\n", aliased, count );
	checks.That( aliased >= count / 10, "G7.random-graphs-exercise-aliasing" );
	checks.Equal( valid, count, "G8.every-compiled-graph-validates" );
	checks.Equal( accepted, count, "G8.the-device-accepts-every-compiled-execution" );
	checks.Equal( releasedAll, count, "G8.every-transient-is-released" );
	checks.Equal( pooledAccepted, count, "G9.the-pooled-executor-runs-every-graph" );
	checks.Equal( sameStream, count, "G9.pooled-recording-equals-serial-recording" );
}

// G10: a pool keeps physical transients between executions and reuses them
// from the usage they were left in; unused ones are released behind their
// token.
void Pooling( testing::Checks &checks )
{
	auto device = ManualDevice();
	auto *control = device::null::Control( *device );
	const std::size_t baseline = device->LiveResourceCount();
	auto build = []
	{
		GraphBuilder builder;
		const ResourceRef a = builder.CreateTexture( "a", fixtures::Color() );
		const ResourceRef b = builder.CreateTexture( "b", fixtures::Color() );
		builder.AddPass( "write-a", PassKind::kRender )
		    .Write( a, ResourceUsage::kColorAttachment )
		    .Execute( fixtures::Noop );
		builder.AddPass( "a-to-b", PassKind::kCopy )
		    .Read( a, ResourceUsage::kCopySource )
		    .Write( b, ResourceUsage::kCopyDestination )
		    .Execute( fixtures::Noop );
		builder.AddPass( "read-b", PassKind::kRender )
		    .Read( b, ResourceUsage::kSampled )
		    .SideEffect()
		    .Execute( fixtures::Noop );
		return CompileGraph( std::move( builder ) ).Value();
	};
	{
		TransientPool pool( *device, 2 );
		SerialGraphExecutor executor( pool );
		const CompiledGraph graph = build();
		auto first = executor.Execute( graph, *device );
		auto second = executor.Execute( graph, *device );
		auto third = executor.Execute( graph, *device );
		checks.That( first && first.Value().transients == 2 && first.Value().reused == 0,
		    "G10.the-first-execution-creates-its-transients" );
		checks.That( second && third && second.Value().transients == 0 &&
		                 second.Value().reused == 2 && third.Value().reused == 2,
		    "G10.later-executions-reuse-them-from-their-last-usage" );
		checks.Equal( pool.Size(), std::size_t( 2 ), "G10.the-pool-holds-them-between-executions" );
		GraphBuilder other;
		const ResourceRef big = other.CreateTexture( "big", fixtures::Color( 32 ) );
		other.AddPass( "big", PassKind::kRender )
		    .Write( big, ResourceUsage::kColorAttachment )
		    .SideEffect()
		    .Execute( fixtures::Noop );
		const CompiledGraph otherGraph = CompileGraph( std::move( other ) ).Value();
		for ( int i = 0; i < 4; ++i )
			(void)executor.Execute( otherGraph, *device );
		checks.Equal( pool.Size(), std::size_t( 1 ), "G10.idle-transients-leave-the-pool" );
	}
	control->CompleteAll();
	(void)device->Poll();
	checks.Equal(
	    device->LiveResourceCount(), baseline, "G10.destroying-the-pool-releases-everything" );
}

} // namespace

int main()
{
	testing::Checks checks;
	Culling( checks );
	Transitions( checks );
	BadGraphs( checks );
	Execution( checks );
	SceneColorCapture( checks );
	RandomGraphs( checks, 1000 );
	Pooling( checks );
	PassTimers( checks );
	return checks.Report();
}
