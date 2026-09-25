//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Microbenchmarks for the real vstdlib CThreadPool and tier0
//          CTSQueue (RFC 0003 R20). Uses only the long-standing IThreadPool,
//          CJob and CTSQueue API, so the same source builds against an older
//          tree for A/B comparison. Every sample is validated (each job or
//          item exactly once) before its time counts.
//
//          threadpoolbench [--samples N] [--workers N] [--filter name]
//
//=============================================================================//

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "tier0/tslist.h"
#include "vstdlib/jobthread.h"
#include "testing/conformance_result.h"

static unsigned long s_checks = 0;
static unsigned long s_failures = 0;

static void Check( bool value, const char *what )
{
	++s_checks;
	if ( !value )
	{
		++s_failures;
		std::printf( "FAIL: %s\n", what );
	}
}

typedef std::chrono::steady_clock Clock;

static double Microseconds( Clock::time_point a, Clock::time_point b )
{
	return std::chrono::duration<double, std::micro>( b - a ).count();
}

// A fixed amount of work that the optimizer cannot remove.
static std::atomic<unsigned> s_sink( 0 );
static void Work( unsigned iterations )
{
	unsigned x = 2166136261u;
	for ( unsigned i = 0; i < iterations; i++ )
		x = ( x ^ i ) * 16777619u;
	s_sink.fetch_add( x & 1u, std::memory_order_relaxed );
}

class CCountJob : public CJob
{
public:
	CCountJob( std::atomic<int> *pDone, unsigned work ) : m_pDone( pDone ), m_work( work ) {}
	virtual JobStatus_t DoExecute()
	{
		Work( m_work );
		m_pDone->fetch_add( 1 );
		return JOB_OK;
	}
	std::atomic<int> *m_pDone;
	unsigned m_work;
};

// Waits without joining any job: the time measures the pool, not the waiter.
static bool SpinUntil( const std::atomic<int> &value, int target )
{
	const Clock::time_point deadline = Clock::now() + std::chrono::seconds( 30 );
	while ( value.load() < target )
	{
		if ( Clock::now() > deadline )
			return false;
		std::this_thread::yield();
	}
	return true;
}

//-----------------------------------------------------------------------------
// Workloads. Each returns elapsed microseconds and validates its result.
//-----------------------------------------------------------------------------

// Main thread submits many small queued jobs; workers drain the shared queue.
static double SharedFanout( IThreadPool *pPool )
{
	const int kJobs = 4096;
	std::atomic<int> done( 0 );
	std::vector<CCountJob *> jobs;
	jobs.reserve( kJobs );
	for ( int i = 0; i < kJobs; i++ )
	{
		jobs.push_back( new CCountJob( &done, 200 ) );
		jobs.back()->SetFlags( JF_QUEUE );
	}
	Clock::time_point start = Clock::now();
	for ( CCountJob *pJob : jobs )
		pPool->AddJob( pJob );
	const bool bDone = SpinUntil( done, kJobs );
	Clock::time_point end = Clock::now();
	Check( bDone && done.load() == kJobs, "shared fan-out ran every job once" );
	for ( CCountJob *pJob : jobs )
		pJob->Release();
	return Microseconds( start, end );
}

// Jobs running on workers spawn their own children (the stealing case).
class CSpawnJob : public CJob
{
public:
	CSpawnJob( IThreadPool *pPool, std::atomic<int> *pDone, int nChildren )
	    : m_pPool( pPool ), m_pDone( pDone ), m_nChildren( nChildren )
	{
	}
	virtual JobStatus_t DoExecute()
	{
		for ( int i = 0; i < m_nChildren; i++ )
		{
			CCountJob *pChild = new CCountJob( m_pDone, 400 );
			pChild->SetFlags( JF_QUEUE );
			m_pPool->AddJob( pChild );
			pChild->Release();
		}
		return JOB_OK;
	}
	IThreadPool *m_pPool;
	std::atomic<int> *m_pDone;
	int m_nChildren;
};

static double NestedSpawn( IThreadPool *pPool )
{
	const int kParents = 8;
	const int kChildren = 256;
	std::atomic<int> done( 0 );
	std::vector<CSpawnJob *> parents;
	for ( int i = 0; i < kParents; i++ )
	{
		parents.push_back( new CSpawnJob( pPool, &done, kChildren ) );
		parents.back()->SetFlags( JF_QUEUE );
	}
	Clock::time_point start = Clock::now();
	for ( CSpawnJob *pParent : parents )
		pPool->AddJob( pParent );
	const bool bDone = SpinUntil( done, kParents * kChildren );
	Clock::time_point end = Clock::now();
	Check( bDone && done.load() == kParents * kChildren, "nested spawn ran every child once" );
	for ( CSpawnJob *pParent : parents )
	{
		pParent->WaitForFinish( TT_INFINITE, pPool );
		pParent->Release();
	}
	return Microseconds( start, end );
}

// Recursive fan-out where each job waits for its own children (nested waits).
static std::atomic<int> s_treeJobs( 0 );
class CTreeJob : public CJob
{
public:
	CTreeJob( IThreadPool *pPool, int depth ) : m_pPool( pPool ), m_depth( depth ) {}
	virtual JobStatus_t DoExecute()
	{
		s_treeJobs.fetch_add( 1 );
		Work( 300 );
		if ( m_depth > 0 )
		{
			CJob *children[4];
			for ( int i = 0; i < 4; i++ )
			{
				children[i] = new CTreeJob( m_pPool, m_depth - 1 );
				children[i]->SetFlags( JF_QUEUE );
				m_pPool->AddJob( children[i] );
			}
			m_pPool->YieldWait( children, 4 );
			for ( int i = 0; i < 4; i++ )
				children[i]->Release();
		}
		return JOB_OK;
	}
	IThreadPool *m_pPool;
	int m_depth;
};

static double RecursiveWaits( IThreadPool *pPool )
{
	s_treeJobs = 0;
	CTreeJob *pRoot = new CTreeJob( pPool, 5 ); // 1365 jobs
	pRoot->SetFlags( JF_QUEUE );
	Clock::time_point start = Clock::now();
	pPool->AddJob( pRoot );
	pRoot->WaitForFinish( TT_INFINITE, pPool );
	Clock::time_point end = Clock::now();
	Check( s_treeJobs.load() == 1365, "recursive waits ran every job once" );
	pRoot->Release();
	return Microseconds( start, end );
}

// ParallelProcess over items of uneven cost.
struct Item
{
	unsigned cost;
	unsigned count;
};
static void ProcessItem( Item &item )
{
	Work( item.cost );
	++item.count;
}

static double ParallelProcessItems( IThreadPool *pPool )
{
	std::vector<Item> items( 8192 );
	for ( size_t i = 0; i < items.size(); i++ )
	{
		items[i].cost = 100 + ( ( i * 2654435761u ) % 900 );
		items[i].count = 0;
	}
	Clock::time_point start = Clock::now();
	ParallelProcess( "bench", pPool, items.data(), (unsigned)items.size(), &ProcessItem );
	Clock::time_point end = Clock::now();
	bool bOnce = true;
	for ( const Item &item : items )
		bOnce = bOnce && item.count == 1;
	Check( bOnce, "ParallelProcess processed every item once" );
	return Microseconds( start, end );
}

// Time from AddJob on the main thread until a worker starts the job, with the
// main thread neither waiting on nor helping the pool.
class CStampJob : public CJob
{
public:
	virtual JobStatus_t DoExecute()
	{
		m_started = Clock::now();
		m_flag.store( 1 );
		return JOB_OK;
	}
	Clock::time_point m_started;
	std::atomic<int> m_flag{ 0 };
};

static double WakeLatency( IThreadPool *pPool )
{
	// Let every worker go idle first.
	std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
	CStampJob *pJob = new CStampJob;
	pJob->SetFlags( JF_QUEUE );
	Clock::time_point start = Clock::now();
	pPool->AddJob( pJob );
	const bool bStarted = SpinUntil( pJob->m_flag, 1 );
	Check( bStarted, "woken worker ran the job" );
	const double us = bStarted ? Microseconds( start, pJob->m_started ) : 0.0;
	pJob->WaitForFinish( TT_INFINITE, pPool );
	pJob->Release();
	return us;
}

// CTSQueue: producers push unique values, consumers pop until all are seen.
static double TSQueueThroughput( int nProducers, int nConsumers )
{
	const int kPerProducer = 200000;
	const int kTotal = kPerProducer * nProducers;
	CTSQueue<int> *pQueue = new CTSQueue<int>;
	std::vector<std::atomic<unsigned char>> seen( kTotal );
	for ( std::atomic<unsigned char> &s : seen )
		s = 0;
	std::atomic<int> popped( 0 );
	std::vector<std::thread> threads;
	Clock::time_point start = Clock::now();
	for ( int p = 0; p < nProducers; p++ )
	{
		threads.push_back( std::thread(
		    [=]
		    {
			    for ( int i = 0; i < kPerProducer; i++ )
				    pQueue->PushItem( p * kPerProducer + i );
		    } ) );
	}
	for ( int c = 0; c < nConsumers; c++ )
	{
		threads.push_back( std::thread(
		    [&]
		    {
			    int value;
			    while ( popped.load( std::memory_order_relaxed ) < kTotal )
			    {
				    if ( pQueue->PopItem( &value ) )
				    {
					    seen[value].fetch_add( 1 );
					    popped.fetch_add( 1 );
				    }
			    }
		    } ) );
	}
	for ( std::thread &t : threads )
		t.join();
	Clock::time_point end = Clock::now();
	int nOnce = 0;
	for ( std::atomic<unsigned char> &s : seen )
		nOnce += ( s == 1 ) ? 1 : 0;
	Check( nOnce == kTotal, "CTSQueue delivered every value once" );
	delete pQueue;
	return Microseconds( start, end );
}

//-----------------------------------------------------------------------------

struct Result
{
	const char *name;
	const char *unit;
	std::vector<double> samples;
};

static double Median( std::vector<double> values )
{
	std::sort( values.begin(), values.end() );
	return values.empty() ? 0.0 : values[values.size() / 2];
}

int main( int argc, char **argv )
{
	int nSamples = 15;
	int nWorkers = 3;
	const char *pszFilter = NULL;
	for ( int i = 1; i + 1 < argc; i += 2 )
	{
		if ( !strcmp( argv[i], "--samples" ) )
			nSamples = atoi( argv[i + 1] );
		else if ( !strcmp( argv[i], "--workers" ) )
			nWorkers = atoi( argv[i + 1] );
		else if ( !strcmp( argv[i], "--filter" ) )
			pszFilter = argv[i + 1];
	}

	IThreadPool *pPool = CreateThreadPool();
	ThreadPoolStartParams_t params;
	params.nThreads = nWorkers;
	params.fDistribute = TRS_FALSE;
	pPool->Start( params, "Bench" );

	struct Workload
	{
		const char *name;
		double ( *fn )( IThreadPool * );
	};
	const Workload workloads[] = {
	    { "shared_fanout_4096", &SharedFanout },
	    { "nested_spawn_8x256", &NestedSpawn },
	    { "recursive_waits_1365", &RecursiveWaits },
	    { "parallel_process_8192", &ParallelProcessItems },
	    { "wake_latency", &WakeLatency },
	};

	std::vector<Result> results;
	for ( const Workload &w : workloads )
	{
		if ( pszFilter && !strstr( w.name, pszFilter ) )
			continue;
		Result r = { w.name, "us", std::vector<double>() };
		w.fn( pPool ); // warm-up
		for ( int s = 0; s < nSamples; s++ )
			r.samples.push_back( w.fn( pPool ) );
		results.push_back( r );
	}
	pPool->Stop();
	DestroyThreadPool( pPool );

	const int queueShapes[][2] = { { 1, 1 }, { 4, 4 } };
	for ( const int *shape : queueShapes )
	{
		static char names[2][32];
		char *name = names[shape[0] == 1 ? 0 : 1];
		snprintf( name, 32, "ctsqueue_%dp%dc_%dk", shape[0], shape[1], shape[0] * 200 );
		if ( pszFilter && !strstr( name, pszFilter ) )
			continue;
		Result r = { name, "us", std::vector<double>() };
		for ( int s = 0; s < std::max( 3, nSamples / 3 ); s++ )
			r.samples.push_back( TSQueueThroughput( shape[0], shape[1] ) );
		results.push_back( r );
	}

	for ( const Result &r : results )
	{
		std::vector<double> sorted = r.samples;
		std::sort( sorted.begin(), sorted.end() );
		std::printf( "bench %-24s median %10.1f %s  min %10.1f  p90 %10.1f  (n=%d, workers=%d)\n",
		    r.name, Median( r.samples ), r.unit, sorted.front(),
		    sorted[( sorted.size() * 9 ) / 10 < sorted.size() ? ( sorted.size() * 9 ) / 10
		                                                      : sorted.size() - 1],
		    (int)r.samples.size(), nWorkers );
	}
	return testing::ReportConformance( s_checks, s_failures );
}
