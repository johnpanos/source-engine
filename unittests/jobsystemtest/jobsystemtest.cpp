//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance tests for the dependency-aware job system (RFC 0003,
//          Phase B/C). Covers the RFC "Scheduler conformance tests" list:
//          chains, fan-out/in, independent roots, empty graphs, exactly-once,
//          cycles (explicit and sequence-induced), resource conflicts, stale
//          handles, failure/cancel propagation, cleanup continuations, and
//          three-mode (serial vs parallel) terminal-state equivalence under
//          stress. Runs standalone (own main) so it is executable without the
//          engine build; it is also wired into the Waf `tests` group.
//
//=============================================================================//

#include "jobsystem/job_graph.h"
#include "jobsystem/graph_executor.h"
#include "jobsystem/parallel_executor.h"
#include "jobsystem/external_completion.h"

#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

using namespace jobsystem;

//-----------------------------------------------------------------------------
// Tiny test harness
//-----------------------------------------------------------------------------
static int g_checks = 0;
static int g_failures = 0;
static const char *g_curTest = "";

#define CHECK( cond ) do { \
	++g_checks; \
	if ( !(cond) ) { ++g_failures; \
		std::printf( "  FAIL [%s] %s:%d: %s\n", g_curTest, __FILE__, __LINE__, #cond ); } \
	} while (0)

#define RUN( fn ) do { g_curTest = #fn; std::printf( "- %s\n", #fn ); fn(); } while (0)

// Records the order in which jobs reach the Running state.
class OrderSink : public ITraceSink
{
public:
	std::vector<uint32_t> runOrder;
	void OnJobState( uint32_t id, const char *, ExecutorToken, JobState s ) override
	{
		if ( s == JobState::Running ) runOrder.push_back( id );
	}
};

static SealedGraph MustSeal( JobGraphBuilder &b )
{
	auto r = b.Seal();
	CHECK( r.HasValue() );
	return r.HasValue() ? std::move( r.Value() ) : SealedGraph{};
}

//-----------------------------------------------------------------------------
// Phase B: builder / validator
//-----------------------------------------------------------------------------

static void Test_EmptyGraph()
{
	JobGraphBuilder b;
	SealedGraph g = MustSeal( b );
	CHECK( g.JobCount() == 0 );
	DeterministicExecutor ex;
	RunResult r = ex.Execute( g, RunOptions{} );
	CHECK( r.executed == 0 && r.AllSucceeded() );
}

static void Test_ChainOrderAndExactlyOnce()
{
	JobGraphBuilder b;
	std::atomic<int> counter{ 0 };
	std::vector<int> runs( 3, 0 );
	auto mk = [&]( int idx ) {
		JobDesc d; d.name = "chain";
		d.function = [&, idx]( JobRunContext & ) { runs[idx]++; counter++; };
		return b.AddJob( d );
	};
	JobHandle a = mk( 0 ), c = mk( 1 ), e = mk( 2 );
	b.AddDependency( a, c );
	b.AddDependency( c, e );
	SealedGraph g = MustSeal( b );

	OrderSink sink; RunOptions opts; opts.trace = &sink;
	DeterministicExecutor ex;
	RunResult r = ex.Execute( g, opts );
	CHECK( r.succeeded == 3 && r.executed == 3 );
	CHECK( counter.load() == 3 );
	for ( int x : runs ) CHECK( x == 1 );            // exactly once each
	CHECK( sink.runOrder.size() == 3 );
	CHECK( sink.runOrder[0] == a.id && sink.runOrder[1] == c.id && sink.runOrder[2] == e.id );
}

static void Test_FanOutFanIn()
{
	JobGraphBuilder b;
	std::atomic<int> n{ 0 };
	auto mk = [&]( const char *nm ) { JobDesc d; d.name = nm;
		d.function = [&]( JobRunContext & ) { n++; }; return b.AddJob( d ); };
	JobHandle root = mk( "root" );
	JobHandle x = mk( "x" ), y = mk( "y" ), z = mk( "z" );
	JobHandle join = mk( "join" );
	b.AddDependency( root, x ); b.AddDependency( root, y ); b.AddDependency( root, z );
	b.AddDependency( x, join ); b.AddDependency( y, join ); b.AddDependency( z, join );
	SealedGraph g = MustSeal( b );

	OrderSink sink; RunOptions opts; opts.trace = &sink;
	DeterministicExecutor ex;
	RunResult r = ex.Execute( g, opts );
	CHECK( r.succeeded == 5 );
	CHECK( n.load() == 5 );
	// root runs first, join runs last.
	CHECK( sink.runOrder.front() == root.id );
	CHECK( sink.runOrder.back() == join.id );
}

static void Test_IndependentRoots()
{
	JobGraphBuilder b;
	std::atomic<int> n{ 0 };
	for ( int i = 0; i < 5; ++i ) { JobDesc d; d.name = "root";
		d.function = [&]( JobRunContext & ) { n++; }; b.AddJob( d ); }
	SealedGraph g = MustSeal( b );
	DeterministicExecutor ex;
	RunResult r = ex.Execute( g, RunOptions{} );
	CHECK( r.succeeded == 5 && n.load() == 5 );
}

static void Test_RejectCycle()
{
	JobGraphBuilder b;
	JobDesc d; d.name = "n";
	JobHandle a = b.AddJob( d ), c = b.AddJob( d ), e = b.AddJob( d );
	b.AddDependency( a, c ); b.AddDependency( c, e ); b.AddDependency( e, a );
	auto r = b.Seal();
	CHECK( !r.HasValue() );
	CHECK( r.HasValue() || r.Error().code == GraphErrorCode::Cycle );
}

static void Test_RejectSequenceInducedCycle()
{
	// Two jobs on the same sequence lane get an implicit A->B ordering edge;
	// an explicit B->A edge then forms a cycle the validator must catch.
	JobGraphBuilder b;
	JobDesc d; d.name = "seq"; d.executor = Executor::Sequence( 0 );
	JobHandle a = b.AddJob( d );
	JobHandle c = b.AddJob( d );
	b.AddDependency( c, a ); // opposes the implicit a->c sequence edge
	auto r = b.Seal();
	CHECK( !r.HasValue() );
	CHECK( r.HasValue() || r.Error().code == GraphErrorCode::Cycle );
}

static void Test_RejectSelfDependency()
{
	JobGraphBuilder b; JobDesc d; d.name = "n";
	JobHandle a = b.AddJob( d );
	b.AddDependency( a, a );
	auto r = b.Seal();
	CHECK( !r.HasValue() && ( r.HasValue() || r.Error().code == GraphErrorCode::SelfDependency ) );
}

static void Test_RejectStaleHandle()
{
	JobGraphBuilder b; JobDesc d; d.name = "n";
	b.AddJob( d );
	b.AddDependency( JobHandle{ 0 }, JobHandle{ 99 } ); // 99 does not exist
	auto r = b.Seal();
	CHECK( !r.HasValue() && ( r.HasValue() || r.Error().code == GraphErrorCode::InvalidHandle ) );
}

static void Test_RejectEmptyName()
{
	JobGraphBuilder b; JobDesc d; d.name = ""; // empty
	b.AddJob( d );
	auto r = b.Seal();
	CHECK( !r.HasValue() && ( r.HasValue() || r.Error().code == GraphErrorCode::EmptyName ) );
}

static void Test_ResourceConflictRejected()
{
	// Two unordered writers to the same version must be rejected, not ordered.
	JobGraphBuilder b; JobDesc d; d.name = "w";
	JobHandle a = b.AddJob( d ), c = b.AddJob( d );
	ResourceVersion rv{ /*domain*/ 7, /*epoch*/ 1, /*partition*/ 0 };
	b.Write( a, rv ); b.Write( c, rv );
	auto r = b.Seal();
	CHECK( !r.HasValue() && ( r.HasValue() || r.Error().code == GraphErrorCode::ResourceConflict ) );
}

static void Test_ResourceOrderedWriteAccepted()
{
	// Ordered writer-before-reader on the same version is fine.
	JobGraphBuilder b; JobDesc d; d.name = "rw";
	JobHandle w = b.AddJob( d ), rd = b.AddJob( d );
	b.AddDependency( w, rd );
	ResourceVersion rv{ 7, 1, 0 };
	b.Write( w, rv ); b.Read( rd, rv );
	auto r = b.Seal();
	CHECK( r.HasValue() );
}

static void Test_ResourceReadReadAccepted()
{
	JobGraphBuilder b; JobDesc d; d.name = "rr";
	JobHandle a = b.AddJob( d ), c = b.AddJob( d );
	ResourceVersion rv{ 7, 1, 0 };
	b.Read( a, rv ); b.Read( c, rv ); // two immutable readers, unordered: ok
	auto r = b.Seal();
	CHECK( r.HasValue() );
}

static void Test_ResourceDisjointPartitionsAccepted()
{
	JobGraphBuilder b; JobDesc d; d.name = "part";
	JobHandle a = b.AddJob( d ), c = b.AddJob( d );
	b.Write( a, ResourceVersion{ 7, 1, 0 } );
	b.Write( c, ResourceVersion{ 7, 1, 1 } ); // different partition, disjoint
	auto r = b.Seal();
	CHECK( r.HasValue() );
}

static void Test_FailurePropagationAndCleanup()
{
	// A fails; B has a Success dep on A (must be canceled, never runs); C has a
	// Terminal dep on A (cleanup: must still run).
	JobGraphBuilder b;
	bool bRan = false, cRan = false;
	JobDesc da; da.name = "A"; da.function = [&]( JobRunContext &c ) { c.Fail(); };
	JobHandle a = b.AddJob( da );
	JobDesc db; db.name = "B"; db.function = [&]( JobRunContext & ) { bRan = true; };
	JobHandle bh = b.AddJob( db );
	JobDesc dc; dc.name = "C"; dc.function = [&]( JobRunContext & ) { cRan = true; };
	JobHandle ch = b.AddJob( dc );
	b.AddDependency( a, bh, DependencyKind::Success );
	b.AddDependency( a, ch, DependencyKind::Terminal );
	SealedGraph g = MustSeal( b );

	DeterministicExecutor ex;
	RunResult r = ex.Execute( g, RunOptions{} );
	CHECK( r.states[a.id] == JobState::Failed );
	CHECK( r.states[bh.id] == JobState::Canceled );
	CHECK( r.states[ch.id] == JobState::Succeeded );
	CHECK( !bRan );
	CHECK( cRan );
}

static void Test_TransitiveCancelCascade()
{
	// A fails; B(Success on A) canceled; D(Success on B) also canceled.
	JobGraphBuilder b;
	JobDesc da; da.name = "A"; da.function = [&]( JobRunContext &c ) { c.Fail(); };
	JobHandle a = b.AddJob( da );
	JobDesc dgen; dgen.name = "n";
	JobHandle bh = b.AddJob( dgen ), dh = b.AddJob( dgen );
	b.AddDependency( a, bh );
	b.AddDependency( bh, dh );
	SealedGraph g = MustSeal( b );
	DeterministicExecutor ex;
	RunResult r = ex.Execute( g, RunOptions{} );
	CHECK( r.states[bh.id] == JobState::Canceled );
	CHECK( r.states[dh.id] == JobState::Canceled );
	CHECK( r.executed == 1 ); // only A ran
}

static void Test_ScopeCancellation()
{
	JobGraphBuilder b;
	std::atomic<int> n{ 0 };
	for ( int i = 0; i < 4; ++i ) { JobDesc d; d.name = "n";
		d.function = [&]( JobRunContext & ) { n++; }; b.AddJob( d ); }
	SealedGraph g = MustSeal( b );
	std::atomic<bool> cancel{ true };
	RunOptions opts; opts.cancel = &cancel;
	DeterministicExecutor ex;
	RunResult r = ex.Execute( g, opts );
	CHECK( r.canceled == 4 && r.executed == 0 && n.load() == 0 );
}

//-----------------------------------------------------------------------------
// Phase C: parallel executor equivalence + stress
//-----------------------------------------------------------------------------

// Build a moderately complex diamond-heavy graph deterministically from a seed.
static void BuildStressGraph( JobGraphBuilder &b, std::vector<int> &order, int layers, int width )
{
	std::vector<std::vector<JobHandle>> layer;
	int counter = 0;
	for ( int L = 0; L < layers; ++L )
	{
		std::vector<JobHandle> row;
		for ( int w = 0; w < width; ++w )
		{
			JobDesc d; d.name = "s";
			int idx = counter++;
			d.function = [&order, idx]( JobRunContext & ) { order[idx]++; };
			JobHandle h = b.AddJob( d );
			row.push_back( h );
			if ( L > 0 )
			{
				// depend on two nodes from the previous layer
				b.AddDependency( layer[L - 1][w % width], h );
				b.AddDependency( layer[L - 1][( w + 1 ) % width], h );
			}
		}
		layer.push_back( row );
	}
	order.assign( counter, 0 );
}

static void Test_ParallelEquivalence()
{
	const int layers = 6, width = 8;
	// Serial reference.
	std::vector<int> serialRuns;
	RunResult serial;
	{
		JobGraphBuilder b; BuildStressGraph( b, serialRuns, layers, width );
		SealedGraph g = MustSeal( b );
		DeterministicExecutor ex; serial = ex.Execute( g, RunOptions{} );
	}
	// Parallel, several worker counts and repeated runs, must match terminal states.
	for ( int workers = 1; workers <= 4; ++workers )
	{
		for ( int rep = 0; rep < 8; ++rep )
		{
			std::vector<int> parRuns;
			JobGraphBuilder b; BuildStressGraph( b, parRuns, layers, width );
			SealedGraph g = MustSeal( b );
			ParallelExecutor ex( workers );
			RunResult par = ex.Execute( g, RunOptions{} );
			CHECK( par.succeeded == serial.succeeded );
			CHECK( par.states == serial.states );
			bool allOnce = true;
			for ( int x : parRuns ) if ( x != 1 ) allOnce = false;
			CHECK( allOnce ); // exactly-once under concurrency
		}
	}
}

static void Test_ParallelFailurePropagation()
{
	for ( int workers = 1; workers <= 4; ++workers )
	{
		JobGraphBuilder b;
		JobDesc da; da.name = "A"; da.function = [&]( JobRunContext &c ) { c.Fail(); };
		JobHandle a = b.AddJob( da );
		JobDesc dgen; dgen.name = "n";
		JobHandle bh = b.AddJob( dgen ), dh = b.AddJob( dgen );
		std::atomic<bool> cleanupRan{ false };
		JobDesc dc; dc.name = "cleanup"; dc.function = [&]( JobRunContext & ) { cleanupRan = true; };
		JobHandle ch = b.AddJob( dc );
		b.AddDependency( a, bh, DependencyKind::Success );
		b.AddDependency( bh, dh, DependencyKind::Success );
		b.AddDependency( a, ch, DependencyKind::Terminal );
		SealedGraph g = MustSeal( b );
		ParallelExecutor ex( workers );
		RunResult r = ex.Execute( g, RunOptions{} );
		CHECK( r.states[a.id] == JobState::Failed );
		CHECK( r.states[bh.id] == JobState::Canceled );
		CHECK( r.states[dh.id] == JobState::Canceled );
		CHECK( r.states[ch.id] == JobState::Succeeded );
		CHECK( cleanupRan.load() );
	}
}

static void Test_ParallelZeroWorkersRunsInline()
{
	// Zero compute workers must still complete the graph on the caller.
	std::vector<int> runs;
	JobGraphBuilder b; BuildStressGraph( b, runs, 4, 4 );
	SealedGraph g = MustSeal( b );
	ParallelExecutor ex( 0 );
	RunResult r = ex.Execute( g, RunOptions{} );
	CHECK( r.succeeded == g.JobCount() );
	for ( int x : runs ) CHECK( x == 1 );
}

//-----------------------------------------------------------------------------
// Phase C: lane affinity, main-thread pump, blocking lane, stall detection
//-----------------------------------------------------------------------------

// A MainThread-affine job runs only on the calling (pumping) thread, never on a
// compute worker.
static void Test_MainThreadJobRunsOnCaller()
{
	const std::thread::id caller = std::this_thread::get_id();
	JobGraphBuilder b;
	std::atomic<bool> ran{ false };
	std::atomic<bool> onCaller{ false };
	JobDesc root; root.name = "root"; // a compute root so workers have work too
	JobHandle r = b.AddJob( root );
	JobDesc d; d.name = "ui"; d.executor = Executor::MainThread();
	d.function = [&]( JobRunContext & ) {
		ran = true;
		onCaller = ( std::this_thread::get_id() == caller );
	};
	JobHandle m = b.AddJob( d );
	b.AddDependency( r, m );
	SealedGraph g = MustSeal( b );

	ParallelExecutor ex( 3 ); // 3 compute workers + the caller pumps main
	RunResult res = ex.Execute( g, RunOptions{} );
	CHECK( res.AllSucceeded() && !res.stalled );
	CHECK( ran.load() );
	CHECK( onCaller.load() ); // affine job executed on the caller, not a worker
	CHECK( res.states[m.id] == JobState::Succeeded );
}

// Without a main-thread pump, a MainThread job cannot be serviced: the run is a
// detectable stall, not a hang, and the job plus its Success-dependents are left
// non-terminal (no false completion claim).
static void Test_MainThreadUnpumpedStalls()
{
	JobGraphBuilder b;
	std::atomic<bool> mRan{ false }, depRan{ false };
	JobDesc d; d.name = "ui"; d.executor = Executor::MainThread();
	d.function = [&]( JobRunContext & ) { mRan = true; };
	JobHandle m = b.AddJob( d );
	JobDesc dd; dd.name = "dep"; dd.function = [&]( JobRunContext & ) { depRan = true; };
	JobHandle dep = b.AddJob( dd );
	b.AddDependency( m, dep, DependencyKind::Success );
	SealedGraph g = MustSeal( b );

	RunOptions opts; opts.pumpMainThread = false;
	ParallelExecutor ex( 2 );
	RunResult res = ex.Execute( g, opts );
	CHECK( res.stalled );
	CHECK( res.unresolved == 2 );
	CHECK( !mRan.load() && !depRan.load() );
	CHECK( res.states[m.id] != JobState::Succeeded );   // left non-terminal
	CHECK( res.states[dep.id] != JobState::Succeeded );
	CHECK( !res.AllSucceeded() );
}

// A dedicated blocking lane services BlockingIO work. With no main pump and no
// compute-worker eligibility for BlockingIO, only the blocking lane can run it,
// so its success proves compute workers did not absorb the blocking job.
static void Test_BlockingLaneServicesBlockingIO()
{
	JobGraphBuilder b;
	std::atomic<int> n{ 0 };
	for ( int i = 0; i < 4; ++i )
	{
		JobDesc d; d.name = "io"; d.executor = Executor::BlockingIO();
		d.function = [&]( JobRunContext & ) { n++; };
		b.AddJob( d );
	}
	SealedGraph g = MustSeal( b );

	RunOptions opts; opts.pumpMainThread = false; // main does not help
	ParallelExecutor ex( 2, /*nBlocking=*/1 );
	RunResult res = ex.Execute( g, opts );
	CHECK( res.AllSucceeded() && !res.stalled );
	CHECK( n.load() == 4 );
}

// BlockingIO with no blocking lane and no main pump stalls; with a main pump the
// main thread services it as a fallback so compute workers stay free.
static void Test_BlockingIOStallAndMainFallback()
{
	auto build = []( JobGraphBuilder &b, std::atomic<int> &n ) {
		JobDesc d; d.name = "io"; d.executor = Executor::BlockingIO();
		d.function = [&n]( JobRunContext & ) { n++; };
		b.AddJob( d );
	};
	{
		std::atomic<int> n{ 0 };
		JobGraphBuilder b; build( b, n );
		SealedGraph g = MustSeal( b );
		RunOptions opts; opts.pumpMainThread = false;
		ParallelExecutor ex( 2, 0 );
		RunResult res = ex.Execute( g, opts );
		CHECK( res.stalled && res.unresolved == 1 && n.load() == 0 );
	}
	{
		std::atomic<int> n{ 0 };
		JobGraphBuilder b; build( b, n );
		SealedGraph g = MustSeal( b );
		RunOptions opts; opts.pumpMainThread = true; // main covers blocking
		ParallelExecutor ex( 2, 0 );
		RunResult res = ex.Execute( g, opts );
		CHECK( res.AllSucceeded() && n.load() == 1 );
	}
}

// Inline mode (zero compute workers) services every lane on the caller, so an
// affine or blocking job never stalls there.
static void Test_InlineModeServicesAllLanes()
{
	JobGraphBuilder b;
	std::atomic<int> n{ 0 };
	JobDesc a; a.name = "c"; a.function = [&]( JobRunContext & ) { n++; };
	JobDesc m; m.name = "ui"; m.executor = Executor::MainThread(); m.function = a.function;
	JobDesc io; io.name = "io"; io.executor = Executor::BlockingIO(); io.function = a.function;
	b.AddJob( a ); b.AddJob( m ); b.AddJob( io );
	SealedGraph g = MustSeal( b );
	RunOptions opts; opts.pumpMainThread = false; // irrelevant in inline mode
	ParallelExecutor ex( 0 );
	RunResult res = ex.Execute( g, opts );
	CHECK( res.AllSucceeded() && !res.stalled && n.load() == 3 );
}

// Mixed graph: affine + compute + blocking with full servicing must match the
// deterministic reference's terminal states across worker counts.
static void Test_MixedLaneEquivalence()
{
	auto buildMixed = []( JobGraphBuilder &b, std::vector<int> &runs ) {
		int idx = 0;
		auto mk = [&]( ExecutorToken e ) {
			JobDesc d; d.name = "mix"; d.executor = e;
			int i = idx++;
			d.function = [&runs, i]( JobRunContext & ) { runs[i]++; };
			return b.AddJob( d );
		};
		JobHandle g0 = mk( Executor::Compute() );
		JobHandle u  = mk( Executor::MainThread() );
		JobHandle io = mk( Executor::BlockingIO() );
		JobHandle s  = mk( Executor::Sequence( 0 ) );
		JobHandle join = mk( Executor::Compute() );
		b.AddDependency( g0, u ); b.AddDependency( g0, io );
		b.AddDependency( u, s ); b.AddDependency( io, s );
		b.AddDependency( s, join );
		runs.assign( idx, 0 );
	};

	std::vector<int> serialRuns;
	RunResult serial;
	{
		JobGraphBuilder b; buildMixed( b, serialRuns );
		SealedGraph g = MustSeal( b );
		DeterministicExecutor ex; serial = ex.Execute( g, RunOptions{} );
	}
	for ( int workers = 1; workers <= 4; ++workers )
	{
		for ( int blk = 0; blk <= 1; ++blk )
		{
			std::vector<int> parRuns;
			JobGraphBuilder b; buildMixed( b, parRuns );
			SealedGraph g = MustSeal( b );
			ParallelExecutor ex( workers, blk );
			RunResult par = ex.Execute( g, RunOptions{} ); // pump on by default
			CHECK( par.states == serial.states );
			CHECK( !par.stalled );
			bool once = true;
			for ( int x : parRuns ) if ( x != 1 ) once = false;
			CHECK( once );
		}
	}
}

// An external completion, adapted onto a BlockingIO job, gates a Success
// dependent through the graph's normal dependency/failure semantics: Complete()
// lets the dependent run; Cancel() fails the wait and cascade-cancels it.
static void Test_ExternalCompletionGatesGraph()
{
	for ( int cancel = 0; cancel <= 1; ++cancel )
	{
		ExternalCompletion token;
		std::atomic<bool> depRan{ false };
		JobGraphBuilder b;
		JobDesc io; io.name = "io"; io.executor = Executor::BlockingIO();
		io.function = MakeExternalWait( &token );
		JobHandle ioh = b.AddJob( io );
		JobDesc d; d.name = "consume";
		d.function = [&]( JobRunContext & ) { depRan = true; };
		JobHandle dh = b.AddJob( d );
		b.AddDependency( ioh, dh, DependencyKind::Success );
		SealedGraph g = MustSeal( b );

		// Signal from another thread so the blocking job actually parks and wakes.
		std::thread producer( [&]{ if ( cancel ) token.Cancel(); else token.Complete(); } );

		ParallelExecutor ex( 2, /*nBlocking=*/1 );
		RunResult res = ex.Execute( g, RunOptions{} );
		producer.join();

		if ( cancel )
		{
			CHECK( res.states[ioh.id] == JobState::Failed );
			CHECK( res.states[dh.id] == JobState::Canceled );
			CHECK( !depRan.load() );
		}
		else
		{
			CHECK( res.states[ioh.id] == JobState::Succeeded );
			CHECK( res.states[dh.id] == JobState::Succeeded );
			CHECK( depRan.load() );
		}
	}
}

int main()
{
	std::printf( "jobsystemtest (RFC 0003 Phase B/C conformance)\n" );

	RUN( Test_EmptyGraph );
	RUN( Test_ChainOrderAndExactlyOnce );
	RUN( Test_FanOutFanIn );
	RUN( Test_IndependentRoots );
	RUN( Test_RejectCycle );
	RUN( Test_RejectSequenceInducedCycle );
	RUN( Test_RejectSelfDependency );
	RUN( Test_RejectStaleHandle );
	RUN( Test_RejectEmptyName );
	RUN( Test_ResourceConflictRejected );
	RUN( Test_ResourceOrderedWriteAccepted );
	RUN( Test_ResourceReadReadAccepted );
	RUN( Test_ResourceDisjointPartitionsAccepted );
	RUN( Test_FailurePropagationAndCleanup );
	RUN( Test_TransitiveCancelCascade );
	RUN( Test_ScopeCancellation );
	RUN( Test_ParallelEquivalence );
	RUN( Test_ParallelFailurePropagation );
	RUN( Test_ParallelZeroWorkersRunsInline );
	RUN( Test_MainThreadJobRunsOnCaller );
	RUN( Test_MainThreadUnpumpedStalls );
	RUN( Test_BlockingLaneServicesBlockingIO );
	RUN( Test_BlockingIOStallAndMainFallback );
	RUN( Test_InlineModeServicesAllLanes );
	RUN( Test_MixedLaneEquivalence );
	RUN( Test_ExternalCompletionGatesGraph );

	std::printf( "\n%d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
