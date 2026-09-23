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
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

namespace jobsystem
{

namespace
{
// Which ready queue a job's executor lane feeds.
enum class Lane : uint8_t
{
	Compute,
	Main,
	Blocking
};

Lane LaneOf( ExecutorKind k )
{
	switch ( k )
	{
	case ExecutorKind::MainThread:
		return Lane::Main;
	case ExecutorKind::BlockingIO:
		return Lane::Blocking;
	case ExecutorKind::Compute:
	case ExecutorKind::Sequence:
	default:
		return Lane::Compute;
	}
}

// The role a servicer thread plays; decides which lanes it may pull from.
enum class Role : uint8_t
{
	Compute,
	Blocking,
	Main,
	Inline,
	Waiter // the caller without a pump: waits for completion only
};

// Servicers sleep on the condition variable of their slot, so new work wakes
// only threads able to run it.
enum Slot
{
	kSlotCompute,
	kSlotBlocking,
	kSlotMain, // the calling thread in any caller role
	kSlotCount
};

int SlotOf( Role role )
{
	switch ( role )
	{
	case Role::Compute:
		return kSlotCompute;
	case Role::Blocking:
		return kSlotBlocking;
	default:
		return kSlotMain;
	}
}

// Shared, mutex-protected run state. The mutex both serializes queue/counter
// updates and provides the publication edge from a producer's output writes
// (before it locks to finish) to a consumer's reads (after it locks to dequeue).
struct RunState
{
	const SealedGraph *graph = nullptr;
	const RunOptions *opts = nullptr;

	std::mutex mtx;
	std::condition_variable cv[kSlotCount];
	// idle: sleepers not yet signaled. signals: wakeups sent but not consumed.
	// A woken sleeper consumes a signal, or else was spurious and leaves idle.
	uint32_t idle[kSlotCount] = {};
	uint32_t signals[kSlotCount] = {};

	std::vector<uint32_t> readyCompute;  // Compute/Sequence, eligible now
	std::vector<uint32_t> readyMain;     // MainThread-affine, eligible now
	std::vector<uint32_t> readyBlocking; // BlockingIO, eligible now
	std::vector<uint32_t> remaining;     // prerequisites not yet resolved
	std::vector<char> willCancel;        // decided at activation
	std::vector<char> willStall;         // executor lane has no servicer
	std::vector<char> stuck;             // resolved as unserviceable
	std::vector<JobState> states;

	bool mainServicesBlocking = false; // main pumps BlockingIO (no blocking lane)
	bool mainPumps = false;            // the caller services the Main role

	uint32_t total = 0;
	uint32_t resolved = 0; // terminal OR stuck; the loop ends at resolved==total
	uint32_t succeeded = 0, failed = 0, canceled = 0, executed = 0, unresolved = 0;

	// Prepare for a run, reusing storage from earlier runs.
	void Reset( const SealedGraph &g, const RunOptions &o )
	{
		const uint32_t n = g.JobCount();
		graph = &g;
		opts = &o;
		for ( int slot = 0; slot < kSlotCount; ++slot )
			idle[slot] = signals[slot] = 0;
		readyCompute.clear();
		readyMain.clear();
		readyBlocking.clear();
		remaining.resize( n );
		willCancel.assign( n, 0 );
		willStall.assign( n, 0 );
		stuck.assign( n, 0 );
		states.assign( n, JobState::Admitted );
		mainServicesBlocking = false;
		mainPumps = false;
		total = n;
		resolved = 0;
		succeeded = failed = canceled = executed = unresolved = 0;
	}

	bool GlobalCancel() const
	{
		return opts->cancel && opts->cancel->load( std::memory_order_acquire );
	}

	// A now-eligible job must be canceled if any Success prerequisite did not
	// succeed, or if the scope was canceled.
	bool DecideCancel( uint32_t id ) const
	{
		if ( GlobalCancel() )
			return true;
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
		if ( willStall[id] )
			return true;
		const SealedGraph::Job &j = graph->GetJob( id );
		for ( const SealedGraph::Prereq &p : j.prereqs )
			if ( stuck[p.producer] )
				return true;
		return false;
	}

	void Enqueue( uint32_t id )
	{
		switch ( LaneOf( graph->GetJob( id ).executor.kind ) )
		{
		case Lane::Main:
			readyMain.push_back( id );
			break;
		case Lane::Blocking:
			readyBlocking.push_back( id );
			break;
		case Lane::Compute:
		default:
			readyCompute.push_back( id );
			break;
		}
	}

	// Whether this role has any eligible ready work right now. Pure: used as
	// the (side-effect-free) wait predicate so a spurious re-evaluation never
	// consumes a job.
	bool CanServe( Role role ) const
	{
		switch ( role )
		{
		case Role::Compute:
			return !readyCompute.empty();
		case Role::Blocking:
			return !readyBlocking.empty();
		case Role::Main:
			return !readyMain.empty() || !readyCompute.empty() ||
			       ( mainServicesBlocking && !readyBlocking.empty() );
		case Role::Inline:
			return !readyMain.empty() || !readyBlocking.empty() || !readyCompute.empty();
		case Role::Waiter:
		default:
			return false;
		}
	}

	// Pop the next id this role may run, or return false. Main helps compute
	// after its own lane; it services blocking only when there is no lane.
	bool Pop( Role role, uint32_t &out )
	{
		auto take = []( std::vector<uint32_t> &q, uint32_t &o )
		{
			if ( q.empty() )
				return false;
			o = q.back();
			q.pop_back();
			return true;
		};
		switch ( role )
		{
		case Role::Compute:
			return take( readyCompute, out );
		case Role::Blocking:
			return take( readyBlocking, out );
		case Role::Main:
			if ( take( readyMain, out ) )
				return true;
			if ( take( readyCompute, out ) )
				return true;
			if ( mainServicesBlocking && take( readyBlocking, out ) )
				return true;
			return false;
		case Role::Inline:
			if ( take( readyMain, out ) )
				return true;
			if ( take( readyBlocking, out ) )
				return true;
			return take( readyCompute, out );
		case Role::Waiter:
		default:
			return false;
		}
	}

	// Called with the lock held; the predicate is rechecked by the caller.
	void Sleep( std::unique_lock<std::mutex> &lk, int slot )
	{
		++idle[slot];
		cv[slot].wait( lk );
		if ( signals[slot] > 0 )
			--signals[slot];
		else
			--idle[slot];
	}

	bool WakeOne( int slot )
	{
		if ( idle[slot] == 0 )
			return false;
		--idle[slot];
		++signals[slot];
		cv[slot].notify_one();
		return true;
	}

	void WakeAll()
	{
		for ( int slot = 0; slot < kSlotCount; ++slot )
		{
			if ( idle[slot] == 0 )
				continue;
			signals[slot] += idle[slot];
			idle[slot] = 0;
			cv[slot].notify_all();
		}
	}

	// Called with the lock held by a servicer of role `self` that will next
	// re-check the queues itself. Wake just enough sleepers for the ready work
	// `self` will not take. Every thread rechecks the queues under this lock
	// before sleeping, so a job is never stranded: either a sleeper is
	// signaled for it or a running servicer will find it.
	void Dispatch( Role self )
	{
		if ( resolved == total )
		{
			WakeAll();
			return;
		}
		if ( self == Role::Inline )
			return; // the only servicer
		size_t compute = readyCompute.size();
		size_t blocking = readyBlocking.size();
		size_t main = readyMain.size();
		switch ( self )
		{
		case Role::Compute:
			compute -= compute ? 1 : 0;
			break;
		case Role::Blocking:
			blocking -= blocking ? 1 : 0;
			break;
		case Role::Main:
			if ( main )
				--main;
			else if ( compute )
				--compute;
			else if ( mainServicesBlocking && blocking )
				--blocking;
			break;
		default:
			break;
		}
		bool mainWoken = false;
		if ( main && self != Role::Main && mainPumps )
			mainWoken = WakeOne( kSlotMain );
		while ( blocking && WakeOne( kSlotBlocking ) )
			--blocking;
		while ( compute && WakeOne( kSlotCompute ) )
			--compute;
		const bool mainCanHelp = compute || ( mainServicesBlocking && blocking );
		if ( mainCanHelp && !mainWoken && self != Role::Main && mainPumps )
			WakeOne( kSlotMain );
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
// dependents whose last prerequisite just resolved. The caller dispatches
// wakeups once the cascade is complete.
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
		case JobState::Succeeded:
			rs.succeeded++;
			break;
		case JobState::Failed:
			rs.failed++;
			break;
		case JobState::Canceled:
			rs.canceled++;
			break;
		default:
			break;
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
				// Resolve the stuck dependent through the same path so its own
				// dependents cascade.
				ResolveLocked( rs, d, JobState::Admitted, /*asStuck=*/true );
			}
			else
			{
				rs.willCancel[d] = rs.DecideCancel( d ) ? 1 : 0;
				rs.Enqueue( d );
			}
		}
	}
}

// One servicer pass for a given role. Returns when the whole graph is resolved.
void ServiceLoop( RunState &rs, Role role )
{
	const int slot = SlotOf( role );
	std::unique_lock<std::mutex> lk( rs.mtx );
	for ( ;; )
	{
		while ( rs.resolved != rs.total && !rs.CanServe( role ) )
			rs.Sleep( lk, slot );
		if ( rs.resolved == rs.total )
			return;

		uint32_t id = 0;
		if ( !rs.Pop( role, id ) )
			continue;

		const bool doCancel = rs.willCancel[id] || rs.GlobalCancel();
		if ( doCancel )
		{
			Trace( rs, id, JobState::Canceled );
			ResolveLocked( rs, id, JobState::Canceled, /*asStuck=*/false );
			rs.Dispatch( role );
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
			if ( job.function )
				job.function( ctx );
			terminal = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
			lk.lock();
			if ( job.function )
				rs.executed++;
		}
		Trace( rs, id, terminal );
		ResolveLocked( rs, id, terminal, /*asStuck=*/false );
		rs.Dispatch( role );
	}
}

// Inline mode: the caller is the only servicer and no other thread can see
// the run state, so jobs run without the scheduling lock. Every lane is
// serviced and nothing stalls, so ready work exists until all are resolved.
void InlineLoop( RunState &rs )
{
	uint32_t id = 0;
	while ( rs.resolved != rs.total && rs.Pop( Role::Inline, id ) )
	{
		if ( rs.willCancel[id] || rs.GlobalCancel() )
		{
			Trace( rs, id, JobState::Canceled );
			ResolveLocked( rs, id, JobState::Canceled, /*asStuck=*/false );
			continue;
		}
		const SealedGraph::Job &job = rs.graph->GetJob( id );
		Trace( rs, id, JobState::Running );
		JobRunContext ctx( rs.opts->frame, id );
		if ( job.function )
		{
			job.function( ctx );
			rs.executed++;
		}
		const JobState terminal = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
		Trace( rs, id, terminal );
		ResolveLocked( rs, id, terminal, /*asStuck=*/false );
	}
}

// Configure lanes and seed the roots. Runs before any servicer enters the run.
void Prepare( RunState &rs, const SealedGraph &graph, const RunOptions &opts, bool inlineMode,
    bool haveBlocking )
{
	const uint32_t n = graph.JobCount();
	const bool pumpMain = opts.pumpMainThread;
	rs.Reset( graph, opts );
	rs.mainServicesBlocking = !haveBlocking; // main covers blocking when no lane
	rs.mainPumps = !inlineMode && pumpMain;

	// Precompute which lanes have no servicer under this configuration. In inline
	// mode the single caller services every lane, so nothing stalls.
	if ( !inlineMode )
	{
		for ( uint32_t i = 0; i < n; ++i )
		{
			const ExecutorKind k = graph.GetJob( i ).executor.kind;
			const bool mainUnserviced = ( k == ExecutorKind::MainThread ) && !pumpMain;
			const bool blockingUnserviced =
			    ( k == ExecutorKind::BlockingIO ) && !haveBlocking && !pumpMain;
			if ( mainUnserviced || blockingUnserviced )
				rs.willStall[i] = 1;
		}
	}

	// Seed roots. Initialize every remaining count first, because resolving a
	// stuck root immediately decrements its dependents' counts; those counts must
	// already be set (RFC 0003 stall accounting must not underflow).
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

// The caller's part of a threaded run: pump main-thread work and help, or
// only wait when it does not pump. Unserviceable lanes were already accounted
// as stuck, so resolved reaches total and every servicer exits.
void ServiceCaller( RunState &rs, bool pumpMain )
{
	ServiceLoop( rs, pumpMain ? Role::Main : Role::Waiter );
}

RunResult Collect( RunState &rs )
{
	RunResult result;
	result.states = std::move( rs.states );
	result.succeeded = rs.succeeded;
	result.failed = rs.failed;
	result.canceled = rs.canceled;
	result.executed = rs.executed;
	result.unresolved = rs.unresolved;
	result.stalled = rs.unresolved > 0;
	return result;
}
} // namespace

//-----------------------------------------------------------------------------
// Persistent workers: threads park on the pool between runs and join a
// published run as its compute/blocking servicers. One run at a time.
//-----------------------------------------------------------------------------
class ParallelExecutor::WorkerPool
{
public:
	WorkerPool( int nCompute, int nBlocking ) : m_nCompute( nCompute ), m_nBlocking( nBlocking ) {}

	~WorkerPool()
	{
		{
			std::lock_guard<std::mutex> lk( m_mtx );
			m_quit = true;
		}
		m_cv.notify_all();
		for ( std::thread &t : m_threads )
			t.join();
	}

	// Claim the pool for one run. The first run of an executor does not claim
	// it: a one-shot executor would pay thread parking and a teardown wakeup
	// for nothing, so persistent workers start only once an executor is reused.
	bool TryAcquire()
	{
		std::lock_guard<std::mutex> lk( m_mtx );
		if ( m_busy )
			return false;
		if ( !m_reused )
		{
			m_reused = true;
			return false;
		}
		m_busy = true;
		return true;
	}

	// Publish a prepared run to every worker, starting them on first use.
	void Start( RunState &rs )
	{
		const int count = m_nCompute + m_nBlocking;
		uint64_t generation;
		{
			std::lock_guard<std::mutex> lk( m_mtx );
			m_run = &rs;
			m_active = count;
			generation = ++m_generation;
			if ( !m_threads.empty() )
			{
				m_cv.notify_all();
				return;
			}
		}
		// First use: new threads enter this run directly instead of parking
		// first. Only the pool owner touches m_threads.
		m_threads.reserve( (size_t)count );
		for ( int i = 0; i < count; ++i )
		{
			const Role role = i < m_nCompute ? Role::Compute : Role::Blocking;
			m_threads.emplace_back( [this, role, &rs, generation] { ThreadMain( role, &rs, generation ); } );
		}
	}

	// Wait until every worker has left the run. The owner may then read the
	// run state; nothing else touches it until Release().
	void WaitForWorkers()
	{
		std::unique_lock<std::mutex> lk( m_mtx );
		m_doneCv.wait( lk, [&] { return m_active == 0; } );
		m_run = nullptr;
	}

	void Release()
	{
		std::lock_guard<std::mutex> lk( m_mtx );
		m_busy = false;
	}

	RunState state; // reused storage for runs on this pool

private:
	void ThreadMain( Role role, RunState *run, uint64_t seen )
	{
		for ( ;; )
		{
			ServiceLoop( *run, role );
			std::unique_lock<std::mutex> lk( m_mtx );
			if ( --m_active == 0 )
				m_doneCv.notify_one();
			m_cv.wait( lk, [&] { return m_quit || m_generation != seen; } );
			if ( m_quit )
				return;
			seen = m_generation;
			run = m_run;
		}
	}

	const int m_nCompute;
	const int m_nBlocking;
	std::mutex m_mtx;
	std::condition_variable m_cv;     // workers: a new run or quit
	std::condition_variable m_doneCv; // owner: all workers left the run
	std::vector<std::thread> m_threads;
	RunState *m_run = nullptr;
	uint64_t m_generation = 0;
	int m_active = 0;
	bool m_busy = false;
	bool m_reused = false;
	bool m_quit = false;
};

ParallelExecutor::ParallelExecutor( int nComputeWorkers, int nBlockingWorkers )
    : m_nWorkers( nComputeWorkers < 0 ? 0 : nComputeWorkers ),
      m_nBlocking( nBlockingWorkers < 0 ? 0 : nBlockingWorkers )
{
	if ( m_nWorkers > 0 )
		m_pool = std::make_unique<WorkerPool>( m_nWorkers, m_nBlocking );
}

ParallelExecutor::~ParallelExecutor() = default;

ParallelExecutor::ParallelExecutor( const ParallelExecutor &other )
    : ParallelExecutor( other.m_nWorkers, other.m_nBlocking )
{
}

ParallelExecutor &ParallelExecutor::operator=( const ParallelExecutor &other )
{
	if ( this != &other )
	{
		m_pool.reset();
		m_nWorkers = other.m_nWorkers;
		m_nBlocking = other.m_nBlocking;
		if ( m_nWorkers > 0 )
			m_pool = std::make_unique<WorkerPool>( m_nWorkers, m_nBlocking );
	}
	return *this;
}

RunResult ParallelExecutor::Execute( const SealedGraph &graph, const RunOptions &opts )
{
	const uint32_t n = graph.JobCount();
	const bool inlineMode = ( m_nWorkers == 0 );
	const bool haveBlocking = ( m_nBlocking > 0 );

	if ( inlineMode || n == 0 )
	{
		// Everything on the caller; the inline role services every lane.
		RunState rs;
		Prepare( rs, graph, opts, /*inlineMode=*/true, haveBlocking );
		InlineLoop( rs );
		return Collect( rs );
	}

	if ( m_pool && m_pool->TryAcquire() )
	{
		RunState &rs = m_pool->state;
		Prepare( rs, graph, opts, /*inlineMode=*/false, haveBlocking );
		m_pool->Start( rs );
		try
		{
			ServiceCaller( rs, opts.pumpMainThread );
		}
		catch ( ... )
		{
			// Workers still reference this run; a job exception cannot unwind
			// past them (as with joinable worker threads before).
			std::terminate();
		}
		m_pool->WaitForWorkers();
		RunResult result = Collect( rs );
		m_pool->Release();
		return result;
	}

	// First run, or pool busy (concurrent or nested Execute): transient
	// workers for this run only.
	RunState rs;
	Prepare( rs, graph, opts, /*inlineMode=*/false, haveBlocking );
	std::vector<std::thread> workers;
	workers.reserve( (size_t)m_nWorkers + (size_t)m_nBlocking );
	for ( int i = 0; i < m_nWorkers; ++i )
		workers.emplace_back( [&rs] { ServiceLoop( rs, Role::Compute ); } );
	for ( int i = 0; i < m_nBlocking; ++i )
		workers.emplace_back( [&rs] { ServiceLoop( rs, Role::Blocking ); } );
	ServiceCaller( rs, opts.pumpMainThread );
	for ( auto &t : workers )
		t.join();
	return Collect( rs );
}

} // namespace jobsystem
