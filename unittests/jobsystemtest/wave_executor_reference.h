//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The retired wave executor (RFC 0003, PooledExecutor until phase I),
//          kept only in tests as a deliberately bad provider for the J1
//          "continuous execution" suite and as the benchmark's reference.
//
//          It repeatedly gathers every job whose prerequisites are terminal and
//          runs that set through a fork/join over the backend's tasks, then
//          advances readiness: a long job holds back the next wave's ready
//          work, which is exactly what J1 forbids. Its terminal states still
//          agree with DeterministicExecutor, so an equivalence suite alone
//          cannot tell it from a continuous executor; the J1 timing oracles
//          must. Not for product use.
//
//=============================================================================//

#ifndef JOBSYSTEMTEST_WAVE_EXECUTOR_REFERENCE_H
#define JOBSYSTEMTEST_WAVE_EXECUTOR_REFERENCE_H

#include <atomic>
#include <cstdint>
#include <vector>

#include "jobsystem/graph_executor.h"
#include "jobsystem/worker_backend.h"

namespace jobsystemtest
{

class WaveExecutorReference final : public jobsystem::IGraphExecutor
{
public:
	explicit WaveExecutorReference( jobsystem::IWorkerBackend *backend ) : m_backend( backend ) {}

	jobsystem::RunResult Execute(
	    const jobsystem::SealedGraph &graph, const jobsystem::RunOptions &opts ) override
	{
		using namespace jobsystem;
		const uint32_t n = graph.JobCount();
		const bool inlineMode = !m_backend || m_backend->WorkerCount() <= 0;
		RunResult result;
		result.states.assign( n, JobState::Admitted );
		std::vector<uint32_t> remaining( n ), ready, next, compute, caller;
		for ( uint32_t i = 0; i < n; ++i )
		{
			remaining[i] = (uint32_t)graph.GetJob( i ).prereqs.size();
			if ( remaining[i] == 0 )
				ready.push_back( i );
		}
		auto cancelled = [&]
		{
			return opts.cancel && opts.cancel->load( std::memory_order_acquire );
		};
		auto runJob = [&]( uint32_t id )
		{
			if ( cancelled() )
			{
				result.states[id] = JobState::Canceled;
				return;
			}
			JobRunContext ctx( opts.frame, id );
			if ( graph.GetJob( id ).function )
				graph.GetJob( id ).function( ctx );
			if ( ctx.Failed() )
				result.states[id] = JobState::Failed;
		};
		while ( !ready.empty() )
		{
			compute.clear();
			caller.clear();
			for ( uint32_t id : ready )
			{
				bool stall = false;
				const ExecutorKind kind = graph.GetJob( id ).executor.kind;
				if ( !inlineMode && !opts.pumpMainThread &&
				     ( kind == ExecutorKind::MainThread || kind == ExecutorKind::BlockingIO ) )
					stall = true;
				for ( const SealedGraph::Prereq &p : graph.GetJob( id ).prereqs )
					stall = stall || !IsTerminal( result.states[p.producer] );
				if ( stall )
					continue;
				bool cancel = cancelled();
				for ( const SealedGraph::Prereq &p : graph.GetJob( id ).prereqs )
					cancel = cancel || ( p.kind == DependencyKind::Success &&
					                       result.states[p.producer] != JobState::Succeeded );
				if ( cancel )
				{
					result.states[id] = JobState::Canceled;
					continue;
				}
				result.states[id] = JobState::Succeeded;
				if ( !inlineMode && graph.GetJob( id ).function &&
				     ( kind == ExecutorKind::Compute || kind == ExecutorKind::Sequence ) )
					compute.push_back( id );
				else
					caller.push_back( id );
			}
			ForkJoin( compute, caller, runJob );
			next.clear();
			for ( uint32_t id : ready )
			{
				const JobState state = result.states[id];
				if ( state == JobState::Succeeded )
					result.succeeded++;
				else if ( state == JobState::Failed )
					result.failed++;
				else if ( state == JobState::Canceled )
					result.canceled++;
				else
					result.unresolved++;
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

private:
	// The wave barrier, as the retired PooledExecutor drove the old pool
	// bridge's ParallelFor/ParallelForWithCaller: runner tasks and the caller
	// claim compute jobs from a cursor after the caller ran its own jobs;
	// withdrawn runners leave their share to the caller.
	template <typename RunJob>
	void ForkJoin(
	    const std::vector<uint32_t> &compute, const std::vector<uint32_t> &caller, RunJob &runJob )
	{
		struct Fork
		{
			const std::vector<uint32_t> *ids;
			RunJob *run;
			std::atomic<size_t> next{ 0 };
			void Drain()
			{
				for ( size_t i = next++; i < ids->size(); i = next++ )
					( *run )( ( *ids )[i] );
			}
			static void Task( void *self ) { static_cast<Fork *>( self )->Drain(); }
		} fork;
		fork.ids = &compute;
		fork.run = &runJob;
		std::vector<void *> tickets;
		// The old bridge ran a lone compute job inline, and queued one runner
		// fewer than the jobs when the caller had none of its own.
		if ( caller.empty() && compute.size() == 1 )
		{
			runJob( compute[0] );
			return;
		}
		if ( m_backend && !compute.empty() )
		{
			const size_t wanted = caller.empty() ? compute.size() - 1 : compute.size();
			const size_t workers = (size_t)m_backend->WorkerCount();
			const size_t runners = wanted < workers ? wanted : workers;
			for ( size_t i = 0; i < runners; ++i )
				if ( void *ticket = m_backend->PostTask( &Fork::Task, &fork ) )
					tickets.push_back( ticket );
		}
		for ( uint32_t id : caller )
			runJob( id );
		fork.Drain();
		for ( void *ticket : tickets )
			m_backend->SettleTask( ticket );
	}

	jobsystem::IWorkerBackend *m_backend;
};

} // namespace jobsystemtest

#endif // JOBSYSTEMTEST_WAVE_EXECUTOR_REFERENCE_H
