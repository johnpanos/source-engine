//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded worker-pool executor implementation (RFC 0003, Phase C).
//
//          Lane-aware: Compute/Sequence work runs on compute workers or the
//          pumping main thread; MainThread work runs only on the caller when it
//          pumps; BlockingIO work runs on a dedicated blocking lane (or the main
//          thread when there is none) so it never occupies a compute worker. A
//          lane the graph uses but the configuration cannot service is reported
//          as a stall, with the affected jobs and their dependents left
//          non-terminal, rather than hanging.
//
//=============================================================================//

#include "jobsystem/parallel_executor.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace jobsystem
{

ParallelExecutor::ParallelExecutor( int nComputeWorkers, int nBlockingWorkers )
	: m_nWorkers( nComputeWorkers < 0 ? 0 : nComputeWorkers )
	, m_nBlocking( nBlockingWorkers < 0 ? 0 : nBlockingWorkers )
{
}

namespace
{
	// Which ready queue a job's executor lane feeds.
	enum class Lane : uint8_t { Compute, Main, Blocking };

	Lane LaneOf( ExecutorKind k )
	{
		switch ( k )
		{
			case ExecutorKind::MainThread: return Lane::Main;
			case ExecutorKind::BlockingIO: return Lane::Blocking;
			case ExecutorKind::Compute:
			case ExecutorKind::Sequence:
			default:                        return Lane::Compute;
		}
	}

	// The role a servicer thread plays; decides which lanes it may pull from.
	enum class Role : uint8_t { Compute, Blocking, Main, Inline };

	// Shared, mutex-protected run state. The mutex both serializes queue/counter
	// updates and provides the publication edge from a producer's output writes
	// (before it locks to finish) to a consumer's reads (after it locks to dequeue).
	struct RunState
	{
		const SealedGraph *graph = nullptr;
		const RunOptions  *opts  = nullptr;

		std::mutex mtx;
		std::condition_variable cv;

		std::vector<uint32_t> readyCompute;  // Compute/Sequence, eligible now
		std::vector<uint32_t> readyMain;     // MainThread-affine, eligible now
		std::vector<uint32_t> readyBlocking; // BlockingIO, eligible now
		std::vector<uint32_t> remaining;     // prerequisites not yet resolved
		std::vector<char>     willCancel;    // decided at activation
		std::vector<char>     willStall;     // executor lane has no servicer
		std::vector<char>     stuck;         // resolved as unserviceable
		std::vector<JobState> states;

		bool mainServicesBlocking = false;   // main pumps BlockingIO (no blocking lane)

		uint32_t total    = 0;
		uint32_t resolved = 0; // terminal OR stuck; the loop ends at resolved==total
		uint32_t succeeded = 0, failed = 0, canceled = 0, executed = 0, unresolved = 0;

		bool GlobalCancel() const
		{
			return opts->cancel && opts->cancel->load( std::memory_order_acquire );
		}

		// A now-eligible job must be canceled if any Success prerequisite did not
		// succeed, or if the scope was canceled.
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

		// A job is stuck if its own lane has no servicer, or any prerequisite was
		// left stuck (an unserviceable producer is never observed as terminal).
		bool DecideStuck( uint32_t id ) const
		{
			if ( willStall[id] ) return true;
			const SealedGraph::Job &j = graph->GetJob( id );
			for ( const SealedGraph::Prereq &p : j.prereqs )
				if ( stuck[p.producer] ) return true;
			return false;
		}

		void Enqueue( uint32_t id )
		{
			switch ( LaneOf( graph->GetJob( id ).executor.kind ) )
			{
				case Lane::Main:     readyMain.push_back( id );     break;
				case Lane::Blocking: readyBlocking.push_back( id ); break;
				case Lane::Compute:
				default:             readyCompute.push_back( id );  break;
			}
		}

		// Pop the next id this role may run, or return false. Main helps compute
		// after its own lane; it services blocking only when there is no lane.
		bool Pop( Role role, uint32_t &out )
		{
			auto take = []( std::vector<uint32_t> &q, uint32_t &o ) {
				if ( q.empty() ) return false;
				o = q.back(); q.pop_back(); return true;
			};
			switch ( role )
			{
				case Role::Compute:
					return take( readyCompute, out );
				case Role::Blocking:
					return take( readyBlocking, out );
				case Role::Main:
					if ( take( readyMain, out ) ) return true;
					if ( take( readyCompute, out ) ) return true;
					if ( mainServicesBlocking && take( readyBlocking, out ) ) return true;
					return false;
				case Role::Inline:
				default:
					if ( take( readyMain, out ) ) return true;
					if ( take( readyBlocking, out ) ) return true;
					return take( readyCompute, out );
			}
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

	// Called with the lock held. Resolve a job (terminal or stuck) and activate
	// dependents whose last prerequisite just resolved.
	void ResolveLocked( RunState &rs, uint32_t id, JobState terminal, bool asStuck )
	{
		if ( asStuck )
		{
			rs.stuck[id] = 1;
			rs.unresolved++;
			// state stays non-terminal: no false claim of completion.
		}
		else
		{
			rs.states[id] = terminal;
			switch ( terminal )
			{
				case JobState::Succeeded: rs.succeeded++; break;
				case JobState::Failed:    rs.failed++;    break;
				case JobState::Canceled:  rs.canceled++;  break;
				default: break;
			}
		}
		rs.resolved++;

		const SealedGraph::Job &j = rs.graph->GetJob( id );
		for ( uint32_t d : j.dependents )
		{
			if ( --rs.remaining[d] == 0 )
			{
				if ( rs.DecideStuck( d ) )
				{
					// Defer resolving the stuck dependent through the same path so
					// its own dependents cascade; enqueue a sentinel by resolving now.
					ResolveLocked( rs, d, JobState::Admitted, /*asStuck=*/true );
				}
				else
				{
					rs.willCancel[d] = rs.DecideCancel( d ) ? 1 : 0;
					rs.Enqueue( d );
				}
			}
		}
		rs.cv.notify_all();
	}

	// One servicer pass for a given role. Returns when the whole graph is resolved.
	void ServiceLoop( RunState &rs, Role role )
	{
		std::unique_lock<std::mutex> lk( rs.mtx );
		for (;;)
		{
			uint32_t id = 0;
			rs.cv.wait( lk, [&]{ return rs.resolved == rs.total || rs.Pop( role, id ); } );
			if ( rs.resolved == rs.total )
				return;
			// rs.Pop already dequeued into id under the predicate.

			const bool doCancel = rs.willCancel[id] || rs.GlobalCancel();
			if ( doCancel )
			{
				Trace( rs, id, JobState::Canceled );
				ResolveLocked( rs, id, JobState::Canceled, /*asStuck=*/false );
				continue;
			}

			// Run outside the lock. The lock released here (and re-acquired in
			// ResolveLocked) publishes this job's writes to later dequeuers.
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
			ResolveLocked( rs, id, terminal, /*asStuck=*/false );
		}
	}
} // namespace

RunResult ParallelExecutor::Execute( const SealedGraph &graph, const RunOptions &opts )
{
	const uint32_t n = graph.JobCount();

	const bool inlineMode   = ( m_nWorkers == 0 );
	const bool pumpMain     = opts.pumpMainThread;
	const bool haveBlocking = ( m_nBlocking > 0 );

	RunState rs;
	rs.graph = &graph;
	rs.opts  = &opts;
	rs.total = n;
	rs.remaining.resize( n );
	rs.willCancel.assign( n, 0 );
	rs.willStall.assign( n, 0 );
	rs.stuck.assign( n, 0 );
	rs.states.assign( n, JobState::Admitted );
	rs.mainServicesBlocking = !haveBlocking; // main covers blocking when no lane

	// Precompute which lanes have no servicer under this configuration. In inline
	// mode the single caller services every lane, so nothing stalls.
	for ( uint32_t i = 0; i < n; ++i )
	{
		if ( inlineMode ) break;
		const ExecutorKind k = graph.GetJob( i ).executor.kind;
		const bool mainUnserviced      = ( k == ExecutorKind::MainThread ) && !pumpMain;
		const bool blockingUnserviced  = ( k == ExecutorKind::BlockingIO ) && !haveBlocking && !pumpMain;
		if ( mainUnserviced || blockingUnserviced )
			rs.willStall[i] = 1;
	}

	// Seed roots. Initialize every remaining count first, because resolving a
	// stuck root immediately decrements its dependents' counts; those counts must
	// already be set (RFC 0003 stall accounting must not underflow).
	{
		std::lock_guard<std::mutex> lk( rs.mtx );
		for ( uint32_t i = 0; i < n; ++i )
			rs.remaining[i] = (uint32_t)graph.GetJob( i ).prereqs.size();

		// Resolve unserviceable roots as stuck up front, enqueue the rest by lane.
		for ( uint32_t i = 0; i < n; ++i )
		{
			if ( rs.remaining[i] != 0 || rs.stuck[i] || rs.states[i] != JobState::Admitted )
				continue; // has prereqs, or already resolved via a stuck cascade
			if ( rs.willStall[i] )
			{
				ResolveLocked( rs, i, JobState::Admitted, /*asStuck=*/true );
			}
			else
			{
				rs.willCancel[i] = rs.DecideCancel( i ) ? 1 : 0;
				rs.Enqueue( i );
			}
		}
	}

	if ( inlineMode || n == 0 )
	{
		// Everything on the caller; the inline role services every lane.
		ServiceLoop( rs, Role::Inline );
	}
	else
	{
		std::vector<std::thread> workers;
		workers.reserve( (size_t)m_nWorkers + (size_t)m_nBlocking );
		for ( int i = 0; i < m_nWorkers; ++i )
			workers.emplace_back( [&rs]{ ServiceLoop( rs, Role::Compute ); } );
		for ( int i = 0; i < m_nBlocking; ++i )
			workers.emplace_back( [&rs]{ ServiceLoop( rs, Role::Blocking ); } );

		if ( pumpMain )
		{
			// The caller is the main-thread pump; it also helps compute.
			ServiceLoop( rs, Role::Main );
		}
		else
		{
			// No pump: the caller only waits. Unserviceable lanes were already
			// accounted as stuck, so resolved reaches total and workers exit.
			std::unique_lock<std::mutex> lk( rs.mtx );
			rs.cv.wait( lk, [&]{ return rs.resolved == rs.total; } );
		}

		for ( auto &t : workers ) t.join();
	}

	RunResult result;
	result.states     = std::move( rs.states );
	result.succeeded  = rs.succeeded;
	result.failed     = rs.failed;
	result.canceled   = rs.canceled;
	result.executed   = rs.executed;
	result.unresolved = rs.unresolved;
	result.stalled    = rs.unresolved > 0;
	return result;
}

} // namespace jobsystem
