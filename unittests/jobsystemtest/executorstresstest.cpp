//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Executor reuse/concurrency stress (RFC 0003 Phase C, Q-JOBS). Seeded
//          random graphs with mixed lanes, failures, Terminal cleanup edges and
//          cancellation run on long-lived ParallelExecutor and PooledExecutor
//          instances and must match DeterministicExecutor's terminal states,
//          exactly-once execution and producer-before-consumer publication.
//          It also exercises the same executor from several threads at once
//          and from inside its own jobs (nested runs), plus copies, so worker
//          reuse cannot leak state between runs. DynamicScope is stressed with
//          children spawned concurrently from running children; outcomes are
//          replayed from the recorded dependencies. Batch graphs reused per
//          thread are exercised re-entrantly and past the cache cap. Run
//          under TSan as well.
//
//=============================================================================//

#include "jobsystem/dynamic_scope.h"
#include "jobsystem/graph_executor.h"
#include "jobsystem/job_graph.h"
#include "jobsystem/parallel_batch.h"
#include "jobsystem/parallel_executor.h"
#include "jobsystem/pooled_executor.h"
#include "jobsystem/worker_backend.h"
#include "testing/conformance_result.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using namespace jobsystem;

namespace
{

std::atomic<int> g_checks{ 0 };
std::atomic<int> g_failures{ 0 };

#define CHECK( condition )                                                                         \
	do                                                                                             \
	{                                                                                              \
		g_checks.fetch_add( 1, std::memory_order_relaxed );                                        \
		if ( !( condition ) )                                                                      \
		{                                                                                          \
			g_failures.fetch_add( 1, std::memory_order_relaxed );                                  \
			std::printf( "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition );                   \
		}                                                                                          \
	} while ( 0 )

struct Rng
{
	uint64_t s;
	uint32_t Next()
	{
		s = s * 6364136223846793005ull + 1442695040888963407ull;
		return (uint32_t)( s >> 33 );
	}
	uint32_t Below( uint32_t n ) { return n ? Next() % n : 0; }
	bool Chance( uint32_t percent ) { return Below( 100 ) < percent; }
};

// A persistent-thread backend (the caller participates), so pooled runs also
// reuse workers across waves and runs.
class PoolBackend final : public IWorkerBackend
{
public:
	explicit PoolBackend( int workers ) : m_workers( workers )
	{
		for ( int i = 0; i < m_workers; ++i )
			m_threads.emplace_back(
			    [this]
			    {
				    WorkerMain();
			    } );
	}

	~PoolBackend() override
	{
		{
			std::lock_guard<std::mutex> lk( m_mtx );
			m_quit = true;
		}
		m_cv.notify_all();
		for ( std::thread &t : m_threads )
			t.join();
	}

	int WorkerCount() const override { return m_workers; }

	void ParallelFor( int n, const std::function<void( int )> &body ) override
	{
		std::unique_lock<std::mutex> lk( m_mtx );
		m_body = &body;
		m_count = n;
		m_next = 0;
		m_left = n;
		++m_generation;
		m_cv.notify_all();
		Drain( lk );
		m_done.wait( lk,
		    [&]
		    {
			    return m_left == 0;
		    } );
		m_body = nullptr;
	}

private:
	void Drain( std::unique_lock<std::mutex> &lk )
	{
		while ( m_body && m_next < m_count )
		{
			const int i = m_next++;
			const std::function<void( int )> *body = m_body;
			lk.unlock();
			( *body )( i );
			lk.lock();
			if ( --m_left == 0 )
				m_done.notify_all();
		}
	}

	void WorkerMain()
	{
		uint64_t seen = 0;
		std::unique_lock<std::mutex> lk( m_mtx );
		for ( ;; )
		{
			m_cv.wait( lk,
			    [&]
			    {
				    return m_quit || m_generation != seen;
			    } );
			if ( m_quit )
				return;
			seen = m_generation;
			Drain( lk );
		}
	}

	const int m_workers;
	std::vector<std::thread> m_threads;
	std::mutex m_mtx;
	std::condition_variable m_cv, m_done;
	bool m_quit = false;
	uint64_t m_generation = 0;
	const std::function<void( int )> *m_body = nullptr;
	int m_count = 0, m_next = 0, m_left = 0;
};

// A random graph plus per-run instrumentation that proves publication: each
// job reads every producer's value (which must be written already for any
// terminal state other than Canceled) and writes its own.
struct Instance
{
	SealedGraph graph;
	std::vector<uint32_t> value;        // written by the job, read by consumers
	std::vector<std::atomic<int>> runs; // exactly-once execution
	std::vector<char> fails;
	std::vector<std::vector<uint32_t>> producers;
	std::atomic<int> violations{ 0 };
	uint32_t n = 0;

	explicit Instance( uint32_t count )
	    : value( count ), runs( count ), fails( count ), producers( count ), n( count )
	{
	}

	void Reset()
	{
		for ( uint32_t i = 0; i < n; ++i )
		{
			value[i] = 0;
			runs[i].store( 0 );
		}
		violations.store( 0 );
	}
};

std::unique_ptr<Instance> MakeInstance( uint64_t seed, bool affine )
{
	Rng rng{ seed };
	const uint32_t n = 1 + rng.Below( 60 );
	auto inst = std::make_unique<Instance>( n );
	Instance *raw = inst.get();
	JobGraphBuilder b;
	for ( uint32_t i = 0; i < n; ++i )
	{
		JobDesc d;
		d.name = "stress";
		const uint32_t lane = rng.Below( 20 );
		if ( affine )
		{
			d.executor = lane == 0   ? Executor::MainThread()
			             : lane == 1 ? Executor::BlockingIO()
			             : lane < 4  ? Executor::Sequence( (uint16_t)rng.Below( 2 ) )
			                         : Executor::Compute();
		}
		inst->fails[i] = rng.Chance( 5 );
		if ( !rng.Chance( 5 ) ) // some empty join nodes
		{
			d.function = [raw, i]( JobRunContext &ctx )
			{
				raw->runs[i].fetch_add( 1, std::memory_order_relaxed );
				uint32_t acc = i + 1;
				for ( uint32_t p : raw->producers[i] )
					acc = acc * 31u + raw->value[p]; // plain read: must be published
				raw->value[i] = acc | 1u;
				if ( raw->fails[i] )
					ctx.Fail();
			};
		}
		b.AddJob( d );
		const uint32_t deps = i ? rng.Below( 4 ) : 0;
		for ( uint32_t k = 0; k < deps; ++k )
		{
			const uint32_t p = rng.Below( i );
			b.AddDependency( JobHandle{ p }, JobHandle{ i },
			    rng.Chance( 25 ) ? DependencyKind::Terminal : DependencyKind::Success );
			inst->producers[i].push_back( p );
		}
	}
	auto sealed = b.Seal();
	CHECK( sealed.HasValue() );
	if ( sealed.HasValue() )
		inst->graph = std::move( sealed.Value() );
	return inst;
}

bool Matches( const RunResult &a, const RunResult &b )
{
	return a.states == b.states && a.succeeded == b.succeeded && a.failed == b.failed &&
	       a.canceled == b.canceled && a.executed == b.executed && a.stalled == b.stalled &&
	       a.unresolved == b.unresolved;
}

void CheckRun( Instance &inst, const RunResult &expected, IGraphExecutor &ex )
{
	inst.Reset();
	const RunResult r = ex.Execute( inst.graph, RunOptions{} );
	CHECK( Matches( r, expected ) );
	for ( uint32_t i = 0; i < inst.n; ++i )
	{
		const bool ran = r.states[i] == JobState::Succeeded || r.states[i] == JobState::Failed;
		CHECK( inst.runs[i].load() == ( ran && inst.graph.GetJob( i ).function ? 1 : 0 ) );
	}
}

// Deterministic values are the publication oracle: a consumer that read an
// unpublished producer value computes a different digest.
std::vector<uint32_t> ReferenceValues( Instance &inst, RunResult &out )
{
	inst.Reset();
	out = DeterministicExecutor().Execute( inst.graph, RunOptions{} );
	return inst.value;
}

void TestRandomReuse()
{
	ParallelExecutor p0( 0 ), p1( 1 ), p4( 4, 1 ), p3( 3 );
	PoolBackend backend( 3 );
	PooledExecutor pooled( &backend );
	PooledExecutor pooledInline( nullptr );
	IGraphExecutor *executors[] = { &p0, &p1, &p4, &p3, &pooled, &pooledInline };
	for ( uint64_t seed = 1; seed <= 600; ++seed )
	{
		auto inst = MakeInstance( seed, /*affine=*/seed % 2 == 0 );
		RunResult expected;
		const std::vector<uint32_t> values = ReferenceValues( *inst, expected );
		for ( IGraphExecutor *ex : executors )
		{
			CheckRun( *inst, expected, *ex );
			CHECK( inst->value == values );
		}
	}
}

// Several threads share one executor: one run owns its workers, the others
// must still complete correctly on their own.
void TestConcurrentExecute()
{
	ParallelExecutor shared( 3, 1 );
	std::vector<std::unique_ptr<Instance>> insts;
	std::vector<RunResult> expected( 4 );
	std::vector<std::vector<uint32_t>> values( 4 );
	for ( int t = 0; t < 4; ++t )
	{
		insts.push_back( MakeInstance( 1000 + t, true ) );
		values[t] = ReferenceValues( *insts[t], expected[t] );
	}
	std::vector<std::thread> threads;
	for ( int t = 0; t < 4; ++t )
	{
		threads.emplace_back(
		    [&, t]
		    {
			    for ( int round = 0; round < 150; ++round )
			    {
				    CheckRun( *insts[t], expected[t], shared );
				    CHECK( insts[t]->value == values[t] );
			    }
		    } );
	}
	for ( std::thread &t : threads )
		t.join();
}

// A job re-enters the executor that is running it.
void TestNestedExecute()
{
	ParallelExecutor ex( 2 );
	auto inner = MakeInstance( 77, true );
	RunResult innerExpected;
	const std::vector<uint32_t> innerValues = ReferenceValues( *inner, innerExpected );
	std::atomic<int> nestedOk{ 0 };
	JobGraphBuilder b;
	for ( int i = 0; i < 3; ++i )
	{
		JobDesc d;
		d.name = "outer";
		d.function = [&, i]( JobRunContext & )
		{
			if ( i != 1 )
				return;
			inner->Reset();
			RunResult r = ex.Execute( inner->graph, RunOptions{} );
			if ( Matches( r, innerExpected ) && inner->value == innerValues )
				nestedOk.fetch_add( 1 );
		};
		b.AddJob( d );
	}
	auto g = b.Seal();
	CHECK( g.HasValue() );
	for ( int round = 0; round < 50; ++round )
	{
		RunResult r = ex.Execute( g.Value(), RunOptions{} );
		CHECK( r.AllSucceeded() && r.executed == 3 );
	}
	CHECK( nestedOk.load() == 50 );
}

// Copies are independent executors with the same configuration; an executor
// that never ran must destroy cleanly, as must one destroyed right after use.
void TestCopiesAndLifetime()
{
	auto inst = MakeInstance( 5, true );
	RunResult expected;
	ReferenceValues( *inst, expected );
	{
		ParallelExecutor unused( 4, 2 );
	}
	ParallelExecutor original( 2, 1 );
	CheckRun( *inst, expected, original );
	ParallelExecutor copy( original );
	CheckRun( *inst, expected, copy );
	ParallelExecutor assigned( 0 );
	assigned = copy;
	CheckRun( *inst, expected, assigned );
	CheckRun( *inst, expected, original );
	for ( int i = 0; i < 20; ++i )
		CheckRun( *inst, expected, *std::make_unique<ParallelExecutor>( 3, 1 ) );
}

// A graph with no pump and main-thread work stalls identically on every run of
// a reused pool, and cancellation observed mid-run cancels the remainder.
void TestStallAndCancelReuse()
{
	ParallelExecutor ex( 2, 0 );
	JobGraphBuilder b;
	JobDesc main;
	main.name = "main";
	main.executor = Executor::MainThread();
	main.function = []( JobRunContext & ) {};
	JobDesc after;
	after.name = "after";
	after.function = []( JobRunContext & ) {};
	JobDesc free;
	free.name = "free";
	free.function = []( JobRunContext & ) {};
	const JobHandle m = b.AddJob( main );
	b.AddDependency( m, b.AddJob( after ) );
	b.AddJob( free );
	auto g = b.Seal();
	CHECK( g.HasValue() );
	RunOptions noPump;
	noPump.pumpMainThread = false;
	for ( int i = 0; i < 50; ++i )
	{
		RunResult r = ex.Execute( g.Value(), noPump );
		CHECK( r.stalled && r.unresolved == 2 && r.succeeded == 1 );
		r = ex.Execute( g.Value(), RunOptions{} );
		CHECK( r.AllSucceeded() && r.succeeded == 3 );
	}

	// Chain whose first job cancels the scope: everything later is canceled.
	std::atomic<bool> cancel{ false };
	JobGraphBuilder c;
	JobHandle prev;
	for ( int i = 0; i < 16; ++i )
	{
		JobDesc d;
		d.name = "chain";
		if ( i == 0 )
			d.function = [&]( JobRunContext & )
			{
				cancel.store( true, std::memory_order_release );
			};
		else
			d.function = []( JobRunContext & ) {};
		const JobHandle h = c.AddJob( d );
		if ( i )
			c.AddDependency( prev, h );
		prev = h;
	}
	auto cg = c.Seal();
	CHECK( cg.HasValue() );
	RunOptions opts;
	opts.cancel = &cancel;
	for ( int i = 0; i < 50; ++i )
	{
		cancel.store( false );
		RunResult r = ex.Execute( cg.Value(), opts );
		CHECK( r.succeeded == 1 && r.canceled == 15 && r.executed == 1 );
	}
}

// Children spawn further children (with dependencies on already-admitted ones)
// while workers run. Afterwards the recorded dependencies are replayed in id
// order: a child fails when its id says so, is canceled when any dependency
// did not succeed, and otherwise succeeds. Every child that ran ran once.
void TestDynamicScopeStress()
{
	auto failsById = []( uint32_t id )
	{
		return ( id * 2654435761u ) % 11u == 0;
	};
	for ( int workers : { 0, 1, 2, 4 } )
	{
		for ( uint64_t seed = 1; seed <= 40; ++seed )
		{
			DynamicScope scope( workers );
			std::mutex mtx;
			std::vector<std::vector<uint32_t>> deps( 4096 );
			std::vector<std::atomic<int>> runs( 4096 );
			std::atomic<int> budget{ 300 }; // signed: exhausted claims must not wrap

			std::function<void( Rng & )> spawnSome;
			std::vector<DynamicScope::ChildHandle> recent; // guarded by mtx

			// Spawn with dependencies on recently admitted children. The lock
			// records dependencies before the new child can finish.
			spawnSome = [&]( Rng &rng )
			{
				if ( budget.fetch_sub( 1 ) <= 0 )
					return;
				std::lock_guard<std::mutex> lk( mtx );
				std::vector<DynamicScope::ChildHandle> d;
				if ( !recent.empty() && rng.Chance( 60 ) )
					d.push_back( recent[rng.Below( (uint32_t)recent.size() )] );
				if ( !recent.empty() && rng.Chance( 30 ) )
					d.push_back( recent[rng.Below( (uint32_t)recent.size() )] );
				const uint64_t childSeed = rng.Next();
				const DynamicScope::ChildHandle h = scope.Spawn(
				    "stress.child",
				    [&, childSeed]( JobRunContext &ctx )
				    {
					    runs[ctx.JobId()].fetch_add( 1 );
					    Rng inner{ childSeed };
					    const uint32_t more = inner.Below( 3 );
					    for ( uint32_t k = 0; k < more; ++k )
						    spawnSome( inner );
					    if ( failsById( ctx.JobId() ) )
						    ctx.Fail();
				    },
				    d );
				if ( !h.IsValid() )
					return;
				for ( auto dep : d )
					deps[h.id].push_back( dep.id );
				recent.push_back( h );
				if ( recent.size() > 8 )
					recent.erase( recent.begin() );
			};

			Rng rng{ seed * 7919 + (uint64_t)workers };
			for ( int i = 0; i < 12; ++i )
				spawnSome( rng );
			scope.Wait();

			const uint32_t n = scope.Admitted();
			CHECK( n > 0 && n <= 300 );
			uint32_t succeeded = 0, failed = 0, canceled = 0;
			for ( uint32_t id = 0; id < n; ++id )
			{
				bool cancel = false;
				for ( uint32_t dep : deps[id] )
				{
					CHECK( dep < id );
					cancel = cancel || scope.StateOf( DynamicScope::ChildHandle{ dep } ) !=
					                       JobState::Succeeded;
				}
				const JobState expected = cancel            ? JobState::Canceled
				                          : failsById( id ) ? JobState::Failed
				                                            : JobState::Succeeded;
				CHECK( scope.StateOf( DynamicScope::ChildHandle{ id } ) == expected );
				CHECK( runs[id].load() == ( cancel ? 0 : 1 ) );
				succeeded += expected == JobState::Succeeded;
				failed += expected == JobState::Failed;
				canceled += expected == JobState::Canceled;
			}
			CHECK( scope.Succeeded() == succeeded && scope.Failed() == failed &&
			       scope.Canceled() == canceled );
		}
	}
}

// Batch graphs are reused per thread: a nested batch re-enters the graph its
// outer batch is executing, names beyond the cache cap still work, and a name
// buffer reused with new contents never aliases an older graph.
struct BatchProbe
{
	std::vector<uint32_t> out;
	std::atomic<int> begins{ 0 }, ends{ 0 };
	BatchDesc inner;
	bool nest = false;
	bool innerOk = true;
};

void ProbeProcess( void *context, unsigned index )
{
	BatchProbe *p = static_cast<BatchProbe *>( context );
	p->out[index] += index + 1;
	if ( p->nest && index == 0 )
	{
		BatchProbe *q = static_cast<BatchProbe *>( p->inner.context );
		// Same name as the outer batch: nested batches run serially with one
		// participant, the same shape as a serial outer batch.
		p->innerOk = ExecuteParallelBatch( p->inner, nullptr, BatchMode::Parallel ) && p->innerOk;
		for ( unsigned i = 0; i < q->out.size(); ++i )
			p->innerOk = p->innerOk && q->out[i] == i + 1;
	}
}

void ProbeBegin( void *context )
{
	static_cast<BatchProbe *>( context )->begins.fetch_add( 1 );
}
void ProbeEnd( void *context )
{
	static_cast<BatchProbe *>( context )->ends.fetch_add( 1 );
}

BatchDesc ProbeDesc( BatchProbe &p, const char *name, unsigned count, unsigned limit )
{
	p.out.assign( count, 0 );
	BatchDesc d;
	d.name = name;
	d.context = &p;
	d.count = count;
	d.process = &ProbeProcess;
	d.begin = &ProbeBegin;
	d.end = &ProbeEnd;
	d.maxParticipants = limit;
	return d;
}

bool ProbeOk( const BatchProbe &p )
{
	for ( unsigned i = 0; i < p.out.size(); ++i )
		if ( p.out[i] != i + 1 )
			return false;
	return p.begins.load() == p.ends.load() && p.begins.load() >= 1;
}

void TestBatchGraphCache()
{
	PoolBackend backend( 3 );
	for ( BatchMode mode : { BatchMode::Serial, BatchMode::Parallel } )
	{
		for ( int round = 0; round < 50; ++round )
		{
			BatchProbe outer, inner;
			BatchDesc od = ProbeDesc( outer, "cache.same", 1 + round % 7, 1 + round % 4 );
			outer.inner = ProbeDesc( inner, "cache.same", 5, 4 );
			outer.nest = true;
			CHECK( ExecuteParallelBatch( od, &backend, mode ) );
			CHECK( ProbeOk( outer ) && ProbeOk( inner ) && outer.innerOk );
		}
	}

	// More distinct shapes than the per-thread cache holds, from a reused and
	// rewritten name buffer, interleaved with a revisited early name.
	char name[32];
	for ( int i = 0; i < 100; ++i )
	{
		std::snprintf( name, sizeof( name ), "cache.name.%d", i );
		BatchProbe p;
		BatchDesc d = ProbeDesc( p, name, 64, 1 + i % 4 );
		CHECK(
		    ExecuteParallelBatch( d, &backend, i % 2 ? BatchMode::Parallel : BatchMode::Serial ) );
		CHECK( ProbeOk( p ) );
		std::memset( name, 'x', sizeof( name ) - 1 ); // the batch kept no pointer to it
		name[sizeof( name ) - 1] = '\0';
		BatchProbe again;
		BatchDesc e = ProbeDesc( again, "cache.name.0", 32, 3 );
		CHECK( ExecuteParallelBatch( e, &backend, BatchMode::Parallel ) );
		CHECK( ProbeOk( again ) );
	}

	// Several threads build their own caches concurrently.
	std::vector<std::thread> threads;
	for ( int t = 0; t < 4; ++t )
	{
		threads.emplace_back(
		    [&backend, t]
		    {
			    for ( int i = 0; i < 200; ++i )
			    {
				    BatchProbe p;
				    BatchDesc d =
				        ProbeDesc( p, i % 3 ? "cache.shared" : "cache.other", 16 + t, 1 + i % 5 );
				    CHECK( ExecuteParallelBatch( d, &backend, BatchMode::Serial ) );
				    CHECK( ProbeOk( p ) );
			    }
		    } );
	}
	for ( std::thread &t : threads )
		t.join();
}

} // namespace

int main()
{
	TestRandomReuse();
	TestConcurrentExecute();
	TestNestedExecute();
	TestCopiesAndLifetime();
	TestStallAndCancelReuse();
	TestDynamicScopeStress();
	TestBatchGraphCache();
	std::printf( "%d checks, %d failures\n", g_checks.load(), g_failures.load() );
	return testing::ReportConformance( g_checks.load(), g_failures.load() );
}
