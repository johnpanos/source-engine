//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native race and contract fixture for the real vstdlib CThreadPool
//          and the tier0 CTSQueue (RFC 0003 R20, RFC 0005 Q-JOBS).
//
//          Built in the legacy C++11 dialect against the real tier0/tier1/
//          vstdlib libraries; run it under the clang TSan Waf profile as the
//          race gate. Covers: exactly-once execution and job accounting,
//          worker steal deques (placement, stealing, bounded spill), the wait
//          rule (no unrelated work runs on a waiting thread; waited jobs run
//          inline only when eligible), quiescent spill/abort of steal deques,
//          recursive fan-out with nested waits, the legacy parallel
//          processors, concurrent status observation, and CTSQueue MPMC
//          exactly-once/FIFO behavior.
//
//=============================================================================//

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

#include "tier0/tslist.h"
#include "vstdlib/jobthread.h"
#include "testing/conformance_result.h"

static std::atomic<unsigned long> s_checks( 0 );
static std::atomic<unsigned long> s_failures( 0 );

static void Check( bool value, const char *expression, int line )
{
	++s_checks;
	if ( !value )
	{
		++s_failures;
		std::printf( "FAIL line %d: %s\n", line, expression );
		std::fflush( stdout );
	}
}
#define CHECK( expression ) Check( ( expression ), #expression, __LINE__ )

static void SleepMs( int ms )
{
	std::this_thread::sleep_for( std::chrono::milliseconds( ms ) );
}

// Waits for a condition with a deadline; a stuck scheduler fails the check
// instead of hanging the run.
template <typename Predicate> static bool WaitUntil( Predicate predicate, int timeoutMs = 20000 )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( timeoutMs );
	while ( !predicate() )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		std::this_thread::yield();
	}
	return true;
}

//-----------------------------------------------------------------------------
// Instrumented job
//-----------------------------------------------------------------------------

class CProbeJob : public CJob
{
public:
	CProbeJob() : m_nExecuted( 0 ), m_nAborted( 0 ), m_thread( 0 ), m_pGate( NULL ), m_pDone( NULL )
	{
	}

	virtual JobStatus_t DoExecute()
	{
		m_thread.store( ThreadGetCurrentId() );
		if ( m_pGate )
		{
			while ( !m_pGate->load() )
				SleepMs( 1 );
		}
		++m_nExecuted;
		if ( m_pDone )
			++*m_pDone;
		return JOB_OK;
	}

	virtual JobStatus_t DoAbort( bool bDiscard )
	{
		++m_nAborted;
		if ( m_pDone )
			++*m_pDone;
		return JOB_STATUS_ABORTED;
	}

	std::atomic<int> m_nExecuted;
	std::atomic<int> m_nAborted;
	std::atomic<ThreadId_t> m_thread;
	std::atomic<bool> *m_pGate;
	std::atomic<int> *m_pDone;
};

static IThreadPool *StartPool( int nThreads, bool bExecOnPoolThreadsOnly = false )
{
	IThreadPool *pPool = CreateThreadPool();
	ThreadPoolStartParams_t params;
	params.nThreads = nThreads;
	params.fDistribute = TRS_FALSE;
	params.bExecOnThreadPoolThreadsOnly = bExecOnPoolThreadsOnly;
	CHECK( pPool->Start( params, "PoolTst" ) );
	CHECK( pPool->NumThreads() == nThreads );
	return pPool;
}

static void StopPool( IThreadPool *pPool )
{
	CHECK( pPool->Stop() );
	DestroyThreadPool( pPool );
}

static ThreadPoolSchedulingStats_t Stats( IThreadPool *pPool )
{
	ThreadPoolSchedulingStats_t stats;
	GetThreadPoolSchedulingStats( pPool, &stats );
	return stats;
}

//-----------------------------------------------------------------------------
// 1. Shared-queue admission from a non-worker thread
//-----------------------------------------------------------------------------

static void TestSharedQueueExactlyOnce()
{
	IThreadPool *pPool = StartPool( 3 );
	const int N = 2000;
	std::vector<CProbeJob *> jobs( N );
	for ( int i = 0; i < N; i++ )
	{
		jobs[i] = new CProbeJob;
		jobs[i]->SetFlags( JF_QUEUE );
		pPool->AddJob( jobs[i] );
	}

	// Concurrent status observation while workers publish results.
	int nFinishedSeen = 0;
	for ( int i = 0; i < N; i++ )
	{
		if ( jobs[i]->IsFinished() )
			++nFinishedSeen;
	}
	CHECK( nFinishedSeen <= N );

	for ( int i = 0; i < N; i++ )
	{
		CHECK( jobs[i]->WaitForFinish( TT_INFINITE, pPool ) );
	}
	int nExecuted = 0;
	for ( int i = 0; i < N; i++ )
	{
		nExecuted += ( jobs[i]->m_nExecuted == 1 && jobs[i]->Executed() ) ? 1 : 0;
		jobs[i]->Release();
	}
	CHECK( nExecuted == N );
	CHECK( WaitUntil(
	    [&]
	    {
		    return pPool->GetJobCount() == 0;
	    } ) );
	CHECK( Stats( pPool ).nStealDequePushes == 0 ); // no worker spawned anything
	StopPool( pPool );
}

//-----------------------------------------------------------------------------
// 2/3. Worker-spawned work: steal deque placement, stealing, bounded spill
//-----------------------------------------------------------------------------

class CSpawnJob : public CJob
{
public:
	CSpawnJob( IThreadPool *pPool, int nChildren, bool bWaitForChildrenWithoutHelping )
	    : m_pPool( pPool ), m_nChildren( nChildren ),
	      m_bWaitForChildren( bWaitForChildrenWithoutHelping ), m_bChildrenFinished( false ),
	      m_thread( 0 ), m_nDone( 0 )
	{
	}

	virtual JobStatus_t DoExecute()
	{
		m_thread = ThreadGetCurrentId();
		m_started = true;
		for ( int i = 0; i < m_nChildren; i++ )
		{
			CProbeJob *pChild = new CProbeJob;
			pChild->SetFlags( JF_QUEUE );
			pChild->m_pDone = &m_nDone;
			m_children.push_back( pChild );
			m_pPool->AddJob( pChild );
		}
		if ( m_bWaitForChildren )
		{
			// Block this worker without helping: the children can complete
			// only if other workers steal them from this worker's deque.
			m_bChildrenFinished = WaitUntil(
			    [&]
			    {
				    return m_nDone.load() == m_nChildren;
			    } );
		}
		return JOB_OK;
	}

	IThreadPool *m_pPool;
	int m_nChildren;
	bool m_bWaitForChildren;
	bool m_bChildrenFinished;
	ThreadId_t m_thread;
	std::atomic<bool> m_started{ false };
	std::atomic<int> m_nDone;
	std::vector<CProbeJob *> m_children;
};

// Joins a job only after a worker started it: waiting earlier would make the
// waiter eligible to run it inline (the wait rule), moving it off the pool.
static void JoinStartedOnWorker( IThreadPool *pPool, CSpawnJob *pJob )
{
	CHECK( WaitUntil(
	    [&]
	    {
		    return pJob->m_started.load();
	    } ) );
	CHECK( pJob->m_thread != ThreadGetCurrentId() );
	CHECK( pJob->WaitForFinish( TT_INFINITE, pPool ) );
}

static void TestStealing()
{
	IThreadPool *pPool = StartPool( 4 );
	const ThreadPoolSchedulingStats_t before = Stats( pPool );

	const int nChildren = 64;
	CSpawnJob *pParent = new CSpawnJob( pPool, nChildren, true );
	pParent->SetFlags( JF_QUEUE );
	pPool->AddJob( pParent );
	JoinStartedOnWorker( pPool, pParent );
	CHECK( pParent->m_bChildrenFinished );

	int nOnce = 0;
	int nOnOtherThread = 0;
	for ( CProbeJob *pChild : pParent->m_children )
	{
		pChild->WaitForFinish( TT_INFINITE, pPool );
		nOnce += ( pChild->m_nExecuted == 1 && pChild->m_nAborted == 0 ) ? 1 : 0;
		nOnOtherThread += ( pChild->m_thread.load() != pParent->m_thread ) ? 1 : 0;
		pChild->Release();
	}
	CHECK( nOnce == nChildren );
	CHECK( nOnOtherThread == nChildren );

	const ThreadPoolSchedulingStats_t after = Stats( pPool );
	std::printf( "stealing: pushes %d spills %d steals %d inline %d\n",
	    after.nStealDequePushes - before.nStealDequePushes,
	    after.nStealDequeSpills - before.nStealDequeSpills, after.nSteals - before.nSteals,
	    after.nWaitedJobsRunInline );
	CHECK( after.nStealDequePushes - before.nStealDequePushes == nChildren );
	CHECK( after.nStealDequeSpills == before.nStealDequeSpills );
	CHECK( after.nSteals - before.nSteals == nChildren );
	CHECK( WaitUntil(
	    [&]
	    {
		    return pPool->GetJobCount() == 0;
	    } ) );
	pParent->Release();
	StopPool( pPool );
}

static void TestBoundedSpill()
{
	// One worker: nothing can steal, so the deque fills exactly to capacity
	// and every later spawn spills to the shared queue.
	IThreadPool *pPool = StartPool( 1 );
	const int nChildren = 1000;
	CSpawnJob *pParent = new CSpawnJob( pPool, nChildren, false );
	pParent->SetFlags( JF_QUEUE );
	pPool->AddJob( pParent );
	JoinStartedOnWorker( pPool, pParent );
	CHECK( WaitUntil(
	    [&]
	    {
		    return pParent->m_nDone.load() == nChildren;
	    } ) );

	int nOnce = 0;
	for ( CProbeJob *pChild : pParent->m_children )
	{
		nOnce +=
		    ( pChild->m_nExecuted == 1 && pChild->m_thread.load() == pParent->m_thread ) ? 1 : 0;
		pChild->Release();
	}
	CHECK( nOnce == nChildren );

	const ThreadPoolSchedulingStats_t stats = Stats( pPool );
	std::printf( "spill: pushes %d spills %d steals %d once %d\n", stats.nStealDequePushes,
	    stats.nStealDequeSpills, stats.nSteals, nOnce );
	CHECK( stats.nStealDequePushes == 256 );
	CHECK( stats.nStealDequeSpills == nChildren - 256 );
	CHECK( stats.nSteals == 0 );
	CHECK( WaitUntil(
	    [&]
	    {
		    return pPool->GetJobCount() == 0;
	    } ) );
	pParent->Release();
	StopPool( pPool );
}

//-----------------------------------------------------------------------------
// 4-6. Wait rule
//-----------------------------------------------------------------------------

struct BlockedPool
{
	IThreadPool *pPool;
	std::atomic<bool> gate;
	std::vector<CProbeJob *> blockers;

	BlockedPool( int nThreads, bool bExecOnPoolThreadsOnly = false ) : gate( false )
	{
		pPool = StartPool( nThreads, bExecOnPoolThreadsOnly );
		for ( int i = 0; i < nThreads; i++ )
		{
			CProbeJob *pBlocker = new CProbeJob;
			pBlocker->SetFlags( JF_QUEUE );
			pBlocker->m_pGate = &gate;
			blockers.push_back( pBlocker );
			pPool->AddJob( pBlocker );
		}
		// Every worker is now inside a blocker.
		CHECK( WaitUntil(
		    [&]
		    {
			    return pPool->NumIdleThreads() == 0;
		    } ) );
		CHECK( WaitUntil(
		    [&]
		    {
			    for ( CProbeJob *p : blockers )
				    if ( !p->m_thread.load() )
					    return false;
			    return true;
		    } ) );
	}

	void Release()
	{
		gate = true;
		for ( CProbeJob *p : blockers )
		{
			p->WaitForFinish( TT_INFINITE, pPool );
			p->Release();
		}
		StopPool( pPool );
	}
};

static CProbeJob *QueueProbe( IThreadPool *pPool, unsigned flags = JF_QUEUE )
{
	CProbeJob *pJob = new CProbeJob;
	pJob->SetFlags( flags );
	pPool->AddJob( pJob );
	return pJob;
}

static void TestWaitOnEventRunsNothing()
{
	BlockedPool blocked( 1 );
	CProbeJob *pUnrelated = QueueProbe( blocked.pPool );

	CThreadEvent event;
	std::thread signaler(
	    [&]
	    {
		    SleepMs( 100 );
		    event.Set();
	    } );
	CHECK( blocked.pPool->YieldWait( event ) );
	signaler.join();

	// The legacy wait popped the shared queue here and ran the unrelated job.
	CHECK( pUnrelated->m_nExecuted == 0 );
	CHECK( !pUnrelated->IsFinished() );

	// Observe completion without waiting on the job: a wait would make it a
	// waited job, which may legitimately run here.
	blocked.gate = true;
	CHECK( WaitUntil(
	    [&]
	    {
		    return pUnrelated->m_nExecuted.load() == 1;
	    } ) );
	CHECK( pUnrelated->m_thread.load() != ThreadGetCurrentId() );
	pUnrelated->Release();
	blocked.Release();
}

static void TestWaitRunsOnlyWaitedJob()
{
	BlockedPool blocked( 1 );
	const ThreadPoolSchedulingStats_t before = Stats( blocked.pPool );
	CProbeJob *pUnrelated = QueueProbe( blocked.pPool );
	CProbeJob *pWaited = QueueProbe( blocked.pPool );

	CHECK( pWaited->WaitForFinish( TT_INFINITE, blocked.pPool ) );
	CHECK( pWaited->m_nExecuted == 1 );
	CHECK( pWaited->m_thread.load() == ThreadGetCurrentId() );
	CHECK( pUnrelated->m_nExecuted == 0 );
	CHECK( Stats( blocked.pPool ).nWaitedJobsRunInline - before.nWaitedJobsRunInline == 1 );

	// CJobSet waits go through the same rule.
	CProbeJob *pSetA = QueueProbe( blocked.pPool );
	CProbeJob *pSetB = QueueProbe( blocked.pPool );
	pSetA->AddRef();
	pSetB->AddRef();
	{
		CJobSet set;
		set += pSetA;
		set += pSetB;
		set.WaitForFinish( blocked.pPool );
	}
	CHECK( pSetA->m_nExecuted == 1 && pSetB->m_nExecuted == 1 );
	CHECK( pUnrelated->m_nExecuted == 0 );
	pSetA->Release();
	pSetB->Release();

	blocked.gate = true;
	CHECK( pUnrelated->WaitForFinish( TT_INFINITE, blocked.pPool ) );
	CHECK( pUnrelated->m_nExecuted == 1 );
	// The worker pops the inline-executed jobs' entries and releases them
	// without rerunning them; accounting returns to zero.
	CHECK( WaitUntil(
	    [&]
	    {
		    return blocked.pPool->GetJobCount() == 0;
	    } ) );
	pUnrelated->Release();
	pWaited->Release();
	blocked.Release();
}

static void TestIneligibleWaitedJobs()
{
	// JF_SERIAL: ordered on worker 0, never run by a waiter.
	{
		BlockedPool blocked( 1 );
		CProbeJob *pSerial = QueueProbe( blocked.pPool, JF_QUEUE | JF_SERIAL );
		std::thread opener(
		    [&]
		    {
			    SleepMs( 100 );
			    blocked.gate = true;
		    } );
		CHECK( pSerial->WaitForFinish( TT_INFINITE, blocked.pPool ) );
		opener.join();
		CHECK( pSerial->m_nExecuted == 1 );
		CHECK( pSerial->m_thread.load() != ThreadGetCurrentId() );
		pSerial->Release();
		blocked.Release();
	}

	// Pool that forbids non-pool threads (e.g. a render thread's GL context).
	{
		BlockedPool blocked( 1, true );
		CProbeJob *pJob = QueueProbe( blocked.pPool );
		std::thread opener(
		    [&]
		    {
			    SleepMs( 100 );
			    blocked.gate = true;
		    } );
		CHECK( pJob->WaitForFinish( TT_INFINITE, blocked.pPool ) );
		opener.join();
		CHECK( pJob->m_nExecuted == 1 );
		CHECK( pJob->m_thread.load() != ThreadGetCurrentId() );
		pJob->Release();
		blocked.Release();
	}

	// A job owned by another pool is not run by this pool's waiter.
	{
		BlockedPool owner( 1 );
		IThreadPool *pOther = StartPool( 1 );
		CProbeJob *pJob = QueueProbe( owner.pPool );
		std::thread opener(
		    [&]
		    {
			    SleepMs( 100 );
			    owner.gate = true;
		    } );
		CHECK( pJob->WaitForFinish( TT_INFINITE, pOther ) );
		opener.join();
		CHECK( pJob->m_nExecuted == 1 );
		CHECK( pJob->m_thread.load() != ThreadGetCurrentId() );
		pJob->Release();
		StopPool( pOther );
		owner.Release();
	}
}

//-----------------------------------------------------------------------------
// 7. Quiescent operations see steal-deque contents
//-----------------------------------------------------------------------------

class CSpawnThenBlockJob : public CJob
{
public:
	CSpawnThenBlockJob( IThreadPool *pPool, int nChildren )
	    : m_pPool( pPool ), m_nChildren( nChildren ), m_gate( false ), m_spawned( false )
	{
	}

	virtual JobStatus_t DoExecute()
	{
		for ( int i = 0; i < m_nChildren; i++ )
		{
			CProbeJob *pChild = new CProbeJob;
			pChild->SetFlags( JF_QUEUE );
			m_children.push_back( pChild );
			m_pPool->AddJob( pChild );
		}
		m_spawned = true;
		while ( !m_gate.load() )
			SleepMs( 1 );
		return JOB_OK;
	}

	IThreadPool *m_pPool;
	int m_nChildren;
	std::atomic<bool> m_gate;
	std::atomic<bool> m_spawned;
	std::vector<CProbeJob *> m_children;
};

enum QuiescentOp
{
	QUIESCENT_ABORT_ALL,
	QUIESCENT_EXECUTE_ALL,
	QUIESCENT_STOP,
};

static void TestQuiescentDequeHandling( QuiescentOp op )
{
	IThreadPool *pPool = StartPool( 1 );
	const int nChildren = 100;
	CSpawnThenBlockJob *pParent = new CSpawnThenBlockJob( pPool, nChildren );
	pParent->SetFlags( JF_QUEUE );
	pPool->AddJob( pParent );
	CHECK( WaitUntil(
	    [&]
	    {
		    return pParent->m_spawned.load();
	    } ) );

	// Release the parent only after the suspend/exit call is pending, so the
	// worker observes the call before popping its deque.
	std::thread opener(
	    [&]
	    {
		    SleepMs( 150 );
		    pParent->m_gate = true;
	    } );

	int nResult = 0;
	switch ( op )
	{
	case QUIESCENT_ABORT_ALL:
		nResult = pPool->AbortAll();
		break;
	case QUIESCENT_EXECUTE_ALL:
		nResult = pPool->ExecuteAll();
		break;
	case QUIESCENT_STOP:
		CHECK( pPool->Stop() );
		break;
	}
	opener.join();

	int nExecuted = 0, nAborted = 0, nOnCaller = 0;
	for ( CProbeJob *pChild : pParent->m_children )
	{
		nExecuted += pChild->m_nExecuted;
		nAborted += pChild->m_nAborted;
		nOnCaller +=
		    ( pChild->m_nExecuted && pChild->m_thread.load() == ThreadGetCurrentId() ) ? 1 : 0;
		CHECK( pChild->IsFinished() );
	}
	switch ( op )
	{
	case QUIESCENT_ABORT_ALL:
		CHECK( nResult == nChildren );
		CHECK( nAborted == nChildren && nExecuted == 0 );
		break;
	case QUIESCENT_EXECUTE_ALL:
		CHECK( nResult == nChildren );
		CHECK( nExecuted == nChildren && nAborted == 0 && nOnCaller == nChildren );
		break;
	case QUIESCENT_STOP:
		CHECK( nAborted == nChildren && nExecuted == 0 );
		break;
	}
	CHECK( pPool->GetJobCount() == 0 );
	for ( CProbeJob *pChild : pParent->m_children )
		pChild->Release();
	pParent->Release();
	if ( op != QUIESCENT_STOP )
		CHECK( pPool->Stop() );
	DestroyThreadPool( pPool );
}

//-----------------------------------------------------------------------------
// 8. Recursive fan-out with nested waits
//-----------------------------------------------------------------------------

static std::atomic<int> s_treeExecuted( 0 );

class CTreeJob : public CJob
{
public:
	CTreeJob( IThreadPool *pPool, int depth ) : m_pPool( pPool ), m_depth( depth ), m_nExecuted( 0 )
	{
	}

	virtual JobStatus_t DoExecute()
	{
		++m_nExecuted;
		++s_treeExecuted;
		if ( m_depth > 0 )
		{
			CTreeJob *children[4];
			for ( int i = 0; i < 4; i++ )
			{
				children[i] = new CTreeJob( m_pPool, m_depth - 1 );
				// Alternate admission paths: queued (steal deque) and
				// run-inline-when-busy (the legacy AddJob rule).
				children[i]->SetFlags( ( i & 1 ) ? JF_QUEUE : 0 );
				m_pPool->AddJob( children[i] );
			}
			// A waiting worker may run its own waited children, nothing else.
			CJob *waits[4] = { children[0], children[1], children[2], children[3] };
			m_pPool->YieldWait( waits, 4 );
			for ( int i = 0; i < 4; i++ )
			{
				if ( children[i]->m_nExecuted != 1 || !children[i]->Executed() )
					++s_treeErrors;
				children[i]->Release();
			}
		}
		return JOB_OK;
	}

	static std::atomic<int> s_treeErrors;
	IThreadPool *m_pPool;
	int m_depth;
	std::atomic<int> m_nExecuted;
};
std::atomic<int> CTreeJob::s_treeErrors( 0 );

static void TestRecursiveFanOut()
{
	IThreadPool *pPool = StartPool( 4 );
	for ( int round = 0; round < 8; round++ )
	{
		s_treeExecuted = 0;
		CTreeJob *pRoot = new CTreeJob( pPool, 4 ); // 1 + 4 + 16 + 64 + 256 = 341 jobs
		pRoot->SetFlags( JF_QUEUE );
		pPool->AddJob( pRoot );
		// Odd rounds let the waiting main thread run the root inline if no
		// worker has started it; even rounds start it on a worker first.
		if ( ( round & 1 ) == 0 )
			CHECK( WaitUntil(
			    [&]
			    {
				    return pRoot->m_nExecuted.load() == 1;
			    } ) );
		CHECK( pRoot->WaitForFinish( TT_INFINITE, pPool ) );
		CHECK( pRoot->m_nExecuted == 1 );
		CHECK( s_treeExecuted == 341 );
		pRoot->Release();
	}
	CHECK( CTreeJob::s_treeErrors == 0 );
	CHECK( WaitUntil(
	    [&]
	    {
		    return pPool->GetJobCount() == 0;
	    } ) );
	CHECK( Stats( pPool ).nSteals > 0 );
	StopPool( pPool );
}

//-----------------------------------------------------------------------------
// 9. Legacy parallel processors (parallel cursor and active counts)
//-----------------------------------------------------------------------------

struct CounterItem
{
	std::atomic<int> count;
	CounterItem() : count( 0 ) {}
};

static void CountItem( CounterItem &item )
{
	++item.count;
}

static void CountIndex( long const &index );
static std::vector<CounterItem> *s_pLoopItems;
static void CountIndex( long const &index )
{
	++( *s_pLoopItems )[index].count;
}

static void TestParallelProcessors()
{
	// ParallelProcess on an explicit pool; ParallelLoopProcess uses the global
	// pool, which this test starts and stops.
	IThreadPool *pPool = StartPool( 3 );
	for ( int round = 0; round < 20; round++ )
	{
		std::vector<CounterItem> items( 5000 );
		ParallelProcess(
		    "PoolTstProcess", pPool, items.data(), (unsigned)items.size(), &CountItem );
		int nOnce = 0;
		for ( CounterItem &item : items )
			nOnce += ( item.count == 1 ) ? 1 : 0;
		CHECK( nOnce == (int)items.size() );
	}
	CHECK( WaitUntil(
	    [&]
	    {
		    return pPool->GetJobCount() == 0;
	    } ) );
	StopPool( pPool );

	ThreadPoolStartParams_t params;
	params.nThreads = 3;
	params.fDistribute = TRS_FALSE;
	CHECK( g_pThreadPool->Start( params ) );
	for ( int round = 0; round < 20; round++ )
	{
		std::vector<CounterItem> items( 5000 );
		s_pLoopItems = &items;
		ParallelLoopProcess( "PoolTstLoop", 0, (unsigned)items.size(), &CountIndex );
		int nOnce = 0;
		for ( CounterItem &item : items )
			nOnce += ( item.count == 1 ) ? 1 : 0;
		CHECK( nOnce == (int)items.size() );
	}
	CHECK( WaitUntil(
	    [&]
	    {
		    return g_pThreadPool->GetJobCount() == 0;
	    } ) );
	CHECK( g_pThreadPool->Stop() );
}

//-----------------------------------------------------------------------------
// 10. CTSQueue
//-----------------------------------------------------------------------------

static void TestTSQueueSingleThread()
{
	CTSQueue<int> queue;
	for ( int i = 0; i < 1000; i++ )
		queue.PushItem( i );
	CHECK( queue.Count() == 1000 );
	CHECK( queue.ValidateQueue() );
	bool bInOrder = true;
	for ( int i = 0; i < 1000; i++ )
	{
		int value = -1;
		if ( !queue.PopItem( &value ) || value != i )
			bInOrder = false;
	}
	CHECK( bInOrder );
	int value;
	CHECK( !queue.PopItem( &value ) );
	CHECK( queue.Count() == 0 );

	// Node API: Pop returns a caller-owned node carrying the element.
	typedef CTSQueue<int>::Node_t Node_t;
	Node_t *pA = new Node_t( 7 );
	Node_t *pB = new Node_t( 8 );
	queue.Push( pA );
	queue.Push( pB );
	Node_t *pFirst = queue.Pop();
	Node_t *pSecond = queue.Pop();
	CHECK( pFirst && pFirst->elem == 7 );
	CHECK( pSecond && pSecond->elem == 8 );
	CHECK( queue.Pop() == NULL );
	delete pFirst;
	queue.FreeNode( pSecond );
	CHECK( queue.ValidateQueue() );

	queue.PushItem( 1 );
	queue.PushItem( 2 );
	queue.RemoveAll();
	CHECK( queue.Count() == 0 );
	CHECK( queue.ValidateQueue() );
}

static void TestTSQueueConcurrent()
{
	const int nProducers = 4;
	const int nConsumers = 4;
	const int nPerProducer = 20000;
	const int nTotal = nProducers * nPerProducer;

	CTSQueue<int> *pQueue = new CTSQueue<int>;
	std::vector<std::atomic<int>> seen( nTotal );
	for ( std::atomic<int> &s : seen )
		s = 0;
	std::atomic<int> nPopped( 0 );
	std::atomic<int> nOrderErrors( 0 );
	std::atomic<int> nProducersDone( 0 );

	std::vector<std::thread> threads;
	for ( int p = 0; p < nProducers; p++ )
	{
		threads.push_back( std::thread(
		    [&, p]
		    {
			    for ( int i = 0; i < nPerProducer; i++ )
				    pQueue->PushItem( p * nPerProducer + i );
			    ++nProducersDone;
		    } ) );
	}
	for ( int c = 0; c < nConsumers; c++ )
	{
		threads.push_back( std::thread(
		    [&]
		    {
			    // FIFO per producer: one consumer sees each producer's values in
			    // increasing order.
			    std::vector<int> last( nProducers, -1 );
			    for ( ;; )
			    {
				    int value;
				    if ( pQueue->PopItem( &value ) )
				    {
					    if ( value >= 0 && value < nTotal )
					    {
						    ++seen[value];
						    const int producer = value / nPerProducer;
						    if ( value <= last[producer] )
							    ++nOrderErrors;
						    last[producer] = value;
					    }
					    ++nPopped;
				    }
				    else if ( nProducersDone.load() == nProducers && pQueue->Count() == 0 )
				    {
					    break;
				    }
			    }
		    } ) );
	}
	for ( std::thread &t : threads )
		t.join();

	int nOnce = 0;
	for ( std::atomic<int> &s : seen )
		nOnce += ( s == 1 ) ? 1 : 0;
	CHECK( nPopped == nTotal );
	CHECK( nOnce == nTotal );
	CHECK( nOrderErrors == 0 );
	CHECK( pQueue->Count() == 0 );
	CHECK( pQueue->ValidateQueue() );
	delete pQueue;
}

//-----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	// A scheduler that loses a wake-up or deadlocks must fail, not hang.
	std::thread watchdog(
	    []
	    {
		    SleepMs( 600000 );
		    std::printf( "FAIL: watchdog expired\n" );
		    std::fflush( stdout );
		    std::_Exit( 3 );
	    } );
	watchdog.detach();

	TestTSQueueSingleThread();
	TestTSQueueConcurrent();
	TestSharedQueueExactlyOnce();
	TestStealing();
	TestBoundedSpill();
	TestWaitOnEventRunsNothing();
	TestWaitRunsOnlyWaitedJob();
	TestIneligibleWaitedJobs();
	TestQuiescentDequeHandling( QUIESCENT_ABORT_ALL );
	TestQuiescentDequeHandling( QUIESCENT_EXECUTE_ALL );
	TestQuiescentDequeHandling( QUIESCENT_STOP );
	TestRecursiveFanOut();
	TestParallelProcessors();

	return testing::ReportConformance( s_checks.load(), s_failures.load() );
}
