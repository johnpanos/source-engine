//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.cull (RFC 0016 GPU-driven submission S4) on
//			render.device.vulkan: cull.comp, dispatched as a render.graph
//			compute pass through the serial executor, against the CPU
//			culler (render::scene::BuildDrawList), and RFC 0003's placement
//			comparison between them.
//
//			- gpu.equals-cpu: on 200 seeded scenes (up to 20,000 instances,
//			  boxes straddling the planes, empty boxes, random view masks
//			  and view bits including "every view"), the GPU's visibility
//			  mask keeps exactly the instances the CPU culler keeps, bit for
//			  bit;
//			- defects: the seeded near-corner, ignored-view-mask and
//			  kept-empty-box kernels (cull_defects_spv.h) each disagree with
//			  the oracle (3 of 3);
//			- queue: the pass asks for the async compute queue; compiled for
//			  a device with one, it is placed there and the readback waits
//			  for it; compiled without, everything runs on graphics, and the
//			  same mask comes back;
//			- capability: on render.device.null without compute, kernel
//			  creation fails with kNoCompute;
//			- validation: with the Khronos validation layer the run reports
//			  no message; without it the clause prints SKIP.
//
//			Placement (RFC 0003, CPU/GPU execution placement): for 1,024 to
//			65,536 instances the suite times the equivalent-output paths at
//			full cost: the CPU culler (serial and pooled on four workers)
//			against the GPU round trip the CPU draw list needs today (pack,
//			upload, dispatch, wait, read back), and the GPU with resident
//			instances (view upload only). It prints the medians and the
//			crossover; timing certifies nothing and fails nothing.
//
//			RENDER_VK_ADAPTER=<n> picks the physical device. A missing
//			Vulkan device fails the run.
//
//=============================================================================//

#include "spv/cull_defects_spv.h"
#include "render/device/null/provider.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/validate.h"
#include "render/pass/cull/cull.h"
#include "render/scene/draw_list.h"
#include "jobsystem/parallel_executor.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <span>
#include <thread>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::pass::cull;
using Clock = std::chrono::steady_clock;

bool Wait( IRenderDevice2 &device, CompletionToken token )
{
	const auto deadline = Clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( Clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

struct Scene
{
	scene::SceneSnapshot snapshot;
	scene::SceneView view;
};

// Boxes in a 4,000-unit cube around a camera looking along a random
// direction, so many straddle the frustum's planes; a few are empty.
Scene RandomScene( std::uint32_t seed, std::uint32_t count )
{
	std::mt19937 random( seed );
	std::uniform_real_distribution<float> position( -2000.0f, 2000.0f );
	std::uniform_real_distribution<float> extent( 0.0f, 120.0f );
	std::uniform_real_distribution<float> unit( -1.0f, 1.0f );
	Scene out;
	out.snapshot.revision = seed;
	out.snapshot.instances.resize( count );
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		scene::MeshInstance &instance = out.snapshot.instances[i];
		const math::float3 center = { position( random ), position( random ), position( random ) };
		const math::float3 half = { extent( random ), extent( random ), extent( random ) };
		instance.worldBounds.min = center - half;
		instance.worldBounds.max = center + half;
		if ( random() % 50 == 0 )
			std::swap( instance.worldBounds.min.y, instance.worldBounds.max.y ); // empty
		instance.desc.viewMask = random() % 4 == 0 ? std::uint32_t( random() ) : ~0u;
		instance.desc.material = random() % 16;
		instance.desc.mesh = random() % 64;
	}
	math::float3 forward = { unit( random ), unit( random ), unit( random ) * 0.3f };
	if ( std::fabs( forward.x ) + std::fabs( forward.y ) < 0.1f )
		forward.x = 1.0f;
	scene::ViewDesc view;
	const math::float3 eye = { position( random ) * 0.2f, position( random ) * 0.2f, 0.0f };
	view.view = math::LookAt( eye, eye + forward, { 0.0f, 0.0f, 1.0f } );
	view.projection = math::Perspective(
	    1.0f + 0.5f * unit( random ), 16.0f / 9.0f, 4.0f, 1500.0f + 1000.0f * unit( random ) );
	view.viewBit = random() % 5 == 0 ? 32 : random() % 32;
	out.view = scene::MakeView( view );
	return out;
}

// The CPU culler's decision as a mask (no visibility provider).
std::vector<std::uint32_t> CpuMask( const Scene &scene )
{
	const auto count = static_cast<std::uint32_t>( scene.snapshot.instances.size() );
	std::vector<std::uint32_t> mask( MaskWords( count ), 0 );
	for ( const scene::DrawItem &item : scene::BuildDrawList( scene.snapshot, scene.view ).items )
		mask[item.instance / 32] |= 1u << ( item.instance % 32 );
	return mask;
}

// Device buffers sized for `capacity` instances, reused across runs.
class GpuCuller
{
public:
	GpuCuller( IRenderDevice2 &device, CullKernel &kernel, std::uint32_t capacity )
	    : m_Device( device ), m_Kernel( kernel ), m_Capacity( capacity )
	{
		auto make = [&]( std::uint64_t size, UsageSet usages, MemoryKind memory, BufferDesc &desc )
		{
			desc.size = size;
			desc.usages = usages;
			desc.memory = memory;
			auto buffer = device.CreateBuffer( desc );
			return buffer ? buffer.Value() : BufferId();
		};
		const UsageSet input = { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead };
		m_Instances = make( std::uint64_t( capacity ) * sizeof( CullInstance ), input,
		    MemoryKind::kDeviceLocal, m_InstancesDesc );
		m_View = make( sizeof( CullView ), input, MemoryKind::kDeviceLocal, m_ViewDesc );
		const std::uint64_t maskBytes = std::uint64_t( MaskWords( capacity ) ) * 4;
		m_Mask = make( maskBytes, { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource },
		    MemoryKind::kDeviceLocal, m_MaskDesc );
		m_Readback = make(
		    maskBytes, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback, m_ReadbackDesc );
	}
	~GpuCuller()
	{
		(void)m_Device.WaitIdle();
		for ( BufferId id : { m_Instances, m_View, m_Mask, m_Readback } )
		{
			if ( id.IsValid() )
				(void)m_Device.Release( id, CompletionToken() );
		}
		(void)m_Device.Poll();
	}
	bool Valid() const
	{
		return m_Instances.IsValid() && m_View.IsValid() && m_Mask.IsValid() &&
		       m_Readback.IsValid();
	}

	// Culls `instances` (uploaded unless `resident`) for `view`; reads the
	// mask back. `options` picks the queues; `queueOfCull` reports where the
	// cull pass ran.
	bool Run( std::span<const CullInstance> instances, const CullView &view, bool resident,
	    std::vector<std::uint32_t> &mask, const graph::CompileOptions &options = {},
	    graph::Queue *queueOfCull = nullptr, bool *validated = nullptr )
	{
		const auto count = static_cast<std::uint32_t>( instances.size() );
		if ( count > m_Capacity )
			return false;
		const std::uint64_t maskBytes = std::uint64_t( MaskWords( count ) ) * 4;
		graph::GraphBuilder builder;
		const auto instancesRef = builder.ImportBuffer( "instances", m_Instances, m_InstancesDesc,
		    m_InstancesUploaded ? ResourceUsage::kStorageRead : ResourceUsage::kUndefined,
		    ResourceUsage::kStorageRead );
		const auto viewRef = builder.ImportBuffer(
		    "view", m_View, m_ViewDesc, ResourceUsage::kUndefined, ResourceUsage::kStorageRead );
		const auto maskRef = builder.ImportBuffer(
		    "mask", m_Mask, m_MaskDesc, ResourceUsage::kUndefined, ResourceUsage::kCopySource );
		const auto readbackRef = builder.ImportBuffer( "readback", m_Readback, m_ReadbackDesc,
		    ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		const bool upload = !resident || !m_InstancesUploaded;
		graph::PassBuilder uploads = builder.AddPass( "upload", graph::PassKind::kCopy );
		if ( upload )
			uploads.Write( instancesRef, ResourceUsage::kCopyDestination );
		uploads.Write( viewRef, ResourceUsage::kCopyDestination );
		uploads.Execute(
		    [&]( graph::RecordContext &context )
		    {
			    if ( upload )
				    context.Encoder().WriteBuffer(
				        context.Buffer( instancesRef ), 0, std::as_bytes( instances ) );
			    context.Encoder().WriteBuffer( context.Buffer( viewRef ), 0,
			        std::as_bytes( std::span<const CullView>( &view, 1 ) ) );
		    } );
		AddCullPass( builder, m_Kernel, { instancesRef, viewRef, maskRef, count, true } );
		builder.AddPass( "readback", graph::PassKind::kCopy )
		    .Read( maskRef, ResourceUsage::kCopySource )
		    .Write( readbackRef, ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [&]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyBuffer( context.Buffer( maskRef ),
			            context.Buffer( readbackRef ), { 0, 0, maskBytes } );
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ), options );
		if ( !compiled )
			return false;
		if ( queueOfCull )
			*queueOfCull = compiled.Value().order[1].queue;
		if ( validated )
			*validated =
			    graph::ValidateCompiledGraph( compiled.Value() ).empty() &&
			    compiled.Value().order[2].waitFor == ( options.asyncCompute ? 1u : UINT32_MAX );
		graph::SerialGraphExecutor executor;
		auto executed = executor.Execute( compiled.Value(), m_Device );
		bool ok = executed.HasValue() && Wait( m_Device, executed.Value().token );
		if ( executed )
			m_Kernel.Collect( executed.Value().token );
		m_InstancesUploaded = true;
		mask.assign( MaskWords( count ), 0 );
		ok = ok && m_Device
		               .ReadBuffer( m_Readback, 0,
		                   std::as_writable_bytes( std::span<std::uint32_t>( mask ) ) )
		               .HasValue();
		return ok;
	}

private:
	IRenderDevice2 &m_Device;
	CullKernel &m_Kernel;
	std::uint32_t m_Capacity;
	BufferId m_Instances, m_View, m_Mask, m_Readback;
	BufferDesc m_InstancesDesc, m_ViewDesc, m_MaskDesc, m_ReadbackDesc;
	bool m_InstancesUploaded = false;
};

std::uint32_t InstancesFor( int seed )
{
	return 1 + static_cast<std::uint32_t>( ( seed * 7919 ) % 20000 );
}

// Scenes whose GPU mask differs from the CPU culler's; -1 when a device
// step failed.
int Disagreements( IRenderDevice2 &device, CullKernel &kernel, int scenes )
{
	GpuCuller culler( device, kernel, 20000 );
	if ( !culler.Valid() )
		return -1;
	int differ = 0;
	for ( int seed = 0; seed < scenes; ++seed )
	{
		const Scene scene = RandomScene( static_cast<std::uint32_t>( seed ), InstancesFor( seed ) );
		const auto packed = PackInstances( scene.snapshot );
		std::vector<std::uint32_t> mask;
		if ( !culler.Run(
		         packed, PackView( scene.view, std::uint32_t( packed.size() ) ), false, mask ) )
			return -1;
		const std::vector<std::uint32_t> expected = CpuMask( scene );
		if ( mask != expected )
		{
			++differ;
			if ( differ <= 3 )
			{
				for ( std::size_t w = 0; w < mask.size(); ++w )
				{
					if ( mask[w] != expected[w] )
					{
						std::printf( "  scene %d: word %zu gpu %08x cpu %08x\n", seed, w, mask[w],
						    expected[w] );
						break;
					}
				}
			}
		}
	}
	return differ;
}

template <typename F> double MedianMicroseconds( int runs, F &&f )
{
	std::vector<double> times;
	for ( int r = 0; r < runs; ++r )
	{
		const auto start = Clock::now();
		f();
		times.push_back(
		    std::chrono::duration<double, std::micro>( Clock::now() - start ).count() );
	}
	std::sort( times.begin(), times.end() );
	return times[times.size() / 2];
}

void Placement( testing::Checks &checks, IRenderDevice2 &device, CullKernel &kernel )
{
	jobsystem::ParallelExecutor jobs( 4 );
	GpuCuller culler( device, kernel, 65536 );
	bool ok = culler.Valid();
	std::uint32_t crossover = 0;
	std::printf( "INFO cull placement (median us): instances cpu-serial cpu-pooled4 "
	             "gpu-roundtrip gpu-resident\n" );
	for ( std::uint32_t count : { 1024u, 4096u, 16384u, 65536u } )
	{
		const Scene scene = RandomScene( 7 + count, count );
		std::vector<std::uint32_t> mask;
		const double serial = MedianMicroseconds( 21,
		    [&]
		    {
			    (void)scene::BuildDrawList( scene.snapshot, scene.view );
		    } );
		const double pooled = MedianMicroseconds( 21,
		    [&]
		    {
			    (void)scene::BuildDrawListPooled( scene.snapshot, scene.view, jobs );
		    } );
		const double roundtrip = MedianMicroseconds( 21,
		    [&]
		    {
			    const auto packed = PackInstances( scene.snapshot );
			    ok = ok && culler.Run( packed, PackView( scene.view, count ), false, mask );
		    } );
		const auto packed = PackInstances( scene.snapshot );
		const double resident = MedianMicroseconds( 21,
		    [&]
		    {
			    ok = ok && culler.Run( packed, PackView( scene.view, count ), true, mask );
		    } );
		std::printf( "INFO cull placement: %u %.1f %.1f %.1f %.1f\n", count, serial, pooled,
		    roundtrip, resident );
		if ( !crossover && roundtrip < std::min( serial, pooled ) )
			crossover = count;
	}
	if ( crossover )
		std::printf(
		    "INFO cull placement: the GPU round trip is faster from %u instances\n", crossover );
	else
		std::printf( "INFO cull placement: the CPU culler is faster at every measured size "
		             "while its consumer is the CPU draw list\n" );
	checks.That( ok, "placement.every-measured-run-completes" );
}

} // namespace

int main()
{
	testing::Checks checks;
	{
		null::NullOptions options;
		options.capabilities.Remove( Capability::kCompute );
		auto noCompute = null::Create( options );
		auto refused = noCompute ? CullKernel::Create( *noCompute.Value() )
		                         : foundation::Expected<std::unique_ptr<CullKernel>, CullStatus>(
		                               foundation::MakeUnexpected( CullStatus::kDevice ) );
		checks.That( !refused && refused.Error() == CullStatus::kNoCompute,
		    "capability.without-compute-kernel-creation-fails-kNoCompute" );
	}

	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<IRenderDevice2> device = std::move( created ).Value();
		const std::string_view name = device->Facts().adapterName;
		std::printf( "INFO cull adapter: %.*s\n", static_cast<int>( name.size() ), name.data() );

		auto kernel = CullKernel::Create( *device );
		if ( !checks.That( kernel.HasValue(), "kernel.is-created" ) )
			return checks.Report();
		const int differ = Disagreements( *device, *kernel.Value(), 200 );
		checks.That( differ >= 0, "gpu.every-dispatch-runs" );
		checks.Equal( differ, 0, "gpu.equals-cpu on 200 seeded scenes, bit for bit" );
		checks.Equal( kernel.Value()->RecordFailures(), 0u, "gpu.records-without-failure" );

		struct Defect
		{
			const char *name;
			std::span<const std::uint32_t> code;
		};
		const Defect defects[] = {
		    { "near-corner", rendertest::cull::spirv::kCullNearCorner },
		    { "view-mask-ignored", rendertest::cull::spirv::kCullViewMaskIgnored },
		    { "empty-kept", rendertest::cull::spirv::kCullEmptyKept },
		};
		int rejected = 0;
		for ( const Defect &defect : defects )
		{
			auto seeded = CullKernel::Create( *device, defect.code );
			const int seededDiffer = seeded ? Disagreements( *device, *seeded.Value(), 40 ) : -1;
			std::printf(
			    "INFO cull defect %s: %d of 40 scenes disagree\n", defect.name, seededDiffer );
			rejected += seededDiffer > 0;
		}
		checks.Equal( rejected, 3, "defects.each-seeded-kernel-disagrees (3 of 3)" );

		// Queues: the pass asks for async compute.
		{
			GpuCuller culler( *device, *kernel.Value(), 4096 );
			const Scene scene = RandomScene( 99, 4096 );
			const auto packed = PackInstances( scene.snapshot );
			const std::vector<std::uint32_t> expected = CpuMask( scene );
			graph::CompileOptions async;
			async.asyncCompute = true;
			std::vector<std::uint32_t> asyncMask, graphicsMask;
			graph::Queue asyncQueue = graph::Queue::kGraphics;
			graph::Queue graphicsQueue = graph::Queue::kAsyncCompute;
			bool asyncValid = false, graphicsValid = false;
			const bool ran = culler.Valid() &&
			                 culler.Run( packed, PackView( scene.view, 4096 ), false, asyncMask,
			                     async, &asyncQueue, &asyncValid ) &&
			                 culler.Run( packed, PackView( scene.view, 4096 ), false, graphicsMask,
			                     {}, &graphicsQueue, &graphicsValid );
			checks.That( ran && asyncQueue == graph::Queue::kAsyncCompute && asyncValid,
			    "queue.with-async-compute-the-pass-runs-there-and-the-readback-waits" );
			checks.That( ran && graphicsQueue == graph::Queue::kGraphics && graphicsValid,
			    "queue.without-it-everything-runs-on-graphics" );
			checks.That( ran && asyncMask == expected && graphicsMask == expected,
			    "queue.both-placements-read-back-the-oracle-mask" );
		}

		Placement( checks, *device, *kernel.Value() );
		kernel.Value().reset();
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation.no-messages: the Khronos validation layer is absent\n" );
	return checks.Report();
}
