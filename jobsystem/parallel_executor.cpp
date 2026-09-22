//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded worker-pool executor implementation (RFC 0003, Phase C).
//
//=============================================================================//

#include "jobsystem/parallel_executor.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace jobsystem
{

ParallelExecutor::ParallelExecutor( int nWorkers )
	: m_nWorkers( nWorkers < 0 ? 0 : nWorkers )
{
}

namespace
{
	// Shared, mutex-protected run state. The mutex both serializes queue/counter
	// updates and provides the publication edge from a producer's output writes
	// (before it locks to finish) to a consumer's reads (after it locks to dequeue).
	struct RunState
	{
		const SealedGraph *graph = nullptr;
		const RunOptions  *opts  = nullptr;

		std::mutex mtx;
		std::condition_variable cv;

		std::vector<uint32_t> readyQueue;   // ids eligible to be serviced
		std::vector<uint32_t> remaining;    // prerequisites not yet terminal
		std::vector<char>     willCancel;    // decided at activation
		std::vector<JobState> states;

		uint32_t total    = 0;
		uint32_t finished = 0;
		uint32_t succeeded = 0, failed = 0, canceled = 0, executed = 0;

		bool GlobalCancel() const
		{
			return opts->cancel && opts->cancel->load( std::memory_order_acquire );
		}

		// Decide whether a now-eligible job must be canceled: any Success
		// prerequisite whose producer did not succeed cancels it.
		bool DecideCancel( uint32_t id ) const
		{
			if ( GlobalCancel() ) return true;
			const SealedGraph::Job &j = graph->GetJob( id );
			for ( const SealedGraph::Prereq &p : j.prereqs )
			{
				if ( p.kind == DependencyKind::Success && states[p.producer] != JobState::Succeeded )
					return true;
			}
			return false;
		}
	};

	void Trace( RunState &rs, uint32_t id, JobState s )
	{
		if ( rs.opts->trace )
		{
			const SealedGraph::Job &j = rs.graph->GetJob( id );
			rs.opts->trace->OnJobState( id, j.name, j.executor, s );
		}
	}

	// Called with the lock held. Finalizes a job's terminal state and activates
	// any dependents whose last prerequisite just completed.
	void FinishLocked( RunState &rs, std::unique_lock<std::mutex> &lk, uint32_t id, JobState terminal )
	{
		rs.states[id] = terminal;
		switch ( terminal )
		{
			case JobState::Succeeded: rs.succeeded++; break;
			case JobState::Failed:    rs.failed++;    break;
			case JobState::Canceled:  rs.canceled++;  break;
			default: break;
		}
		rs.finished++;

		const SealedGraph::Job &j = rs.graph->GetJob( id );
		for ( uint32_t d : j.dependents )
		{
			if ( --rs.remaining[d] == 0 )
			{
				rs.willCancel[d] = rs.DecideCancel( d ) ? 1 : 0;
				rs.readyQueue.push_back( d );
			}
		}
		(void)lk;
		rs.cv.notify_all();
	}

	// One service pass: pop ready jobs, run or cancel them, propagate. Returns
	// when the whole graph is terminal.
	void WorkerLoop( RunState &rs )
	{
		std::unique_lock<std::mutex> lk( rs.mtx );
		for (;;)
		{
			rs.cv.wait( lk, [&]{ return !rs.readyQueue.empty() || rs.finished == rs.total; } );
			if ( rs.readyQueue.empty() && rs.finished == rs.total )
				return;
			if ( rs.readyQueue.empty() )
				continue;

			const uint32_t id = rs.readyQueue.back();
			rs.readyQueue.pop_back();

			const bool doCancel = rs.willCancel[id] || rs.GlobalCancel();
			if ( doCancel )
			{
				Trace( rs, id, JobState::Canceled );
				FinishLocked( rs, lk, id, JobState::Canceled );
				continue;
			}

			// Run outside the lock. The lock we release here (and re-acquire in
			// FinishLocked) publishes this job's writes to later dequeuers.
			const SealedGraph::Job &job = rs.graph->GetJob( id );
			JobState terminal;
			{
				Trace( rs, id, JobState::Running );
				lk.unlock();
				JobRunContext ctx( rs.opts->frame, id );
				if ( job.function ) job.function( ctx );
				terminal = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
				lk.lock();
				if ( job.function ) rs.executed++;
			}
			Trace( rs, id, terminal );
			FinishLocked( rs, lk, id, terminal );
		}
	}
} // namespace

RunResult ParallelExecutor::Execute( const SealedGraph &graph, const RunOptions &opts )
{
	const uint32_t n = graph.JobCount();

	RunState rs;
	rs.graph = &graph;
	rs.opts  = &opts;
	rs.total = n;
	rs.remaining.resize( n );
	rs.willCancel.assign( n, 0 );
	rs.states.assign( n, JobState::Admitted );

	// Seed roots (no prerequisites) as ready.
	{
		std::lock_guard<std::mutex> lk( rs.mtx );
		for ( uint32_t i = 0; i < n; ++i )
		{
			rs.remaining[i] = (uint32_t)graph.GetJob( i ).prereqs.size();
			if ( rs.remaining[i] == 0 )
			{
				rs.willCancel[i] = rs.DecideCancel( i ) ? 1 : 0;
				rs.readyQueue.push_back( i );
			}
		}
	}

	if ( m_nWorkers == 0 || n == 0 )
	{
		// Ready-driven loop on the calling thread.
		WorkerLoop( rs );
	}
	else
	{
		std::vector<std::thread> workers;
		workers.reserve( m_nWorkers );
		for ( int i = 0; i < m_nWorkers; ++i )
			workers.emplace_back( [&rs]{ WorkerLoop( rs ); } );
		for ( auto &t : workers ) t.join();
	}

	RunResult result;
	result.states    = std::move( rs.states );
	result.succeeded = rs.succeeded;
	result.failed    = rs.failed;
	result.canceled  = rs.canceled;
	result.executed  = rs.executed;
	return result;
}

} // namespace jobsystem
