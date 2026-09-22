//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy thread-pool bridge implementation (RFC 0003, Phase C / R20).
//
//          Adapts the real engine CThreadPool + CParallelProcessor to the
//          scheduler's IWorkerBackend. Graph waves execute through the same
//          fork/join path the engine already uses, on real worker threads.
//
//=============================================================================//

#include <limits.h>
#include <vector>

#include "vstdlib/jobgraph_pool_bridge.h"
#include "jobsystem/worker_backend.h"
#include "vstdlib/jobthread.h"

#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// IWorkerBackend backed by a private CThreadPool.
//-----------------------------------------------------------------------------
class CThreadPoolWorkerBackend : public jobsystem::IWorkerBackend
{
public:
	CThreadPoolWorkerBackend( IThreadPool *pPool, int nWorkers )
		: m_pPool( pPool ), m_nWorkers( nWorkers ), m_pCurBody( NULL ) {}
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

		// Drive the real engine parallel primitive over indices [0, n). Workers
		// (and the calling thread) claim indices via CParallelProcessor's shared
		// cursor; each index is processed exactly once, and Run() joins.
		std::vector<int> items( (size_t)n );
		for ( int i = 0; i < n; ++i )
			items[(size_t)i] = i;

		m_pCurBody = &body;
		CParallelProcessor< int, CMemberFuncJobItemProcessor< int, CThreadPoolWorkerBackend > > processor( "jobgraph.wave" );
		processor.m_ItemProcessor.Init( this, &CThreadPoolWorkerBackend::ProcessIndex );
		processor.Run( items.data(), (unsigned)n, INT_MAX, m_pPool );
		m_pCurBody = NULL;
	}

	virtual int WorkerCount() const { return m_nWorkers; }

	IThreadPool *m_pPool;

private:
	void ProcessIndex( int &i ) { ( *m_pCurBody )( i ); }

	int m_nWorkers;
	const std::function<void( int )> *m_pCurBody;
};

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
