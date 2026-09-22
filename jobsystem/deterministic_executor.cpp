//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Single-threaded reference executor (RFC 0003, Phase B).
//
//          Runs a SealedGraph in its stable topological order. Because every
//          producer precedes its consumers in that order, prerequisite states
//          are always terminal by the time a consumer is considered. This is
//          the canonical order used to validate the parallel executor.
//
//=============================================================================//

#include "jobsystem/graph_executor.h"

namespace jobsystem
{

static void Trace( const RunOptions &opts, uint32_t id, const SealedGraph::Job &j, JobState s )
{
	if ( opts.trace ) opts.trace->OnJobState( id, j.name, j.executor, s );
}

RunResult DeterministicExecutor::Execute( const SealedGraph &graph, const RunOptions &opts )
{
	const uint32_t n = graph.JobCount();
	RunResult result;
	result.states.assign( n, JobState::Admitted );

	for ( uint32_t id : graph.TopoOrder() )
	{
		const SealedGraph::Job &job = graph.GetJob( id );

		// Decide eligibility from prerequisites. A Success prereq that did not
		// succeed cancels this job; Terminal prereqs only need to be terminal
		// (guaranteed by topo order). Scope cancellation also cancels the job.
		bool cancel = ( opts.cancel && opts.cancel->load( std::memory_order_acquire ) );
		if ( !cancel )
		{
			for ( const SealedGraph::Prereq &p : job.prereqs )
			{
				if ( p.kind == DependencyKind::Success && result.states[p.producer] != JobState::Succeeded )
				{
					cancel = true;
					break;
				}
			}
		}

		if ( cancel )
		{
			result.states[id] = JobState::Canceled;
			result.canceled++;
			Trace( opts, id, job, JobState::Canceled );
			continue;
		}

		Trace( opts, id, job, JobState::Ready );
		result.states[id] = JobState::Running;
		Trace( opts, id, job, JobState::Running );

		JobRunContext ctx( opts.frame, id );
		if ( job.function )
		{
			job.function( ctx );
			result.executed++;
		}

		if ( ctx.Failed() )
		{
			result.states[id] = JobState::Failed;
			result.failed++;
			Trace( opts, id, job, JobState::Failed );
		}
		else
		{
			result.states[id] = JobState::Succeeded;
			result.succeeded++;
			Trace( opts, id, job, JobState::Succeeded );
		}
	}

	return result;
}

} // namespace jobsystem
