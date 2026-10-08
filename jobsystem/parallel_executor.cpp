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
//          Runner bindings (RunOptions::mainThreadRunner/blockingRunner): a
//          bound lane's ready jobs are posted to the runner, run there, and
//          resolve under the scheduling mutex like any other job. A job the
//          runner refuses or drops unrun resolves as stuck.
//
//          Two worker sources share this one scheduler. ParallelExecutor owns
//          dedicated compute/blocking threads that sleep on the run between
//          jobs. TaskExecutor (task_executor.h) borrows an IWorkerBackend and
//          drains the compute queue with runner tasks that never sleep: a
//          runner exits when the queue is empty, and Dispatch posts new
//          runners whenever ready compute work exceeds the runners that will
//          still look at the queue (RFC 0003 J1).
//
//=============================================================================//

#include "jobsystem/parallel_executor.h"
#include "jobsystem/task_executor.h"

#include <atomic>
#include <condition_variable>
#include <exception>
#include <memory>
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
	Waiter,   // the caller without a pump: waits for completion only
	External, // a bound runner's task; takes no work from the ready queues
	Runner,   // a TaskExecutor runner task: compute only, never sleeps
	Helper    // a TaskExecutor caller without a pump: compute only
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
	// Runner bindings in effect for this run (null: the lane is serviced here).
	platform::ITaskRunner *mainRunner = nullptr;
	platform::ITaskRunner *blockingRunner = nullptr;

	// Task mode (TaskExecutor): runner tasks posted to a borrowed backend drain
	// the compute queue. A reserved runner counts in runners, seeking and
	// starting until its task starts (or its post is refused); a started
	// runner counts in seeking whenever it will look at the queue before it
	// exits. Every count is guarded by mtx.
	IWorkerBackend *tasks = nullptr;
	uint32_t taskBudget = 0; // most runner tasks at once
	uint32_t runners = 0;    // reserved or started, not exited
	uint32_t seeking = 0;    // will look at the compute queue (includes starting)
	uint32_t starting = 0;   // reserved or posted, not started
	uint32_t toPost = 0;     // reserved, not yet handed to PostTask
	uint32_t posting = 0;    // handed to PostTask, result not yet recorded
	bool refused = false;    // a post was refused in this run
	std::vector<void *> tickets;
	std::condition_variable postsDone; // the caller: no post in flight
	TaskRunStats stats;

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
		mainRunner = nullptr;
		blockingRunner = nullptr;
		tasks = nullptr;
		taskBudget = runners = seeking = starting = toPost = posting = 0;
		refused = false;
		tickets.clear();
		stats = TaskRunStats{};
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

	// The runner a job's lane is bound to, or null.
	platform::ITaskRunner *BoundRunner( uint32_t id ) const
	{
		switch ( LaneOf( graph->GetJob( id ).executor.kind ) )
		{
		case Lane::Main:
			return mainRunner;
		case Lane::Blocking:
			return blockingRunner;
		default:
			return nullptr;
		}
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
		case Role::Runner:
		case Role::Helper:
			return !readyCompute.empty();
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
		case Role::Runner:
		case Role::Helper:
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
		if ( tasks )
		{
			DispatchTasks( self );
			return;
		}
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

	// Dispatch in task mode. Reserve runners (posted by DispatchAndPost after
	// the lock is released) until every ready compute job has a runner that
	// will look at the queue, up to the budget. The caller helps with compute
	// work no started runner covers, so progress never waits for the backend
	// to start a task, and it services its own lanes.
	void DispatchTasks( Role self )
	{
		size_t compute = readyCompute.size();
		size_t blocking = readyBlocking.size();
		size_t main = readyMain.size();
		switch ( self )
		{
		case Role::Main:
			if ( main )
				--main;
			else if ( compute )
				--compute;
			else if ( mainServicesBlocking && blocking )
				--blocking;
			break;
		case Role::Helper:
			compute -= compute ? 1 : 0;
			break;
		default:
			break; // a runner is in seeking; External takes nothing
		}
		while ( compute > seeking && runners < taskBudget )
		{
			++runners;
			++seeking;
			++starting;
			++toPost;
		}
		if ( runners > stats.peakRunners )
			stats.peakRunners = runners;
		// J1: past this point every ready compute job has a looking runner,
		// or the budget is spent, or the backend refused a runner.
		if ( compute > seeking && runners < taskBudget && !refused )
			++stats.uncoveredReady;

		const bool callerAwake = self == Role::Main || self == Role::Helper;
		if ( callerAwake )
			return;
		const size_t started = seeking - starting;
		const bool callerWork = compute > started || ( mainPumps && main ) ||
		                        ( mainPumps && mainServicesBlocking && blocking );
		if ( callerWork )
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

void EnqueueLocked( RunState &rs, uint32_t id );
void RunBoundJob( RunState &rs, uint32_t id );
void RunnerTask( void *context );

// Dispatch, then hand the runners it reserved to the backend. PostTask runs
// without the scheduling lock: the backend may start the task, which takes
// the lock, before PostTask returns. A refusal releases every outstanding
// reservation and leaves the work to the caller, which helps with compute.
void DispatchAndPost( RunState &rs, std::unique_lock<std::mutex> &lk, Role self )
{
	rs.Dispatch( self );
	while ( rs.toPost > 0 )
	{
		if ( rs.resolved == rs.total )
		{
			// Nothing left for a runner to find.
			rs.runners -= rs.toPost;
			rs.seeking -= rs.toPost;
			rs.starting -= rs.toPost;
			rs.toPost = 0;
			break;
		}
		// Hand every reservation to the backend in one unlocked span, then
		// record the tickets under one relock: started runners never queue
		// on the scheduling lock behind the poster between posts.
		constexpr uint32_t kMaxBatch = 16;
		void *tickets[kMaxBatch];
		const uint32_t count = rs.toPost < kMaxBatch ? rs.toPost : kMaxBatch;
		rs.toPost -= count;
		rs.posting += count;
		lk.unlock();
		uint32_t accepted = 0;
		for ( ; accepted < count; ++accepted )
		{
			tickets[accepted] = rs.tasks->PostTask( &RunnerTask, &rs );
			if ( !tickets[accepted] )
				break;
		}
		lk.lock();
		rs.posting -= count;
		for ( uint32_t i = 0; i < accepted; ++i )
			rs.tickets.push_back( tickets[i] );
		rs.stats.tasksPosted += accepted;
		if ( accepted == count )
			continue;
		// Refused: release this batch's unposted reservations and every
		// later one; the caller, which helps with compute, does the work.
		const uint32_t released = ( count - accepted ) + rs.toPost;
		rs.runners -= released;
		rs.seeking -= released;
		rs.starting -= released;
		rs.toPost = 0;
		rs.refused = true;
		rs.stats.tasksRefused++;
		if ( self != Role::Main && self != Role::Helper )
			rs.WakeOne( kSlotMain );
	}
	if ( rs.tasks && rs.posting == 0 && rs.resolved == rs.total )
		rs.postsDone.notify_all();
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
				EnqueueLocked( rs, d );
			}
		}
	}
}

// A bound job's task. Its ownership token settles, exactly once, whether the
// job ran, was accepted and then dropped unrun (the guard resolves it as
// stuck), or was refused or dropped during the post (the poster resolves it).
struct BoundJob
{
	enum : int
	{
		kPosting,
		kAccepted,
		kRan,
		kDroppedEarly
	};

	RunState *rs;
	uint32_t id;
	std::shared_ptr<std::atomic<int>> token;

	BoundJob( RunState *state, uint32_t job, std::shared_ptr<std::atomic<int>> t )
	    : rs( state ), id( job ), token( std::move( t ) )
	{
	}
	BoundJob( BoundJob &&other ) noexcept
	    : rs( other.rs ), id( other.id ), token( std::move( other.token ) )
	{
	}
	BoundJob( const BoundJob & ) = delete;

	~BoundJob()
	{
		if ( !token )
			return; // moved from
		int expected = kPosting;
		if ( token->compare_exchange_strong( expected, kDroppedEarly ) )
			return; // the poster sees this and resolves the job under its lock
		if ( expected != kAccepted )
			return; // it ran
		// Accepted, then destroyed without running (runner shutdown).
		std::unique_lock<std::mutex> lk( rs->mtx );
		ResolveLocked( *rs, id, JobState::Admitted, /*asStuck=*/true );
		DispatchAndPost( *rs, lk, Role::External );
	}

	void operator()()
	{
		token->store( kRan );
		RunBoundJob( *rs, id );
	}
};

// Called with the lock held. The runner never runs the task inside the post
// (platform.task-runner.v1), so posting under the scheduling mutex cannot
// re-enter it; a task that starts on another thread waits for this lock.
void PostBoundLocked( RunState &rs, uint32_t id, platform::ITaskRunner &runner )
{
	auto token = std::make_shared<std::atomic<int>>( BoundJob::kPosting );
	const platform::PostResult posted = runner.PostTask( BoundJob( &rs, id, token ) );
	int expected = BoundJob::kPosting;
	if ( posted == platform::PostResult::kAccepted &&
	     token->compare_exchange_strong( expected, BoundJob::kAccepted ) )
		return;
	if ( posted == platform::PostResult::kAccepted && expected == BoundJob::kRan )
		return;
	// Refused, or dropped unrun before the post returned.
	ResolveLocked( rs, id, JobState::Admitted, /*asStuck=*/true );
}

void EnqueueLocked( RunState &rs, uint32_t id )
{
	if ( platform::ITaskRunner *runner = rs.BoundRunner( id ) )
		PostBoundLocked( rs, id, *runner );
	else
		rs.Enqueue( id );
}

// Runs one bound job on its runner's thread, then resolves it like any job.
void RunBoundJob( RunState &rs, uint32_t id )
{
	std::unique_lock<std::mutex> lk( rs.mtx );
	JobState terminal = JobState::Canceled;
	if ( !rs.willCancel[id] && !rs.GlobalCancel() )
	{
		const SealedGraph::Job &job = rs.graph->GetJob( id );
		Trace( rs, id, JobState::Running );
		lk.unlock();
		JobRunContext ctx( rs.opts->frame, id );
		if ( job.function )
			job.function( ctx );
		terminal = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
		lk.lock();
		if ( job.function )
		{
			rs.executed++;
			rs.stats.jobsOnBound++;
		}
	}
	Trace( rs, id, terminal );
	ResolveLocked( rs, id, terminal, /*asStuck=*/false );
	DispatchAndPost( rs, lk, Role::External );
	// The unlock is this task's last access to the run: the caller returns
	// only after it observes every job resolved under this mutex.
}

// One servicer pass for a given role. Returns when the whole graph is resolved,
// or, for a runner task, as soon as the compute queue is empty.
void ServiceLoop( RunState &rs, Role role )
{
	const int slot = SlotOf( role );
	const bool runner = role == Role::Runner;
	std::unique_lock<std::mutex> lk( rs.mtx );
	if ( runner )
	{
		--rs.starting;
		rs.stats.tasksRan++;
	}
	for ( ;; )
	{
		if ( runner )
		{
			if ( rs.resolved == rs.total || !rs.CanServe( role ) )
			{
				// The unlock below is this task's last access to the run.
				--rs.seeking;
				--rs.runners;
				return;
			}
		}
		else
		{
			while ( rs.resolved != rs.total && !rs.CanServe( role ) )
				rs.Sleep( lk, slot );
			if ( rs.resolved == rs.total )
				return;
		}

		uint32_t id = 0;
		if ( !rs.Pop( role, id ) )
			continue;
		if ( runner )
			--rs.seeking;

		// In task mode, a servicer that finds a long compute queue claims a
		// guided share of it with this job (at most kMaxChunk), so a wide
		// graph of small jobs takes the scheduling lock once per share rather
		// than twice per job. The share is the queue divided among every
		// thread that will look at it (live runners and the caller), so the
		// others still find work; a short queue (a chain, a small fan-out)
		// is taken one job at a time, which keeps dependents' latency.
		constexpr size_t kMaxChunk = 32;
		uint32_t chunk[kMaxChunk];
		bool cancel[kMaxChunk];
		size_t count = 1;
		chunk[0] = id;
		if ( rs.tasks && LaneOf( rs.graph->GetJob( id ).executor.kind ) == Lane::Compute )
		{
			const size_t lookers = (size_t)rs.seeking + 2; // this thread and one more
			size_t share = rs.readyCompute.size() / lookers;
			if ( share > kMaxChunk - 1 )
				share = kMaxChunk - 1;
			for ( ; share > 0; --share, ++count )
			{
				chunk[count] = rs.readyCompute.back();
				rs.readyCompute.pop_back();
			}
		}
		for ( size_t i = 0; i < count; ++i )
			cancel[i] = rs.willCancel[chunk[i]] != 0;

		// Run outside the lock. The lock released here, and re-acquired to
		// resolve the share, publishes these jobs' writes to later dequeuers.
		// A cancellation published while the share runs cancels the jobs not
		// yet started, as it would had they been popped one by one.
		JobState terminal[kMaxChunk];
		bool ran[kMaxChunk];
		lk.unlock();
		for ( size_t i = 0; i < count; ++i )
		{
			ran[i] = false;
			if ( cancel[i] || rs.GlobalCancel() )
			{
				terminal[i] = JobState::Canceled;
				continue;
			}
			const SealedGraph::Job &job = rs.graph->GetJob( chunk[i] );
			Trace( rs, chunk[i], JobState::Running );
			JobRunContext ctx( rs.opts->frame, chunk[i] );
			if ( job.function )
			{
				job.function( ctx );
				ran[i] = true;
			}
			terminal[i] = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
		}
		lk.lock();
		for ( size_t i = 0; i < count; ++i )
		{
			if ( ran[i] )
			{
				rs.executed++;
				if ( runner )
					rs.stats.jobsOnRunners++;
				else
					rs.stats.jobsOnCaller++;
			}
			Trace( rs, chunk[i], terminal[i] );
			ResolveLocked( rs, chunk[i], terminal[i], /*asStuck=*/false );
		}
		if ( runner )
			++rs.seeking;
		DispatchAndPost( rs, lk, role );
	}
}

// A TaskExecutor runner: drain the compute queue, then return the worker.
void RunnerTask( void *context )
{
	ServiceLoop( *static_cast<RunState *>( context ), Role::Runner );
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
			rs.stats.jobsOnCaller++;
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
	if ( !inlineMode )
	{
		// A caller that already is the runner cannot wait for it: it services
		// the lane itself, as a pump.
		if ( opts.mainThreadRunner && !opts.mainThreadRunner->BelongsToCurrentThread() )
			rs.mainRunner = opts.mainThreadRunner;
		if ( opts.blockingRunner && !opts.blockingRunner->RunsTasksInCurrentSequence() )
			rs.blockingRunner = opts.blockingRunner;
	}
	const bool callerIsMainRunner =
	    opts.mainThreadRunner && opts.mainThreadRunner->BelongsToCurrentThread();
	const bool callerIsBlockingRunner =
	    opts.blockingRunner && opts.blockingRunner->RunsTasksInCurrentSequence();
	rs.mainServicesBlocking =
	    !haveBlocking || callerIsBlockingRunner; // main covers blocking when no lane
	rs.mainPumps = !inlineMode && ( pumpMain || callerIsMainRunner || callerIsBlockingRunner );

	// Precompute which lanes have no servicer under this configuration. In inline
	// mode the single caller services every lane, so nothing stalls.
	if ( !inlineMode )
	{
		for ( uint32_t i = 0; i < n; ++i )
		{
			const ExecutorKind k = graph.GetJob( i ).executor.kind;
			const bool mainUnserviced =
			    ( k == ExecutorKind::MainThread ) && !rs.mainPumps && !rs.mainRunner;
			const bool blockingUnserviced = ( k == ExecutorKind::BlockingIO ) && !haveBlocking &&
			                                !rs.mainPumps && !rs.blockingRunner;
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
			EnqueueLocked( rs, i );
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
			m_threads.emplace_back(
			    [this, role, &rs, generation]
			    {
				    ThreadMain( role, &rs, generation );
			    } );
		}
	}

	// Wait until every worker has left the run. The owner may then read the
	// run state; nothing else touches it until Release().
	void WaitForWorkers()
	{
		std::unique_lock<std::mutex> lk( m_mtx );
		m_doneCv.wait( lk,
		    [&]
		    {
			    return m_active == 0;
		    } );
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
			m_cv.wait( lk,
			    [&]
			    {
				    return m_quit || m_generation != seen;
			    } );
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
			ServiceCaller( rs, rs.mainPumps );
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
		workers.emplace_back(
		    [&rs]
		    {
			    ServiceLoop( rs, Role::Compute );
		    } );
	for ( int i = 0; i < m_nBlocking; ++i )
		workers.emplace_back(
		    [&rs]
		    {
			    ServiceLoop( rs, Role::Blocking );
		    } );
	ServiceCaller( rs, rs.mainPumps );
	for ( auto &t : workers )
		t.join();
	return Collect( rs );
}

//-----------------------------------------------------------------------------
// TaskExecutor: the same scheduler, with runner tasks on a borrowed backend in
// place of dedicated threads.
//-----------------------------------------------------------------------------
namespace
{
// Run state storage is reused per thread, so a frame's repeated batches do
// not allocate. A nested Execute on the same thread leases another entry.
std::vector<std::unique_ptr<RunState>> &FreeRunStates()
{
	thread_local std::vector<std::unique_ptr<RunState>> t_free;
	return t_free;
}

struct RunStateLease
{
	RunStateLease()
	{
		std::vector<std::unique_ptr<RunState>> &free = FreeRunStates();
		if ( free.empty() )
		{
			rs = std::make_unique<RunState>();
			return;
		}
		rs = std::move( free.back() );
		free.pop_back();
	}
	~RunStateLease()
	{
		constexpr size_t kMaxCachedRuns = 4;
		std::vector<std::unique_ptr<RunState>> &free = FreeRunStates();
		if ( free.size() < kMaxCachedRuns )
			free.push_back( std::move( rs ) );
	}
	RunStateLease( const RunStateLease & ) = delete;
	RunStateLease &operator=( const RunStateLease & ) = delete;

	std::unique_ptr<RunState> rs;
};
} // namespace

RunResult TaskExecutor::Execute(
    const SealedGraph &graph, const RunOptions &opts, TaskRunStats *stats )
{
	const uint32_t n = graph.JobCount();
	const int workers = m_backend ? m_backend->WorkerCount() : 0;
	const bool inlineMode = n == 0 || workers <= 0 || m_backend->ShouldRunInline();

	RunStateLease lease;
	RunState &rs = *lease.rs;
	if ( inlineMode )
	{
		// Everything on the caller; the inline role services every lane.
		Prepare( rs, graph, opts, /*inlineMode=*/true, /*haveBlocking=*/false );
		InlineLoop( rs );
		rs.stats.inlineRun = true;
		if ( stats )
			*stats = rs.stats;
		return Collect( rs );
	}

	// No blocking lane: the pumping caller services BlockingIO work, as with
	// a ParallelExecutor that has no blocking workers.
	Prepare( rs, graph, opts, /*inlineMode=*/false, /*haveBlocking=*/false );
	const Role callerRole = rs.mainPumps ? Role::Main : Role::Helper;
	{
		std::unique_lock<std::mutex> lk( rs.mtx );
		rs.tasks = m_backend;
		rs.taskBudget = (uint32_t)workers;
		rs.stats.budget = rs.taskBudget;
		// The caller enters its service loop next and takes the first job it
		// may run, so it counts as a servicer here: a lone ready job, a chain
		// or a one-participant batch posts no runner at all.
		DispatchAndPost( rs, lk, callerRole );
	}
	ServiceLoop( rs, callerRole );

	// Every job is resolved. Wait for posts still in flight, then withdraw the
	// runners that never started and wait for the rest to return, so no task
	// touches this run after Execute returns.
	std::vector<void *> tickets;
	{
		std::unique_lock<std::mutex> lk( rs.mtx );
		rs.postsDone.wait( lk,
		    [&]
		    {
			    return rs.posting == 0 && rs.toPost == 0;
		    } );
		tickets.swap( rs.tickets );
	}
	uint32_t withdrawn = 0;
	for ( void *ticket : tickets )
	{
		if ( !m_backend->SettleTask( ticket ) )
			++withdrawn;
	}
	{
		std::lock_guard<std::mutex> lk( rs.mtx );
		rs.stats.tasksWithdrawn = withdrawn;
		rs.tasks = nullptr;
		if ( stats )
			*stats = rs.stats;
	}
	tickets.clear();
	rs.tickets.swap( tickets ); // keep the capacity for the next run
	return Collect( rs );
}

} // namespace jobsystem
