//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Engine integration test (RFC 0003, Phase C / R20). Executes real
//          job graphs on the actual engine vstdlib CThreadPool via the
//          jobgraph_pool_bridge, and checks that engine-shaped subsystem work
//          (the particle gather/compute/commit graph) produces results identical
//          to the serial reference and the deterministic executor when run on
//          real worker threads.
//
//          This is the "main engine functionality riding on the job system"
//          slice that is buildable and runnable in the --tests configuration:
//          the production thread pool the engine uses for ParallelProcess now
//          also executes dependency-aware job graphs.
//
//=============================================================================//

#include "jobsystem/frame_graph.h"
#include "jobsystem/graph_executor.h"
#include "jobsystem/pooled_executor.h"
#include "jobsystem/pilot_particles.h"
#include "vstdlib/jobgraph_pool_bridge.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

using namespace jobsystem;

static int g_checks = 0, g_failures = 0;
static const char *g_curTest = "";
#define CHECK( cond ) do { ++g_checks; if ( !(cond) ) { ++g_failures; \
	std::printf( "  FAIL [%s] %s:%d: %s\n", g_curTest, __FILE__, __LINE__, #cond ); } } while (0)
#define RUN( fn ) do { g_curTest = #fn; std::printf( "- %s\n", #fn ); fn(); } while (0)

static std::vector<double> MakeInputs( int n )
{
	std::vector<double> v( n );
	for ( int i = 0; i < n; ++i ) v[i] = 1.0 + i * 0.25 - ( i % 3 );
	return v;
}
static bool StatesEqual( const ParticleFrameState &a, const ParticleFrameState &b )
{
	return a.boneValue == b.boneValue && a.controlPoints == b.controlPoints
	    && a.perSystemOut == b.perSystemOut && a.changeDigest == b.changeDigest;
}
static FrameContext MakeFrame() { FrameContext f; f.tickTime = 0.015; f.frameTime = 0.015; f.epoch = 7; return f; }

static Expected<SealedGraph, GraphError> BuildParticleFrame( ParticlePilotContributor *pilot, FrameCoordinator &fc )
{
	fc.AddLegacyPhase( "apply_input" );
	JobHandle p1 = fc.AddLegacyPhase( "stable_inputs" );
	JobHandle p2 = fc.AddLegacyPhase( "publish" );
	fc.AddLegacyPhase( "cleanup" );
	fc.SetContributorRegion( p1, p2 );
	fc.AddContributor( pilot );
	return fc.Build( true );
}

static void BuildStressGraph( JobGraphBuilder &b, std::vector<int> &order, int layers, int width )
{
	std::vector<std::vector<JobHandle>> layer;
	int counter = 0;
	for ( int L = 0; L < layers; ++L )
	{
		std::vector<JobHandle> row;
		for ( int w = 0; w < width; ++w )
		{
			JobDesc d; d.name = "s"; int idx = counter++;
			d.function = [&order, idx]( JobRunContext & ) { order[idx]++; };
			JobHandle h = b.AddJob( d );
			row.push_back( h );
			if ( L > 0 ) { b.AddDependency( layer[L-1][w % width], h ); b.AddDependency( layer[L-1][(w+1) % width], h ); }
		}
		layer.push_back( row );
	}
	order.assign( counter, 0 );
}

// The real engine pool, wrapped as a backend for the duration of a test.
struct RealPool
{
	IWorkerBackend *backend;
	explicit RealPool( int n ) : backend( CreateThreadPoolWorkerBackend( n ) ) {}
	~RealPool() { DestroyThreadPoolWorkerBackend( backend ); }
};

static void Test_RealPoolCreatesWorkers()
{
	RealPool pool( 3 );
	CHECK( pool.backend != nullptr );
	CHECK( pool.backend->WorkerCount() == 3 ); // real threads actually started
}

static void Test_RealPoolParticleEquivalence()
{
	const int N = 32;
	std::vector<double> inputs = MakeInputs( N );
	ParticleFrameState ref; ref.inputs = inputs; ParticleReferenceUpdate( ref );

	RealPool pool( 3 );
	for ( int rep = 0; rep < 12; ++rep )
	{
		ParticleFrameState s; s.inputs = inputs;
		ParticlePilotContributor pilot( &s );
		FrameCoordinator fc( MakeFrame() );
		auto sealed = BuildParticleFrame( &pilot, fc );
		CHECK( sealed.HasValue() );
		if ( !sealed.HasValue() ) continue;
		RunOptions opts; opts.frame = fc.Frame();
		RunResult r = PooledExecutor( pool.backend ).Execute( sealed.Value(), opts );
		CHECK( r.AllSucceeded() );
		CHECK( StatesEqual( s, ref ) ); // engine-pool result == serial reference
	}
}

static void Test_RealPoolMatchesDeterministic()
{
	const int layers = 7, width = 10;
	std::vector<int> serialRuns; RunResult serial;
	{
		JobGraphBuilder b; BuildStressGraph( b, serialRuns, layers, width );
		SealedGraph g = b.Seal().Value();
		serial = DeterministicExecutor().Execute( g, RunOptions{} );
	}
	RealPool pool( 4 );
	for ( int rep = 0; rep < 10; ++rep )
	{
		std::vector<int> runs; JobGraphBuilder b; BuildStressGraph( b, runs, layers, width );
		SealedGraph g = b.Seal().Value();
		RunResult r = PooledExecutor( pool.backend ).Execute( g, RunOptions{} );
		CHECK( r.states == serial.states );
		CHECK( r.succeeded == serial.succeeded );
		bool once = true; for ( int x : runs ) if ( x != 1 ) once = false;
		CHECK( once ); // real pool ran each graph node exactly once
	}
}

static void Test_RealPoolFailurePropagation()
{
	RealPool pool( 3 );
	JobGraphBuilder b;
	JobDesc da; da.name = "A"; da.function = []( JobRunContext &c ) { c.Fail(); };
	JobHandle a = b.AddJob( da );
	JobDesc dgen; dgen.name = "n";
	JobHandle bh = b.AddJob( dgen ), dh = b.AddJob( dgen );
	bool cleanup = false;
	JobDesc dc; dc.name = "cleanup"; dc.function = [&]( JobRunContext & ) { cleanup = true; };
	JobHandle ch = b.AddJob( dc );
	b.AddDependency( a, bh, DependencyKind::Success );
	b.AddDependency( bh, dh, DependencyKind::Success );
	b.AddDependency( a, ch, DependencyKind::Terminal );
	SealedGraph g = b.Seal().Value();
	RunResult r = PooledExecutor( pool.backend ).Execute( g, RunOptions{} );
	CHECK( r.states[a.id] == JobState::Failed );
	CHECK( r.states[bh.id] == JobState::Canceled );
	CHECK( r.states[dh.id] == JobState::Canceled );
	CHECK( r.states[ch.id] == JobState::Succeeded );
	CHECK( cleanup );
}

static void Test_RealPoolZeroWorkersInline()
{
	RealPool pool( 0 ); // no worker threads
	CHECK( pool.backend->WorkerCount() == 0 );
	std::vector<int> runs; JobGraphBuilder b; BuildStressGraph( b, runs, 4, 4 );
	SealedGraph g = b.Seal().Value();
	RunResult r = PooledExecutor( pool.backend ).Execute( g, RunOptions{} );
	CHECK( r.succeeded == g.JobCount() );
	for ( int x : runs ) CHECK( x == 1 );
}


// The engine pool bridge overlaps a wave's main-thread jobs with its compute
// jobs: each waits for the other to start, which only concurrent execution
// satisfies before the deadline.
static bool AwaitFlag( const std::atomic<bool> &flag )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 5 );
	while ( !flag.load() )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		std::this_thread::yield();
	}
	return true;
}

static void Test_RealPoolOverlapsCallerJobs()
{
	RealPool pool( 2 );
	for ( int round = 0; round < 20; ++round )
	{
		JobGraphBuilder b;
		std::atomic<bool> computeStarted( false ), mainStarted( false );
		std::atomic<bool> computeSaw( false ), mainSaw( false );
		JobDesc compute;
		compute.name = "compute";
		compute.function = [&]( JobRunContext & )
		{
			computeStarted = true;
			computeSaw = AwaitFlag( mainStarted );
		};
		JobDesc main;
		main.name = "main";
		main.executor = Executor::MainThread();
		main.function = [&]( JobRunContext & )
		{
			mainStarted = true;
			mainSaw = AwaitFlag( computeStarted );
		};
		b.AddJob( compute );
		b.AddJob( main );
		SealedGraph g = b.Seal().Value();
		RunOptions options;
		options.pumpMainThread = true;
		RunResult r = PooledExecutor( pool.backend ).Execute( g, options );
		CHECK( r.AllSucceeded() );
		CHECK( computeSaw.load() && mainSaw.load() );
	}
}

int main()
{
	std::printf( "enginebridgetest (RFC 0003 real vstdlib pool executes job graphs)\n" );
	RUN( Test_RealPoolCreatesWorkers );
	RUN( Test_RealPoolParticleEquivalence );
	RUN( Test_RealPoolMatchesDeterministic );
	RUN( Test_RealPoolFailurePropagation );
	RUN( Test_RealPoolZeroWorkersInline );
	RUN( Test_RealPoolOverlapsCallerJobs );
	std::printf( "\n%d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
