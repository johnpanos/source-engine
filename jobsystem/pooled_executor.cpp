//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Wave-based graph executor over an IWorkerBackend (RFC 0003).
//
//=============================================================================//

#include "jobsystem/pooled_executor.h"

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace jobsystem
{

namespace
{
// A bounded id list over caller-provided storage.
struct IdList
{
	uint32_t *data;
	uint32_t count = 0;

	void push_back( uint32_t id ) { data[count++] = id; }
	void clear() { count = 0; }
	bool empty() const { return count == 0; }
	uint32_t size() const { return count; }
	uint32_t operator[]( std::size_t i ) const { return data[i]; }
	const uint32_t *begin() const { return data; }
	const uint32_t *end() const { return data + count; }
};
} // namespace

RunResult PooledExecutor::Execute( const SealedGraph &graph, const RunOptions &opts )
{
	const uint32_t n = graph.JobCount();
	const bool inlineMode = !m_backend || m_backend->WorkerCount() <= 0;
	RunResult result;
	result.states.assign( n, JobState::Admitted );
	if ( n == 0 )
		return result;

	auto trace = [&]( uint32_t id, JobState s )
	{
		if ( opts.trace )
			opts.trace->OnJobState( id, graph.GetJob( id ).name, graph.GetJob( id ).executor, s );
	};
	auto globalCancel = [&]
	{
		return opts.cancel && opts.cancel->load( std::memory_order_acquire );
	};

	// The backend services compute work only. Without an explicit caller pump,
	// affine/blocking work has no servicer. No dependent may observe a stalled
	// prerequisite as terminal, including a Terminal (cleanup) dependency.
	auto decideStall = [&]( uint32_t id )
	{
		const ExecutorKind kind = graph.GetJob( id ).executor.kind;
		if ( !inlineMode && !opts.pumpMainThread &&
		     ( kind == ExecutorKind::MainThread || kind == ExecutorKind::BlockingIO ) )
			return true;
		for ( const SealedGraph::Prereq &p : graph.GetJob( id ).prereqs )
			if ( !IsTerminal( result.states[p.producer] ) )
				return true;
		return false;
	};

	// A Success prereq whose producer did not succeed cancels the consumer.
	auto decideCancel = [&]( uint32_t id )
	{
		if ( globalCancel() )
			return true;
		for ( const SealedGraph::Prereq &p : graph.GetJob( id ).prereqs )
			if ( p.kind == DependencyKind::Success &&
			     result.states[p.producer] != JobState::Succeeded )
				return true;
		return false;
	};

	// Remaining counts and the four per-wave id lists share one allocation;
	// each list holds at most n ids.
	std::vector<uint32_t> arena( 5 * (std::size_t)n );
	uint32_t *const remaining = arena.data();
	IdList ready{ remaining + n }, next{ remaining + 2 * (std::size_t)n },
	    compute{ remaining + 3 * (std::size_t)n }, caller{ remaining + 4 * (std::size_t)n };
	for ( uint32_t i = 0; i < n; ++i )
	{
		remaining[i] = (uint32_t)graph.GetJob( i ).prereqs.size();
		if ( remaining[i] == 0 )
			ready.push_back( i );
	}

	// Each invocation owns its state slot. The slot of an admitted job already
	// holds Succeeded, so the common outcome writes nothing and workers do not
	// contend on neighboring slots; only a failed or canceled job overwrites
	// it. The slot is not read before this wave is finalized. The backend's
	// join barrier publishes those writes and the job outputs before
	// finalization or the next wave.
	auto runJob = [&]( uint32_t id )
	{
		// Cancellation may arrive after a wave was admitted but before this
		// invocation starts (including while another caller job is running).
		if ( globalCancel() )
		{
			result.states[id] = JobState::Canceled;
			return;
		}
		trace( id, JobState::Running );
		JobRunContext ctx( opts.frame, id );
		if ( graph.GetJob( id ).function )
			graph.GetJob( id ).function( ctx );
		if ( ctx.Failed() )
			result.states[id] = JobState::Failed;
	};
	const std::function<void( int )> runCompute = [&]( int k )
	{
		runJob( compute[(std::size_t)k] );
	};

	while ( !ready.empty() )
	{
		// Jobs ready in the same wave are mutually independent (none is an
		// unfinished prerequisite of another), so running them concurrently is
		// safe. Only Compute/Sequence jobs may enter the backend. In inline
		// mode retain ready order while the caller services every lane.
		compute.clear();
		caller.clear();
		for ( uint32_t id : ready )
		{
			if ( decideStall( id ) )
				continue;
			if ( decideCancel( id ) )
			{
				result.states[id] = JobState::Canceled;
				continue;
			}
			trace( id, JobState::Ready );
			result.states[id] = JobState::Succeeded; // until the job reports otherwise
			const ExecutorKind kind = graph.GetJob( id ).executor.kind;
			// Empty compute nodes need no worker dispatch; their ordering edges
			// still advance only after this wave's completion barrier.
			if ( !inlineMode && graph.GetJob( id ).function &&
			     ( kind == ExecutorKind::Compute || kind == ExecutorKind::Sequence ) )
				compute.push_back( id );
			else
				caller.push_back( id );
		}

		// The wave's caller jobs run on this thread while workers run its
		// compute jobs (they are mutually independent); the backend joins both.
		if ( !compute.empty() && !caller.empty() )
		{
			m_backend->ParallelForWithCaller( (int)compute.size(), runCompute,
			    [&]
			    {
				    for ( uint32_t id : caller )
					    runJob( id );
			    } );
		}
		else if ( !compute.empty() )
		{
			m_backend->ParallelFor( (int)compute.size(), runCompute );
		}
		else
		{
			for ( uint32_t id : caller )
				runJob( id );
		}

		// Finalize this wave and advance readiness (single-threaded; the barrier
		// above published all worker writes).
		next.clear();
		for ( uint32_t id : ready )
		{
			const JobState state = result.states[id];
			switch ( state )
			{
			case JobState::Succeeded:
				result.succeeded++;
				break;
			case JobState::Failed:
				result.failed++;
				break;
			case JobState::Canceled:
				result.canceled++;
				break;
			default:
				result.unresolved++;
				break;
			}
			if ( IsTerminal( state ) )
				trace( id, state );
			if ( ( state == JobState::Succeeded || state == JobState::Failed ) &&
			     graph.GetJob( id ).function )
				result.executed++;

			for ( uint32_t d : graph.GetJob( id ).dependents )
				if ( --remaining[d] == 0 )
					next.push_back( d );
		}
		std::swap( ready, next );
	}

	result.stalled = result.unresolved > 0;
	return result;
}

} // namespace jobsystem
