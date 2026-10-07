//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1.vulkan (RFC 0016 K2 "Graph suite" and
//			"Synchronization validated" on the Vulkan adapter).
//
//			The 1,000 seeded random graphs of render.graph.v1, with real work
//			for their writes (a render pass clearing each color attachment, a
//			clear for each texture copy destination, a buffer write for each
//			buffer copy destination), compiled, checked by
//			ValidateCompiledGraph, and executed on render.device.vulkan twice:
//			through the serial executor, then through the pooled executor (one
//			encoder per pass on a 4-worker jobs.graph ParallelExecutor) with a
//			TransientPool, so physical transients are reused across graphs.
//			Every submission must be accepted and complete, and with the
//			Khronos validation layer installed (synchronization validation
//			included) the run must report no message; without the layer that
//			clause prints SKIP and certifies nothing.
//
//			Built with RENDERTEST_GRAPH_D3D12 it is render.graph.v1.d3d12 (RFC
//			0024 X3): the same graphs on render.device.d3d12 under Wine with
//			vkd3d-proton (tools/render/d3d12_lane.py), with the D3D12 debug
//			layer in place of the Khronos layer.
//
//=============================================================================//

#include "graph_fixtures.h"
#include "jobsystem/parallel_executor.h"
#if defined( RENDERTEST_GRAPH_D3D12 )
#include "render/device/d3d12/provider.h"
#define RG_ADAPTER "d3d12"
#else
#include "render/device/vulkan/provider.h"
#define RG_ADAPTER "vulkan"
#endif
#include "render/graph/scene_color.h"
#include "render/graph/validate.h"
#include "testing/checks.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace
{

using namespace render;
using namespace render::graph;
using device::ResourceUsage;
namespace fixtures = rendertest::graph;
#if defined( RENDERTEST_GRAPH_D3D12 )
namespace d3d12 = render::device::d3d12;
#else
namespace vulkan = render::device::vulkan;
#endif

bool Wait( device::IRenderDevice2 &device, device::CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

// Real commands for a pass's writes; reads and storage writes record
// nothing beyond their transitions.
ExecuteFn RealWork(
    const std::vector<Access> &accesses, const std::vector<ResourceDecl> &resources )
{
	return [accesses, resources]( RecordContext &context )
	{
		static const std::byte kBytes[16] = {};
		device::CommandEncoder &encoder = context.Encoder();
		for ( const Access &access : accesses )
		{
			if ( !access.write )
				continue;
			const ResourceDecl &decl = resources[access.resource.index];
			if ( decl.isTexture && access.usage == ResourceUsage::kColorAttachment )
			{
				device::ColorAttachment color;
				color.texture = context.Texture( access.resource );
				color.clear = { 0.25f, 0.5f, 0.75f, 1.0f };
				const device::ColorAttachment colors[] = { color };
				device::RenderingDesc rendering;
				rendering.colors = colors;
				rendering.width = decl.texture.width;
				rendering.height = decl.texture.height;
				encoder.BeginRendering( rendering );
				encoder.EndRendering();
			}
			else if ( decl.isTexture && access.usage == ResourceUsage::kCopyDestination )
			{
				encoder.ClearTexture(
				    context.Texture( access.resource ), { 1.0f, 0.0f, 0.0f, 1.0f } );
			}
			else if ( !decl.isTexture && access.usage == ResourceUsage::kCopyDestination )
			{
				encoder.WriteBuffer( context.Buffer( access.resource ), 0, kBytes );
			}
		}
	};
}

void SceneColorCapture(
    testing::Checks &checks, device::IRenderDevice2 &device, std::uint32_t samples )
{
	device::BufferDesc readbackDesc;
	readbackDesc.size = 4 * 4 * 4;
	readbackDesc.memory = device::MemoryKind::kReadback;
	readbackDesc.usages = { ResourceUsage::kCopyDestination };
	const auto readbackResult = device.CreateBuffer( readbackDesc );
	if ( !checks.That( readbackResult.HasValue(), "G11." RG_ADAPTER "-readback-created" ) )
		return;
	const device::BufferId readbackId = readbackResult.Value();
	GraphBuilder builder;
	device::TextureDesc sceneDesc = fixtures::Color( 4 );
	sceneDesc.sampleCount = samples;
	const ResourceRef scene = builder.CreateTexture( "scene", sceneDesc );
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
	if ( !checks.That( snapshot.has_value(), "G11." RG_ADAPTER "-scene-color-captured" ) )
		return;
	builder.AddPass( "transmission-consumer", PassKind::kRender )
	    .Read( *snapshot, ResourceUsage::kSampled )
	    .SideEffect()
	    .Execute( fixtures::Noop );
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
	if ( !checks.That( graph.HasValue(), "G11." RG_ADAPTER "-capture-compiles" ) )
		return;
	SerialGraphExecutor executor;
	auto result = executor.Execute( graph.Value(), device );
	if ( !result )
	{
		std::printf( "G11 %ux MSAA: %s at %s (native %d)\n", samples,
		    device::DescribeStatus( result.Error().status ),
		    device::DescribeOperation( result.Error().operation ), result.Error().nativeCode );
		std::printf( "%s", graph.Value().trace.ToString().c_str() );
	}
	if ( !checks.That(
	         result && Wait( device, result.Value().token ), "G11." RG_ADAPTER "-capture-completes" ) )
		return;
	std::array<std::byte, 64> pixels{};
	if ( checks.That( device.ReadBuffer( readbackId, 0, pixels ).HasValue(),
	         "G11." RG_ADAPTER "-capture-readback" ) )
	{
		bool matches = true;
		for ( std::size_t pixel = 0; pixel < 16; ++pixel )
		{
			const std::array<std::byte, 4> expected = {
			    std::byte{ 51 }, std::byte{ 102 }, std::byte{ 153 }, std::byte{ 255 } };
			matches &= std::equal( expected.begin(), expected.end(), pixels.begin() + pixel * 4 );
		}
		checks.That( matches, "G11." RG_ADAPTER "-snapshot-preserves-scene-pixels" );
	}
	(void)device.Release( readbackId, result.Value().token );
}

} // namespace

int main()
{
	testing::Checks checks;
	std::atomic<std::uint64_t> messages{ 0 };
#if defined( RENDERTEST_GRAPH_D3D12 )
	const bool layer = true; // the debug layer counts its own messages
	d3d12::D3d12AdapterOptions options;
	options.validation = true;
	options.validationCounter = &messages;
#else
	const bool layer = vulkan::ValidationLayerAvailable();
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
#endif
	int compiled = 0, clean = 0, serialRan = 0, pooledRan = 0;
	std::uint64_t reused = 0, encoders = 0;
	{
#if defined( RENDERTEST_GRAPH_D3D12 )
		auto created = d3d12::Create( options );
#else
		auto created = vulkan::Create( options );
#endif
		if ( !checks.That( created.HasValue(), "device.a-" RG_ADAPTER "-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<device::IRenderDevice2> device = std::move( created ).Value();
		jobsystem::ParallelExecutor jobs( 4 );
		TransientPool pool( *device );
		SerialGraphExecutor serial;
		PooledGraphExecutor pooled( jobs, &pool );
		constexpr int kGraphs = 1000;
		for ( int seed = 0; seed < kGraphs; ++seed )
		{
			for ( int mode = 0; mode < 2; ++mode )
			{
				fixtures::RandomGraph random = fixtures::MakeRandomGraph(
				    static_cast<std::uint32_t>( seed ), *device, fixtures::Noop, RealWork );
				std::vector<device::ResourceId> imports;
				for ( const ResourceDecl &decl : random.resources )
				{
					if ( decl.imported )
						imports.push_back( decl.isTexture
						                       ? device::ResourceId( decl.importedTexture )
						                       : device::ResourceId( decl.importedBuffer ) );
				}
				auto graph = CompileGraph( std::move( random.builder ) );
				if ( graph )
				{
					compiled += mode == 0;
					clean += mode == 0 && ValidateCompiledGraph( graph.Value() ).empty();
					auto executed = mode == 0 ? serial.Execute( graph.Value(), *device )
					                          : pooled.Execute( graph.Value(), *device );
					const bool ran = executed && Wait( *device, executed.Value().token );
					( mode == 0 ? serialRan : pooledRan ) += ran;
					if ( ran && mode == 1 )
					{
						reused += executed.Value().reused;
						encoders += executed.Value().encoders;
					}
				}
				for ( device::ResourceId id : imports )
					(void)device->Release( id, device::CompletionToken() );
				(void)device->Poll();
			}
		}
		std::printf(
		    "INFO graph " RG_ADAPTER ": %d graphs compiled; serial ran %d, pooled ran %d with %llu "
		    "encoders and %llu pooled transients reused\n",
		    compiled, serialRan, pooledRan, static_cast<unsigned long long>( encoders ),
		    static_cast<unsigned long long>( reused ) );
		checks.That( compiled > 900, "G7.most-random-graphs-compile" );
		checks.Equal( clean, compiled, "G8.the-validator-accepts-every-compiled-graph" );
		checks.Equal( serialRan, compiled, "G6.the-serial-executor-runs-every-graph-on-" RG_ADAPTER "" );
		checks.Equal( pooledRan, compiled, "G9.the-pooled-executor-runs-every-graph-on-" RG_ADAPTER "" );
		checks.That( reused > 0, "G10.the-pool-reuses-transients-on-" RG_ADAPTER "" );
		SceneColorCapture( checks, *device, 1 );
		SceneColorCapture( checks, *device, 4 );
		// G12 on a real second queue (RFC 0016 S8): two-queue random graphs
		// whose writes are transfer writes, run as per-queue submissions with
		// the compiled cross-queue waits, under synchronization validation.
		const bool async = device->Facts().capabilities.Has( device::Capability::kAsyncCompute );
		if ( !async )
			std::printf( "SKIP G12." RG_ADAPTER " the device has no async compute queue\n" );
		else
		{
			int queueCompiled = 0, queueRan = 0, waited = 0, pooledQueueRan = 0;
			std::uint64_t serialCompute = 0, pooledCompute = 0;
			std::uint64_t queueEncoders = 0;
			constexpr int kQueueGraphs = 200;
			for ( int run = 0; run < 2 * kQueueGraphs; ++run )
			{
				// Each seed runs on the serial executor, then the pooled one.
				const int seed = run / 2;
				const bool pooledRun = run % 2 == 1;
				fixtures::RandomGraph random = fixtures::MakeRandomQueueGraph(
				    static_cast<std::uint32_t>( seed ), *device, true, RealWork );
				std::vector<device::ResourceId> imports;
				for ( const ResourceDecl &decl : random.resources )
				{
					if ( decl.imported )
						imports.push_back( device::ResourceId( decl.importedBuffer ) );
				}
				CompileOptions options;
				options.asyncCompute = true;
				auto graph = CompileGraph( std::move( random.builder ), options );
				if ( graph && ValidateCompiledGraph( graph.Value() ).empty() && pooledRun )
				{
					auto executed = pooled.Execute( graph.Value(), *device );
					const bool ran = executed && Wait( *device, executed.Value().token );
					pooledQueueRan += ran;
					if ( ran )
						pooledCompute += executed.Value().computeSubmissions;
				}
				else if ( graph && ValidateCompiledGraph( graph.Value() ).empty() )
				{
					++queueCompiled;
					waited += !graph.Value().trace.waits.empty();
					auto executed = serial.Execute( graph.Value(), *device );
					const bool ran = executed && Wait( *device, executed.Value().token );
					queueRan += ran;
					if ( ran )
					{
						queueEncoders += executed.Value().encoders;
						serialCompute += executed.Value().computeSubmissions;
					}
				}
				for ( device::ResourceId id : imports )
					(void)device->Release( id, device::CompletionToken() );
				(void)device->Poll();
			}
			std::printf( "INFO graph " RG_ADAPTER " two queues: %d graphs (%d with cross-queue waits), "
			             "%d ran in %llu submissions\n",
			    queueCompiled, waited, queueRan,
			    static_cast<unsigned long long>( queueEncoders ) );
			checks.Equal( queueCompiled, kQueueGraphs, "G12." RG_ADAPTER "-every-two-queue-graph-compiles" );
			checks.Equal( queueRan, queueCompiled,
			    "G12." RG_ADAPTER "-every-two-queue-graph-runs-on-the-compute-and-graphics-queues" );
			checks.Equal( pooledQueueRan, queueCompiled,
			    "G12." RG_ADAPTER "-the-pooled-executor-runs-every-two-queue-graph" );
			std::printf( "INFO graph " RG_ADAPTER " two queues: compute submissions serial %llu, pooled %llu\n",
			    static_cast<unsigned long long>( serialCompute ),
			    static_cast<unsigned long long>( pooledCompute ) );
			checks.That( serialCompute > 0 && pooledCompute == serialCompute,
			    "G12." RG_ADAPTER "-the-pooled-executor-submits-the-serial-executors-compute-runs" );
			checks.That( waited > kQueueGraphs / 4 && queueEncoders > std::uint64_t( queueRan ),
			    "G12." RG_ADAPTER "-the-graphs-cross-queues-with-waits" );
		}
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
