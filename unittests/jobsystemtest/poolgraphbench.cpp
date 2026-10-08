//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TaskExecutor against the retired wave executor on the real engine
//          thread pool (RFC 0003 phase I, J4 evidence). Both run the same
//          graphs on one CThreadPool through the vstdlib bridge, alternating
//          sample by sample, so the comparison is the scheduler alone.
//
//          Shapes are the product's: a batch (one participant per worker plus
//          the caller, items claimed from a cursor, then a join), fan-outs of
//          independent jobs, chains, and layered graphs, each with empty jobs
//          and with work. Every sample is validated (each job ran once, the
//          run succeeded) before it counts.
//
//          Run on the benchmark runner, not a development host:
//            ./jobsystempoolgraphbench [--workers N] [--samples N] [--filter s]
//                [--executor task|wave]
//
//=============================================================================//

#include "jobsystem/task_executor.h"
#include "testing/conformance_result.h"
#include "vstdlib/jobgraph_pool_bridge.h"
#include "wave_executor_reference.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace jobsystem;

namespace
{

int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "  FAIL %s\n", what );
	}
}

volatile uint32_t g_sink;

void Spin( uint32_t steps )
{
	uint32_t x = 1;
	for ( uint32_t i = 0; i < steps; ++i )
		x = x * 1664525u + 1013904223u;
	g_sink = x;
}

struct Fixture
{
	std::vector<std::atomic<int>> runs;
	std::atomic<uint32_t> cursor{ 0 };
	// --starts: each participant's start and item count, relative to t0.
	std::chrono::steady_clock::time_point t0;
	double started[8] = {};
	uint32_t claimed[8] = {};
	uint32_t items = 0;
	uint32_t steps = 0;
	explicit Fixture( uint32_t n ) : runs( n ) {}
	void Reset()
	{
		for ( auto &r : runs )
			r.store( 0, std::memory_order_relaxed );
		cursor.store( 0, std::memory_order_relaxed );
	}
};

enum class Shape
{
	Batch,
	Wide,
	Chain,
	Layered
};

const char *Name( Shape s )
{
	switch ( s )
	{
	case Shape::Batch:
		return "batch";
	case Shape::Wide:
		return "wide";
	case Shape::Chain:
		return "chain";
	default:
		return "layered";
	}
}

// Batch: `participants` jobs claim `items` from a cursor, then a join.
// Others: `n` jobs of `steps` work each.
SealedGraph Build( Shape shape, uint32_t n, uint32_t participants, Fixture &fx )
{
	JobGraphBuilder b;
	if ( shape == Shape::Batch )
	{
		JobDesc join;
		join.name = "join";
		const JobHandle j = b.AddJob( join );
		for ( uint32_t p = 0; p < participants; ++p )
		{
			JobDesc d;
			d.name = "participant";
			d.function = [&fx, p]( JobRunContext & )
			{
				if ( p < 8 )
					fx.started[p] = std::chrono::duration<double, std::nano>(
					    std::chrono::steady_clock::now() - fx.t0 )
					                    .count();
				uint32_t mine = 0;
				for ( uint32_t i = fx.cursor.fetch_add( 1 ); i < fx.items; i = fx.cursor.fetch_add( 1 ) )
				{
					Spin( fx.steps );
					fx.runs[i].fetch_add( 1, std::memory_order_relaxed );
					++mine;
				}
				if ( p < 8 )
					fx.claimed[p] = mine;
			};
			b.AddDependency( b.AddJob( d ), j );
		}
		return std::move( b.Seal().Value() );
	}
	std::vector<JobHandle> handles;
	const uint32_t width = shape == Shape::Layered ? 16 : n;
	for ( uint32_t i = 0; i < n; ++i )
	{
		JobDesc d;
		d.name = "job";
		d.function = [&fx, i]( JobRunContext & )
		{
			Spin( fx.steps );
			fx.runs[i].fetch_add( 1, std::memory_order_relaxed );
		};
		const JobHandle h = b.AddJob( d );
		if ( shape == Shape::Chain && i > 0 )
			b.AddDependency( handles[i - 1], h );
		if ( shape == Shape::Layered && i >= width )
		{
			b.AddDependency( handles[i - width], h );
			b.AddDependency( handles[i - width + ( i + 7 ) % width - i % width], h );
		}
		handles.push_back( h );
	}
	return std::move( b.Seal().Value() );
}

double Median( std::vector<double> v )
{
	std::sort( v.begin(), v.end() );
	return v.empty() ? 0.0 : v[v.size() / 2];
}

} // namespace

int main( int argc, char **argv )
{
	int workers = 3, samples = 41;
	const char *filter = nullptr;
	const char *only = nullptr;
	bool starts = false;
	for ( int i = 1; i < argc; ++i )
	{
		if ( !std::strcmp( argv[i], "--workers" ) && i + 1 < argc )
			workers = std::atoi( argv[++i] );
		else if ( !std::strcmp( argv[i], "--samples" ) && i + 1 < argc )
			samples = std::max( 3, std::atoi( argv[++i] ) );
		else if ( !std::strcmp( argv[i], "--filter" ) && i + 1 < argc )
			filter = argv[++i];
		else if ( !std::strcmp( argv[i], "--starts" ) )
			starts = true; // print batch participants' start times and shares
		else if ( !std::strcmp( argv[i], "--executor" ) && i + 1 < argc )
			only = argv[++i]; // profile one executor: "task" or "wave"
	}
	IWorkerBackend *backend = CreateThreadPoolWorkerBackend( workers );
	Check( backend && backend->WorkerCount() == workers, "the engine pool started" );
	TaskExecutor task( backend );
	jobsystemtest::WaveExecutorReference wave( backend );
	IGraphExecutor *executors[2] = { &task, &wave };
	const char *names[2] = { "task", "wave" };

	struct Case
	{
		Shape shape;
		uint32_t n;
	};
	const Case cases[] = { { Shape::Batch, 1 }, { Shape::Batch, 8 }, { Shape::Batch, 64 },
	    { Shape::Batch, 1024 }, { Shape::Wide, 4 }, { Shape::Wide, 16 }, { Shape::Wide, 256 },
	    { Shape::Chain, 16 }, { Shape::Chain, 256 }, { Shape::Layered, 256 } };
	for ( const Case &c : cases )
	{
		for ( uint32_t steps : { 0u, 256u, 4096u } )
		{
			char label[96];
			std::snprintf( label, sizeof( label ), "%s/%u/steps=%u/w%d", Name( c.shape ), c.n, steps,
			    workers );
			if ( filter && !std::strstr( label, filter ) )
				continue;
			Fixture fx( c.n );
			fx.items = c.n;
			fx.steps = steps;
			const uint32_t participants = std::min<uint32_t>( c.n, (uint32_t)workers + 1 );
			SealedGraph g = Build( c.shape, c.n, participants, fx );
			// Repetitions per sample keep a sample well above clock resolution.
			const int reps = std::max( 1, (int)( 20000 / ( 1 + c.n * ( 1 + steps / 64 ) ) ) );
			std::vector<double> times[2];
			bool ok[2] = { true, true };
			for ( int s = 0; s < samples + 2; ++s )
			{
				for ( int k = 0; k < 2; ++k )
				{
					const int e = ( s % 2 ) ? 1 - k : k; // ABBA
					if ( only && std::strcmp( only, names[e] ) != 0 )
						continue;
					const auto t0 = std::chrono::steady_clock::now();
					for ( int r = 0; r < reps; ++r )
					{
						fx.Reset();
						fx.t0 = std::chrono::steady_clock::now();
						RunResult res = executors[e]->Execute( g, RunOptions{} );
						bool once = res.AllSucceeded();
						for ( auto &x : fx.runs )
							once = once && x.load( std::memory_order_relaxed ) == 1;
						ok[e] = ok[e] && once;
					}
					const double ns = std::chrono::duration<double, std::nano>(
					                      std::chrono::steady_clock::now() - t0 )
					                      .count() /
					                  reps;
					if ( s >= 2 ) // two warm-up samples
						times[e].push_back( ns );
				}
			}
			for ( int e = 0; e < 2; ++e )
				Check( ok[e], names[e] );
			// The task executor's own observations over a few extra runs.
			TaskRunStats stats, sum;
			const int statRuns = 64;
			for ( int r = 0; r < statRuns; ++r )
			{
				fx.Reset();
				task.Execute( g, RunOptions{}, &stats );
				sum.tasksPosted += stats.tasksPosted;
				sum.tasksRan += stats.tasksRan;
				sum.tasksWithdrawn += stats.tasksWithdrawn;
				sum.jobsOnRunners += stats.jobsOnRunners;
				sum.jobsOnCaller += stats.jobsOnCaller;
			}
			if ( starts && c.shape == Shape::Batch )
			{
				for ( int e = 0; e < 2; ++e )
				{
					std::vector<double> at[8];
					std::vector<double> share[8];
					for ( int r = 0; r < 201; ++r )
					{
						fx.Reset();
						for ( double &x : fx.started )
							x = -1;
						for ( uint32_t &x : fx.claimed )
							x = 0;
						fx.t0 = std::chrono::steady_clock::now();
						executors[e]->Execute( g, RunOptions{} );
						for ( uint32_t p = 0; p < participants && p < 8; ++p )
						{
							at[p].push_back( fx.started[p] );
							share[p].push_back( fx.claimed[p] );
						}
					}
					std::printf( "STARTS %-30s %s:", label, names[e] );
					for ( uint32_t p = 0; p < participants && p < 8; ++p )
						std::printf( "  p%u at %.0f ns, %.0f items", p, Median( at[p] ), Median( share[p] ) );
					std::printf( "\n" );
				}
			}
			const double mt = Median( times[0] ), mw = Median( times[1] );
			std::printf( "POOLBENCH %-30s task=%10.1f ns  wave=%10.1f ns  task/wave=%5.2f  "
			             "per run: posted %.2f ran %.2f withdrawn %.2f jobs on runners %.2f caller %.2f\n",
			    label, mt, mw, mw > 0 ? mt / mw : 0.0, sum.tasksPosted / (double)statRuns,
			    sum.tasksRan / (double)statRuns, sum.tasksWithdrawn / (double)statRuns,
			    sum.jobsOnRunners / (double)statRuns, sum.jobsOnCaller / (double)statRuns );
			std::fflush( stdout );
		}
	}
	DestroyThreadPoolWorkerBackend( backend );
	std::printf( "%d checks, %d failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
