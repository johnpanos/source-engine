//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.vulkan.emit-convert-batch (R32, Q-JOBS): the chunked form of
//          native Vulkan emit's per-vertex conversion (vulkan_emit_convert.h)
//          on the job system's synchronous batch must produce the serial
//          conversion's records and bone maximum byte for byte.
//
//          The conversion is a stand-in with the product's shape: slots in
//          first-use order map to mesh vertices; each record is three-bone
//          skinning of a position and a normal plus copied attributes, 22
//          floats; the bone maximum follows EmitToNativeQueue's rule (an index
//          past the palette reads bone 0). Draw sizes cross the chunk
//          boundaries, chunk sizes include one slot, and 1, 2 and N real
//          worker threads run the batch. The byte comparison is shown to catch
//          a chunk-offset defect. Build the same source with
//          -fsanitize=thread for the TSan lane; -DEMIT_CONVERT_SEED_RACE adds
//          the lane's negative control (chunks raising one shared, unguarded
//          bone maximum), which ThreadSanitizer must report.
//
//===========================================================================//

#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"
#include "testing/conformance_result.h"

#include "materialsystem/shaderapivulkan/vulkan_emit_convert.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace
{
using render_vulkan::CEmitConvertBatch;
using render_vulkan::EmitConvertFault;

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const std::string &description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL: %s\n", description.c_str() );
	}
}

// Real threads with a join barrier: one per participant.
class ThreadBackend final : public jobsystem::IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ) {}
	void ParallelFor( int count, const std::function<void( int )> &body ) override
	{
		std::vector<std::thread> threads;
		for ( int index = 1; index < count; ++index )
			threads.emplace_back( body, index );
		if ( count > 0 )
			body( 0 );
		for ( auto &thread : threads )
			thread.join();
	}
	int WorkerCount() const override { return m_workers; }

private:
	int m_workers;
};

enum
{
	kRecordFloats = 22, // CVulkanContext::kDynVertexFloats
	kMaxBoneMatrices = 53
};

struct Vertex
{
	float position[3];
	float normal[3];
	float weights[2];
	unsigned char bones[3];
	float uv[2];
};

struct Draw
{
	std::vector<Vertex> vertices;
	std::vector<int> slotVertex; // slot -> vertex, first-use order
	float bones[kMaxBoneMatrices][12];
};

// A draw of `slots` unique vertices referenced by an index list twice as long,
// in first-use order, as EmitToNativeQueue's slot assignment gives them.
Draw MakeDraw( uint32_t slots, uint32_t seed )
{
	std::mt19937 rng( seed );
	std::uniform_real_distribution<float> unit( -1.0f, 1.0f );
	Draw draw;
	const uint32_t meshVertices = slots + slots / 3 + 1;
	draw.vertices.resize( meshVertices );
	for ( Vertex &v : draw.vertices )
	{
		for ( int k = 0; k < 3; ++k )
		{
			v.position[k] = 64.0f * unit( rng );
			v.normal[k] = unit( rng );
		}
		v.weights[0] = 0.5f + 0.5f * unit( rng );
		v.weights[1] = 0.25f + 0.25f * unit( rng );
		// Indices up to 60: some past the palette, which read bone 0.
		for ( unsigned char &bone : v.bones )
			bone = static_cast<unsigned char>( rng() % 61 );
		v.uv[0] = unit( rng );
		v.uv[1] = unit( rng );
	}
	for ( auto &bone : draw.bones )
		for ( float &value : bone )
			value = unit( rng );
	std::vector<uint32_t> stamp( meshVertices, 0 );
	std::uniform_int_distribution<uint32_t> pick( 0, meshVertices - 1 );
	while ( draw.slotVertex.size() < slots )
	{
		const uint32_t v = pick( rng );
		if ( !stamp[v] )
		{
			stamp[v] = 1;
			draw.slotVertex.push_back( static_cast<int>( v ) );
		}
	}
	return draw;
}

void ConvertVertex( const Draw &draw, int v, float *out, int &maxBone )
{
	const Vertex &vertex = draw.vertices[static_cast<size_t>( v )];
	const float weights[3] = {
	    vertex.weights[0], vertex.weights[1], 1.0f - ( vertex.weights[0] + vertex.weights[1] ) };
	float position[3] = { 0.0f, 0.0f, 0.0f }, normal[3] = { 0.0f, 0.0f, 0.0f };
	for ( int b = 0; b < 3; ++b )
	{
		const int bone = vertex.bones[b] < kMaxBoneMatrices ? vertex.bones[b] : 0;
		maxBone = std::max( maxBone, bone );
		const float *m = draw.bones[bone];
		for ( int r = 0; r < 3; ++r )
		{
			position[r] +=
			    weights[b] * ( m[r * 4] * vertex.position[0] + m[r * 4 + 1] * vertex.position[1] +
			                     m[r * 4 + 2] * vertex.position[2] + m[r * 4 + 3] );
			normal[r] +=
			    weights[b] * ( m[r * 4] * vertex.normal[0] + m[r * 4 + 1] * vertex.normal[1] +
			                     m[r * 4 + 2] * vertex.normal[2] );
		}
	}
	std::fill( out, out + kRecordFloats, 0.0f );
	std::memcpy( out, position, sizeof( position ) );
	out[6] = vertex.uv[0];
	out[7] = vertex.uv[1];
	std::memcpy( out + 10, normal, sizeof( normal ) );
	out[17] = weights[2];
}

struct Result
{
	std::vector<float> records;
	int maxBone = -1;
	bool accepted = true;
	int threads = 0; // distinct threads that ran chunks
};

// The serial conversion EmitToNativeQueue runs by default: the oracle.
Result ConvertSerial( const Draw &draw )
{
	Result result;
	result.records.assign( draw.slotVertex.size() * kRecordFloats, 0.0f );
	for ( size_t slot = 0; slot < draw.slotVertex.size(); ++slot )
		ConvertVertex(
		    draw, draw.slotVertex[slot], &result.records[slot * kRecordFloats], result.maxBone );
	return result;
}

// Counts the distinct threads that ran a batch's chunks, with relaxed atomics
// only, which order nothing, so the count cannot hide a race from TSan.
std::atomic<int> g_threadsSeen{ 0 };
std::atomic<int> g_generation{ 0 };
thread_local int t_generation = -1;

// The same conversion through CEmitConvertBatch, as convertPooled does.
Result ConvertBatched( const Draw &draw, jobsystem::IWorkerBackend *backend,
    jobsystem::BatchMode mode, uint32_t chunkSize, EmitConvertFault fault = EmitConvertFault::None )
{
	Result result;
	const uint32_t count = static_cast<uint32_t>( draw.slotVertex.size() );
	// Unwritten records keep a pattern no conversion produces (all bits set is
	// a NaN), so a skipped slot is a byte mismatch.
	result.records.resize( size_t( count ) * kRecordFloats );
	std::memset( result.records.data(), 0xFF, result.records.size() * sizeof( float ) );
	float *const records = result.records.data();
	auto range = [&]( uint32_t begin, uint32_t end, int &maxBone )
	{
		const int generation = g_generation.load( std::memory_order_relaxed );
		if ( t_generation != generation )
		{
			t_generation = generation;
			g_threadsSeen.fetch_add( 1, std::memory_order_relaxed );
		}
		for ( uint32_t slot = begin; slot < end; ++slot )
			ConvertVertex(
			    draw, draw.slotVertex[slot], records + size_t( slot ) * kRecordFloats, maxBone );
	};
	typedef CEmitConvertBatch<decltype( range )> Batch;
	std::vector<int> chunkMaxBone( Batch::ChunkCount( count, chunkSize ), -2 );
	Batch batch( range, count, chunkMaxBone.data(), chunkSize, fault );
	jobsystem::BatchDesc desc;
	desc.name = "vulkan.emit_convert";
	desc.context = &batch;
	desc.count = batch.Chunks();
	desc.process = &Batch::Process;
	desc.maxParticipants = backend ? unsigned( backend->WorkerCount() ) + 1 : 1;
	g_generation.fetch_add( 1, std::memory_order_relaxed );
	g_threadsSeen.store( 0, std::memory_order_relaxed );
	result.accepted = jobsystem::ExecuteParallelBatch( desc, backend, mode );
	result.threads = g_threadsSeen.load( std::memory_order_relaxed );
	result.maxBone = batch.ReduceMaxBone();
	return result;
}

bool SameBytes( const Result &a, const Result &b )
{
	return a.records.size() == b.records.size() && a.maxBone == b.maxBone &&
	       ( a.records.empty() || !std::memcmp( a.records.data(), b.records.data(),
	                                  a.records.size() * sizeof( float ) ) );
}

} // namespace

int main()
{
	const uint32_t sizes[] = { 0, 1, 2, 127, 128, 129, 255, 256, 257, 1000, 2048, 4097, 20000 };
	const uint32_t chunkSizes[] = { 0, 1, 7, 128, 1000 };

	// Chunk arithmetic: every slot in exactly one chunk.
	typedef CEmitConvertBatch<void ( * )( uint32_t, uint32_t, int & )> Arithmetic;
	Check( Arithmetic::ChunkCount( 0 ) == 0 && Arithmetic::ChunkCount( 1 ) == 1 &&
	           Arithmetic::ChunkCount( render_vulkan::kEmitConvertChunk ) == 1 &&
	           Arithmetic::ChunkCount( render_vulkan::kEmitConvertChunk + 1 ) == 2 &&
	           Arithmetic::ChunkCount( 10, 3 ) == 4 && Arithmetic::ChunkCount( 9, 3 ) == 3,
	    "chunk counts cover every slot once" );

	// The serial job-system mode and 1, 2 and N real workers match the serial
	// conversion byte for byte, bone maximum included.
	const int hardware = int( std::max( 4u, std::thread::hardware_concurrency() ) );
	const int workerCounts[] = { 1, 2, hardware };
	int mostThreads = 0;
	for ( uint32_t size : sizes )
	{
		const Draw draw = MakeDraw( size, 1000u + size );
		const Result oracle = ConvertSerial( draw );
		for ( uint32_t chunk : chunkSizes )
		{
			const std::string what =
			    std::to_string( size ) + " slots in chunks of " + std::to_string( chunk );
			const Result serial =
			    ConvertBatched( draw, nullptr, jobsystem::BatchMode::Serial, chunk );
			Check( serial.accepted && SameBytes( serial, oracle ),
			    "serial batch equals the serial conversion: " + what );
			for ( int workers : workerCounts )
			{
				ThreadBackend backend( workers );
				const int repeats = workers == hardware ? 3 : 1;
				bool same = true;
				for ( int repeat = 0; repeat < repeats; ++repeat )
				{
					const Result pooled =
					    ConvertBatched( draw, &backend, jobsystem::BatchMode::Parallel, chunk );
					same = same && pooled.accepted && SameBytes( pooled, oracle );
					if ( chunk == 1 && size >= 2048 )
						mostThreads = std::max( mostThreads, pooled.threads );
				}
				Check( same, "parallel batch on " + std::to_string( workers ) +
				                 " worker(s) equals the serial conversion: " + what );
			}
		}
	}
	std::printf( "up to %d threads ran one draw's chunks (%d workers)\n", mostThreads, hardware );
	Check( mostThreads > 1, "a large draw's chunks ran on several threads" );

	// Negative control: a chunk-offset defect (records shifted one slot) is a
	// byte mismatch, whatever the chunk size. Serial mode, so the defect's
	// overlapping writes are not also a data race in the TSan lane.
	for ( uint32_t chunk : chunkSizes )
	{
		const Draw draw = MakeDraw( 2048, 77u );
		const Result shifted = ConvertBatched(
		    draw, nullptr, jobsystem::BatchMode::Serial, chunk, EmitConvertFault::ChunkOffset );
		Check( !SameBytes( shifted, ConvertSerial( draw ) ),
		    "the comparison catches a chunk-offset defect in chunks of " +
		        std::to_string( chunk ) );
	}
	// A dropped bone maximum is caught too: the shared-maximum defect run
	// serially is correct (nothing races), so seed a wrong reduction instead.
	{
		const Draw draw = MakeDraw( 512, 5u );
		Result reduced = ConvertBatched( draw, nullptr, jobsystem::BatchMode::Serial, 0 );
		const Result oracle = ConvertSerial( draw );
		Check( SameBytes( reduced, oracle ) && oracle.maxBone > 0,
		    "the reduced bone maximum equals the serial one" );
		reduced.maxBone = oracle.maxBone - 1;
		Check( !SameBytes( reduced, oracle ), "the comparison catches a wrong bone maximum" );
	}

#ifdef EMIT_CONVERT_SEED_RACE
	// The TSan lane's negative control: chunks raise one shared bone maximum
	// without synchronization. ThreadSanitizer must fail this build.
	{
		const Draw draw = MakeDraw( 20000, 9u );
		ThreadBackend backend( hardware );
		for ( int repeat = 0; repeat < 8; ++repeat )
		{
			const Result racy = ConvertBatched( draw, &backend, jobsystem::BatchMode::Parallel, 1,
			    EmitConvertFault::SharedMaxBone );
			std::printf( "seeded race: shared bone maximum %d\n", racy.maxBone );
		}
	}
#endif

	return testing::ReportConformance( g_checks, g_failures );
}
