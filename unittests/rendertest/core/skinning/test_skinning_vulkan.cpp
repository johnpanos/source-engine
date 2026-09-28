//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.skinning (RFC 0016 K6) on render.device.vulkan: skin.comp,
//			dispatched as a render.graph compute pass through the serial
//			executor, against the CPU oracle SkinReference.
//
//			- gpu.equals-cpu: over 64 seeded meshes (up to 4,000 vertices,
//			  128 bones, 40 stereo flexes, some bone indices past the
//			  palette), the largest position difference is at most 1e-3
//			  units and the largest normal and tangent component difference
//			  at most 1e-3 (the K6 tolerance), and the wrinkle agrees too;
//			- defects: the seeded bone-index and flex-weight variants of
//			  skin.comp (skin_defects_spv.h) each exceed the tolerance
//			  (2 of 2);
//			- validation: with the Khronos validation layer (synchronization
//			  validation included) the run reports no message; without the
//			  layer the clause prints SKIP and certifies nothing.
//
//			Real content (every model in the K0 views and Portal 2's
//			characters) is the next K6 slice; these meshes are synthetic.
//			RENDER_VK_ADAPTER=<n> picks the physical device. A missing
//			Vulkan device fails the run.
//
//=============================================================================//

#include "spv/skin_defects_spv.h"
#include "skinning_fixtures.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "testing/checks.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>
#include <thread>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::pass::skinning;
namespace fixtures = rendertest::skinning;

constexpr float kTolerance = 1e-3f;

bool Wait( IRenderDevice2 &device, CompletionToken token )
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

template <typename T> std::span<const std::byte> Bytes( const std::vector<T> &values )
{
	return std::as_bytes( std::span<const T>( values ) );
}

// Skins `mesh` on the device through a graph pass and reads the result back.
// Returns false when any device step fails.
bool SkinOnDevice( IRenderDevice2 &device, SkinningKernel &kernel, const fixtures::Mesh &mesh,
    std::vector<SkinnedVertex> &out )
{
	const std::uint32_t vertexCount = static_cast<std::uint32_t>( mesh.vertices.size() );
	std::vector<std::uint32_t> offsets = mesh.flexOffsets;
	if ( offsets.empty() )
		offsets.assign( vertexCount + 1, 0 );
	std::vector<FlexDelta> deltas = mesh.flexDeltas;
	if ( deltas.empty() )
		deltas.resize( 1 );
	std::vector<FlexWeights> weights = mesh.flexWeights;
	if ( weights.empty() )
		weights.resize( 1 );

	std::vector<BufferId> created;
	auto make =
	    [&]( std::uint64_t size, UsageSet usages, MemoryKind memory = MemoryKind::kDeviceLocal )
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = usages;
		desc.memory = memory;
		auto buffer = device.CreateBuffer( desc );
		if ( !buffer )
			return std::make_pair( BufferId(), desc );
		created.push_back( buffer.Value() );
		return std::make_pair( buffer.Value(), desc );
	};
	struct Input
	{
		BufferId id;
		BufferDesc desc;
		std::span<const std::byte> bytes;
	};
	auto input = [&]( std::span<const std::byte> bytes )
	{
		auto [id, desc] =
		    make( bytes.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead } );
		return Input{ id, desc, bytes };
	};
	Input inputs[] = { input( Bytes( mesh.vertices ) ), input( Bytes( mesh.bones ) ),
	    input( Bytes( offsets ) ), input( Bytes( deltas ) ), input( Bytes( weights ) ) };
	const std::uint64_t outBytes = std::uint64_t( vertexCount ) * sizeof( SkinnedVertex );
	auto [output, outputDesc] =
	    make( outBytes, { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource } );
	auto [readback, readbackDesc] =
	    make( outBytes, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
	bool ok = output.IsValid() && readback.IsValid();
	for ( const Input &i : inputs )
		ok = ok && i.id.IsValid();

	if ( ok )
	{
		graph::GraphBuilder builder;
		graph::ResourceRef refs[5];
		const char *names[] = { "vertices", "bones", "offsets", "deltas", "weights" };
		for ( int i = 0; i < 5; ++i )
			refs[i] = builder.ImportBuffer( names[i], inputs[i].id, inputs[i].desc,
			    ResourceUsage::kUndefined, ResourceUsage::kStorageRead );
		const graph::ResourceRef skinned = builder.ImportBuffer(
		    "skinned", output, outputDesc, ResourceUsage::kUndefined, ResourceUsage::kCopySource );
		const graph::ResourceRef copy = builder.ImportBuffer( "readback", readback, readbackDesc,
		    ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		// Uploads: one copy pass writing every input.
		graph::PassBuilder upload = builder.AddPass( "upload", graph::PassKind::kCopy );
		for ( const graph::ResourceRef &ref : refs )
			upload.Write( ref, ResourceUsage::kCopyDestination );
		upload.Execute(
		    [&]( graph::RecordContext &context )
		    {
			    for ( int i = 0; i < 5; ++i )
				    context.Encoder().WriteBuffer( context.Buffer( refs[i] ), 0, inputs[i].bytes );
		    } );
		SkinningPassResources resources{ refs[0], refs[1], refs[2], refs[3], refs[4], skinned,
		    vertexCount, static_cast<std::uint32_t>( mesh.bones.size() ),
		    static_cast<std::uint32_t>( deltas.size() ),
		    static_cast<std::uint32_t>( weights.size() ) };
		AddSkinningPass( builder, kernel, resources );
		builder.AddPass( "readback", graph::PassKind::kCopy )
		    .Read( skinned, ResourceUsage::kCopySource )
		    .Write( copy, ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [&]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyBuffer(
			            context.Buffer( skinned ), context.Buffer( copy ), { 0, 0, outBytes } );
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ) );
		ok = compiled.HasValue();
		if ( ok )
		{
			graph::SerialGraphExecutor executor;
			auto executed = executor.Execute( compiled.Value(), device );
			ok = executed.HasValue() && Wait( device, executed.Value().token );
			if ( executed )
				kernel.Collect( executed.Value().token );
		}
	}
	if ( ok )
	{
		out.resize( vertexCount );
		ok = device
		         .ReadBuffer(
		             readback, 0, std::as_writable_bytes( std::span<SkinnedVertex>( out ) ) )
		         .HasValue();
	}
	for ( BufferId id : created )
		(void)device.Release( id, CompletionToken() );
	(void)device.Poll();
	return ok;
}

fixtures::Mesh MeshFor( std::uint32_t seed )
{
	return fixtures::RandomMesh( 1000 + seed, 1 + ( seed * 977 ) % 4000, 1 + ( seed * 13 ) % 128,
	    seed % 3 ? 1 + seed % 40 : 0 );
}

// The worst error of `kernel` against the oracle over the seeded meshes;
// -1 when a device step failed.
SkinError Measure( IRenderDevice2 &device, SkinningKernel &kernel, int meshes, bool &ok )
{
	SkinError worst;
	for ( int seed = 0; seed < meshes && ok; ++seed )
	{
		const fixtures::Mesh mesh = MeshFor( static_cast<std::uint32_t>( seed ) );
		std::vector<SkinnedVertex> expected( mesh.vertices.size() );
		SkinReference( mesh.Inputs(), expected );
		std::vector<SkinnedVertex> actual;
		ok = SkinOnDevice( device, kernel, mesh, actual );
		if ( !ok )
			break;
		const SkinError error = CompareSkinned( actual, expected );
		worst.position = std::max( worst.position, error.position );
		worst.normal = std::max( worst.normal, error.normal );
		worst.tangent = std::max( worst.tangent, error.tangent );
		worst.wrinkle = std::max( worst.wrinkle, error.wrinkle );
	}
	return worst;
}

bool Within( const SkinError &error )
{
	return error.position <= kTolerance && error.normal <= kTolerance &&
	       error.tangent <= kTolerance && error.wrinkle <= kTolerance;
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
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<IRenderDevice2> device = std::move( created ).Value();
		const std::string_view name = device->Facts().adapterName;
		std::printf(
		    "INFO skinning adapter: %.*s\n", static_cast<int>( name.size() ), name.data() );

		auto kernel = SkinningKernel::Create( *device );
		if ( !checks.That( kernel.HasValue(), "kernel.is-created" ) )
			return checks.Report();
		bool ok = true;
		const SkinError error = Measure( *device, *kernel.Value(), 64, ok );
		checks.That( ok, "gpu.every-dispatch-runs" );
		std::printf(
		    "INFO skinning gpu vs cpu: position %.3g normal %.3g tangent %.3g wrinkle %.3g "
		    "(64 meshes)\n",
		    error.position, error.normal, error.tangent, error.wrinkle );
		checks.That( ok && Within( error ), "gpu.equals-cpu within 1e-3" );
		checks.Equal( kernel.Value()->RecordFailures(), 0u, "gpu.records-without-failure" );

		struct Defect
		{
			const char *name;
			std::span<const std::uint32_t> code;
		};
		const Defect defects[] = {
		    { "bone-index", rendertest::skinning::spirv::kSkinBoneIndexError },
		    { "flex-weight", rendertest::skinning::spirv::kSkinFlexWeightError },
		};
		for ( const Defect &defect : defects )
		{
			auto broken = SkinningKernel::Create( *device, defect.code );
			bool ran = broken.HasValue();
			const SkinError seeded =
			    ran ? Measure( *device, *broken.Value(), 16, ran ) : SkinError();
			std::printf( "INFO skinning seeded %s: position %.3g normal %.3g\n", defect.name,
			    seeded.position, seeded.normal );
			checks.That(
			    ran && !Within( seeded ), std::string( "defects.detects-" ) + defect.name );
		}
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
