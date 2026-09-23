//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Linux host-tool implementation of the legacy compile-tool thread API.
//
//=============================================================================//

#include "cmdlib.h"
#define NO_THREAD_NAMES
#include "threads.h"
#include "pacifier.h"

#include <algorithm>
#include <cerrno>
#include <condition_variable>
#include <mutex>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>
#include <vector>

int numthreads = -1;
bool g_bLowPriorityThreads = false;

namespace
{
std::recursive_mutex g_WorkLock;
std::vector<std::thread> g_Workers;
ThreadWorkerFn g_WorkFunction = nullptr;
int g_Dispatch = 0;
int g_WorkCount = 0;
bool g_Pacifier = false;
bool g_Threaded = false;
bool g_LockHeld = false;
struct WorkerStartGate
{
	std::mutex mutex;
	std::condition_variable ready;
	bool released = false;
	bool cancelled = false;
};
WorkerStartGate g_StartGate;

void SetWorkerPriority( ERunThreadsPriority priority )
{
	if ( priority == k_eRunThreadsPriority_Normal ||
	     ( priority == k_eRunThreadsPriority_UseGlobalState && !g_bLowPriorityThreads ) )
		return;
	const int niceValue = priority == k_eRunThreadsPriority_Idle ? 19 : 10;
	const auto tid = static_cast<id_t>( syscall( SYS_gettid ) );
	if ( setpriority( PRIO_PROCESS, tid, niceValue ) != 0 )
		Warning( "Could not lower compile-tool worker priority: %d\n", errno );
}

void RunIndividualWorker( int iThread, void * )
{
	for ( int work = GetThreadWork(); work != -1; work = GetThreadWork() )
		g_WorkFunction( iThread, work );
}
} // namespace

void SetLowPriority()
{
	if ( setpriority( PRIO_PROCESS, 0, 19 ) != 0 )
		Warning( "Could not lower compile-tool priority: %d\n", errno );
}

void ThreadSetDefault()
{
	if ( numthreads == -1 )
	{
		const unsigned cores = std::thread::hardware_concurrency();
		numthreads = std::min<unsigned>( cores ? cores : 1, MAX_TOOL_THREADS );
	}
	Msg( "%i threads\n", numthreads );
}

void ThreadLock()
{
	if ( !g_Threaded )
		return;
	g_WorkLock.lock();
	if ( g_LockHeld )
		Error( "Recursive ThreadLock\n" );
	g_LockHeld = true;
}

void ThreadUnlock()
{
	if ( !g_Threaded )
		return;
	if ( !g_LockHeld )
		Error( "ThreadUnlock without lock\n" );
	g_LockHeld = false;
	g_WorkLock.unlock();
}

int GetThreadWork()
{
	ThreadLock();
	if ( g_Dispatch == g_WorkCount )
	{
		ThreadUnlock();
		return -1;
	}
	UpdatePacifier( static_cast<float>( g_Dispatch ) / g_WorkCount );
	const int work = g_Dispatch++;
	ThreadUnlock();
	return work;
}

void RunThreads_Start( RunThreadsFn fn, void *pUserData, ERunThreadsPriority priority )
{
	if ( !fn || numthreads < 1 || !g_Workers.empty() )
		Error( "Invalid compile-tool thread start\n" );
	numthreads = std::min( numthreads, MAX_TOOL_THREADS );
	const int count = numthreads;
	{
		std::lock_guard<std::mutex> lock( g_StartGate.mutex );
		g_StartGate.released = false;
		g_StartGate.cancelled = false;
	}
	g_Threaded = true;
	try
	{
		g_Workers.reserve( count );
		for ( int i = 0; i < count; ++i )
		{
			g_Workers.emplace_back(
			    [=]()
			    {
				    std::unique_lock<std::mutex> lock( g_StartGate.mutex );
				    g_StartGate.ready.wait( lock,
				        []()
				        {
					        return g_StartGate.released;
				        } );
				    const bool cancelled = g_StartGate.cancelled;
				    lock.unlock();
				    if ( cancelled )
					    return;
				    SetWorkerPriority( priority );
				    fn( i, pUserData );
			    } );
		}
	}
	catch ( ... )
	{
		{
			std::lock_guard<std::mutex> lock( g_StartGate.mutex );
			g_StartGate.cancelled = true;
			g_StartGate.released = true;
		}
		g_StartGate.ready.notify_all();
		RunThreads_End();
		Error( "Could not start compile-tool worker\n" );
	}
	{
		std::lock_guard<std::mutex> lock( g_StartGate.mutex );
		g_StartGate.released = true;
	}
	g_StartGate.ready.notify_all();
}

void RunThreads_End()
{
	for ( std::thread &worker : g_Workers )
		worker.join();
	g_Workers.clear();
	g_Threaded = false;
}

void RunThreadsOn( int workcnt, qboolean showpacifier, RunThreadsFn fn, void *pUserData )
{
	if ( workcnt < 0 )
		Error( "Invalid compile-tool work count\n" );
	const int start = Plat_FloatTime();
	g_Dispatch = 0;
	g_WorkCount = workcnt;
	StartPacifier( "" );
	g_Pacifier = showpacifier;
	RunThreads_Start( fn, pUserData );
	RunThreads_End();
	if ( g_Pacifier )
	{
		EndPacifier( false );
		printf( " (%i)\n", int( Plat_FloatTime() ) - start );
	}
}

void RunThreadsOnIndividual( int workcnt, qboolean showpacifier, ThreadWorkerFn fn )
{
	if ( numthreads == -1 )
		ThreadSetDefault();
	g_WorkFunction = fn;
	RunThreadsOn( workcnt, showpacifier, RunIndividualWorker );
}
