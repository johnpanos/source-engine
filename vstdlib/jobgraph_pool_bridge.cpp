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
	CThreadPoolWorkerBackend( IThreadPool *pPool, int nWorkers )
	    : m_pPool( pPool ), m_nWorkers( nWorkers )
	{
	}
	virtual ~CThreadPoolWorkerBackend() {}

	virtual void ParallelFor( int n, const std::function<void( int )> &body )
	{
		if ( n <= 0 )
			return;

		// No workers, or a single item: run inline. Avoids pool overhead and the
		// documented risk of helping unrelated queued work while waiting.
		if ( !m_pPool || m_nWorkers <= 0 || n == 1 )
		{
			for ( int i = 0; i < n; ++i )
				body( i );
			return;
		}

		// This call owns every runner and callback borrow. Queue at most one
		// runner per available worker, including a pool with just one worker.
		// The caller claims only this call's indices and never helps other jobs.
		const int nRunners = n - 1 < m_nWorkers ? n - 1 : m_nWorkers;
		std::vector<CJob *> jobs;
		jobs.reserve( (size_t)nRunners );
		ParallelRun run( (unsigned)n, body );
		for ( int i = 0; i < nRunners; ++i )
			jobs.push_back( m_pPool->QueueCall( &run, &ParallelRun::Run ) );

		run.Run();
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

	virtual int WorkerCount() const { return m_nWorkers; }

	IThreadPool *m_pPool;

private:
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

	int m_nWorkers;
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

//-----------------------------------------------------------------------------

VSTDLIB_INTERFACE jobsystem::IWorkerBackend *CreateThreadPoolWorkerBackend( int nThreads )
{
	IThreadPool *pPool = CreateThreadPool();

	ThreadPoolStartParams_t params;
	params.nThreads = ( nThreads > 0 ) ? nThreads : 0; // explicit count: no CommandLine/CPU probe
	params.bExecOnThreadPoolThreadsOnly = false;
	pPool->Start( params );

	return new CThreadPoolWorkerBackend( pPool, pPool->NumThreads() );
}

VSTDLIB_INTERFACE void DestroyThreadPoolWorkerBackend( jobsystem::IWorkerBackend *pBackend )
{
	if ( !pBackend )
		return;

	CThreadPoolWorkerBackend *pImpl = static_cast< CThreadPoolWorkerBackend * >( pBackend );
	if ( pImpl->m_pPool )
	{
		pImpl->m_pPool->Stop();
		DestroyThreadPool( pImpl->m_pPool );
	}
	delete pImpl;
}
