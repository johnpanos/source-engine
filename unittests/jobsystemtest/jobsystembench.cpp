//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Job-system microbenchmarks (RFC 0003 scheduler overhead, RFC 0005
//          "controlled benchmarks"). Measures graph construction/sealing, each
//          IGraphExecutor, the synchronous batch facade, dynamic scopes and the
//          external-completion token through their public contracts only.
//
//          Every sample is validated against an independent oracle (terminal
//          counts, per-item outputs, hook balance); a wrong result fails the
//          run even in release builds, so a "faster" scheduler that drops work
//          cannot report a number. Timings are informational: they are not an
//          acceptance gate and carry no frame-time claim.
//
//          Default mode is a short smoke run suitable for the conformance
//          runner (few samples, still fully validated). Use --full for stable
//          numbers from an optimized build, e.g.:
//            g++ -std=c++20 -O2 -pthread -I public -o jobsystembench
//                jobsystem/*.cpp unittests/jobsystemtest/jobsystembench.cpp
//            ./jobsystembench --full [--filter substr] [--json out.json]
//
//=============================================================================//

#include "jobsystem/dynamic_scope.h"
#include "jobsystem/external_completion.h"
#include "jobsystem/graph_executor.h"
#include "jobsystem/job_graph.h"
#include "jobsystem/parallel_batch.h"
#include "jobsystem/parallel_executor.h"
#include "jobsystem/pooled_executor.h"
#include "jobsystem/worker_backend.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace jobsystem;

namespace
{

int g_failures = 0;
long long g_checks = 0;

#define BENCH_CHECK( condition )                                                                   \
	do                                                                                             \
	{                                                                                              \
		++g_checks;                                                                                \
		if ( !( condition ) )                                                                      \
		{                                                                                          \
			++g_failures;                                                                          \
			std::printf( "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition );                   \
		}                                                                                          \
	} while ( 0 )

//-----------------------------------------------------------------------------
// Persistent worker pool backend. Workers park between calls so the benchmark
// measures scheduler dispatch, not thread creation. The caller participates.
// The completion counter is released by each participant and acquired by the
// caller: every body write happens-before ParallelFor returns.
//-----------------------------------------------------------------------------
class PoolBackend final : public IWorkerBackend
{
public:
	explicit PoolBackend( int workers ) : m_workers( workers < 0 ? 0 : workers )
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
		if ( n <= 0 )
			return;
		if ( m_workers == 0 || n == 1 )
		{
			for ( int i = 0; i < n; ++i )
				body( i );
			return;
		}
		{
			std::lock_guard<std::mutex> lk( m_mtx );
			m_body = &body;
			m_count = n;
			m_next.store( 0, std::memory_order_relaxed );
			m_pending.store( m_workers, std::memory_order_relaxed );
			++m_generation;
		}
		m_cv.notify_all();
		Drain();
		// Wait for every worker to leave this generation before the borrowed
		// body goes out of scope.
		while ( m_pending.load( std::memory_order_acquire ) != 0 )
			std::this_thread::yield();
	}

private:
	void Drain()
	{
		for ( ;; )
		{
			const int i = m_next.fetch_add( 1, std::memory_order_relaxed );
			if ( i >= m_count )
				return;
			( *m_body )( i );
		}
	}

	void WorkerMain()
	{
		uint64_t seen = 0;
		for ( ;; )
		{
			{
				std::unique_lock<std::mutex> lk( m_mtx );
				m_cv.wait( lk,
				    [&]
				    {
					    return m_quit || m_generation != seen;
				    } );
				if ( m_quit )
					return;
				seen = m_generation;
			}
			Drain();
			m_pending.fetch_sub( 1, std::memory_order_acq_rel );
		}
	}

	const int m_workers;
	std::vector<std::thread> m_threads;
	std::mutex m_mtx;
	std::condition_variable m_cv;
	bool m_quit = false;
	uint64_t m_generation = 0;
	const std::function<void( int )> *m_body = nullptr;
	int m_count = 0;
	std::atomic<int> m_next{ 0 };
	std::atomic<int> m_pending{ 0 };
};

//-----------------------------------------------------------------------------
// Measurement
//-----------------------------------------------------------------------------
struct Config
{
	int warmup = 2;
	int samples = 5;
	double minSampleUs = 0.0; // repeat the op until a sample spans this long
	const char *filter = nullptr;
	const char *jsonPath = nullptr;
};

struct Result
{
	std::string name;
	double medianNs = 0, p95Ns = 0, minNs = 0;
	int samples = 0;
	long long opsPerSample = 0;
};

Config g_config;
std::vector<Result> g_results;

bool Selected( const std::string &name )
{
	return !g_config.filter || name.find( g_config.filter ) != std::string::npos;
}

// op() performs one operation and returns true iff its oracle passed.
template <typename Op> void Bench( const std::string &name, Op &&op )
{
	if ( !Selected( name ) )
		return;

	using Clock = std::chrono::steady_clock;
	for ( int i = 0; i < g_config.warmup; ++i )
		BENCH_CHECK( op() );

	// Calibrate the per-sample repetition count so short ops are not dominated
	// by clock resolution.
	long long reps = 1;
	if ( g_config.minSampleUs > 0 )
	{
		for ( ;; )
		{
			const auto t0 = Clock::now();
			for ( long long r = 0; r < reps; ++r )
				BENCH_CHECK( op() );
			const double us =
			    std::chrono::duration<double, std::micro>( Clock::now() - t0 ).count();
			if ( us >= g_config.minSampleUs || reps >= ( 1ll << 20 ) )
				break;
			reps *= 2;
		}
	}

	std::vector<double> ns;
	ns.reserve( (size_t)g_config.samples );
	for ( int s = 0; s < g_config.samples; ++s )
	{
		bool ok = true;
		const auto t0 = Clock::now();
		for ( long long r = 0; r < reps; ++r )
			ok = op() && ok;
		const auto t1 = Clock::now();
		BENCH_CHECK( ok );
		ns.push_back( std::chrono::duration<double, std::nano>( t1 - t0 ).count() / (double)reps );
	}
	std::sort( ns.begin(), ns.end() );

	Result r;
	r.name = name;
	r.samples = (int)ns.size();
	r.opsPerSample = reps;
	r.minNs = ns.front();
	r.medianNs = ns[ns.size() / 2];
	r.p95Ns = ns[std::min( ns.size() - 1, ( ns.size() * 95 ) / 100 )];
	std::printf( "BENCH %-48s median=%12.1f ns  p95=%12.1f ns  min=%12.1f ns  (n=%d x%lld)\n",
	    r.name.c_str(), r.medianNs, r.p95Ns, r.minNs, r.samples, r.opsPerSample );
	std::fflush( stdout );
	g_results.push_back( r );
}

// Synthetic work that the optimizer cannot remove.
inline uint32_t Spin( uint32_t seed, uint32_t steps )
{
	uint32_t x = seed * 2654435761u + 1u;
	for ( uint32_t i = 0; i < steps; ++i )
		x = x * 1664525u + 1013904223u;
	return x;
}

//-----------------------------------------------------------------------------
// Graph shapes. Each job writes a checksum slot so outputs can be verified.
//-----------------------------------------------------------------------------
enum class Shape
{
	Wide,    // N independent jobs + one join
	Chain,   // N jobs in a single dependency chain
	Layered, // L layers of W jobs, each depending on two jobs of the previous layer
};

const char *ShapeName( Shape s )
{
	switch ( s )
	{
	case Shape::Wide:
		return "wide";
	case Shape::Chain:
		return "chain";
	case Shape::Layered:
	default:
		return "layered";
	}
}

struct GraphFixture
{
	std::vector<uint32_t> out;
	uint32_t steps = 0;
	uint32_t jobs = 0;
};

SealedGraph BuildGraph( Shape shape, uint32_t n, GraphFixture &fx, bool withResources = false )
{
	fx.out.assign( n + 1, 0 );
	JobGraphBuilder b;
	std::vector<JobHandle> h;
	h.reserve( n + 1 );
	for ( uint32_t i = 0; i < n; ++i )
	{
		JobDesc d;
		d.name = "bench.job";
		uint32_t *slot = &fx.out[i];
		const uint32_t steps = fx.steps;
		d.function = [slot, i, steps]( JobRunContext & )
		{
			*slot = Spin( i, steps ) | 1u;
		};
		h.push_back( b.AddJob( d ) );
	}

	switch ( shape )
	{
	case Shape::Wide:
	{
		JobDesc join;
		join.name = "bench.join";
		const JobHandle j = b.AddJob( join );
		for ( uint32_t i = 0; i < n; ++i )
			b.AddDependency( h[i], j );
		if ( withResources )
		{
			for ( uint32_t i = 0; i < n; ++i )
			{
				b.Write( h[i], ResourceVersion{ 1, 0, i } );
				b.Read( h[i], ResourceVersion{ 2, 0, 0 } );
			}
			b.Write( j, ResourceVersion{ 2, 1, 0 } );
		}
		break;
	}
	case Shape::Chain:
		for ( uint32_t i = 1; i < n; ++i )
		{
			b.AddDependency( h[i - 1], h[i] );
			if ( withResources )
				b.Write( h[i], ResourceVersion{ 3, 0, 0 } );
		}
		break;
	case Shape::Layered:
	default:
	{
		const uint32_t width = 16;
		for ( uint32_t i = width; i < n; ++i )
		{
			const uint32_t layerStart = ( i / width - 1 ) * width;
			b.AddDependency( h[layerStart + ( i % width )], h[i] );
			b.AddDependency( h[layerStart + ( ( i * 7 + 3 ) % width )], h[i] );
			if ( withResources )
			{
				// Each job reads its two producers' partitions and writes its own.
				b.Write( h[i], ResourceVersion{ 4, 0, i } );
				b.Read( h[i], ResourceVersion{ 4, 0, layerStart + ( i % width ) } );
			}
		}
		break;
	}
	}

	auto sealed = b.Seal();
	BENCH_CHECK( sealed.HasValue() );
	fx.jobs = n + ( shape == Shape::Wide ? 1 : 0 );
	return sealed.HasValue() ? std::move( sealed.Value() ) : SealedGraph{};
}

bool VerifyRun( const RunResult &r, const GraphFixture &fx, uint32_t n )
{
	if ( r.succeeded != fx.jobs || r.failed || r.canceled || r.stalled )
		return false;
	for ( uint32_t i = 0; i < n; ++i )
	{
		if ( fx.out[i] != ( Spin( i, fx.steps ) | 1u ) )
			return false;
	}
	return true;
}

void ClearOutputs( GraphFixture &fx )
{
	std::fill( fx.out.begin(), fx.out.end(), 0u );
}

//-----------------------------------------------------------------------------
// Benchmarks
//-----------------------------------------------------------------------------
void BenchSeal()
{
	for ( Shape shape : { Shape::Wide, Shape::Chain, Shape::Layered } )
	{
		for ( uint32_t n : { 16u, 256u, 2048u } )
		{
			for ( bool res : { false, true } )
			{
				char name[96];
				std::snprintf( name, sizeof( name ), "seal.%s%s/%u", ShapeName( shape ),
				    res ? ".resources" : "", n );
				Bench( name,
				    [&]
				    {
					    GraphFixture fx;
					    SealedGraph g = BuildGraph( shape, n, fx, res );
					    return g.JobCount() == fx.jobs;
				    } );
			}
		}
	}

	// A graph whose unordered conflict is found only after scanning many decls:
	// exercises the validator's rejection path.
	for ( uint32_t n : { 256u, 2048u } )
	{
		char name[96];
		std::snprintf( name, sizeof( name ), "seal.conflict.reject/%u", n );
		Bench( name,
		    [&]
		    {
			    JobGraphBuilder b;
			    std::vector<JobHandle> h;
			    for ( uint32_t i = 0; i < n; ++i )
			    {
				    JobDesc d;
				    d.name = "bench.job";
				    h.push_back( b.AddJob( d ) );
				    b.Write( h.back(), ResourceVersion{ 5, 0, i } );
			    }
			    b.Write( h[0], ResourceVersion{ 6, 0, 0 } );
			    b.Write( h[n - 1], ResourceVersion{ 6, 0, 0 } );
			    auto sealed = b.Seal();
			    return !sealed.HasValue() &&
			           sealed.Error().code == GraphErrorCode::ResourceConflict &&
			           sealed.Error().jobA == 0 && sealed.Error().jobB == n - 1;
		    } );
	}
}

void BenchExecutor( const char *label, IGraphExecutor &ex, std::initializer_list<uint32_t> sizes,
    std::initializer_list<uint32_t> stepsList )
{
	for ( Shape shape : { Shape::Wide, Shape::Chain, Shape::Layered } )
	{
		for ( uint32_t n : sizes )
		{
			for ( uint32_t steps : stepsList )
			{
				GraphFixture fx;
				fx.steps = steps;
				SealedGraph g = BuildGraph( shape, n, fx );
				char name[96];
				std::snprintf( name, sizeof( name ), "exec.%s.%s/%u/steps=%u", label,
				    ShapeName( shape ), n, steps );
				Bench( name,
				    [&]
				    {
					    ClearOutputs( fx );
					    RunResult r = ex.Execute( g, RunOptions{} );
					    return VerifyRun( r, fx, n );
				    } );
			}
		}
	}
}

void BenchExecutors()
{
	{
		DeterministicExecutor ex;
		BenchExecutor( "deterministic", ex, { 16u, 256u, 2048u }, { 0u, 256u } );
	}
	for ( int workers : { 0, 1, 4 } )
	{
		ParallelExecutor ex( workers );
		char label[32];
		std::snprintf( label, sizeof( label ), "parallel.w%d", workers );
		BenchExecutor( label, ex, { 16u, 256u, 2048u }, { 0u, 256u } );
	}
	for ( int workers : { 0, 1, 4 } )
	{
		PoolBackend backend( workers );
		PooledExecutor ex( &backend );
		char label[32];
		std::snprintf( label, sizeof( label ), "pooled.w%d", workers );
		BenchExecutor( label, ex, { 16u, 256u, 2048u }, { 0u, 256u } );
	}

	// A fresh executor per run: the common "ParallelExecutor( n ).Execute" idiom.
	for ( int workers : { 1, 4 } )
	{
		GraphFixture fx;
		SealedGraph g = BuildGraph( Shape::Wide, 16, fx );
		char name[96];
		std::snprintf( name, sizeof( name ), "exec.parallel.fresh.w%d.wide/16", workers );
		Bench( name,
		    [&]
		    {
			    ClearOutputs( fx );
			    RunResult r = ParallelExecutor( workers ).Execute( g, RunOptions{} );
			    return VerifyRun( r, fx, 16 );
		    } );
	}

	// Mixed lanes: main-thread and blocking jobs interleaved with compute.
	for ( int workers : { 0, 4 } )
	{
		JobGraphBuilder b;
		std::vector<uint32_t> out( 256, 0 );
		std::vector<JobHandle> h;
		for ( uint32_t i = 0; i < 256; ++i )
		{
			JobDesc d;
			d.name = "bench.lane";
			d.executor = i % 8 == 0   ? Executor::MainThread()
			             : i % 8 == 1 ? Executor::BlockingIO()
			             : i % 8 == 2 ? Executor::Sequence( 1 )
			                          : Executor::Compute();
			uint32_t *slot = &out[i];
			d.function = [slot, i]( JobRunContext & )
			{
				*slot = i + 1;
			};
			h.push_back( b.AddJob( d ) );
			if ( i >= 8 )
				b.AddDependency( h[i - 8], h[i] );
		}
		auto sealed = b.Seal();
		BENCH_CHECK( sealed.HasValue() );
		SealedGraph g = std::move( sealed.Value() );
		ParallelExecutor ex( workers, workers ? 1 : 0 );
		char name[96];
		std::snprintf( name, sizeof( name ), "exec.parallel.w%d.lanes/256", workers );
		Bench( name,
		    [&]
		    {
			    std::fill( out.begin(), out.end(), 0u );
			    RunResult r = ex.Execute( g, RunOptions{} );
			    bool ok = r.succeeded == 256 && !r.stalled;
			    for ( uint32_t i = 0; i < 256; ++i )
				    ok = ok && out[i] == i + 1;
			    return ok;
		    } );
	}
}

struct BatchFixture
{
	std::vector<uint32_t> out;
	uint32_t steps = 0;
	std::atomic<int> begins{ 0 };
	std::atomic<int> ends{ 0 };
};

void BatchProcess( void *context, unsigned index )
{
	BatchFixture *fx = static_cast<BatchFixture *>( context );
	fx->out[index] = Spin( index, fx->steps ) | 1u;
}

void BatchBegin( void *context )
{
	static_cast<BatchFixture *>( context )->begins.fetch_add( 1, std::memory_order_relaxed );
}

void BatchEnd( void *context )
{
	static_cast<BatchFixture *>( context )->ends.fetch_add( 1, std::memory_order_relaxed );
}

void BenchBatches()
{
	for ( int workers : { 0, 1, 4 } )
	{
		PoolBackend backend( workers );
		for ( BatchMode mode : { BatchMode::Serial, BatchMode::Parallel } )
		{
			if ( mode == BatchMode::Serial && workers != 0 )
				continue; // serial never touches the backend
			for ( unsigned count : { 1u, 8u, 128u, 2048u } )
			{
				for ( uint32_t steps : { 0u, 256u } )
				{
					BatchFixture fx;
					fx.steps = steps;
					fx.out.assign( count, 0 );
					BatchDesc desc;
					desc.name = "bench.batch";
					desc.context = &fx;
					desc.count = count;
					desc.process = &BatchProcess;
					desc.begin = &BatchBegin;
					desc.end = &BatchEnd;
					desc.maxParticipants = 64;
					char name[96];
					std::snprintf( name, sizeof( name ), "batch.%s.w%d/%u/steps=%u",
					    mode == BatchMode::Serial ? "serial" : "parallel", workers, count, steps );
					Bench( name,
					    [&]
					    {
						    std::fill( fx.out.begin(), fx.out.end(), 0u );
						    fx.begins.store( 0 );
						    fx.ends.store( 0 );
						    bool ok = ExecuteParallelBatch( desc, &backend, mode );
						    ok = ok && fx.begins.load() == fx.ends.load() && fx.begins.load() >= 1;
						    for ( unsigned i = 0; i < count && ok; ++i )
							    ok = fx.out[i] == ( Spin( i, steps ) | 1u );
						    return ok;
					    } );
				}
			}
		}
	}
}

void BenchDynamic()
{
	for ( int workers : { 0, 4 } )
	{
		for ( uint32_t n : { 16u, 256u, 2048u } )
		{
			char name[96];
			std::snprintf( name, sizeof( name ), "dynamic.w%d.spawn/%u", workers, n );
			std::vector<uint32_t> out( n, 0 );
			Bench( name,
			    [&]
			    {
				    std::fill( out.begin(), out.end(), 0u );
				    DynamicScope scope( workers );
				    for ( uint32_t i = 0; i < n; ++i )
				    {
					    uint32_t *slot = &out[i];
					    scope.Spawn( "bench.child",
					        [slot, i]( JobRunContext & )
					        {
						        *slot = i + 1;
					        } );
				    }
				    scope.Wait();
				    bool ok = scope.Succeeded() == n && scope.Admitted() == n;
				    for ( uint32_t i = 0; i < n && ok; ++i )
					    ok = out[i] == i + 1;
				    return ok;
			    } );

			// Continuation chains spawned from inside running children.
			std::snprintf( name, sizeof( name ), "dynamic.w%d.continuations/%u", workers, n );
			std::atomic<uint32_t> ran{ 0 };
			Bench( name,
			    [&]
			    {
				    ran.store( 0 );
				    DynamicScope scope( workers );
				    const uint32_t chains = 8;
				    const uint32_t depth = n / chains;
				    std::function<void( uint32_t, DynamicScope::ChildHandle )> spawnNext;
				    spawnNext = [&]( uint32_t remaining, DynamicScope::ChildHandle prev )
				    {
					    std::vector<DynamicScope::ChildHandle> deps;
					    if ( prev.IsValid() )
						    deps.push_back( prev );
					    scope.Spawn(
					        "bench.cont",
					        [&, remaining]( JobRunContext &ctx )
					        {
						        ran.fetch_add( 1, std::memory_order_relaxed );
						        // A running child is admitted but not terminal, so its
						        // continuation may depend on it.
						        if ( remaining > 1 )
							        spawnNext(
							            remaining - 1, DynamicScope::ChildHandle{ ctx.JobId() } );
					        },
					        deps );
				    };
				    for ( uint32_t c = 0; c < chains; ++c )
					    spawnNext( depth, DynamicScope::ChildHandle{} );
				    scope.Wait();
				    return ran.load() == chains * depth && scope.Succeeded() == chains * depth;
			    } );
		}
	}
}

void BenchExternalCompletion()
{
	Bench( "external.complete_then_register",
	    []
	    {
		    ExternalCompletion token;
		    int fired = 0;
		    const bool first = token.Complete();
		    token.OnComplete(
		        [&]( CompletionResult r )
		        {
			        fired += r == CompletionResult::Completed;
		        } );
		    return first && fired == 1 && !token.Cancel();
	    } );
	Bench( "external.register_then_complete",
	    []
	    {
		    ExternalCompletion token;
		    int fired = 0;
		    token.OnComplete(
		        [&]( CompletionResult r )
		        {
			        fired += r == CompletionResult::Completed;
		        } );
		    const bool first = token.Complete();
		    return first && fired == 1 && token.Poll() == CompletionResult::Completed;
	    } );
}

void WriteJson( const char *path )
{
	FILE *f = std::fopen( path, "w" );
	if ( !f )
	{
		std::printf( "  FAIL cannot write %s\n", path );
		++g_failures;
		return;
	}
	std::fprintf( f,
	    "{\n  \"schema\": \"jobsystem-bench/v1\",\n  \"warmup\": %d,\n  \"samples\": %d,\n",
	    g_config.warmup, g_config.samples );
	std::fprintf(
	    f, "  \"hardware_threads\": %u,\n  \"results\": [\n", std::thread::hardware_concurrency() );
	for ( size_t i = 0; i < g_results.size(); ++i )
	{
		const Result &r = g_results[i];
		std::fprintf( f,
		    "    {\"name\": \"%s\", \"median_ns\": %.1f, \"p95_ns\": %.1f, \"min_ns\": %.1f, "
		    "\"samples\": %d, \"ops_per_sample\": %lld}%s\n",
		    r.name.c_str(), r.medianNs, r.p95Ns, r.minNs, r.samples, r.opsPerSample,
		    i + 1 < g_results.size() ? "," : "" );
	}
	std::fprintf( f, "  ]\n}\n" );
	std::fclose( f );
}

} // namespace

int main( int argc, char **argv )
{
	for ( int i = 1; i < argc; ++i )
	{
		if ( std::strcmp( argv[i], "--full" ) == 0 )
		{
			g_config.warmup = 20;
			g_config.samples = 51;
			g_config.minSampleUs = 2000.0;
		}
		else if ( std::strcmp( argv[i], "--filter" ) == 0 && i + 1 < argc )
			g_config.filter = argv[++i];
		else if ( std::strcmp( argv[i], "--json" ) == 0 && i + 1 < argc )
			g_config.jsonPath = argv[++i];
		else if ( std::strcmp( argv[i], "--samples" ) == 0 && i + 1 < argc )
			g_config.samples = std::max( 1, std::atoi( argv[++i] ) );
		else
		{
			std::printf(
			    "usage: %s [--full] [--samples N] [--filter substr] [--json path]\n", argv[0] );
			return 2;
		}
	}

	BenchSeal();
	BenchExecutors();
	BenchBatches();
	BenchDynamic();
	BenchExternalCompletion();

	if ( g_results.empty() )
	{
		std::printf( "  FAIL no benchmark selected\n" );
		++g_failures;
	}
	if ( g_config.jsonPath )
		WriteJson( g_config.jsonPath );
	std::printf(
	    "%zu benchmarks, %lld checks, %d failures\n", g_results.size(), g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
