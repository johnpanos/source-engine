//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <deque>
#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "vstdlib/jobthread.h"
#include "vstdlib/random.h"
#include "tier1/functors.h"
#include "tier1/fmtstr.h"
#include "tier1/utlvector.h"
#include "tier1/generichash.h"
#include "tier0/vprof.h"
#include "job_steal_deque.h"

#include "tier0/native_module_load_telemetry.h"
#include "tier0/memdbgon.h"


class CJobThread;

//-----------------------------------------------------------------------------

inline void ServiceJobAndRelease( CJob *pJob, int iThread = -1 )
{
	// TryLock() would only fail if another thread has entered
	// Execute() or Abort()
	if ( !pJob->IsFinished() && pJob->TryLock() )
	{
		// ...service the request
		pJob->SetServiceThread( iThread );
		pJob->Execute();
		pJob->Unlock();
	}
	pJob->Release();
}

//-----------------------------------------------------------------------------

// A mutex-guarded priority FIFO. Topology: MPMC; producers are any thread,
// consumers are the owning pool's workers, a waiting thread running a job it
// waits for (by identity, never by popping), and the quiescent bulk operations.
// Payloads are counted CJob references: an entry holds one reference from
// admission until the consumer that pops it releases it.
//
// Capacity bounds admission through TryPush, which never runs or drops a job:
// a full queue refuses and the pool applies its overflow policy
// (CThreadPool::AdmitToFullSharedQueue). ForcePush re-inserts already
// admitted work (priority changes, quiescent spills and put-backs) and may
// exceed capacity. Direct (affinity) queues keep the default unbounded
// capacity. The space event is set whenever the queue holds fewer items than
// its capacity; it is a wake hint for blocked producers, and TryPush under the
// queue mutex is the authoritative check.
class ALIGN16 CJobQueue
{
public:
	CJobQueue() : m_nItems( 0 ), m_nMaxItems( INT_MAX ), m_nPeakItems( 0 )
	{
		m_SpaceAvailableEvent.Set();
	}

	int Count()
	{
		AUTO_LOCK( m_mutex );
		return m_nItems;
	}

	int Count( JobPriority_t priority )
	{
		AUTO_LOCK( m_mutex );
		return (int)m_Queues[priority].size();
	}

	int PeakCount()
	{
		AUTO_LOCK( m_mutex );
		return m_nPeakItems;
	}

	int Capacity()
	{
		AUTO_LOCK( m_mutex );
		return m_nMaxItems;
	}

	void SetCapacity( int nMaxItems )
	{
		AUTO_LOCK( m_mutex );
		m_nMaxItems = nMaxItems;
		UpdateSpaceLocked();
	}

	// Admits pJob unless the queue already holds its capacity.
	bool TryPush( CJob *pJob )
	{
		AUTO_LOCK( m_mutex );
		if ( m_nItems >= m_nMaxItems )
			return false;
		PushLocked( pJob );
		return true;
	}

	// Admits pJob regardless of capacity (see the class comment).
	void ForcePush( CJob *pJob )
	{
		AUTO_LOCK( m_mutex );
		PushLocked( pJob );
	}

	bool Pop( CJob **ppJob )
	{
		AUTO_LOCK( m_mutex );
		return PopLocked( ppJob );
	}

	CThreadEvent &GetEventHandle()
	{
		return m_JobAvailableEvent;
	}

	// Blocks until the queue has space or the timeout elapses; runs nothing.
	void WaitForSpace( unsigned timeoutMs ) { m_SpaceAvailableEvent.Wait( timeoutMs ); }

	// Wakes blocked producers so they re-evaluate the pool's state.
	void NotifySpaceWaiters() { m_SpaceAvailableEvent.Set(); }

	void Flush()
	{
		// Only safe to call when the system is suspended. Detach queue ownership
		// first so Abort/cleanup/destructors never run under the queue mutex.
		std::deque<CJob *> pending[JP_HIGH + 1];
		{
			AUTO_LOCK( m_mutex );
			for ( int i = JP_HIGH; i >= 0; --i )
				pending[i].swap( m_Queues[i] );
			m_nItems = 0;
			m_JobAvailableEvent.Reset();
			UpdateSpaceLocked();
		}
		for ( int i = JP_HIGH; i >= 0; --i )
		{
			for ( CJob *pJob : pending[i] )
			{
				pJob->Abort();
				pJob->Release();
			}
		}
	}

private:
	// Queue mutation and notification are one locked transition.
	void PushLocked( CJob *pJob )
	{
		pJob->AddRef();
		m_Queues[pJob->GetPriority()].push_back( pJob );
		if ( ++m_nItems == 1 )
			m_JobAvailableEvent.Set();
		if ( m_nItems > m_nPeakItems )
			m_nPeakItems = m_nItems;
		UpdateSpaceLocked();
	}

	bool PopLocked( CJob **ppJob )
	{
		for ( int i = JP_HIGH; i >= 0; --i )
		{
			if ( !m_Queues[i].empty() )
			{
				*ppJob = m_Queues[i].front();
				m_Queues[i].pop_front();
				if ( --m_nItems == 0 )
					m_JobAvailableEvent.Reset();
				UpdateSpaceLocked();
				return true;
			}
		}
		Assert( m_nItems == 0 );
		*ppJob = NULL;
		return false;
	}

	void UpdateSpaceLocked()
	{
		if ( m_nItems < m_nMaxItems )
			m_SpaceAvailableEvent.Set();
		else
			m_SpaceAvailableEvent.Reset();
	}

	std::deque<CJob *>	m_Queues[JP_HIGH + 1];
	int					m_nItems;
	int					m_nMaxItems;
	int					m_nPeakItems;
	CThreadMutex		m_mutex;
	CThreadManualEvent	m_JobAvailableEvent;
	CThreadManualEvent	m_SpaceAvailableEvent;

} ALIGN16_POST;

//-----------------------------------------------------------------------------
//
// CThreadPool
//
//-----------------------------------------------------------------------------

class CThreadPool : public CRefCounted1<IThreadPool, CRefCountServiceMT>
{
public:
	CThreadPool();
	~CThreadPool();

	//-----------------------------------------------------
	// Thread functions
	//-----------------------------------------------------
	bool Start( const ThreadPoolStartParams_t &startParams = ThreadPoolStartParams_t() ) { return Start( startParams, NULL ); }
	bool Start( const ThreadPoolStartParams_t &startParams, const char *pszNameOverride );
	bool Stop( int timeout = TT_INFINITE );
	void Distribute( bool bDistribute = true, int *pAffinityTable = NULL );

	//-----------------------------------------------------
	// Functions for any thread
	//-----------------------------------------------------
	unsigned GetJobCount() { return m_nJobs.AtomicAdd( 0 ); }
	int NumThreads();
	int NumIdleThreads();

	//-----------------------------------------------------
	// Pause/resume processing jobs
	//-----------------------------------------------------
	int SuspendExecution();
	int ResumeExecution();

	//-----------------------------------------------------
	// Offer the current thread to the pool
	//-----------------------------------------------------
	virtual int YieldWait( CThreadEvent **pEvents, int nEvents, bool bWaitAll = true, unsigned timeout = TT_INFINITE );
	virtual int YieldWait( CJob **, int nJobs, bool bWaitAll = true, unsigned timeout = TT_INFINITE );
	inline void Yield( unsigned timeout )
	{
		Assert( ThreadInMainThread() );
		ThreadSleep( timeout );
	}

	//-----------------------------------------------------
	// Add a native job to the queue (master thread)
	//-----------------------------------------------------
	void AddJob( CJob * );
	void InsertJobInQueue( CJob * );

	//-----------------------------------------------------
	// All threads execute pFunctor asap. Thread will either wake up
	//  and execute or execute pFunctor right after completing current job and
	//  before looking for another job.
	//-----------------------------------------------------
	// void ExecuteHighPriorityFunctor( CFunctor *pFunctor );

	//-----------------------------------------------------
	// Add an function object to the queue (master thread)
	//-----------------------------------------------------
	void AddFunctorInternal( CFunctor *, CJob ** = NULL, const char *pszDescription = NULL, unsigned flags = 0 );

	//-----------------------------------------------------
	// Remove a job from the queue (master thread)
	//-----------------------------------------------------
	virtual void ChangePriority( CJob *p, JobPriority_t priority );

	//-----------------------------------------------------
	// Bulk job manipulation (blocking)
	//-----------------------------------------------------
	int ExecuteToPriority( JobPriority_t toPriority, JobFilter_t pfnFilter = NULL  );
	int AbortAll();

	virtual void Reserved1() {}

private:
	enum
	{
		IO_STACKSIZE = ( 64 * 1024 ),
		COMPUTATION_STACKSIZE = 0,
	};

	//-----------------------------------------------------
	//
	//-----------------------------------------------------
	CJob *PeekJob();
	CJob *GetDummyJob();

	//-----------------------------------------------------
	// Thread functions
	//-----------------------------------------------------
	int Run();

	//-----------------------------------------------------
	// Worker scheduling (see CJobThread)
	//-----------------------------------------------------
	CJobThread *FindCurrentWorker();
	bool TakeJob( CJobThread *pWorker, CJob **ppJob );
	bool HasWorkFor( CJobThread *pWorker );
	void WakeWorkers( CJobThread *pExclude );
	bool TryExecuteWaitedJob( CJob *pJob, CJobThread *pWorker );
	bool IsEligibleWaiter( CJob *pJob, CJobThread *pWorker );
	void CheckNestedWait( CJob **ppJobs, int nJobs, CJobThread *pWorker );
	void AdmitToFullSharedQueue( CJob *pJob, CJobThread *pWorker );
	bool HasPendingWork();
	void BeginWorkerWait();
	void SpillStealDeques();
	int AbortStealDeques();
	int WaitForEvents( CThreadEvent **pEvents, int nEvents, bool bWaitAll );

private:
	friend class CJobThread;
	friend void GetThreadPoolSchedulingStats( IThreadPool *, ThreadPoolSchedulingStats_t * );
	friend void SetThreadPoolSharedQueueCapacity( IThreadPool *, int );

	CJobQueue				m_SharedQueue;
	CInterlockedInt			m_nIdleThreads;
	CUtlVector<CJobThread *> m_Threads;
	CUtlVector<CThreadEvent *>		m_IdleEvents;

	// Workers read the steal-victim set without touching m_Threads, which only
	// the controlling thread mutates. Entries [0, m_nWorkers) are published
	// before the count and stay valid until Stop has joined every worker.
	CJobThread *m_pWorkers[TP_MAX_POOL_THREADS];
	CInterlockedInt m_nWorkers;
	CInterlockedInt m_nWaitedJobsRunInline;

	// Bounded shared queue and nested-wait observations (see
	// ThreadPoolSchedulingStats_t).
	CInterlockedInt m_nSharedQueueCallerRuns;
	CInterlockedInt m_nSharedQueueBlockedAdmissions;
	CInterlockedInt m_nSharedQueueOverCapacity;
	CInterlockedInt m_nNestedWaits;
	CInterlockedInt m_nForbiddenNestedWaits;
	CInterlockedInt m_nWorkerEventWaits;
	CInterlockedInt m_nWorkersBlockedInWait;
	CInterlockedInt m_nStarvationEvents;

	CThreadMutex			m_SuspendMutex;
	CInterlockedInt			m_nSuspend; // written under m_SuspendMutex; read by producers
	CInterlockedInt			m_nJobs;

	// Some jobs should only be executed on the threadpool thread(s). Ie: the rendering thread has the GL context
	//	and the main thread coming in and "helping" with jobs breaks that pretty nicely. This flag states that
	//	only the threadpool threads should execute these jobs.
	bool					m_bExecOnThreadPoolThreadsOnly;
};

//-----------------------------------------------------------------------------

JOB_INTERFACE IThreadPool *CreateThreadPool()
{
	return new CThreadPool;
}

JOB_INTERFACE void DestroyThreadPool( IThreadPool *pPool )
{
	delete pPool;
}

// Defined after CJobThread.
JOB_INTERFACE void GetThreadPoolSchedulingStats(
    IThreadPool *pPool, ThreadPoolSchedulingStats_t *pStats );

//-----------------------------------------------------------------------------

class CGlobalThreadPool : public CThreadPool
{
public:
	virtual bool Start( const ThreadPoolStartParams_t &startParamsIn )
	{
		int nThreads = ( CommandLine()->ParmValue( "-threads", -1 ) - 1 );
		ThreadPoolStartParams_t startParams = startParamsIn;

		if ( nThreads >= 0 )
		{
			startParams.nThreads = nThreads;
		}
		else
		{
			// Cap the GlobPool threads at 4.
			startParams.nThreadsMax = 4;
		}
		return CThreadPool::Start( startParams, "Glob" );
	}

	virtual bool OnFinalRelease()
	{
		AssertMsg( 0, "Releasing global thread pool object!" );
		return false;
	}
};

//-----------------------------------------------------------------------------

// Each worker services, in order: its direct queue (thread-affine and
// JF_SERIAL work, never stolen), its own steal deque (jobs it spawned, newest
// first), the pool's shared queue, then the oldest entry of another worker's
// steal deque. A worker sleeps on its own auto-reset wake event, which every
// producer sets after publishing work the worker may take, and on its call
// event (exit/suspend). A long fallback timeout bounds the cost of any missed
// wake to latency; it is not part of the wake protocol.
class CJobThread : public CWorkerThread
{
public:
	enum
	{
		STEAL_DEQUE_CAPACITY = 256,
		WAKE_FALLBACK_MS = 1000,
	};

	CJobThread( CThreadPool *pOwner, int iThread )
	    : m_SharedQueue( pOwner->m_SharedQueue ), m_pOwner( pOwner ), m_WakeEvent( false ),
	      m_iThread( iThread ), m_nThreadId( 0 ), m_nStealDequePushes( 0 ),
	      m_nStealDequeSpills( 0 ), m_nSteals( 0 )
	{
	}

	CThreadEvent &GetIdleEvent()
	{
		return m_IdleEvent;
	}

	CJobQueue &AccessDirectQueue()
	{ 
		return m_DirectQueue;
	}

	CJobStealDeque<CJob *, STEAL_DEQUE_CAPACITY> &AccessStealDeque() { return m_StealDeque; }

	// Called after publishing work this worker may take, or after sending it
	// a call. Idempotent.
	void Wake() { m_WakeEvent.Set(); }

	bool IsCurrentThread() const
	{
		return ThreadAtomicLoad( &m_nThreadId ) == ThreadGetCurrentId();
	}

	bool JoinForShutdown()
	{
		// ThreadProc clears IsAlive before its final event signal and mutex
		// release. Wait for those publications before joining or destroying any
		// derived fields that the worker may still be using.
		if ( !m_ExitEvent.Wait() )
			return false;
		m_Lock.Lock();
		m_Lock.Unlock();
		return CThread::Join();
	}

private:
	friend class CThreadPool;
	friend void GetThreadPoolSchedulingStats( IThreadPool *, ThreadPoolSchedulingStats_t * );

	unsigned Wait()
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_IDLE, "%s", __FUNCTION__ );
#ifdef WIN32
		enum Event_t
		{
			CALL_FROM_MASTER,
			SHARED_QUEUE,
			DIRECT_QUEUE,
			WAKE,

			NUM_EVENTS
		};

		HANDLE	 waitHandles[NUM_EVENTS];
		
		waitHandles[CALL_FROM_MASTER]	= GetCallHandle().GetHandle();
		waitHandles[SHARED_QUEUE]		= m_SharedQueue.GetEventHandle().GetHandle();
		waitHandles[DIRECT_QUEUE] 		= m_DirectQueue.GetEventHandle().GetHandle();
		waitHandles[WAKE] = m_WakeEvent.GetHandle();

		for ( ;; )
		{
			if ( PeekCall() || m_pOwner->HasWorkFor( this ) )
				return WAIT_OBJECT_0;
			unsigned waitResult = WaitForMultipleObjects(
			    ARRAYSIZE( waitHandles ), waitHandles, FALSE, WAKE_FALLBACK_MS );
			if ( waitResult == WAIT_FAILED )
				return waitResult;
		}
#else
		for ( ;; )
		{
			// Check after every wake: the wake event is set after publication,
			// so work published before this check is seen here and work
			// published after it leaves the event set for the wait below.
			if ( PeekCall() || m_pOwner->HasWorkFor( this ) )
				return WAIT_OBJECT_0;
			m_WakeEvent.Wait( WAKE_FALLBACK_MS );
		}
#endif
	}

	int Run()
	{
		// Wait for either a call from the master thread, or an item in the queue...
		unsigned waitResult;
		bool	 bExit = false;

		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s", __FUNCTION__ );

		ThreadAtomicStore( &m_nThreadId, ThreadGetCurrentId() );
		m_pOwner->m_nIdleThreads++;
		m_IdleEvent.Set();
		while (!bExit && ( ( waitResult = Wait() ) != WAIT_FAILED ) )
		{
			if ( PeekCall() )
			{
				tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s PeekCall():%d", __FUNCTION__, GetCallParam() );

				switch ( GetCallParam() )
				{
				case TPM_EXIT:
					Reply( true );
					bExit = TRUE;
					break;

				case TPM_SUSPEND:
					Reply( true );
					Suspend();
					break;

				default:
					AssertMsg( 0, "Unknown call to thread" );
					Reply( false );
					break;
				}
			}
			else
			{
				tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s !PeekCall()", __FUNCTION__ );

				CJob *pJob;
				bool bTookJob = false;
				do
				{
					if ( !m_pOwner->TakeJob( this, &pJob ) )
					{
						// Nothing to process, return to wait state
						break;
					}
					if ( !bTookJob )
					{
						m_IdleEvent.Reset();
						m_pOwner->m_nIdleThreads--;
						bTookJob = true;
					}
					ServiceJobAndRelease( pJob, m_iThread );
					m_pOwner->m_nJobs--;
				} while ( !PeekCall() );

				if ( bTookJob )
				{
					m_pOwner->m_nIdleThreads++;
					m_IdleEvent.Set();
				}
			}
		}
		m_pOwner->m_nIdleThreads--;
		m_IdleEvent.Reset();
		return 0;
	}

	CJobQueue			m_DirectQueue;
	CJobStealDeque<CJob *, STEAL_DEQUE_CAPACITY> m_StealDeque;
	CJobQueue &			m_SharedQueue;
	CThreadPool *		m_pOwner;
	CThreadManualEvent	m_IdleEvent;
	CThreadEvent m_WakeEvent;
	int					m_iThread;
	volatile ThreadId_t m_nThreadId;

	// Written only by this worker's thread; read atomically by stats queries.
	static void CountEvent( volatile int *pCounter )
	{
		ThreadAtomicStore( pCounter, *pCounter + 1 );
	}
	volatile int m_nStealDequePushes;
	volatile int m_nStealDequeSpills;
	volatile int m_nSteals;
};

//-----------------------------------------------------------------------------

CGlobalThreadPool g_ThreadPool;
IThreadPool *g_pThreadPool = &g_ThreadPool;

JOB_INTERFACE void GetThreadPoolSchedulingStats(
    IThreadPool *pPool, ThreadPoolSchedulingStats_t *pStats )
{
	CThreadPool *pThreadPool = static_cast<CThreadPool *>( pPool );
	memset( pStats, 0, sizeof( *pStats ) );
	const int nWorkers = pThreadPool->m_nWorkers;
	for ( int i = 0; i < nWorkers; i++ )
	{
		CJobThread *pWorker = pThreadPool->m_pWorkers[i];
		pStats->nStealDequePushes += ThreadAtomicLoad( &pWorker->m_nStealDequePushes );
		pStats->nStealDequeSpills += ThreadAtomicLoad( &pWorker->m_nStealDequeSpills );
		pStats->nSteals += ThreadAtomicLoad( &pWorker->m_nSteals );
	}
	pStats->nWaitedJobsRunInline = pThreadPool->m_nWaitedJobsRunInline;
	pStats->nSharedQueueCapacity = pThreadPool->m_SharedQueue.Capacity();
	pStats->nSharedQueuePeak = pThreadPool->m_SharedQueue.PeakCount();
	pStats->nSharedQueueCallerRuns = pThreadPool->m_nSharedQueueCallerRuns;
	pStats->nSharedQueueBlockedAdmissions = pThreadPool->m_nSharedQueueBlockedAdmissions;
	pStats->nSharedQueueOverCapacity = pThreadPool->m_nSharedQueueOverCapacity;
	pStats->nNestedWaits = pThreadPool->m_nNestedWaits;
	pStats->nForbiddenNestedWaits = pThreadPool->m_nForbiddenNestedWaits;
	pStats->nWorkerEventWaits = pThreadPool->m_nWorkerEventWaits;
	pStats->nStarvationEvents = pThreadPool->m_nStarvationEvents;
}

JOB_INTERFACE void SetThreadPoolSharedQueueCapacity( IThreadPool *pPool, int nCapacity )
{
	Assert( nCapacity >= 1 );
	static_cast<CThreadPool *>( pPool )->m_SharedQueue.SetCapacity( MAX( nCapacity, 1 ) );
}

//-----------------------------------------------------------------------------
//
// CThreadPool
//
//-----------------------------------------------------------------------------

CThreadPool::CThreadPool()
    : m_nIdleThreads( 0 ), m_nWorkers( 0 ), m_nWaitedJobsRunInline( 0 ),
      m_nSharedQueueCallerRuns( 0 ), m_nSharedQueueBlockedAdmissions( 0 ),
      m_nSharedQueueOverCapacity( 0 ), m_nNestedWaits( 0 ), m_nForbiddenNestedWaits( 0 ),
      m_nWorkerEventWaits( 0 ), m_nWorkersBlockedInWait( 0 ), m_nStarvationEvents( 0 ),
      m_nSuspend( 0 ), m_nJobs( 0 ), m_bExecOnThreadPoolThreadsOnly( false )
{
	memset( m_pWorkers, 0, sizeof( m_pWorkers ) );
	m_SharedQueue.SetCapacity( TP_DEFAULT_SHARED_QUEUE_CAPACITY );
}

//---------------------------------------------------------

CThreadPool::~CThreadPool()
{
	Stop();
}

//---------------------------------------------------------
// 
//---------------------------------------------------------
int CThreadPool::NumThreads()
{
	return m_Threads.Count();
}

//---------------------------------------------------------
// 
//---------------------------------------------------------
int CThreadPool::NumIdleThreads()
{
	return m_nIdleThreads.AtomicAdd( 0 );
}

/*void CThreadPool::ExecuteHighPriorityFunctor( CFunctor *pFunctor )
{
	int i;
	for ( i = 0; i < m_Threads.Count(); i++ )
	{
		m_Threads[i]->CallWorker( TPM_RUNFUNCTOR, 0, false, pFunctor );
	}

	for ( i = 0; i < m_Threads.Count(); i++ )
	{
		m_Threads[i]->WaitForReply();
	}
}*/

//---------------------------------------------------------
// Pause/resume processing jobs
//---------------------------------------------------------
int CThreadPool::SuspendExecution()
{
	AUTO_LOCK( m_SuspendMutex );

	// If not already suspended
	if ( m_nSuspend == 0 )
	{
		// Make sure state is correct
		int i;
		for ( i = 0; i < m_Threads.Count(); i++ )
		{
			// Send without waiting, then wake: a sleeping worker observes its
			// call event only when woken (see CJobThread::Wait).
			m_Threads[i]->CallWorker( TPM_SUSPEND, 0 );
			m_Threads[i]->Wake();
		}

		for ( i = 0; i < m_Threads.Count(); i++ )
		{
			m_Threads[i]->WaitForReply();
		}

		// Because worker must signal before suspending, we could reach
		// here with the thread not actually suspended
		for ( i = 0; i < m_Threads.Count(); i++ )
		{
			while ( !m_Threads[i]->IsSuspended() )
			{
				ThreadSleep();
			}
		}
	}

	int result = m_nSuspend++;
	// A producer blocked on a full shared queue must now admit instead.
	m_SharedQueue.NotifySpaceWaiters();
	return result;
}

//---------------------------------------------------------

int CThreadPool::ResumeExecution()
{
	AUTO_LOCK( m_SuspendMutex );
	AssertMsg( m_nSuspend >= 1, "Attempted resume when not suspended");
	int result = m_nSuspend--;
	if (m_nSuspend == 0 )
	{
		for ( int i = 0; i < m_Threads.Count(); i++ )
		{
			m_Threads[i]->Resume();
		}
	}
	return result;
}

//---------------------------------------------------------

// Wait rule: a waiting thread never services unrelated queued work. The
// previous implementation popped arbitrary shared-queue jobs while waiting,
// which ran them at the waiter's program point (for example queued AI work
// in the middle of a host frame). A wait may only run the specific jobs it is
// waiting for, and only when they are eligible to run on this thread (see
// TryExecuteWaitedJob). Waiting on bare events therefore helps nothing.
int CThreadPool::YieldWait( CThreadEvent **pEvents, int nEvents, bool bWaitAll, unsigned timeout )
{
	tmZone( TELEMETRY_LEVEL0, TMZF_IDLE, "%s(%d) %t", __FUNCTION__, timeout,
	    tmSendCallStack( TELEMETRY_LEVEL0, 0 ) );

	// Timeouts remain unimplemented: the legacy loop always waited until the
	// events were signaled, and callers depend on that.
	Assert( timeout == TT_INFINITE );

	// A worker blocking on bare events holds a worker for a dependency the
	// pool cannot see. It is counted, not refused.
	CJobThread *pWorker = FindCurrentWorker();
	if ( !pWorker )
		return WaitForEvents( pEvents, nEvents, bWaitAll );

	m_nWorkerEventWaits++;
	BeginWorkerWait();
	int result = WaitForEvents( pEvents, nEvents, bWaitAll );
	m_nWorkersBlockedInWait--;
	return result;
}

int CThreadPool::WaitForEvents( CThreadEvent **pEvents, int nEvents, bool bWaitAll )
{
	if ( nEvents == 1 || bWaitAll )
	{
		// Blocking waits in sequence are equivalent to waiting for all, and
		// avoid the polling multi-object wait on POSIX.
		for ( int i = 0; i < nEvents; i++ )
		{
			pEvents[i]->Wait();
		}
		return 0;
	}

	int result;
	while ( ( result = CThreadEvent::WaitForMultiple( nEvents, pEvents, false, TT_INFINITE ) ) ==
	        TW_TIMEOUT )
	{
	}
	return result;
}

//---------------------------------------------------------

// Whether the waiting thread may run pJob itself. pWorker is the waiter's
// worker in this pool, or NULL for any other thread. The job must belong to
// this pool; a thread that is not one of its workers is eligible only when the
// pool allows that (m_bExecOnThreadPoolThreadsOnly) and the job has no thread
// requirement; a worker is eligible for unbound jobs and for jobs bound to it
// (its service thread, or JF_SERIAL work for worker 0). Call with the job
// mutex held: a job's service thread is written under it.
bool CThreadPool::IsEligibleWaiter( CJob *pJob, CJobThread *pWorker )
{
	if ( pJob->m_pThreadPool != this )
		return false;
	const int iRequired = ( pJob->GetFlags() & JF_SERIAL ) ? 0 : pJob->GetServiceThread();
	if ( !pWorker )
		return !m_bExecOnThreadPoolThreadsOnly && iRequired == -1;
	return iRequired == -1 || iRequired == pWorker->m_iThread;
}

// Runs a waited job on the waiting thread when no thread has started it and
// the waiter is eligible (IsEligibleWaiter). Its queue entry stays admitted
// and is released (not rerun) by the worker that later pops it.
bool CThreadPool::TryExecuteWaitedJob( CJob *pJob, CJobThread *pWorker )
{
	if ( pJob->m_pThreadPool != this || !pJob->CanExecute() )
		return false;

	// TryLock fails only if another thread is executing or aborting it.
	if ( !pJob->TryLock() )
		return false;

	bool bExecuted = false;
	if ( pJob->CanExecute() && IsEligibleWaiter( pJob, pWorker ) )
	{
		pJob->Execute();
		m_nWaitedJobsRunInline++;
		bExecuted = true;
	}
	pJob->Unlock();
	return bExecuted;
}

//---------------------------------------------------------
// Nested waits. A worker that waits inside a job holds a worker while it
// waits. Waiting for a job another thread is already running is allowed.
// Waiting for a job of this pool that no thread has started and that the
// waiter cannot run itself (it is bound to another worker) is a forbidden
// nested wait: the graph contract never implements dependencies by blocking
// pool jobs, and such a wait deadlocks if the job's worker is (transitively)
// waiting for the waiter. Forbidden waits are detected and counted, and warned
// about once; the wait still proceeds as the legacy contract requires. When
// every worker is blocked in a wait while queued work remains, the pool can
// make no progress: that is counted and warned about as starvation.
//---------------------------------------------------------

void CThreadPool::CheckNestedWait( CJob **ppJobs, int nJobs, CJobThread *pWorker )
{
	m_nNestedWaits++;
	for ( int i = 0; i < nJobs; i++ )
	{
		CJob *pJob = ppJobs[i];
		if ( pJob->m_pThreadPool != this || !pJob->CanExecute() || !pJob->TryLock() )
			continue; // another pool's job, finished, or running
		const bool bForbidden = pJob->CanExecute() && !IsEligibleWaiter( pJob, pWorker );
		pJob->Unlock();
		if ( bForbidden )
		{
			static CInterlockedInt s_nWarned( 0 );
			if ( m_nForbiddenNestedWaits++ == 0 && s_nWarned++ == 0 )
			{
				Warning( "Thread pool worker %d waits for unstarted job '%s' bound to another "
				         "thread (forbidden nested wait)\n",
				    pWorker->m_iThread, pJob->Describe() );
			}
		}
	}
}

// Marks the calling worker blocked in a wait and reports starvation.
void CThreadPool::BeginWorkerWait()
{
	if ( ++m_nWorkersBlockedInWait == m_nWorkers && HasPendingWork() )
	{
		static CInterlockedInt s_nWarned( 0 );
		if ( m_nStarvationEvents++ == 0 && s_nWarned++ == 0 )
		{
			Warning( "Thread pool starvation: all %d workers are blocked in waits with queued "
			         "work\n",
			    (int)m_nWorkers );
		}
	}
}

bool CThreadPool::HasPendingWork()
{
	if ( m_SharedQueue.Count() )
		return true;
	const int nWorkers = m_nWorkers;
	for ( int i = 0; i < nWorkers; i++ )
	{
		if ( m_pWorkers[i]->AccessDirectQueue().Count() ||
		     m_pWorkers[i]->AccessStealDeque().Count() )
		{
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------

int CThreadPool::YieldWait( CJob **ppJobs, int nJobs, bool bWaitAll, unsigned timeout )
{
	CUtlVectorFixed<CThreadEvent *, 64> handles;
	if ( nJobs > handles.NumAllocated() - 2 )
	{
		return TW_FAILED;
	}

	CJobThread *pWorker = FindCurrentWorker();
	for ( int i = 0; i < nJobs; i++ )
	{
		if ( TryExecuteWaitedJob( ppJobs[i], pWorker ) && !bWaitAll )
			break;
	}

	for ( int i = 0; i < nJobs; i++ )
	{
		handles.AddToTail( ppJobs[i]->AccessEvent() );
	}

	Assert( timeout == TT_INFINITE );
	if ( !pWorker )
		return WaitForEvents( handles.Base(), handles.Count(), bWaitAll );

	CheckNestedWait( ppJobs, nJobs, pWorker );
	BeginWorkerWait();
	int result = WaitForEvents( handles.Base(), handles.Count(), bWaitAll );
	m_nWorkersBlockedInWait--;
	return result;
}

//---------------------------------------------------------
// Add a job to the queue
//---------------------------------------------------------

void CThreadPool::AddJob( CJob *pJob )
{
	if ( !pJob )
	{
		return;
	}

	if ( pJob->m_ThreadPoolData != JOB_NO_DATA )
	{
		Warning( "Cannot add a thread job already committed to another thread pool\n" );
		return;
	}

	if ( m_Threads.Count() == 0 )
	{
		// So only threadpool jobs are supposed to execute the jobs, but there are no threadpool threads?
		Assert( !m_bExecOnThreadPoolThreadsOnly );

		pJob->Execute();
		return;
	}

	int flags = pJob->GetFlags();

	if ( !m_bExecOnThreadPoolThreadsOnly && ( ( flags & ( JF_IO | JF_QUEUE ) ) == 0 ) /* @TBD && !m_queue.Count() */ )
	{
		if ( !NumIdleThreads() )
		{
			pJob->Execute();
			return;
		}
		pJob->SetPriority( JP_HIGH );
	}


	if ( !pJob->CanExecute() )
	{
		// Already handled
		ExecuteOnce( Warning( "Attempted to add job to job queue that has already been completed\n" ) );
		return;
	}

	pJob->m_pThreadPool = this;
	pJob->SetStatus( JOB_STATUS_PENDING );
	// Count admission before queue publication: a worker may finish as soon
	// as Push signals the queue, and must not decrement an uncounted job.
	++m_nJobs;
	InsertJobInQueue( pJob );
}

//---------------------------------------------------------
//
//---------------------------------------------------------

void CThreadPool::InsertJobInQueue( CJob *pJob )
{
	// Thread-affine and JF_SERIAL work goes to that worker's direct queue,
	// which is unbounded and never stolen.
	CJobThread *pDirectWorker = NULL;
	if ( pJob->GetFlags() & JF_SERIAL )
	{
		pDirectWorker = m_Threads[0];
	}
	else
	{
		int iThread = pJob->GetServiceThread();
		if ( iThread != -1 && m_Threads.IsValidIndex( iThread ) )
			pDirectWorker = m_Threads[iThread];
	}
	if ( pDirectWorker )
	{
		pDirectWorker->AccessDirectQueue().ForcePush( pJob );
		pDirectWorker->Wake();
		return;
	}

	// Work spawned by one of this pool's workers goes to that worker's
	// bounded steal deque; when it is full, the shared queue takes it.
	CJobThread *pWorker = FindCurrentWorker();
	if ( pWorker )
	{
		pJob->AddRef(); // the deque entry's reference
		if ( pWorker->AccessStealDeque().PushBottom( pJob ) )
		{
			CJobThread::CountEvent( &pWorker->m_nStealDequePushes );
			if ( NumIdleThreads() )
				WakeWorkers( pWorker );
			return;
		}
		pJob->Release();
		CJobThread::CountEvent( &pWorker->m_nStealDequeSpills );
	}

	if ( !m_SharedQueue.TryPush( pJob ) )
	{
		AdmitToFullSharedQueue( pJob, pWorker );
		return;
	}
	WakeWorkers( NULL );
}

//---------------------------------------------------------
// Overflow policy for the bounded shared queue (RFC 0006 "Ring buffers and
// bounded queues"). Required work is never dropped, overwritten or run on a
// thread that did not submit it:
//  - A worker of this pool runs the job it is submitting itself (caller
//    runs). Blocking a worker on queue space could stall every worker.
//  - Any other thread blocks until the queue has space. The wait runs no job
//    and holds no pool lock.
//  - While the pool cannot consume (suspended, or no running workers), the
//    job is admitted beyond capacity instead of blocking forever; each such
//    admission is counted.
//---------------------------------------------------------

void CThreadPool::AdmitToFullSharedQueue( CJob *pJob, CJobThread *pWorker )
{
	if ( pWorker )
	{
		m_nSharedQueueCallerRuns++;
		pJob->AddRef(); // the reference a queue entry would have held
		ServiceJobAndRelease( pJob, pWorker->m_iThread );
		m_nJobs--;
		return;
	}

	m_nSharedQueueBlockedAdmissions++;
	for ( ;; )
	{
		if ( m_nSuspend != 0 || m_nWorkers == 0 )
		{
			m_SharedQueue.ForcePush( pJob );
			m_nSharedQueueOverCapacity++;
			break;
		}
		// Workers were woken when the queue filled; the timeout bounds the
		// time to notice suspension, not the wake protocol.
		m_SharedQueue.WaitForSpace( 100 );
		if ( m_SharedQueue.TryPush( pJob ) )
			break;
	}
	WakeWorkers( NULL );
}

//---------------------------------------------------------
// Worker scheduling
//---------------------------------------------------------

CJobThread *CThreadPool::FindCurrentWorker()
{
	const int nWorkers = m_nWorkers;
	for ( int i = 0; i < nWorkers; i++ )
	{
		if ( m_pWorkers[i]->IsCurrentThread() )
			return m_pWorkers[i];
	}
	return NULL;
}

bool CThreadPool::TakeJob( CJobThread *pWorker, CJob **ppJob )
{
	if ( pWorker->AccessDirectQueue().Pop( ppJob ) )
		return true;
	if ( pWorker->AccessStealDeque().PopBottom( ppJob ) )
		return true;
	if ( m_SharedQueue.Pop( ppJob ) )
		return true;

	// Steal the oldest entry of another worker, starting after this one so
	// that thieves spread across victims.
	const int nWorkers = m_nWorkers;
	for ( int i = 1; i < nWorkers; i++ )
	{
		CJobThread *pVictim = m_pWorkers[( pWorker->m_iThread + i ) % nWorkers];
		if ( pVictim->AccessStealDeque().StealTop( ppJob ) )
		{
			CJobThread::CountEvent( &pWorker->m_nSteals );
			return true;
		}
	}
	return false;
}

bool CThreadPool::HasWorkFor( CJobThread *pWorker )
{
	if ( pWorker->AccessDirectQueue().Count() || pWorker->AccessStealDeque().Count() ||
	     m_SharedQueue.Count() )
	{
		return true;
	}

	const int nWorkers = m_nWorkers;
	for ( int i = 0; i < nWorkers; i++ )
	{
		if ( m_pWorkers[i] != pWorker && m_pWorkers[i]->AccessStealDeque().Count() )
			return true;
	}
	return false;
}

void CThreadPool::WakeWorkers( CJobThread *pExclude )
{
	const int nWorkers = m_nWorkers;
	for ( int i = 0; i < nWorkers; i++ )
	{
		if ( m_pWorkers[i] != pExclude )
			m_pWorkers[i]->Wake();
	}
}

// Quiescent only (suspended or stopped): return every steal-deque entry to the
// shared queue so priority-ordered bulk operations see it.
void CThreadPool::SpillStealDeques()
{
	for ( int i = 0; i < m_Threads.Count(); i++ )
	{
		CJob *pJob;
		while ( m_Threads[i]->AccessStealDeque().StealTop( &pJob ) )
		{
			m_SharedQueue.ForcePush( pJob );
			pJob->Release(); // the shared queue holds its own reference
		}
	}
}

// Quiescent only: abort and release every steal-deque entry.
int CThreadPool::AbortStealDeques()
{
	int nAborted = 0;
	for ( int i = 0; i < m_Threads.Count(); i++ )
	{
		CJob *pJob;
		while ( m_Threads[i]->AccessStealDeque().StealTop( &pJob ) )
		{
			pJob->Abort();
			pJob->Release();
			nAborted++;
		}
	}
	return nAborted;
}

//---------------------------------------------------------
// Add an function object to the queue (master thread)
//---------------------------------------------------------

void CThreadPool::AddFunctorInternal( CFunctor *pFunctor, CJob **ppJob, const char *pszDescription, unsigned flags )
{
	// Note: assumes caller has handled refcount
	CJob *pJob = new CFunctorJob( pFunctor, pszDescription );

	pJob->SetFlags( flags );

	AddJob( pJob );

	if ( ppJob )
	{
		*ppJob = pJob;
	}
	else
	{
		pJob->Release();
	}
}

//---------------------------------------------------------
// Remove a job from the queue
//---------------------------------------------------------

void CThreadPool::ChangePriority( CJob *pJob, JobPriority_t priority )
{
	// Right now, only support upping the priority
	if ( pJob->GetPriority() < priority )
	{
		pJob->SetPriority( priority );
		// A re-insertion of admitted work: the earlier entry is released
		// unrun when popped, so this does not count against admission.
		m_SharedQueue.ForcePush( pJob );
		WakeWorkers( NULL );
	}
	else
	{
		ExecuteOnce( if ( pJob->GetPriority() != priority ) DevMsg( "CThreadPool::RemoveJob not implemented right now" ) );
	}

}

//---------------------------------------------------------
// Execute to a specified priority
//---------------------------------------------------------

int CThreadPool::ExecuteToPriority( JobPriority_t iToPriority, JobFilter_t pfnFilter )
{
	SuspendExecution();
	SpillStealDeques();

	CJob *pJob;
	int nExecuted = 0;
	int i;
	int nJobsTotal = GetJobCount();
	CUtlVector<CJob *> jobsToPutBack;

	for ( int iCurPriority = JP_HIGH; iCurPriority >= iToPriority; --iCurPriority )
	{
		for ( i = 0; i < m_Threads.Count(); i++ )
		{
			CJobQueue &queue = m_Threads[i]->AccessDirectQueue();
			while ( queue.Count( (JobPriority_t)iCurPriority ) )
			{
				queue.Pop( &pJob );
				if ( pfnFilter && !(*pfnFilter)( pJob ) )
				{
					if ( pJob->CanExecute() )
					{
						jobsToPutBack.EnsureCapacity( nJobsTotal );
						jobsToPutBack.AddToTail( pJob );
					}
					else
					{
						m_nJobs--;
						pJob->Release(); // an already serviced job in queue, may as well ditch it (as in, main thread probably force executed)
					}
					continue;
				}
				ServiceJobAndRelease( pJob );
				m_nJobs--;
				nExecuted++;
			}

		}

		while ( m_SharedQueue.Count( (JobPriority_t)iCurPriority ) )
		{
			m_SharedQueue.Pop( &pJob );
			if ( pfnFilter && !(*pfnFilter)( pJob ) )
			{
				if ( pJob->CanExecute() )
				{
					jobsToPutBack.EnsureCapacity( nJobsTotal );
					jobsToPutBack.AddToTail( pJob );
				}
				else
				{
					m_nJobs--;
					pJob->Release(); // see above
				}
				continue;
			}

			ServiceJobAndRelease( pJob );
			m_nJobs--;
			nExecuted++;
		}
	}

	for ( i = 0; i < jobsToPutBack.Count(); i++ )
	{
		InsertJobInQueue( jobsToPutBack[i] );
		jobsToPutBack[i]->Release();
	}

	ResumeExecution();

	return nExecuted;
}

//---------------------------------------------------------
//
//---------------------------------------------------------

int CThreadPool::AbortAll()
{
	SuspendExecution();
	CJob *pJob;

	int iAborted = 0;
	while ( m_SharedQueue.Pop( &pJob ) )
	{
		pJob->Abort();
		pJob->Release();
		iAborted++;
	}

	for ( int i = 0; i < m_Threads.Count(); i++ )
	{
		CJobQueue &queue = m_Threads[i]->AccessDirectQueue();
		while ( queue.Pop( &pJob ) )
		{
			pJob->Abort();
			pJob->Release();
			iAborted++;
		}

	}
	iAborted += AbortStealDeques();

	m_nJobs = 0;

	ResumeExecution();

	return iAborted;
}

//---------------------------------------------------------
// CThreadPool thread functions
//---------------------------------------------------------

bool CThreadPool::Start( const ThreadPoolStartParams_t &startParams, const char *pszName )
{
	int nThreads = startParams.nThreads;

	m_bExecOnThreadPoolThreadsOnly = startParams.bExecOnThreadPoolThreadsOnly;

	if ( nThreads < 0 )
	{
		const CPUInformation &ci = *GetCPUInformation();
		if ( startParams.bIOThreads )
		{
			nThreads = ci.m_nLogicalProcessors;
		}
		else
		{
			nThreads = ( ci.m_nLogicalProcessors / (( ci.m_bHT ) ? 2 : 1) ) - 1; // One per
			if ( IsPC() )
			{
				if ( nThreads > 3 )
				{
					DevMsg( "Defaulting to limit of 3 worker threads, use -threads on command line if want more\n" ); // Current >4 processor configs don't really work so well, probably due to cache issues? (toml 7/12/2007)
					nThreads = 3;
				}
			}
		}

		if ( ( startParams.nThreadsMax >= 0 ) && ( nThreads > startParams.nThreadsMax ) )
		{
			nThreads = startParams.nThreadsMax;
		}
	}

	if ( nThreads <= 0 )
	{
		return true;
	}

	if ( nThreads > TP_MAX_POOL_THREADS )
	{
		nThreads = TP_MAX_POOL_THREADS;
	}

	int nStackSize = startParams.nStackSize;

	if ( nStackSize < 0 )
	{
		if ( startParams.bIOThreads )
		{
			nStackSize = IO_STACKSIZE;
		}
		else
		{
			nStackSize = COMPUTATION_STACKSIZE;
		}
	}

	int priority = startParams.iThreadPriority;

	if ( priority == SHRT_MIN )
	{
		if ( startParams.bIOThreads )
		{
			priority = THREAD_PRIORITY_HIGHEST;
		}
		else
		{
			priority = ThreadGetPriority();
		}
	}

	bool bDistribute;
	if ( startParams.fDistribute != TRS_NONE )
	{
		bDistribute = ( startParams.fDistribute == TRS_TRUE );
	}
	else
	{
		bDistribute = !startParams.bIOThreads;
	}

	//--------------------------------------------------------

	m_Threads.EnsureCapacity( nThreads );
	m_IdleEvents.EnsureCapacity( nThreads );

	if ( !pszName )
	{
		pszName = ( startParams.bIOThreads ) ? "IOJobX" : "CmpJobX";
	}
	while ( nThreads-- )
	{
		int iThread = m_Threads.AddToTail();
		m_IdleEvents.AddToTail();
		m_Threads[iThread] = new CJobThread( this, iThread );
		m_IdleEvents[iThread] = &m_Threads[iThread]->GetIdleEvent();
		m_Threads[iThread]->SetName( CFmtStr( "%s%d", pszName, iThread ) );
		// Publish the victim before the count; running workers may steal from
		// it once they observe the count.
		m_pWorkers[iThread] = m_Threads[iThread];
		m_nWorkers = iThread + 1;
		m_Threads[iThread]->Start( nStackSize );
		m_Threads[iThread]->GetIdleEvent().Wait();
#ifdef WIN32
		ThreadSetPriority( (ThreadHandle_t)m_Threads[iThread]->GetThreadHandle(), priority );
#endif
	}

	Distribute( bDistribute, startParams.bUseAffinityTable ? (int *)startParams.iAffinityTable : NULL );

	return true;
}

//---------------------------------------------------------

void CThreadPool::Distribute( bool bDistribute, int *pAffinityTable )
{
	if ( bDistribute )
	{
		const CPUInformation &ci = *GetCPUInformation();
		int nHwThreadsPer = (( ci.m_bHT ) ? 2 : 1);
		if ( ci.m_nLogicalProcessors > 1 )
		{
			if ( !pAffinityTable )
			{
#if defined( IS_WINDOWS_PC )
				// no affinity table, distribution is cycled across all available
				HINSTANCE hInst = LoadLibrary( "kernel32.dll" );
				if ( hInst )
				{
					typedef DWORD (WINAPI *SetThreadIdealProcessorFn)(ThreadHandle_t hThread, DWORD dwIdealProcessor);
					SetThreadIdealProcessorFn Thread_SetIdealProcessor = (SetThreadIdealProcessorFn)GetProcAddress( hInst, "SetThreadIdealProcessor" );
					if ( Thread_SetIdealProcessor )
					{
						ThreadHandle_t hMainThread = ThreadGetCurrentHandle();
						Thread_SetIdealProcessor( hMainThread, 0 );
						int iProc = 0;
						for ( int i = 0; i < m_Threads.Count(); i++ )
						{
							iProc += nHwThreadsPer;
							if ( iProc >= ci.m_nLogicalProcessors )
							{
								iProc %= ci.m_nLogicalProcessors;
								if ( nHwThreadsPer > 1 )
								{
									iProc = ( iProc + 1 ) % nHwThreadsPer;
								}
							}
							Thread_SetIdealProcessor((ThreadHandle_t)m_Threads[i]->GetThreadHandle(), iProc);
						}
					}
					FreeLibrary( hInst );
				}
#else
				// no affinity table, distribution is cycled across all available
				int iProc = 0;
				for ( int i = 0; i < m_Threads.Count(); i++ )
				{
					iProc += nHwThreadsPer;
					if ( iProc >= ci.m_nLogicalProcessors )
					{
						iProc %= ci.m_nLogicalProcessors;
						if ( nHwThreadsPer > 1 )
						{
							iProc = ( iProc + 1 ) % nHwThreadsPer;
						}
					}
#ifdef WIN32
					ThreadSetAffinity( (ThreadHandle_t)m_Threads[i]->GetThreadHandle(), 1 << iProc );
#endif
				}
#endif
			}
			else
			{
				// distribution is from affinity table
				for ( int i = 0; i < m_Threads.Count(); i++ )
				{
#ifdef WIN32
					ThreadSetAffinity( (ThreadHandle_t)m_Threads[i]->GetThreadHandle(), pAffinityTable[i] );
#endif
				}
			}
		}
	}
	else
	{
#ifdef WIN32
		DWORD_PTR dwProcessAffinity, dwSystemAffinity;
		if ( GetProcessAffinityMask( GetCurrentProcess(), &dwProcessAffinity, &dwSystemAffinity ) )
		{
			for ( int i = 0; i < m_Threads.Count(); i++ )
			{
				ThreadSetAffinity( (ThreadHandle_t)m_Threads[i]->GetThreadHandle(), dwProcessAffinity );
			}
		}
#endif
	}
}

//---------------------------------------------------------

bool CThreadPool::Stop( int timeout )
{
	// Retain the legacy synchronous stop protocol: timeout was not honored by
	// CallWorker or the old IsAlive spin. An exit acknowledgment is not a join.
	for ( int i = 0; i < m_Threads.Count(); i++ )
	{
		// Send, wake, then wait for the acknowledgment (see CJobThread::Wait).
		m_Threads[i]->CallWorker( TPM_EXIT, 0 );
		m_Threads[i]->Wake();
		m_Threads[i]->WaitForReply();
	}

	for ( int i = 0; i < m_Threads.Count(); ++i )
	{
		if ( !m_Threads[i]->JoinForShutdown() )
			return false;
	}
	AbortStealDeques();
	m_nWorkers = 0;
	for ( int i = 0; i < m_Threads.Count(); ++i )
	{
		m_Threads[i]->AccessDirectQueue().Flush();
		m_pWorkers[i] = NULL;
		delete m_Threads[i];
	}

	m_nJobs = 0;
	m_SharedQueue.Flush();
	m_nIdleThreads = 0;
	m_Threads.RemoveAll();
	m_IdleEvents.RemoveAll();

	return true;
}

//---------------------------------------------------------

CJob *CThreadPool::GetDummyJob()
{
	class CDummyJob : public CJob
	{
	public:
		CDummyJob()
		{
			Execute();
		}

		virtual JobStatus_t DoExecute() { return JOB_OK; }
	};

	static CDummyJob dummyJob;

	dummyJob.AddRef();
	return &dummyJob;
}

//-----------------------------------------------------------------------------


namespace ThreadPoolTest 
{
int g_iSleep;

CThreadEvent g_done;
int g_nTotalToComplete;
CThreadPool *g_pTestThreadPool;

class CCountJob : public CJob
{
public:
	virtual JobStatus_t DoExecute()
	{
		m_nCount++;
		ThreadPause();
		if ( g_iSleep >= 0)
			ThreadSleep( g_iSleep );
		if ( bDoWork )
		{
			byte pMemory[1024];
			int i;
			for ( i = 0; i < 1024; i++ )
			{
				pMemory[i] = rand();
			}
			for ( i = 0; i < 50; i++ )
			{
				sqrt( (float)HashBlock( pMemory, 1024 ) + HashBlock( pMemory, 1024 ) + 10.0 );
			}
			bDoWork = false;
		}
		if ( m_nCount == g_nTotalToComplete )
			g_done.Set();
		return 0;
	}

	static CInterlockedInt m_nCount;
	bool bDoWork;
};
CInterlockedInt CCountJob::m_nCount;
int g_nTotalAtFinish;

void Test( bool bDistribute, bool bSleep = true, bool bFinishExecute = false, bool bDoWork = false )
{
	for ( int bInterleavePushPop = 0; bInterleavePushPop < 2; bInterleavePushPop++ )
	{
		for ( g_iSleep = -10; g_iSleep <= 10; g_iSleep += 10 )
		{
			Msg( "ThreadPoolTest:         Testing! Sleep %d, interleave %d \n", g_iSleep, bInterleavePushPop );
			int nMaxThreads =  8;
			int nIncrement =  2;
			for ( int i = 1; i <= nMaxThreads; i += nIncrement )
			{
				CCountJob::m_nCount = 0;
				g_nTotalAtFinish = 0;
				ThreadPoolStartParams_t params;
				params.nThreads = i;
				params.fDistribute = ( bDistribute) ? TRS_TRUE : TRS_FALSE;
				g_pTestThreadPool->Start( params, "Tst" );
				if ( !bInterleavePushPop )
				{
					g_pTestThreadPool->SuspendExecution();
				}

				CCountJob jobs[4000];
				g_nTotalToComplete = ARRAYSIZE(jobs);

				CFastTimer timer, suspendTimer;

				suspendTimer.Start();
				timer.Start();
				for ( int j = 0; j < ARRAYSIZE(jobs); j++ )
				{
					jobs[j].SetFlags( JF_QUEUE );
					jobs[j].bDoWork = bDoWork;
					g_pTestThreadPool->AddJob( &jobs[j] );
					if ( bSleep && j % 16 == 0 )
					{
						ThreadSleep( 0 );
					}
				}
				if ( !bInterleavePushPop )
				{
					g_pTestThreadPool->ResumeExecution();
				}
				if ( bFinishExecute && g_iSleep <= 1 )
				{
					g_done.Wait();
				}
				g_nTotalAtFinish = CCountJob::m_nCount;
				timer.End();
				g_pTestThreadPool->SuspendExecution();
				suspendTimer.End();
				g_pTestThreadPool->ResumeExecution();
				g_pTestThreadPool->Stop();
				g_done.Reset();

				int counts[8] = { 0 };
				for ( int j = 0; j < ARRAYSIZE(jobs); j++ )
				{
					if ( jobs[j].GetServiceThread() != -1 )
					{
						counts[jobs[j].GetServiceThread()]++;
						jobs[j].ClearServiceThread();
					}
				}

				Msg( "ThreadPoolTest:         %d threads -- %d (%d) jobs processed in %fms, %fms to suspend (%f/%f) [%d, %d, %d, %d, %d, %d, %d, %d]\n", 
					i, g_nTotalAtFinish, (int)CCountJob::m_nCount, timer.GetDuration().GetMillisecondsF(), suspendTimer.GetDuration().GetMillisecondsF() - timer.GetDuration().GetMillisecondsF(),
					timer.GetDuration().GetMillisecondsF() / (float)CCountJob::m_nCount, (suspendTimer.GetDuration().GetMillisecondsF())/(float)g_nTotalAtFinish,
					counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], counts[6], counts[7] );
			}
		}
	}
}


bool g_bOutputError;
volatile int g_ReadyToExecute;
CInterlockedInt g_nReady;

class CExecuteTestJob : public CJob
{
public:
	virtual JobStatus_t DoExecute()
	{
		byte pMemory[1024];
		int i;
		for ( i = 0; i < 1024; i++ )
		{
			pMemory[i] = rand();
		}
		for ( i = 0; i < 50; i++ )
		{
			sqrt( (float)HashBlock( pMemory, 1024 ) + HashBlock( pMemory, 1024 ) + 10.0 );
		}
		if ( AccessEvent()->Check() || IsFinished() )
		{
			if ( !g_bOutputError )
			{
				Msg( "Forced execute test failed!\n" );
				DebuggerBreakIfDebugging();
			}
		}
		return 0;
	}
};

class CExecuteTestExecuteJob : public CJob
{
public:
	virtual JobStatus_t DoExecute()
	{
		bool bAbort = ( RandomInt(  1, 10 ) == 1 );
		g_nReady++;
		while ( !g_ReadyToExecute )
		{
			ThreadPause();
		}

		if ( !bAbort )
			m_pTestJob->Execute();
		else
			m_pTestJob->Abort();
		g_nReady--;
		return 0;
	}

	CExecuteTestJob *m_pTestJob;
};


void TestForcedExecute()
{
	Msg( "TestForcedExecute\n" );
	for ( int tests = 0; tests < 30; tests++ )
	{
		for ( int i = 1; i <= 5; i += 2 )
		{
			g_nReady = 0;
			ThreadPoolStartParams_t params;
			params.nThreads = i;
			params.fDistribute = TRS_TRUE;
			g_pTestThreadPool->Start( params, "Tst" );

			static CExecuteTestJob jobs[4000];
			for ( int j = 0; j < ARRAYSIZE(jobs); j++ )
			{
				g_ReadyToExecute = false;
				for ( int k = 0; k < i; k++ )
				{
					CExecuteTestExecuteJob *pJob = new CExecuteTestExecuteJob;
					pJob->SetFlags( JF_QUEUE );
					pJob->m_pTestJob = &jobs[j];
					g_pTestThreadPool->AddJob( pJob );
					pJob->Release();
				}
				while ( g_nReady < i )
				{
					ThreadPause();
				}
				g_ReadyToExecute = true;
				ThreadSleep();
				jobs[j].Execute();
				while ( g_nReady > 0 )
				{
					ThreadPause();
				}
			}
			g_pTestThreadPool->Stop();
		}
	}
	Msg( "TestForcedExecute DONE\n" );
}

} // namespace ThreadPoolTest

void RunThreadPoolTests()
{
	CThreadPool pool;
	ThreadPoolTest::g_pTestThreadPool = &pool;
	{
		// RunTSQueueTests(10000);
		// RunTSListTests(10000);
	}

#ifdef _WIN32
	DWORD_PTR mask1 = 0;
	--mask1;
	DWORD_PTR mask2 = 0;
	--mask2;
	GetProcessAffinityMask( GetCurrentProcess(), &mask1, &mask2 );
#else
	int32 mask1=-1;
#endif
	Msg( "ThreadPoolTest: Job distribution speed\n" );
	for ( int i = 0; i < 2; i++ )
	{
		bool bToCompletion = ( i % 2 != 0 );
		{
			Msg( "ThreadPoolTest:     Non-distribute\n" );
			ThreadPoolTest::Test( false, true, bToCompletion );
		}

		Msg( "ThreadPoolTest:     Distribute\n" );
		ThreadPoolTest::Test( true, true, bToCompletion  );

		Msg( "ThreadPoolTest:     One core\n" );
		ThreadSetAffinity( 0, 1 );
		ThreadPoolTest::Test( false, true, bToCompletion  );
		ThreadSetAffinity( 0, mask1 );

		Msg( "ThreadPoolTest:     NO Sleep\n" );
		ThreadPoolTest::Test( false, false, bToCompletion  );

		Msg( "ThreadPoolTest:     Distribute\n" );
		ThreadPoolTest::Test( true, false, bToCompletion  );

		Msg( "ThreadPoolTest:     One core\n" );
		ThreadSetAffinity( 0, 1 );
		ThreadPoolTest::Test( false, false, bToCompletion  );
		ThreadSetAffinity( 0, mask1 );
	}

	Msg( "ThreadPoolTest: Jobs doing work\n" );
	for ( int i = 0; i < 2; i++ )
	{
		bool bToCompletion = true;// = ( i % 2 != 0 );
		{
			Msg( "ThreadPoolTest:     Non-distribute\n" );
			ThreadPoolTest::Test( false, true, bToCompletion, true );
		}

		Msg( "ThreadPoolTest:     Distribute\n" );
		ThreadPoolTest::Test( true, true, bToCompletion, true );

		Msg( "ThreadPoolTest:     One core\n" );
		ThreadSetAffinity( 0, 1 );
		ThreadPoolTest::Test( false, true, bToCompletion, true  );
		ThreadSetAffinity( 0, mask1 );

		Msg( "ThreadPoolTest:     NO Sleep\n" );
		ThreadPoolTest::Test( false, false, bToCompletion, true  );

		Msg( "ThreadPoolTest:     Distribute\n" );
		ThreadPoolTest::Test( true, false, bToCompletion, true  );

		Msg( "ThreadPoolTest:     One core\n" );
		ThreadSetAffinity( 0, 1 );
		ThreadPoolTest::Test( false, false, bToCompletion, true  );
		ThreadSetAffinity( 0, mask1 );
	}
#ifdef _WIN32
	GetProcessAffinityMask( GetCurrentProcess(), &mask1, &mask2 );
#endif

	ThreadPoolTest::TestForcedExecute();
}
