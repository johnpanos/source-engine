//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: jobsystem.radiosity (RFC 0011 G4, Q-JOBS): the precomputed
//          radiosity producer's gather, probe gather and composition run on
//          the job system's batch executor with 1, 2 and N real worker
//          threads and must produce the serial oracle's bytes exactly.
//
//          The workload is a synthetic stress transfer (tens of thousands of
//          patches, random sparse links) paired with the contract volume, so
//          every batch has many items and every worker runs at once. Build
//          the same source with -fsanitize=thread for the TSan lane
//          (jobsystem.radiosity.tsan).
//
//=============================================================================//

#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"
#include "render/indirect_radiosity.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace
{
using namespace indirect_light;

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

// The producer's executor over the job system's batch boundary.
class BatchExecutor final : public IBatchExecutor
{
public:
	BatchExecutor( jobsystem::IWorkerBackend *backend, jobsystem::BatchMode mode )
	    : m_backend( backend ), m_mode( mode )
	{
	}
	void ParallelFor( const char *name, uint32_t count, void ( *body )( void *, uint32_t ),
	    void *context ) override
	{
		jobsystem::BatchDesc desc;
		desc.name = name;
		desc.context = context;
		desc.count = count;
		desc.process = body;
		desc.maxParticipants = m_backend ? unsigned( m_backend->WorkerCount() ) + 1 : 1;
		if ( !jobsystem::ExecuteParallelBatch( desc, m_backend, m_mode ) )
			failed = true;
	}
	bool failed = false;

private:
	jobsystem::IWorkerBackend *m_backend;
	jobsystem::BatchMode m_mode;
};

std::vector<unsigned char> Load( const char *path )
{
	std::ifstream file( path, std::ios::binary );
	return std::vector<unsigned char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

template <typename T> void Put( std::vector<unsigned char> &bytes, uint64_t offset, const T &value )
{
	std::memcpy( bytes.data() + offset, &value, sizeof( T ) );
}

// A random RTRN in the documented encoding (radiosity_transfer.py): rows
// sum below one, gathers are normalized, every index is in range.
std::vector<unsigned char> StressTransfer( const Volume &volume, uint32_t patches )
{
	std::mt19937 random( 20260925u );
	std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
	const uint32_t sources = 4, links = 48, gatherLinks = 400;
	uint32_t probes = 0;
	for ( uint32_t g = 0; g < volume.layout.gridCount; ++g )
		probes += volume.layout.grids[g].probeCount;
	const uint64_t sizes[9] = { sources * 64ull, uint64_t( patches ) * 48, ( patches + 1ull ) * 4,
	    uint64_t( patches ) * links * 8, ( sources + 1ull ) * 4, uint64_t( patches ) * 16,
	    ( probes + 1ull ) * 4, uint64_t( probes ) * gatherLinks * 40,
	    uint64_t( sources ) * probes * 36 * 12 };
	uint64_t offsets[9], cursor = 64;
	for ( int i = 0; i < 9; ++i )
	{
		cursor = ( cursor + 15 ) & ~15ull;
		offsets[i] = cursor;
		cursor += sizes[i];
	}
	std::vector<unsigned char> bytes( cursor, 0 );
	const uint32_t header[12] = { mapcontainer::kLumpRadiosityTransfer, 1, 64, 0, sources, patches,
	    probes, 36, patches * links, patches, probes * gatherLinks, 9 };
	std::memcpy( bytes.data(), header, sizeof( header ) );
	Put( bytes, 48, mapcontainer::ProbeVolumeTopologyHash( volume.bytes.data(), volume.layout ) );
	for ( uint32_t s = 0; s < sources; ++s )
	{
		mapcontainer::RadiositySource source = {};
		source.kind = s % 3;
		source.style = int32_t( 32 + s );
		std::snprintf( source.name, sizeof( source.name ), "Source%u", s );
		Put( bytes, offsets[0] + s * 64ull, source );
	}
	const auto pick = [&]( uint32_t count, uint32_t limit )
	{
		std::vector<uint32_t> chosen;
		while ( chosen.size() < count )
		{
			chosen.push_back( uint32_t( random() % limit ) );
			std::sort( chosen.begin(), chosen.end() );
			chosen.erase( std::unique( chosen.begin(), chosen.end() ), chosen.end() );
		}
		return chosen;
	};
	for ( uint32_t p = 0; p < patches; ++p )
	{
		mapcontainer::RadiosityPatch patch = {};
		patch.position[0] = unit( random ) * 512.0f;
		patch.normal[2] = 1.0f;
		patch.area = 16.0f;
		for ( float &albedo : patch.albedo )
			albedo = 0.3f + 0.6f * unit( random );
		Put( bytes, offsets[1] + p * 48ull, patch );
		Put( bytes, offsets[2] + p * 4ull, p * links );
		const std::vector<uint32_t> targets = pick( links, patches );
		for ( uint32_t k = 0; k < links; ++k )
			Put( bytes, offsets[3] + ( uint64_t( p ) * links + k ) * 8,
			    mapcontainer::RadiosityTransferLink{ targets[k], 0.9f / float( links ) * unit( random ) } );
		// Each patch lit by one source.
		mapcontainer::RadiosityInjectionLink injection = {};
		injection.patch = p;
		for ( float &light : injection.light )
			light = unit( random );
		Put( bytes, offsets[5] + uint64_t( p ) * 16, injection );
	}
	Put( bytes, offsets[2] + patches * 4ull, patches * links );
	// Injection rows: source s owns patches [s * n / 4, (s + 1) * n / 4).
	for ( uint32_t s = 0; s <= sources; ++s )
		Put( bytes, offsets[4] + s * 4ull, uint32_t( uint64_t( patches ) * s / sources ) );
	for ( uint32_t i = 0; i <= probes; ++i )
		Put( bytes, offsets[6] + i * 4ull, i * gatherLinks );
	for ( uint32_t i = 0; i < probes; ++i )
	{
		const std::vector<uint32_t> targets = pick( gatherLinks, patches );
		for ( uint32_t k = 0; k < gatherLinks; ++k )
		{
			mapcontainer::RadiosityGatherLink link = {};
			link.patch = targets[k];
			link.sh[0] = 3.0f / float( gatherLinks ) * unit( random );
			for ( int c = 1; c < 9; ++c )
				link.sh[c] = ( unit( random ) - 0.5f ) * 0.01f;
			Put( bytes, offsets[7] + ( uint64_t( i ) * gatherLinks + k ) * 40, link );
		}
	}
	for ( uint64_t k = 0; k < sizes[8] / 4; ++k )
		Put( bytes, offsets[8] + k * 4, 0.05f * unit( random ) );
	return bytes;
}

// A producer's run: a scalar change, then updates and compositions. Returns
// every composed volume's bytes and the final patch changes.
std::vector<unsigned char> Run( const std::shared_ptr<const Transfer> &transfer,
    const Volume &base, IBatchExecutor *executor, int frames )
{
	RadiositySolver solver;
	solver.Init( transfer, std::vector<float>( transfer->layout.sourceCount, 1.0f ),
	    RadiosityOptions() );
	std::vector<unsigned char> trace;
	for ( int frame = 0; frame < frames; ++frame )
	{
		std::vector<float> scalars( transfer->layout.sourceCount, 1.0f );
		scalars[frame / 4 % scalars.size()] = frame % 2 ? 0.0f : 0.25f;
		solver.SetScalars( scalars );
		if ( !solver.Update( executor ) )
			continue;
		const auto volume = solver.Compose( base, executor );
		trace.insert( trace.end(), volume->bytes.begin() + volume->layout.atlasOffset,
		    volume->bytes.end() );
	}
	for ( uint32_t p = 0; p < transfer->layout.patchCount; ++p )
	{
		const unsigned char *change =
		    reinterpret_cast<const unsigned char *>( solver.PatchChange( p ) );
		trace.insert( trace.end(), change, change + 12 );
	}
	return trace;
}

} // namespace

int main()
{
	const auto base = Volume::FromBytes( Load( "quality/fixtures/gi/prbv/contract.prbv" ) );
	Check( base != nullptr, "the contract volume loads" );
	if ( !base )
		return testing::ReportConformance( g_checks, g_failures );
	const char *stress = std::getenv( "RADIOSITY_STRESS_PATCHES" );
	const uint32_t patches = stress ? uint32_t( std::strtoul( stress, nullptr, 10 ) ) : 40000;
	const auto transfer = Transfer::FromBytes( StressTransfer( *base, patches ), *base );
	Check( transfer != nullptr, "the synthetic stress transfer validates" );
	for ( const char *fixture : { "quality/fixtures/gi/rtrn/contract.rtrn" } )
	{
		const auto contract = Transfer::FromBytes( Load( fixture ), *base );
		Check( contract != nullptr, "the contract transfer loads" );
	}
	if ( !transfer )
		return testing::ReportConformance( g_checks, g_failures );

	const int frames = 24;
	const std::vector<unsigned char> oracle = Run( transfer, *base, nullptr, frames );
	BatchExecutor serial( nullptr, jobsystem::BatchMode::Serial );
	Check( Run( transfer, *base, &serial, frames ) == oracle && !serial.failed,
	    "the job system's serial mode matches the inline oracle byte for byte" );
	const int hardware = int( std::max( 4u, std::thread::hardware_concurrency() ) );
	for ( int workers : { 1, 2, hardware } )
	{
		ThreadBackend backend( workers );
		BatchExecutor pooled( &backend, jobsystem::BatchMode::Parallel );
		bool identical = true;
		const int repeats = workers == hardware ? 4 : 2;
		for ( int repeat = 0; repeat < repeats; ++repeat )
			identical = identical && Run( transfer, *base, &pooled, frames ) == oracle;
		Check( identical && !pooled.failed, "pooled execution with " + std::to_string( workers ) +
		                                        " worker(s) matches the serial oracle byte for byte" );
	}
	std::printf( "jobsystem.radiosity: %u patches, %u transfer links, %zu trace bytes, %d workers "
	             "max\n",
	    transfer->layout.patchCount, transfer->layout.transferLinks, oracle.size(), hardware );
	return testing::ReportConformance( g_checks, g_failures );
}
