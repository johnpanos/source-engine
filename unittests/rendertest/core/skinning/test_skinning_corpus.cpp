//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.skinning.corpus (RFC 0016 K6 "GPU equals CPU skinning" on
//			real content): skin.comp against studiorender's software skinning
//			(R_StudioSoftwareProcessMesh, the legacy CPU path), captured from
//			product runs by studiorender/skin_capture.cpp and compacted by
//			tools/render/skin_corpus.py.
//
//			For every captured mesh the captured input (flexed where the
//			vertex was flexed, so flex is already applied) and pose-to-world
//			palette go through the kernel as a render.graph compute pass. The
//			GPU output must equal the legacy output within 1e-3 per component
//			in normal and tangent, and in position within the larger of 1e-3
//			units and 4 ulp of the legacy coordinate (tolerance version 2,
//			RFC 0016 K6: fp32 spacing is 4.9e-4 at 7,000 units, where Portal
//			2's maps place models, and the legacy path blends matrices where
//			the kernel blends points, so both round differently). The CPU
//			oracle SkinReference is compared too, which separates a kernel
//			error from a difference in formulation (the kernel's third weight
//			is 1 - w0 - w1, as hardware skinning computes it). The seeded
//			bone-index and flex-weight kernels must miss on the corpus.
//
//			RENDER_SKIN_CORPUS names the corpus file. Missing, empty or
//			unreadable corpora fail the run.
//
//=============================================================================//

#include "skin_defects_spv.h"
#include "skinning_fixtures.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "testing/checks.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::pass::skinning;
namespace vulkan = render::device::vulkan;

constexpr float kTolerance = 1e-3f;
constexpr float kPositionUlps = 4.0f;

// The position tolerance at a legacy coordinate (tolerance version 2).
float PositionTolerance( float coordinate )
{
	const float magnitude = std::fabs( coordinate );
	const float ulp = std::nextafter( magnitude, INFINITY ) - magnitude;
	return std::max( kTolerance, kPositionUlps * ulp );
}

// The largest position error as a fraction of its tolerance (<= 1 passes).
float PositionRatio(
    const std::vector<SkinnedVertex> &actual, const std::vector<SkinnedVertex> &legacy )
{
	if ( actual.size() != legacy.size() )
		return INFINITY;
	float worst = 0.0f;
	for ( std::size_t v = 0; v < actual.size(); ++v )
		for ( int k = 0; k < 3; ++k )
		{
			const float error = std::fabs( actual[v].position[k] - legacy[v].position[k] );
			worst = std::isnan( error )
			            ? INFINITY
			            : std::max( worst, error / PositionTolerance( legacy[v].position[k] ) );
		}
	return worst;
}

struct CorpusMesh
{
	std::vector<SkinVertex> vertices;
	std::vector<BoneMatrix> bones;
	std::vector<SkinnedVertex> legacy;
	std::uint32_t flexed = 0;
	std::uint32_t tangents = 0;
};

// Parses the compacted corpus (the capture format, see skin_capture.h).
bool LoadCorpus( const char *path, std::vector<CorpusMesh> &meshes )
{
	std::ifstream file( path, std::ios::binary );
	std::vector<char> data(
	    ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );
	if ( data.size() < 8 || std::memcmp( data.data(), "SKCAP001", 8 ) != 0 )
		return false;
	std::size_t at = 8;
	auto take = [&]( void *out, std::size_t bytes )
	{
		if ( at + bytes > data.size() )
			return false;
		std::memcpy( out, data.data() + at, bytes );
		at += bytes;
		return true;
	};
	while ( at < data.size() )
	{
		char magic[4];
		std::uint32_t header[2];
		if ( !take( magic, 4 ) || std::memcmp( magic, "MESH", 4 ) != 0 || !take( header, 8 ) )
			return false;
		CorpusMesh mesh;
		mesh.bones.resize( header[1] );
		if ( !take( mesh.bones.data(), mesh.bones.size() * sizeof( BoneMatrix ) ) )
			return false;
		for ( std::uint32_t v = 0; v < header[0]; ++v )
		{
			float in[10], weights[3], out[10];
			std::uint8_t bones[4];
			std::uint32_t flags;
			if ( !take( in, sizeof( in ) ) || !take( bones, 4 ) || !take( weights, 12 ) ||
			     !take( &flags, 4 ) || !take( out, sizeof( out ) ) )
				return false;
			SkinVertex vertex;
			std::memcpy( vertex.position, in, 12 );
			std::memcpy( vertex.normal, in + 3, 12 );
			std::memcpy( vertex.tangent, in + 6, 16 );
			const int count = bones[0];
			vertex.weight0 = weights[0];
			vertex.weight1 = count >= 2 ? weights[1] : 0.0f;
			std::uint32_t packed = 0;
			for ( int b = 0; b < 3; ++b )
				packed |= std::uint32_t( b < count ? bones[1 + b] : bones[1] ) << ( 8 * b );
			vertex.bones = packed;
			mesh.vertices.push_back( vertex );
			SkinnedVertex legacy;
			std::memcpy( legacy.position, out, 12 );
			std::memcpy( legacy.normal, out + 3, 12 );
			std::memcpy( legacy.tangent, out + 6, 16 );
			if ( !( flags & 2u ) )
				std::memset( legacy.tangent, 0, sizeof( legacy.tangent ) );
			mesh.legacy.push_back( legacy );
			mesh.flexed += ( flags & 1u ) != 0;
			mesh.tangents += ( flags & 2u ) != 0;
		}
		meshes.push_back( std::move( mesh ) );
	}
	return true;
}

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

// One dispatch through the graph; false when a device step fails.
bool Skin( IRenderDevice2 &device, SkinningKernel &kernel, const CorpusMesh &mesh,
    std::vector<SkinnedVertex> &out )
{
	const std::uint32_t count = static_cast<std::uint32_t>( mesh.vertices.size() );
	const std::vector<std::uint32_t> offsets( count + 1, 0 );
	const std::vector<FlexDelta> deltas( 1 );
	const std::vector<FlexWeights> weights( 1 );
	std::vector<BufferId> created;
	auto make = [&]( std::uint64_t size, UsageSet usages, MemoryKind memory )
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = usages;
		desc.memory = memory;
		auto buffer = device.CreateBuffer( desc );
		if ( buffer )
			created.push_back( buffer.Value() );
		return std::make_pair( buffer ? buffer.Value() : BufferId(), desc );
	};
	const std::span<const std::byte> inputs[] = { Bytes( mesh.vertices ), Bytes( mesh.bones ),
	    Bytes( offsets ), Bytes( deltas ), Bytes( weights ) };
	const std::uint64_t outBytes = std::uint64_t( count ) * sizeof( SkinnedVertex );
	graph::GraphBuilder builder;
	graph::ResourceRef refs[5];
	bool ok = true;
	for ( int i = 0; i < 5; ++i )
	{
		auto [id, desc] = make( inputs[i].size(),
		    { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead },
		    MemoryKind::kDeviceLocal );
		ok = ok && id.IsValid();
		refs[i] = builder.ImportBuffer(
		    "input", id, desc, ResourceUsage::kUndefined, ResourceUsage::kStorageRead );
	}
	auto [output, outputDesc] = make( outBytes,
	    { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource }, MemoryKind::kDeviceLocal );
	auto [readback, readbackDesc] =
	    make( outBytes, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
	ok = ok && output.IsValid() && readback.IsValid();
	if ( ok )
	{
		const graph::ResourceRef skinned = builder.ImportBuffer(
		    "skinned", output, outputDesc, ResourceUsage::kUndefined, ResourceUsage::kCopySource );
		const graph::ResourceRef copy = builder.ImportBuffer( "readback", readback, readbackDesc,
		    ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		graph::PassBuilder upload = builder.AddPass( "upload", graph::PassKind::kCopy );
		for ( const graph::ResourceRef &ref : refs )
			upload.Write( ref, ResourceUsage::kCopyDestination );
		upload.Execute(
		    [&]( graph::RecordContext &context )
		    {
			    for ( int i = 0; i < 5; ++i )
				    context.Encoder().WriteBuffer( context.Buffer( refs[i] ), 0, inputs[i] );
		    } );
		AddSkinningPass( builder, kernel,
		    { refs[0], refs[1], refs[2], refs[3], refs[4], skinned, count,
		        static_cast<std::uint32_t>( mesh.bones.size() ), 1, 1 } );
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
		if ( ok )
		{
			out.resize( count );
			ok = device
			         .ReadBuffer(
			             readback, 0, std::as_writable_bytes( std::span<SkinnedVertex>( out ) ) )
			         .HasValue();
		}
	}
	for ( BufferId id : created )
		(void)device.Release( id, CompletionToken() );
	(void)device.Poll();
	return ok;
}

// Tangents of meshes captured without tangent space are zero on both sides.
void DropTangents( const CorpusMesh &mesh, std::vector<SkinnedVertex> &values )
{
	if ( mesh.tangents == 0 )
		for ( SkinnedVertex &value : values )
			std::memset( value.tangent, 0, sizeof( value.tangent ) );
}

void Worst( SkinError &into, const SkinError &error )
{
	into.position = std::max( into.position, error.position );
	into.normal = std::max( into.normal, error.normal );
	into.tangent = std::max( into.tangent, error.tangent );
}

// Normal and tangent against 1e-3; position is judged by PositionRatio.
bool DirectionsWithin( const SkinError &error )
{
	return error.normal <= kTolerance && error.tangent <= kTolerance;
}

} // namespace

int main()
{
	testing::Checks checks;
	const char *path = std::getenv( "RENDER_SKIN_CORPUS" );
	std::vector<CorpusMesh> meshes;
	const bool loaded = path && LoadCorpus( path, meshes );
	if ( !checks.That( loaded && !meshes.empty(), "corpus.loads-and-is-not-empty" ) )
		return checks.Report();
	std::uint64_t vertices = 0, flexed = 0;
	for ( const CorpusMesh &mesh : meshes )
	{
		vertices += mesh.vertices.size();
		flexed += mesh.flexed;
	}
	std::printf( "INFO skinning corpus: %zu meshes, %llu vertices (%llu flexed) from %s\n",
	    meshes.size(), static_cast<unsigned long long>( vertices ),
	    static_cast<unsigned long long>( flexed ), path );

	vulkan::VulkanAdapterOptions options;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	auto created = vulkan::Create( options );
	if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
		return checks.Report();
	std::unique_ptr<IRenderDevice2> device = std::move( created ).Value();
	auto kernel = SkinningKernel::Create( *device );
	auto boneDefect =
	    SkinningKernel::Create( *device, rendertest::skinning::spirv::kSkinBoneIndexError );
	if ( !checks.That( kernel && boneDefect, "kernel.is-created" ) )
		return checks.Report();

	SkinError gpu, cpu, seeded;
	std::size_t failedMeshes = 0, worstMesh = 0, blendedMeshes = 0;
	float gpuRatio = 0.0f, cpuRatio = 0.0f, seededRatio = 0.0f;
	bool ran = true;
	for ( std::size_t m = 0; m < meshes.size() && ran; ++m )
	{
		const CorpusMesh &mesh = meshes[m];
		std::vector<SkinnedVertex> actual;
		ran = Skin( *device, *kernel.Value(), mesh, actual );
		if ( !ran )
			break;
		DropTangents( mesh, actual );
		const SkinError error = CompareSkinned( actual, mesh.legacy );
		const float ratio = PositionRatio( actual, mesh.legacy );
		Worst( gpu, error );
		if ( !DirectionsWithin( error ) || ratio > 1.0f )
			++failedMeshes;
		if ( ratio > gpuRatio )
		{
			gpuRatio = ratio;
			worstMesh = m;
		}
		std::vector<SkinnedVertex> reference( mesh.vertices.size() );
		SkinReference( { mesh.vertices, mesh.bones, {}, {}, {} }, reference );
		DropTangents( mesh, reference );
		Worst( cpu, CompareSkinned( reference, mesh.legacy ) );
		cpuRatio = std::max( cpuRatio, PositionRatio( reference, mesh.legacy ) );
		// The seeded kernel reads each vertex's bones rotated by a byte, which
		// only changes vertices whose bones differ.
		bool blended = false;
		for ( const SkinVertex &vertex : mesh.vertices )
			blended = blended || ( vertex.bones & 0xffu ) != ( ( vertex.bones >> 8 ) & 0xffu );
		if ( blended )
		{
			++blendedMeshes;
			std::vector<SkinnedVertex> broken;
			ran = Skin( *device, *boneDefect.Value(), mesh, broken );
			DropTangents( mesh, broken );
			Worst( seeded, CompareSkinned( broken, mesh.legacy ) );
			seededRatio = std::max( seededRatio, PositionRatio( broken, mesh.legacy ) );
		}
	}
	std::printf(
	    "INFO skinning corpus gpu vs legacy: position %.3g units (%.2f of its tolerance) normal "
	    "%.3g tangent %.3g; %zu meshes over tolerance, worst mesh %zu (%zu vertices)\n",
	    gpu.position, gpuRatio, gpu.normal, gpu.tangent, failedMeshes, worstMesh,
	    meshes[worstMesh].vertices.size() );
	std::printf( "INFO skinning corpus cpu oracle vs legacy: position %.3g (%.2f) normal %.3g "
	             "tangent %.3g\n",
	    cpu.position, cpuRatio, cpu.normal, cpu.tangent );
	std::printf(
	    "INFO skinning corpus seeded bone-index kernel: position %.3g (%.1f of its tolerance) over "
	    "%zu blended meshes\n",
	    seeded.position, seededRatio, blendedMeshes );
	checks.That( ran, "gpu.every-mesh-runs" );
	checks.That( ran && DirectionsWithin( gpu ) && gpuRatio <= 1.0f,
	    "gpu.equals-legacy-cpu-skinning within tolerance v2" );
	checks.That( ran && DirectionsWithin( cpu ) && cpuRatio <= 1.0f,
	    "oracle.equals-legacy-cpu-skinning within tolerance v2" );
	checks.That( blendedMeshes > 0, "corpus.has-blended-vertices" );
	checks.That( ran && seededRatio > 1.0f, "defects.detects-bone-index-on-the-corpus" );
	(void)device->WaitIdle();
	return checks.Report();
}
