//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Wave-based graph executor over an IWorkerBackend (RFC 0003).
//
//=============================================================================//

#include "jobsystem/pooled_executor.h"

#include <cstddef>
#include <functional>
#include <vector>

namespace jobsystem
{

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

	std::vector<uint32_t> remaining( n );
	std::vector<uint32_t> ready, compute, caller;
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

	std::vector<uint32_t> next;
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

		if ( !compute.empty() )
			m_backend->ParallelFor( (int)compute.size(), runCompute );
		for ( uint32_t id : caller )
			runJob( id );

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
		ready.swap( next );
	}

	result.stalled = result.unresolved > 0;
	return result;
}

} // namespace jobsystem
