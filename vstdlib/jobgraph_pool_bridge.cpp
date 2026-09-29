//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy thread-pool bridge implementation (RFC 0003, Phase C / R20).
//
//          Adapts the real engine CThreadPool to the scheduler's IWorkerBackend.
//          Each call owns its runners and joins only its own queued work.
//
//=============================================================================//

#include <atomic>
#include <vector>

#include "vstdlib/jobgraph_pool_bridge.h"
#include "vstdlib/jobgraph_frame.h"
#include "vstdlib/jobgraph_parallel.h"
#include "jobsystem/worker_backend.h"
#include "vstdlib/jobthread.h"

#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// IWorkerBackend backed by a borrowed or privately owned CThreadPool.
//-----------------------------------------------------------------------------
class CThreadPoolWorkerBackend : public jobsystem::IWorkerBackend
{
public:
	// nWorkers < 0 follows the pool's thread count at each call (a borrowed pool
	// the engine starts and stops). With bInlineOnOwnWorkers, a call made on one
	// of the pool's workers runs inline and is counted instead of queuing
	// runners behind its caller.
	CThreadPoolWorkerBackend(
	    IThreadPool *pPool, int nWorkers, bool bOwnsPool = false, bool bInlineOnOwnWorkers = false )
	    : m_pPool( pPool ), m_bOwnsPool( bOwnsPool ), m_nWorkers( nWorkers ),
	      m_bInlineOnOwnWorkers( bInlineOnOwnWorkers ), m_nNestedCalls( 0 )
	{
	}
	virtual ~CThreadPoolWorkerBackend() {}

	virtual void ParallelFor( int n, const std::function<void( int )> &body )
	{
		if ( n <= 0 )
			return;

		// No workers, or a single item: run inline. Avoids pool overhead and the
		// documented risk of helping unrelated queued work while waiting.
		const int nWorkers = WorkerCount();
		if ( !m_pPool || nWorkers <= 0 || n == 1 || NestedOnOwnWorker() )
		{
			for ( int i = 0; i < n; ++i )
				body( i );
			return;
		}

		// This call owns every runner and callback borrow. Queue at most one
		// runner per available worker, including a pool with just one worker.
		// The caller claims only this call's indices and never helps other jobs.
		const int nRunners = n - 1 < nWorkers ? n - 1 : nWorkers;
		std::vector<CJob *> jobs;
		jobs.reserve( (size_t)nRunners );
		ParallelRun run( (unsigned)n, body );
		for ( int i = 0; i < nRunners; ++i )
			jobs.push_back( m_pPool->QueueCall( &run, &ParallelRun::Run ) );

		run.Run();
		JoinRunners( jobs );
	}

	// Workers start on body() before the caller runs caller(); the caller then
	// claims whatever body() indices remain and joins. Only this call's runners
	// are queued and joined.
	virtual void ParallelForWithCaller(
	    int n, const std::function<void( int )> &body, const std::function<void()> &caller )
	{
		const int nWorkers = WorkerCount();
		if ( n <= 0 || !m_pPool || nWorkers <= 0 || NestedOnOwnWorker() )
		{
			caller();
			for ( int i = 0; i < n; ++i )
				body( i );
			return;
		}

		// The caller is busy with caller() first, so queue a runner per index
		// up to the worker count.
		const int nRunners = n < nWorkers ? n : nWorkers;
		std::vector<CJob *> jobs;
		jobs.reserve( (size_t)nRunners );
		ParallelRun run( (unsigned)n, body );
		for ( int i = 0; i < nRunners; ++i )
			jobs.push_back( m_pPool->QueueCall( &run, &ParallelRun::Run ) );

		caller();
		run.Run();
		JoinRunners( jobs );
	}

	virtual int WorkerCount() const
	{
		if ( m_nWorkers >= 0 )
			return m_nWorkers;
		return m_pPool ? m_pPool->NumThreads() : 0;
	}

	unsigned NestedCalls() const { return m_nNestedCalls.load( std::memory_order_relaxed ); }

	IThreadPool *m_pPool;
	const bool m_bOwnsPool;

private:
	bool NestedOnOwnWorker()
	{
		if ( !m_bInlineOnOwnWorkers || !IsThreadPoolWorkerThread( m_pPool ) )
			return false;
		m_nNestedCalls.fetch_add( 1, std::memory_order_relaxed );
		return true;
	}

	static void JoinRunners( std::vector<CJob *> &jobs )
	{
		for ( CJob *job : jobs )
		{
			// Abort cancels unclaimed runners or joins a running one. Acquire the
			// recursive job mutex explicitly: Abort's finished fast path alone
			// is not a publication barrier for an already-completed callback.
			job->Lock();
			job->Abort();
			job->Unlock();
			job->Release();
		}
	}

	class ParallelRun
	{
	public:
		ParallelRun( unsigned count, const std::function<void( int )> &body )
		    : m_count( count ), m_body( body ), m_next( 0 )
		{
		}

		void Run()
		{
			for ( ;; )
			{
				// The cursor assigns disjoint indices only. Queue publication and
				// the owning CJob mutex publish inputs/outputs, not this counter.
				const unsigned index = m_next.fetch_add( 1, std::memory_order_relaxed );
				if ( index >= m_count )
					return;
				m_body( (int)index );
			}
		}

	private:
		const unsigned m_count;
		const std::function<void( int )> &m_body;
		// count <= INT_MAX and there are at most count claimants, so the final
		// unsuccessful claims cannot overflow this unsigned counter.
		std::atomic<unsigned> m_next;
	};

	const int m_nWorkers;
	const bool m_bInlineOnOwnWorkers;
	std::atomic<unsigned> m_nNestedCalls;
};

VSTDLIB_INTERFACE bool RunThreadPoolJobBatch(
    IThreadPool *pool, const jobsystem::BatchDesc &desc, jobsystem::BatchMode mode )
{
	// Stack-owned binding permits concurrent calls and borrows the same process
	// worker budget. Each backend invocation runs only its own callbacks on the
	// caller and joins/aborts only its own CJobs; no unrelated queue is drained.
	CThreadPoolWorkerBackend backend( pool, pool ? pool->NumThreads() : 0 );
	return jobsystem::ExecuteParallelBatch( desc, &backend, mode );
}

VSTDLIB_INTERFACE bool RunDeclaredFrameGraph( IThreadPool *pPool,
    jobsystem::DeclaredFrameGraph *pGraph, const jobsystem::FrameNodeDesc *pNodes, unsigned nNodes,
    jobsystem::FrameGraphMode mode, jobsystem::DeclaredFrameRun *pResult )
{
	// As RunThreadPoolJobBatch: a stack-owned binding to the borrowed pool.
	CThreadPoolWorkerBackend backend( pPool, pPool ? pPool->NumThreads() : 0 );
	*pResult = pGraph->Run( pNodes, nNodes, &backend, mode );
	return pResult->valid;
}

//-----------------------------------------------------------------------------

VSTDLIB_INTERFACE jobsystem::IWorkerBackend *CreateThreadPoolWorkerBackend( int nThreads )
{
	IThreadPool *pPool = CreateThreadPool();

	ThreadPoolStartParams_t params;
	params.nThreads = ( nThreads > 0 ) ? nThreads : 0; // explicit count: no CommandLine/CPU probe
	params.bExecOnThreadPoolThreadsOnly = false;
	pPool->Start( params );

	return new CThreadPoolWorkerBackend( pPool, pPool->NumThreads(), true );
}

VSTDLIB_INTERFACE void DestroyThreadPoolWorkerBackend( jobsystem::IWorkerBackend *pBackend )
{
	if ( !pBackend )
		return;

	CThreadPoolWorkerBackend *pImpl = static_cast< CThreadPoolWorkerBackend * >( pBackend );
	if ( pImpl->m_pPool && pImpl->m_bOwnsPool )
	{
		pImpl->m_pPool->Stop();
		DestroyThreadPool( pImpl->m_pPool );
	}
	delete pImpl;
}

//-----------------------------------------------------------------------------

VSTDLIB_INTERFACE jobsystem::IWorkerBackend *CreateComputePoolWorkerBackend()
{
	return new CThreadPoolWorkerBackend( g_pThreadPool, -1, false, true );
}

VSTDLIB_INTERFACE void DestroyComputePoolWorkerBackend( jobsystem::IWorkerBackend *pBackend )
{
	delete static_cast<CThreadPoolWorkerBackend *>( pBackend );
}

VSTDLIB_INTERFACE unsigned ComputePoolWorkerBackendNestedCalls(
    const jobsystem::IWorkerBackend *pBackend )
{
	return pBackend ? static_cast<const CThreadPoolWorkerBackend *>( pBackend )->NestedCalls() : 0;
}
