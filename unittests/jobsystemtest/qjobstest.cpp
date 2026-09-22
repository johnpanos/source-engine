//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Q-JOBS independent / adversarial harness (RFC 0003 "Independent model
//          and adversarial execution"; RFC 0005 Q-JOBS). This suite deliberately
//          does NOT trust the production executor by running its own serial path:
//
//            1. Independent reference model. A pure fixpoint over an abstract
//               graph spec computes each job's expected terminal state without
//               touching any production executor. Order-invariance of that model
//               is proven by re-deriving it under many seeded topological orders.
//
//            2. Equivalence. The Deterministic and Parallel production executors
//               are compared against the model across seeded graphs, worker
//               counts, and repetitions, with exactly-once run counts and a
//               producer->consumer publication read-check carried in the job
//               bodies (a consumer that observes an unpublished producer slot
//               fails, so a real publication defect shows up as a state mismatch).
//
//            3. Negative executors. Deliberately broken IGraphExecutors (drop a
//               job, run one twice, ignore dependency order, never cancel) are run
//               through the SAME comparator, which MUST flag each one. This proves
//               the comparator detects defects rather than passing vacuously.
//
//            4. External-completion races. registration-before/after-event,
//               repeated notification, and a Complete/Cancel thread race, all with
//               exactly-once continuation firing.
//
//          On a mismatch the harness prints the seed and the first divergent job
//          (first-divergence diagnostics), as RFC 0005 requires.
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

static int g_checks = 0;
static int g_failures = 0;
static const char *g_curTest = "";

#define CHECK( cond )                                                                              \
	do                                                                                             \
	{                                                                                              \
		++g_checks;                                                                                \
		if ( !( cond ) )                                                                           \
		{                                                                                          \
			++g_failures;                                                                          \
			std::printf( "  FAIL [%s] %s:%d: %s\n", g_curTest, __FILE__, __LINE__, #cond );        \
		}                                                                                          \
	} while ( 0 )

#define RUN( fn )                                                                                  \
	do                                                                                             \
	{                                                                                              \
		g_curTest = #fn;                                                                           \
		std::printf( "- %s\n", #fn );                                                              \
		fn();                                                                                      \
	} while ( 0 )

//-----------------------------------------------------------------------------
// Abstract graph spec + independent reference model
//-----------------------------------------------------------------------------

struct SpecEdge
{
	uint32_t producer;
	uint32_t consumer;
	bool success;
};

struct GraphSpec
{
	uint32_t n = 0;
	std::vector<SpecEdge> edges;
	std::vector<char> willFail; // per job: job reports failure when it runs
	bool globalCancel = false;
};

// Independent model: derive each job's terminal state by fixpoint, using only the
// abstract spec. A job is Canceled if the scope is canceled or any Success prereq
// did not Succeed; otherwise it runs and is Failed iff willFail, else Succeeded.
// Terminal prereqs never gate on success, only on the producer being terminal
// (always true once we resolve in dependency order). This is intentionally a
// different implementation from any production executor.
static std::vector<JobState> ModelStates( const GraphSpec &s )
{
	std::vector<std::vector<SpecEdge>> incoming( s.n );
	std::vector<uint32_t> indeg( s.n, 0 );
	std::vector<std::vector<uint32_t>> out( s.n );
	for ( const SpecEdge &e : s.edges )
	{
		incoming[e.consumer].push_back( e );
		out[e.producer].push_back( e.consumer );
		indeg[e.consumer]++;
	}
	std::vector<JobState> st( s.n, JobState::Admitted );
	std::vector<uint32_t> work = indeg;
	std::vector<char> done( s.n, 0 );
	// Resolve in a dependency-respecting order (Kahn); the model's own order is
	// irrelevant to the result, which Test_ModelOrderInvariance verifies.
	for ( uint32_t placed = 0; placed < s.n; )
	{
		uint32_t pick = 0xFFFFFFFFu;
		for ( uint32_t i = 0; i < s.n; ++i )
			if ( !done[i] && work[i] == 0 )
			{
				pick = i;
				break;
			}
		if ( pick == 0xFFFFFFFFu )
			break; // acyclic specs only
		done[pick] = 1;
		++placed;

		bool cancel = s.globalCancel;
		for ( const SpecEdge &e : incoming[pick] )
			if ( e.success && st[e.producer] != JobState::Succeeded )
				cancel = true;

		if ( cancel )
			st[pick] = JobState::Canceled;
		else if ( s.willFail[pick] )
			st[pick] = JobState::Failed;
		else
			st[pick] = JobState::Succeeded;

		for ( uint32_t c : out[pick] )
			--work[c];
	}
	return st;
}

// The same model re-derived under a seeded permutation of the ready set, to prove
// the terminal states do not depend on resolution order.
static std::vector<JobState> ModelStatesSeededOrder( const GraphSpec &s, uint32_t seed )
{
	std::vector<std::vector<SpecEdge>> incoming( s.n );
	std::vector<uint32_t> indeg( s.n, 0 );
	std::vector<std::vector<uint32_t>> out( s.n );
	for ( const SpecEdge &e : s.edges )
	{
		incoming[e.consumer].push_back( e );
		out[e.producer].push_back( e.consumer );
		indeg[e.consumer]++;
	}
	std::vector<JobState> st( s.n, JobState::Admitted );
	std::vector<uint32_t> work = indeg;
	std::vector<char> done( s.n, 0 );
	auto rnd = [&]
	{
		seed = seed * 1103515245u + 12345u;
		return seed >> 16;
	};
	for ( uint32_t placed = 0; placed < s.n; )
	{
		std::vector<uint32_t> ready;
		for ( uint32_t i = 0; i < s.n; ++i )
			if ( !done[i] && work[i] == 0 )
				ready.push_back( i );
		if ( ready.empty() )
			break;
		uint32_t pick = ready[rnd() % ready.size()]; // random ready choice
		done[pick] = 1;
		++placed;

		bool cancel = s.globalCancel;
		for ( const SpecEdge &e : incoming[pick] )
			if ( e.success && st[e.producer] != JobState::Succeeded )
				cancel = true;
		if ( cancel )
			st[pick] = JobState::Canceled;
		else if ( s.willFail[pick] )
			st[pick] = JobState::Failed;
		else
			st[pick] = JobState::Succeeded;
		for ( uint32_t c : out[pick] )
			--work[c];
	}
	return st;
}

//-----------------------------------------------------------------------------
// Spec -> SealedGraph with instrumented bodies (run count + publication check)
//-----------------------------------------------------------------------------

struct Instrumentation
{
	std::vector<std::atomic<int>> runCount; // per job: times its body ran
	std::vector<std::atomic<int>> slot;     // per job: published value (id+1)
	std::atomic<int> pubViolations{ 0 };    // consumer saw unpublished producer

	explicit Instrumentation( uint32_t n ) : runCount( n ), slot( n )
	{
		for ( uint32_t i = 0; i < n; ++i )
		{
			runCount[i].store( 0 );
			slot[i].store( 0 );
		}
	}
};

static SealedGraph BuildSealed( const GraphSpec &s, Instrumentation &inst, bool &ok )
{
	JobGraphBuilder b;
	std::vector<JobHandle> h( s.n );
	// Producers a job must observe as published (its Success prereqs).
	std::vector<std::vector<uint32_t>> mustSee( s.n );
	for ( const SpecEdge &e : s.edges )
		if ( e.success )
			mustSee[e.consumer].push_back( e.producer );

	for ( uint32_t i = 0; i < s.n; ++i )
	{
		JobDesc d;
		d.name = "q";
		const bool fail = s.willFail[i] != 0;
		const std::vector<uint32_t> see = mustSee[i];
		d.function = [i, fail, see, &inst]( JobRunContext &ctx )
		{
			// Publication read-check: every Success producer must already be visible.
			for ( uint32_t p : see )
				if ( inst.slot[p].load( std::memory_order_relaxed ) == 0 )
					inst.pubViolations.fetch_add( 1, std::memory_order_relaxed );
			inst.runCount[i].fetch_add( 1, std::memory_order_relaxed );
			inst.slot[i].store( (int)i + 1, std::memory_order_relaxed ); // publish
			if ( fail )
				ctx.Fail();
		};
		h[i] = b.AddJob( d );
	}
	for ( const SpecEdge &e : s.edges )
		b.AddDependency( h[e.producer], h[e.consumer],
		    e.success ? DependencyKind::Success : DependencyKind::Terminal );

	auto r = b.Seal();
	ok = r.HasValue();
	return ok ? std::move( r.Value() ) : SealedGraph{};
}

// Deterministic pseudo-random acyclic spec: edges only go from lower to higher id.
static GraphSpec RandomSpec( uint32_t seed, uint32_t n, bool allowFail, bool globalCancel )
{
	GraphSpec s;
	s.n = n;
	s.willFail.assign( n, 0 );
	s.globalCancel = globalCancel;
	auto rnd = [&]
	{
		seed = seed * 1103515245u + 12345u;
		return seed >> 16;
	};
	for ( uint32_t c = 1; c < n; ++c )
	{
		uint32_t deps = rnd() % 3;
		for ( uint32_t k = 0; k < deps; ++k )
		{
			uint32_t p = rnd() % c;            // strictly earlier -> acyclic
			bool success = ( rnd() % 4 ) != 0; // 3:1 Success:Terminal
			s.edges.push_back( SpecEdge{ p, c, success } );
		}
		if ( allowFail && ( rnd() % 7 ) == 0 )
			s.willFail[c] = 1;
	}
	return s;
}

// Compare a produced result against the model; print first divergence with seed.
static bool MatchesModel( const std::vector<JobState> &model, const std::vector<JobState> &got,
    uint32_t seed, const char *who )
{
	if ( model.size() != got.size() )
		return false;
	for ( uint32_t i = 0; i < model.size(); ++i )
		if ( model[i] != got[i] )
		{
			std::printf( "    [%s] seed=%u first divergence at job %u: model=%d got=%d\n", who,
			    seed, i, (int)model[i], (int)got[i] );
			return false;
		}
	return true;
}

//-----------------------------------------------------------------------------
// Negative executors: deliberately broken; the comparator MUST flag them.
//-----------------------------------------------------------------------------

// Skips every 3rd job (leaves it non-terminal).
class DropExecutor : public IGraphExecutor
{
public:
	RunResult Execute( const SealedGraph &g, const RunOptions &opts ) override
	{
		RunResult r;
		r.states.assign( g.JobCount(), JobState::Admitted );
		for ( uint32_t id : g.TopoOrder() )
		{
			if ( id % 3 == 2 )
				continue; // drop
			JobRunContext ctx( opts.frame, id );
			if ( g.GetJob( id ).function )
				g.GetJob( id ).function( ctx );
			r.states[id] = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
		}
		return r;
	}
};

// Runs one designated job's body twice (duplicate completion).
class DuplicateExecutor : public IGraphExecutor
{
public:
	RunResult Execute( const SealedGraph &g, const RunOptions &opts ) override
	{
		RunResult r;
		r.states.assign( g.JobCount(), JobState::Admitted );
		for ( uint32_t id : g.TopoOrder() )
		{
			JobRunContext ctx( opts.frame, id );
			if ( g.GetJob( id ).function )
				g.GetJob( id ).function( ctx );
			if ( id == 0 && g.GetJob( id ).function )
			{
				JobRunContext ctx2( opts.frame, id );
				g.GetJob( id ).function( ctx2 ); // run again
			}
			r.states[id] = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
		}
		return r;
	}
};

// Runs jobs in reverse topological order (consumers before producers).
class MisorderExecutor : public IGraphExecutor
{
public:
	RunResult Execute( const SealedGraph &g, const RunOptions &opts ) override
	{
		RunResult r;
		r.states.assign( g.JobCount(), JobState::Admitted );
		const std::vector<uint32_t> &topo = g.TopoOrder();
		for ( auto it = topo.rbegin(); it != topo.rend(); ++it )
		{
			JobRunContext ctx( opts.frame, *it );
			if ( g.GetJob( *it ).function )
				g.GetJob( *it ).function( ctx );
			r.states[*it] = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
		}
		return r;
	}
};

// Never cancels: runs every job even when a Success prereq failed.
class NoCancelExecutor : public IGraphExecutor
{
public:
	RunResult Execute( const SealedGraph &g, const RunOptions &opts ) override
	{
		RunResult r;
		r.states.assign( g.JobCount(), JobState::Admitted );
		for ( uint32_t id : g.TopoOrder() )
		{
			JobRunContext ctx( opts.frame, id );
			if ( g.GetJob( id ).function )
				g.GetJob( id ).function( ctx );
			r.states[id] = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
		}
		return r;
	}
};

//-----------------------------------------------------------------------------
// Tests
//-----------------------------------------------------------------------------

static void Test_ModelOrderInvariance()
{
	// The independent model's terminal states must not depend on resolution order.
	for ( uint32_t seed = 1; seed <= 200; ++seed )
	{
		GraphSpec s = RandomSpec( seed, 6 + seed % 20, /*allowFail=*/true, /*cancel=*/false );
		std::vector<JobState> base = ModelStates( s );
		for ( uint32_t o = 0; o < 6; ++o )
		{
			std::vector<JobState> alt = ModelStatesSeededOrder( s, seed * 31 + o );
			CHECK( base == alt );
		}
	}
}

static void Test_DeterministicMatchesModel()
{
	DeterministicExecutor ex;
	for ( uint32_t seed = 1; seed <= 300; ++seed )
	{
		GraphSpec s = RandomSpec( seed, 5 + seed % 25, true, ( seed % 11 ) == 0 );
		Instrumentation inst( s.n );
		bool ok = false;
		SealedGraph g = BuildSealed( s, inst, ok );
		CHECK( ok );
		std::atomic<bool> cancel{ s.globalCancel != 0 };
		RunOptions opts;
		opts.cancel = &cancel;
		RunResult r = ex.Execute( g, opts );
		CHECK( MatchesModel( ModelStates( s ), r.states, seed, "deterministic" ) );
		CHECK( inst.pubViolations.load() == 0 );
	}
}

static void Test_ParallelMatchesModelUnderStress()
{
	for ( uint32_t seed = 1; seed <= 200; ++seed )
	{
		GraphSpec s = RandomSpec( seed, 8 + seed % 30, true, ( seed % 13 ) == 0 );
		std::vector<JobState> model = ModelStates( s );
		for ( int workers = 0; workers <= 4; ++workers )
		{
			Instrumentation inst( s.n );
			bool ok = false;
			SealedGraph g = BuildSealed( s, inst, ok );
			CHECK( ok );
			std::atomic<bool> cancel{ s.globalCancel != 0 };
			RunOptions opts;
			opts.cancel = &cancel;
			ParallelExecutor ex( workers );
			RunResult r = ex.Execute( g, opts );
			CHECK( MatchesModel( model, r.states, seed, "parallel" ) );
			// Exactly-once: every job that the model says reached a run (not canceled)
			// ran exactly once; canceled jobs never ran.
			bool exactlyOnce = true;
			for ( uint32_t i = 0; i < s.n; ++i )
			{
				const int rc = inst.runCount[i].load();
				const bool ranExpected =
				    ( model[i] == JobState::Succeeded || model[i] == JobState::Failed );
				if ( ranExpected ? rc != 1 : rc != 0 )
					exactlyOnce = false;
			}
			CHECK( exactlyOnce );
			CHECK( inst.pubViolations.load() == 0 ); // publication held under concurrency
		}
	}
}

// Each broken executor must be CAUGHT by the comparator (or the run-count /
// publication oracle). This proves the suite detects defects.
static void Test_NegativeExecutorsAreDetected()
{
	// A spec with a fan-out chain and a failing node so cancellation matters.
	GraphSpec s;
	s.n = 6;
	s.willFail.assign( 6, 0 );
	s.willFail[1] = 1; // job 1 fails
	s.edges.push_back( { 0, 1, true } );
	s.edges.push_back( { 1, 2, true } ); // 2 depends on failing 1 -> canceled
	s.edges.push_back( { 0, 3, true } );
	s.edges.push_back( { 3, 4, true } );
	s.edges.push_back( { 4, 5, true } );
	std::vector<JobState> model = ModelStates( s );

	// Sanity: a correct executor matches (non-vacuous baseline).
	{
		Instrumentation inst( s.n );
		bool ok = false;
		SealedGraph g = BuildSealed( s, inst, ok );
		DeterministicExecutor ex;
		RunResult r = ex.Execute( g, RunOptions{} );
		CHECK( ok );
		CHECK( MatchesModel( model, r.states, 0, "correct-baseline" ) );
	}

	// Drop: a job is left non-terminal -> mismatch detected.
	{
		Instrumentation inst( s.n );
		bool ok = false;
		SealedGraph g = BuildSealed( s, inst, ok );
		DropExecutor ex;
		RunResult r = ex.Execute( g, RunOptions{} );
		bool matched = ( model == r.states );
		CHECK( !matched ); // comparator flags the drop
	}

	// Duplicate: job 0 runs twice -> run-count oracle catches it.
	{
		Instrumentation inst( s.n );
		bool ok = false;
		SealedGraph g = BuildSealed( s, inst, ok );
		DuplicateExecutor ex;
		RunResult r = ex.Execute( g, RunOptions{} );
		(void)r;
		CHECK( inst.runCount[0].load() > 1 ); // exactly-once oracle catches it
	}

	// Misorder: consumers before producers -> publication violations AND state
	// mismatch, both detected.
	{
		Instrumentation inst( s.n );
		bool ok = false;
		SealedGraph g = BuildSealed( s, inst, ok );
		MisorderExecutor ex;
		RunResult r = ex.Execute( g, RunOptions{} );
		bool matched = ( model == r.states );
		CHECK( !matched || inst.pubViolations.load() > 0 ); // at least one oracle fires
		CHECK( inst.pubViolations.load() > 0 );             // publication defect seen
	}

	// NoCancel: job 2 should be Canceled (its Success prereq 1 failed) but runs.
	{
		Instrumentation inst( s.n );
		bool ok = false;
		SealedGraph g = BuildSealed( s, inst, ok );
		NoCancelExecutor ex;
		RunResult r = ex.Execute( g, RunOptions{} );
		CHECK( model[2] == JobState::Canceled );
		CHECK( r.states[2] != JobState::Canceled ); // broken executor ran it
		CHECK( model != r.states );                 // mismatch detected
	}
}

//-----------------------------------------------------------------------------
// External completion adversarial races
//-----------------------------------------------------------------------------

static void Test_ExternalCompletionExactlyOnce()
{
	// Register before event: continuation fires once with the terminal result.
	{
		ExternalCompletion t;
		std::atomic<int> fired{ 0 };
		CompletionResult saw = CompletionResult::Pending;
		bool later = t.OnComplete(
		    [&]( CompletionResult r )
		    {
			    fired++;
			    saw = r;
		    } );
		CHECK( later );         // stored to fire later
		CHECK( t.Complete() );  // performed the transition
		CHECK( !t.Complete() ); // repeated notification is a no-op
		CHECK( !t.Cancel() );   // terminal is final
		CHECK( fired.load() == 1 && saw == CompletionResult::Completed );
	}
	// Completion before registration: continuation fires immediately, once.
	{
		ExternalCompletion t;
		std::atomic<int> fired{ 0 };
		CHECK( t.Cancel() );
		bool later = t.OnComplete(
		    [&]( CompletionResult r )
		    {
			    fired++;
			    CHECK( r == CompletionResult::Canceled );
		    } );
		CHECK( !later ); // fired immediately
		CHECK( fired.load() == 1 );
		CHECK( t.Poll() == CompletionResult::Canceled );
	}
}

static void Test_ExternalCompletionCompleteCancelRace()
{
	// Many threads race Complete vs Cancel: exactly one transition wins, the
	// continuation fires exactly once with the winning result, all Wait()ers agree.
	for ( int rep = 0; rep < 200; ++rep )
	{
		ExternalCompletion t;
		std::atomic<int> cbFired{ 0 };
		CompletionResult cbResult = CompletionResult::Pending;
		t.OnComplete(
		    [&]( CompletionResult r )
		    {
			    cbResult = r;
			    cbFired.fetch_add( 1 );
		    } );

		std::atomic<int> transitions{ 0 };
		std::vector<std::thread> ths;
		std::vector<CompletionResult> waited( 4 );
		for ( int i = 0; i < 6; ++i )
		{
			ths.emplace_back(
			    [&, i]
			    {
				    if ( i % 2 == 0 )
				    {
					    if ( t.Complete() )
						    transitions.fetch_add( 1 );
				    }
				    else
				    {
					    if ( t.Cancel() )
						    transitions.fetch_add( 1 );
				    }
			    } );
		}
		for ( int i = 0; i < 4; ++i )
			ths.emplace_back(
			    [&, i]
			    {
				    waited[i] = t.Wait();
			    } );
		for ( auto &th : ths )
			th.join();

		CHECK( transitions.load() == 1 ); // exactly one winner
		CHECK( cbFired.load() == 1 );     // continuation fired exactly once
		CompletionResult term = t.Poll();
		CHECK( term != CompletionResult::Pending );
		CHECK( cbResult == term );
		for ( CompletionResult w : waited )
			CHECK( w == term ); // all waiters agree
	}
}

int main()
{
	std::printf( "qjobstest (RFC 0003 / RFC 0005 Q-JOBS independent + adversarial harness)\n" );
	RUN( Test_ModelOrderInvariance );
	RUN( Test_DeterministicMatchesModel );
	RUN( Test_ParallelMatchesModelUnderStress );
	RUN( Test_NegativeExecutorsAreDetected );
	RUN( Test_ExternalCompletionExactlyOnce );
	RUN( Test_ExternalCompletionCompleteCancelRace );
	std::printf( "\n%d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
