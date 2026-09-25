//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded independent compute batches on dependency-aware job graphs.
//
//=============================================================================//

#include "jobsystem/parallel_batch.h"
#include "jobsystem/pooled_executor.h"
#include "batch_depth.h"

#include <algorithm>
#include <atomic>
#include <climits>
#include <memory>
#include <string>
#include <vector>

namespace jobsystem
{
namespace detail
{
unsigned &BatchDepth()
{
	// A callback cannot start a blocking fork/join on a pool whose workers it
	// may occupy.
	thread_local unsigned t_depth = 0;
	return t_depth;
}
} // namespace detail

namespace
{
class ParticipantScope
{
public:
	explicit ParticipantScope( const BatchDesc &desc ) : m_desc( desc )
	{
		if ( m_desc.begin )
			m_desc.begin( m_desc.context );
	}

	~ParticipantScope()
	{
		if ( m_desc.end )
			m_desc.end( m_desc.context );
	}

private:
	detail::BatchDepthScope m_depth; // entered before begin, left after end
	const BatchDesc &m_desc;
};

// State shared by every participant of one call. Participant jobs capture
// nothing; they find this through the run's frame context.
struct BatchRun
{
	const BatchDesc &desc;
	unsigned participants;
	// Only index reservation uses this atomic. Input publication and
	// completion visibility come from the backend's fork/join contract, not
	// the cursor. The cursor never passes count, so it cannot wrap.
	std::atomic<uint64_t> next{ 0 };

	// Guided self-scheduling: claim a contiguous range sized to a fraction of
	// the remaining items per participant. Early claims are large (few
	// contended cursor updates, no neighboring-item sharing between runners)
	// and shrink to single items at the end for load balance.
	bool Claim( unsigned &begin, unsigned &end )
	{
		uint64_t cursor = next.load( std::memory_order_relaxed );
		for ( ;; )
		{
			if ( cursor >= desc.count )
				return false;
			const uint64_t remaining = desc.count - cursor;
			const uint64_t chunk = std::max<uint64_t>( 1, remaining / ( 4ull * participants ) );
			if ( next.compare_exchange_weak( cursor, cursor + chunk, std::memory_order_relaxed ) )
			{
				begin = static_cast<unsigned>( cursor );
				end = static_cast<unsigned>( cursor + chunk );
				return true;
			}
		}
	}

	void Participate()
	{
		// The descriptor is immutable for the call; keep its fields local.
		void ( *const process )( void *, unsigned ) = desc.process;
		void *const context = desc.context;
		if ( participants == 1 )
		{
			ParticipantScope scope( desc );
			const unsigned count = desc.count;
			for ( unsigned index = 0; index < count; ++index )
				process( context, index );
			return;
		}
		unsigned begin, end;
		if ( !Claim( begin, end ) )
			return;
		ParticipantScope scope( desc );
		do
		{
			for ( unsigned index = begin; index < end; ++index )
				process( context, index );
		} while ( Claim( begin, end ) );
	}
};

void BatchParticipant( JobRunContext &ctx )
{
	static_cast<BatchRun *>( ctx.Frame().user )->Participate();
}

bool SealBatchGraph( const char *name, unsigned participants, SealedGraph &out )
{
	JobGraphBuilder builder;
	JobDesc complete;
	complete.name = "batch.complete";
	const JobHandle join = builder.AddJob( complete );
	JobDesc compute;
	compute.name = name;
	compute.function = &BatchParticipant;
	for ( unsigned i = 0; i < participants; ++i )
		builder.AddDependency( builder.AddJob( compute ), join );
	auto graph = builder.Seal();
	if ( !graph.HasValue() )
		return false;
	out = std::move( graph.Value() );
	return true;
}

// A batch graph's shape depends only on its job name and participant count, so
// each thread keeps the sealed graphs it has used. An entry owns a copy of the
// name (the graph never points at caller memory) and is never evicted: a
// nested batch may use or add entries while an outer batch on the same thread
// still executes one. Past the cap, graphs are sealed per call instead.
struct CachedGraph
{
	std::string name;
	unsigned participants = 0;
	SealedGraph graph;
};

const SealedGraph *BatchGraph( const char *name, unsigned participants, SealedGraph &uncached )
{
	constexpr size_t kMaxCachedGraphs = 32;
	thread_local std::vector<std::unique_ptr<CachedGraph>> t_graphs;
	for ( const auto &entry : t_graphs )
	{
		if ( entry->participants == participants && entry->name == name )
			return &entry->graph;
	}
	if ( t_graphs.size() >= kMaxCachedGraphs )
		return SealBatchGraph( name, participants, uncached ) ? &uncached : nullptr;
	auto entry = std::make_unique<CachedGraph>();
	entry->name = name;
	entry->participants = participants;
	if ( !SealBatchGraph( entry->name.c_str(), participants, entry->graph ) )
		return nullptr;
	t_graphs.push_back( std::move( entry ) );
	return &t_graphs.back()->graph;
}
} // namespace

bool ExecuteParallelBatch( const BatchDesc &desc, IWorkerBackend *backend, BatchMode mode )
{
	if ( !desc.name || !desc.name[0] ||
	     ( mode != BatchMode::Serial && mode != BatchMode::Parallel ) )
		return false;
	if ( desc.count == 0 )
		return true;
	if ( !desc.process || desc.maxParticipants == 0 )
		return false;

	const bool serial = mode == BatchMode::Serial || detail::BatchDepth() != 0;
	unsigned participants = 1;
	if ( !serial && backend && backend->WorkerCount() > 0 )
	{
		const unsigned capacity = static_cast<unsigned>( backend->WorkerCount() ) + 1u;
		participants = std::min(
		    { desc.count, desc.maxParticipants, capacity, static_cast<unsigned>( INT_MAX ) } );
	}

	SealedGraph uncached;
	const SealedGraph *graph = BatchGraph( desc.name, participants, uncached );
	if ( !graph )
		return false;

	// A constructed batch contains only infallible Compute nodes and a join.
	// There is no cancellation or affinity-dependent progress requirement.
	BatchRun run{ desc, participants };
	RunOptions options;
	options.frame.user = &run;
	if ( serial )
		DeterministicExecutor().Execute( *graph, options );
	else
		PooledExecutor( backend ).Execute( *graph, options );
	return true;
}

} // namespace jobsystem
