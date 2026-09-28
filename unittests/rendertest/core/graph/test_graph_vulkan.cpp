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
//=============================================================================//

#include "graph_fixtures.h"
#include "jobsystem/parallel_executor.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/validate.h"
#include "testing/checks.h"

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
namespace vulkan = render::device::vulkan;

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

} // namespace

int main()
{
	testing::Checks checks;
	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	int compiled = 0, clean = 0, serialRan = 0, pooledRan = 0;
	std::uint64_t reused = 0, encoders = 0;
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
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
		    "INFO graph vulkan: %d graphs compiled; serial ran %d, pooled ran %d with %llu "
		    "encoders and %llu pooled transients reused\n",
		    compiled, serialRan, pooledRan, static_cast<unsigned long long>( encoders ),
		    static_cast<unsigned long long>( reused ) );
		checks.That( compiled > 900, "G7.most-random-graphs-compile" );
		checks.Equal( clean, compiled, "G8.the-validator-accepts-every-compiled-graph" );
		checks.Equal( serialRan, compiled, "G6.the-serial-executor-runs-every-graph-on-vulkan" );
		checks.Equal( pooledRan, compiled, "G9.the-pooled-executor-runs-every-graph-on-vulkan" );
		checks.That( reused > 0, "G10.the-pool-reuses-transients-on-vulkan" );
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
