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
#include "../device/test_shaders.h"
#include "render/device/null/provider.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/validate.h"
#include "render/culling/cull.h"
#include "render/scene/draw_list.h"
#include "jobsystem/parallel_executor.h"
#include "testing/checks.h"

#include <algorithm>
#include <bit>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <span>
#include <thread>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::culling;
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

// Compaction and indirect draws (RFC 0016 GPU-driven submission S3/S4).
// One graph uploads the scene, culls it, compacts the mask into indexed
// indirect commands and, when `draw` is set, draws a 64x64 image from them
// with DrawIndexedIndirectCount; `direct` instead draws the CPU culler's
// kept instances one DrawIndexed each, in instance order. Each instance is a
// 4x4-pixel quad in a 16x16 grid (instance i at cell i % 256), so the image
// shows exactly which instances were drawn.
class IndirectChain
{
public:
	static constexpr std::uint32_t kSize = 64;

	IndirectChain( IRenderDevice2 &device, CullKernel &cull, CompactKernel &compact )
	    : m_Device( device ), m_Cull( cull ), m_Compact( compact )
	{
		static const BindingDesc material[] = {
		    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
		auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
		if ( !layout )
			return;
		m_Layout = layout.Value();
		const BindGroupLayoutId layouts[] = { {}, {}, m_Layout };
		static const ReflectedBinding used[] = { { 2, 0, BindingKind::kUniformBuffer } };
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
		        std::as_bytes( std::span( rendertest::shaders::kPositionVertex ) ), "main", {} },
		    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
		        std::as_bytes( std::span( rendertest::shaders::kColorFragment ) ), "main", used } };
		const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat2, 0, 0 } };
		const VertexBufferLayout buffers[] = { { 8, false } };
		const Format colors[] = { Format::kRGBA8Unorm };
		PipelineDesc desc;
		desc.stages = stages;
		desc.layouts = layouts;
		desc.vertex = { attributes, buffers };
		desc.colorFormats = colors;
		desc.raster.cull = CullMode::kNone;
		auto pipeline = device.CreatePipeline( desc );
		if ( pipeline )
			m_Pipeline = pipeline.Value();
	}
	~IndirectChain()
	{
		(void)m_Device.WaitIdle();
		if ( m_Pipeline.IsValid() )
			(void)m_Device.Release( m_Pipeline, CompletionToken() );
		if ( m_Layout.IsValid() )
			(void)m_Device.Release( m_Layout, CompletionToken() );
		(void)m_Device.Poll();
	}
	bool Valid() const { return m_Pipeline.IsValid(); }

	struct Output
	{
		std::uint32_t drawCount = 0; // over every bucket
		// Per bucket, its first count commands.
		std::vector<std::vector<DrawIndexedIndirectCommand>> buckets;
		std::vector<std::byte> pixels;
	};

	// The buckets the templates' bucket fields order (contiguous, ascending).
	static std::vector<DrawBucket> BucketsOf( std::span<const DrawTemplate> templates )
	{
		std::vector<DrawBucket> buckets;
		for ( std::uint32_t i = 0; i < templates.size(); ++i )
		{
			while ( buckets.size() <= templates[i].bucket )
				buckets.push_back( { i, 0 } );
			++buckets[templates[i].bucket].count;
		}
		if ( buckets.empty() )
			buckets.push_back( { 0, 0 } );
		return buckets;
	}

	// The quad grid's geometry for `count` instances: vertices, indices and
	// each instance's template (its own six indices, vertex offset 0).
	static void Grid( std::uint32_t count, std::vector<float> &vertices,
	    std::vector<std::uint32_t> &indices, std::vector<DrawTemplate> &templates )
	{
		vertices.clear();
		indices.clear();
		templates.assign( count, {} );
		for ( std::uint32_t i = 0; i < count; ++i )
		{
			const std::uint32_t cell = i % 256;
			const float x0 = -1.0f + 0.125f * float( cell % 16 );
			const float y0 = -1.0f + 0.125f * float( cell / 16 );
			const float x1 = x0 + 0.125f;
			const float y1 = y0 + 0.125f;
			const std::uint32_t base = static_cast<std::uint32_t>( vertices.size() / 2 );
			for ( float v : { x0, y0, x1, y0, x1, y1, x0, y1 } )
				vertices.push_back( v );
			// Seven buckets of up to 40: one indirect draw each.
			templates[i] = { 6, static_cast<std::uint32_t>( indices.size() ), 0, i / 40 };
			for ( std::uint32_t k : { 0u, 1u, 2u, 0u, 2u, 3u } )
				indices.push_back( base + k );
		}
	}

	// mode: 0 compaction only, 1 indirect draw, 2 direct draws of `kept`.
	bool Run( const Scene &scene, std::span<const DrawTemplate> templates, int mode,
	    const std::vector<std::uint32_t> &kept, Output &out )
	{
		const auto count = static_cast<std::uint32_t>( scene.snapshot.instances.size() );
		const auto packed = PackInstances( scene.snapshot );
		const CullView view = PackView( scene.view, count );
		std::vector<float> vertices;
		std::vector<std::uint32_t> indices;
		std::vector<DrawTemplate> gridTemplates;
		Grid( count, vertices, indices, gridTemplates );
		const float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		std::vector<BufferId> made;
		auto buffer = [&]( std::uint64_t size, UsageSet usages, BufferDesc &desc,
		                  MemoryKind memory = MemoryKind::kDeviceLocal )
		{
			desc.size = std::max<std::uint64_t>( size, 16 );
			desc.usages = usages;
			desc.memory = memory;
			auto created = m_Device.CreateBuffer( desc );
			if ( !created )
				return BufferId();
			made.push_back( created.Value() );
			return created.Value();
		};
		const UsageSet storageIn = { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead };
		BufferDesc dInstances, dView, dMask, dTemplates, dCommands, dReadback, dVertices, dIndices,
		    dUniform, dPixels;
		const BufferId instances =
		    buffer( packed.size() * sizeof( CullInstance ), storageIn, dInstances );
		const BufferId viewBuffer = buffer( sizeof( CullView ), storageIn, dView );
		const BufferId mask = buffer( std::uint64_t( MaskWords( count ) ) * 4,
		    { ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead }, dMask );
		const BufferId templateBuffer =
		    buffer( templates.size() * sizeof( DrawTemplate ), storageIn, dTemplates );
		const std::vector<DrawBucket> buckets = BucketsOf( templates );
		const auto bucketCount = static_cast<std::uint32_t>( buckets.size() );
		const std::uint64_t commandBytes = CommandBufferBytes( count, bucketCount );
		BufferDesc dBuckets;
		const BufferId bucketBuffer =
		    buffer( buckets.size() * sizeof( DrawBucket ), storageIn, dBuckets );
		const BufferId commands = buffer( commandBytes,
		    { ResourceUsage::kStorageWrite, ResourceUsage::kIndirect, ResourceUsage::kCopySource },
		    dCommands );
		const BufferId readback = buffer(
		    commandBytes, { ResourceUsage::kCopyDestination }, dReadback, MemoryKind::kReadback );
		const BufferId vertexBuffer = buffer( vertices.size() * 4,
		    { ResourceUsage::kCopyDestination, ResourceUsage::kVertex }, dVertices );
		const BufferId indexBuffer = buffer( indices.size() * 4,
		    { ResourceUsage::kCopyDestination, ResourceUsage::kIndex }, dIndices );
		const BufferId uniform =
		    buffer( 256, { ResourceUsage::kCopyDestination, ResourceUsage::kUniform }, dUniform );
		const BufferId pixelBuffer = buffer( kSize * kSize * 4, { ResourceUsage::kCopyDestination },
		    dPixels, MemoryKind::kReadback );
		TextureDesc target;
		target.format = Format::kRGBA8Unorm;
		target.width = kSize;
		target.height = kSize;
		target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
		auto texture = m_Device.CreateTexture( target );
		bool ok = texture.HasValue();
		for ( BufferId id : made )
			ok = ok && id.IsValid();
		const BindGroupEntry entry[] = { { 0, uniform, 0, 16, {}, {} } };
		BindGroupId group;
		if ( ok )
		{
			auto created = m_Device.CreateBindGroup( { m_Layout, entry } );
			ok = created.HasValue();
			if ( ok )
				group = created.Value();
		}
		if ( ok )
		{
			using graph::ResourceRef;
			graph::GraphBuilder b;
			auto import =
			    [&]( const char *name, BufferId id, const BufferDesc &desc, ResourceUsage final )
			{
				return b.ImportBuffer( name, id, desc, ResourceUsage::kUndefined, final );
			};
			const ResourceRef rInstances =
			    import( "instances", instances, dInstances, ResourceUsage::kStorageRead );
			const ResourceRef rView =
			    import( "view", viewBuffer, dView, ResourceUsage::kStorageRead );
			const ResourceRef rMask = import( "mask", mask, dMask, ResourceUsage::kStorageRead );
			const ResourceRef rTemplates =
			    import( "templates", templateBuffer, dTemplates, ResourceUsage::kStorageRead );
			const ResourceRef rBuckets =
			    import( "buckets", bucketBuffer, dBuckets, ResourceUsage::kStorageRead );
			const ResourceRef rCommands =
			    import( "commands", commands, dCommands, ResourceUsage::kCopySource );
			const ResourceRef rReadback =
			    import( "readback", readback, dReadback, ResourceUsage::kCopyDestination );
			const ResourceRef rVertices =
			    import( "vertices", vertexBuffer, dVertices, ResourceUsage::kVertex );
			const ResourceRef rIndices =
			    import( "indices", indexBuffer, dIndices, ResourceUsage::kIndex );
			const ResourceRef rUniform =
			    import( "uniform", uniform, dUniform, ResourceUsage::kUniform );
			const ResourceRef rPixels =
			    import( "pixels", pixelBuffer, dPixels, ResourceUsage::kCopyDestination );
			const ResourceRef rColor = b.ImportTexture( "color", texture.Value(), target,
			    ResourceUsage::kUndefined, ResourceUsage::kCopySource );
			b.AddPass( "upload", graph::PassKind::kCopy )
			    .Write( rInstances, ResourceUsage::kCopyDestination )
			    .Write( rView, ResourceUsage::kCopyDestination )
			    .Write( rTemplates, ResourceUsage::kCopyDestination )
			    .Write( rBuckets, ResourceUsage::kCopyDestination )
			    .Write( rVertices, ResourceUsage::kCopyDestination )
			    .Write( rIndices, ResourceUsage::kCopyDestination )
			    .Write( rUniform, ResourceUsage::kCopyDestination )
			    .Execute(
			        [&]( graph::RecordContext &c )
			        {
				        CommandEncoder &e = c.Encoder();
				        if ( !packed.empty() )
					        e.WriteBuffer(
					            c.Buffer( rInstances ), 0, std::as_bytes( std::span( packed ) ) );
				        e.WriteBuffer(
				            c.Buffer( rView ), 0, std::as_bytes( std::span( &view, 1 ) ) );
				        if ( !templates.empty() )
					        e.WriteBuffer( c.Buffer( rTemplates ), 0, std::as_bytes( templates ) );
				        e.WriteBuffer(
				            c.Buffer( rBuckets ), 0, std::as_bytes( std::span( buckets ) ) );
				        if ( !vertices.empty() )
					        e.WriteBuffer(
					            c.Buffer( rVertices ), 0, std::as_bytes( std::span( vertices ) ) );
				        if ( !indices.empty() )
					        e.WriteBuffer(
					            c.Buffer( rIndices ), 0, std::as_bytes( std::span( indices ) ) );
				        e.WriteBuffer(
				            c.Buffer( rUniform ), 0, std::as_bytes( std::span( color ) ) );
			        } );
			if ( mode != 2 )
			{
				AddCullPass( b, m_Cull, { rInstances, rView, rMask, count, false } );
				AddCompactPass( b, m_Compact,
				    { rMask, rTemplates, rView, rBuckets, rCommands, count, bucketCount, false } );
			}
			if ( mode != 0 )
			{
				graph::PassBuilder draw = b.AddPass( "draw", graph::PassKind::kRender );
				if ( mode == 1 )
					draw.Read( rCommands, ResourceUsage::kIndirect );
				draw.Read( rVertices, ResourceUsage::kVertex )
				    .Read( rIndices, ResourceUsage::kIndex )
				    .Read( rUniform, ResourceUsage::kUniform )
				    .Write( rColor, ResourceUsage::kColorAttachment )
				    .Execute(
				        [&]( graph::RecordContext &c )
				        {
					        CommandEncoder &e = c.Encoder();
					        const ColorAttachment attachments[] = { { c.Texture( rColor ),
					            LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
					        RenderingDesc rendering;
					        rendering.colors = attachments;
					        rendering.width = kSize;
					        rendering.height = kSize;
					        e.BeginRendering( rendering );
					        e.SetPipeline( m_Pipeline );
					        e.SetBindGroup( BindGroupRole::kMaterial, group );
					        e.SetVertexBuffer( 0, c.Buffer( rVertices ) );
					        e.SetIndexBuffer( c.Buffer( rIndices ), 0, IndexFormat::kUint32 );
					        if ( mode == 1 )
					        {
						        // One draw per bucket, each its own command list.
						        const BufferId records = c.Buffer( rCommands );
						        for ( std::uint32_t k = 0; k < bucketCount; ++k )
							        e.DrawIndexedIndirectCount( records,
							            CommandsOffset( bucketCount ) +
							                std::uint64_t( buckets[k].first ) *
							                    sizeof( DrawIndexedIndirectCommand ),
							            records, std::uint64_t( k ) * 4, buckets[k].count,
							            sizeof( DrawIndexedIndirectCommand ) );
					        }
					        else
					        {
						        for ( std::uint32_t i : kept )
							        e.DrawIndexed( templates[i].indexCount, 1,
							            templates[i].firstIndex, templates[i].vertexOffset, i );
					        }
					        e.EndRendering();
				        } );
				b.AddPass( "pixels", graph::PassKind::kCopy )
				    .Read( rColor, ResourceUsage::kCopySource )
				    .Write( rPixels, ResourceUsage::kCopyDestination )
				    .SideEffect()
				    .Execute(
				        [&]( graph::RecordContext &c )
				        {
					        c.Encoder().CopyTextureToBuffer( c.Texture( rColor ),
					            c.Buffer( rPixels ), { 0, 0, 0, kSize, kSize } );
				        } );
			}
			if ( mode != 2 )
				b.AddPass( "commands", graph::PassKind::kCopy )
				    .Read( rCommands, ResourceUsage::kCopySource )
				    .Write( rReadback, ResourceUsage::kCopyDestination )
				    .SideEffect()
				    .Execute(
				        [&]( graph::RecordContext &c )
				        {
					        c.Encoder().CopyBuffer( c.Buffer( rCommands ), c.Buffer( rReadback ),
					            { 0, 0, commandBytes } );
				        } );
			auto compiled = graph::CompileGraph( std::move( b ) );
			ok = compiled.HasValue() && graph::ValidateCompiledGraph( compiled.Value() ).empty();
			if ( ok )
			{
				graph::SerialGraphExecutor executor;
				auto executed = executor.Execute( compiled.Value(), m_Device );
				ok = executed.HasValue() && Wait( m_Device, executed.Value().token );
				if ( executed )
				{
					m_Cull.Collect( executed.Value().token );
					m_Compact.Collect( executed.Value().token );
				}
			}
		}
		if ( ok && mode != 2 )
		{
			std::vector<std::byte> bytes( commandBytes );
			ok = m_Device.ReadBuffer( readback, 0, bytes ).HasValue();
			out.drawCount = 0;
			out.buckets.assign( bucketCount, {} );
			for ( std::uint32_t k = 0; k < bucketCount; ++k )
			{
				std::uint32_t n = 0;
				std::memcpy( &n, bytes.data() + 4 * k, 4 );
				out.drawCount += n;
				out.buckets[k].resize( std::min( n, buckets[k].count ) );
				if ( !out.buckets[k].empty() )
					std::memcpy( out.buckets[k].data(),
					    bytes.data() + CommandsOffset( bucketCount ) +
					        std::uint64_t( buckets[k].first ) *
					            sizeof( DrawIndexedIndirectCommand ),
					    out.buckets[k].size() * sizeof( DrawIndexedIndirectCommand ) );
			}
		}
		if ( ok && mode != 0 )
		{
			out.pixels.resize( kSize * kSize * 4 );
			ok = m_Device.ReadBuffer( pixelBuffer, 0, out.pixels ).HasValue();
		}
		(void)m_Device.WaitIdle();
		if ( group.IsValid() )
			(void)m_Device.Release( group, CompletionToken() );
		if ( texture )
			(void)m_Device.Release( texture.Value(), CompletionToken() );
		for ( BufferId id : made )
			(void)m_Device.Release( id, CompletionToken() );
		(void)m_Device.Poll();
		return ok;
	}

private:
	IRenderDevice2 &m_Device;
	CullKernel &m_Cull;
	CompactKernel &m_Compact;
	BindGroupLayoutId m_Layout;
	PipelineId m_Pipeline;
};

bool SameCommand( const DrawIndexedIndirectCommand &a, const DrawIndexedIndirectCommand &b )
{
	return a.indexCount == b.indexCount && a.instanceCount == b.instanceCount &&
	       a.firstIndex == b.firstIndex && a.vertexOffset == b.vertexOffset &&
	       a.firstInstance == b.firstInstance;
}

// Templates with varied counts, first indices and signed vertex offsets, in
// 1 to 64 buckets of random sizes (some empty).
std::vector<DrawTemplate> RandomTemplates( std::uint32_t seed, std::uint32_t count )
{
	std::mt19937 random( seed * 31 + 7 );
	std::vector<DrawTemplate> out( count );
	const std::uint32_t bucketCount = 1 + random() % 64;
	std::vector<std::uint32_t> cuts;
	for ( std::uint32_t k = 1; k < bucketCount; ++k )
		cuts.push_back( count ? static_cast<std::uint32_t>( random() % ( count + 1 ) ) : 0 );
	std::sort( cuts.begin(), cuts.end() );
	std::uint32_t bucket = 0;
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		while ( bucket < cuts.size() && cuts[bucket] <= i )
			++bucket;
		out[i] = { static_cast<std::uint32_t>( 3 * ( 1 + random() % 100 ) ),
		    static_cast<std::uint32_t>( random() % 100000 ),
		    static_cast<std::int32_t>( random() % 2001 ) - 1000, bucket };
	}
	return out;
}

// Scenes whose compacted commands differ from CompactReference on the CPU
// culler's mask; -1 when a device step failed.
int CompactionDisagreements(
    IRenderDevice2 &device, CullKernel &cull, CompactKernel &compact, int scenes )
{
	IndirectChain chain( device, cull, compact );
	if ( !chain.Valid() )
		return -1;
	int differ = 0;
	for ( int seed = 0; seed < scenes; ++seed )
	{
		const std::uint32_t count = seed == 0 ? 0 : 1 + ( std::uint32_t( seed ) * 7919 ) % 20000;
		const Scene scene = RandomScene( 500 + seed, count );
		const std::vector<DrawTemplate> templates = RandomTemplates( seed, count );
		IndirectChain::Output out;
		if ( !chain.Run( scene, templates, 0, {}, out ) )
			return -1;
		const auto expected = CompactReference(
		    CpuMask( scene ), templates, IndirectChain::BucketsOf( templates ), count );
		bool same = out.buckets.size() == expected.size();
		std::size_t expectedDraws = 0;
		for ( std::size_t k = 0; same && k < expected.size(); ++k )
		{
			expectedDraws += expected[k].size();
			same = out.buckets[k].size() == expected[k].size() &&
			       std::equal( out.buckets[k].begin(), out.buckets[k].end(), expected[k].begin(),
			           SameCommand );
		}
		same = same && out.drawCount == expectedDraws;
		if ( !same && ++differ <= 2 )
			std::printf( "  scene %d: %u instances in %zu buckets, gpu draws %u, cpu %zu\n", seed,
			    count, expected.size(), out.drawCount, expectedDraws );
	}
	return differ;
}

void Compaction( testing::Checks &checks, IRenderDevice2 &device, CullKernel &cull )
{
	auto compact = CompactKernel::Create( device );
	if ( !checks.That( compact.HasValue(), "compact.kernel-is-created" ) )
		return;
	const int differ = CompactionDisagreements( device, cull, *compact.Value(), 60 );
	checks.That( differ >= 0, "compact.every-dispatch-runs" );
	checks.Equal( differ, 0,
	    "compact.each-bucket's-commands-and-count-equal-the-reference on 60 seeded scenes (0 to "
	    "20,000 instances, 1 to 64 buckets), in instance order" );
	struct Defect
	{
		const char *name;
		std::span<const std::uint32_t> code;
	};
	const Defect defects[] = {
	    { "drops-last", rendertest::cull::spirv::kCompactDropsLast },
	    { "wrong-instance", rendertest::cull::spirv::kCompactWrongInstance },
	};
	int rejected = 0;
	for ( const Defect &defect : defects )
	{
		auto seeded = CompactKernel::Create( device, defect.code );
		const int seededDiffer =
		    seeded ? CompactionDisagreements( device, cull, *seeded.Value(), 12 ) : -1;
		std::printf(
		    "INFO compact defect %s: %d of 12 scenes disagree\n", defect.name, seededDiffer );
		rejected += seededDiffer > 0;
	}
	checks.Equal( rejected, 2, "compact.defects.each-seeded-kernel-disagrees (2 of 2)" );

	// Draws: the image from the compacted commands equals the image from the
	// CPU culler's kept instances drawn one by one.
	const bool counted = device.Facts().capabilities.Has( Capability::kDrawIndirectCount );
	if ( !counted )
	{
		std::printf( "SKIP compact.draw the device does not claim kDrawIndirectCount\n" );
		return;
	}
	IndirectChain chain( device, cull, *compact.Value() );
	int same = 0;
	int lit = 0;
	int mixed = 0;
	constexpr int kImages = 8;
	for ( int seed = 0; seed < kImages; ++seed )
	{
		const Scene scene = RandomScene( 900 + seed, 256 );
		std::vector<float> vertices;
		std::vector<std::uint32_t> indices;
		std::vector<DrawTemplate> templates;
		IndirectChain::Grid( 256, vertices, indices, templates );
		const std::vector<std::uint32_t> mask = CpuMask( scene );
		std::vector<std::uint32_t> kept;
		for ( std::uint32_t i = 0; i < 256; ++i )
		{
			if ( ( mask[i / 32] >> ( i % 32 ) ) & 1u )
				kept.push_back( i );
		}
		IndirectChain::Output indirect, direct;
		const bool ran = chain.Valid() && chain.Run( scene, templates, 1, {}, indirect ) &&
		                 chain.Run( scene, templates, 2, kept, direct );
		same += ran && !indirect.pixels.empty() && indirect.pixels == direct.pixels &&
		        indirect.drawCount == kept.size();
		std::uint32_t white = 0;
		for ( std::size_t p = 0; p < indirect.pixels.size(); p += 4 )
			white += indirect.pixels[p] == std::byte( 255 );
		lit += ran && white == kept.size() * 16;
		mixed += !kept.empty() && kept.size() < 256;
	}
	checks.Equal( same, kImages,
	    "draw.the-indirect-image-equals-direct-draws-of-the-cpu-kept-instances (8 scenes)" );
	checks.Equal( lit, kImages, "draw.each-kept-instance-and-only-those-lights-its-16-pixels" );
	checks.That( mixed >= kImages / 2, "draw.the-scenes-keep-some-instances-and-cull-others" );
}

// Occlusion culling (S4): a wall drawn into the depth buffer, and cubes in
// front of it, behind it and around its edges. The pyramid's upper levels
// must equal the CPU rebuild of its level 0; the GPU mask must agree with
// OcclusionReference on the read-back pyramid; drawing the wall and only
// the kept cubes must give the image drawing every cube gives, byte for
// byte (nothing visible is culled); and a real share of the frustum-kept
// cubes must be culled.
class OcclusionScene
{
public:
	static constexpr std::uint32_t kSize = 128;
	static constexpr std::uint32_t kCubes = 400;

	struct Result
	{
		bool ran = false;
		std::vector<std::uint32_t> frustum, mask;
		std::vector<float> pyramid;
		std::vector<std::byte> culledImage, fullImage;
		OcclusionView view;
		std::vector<CullInstance> instances;
	};

	OcclusionScene( IRenderDevice2 &device ) : m_Device( device )
	{
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
		        std::as_bytes( std::span( rendertest::cull::spirv::kBoxVertex ) ), "main", {}, 64 },
		    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
		        std::as_bytes( std::span( rendertest::cull::spirv::kBoxFragment ) ), "main", {} } };
		const VertexAttribute attributes[] = {
		    { 0, VertexFormat::kFloat3, 0, 0 }, { 1, VertexFormat::kFloat3, 12, 0 } };
		const VertexBufferLayout buffers[] = { { 24, false } };
		const Format colors[] = { Format::kRGBA8Unorm };
		PipelineDesc desc;
		desc.stages = stages;
		desc.vertex = { attributes, buffers };
		desc.colorFormats = colors;
		desc.depthFormat = Format::kD32Float;
		desc.depthStencil.depthTest = true;
		desc.depthStencil.depthWrite = true;
		desc.depthStencil.compare = CompareOp::kLess;
		desc.raster.cull = CullMode::kNone;
		desc.drawConstantBytes = 64;
		auto pipeline = device.CreatePipeline( desc );
		if ( pipeline )
			m_Pipeline = pipeline.Value();
		else
			std::printf(
			    "INFO occlusion box pipeline: %s\n", DescribeStatus( pipeline.Error().status ) );
		SamplerDesc sampler;
		auto made = device.CreateSampler( sampler );
		if ( made )
			m_Sampler = made.Value();
		else
			std::printf( "INFO occlusion sampler: %s\n", DescribeStatus( made.Error().status ) );
	}
	~OcclusionScene()
	{
		(void)m_Device.WaitIdle();
		if ( m_Pipeline.IsValid() )
			(void)m_Device.Release( m_Pipeline, CompletionToken() );
		if ( m_Sampler.IsValid() )
			(void)m_Device.Release( m_Sampler, CompletionToken() );
		(void)m_Device.Poll();
	}
	bool Valid() const { return m_Pipeline.IsValid() && m_Sampler.IsValid(); }

	// flagEvery > 0: every flagEvery-th cube is flagged kCullNeverOcclude.
	Result Run( std::uint32_t seed, CullKernel &cull, OcclusionKernels &occlusion,
	    std::uint32_t flagEvery = 0 )
	{
		Result out;
		std::mt19937 random( seed * 977 + 13 );
		std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
		// Box 0 is the wall; boxes 1.. are the cubes (instances 0..).
		struct Box
		{
			math::float3 lo, hi, color;
		};
		std::vector<Box> boxes;
		boxes.push_back( { { -60, -20, -50 }, { 60, -15, 50 }, { 0.2f, 0.2f, 0.25f } } );
		for ( std::uint32_t i = 0; i < kCubes; ++i )
		{
			const math::float3 c = { -150 + 300 * unit( random ), -150 + 320 * unit( random ),
			    -120 + 240 * unit( random ) };
			const float h = 1.0f + 9.0f * unit( random );
			boxes.push_back( { { c.x - h, c.y - h, c.z - h }, { c.x + h, c.y + h, c.z + h },
			    { 0.3f + 0.7f * unit( random ), 0.3f + 0.7f * unit( random ),
			        0.3f + 0.7f * unit( random ) } } );
		}
		scene::ViewDesc viewDesc;
		viewDesc.view = math::LookAt( { 0, -200, 0 }, { 0, 0, 0 }, { 0, 0, 1 } );
		viewDesc.projection = math::Perspective( 1.0f, 1.0f, 2.0f, 1000.0f );
		viewDesc.viewBit = 32;
		const scene::SceneView view = scene::MakeView( viewDesc );
		scene::SceneSnapshot snapshot;
		snapshot.instances.resize( kCubes );
		for ( std::uint32_t i = 0; i < kCubes; ++i )
			snapshot.instances[i].worldBounds = { boxes[i + 1].lo, boxes[i + 1].hi };
		out.instances = PackInstances( snapshot );
		for ( std::size_t i = 0; flagEvery && i < out.instances.size(); i += flagEvery )
			out.instances[i].flags = kCullNeverOcclude;
		const CullView cullView = PackView( view, kCubes );
		out.view = MakeOcclusionView( view.viewProjection, kSize, kSize, kCubes );
		std::vector<float> vertices;
		std::vector<std::uint32_t> indices;
		for ( const Box &b : boxes )
		{
			const std::uint32_t base = static_cast<std::uint32_t>( vertices.size() / 6 );
			for ( std::uint32_t c = 0; c < 8; ++c )
				for ( float v : { ( c & 1 ) ? b.hi.x : b.lo.x, ( c & 2 ) ? b.hi.y : b.lo.y,
				          ( c & 4 ) ? b.hi.z : b.lo.z, b.color.x, b.color.y, b.color.z } )
					vertices.push_back( v );
			static const std::uint32_t kFaces[36] = { 0, 1, 3, 0, 3, 2, 4, 6, 7, 4, 7, 5, 0, 4, 5,
			    0, 5, 1, 2, 3, 7, 2, 7, 6, 0, 2, 6, 0, 6, 4, 1, 5, 7, 1, 7, 3 };
			for ( std::uint32_t k : kFaces )
				indices.push_back( base + k );
		}
		float rows[16];
		for ( int r = 0; r < 4; ++r )
		{
			rows[r * 4 + 0] = view.viewProjection.rows[r].x;
			rows[r * 4 + 1] = view.viewProjection.rows[r].y;
			rows[r * 4 + 2] = view.viewProjection.rows[r].z;
			rows[r * 4 + 3] = view.viewProjection.rows[r].w;
		}

		std::vector<BufferId> made;
		auto buffer =
		    [&]( std::uint64_t size, UsageSet usages, MemoryKind memory = MemoryKind::kDeviceLocal )
		{
			BufferDesc desc;
			desc.size = std::max<std::uint64_t>( size, 16 );
			desc.usages = usages;
			desc.memory = memory;
			auto created = m_Device.CreateBuffer( desc );
			made.push_back( created ? created.Value() : BufferId() );
			return made.back();
		};
		const UsageSet in = { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead };
		const std::uint64_t maskBytes = std::uint64_t( MaskWords( kCubes ) ) * 4;
		const std::uint64_t pyramidBytes = PyramidFloats( out.view ) * 4;
		const BufferId instances = buffer( out.instances.size() * sizeof( CullInstance ), in );
		const BufferId cullViewBuffer = buffer( sizeof( CullView ), in );
		const BufferId occlusionView = buffer( sizeof( OcclusionView ), in );
		const BufferId frustum =
		    buffer( maskBytes, { ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead,
		                           ResourceUsage::kCopySource } );
		const BufferId mask =
		    buffer( maskBytes, { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource } );
		const BufferId pyramid =
		    buffer( pyramidBytes, { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource } );
		const BufferId vertexBuffer = buffer(
		    vertices.size() * 4, { ResourceUsage::kCopyDestination, ResourceUsage::kVertex } );
		const BufferId indexBuffer = buffer(
		    indices.size() * 4, { ResourceUsage::kCopyDestination, ResourceUsage::kIndex } );
		const BufferId readMasks = buffer( maskBytes * 2 + pyramidBytes,
		    { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
		const BufferId readPixels =
		    buffer( kSize * kSize * 4, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
		TextureDesc colorDesc;
		colorDesc.format = Format::kRGBA8Unorm;
		colorDesc.width = colorDesc.height = kSize;
		colorDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
		TextureDesc depthDesc = colorDesc;
		depthDesc.format = Format::kD32Float;
		depthDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
		auto color = m_Device.CreateTexture( colorDesc );
		auto depth = m_Device.CreateTexture( depthDesc );
		bool ok = color && depth;
		for ( BufferId id : made )
			ok = ok && id.IsValid();

		// Draws boxes [first, first + count) or the listed cubes after the wall.
		auto draw = [&]( CommandEncoder &e, const std::vector<std::uint32_t> *cubes )
		{
			const ColorAttachment colorAttachment[] = {
			    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
			RenderingDesc rendering;
			rendering.colors = colorAttachment;
			rendering.depth =
			    DepthAttachment{ depth.Value(), LoadOp::kClear, StoreOp::kStore, 1.0f };
			rendering.width = rendering.height = kSize;
			e.BeginRendering( rendering );
			e.SetPipeline( m_Pipeline );
			e.SetDrawConstants( 0, std::as_bytes( std::span( rows ) ) );
			e.SetVertexBuffer( 0, vertexBuffer );
			e.SetIndexBuffer( indexBuffer, 0, IndexFormat::kUint32 );
			e.DrawIndexed( 36, 1, 0, 0, 0 ); // the wall
			if ( cubes )
				for ( std::uint32_t i : *cubes )
				{
					e.SetDrawConstants( 0, std::as_bytes( std::span( rows ) ) );
					e.DrawIndexed( 36, 1, 36 * ( i + 1 ), 0, 0 );
				}
			e.EndRendering();
		};
		auto submit = [&]( CommandEncoder &e )
		{
			auto token = m_Device.Submit( QueueKind::kGraphics, { &e, 1 }, {} );
			return token && Wait( m_Device, token.Value() );
		};
		if ( ok )
		{
			auto encoder = m_Device.BeginEncoder( QueueKind::kGraphics );
			ok = encoder.HasValue();
			if ( ok )
			{
				CommandEncoder &e = encoder.Value();
				auto fill = [&]( BufferId b, std::span<const std::byte> bytes, ResourceUsage after )
				{
					e.TransitionBuffer(
					    b, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
					e.WriteBuffer( b, 0, bytes );
					e.TransitionBuffer( b, ResourceUsage::kCopyDestination, after );
				};
				fill( instances, std::as_bytes( std::span( out.instances ) ),
				    ResourceUsage::kStorageRead );
				fill( cullViewBuffer, std::as_bytes( std::span( &cullView, 1 ) ),
				    ResourceUsage::kStorageRead );
				fill( occlusionView, std::as_bytes( std::span( &out.view, 1 ) ),
				    ResourceUsage::kStorageRead );
				fill(
				    vertexBuffer, std::as_bytes( std::span( vertices ) ), ResourceUsage::kVertex );
				fill( indexBuffer, std::as_bytes( std::span( indices ) ), ResourceUsage::kIndex );
				e.TransitionTexture(
				    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
				e.TransitionTexture(
				    depth.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
				draw( e, nullptr ); // the occluder's depth
				e.TransitionTexture(
				    depth.Value(), ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
				e.TransitionBuffer(
				    frustum, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
				(void)cull.Record( e, { instances, cullViewBuffer, frustum, kCubes } );
				e.TransitionBuffer(
				    frustum, ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead );
				e.TransitionBuffer(
				    pyramid, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
				e.TransitionBuffer( mask, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
				(void)occlusion.RecordPyramid(
				    e, { depth.Value(), m_Sampler, occlusionView, pyramid }, out.view );
				(void)occlusion.RecordOcclusion(
				    e, { instances, occlusionView, pyramid, frustum, mask, kCubes } );
				e.TransitionBuffer(
				    frustum, ResourceUsage::kStorageRead, ResourceUsage::kCopySource );
				e.TransitionBuffer(
				    mask, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
				e.TransitionBuffer(
				    pyramid, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
				e.TransitionBuffer(
				    readMasks, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
				e.CopyBuffer( frustum, readMasks, { 0, 0, maskBytes } );
				e.CopyBuffer( mask, readMasks, { 0, maskBytes, maskBytes } );
				e.CopyBuffer( pyramid, readMasks, { 0, maskBytes * 2, pyramidBytes } );
				ok = submit( e );
				if ( ok )
				{
					cull.Collect( CompletionToken() );
					occlusion.Collect( CompletionToken() );
				}
			}
		}
		if ( ok )
		{
			std::vector<std::byte> bytes( maskBytes * 2 + pyramidBytes );
			ok = m_Device.ReadBuffer( readMasks, 0, bytes ).HasValue();
			out.frustum.resize( MaskWords( kCubes ) );
			out.mask.resize( MaskWords( kCubes ) );
			out.pyramid.resize( PyramidFloats( out.view ) );
			std::memcpy( out.frustum.data(), bytes.data(), maskBytes );
			std::memcpy( out.mask.data(), bytes.data() + maskBytes, maskBytes );
			std::memcpy( out.pyramid.data(), bytes.data() + maskBytes * 2, pyramidBytes );
		}
		// The two images: every frustum-kept cube, and the occlusion-kept ones.
		auto image = [&]( const std::vector<std::uint32_t> &words, std::vector<std::byte> &pixels )
		{
			std::vector<std::uint32_t> cubes;
			for ( std::uint32_t i = 0; i < kCubes; ++i )
				if ( ( words[i / 32] >> ( i % 32 ) ) & 1u )
					cubes.push_back( i );
			auto encoder = m_Device.BeginEncoder( QueueKind::kGraphics );
			if ( !encoder )
				return false;
			CommandEncoder &e = encoder.Value();
			e.TransitionTexture(
			    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
			e.TransitionTexture(
			    depth.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
			draw( e, &cubes );
			e.TransitionTexture(
			    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
			e.TransitionBuffer(
			    readPixels, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			e.CopyTextureToBuffer( color.Value(), readPixels, { 0, 0, 0, kSize, kSize } );
			if ( !submit( e ) )
				return false;
			pixels.resize( kSize * kSize * 4 );
			return m_Device.ReadBuffer( readPixels, 0, pixels ).HasValue();
		};
		ok = ok && image( out.frustum, out.fullImage ) && image( out.mask, out.culledImage );
		out.ran = ok;
		(void)m_Device.WaitIdle();
		if ( color )
			(void)m_Device.Release( color.Value(), CompletionToken() );
		if ( depth )
			(void)m_Device.Release( depth.Value(), CompletionToken() );
		for ( BufferId id : made )
			if ( id.IsValid() )
				(void)m_Device.Release( id, CompletionToken() );
		(void)m_Device.Poll();
		return out;
	}

private:
	IRenderDevice2 &m_Device;
	PipelineId m_Pipeline;
	SamplerId m_Sampler;
};

int Bits( const std::vector<std::uint32_t> &words )
{
	int n = 0;
	for ( std::uint32_t w : words )
		n += std::popcount( w );
	return n;
}

void Occlusion( testing::Checks &checks, IRenderDevice2 &device, CullKernel &cull )
{
	auto kernels = OcclusionKernels::Create( device );
	if ( !checks.That( kernels.HasValue(), "occlusion.kernels-are-created" ) )
		return;
	OcclusionScene scene( device );
	if ( !checks.That( scene.Valid(), "occlusion.the-draw-fixture-is-created" ) )
		return;
	constexpr int kScenes = 6;
	int ran = 0, pyramids = 0, sameImages = 0;
	int kept = 0, culled = 0, disagreements = 0;
	for ( int seed = 0; seed < kScenes; ++seed )
	{
		const OcclusionScene::Result r = scene.Run( seed, cull, *kernels.Value() );
		if ( !r.ran )
			continue;
		++ran;
		std::vector<float> level0(
		    r.pyramid.begin(), r.pyramid.begin() + r.view.width * r.view.height );
		pyramids += DepthPyramidReference( level0, r.view ) == r.pyramid;
		sameImages += r.culledImage == r.fullImage;
		kept += Bits( r.frustum );
		culled += Bits( r.frustum ) - Bits( r.mask );
		const auto expected = OcclusionReference( r.instances, r.frustum, r.pyramid, r.view );
		for ( std::size_t w = 0; w < expected.size(); ++w )
			disagreements += std::popcount( expected[w] ^ r.mask[w] );
	}
	std::printf( "INFO occlusion: %d of %d frustum-kept cubes culled over %d scenes; %d "
	             "disagree with the CPU reference\n",
	    culled, kept, ran, disagreements );
	checks.Equal( ran, kScenes, "occlusion.every-scene-runs" );
	checks.Equal( pyramids, kScenes, "occlusion.pyramid-levels-equal-the-cpu-rebuild-exactly" );
	checks.Equal( sameImages, kScenes,
	    "occlusion.drawing-only-kept-cubes-gives-the-full-image-byte-for-byte" );
	checks.That( culled * 10 >= kept, "occlusion.at-least-a-tenth-of-the-kept-cubes-are-culled" );
	checks.That( disagreements * 200 <= kept,
	    "occlusion.gpu-and-cpu-reference-agree-on-at-least-99.5-percent" );

	// kCullNeverOcclude: every third cube flagged; each flagged cube the
	// frustum keeps stays, the rest are tested as before, and the image holds.
	int flaggedKept = 0, flaggedDropped = 0, flaggedRan = 0, flaggedImages = 0;
	int flaggedDisagree = 0, flaggedCulled = 0;
	for ( int seed = 0; seed < kScenes; ++seed )
	{
		const OcclusionScene::Result r = scene.Run( seed, cull, *kernels.Value(), 3 );
		if ( !r.ran )
			continue;
		++flaggedRan;
		flaggedImages += r.culledImage == r.fullImage;
		flaggedCulled += Bits( r.frustum ) - Bits( r.mask );
		for ( std::size_t i = 0; i < r.instances.size(); i += 3 )
		{
			const bool inFrustum = ( r.frustum[i / 32] >> ( i % 32 ) ) & 1u;
			const bool inMask = ( r.mask[i / 32] >> ( i % 32 ) ) & 1u;
			flaggedKept += inFrustum && inMask;
			flaggedDropped += inFrustum && !inMask;
		}
		const auto expected = OcclusionReference( r.instances, r.frustum, r.pyramid, r.view );
		for ( std::size_t w = 0; w < expected.size(); ++w )
			flaggedDisagree += std::popcount( expected[w] ^ r.mask[w] );
	}
	std::printf( "INFO occlusion never-occlude: %d flagged kept, %d dropped, %d culled, %d "
	             "disagree\n",
	    flaggedKept, flaggedDropped, flaggedCulled, flaggedDisagree );
	checks.Equal( flaggedRan, kScenes, "occlusion.never-occlude.every-scene-runs" );
	checks.That( flaggedKept > 0 && flaggedDropped == 0,
	    "occlusion.never-occlude.every-flagged-frustum-kept-cube-stays" );
	checks.That( flaggedCulled > 0 && flaggedCulled < culled,
	    "occlusion.never-occlude.unflagged-cubes-are-still-culled" );
	checks.Equal( flaggedDisagree, 0, "occlusion.never-occlude.the-reference-honours-the-flag" );
	checks.Equal( flaggedImages, kScenes, "occlusion.never-occlude.the-image-holds" );

	struct Defect
	{
		const char *name;
		std::span<const std::uint32_t> code;
	};
	const Defect defects[] = {
	    { "far-corner", rendertest::cull::spirv::kOcclusionFarCorner },
	    { "one-texel", rendertest::cull::spirv::kOcclusionOneTexel },
	};
	int rejected = 0;
	for ( const Defect &defect : defects )
	{
		auto seeded = OcclusionKernels::Create( device, defect.code );
		int changed = 0;
		for ( int seed = 0; seeded && seed < kScenes; ++seed )
		{
			const OcclusionScene::Result r = scene.Run( seed, cull, *seeded.Value() );
			changed += r.ran && r.culledImage != r.fullImage;
		}
		std::printf(
		    "INFO occlusion defect %s: %d of %d images change\n", defect.name, changed, kScenes );
		rejected += changed > 0;
	}
	checks.Equal( rejected, 2, "occlusion.defects.each-seeded-kernel-changes-an-image (2 of 2)" );
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

		Compaction( checks, *device, *kernel.Value() );
		Occlusion( checks, *device, *kernel.Value() );
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
