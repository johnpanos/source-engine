//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy thread-pool bridge implementation (RFC 0003, Phase C / R20).
//
//          Adapts the real engine CThreadPool to the scheduler's IWorkerBackend:
//          each posted task is one CJob, settled by its poster alone.
//
//=============================================================================//

#include <atomic>

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

	// A runner task of the graph scheduler, as one CJob on the pool. Calls
	// from the pool's own workers are accepted: that is how a running task
	// asks for more help. The scheduler never posts under its own lock, so a
	// full shared queue that runs the job on its posting worker is safe.
	virtual void *PostTask( jobsystem::WorkerTaskFn task, void *context )
	{
		if ( !task || !m_pPool || WorkerCount() <= 0 )
			return NULL;
		return m_pPool->QueueCall( task, context );
	}

	// Abort withdraws a job no worker has claimed, or waits for a claimed one
	// under its mutex. Acquire the recursive job mutex explicitly: Abort's
	// finished fast path alone is not a publication barrier for an
	// already-completed callback.
	virtual bool SettleTask( void *ticket )
	{
		CJob *job = static_cast<CJob *>( ticket );
		job->Lock();
		const JobStatus_t status = job->Abort();
		job->Unlock();
		job->Release();
		return status != JOB_STATUS_ABORTED;
	}

	// A graph entered on one of the pool's own workers runs inline and is
	// counted, instead of posting runners behind the job that waits for them.
	virtual bool ShouldRunInline() { return NestedOnOwnWorker(); }

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

	const int m_nWorkers;
	const bool m_bInlineOnOwnWorkers;
	std::atomic<unsigned> m_nNestedCalls;
};

VSTDLIB_INTERFACE bool RunThreadPoolJobBatch(
    IThreadPool *pool, const jobsystem::BatchDesc &desc, jobsystem::BatchMode mode )
{
	// Stack-owned binding permits concurrent calls and borrows the same process
	// worker budget. Each run settles only its own CJobs; no unrelated queue is
	// drained.
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
