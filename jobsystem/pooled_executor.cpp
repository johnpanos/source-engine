//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Wave-based graph executor over an IWorkerBackend (RFC 0003).
//
//=============================================================================//

#include "jobsystem/pooled_executor.h"

#include <cstddef>
#include <vector>

namespace jobsystem
{

RunResult PooledExecutor::Execute( const SealedGraph &graph, const RunOptions &opts )
{
	const uint32_t n = graph.JobCount();
	RunResult result;
	result.states.assign( n, JobState::Admitted );
	if ( n == 0 ) return result;

	auto trace = [&]( uint32_t id, JobState s ) {
		if ( opts.trace ) opts.trace->OnJobState( id, graph.GetJob( id ).name, graph.GetJob( id ).executor, s );
	};
	auto globalCancel = [&] { return opts.cancel && opts.cancel->load( std::memory_order_acquire ); };

	// A Success prereq whose producer did not succeed cancels the consumer.
	auto decideCancel = [&]( uint32_t id ) {
		if ( globalCancel() ) return true;
		for ( const SealedGraph::Prereq &p : graph.GetJob( id ).prereqs )
			if ( p.kind == DependencyKind::Success && result.states[p.producer] != JobState::Succeeded )
				return true;
		return false;
	};

	std::vector<uint32_t> remaining( n );
	std::vector<uint32_t> ready;
	for ( uint32_t i = 0; i < n; ++i )
	{
		remaining[i] = (uint32_t)graph.GetJob( i ).prereqs.size();
		if ( remaining[i] == 0 ) ready.push_back( i );
	}

	// Per-run failure flags, written by exactly one worker per job (its own slot).
	std::vector<char> failed( n, 0 );

	while ( !ready.empty() )
	{
		// Jobs ready in the same wave are mutually independent (none is an
		// unfinished prerequisite of another), so running them concurrently is
		// safe. Decide cancel first, then run only the surviving ones.
		std::vector<uint32_t> toRun; // ready, not canceled, with a function
		std::vector<char> cancel( ready.size(), 0 );
		for ( std::size_t k = 0; k < ready.size(); ++k )
		{
			const uint32_t id = ready[k];
			if ( decideCancel( id ) ) { cancel[k] = 1; continue; }
			if ( graph.GetJob( id ).function ) toRun.push_back( id );
		}

		if ( !toRun.empty() )
		{
			const std::vector<uint32_t> &run = toRun;
			auto body = [&]( int k ) {
				const uint32_t id = run[(std::size_t)k];
				trace( id, JobState::Running );
				JobRunContext ctx( opts.frame, id );
				graph.GetJob( id ).function( ctx );
				failed[id] = ctx.Failed() ? 1 : 0;
			};
			if ( m_backend && m_backend->WorkerCount() > 0 )
				m_backend->ParallelFor( (int)run.size(), body );
			else
				for ( std::size_t k = 0; k < run.size(); ++k ) body( (int)k );
			result.executed += (uint32_t)run.size();
		}

		// Finalize this wave and advance readiness (single-threaded; the barrier
		// above published all worker writes).
		std::vector<uint32_t> next;
		for ( std::size_t k = 0; k < ready.size(); ++k )
		{
			const uint32_t id = ready[k];
			JobState terminal;
			if ( cancel[k] )          { terminal = JobState::Canceled;  result.canceled++;  }
			else if ( failed[id] )    { terminal = JobState::Failed;    result.failed++;    trace( id, JobState::Failed ); }
			else                      { terminal = JobState::Succeeded; result.succeeded++; trace( id, JobState::Succeeded ); }
			if ( cancel[k] ) trace( id, JobState::Canceled );
			result.states[id] = terminal;

			for ( uint32_t d : graph.GetJob( id ).dependents )
				if ( --remaining[d] == 0 ) next.push_back( d );
		}
		ready.swap( next );
	}

	return result;
}

} // namespace jobsystem
