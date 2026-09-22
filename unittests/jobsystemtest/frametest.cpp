//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frame-graph + particle-pilot tests (RFC 0003, "Tick and render-frame
//          composition" and Phase D). Demonstrates engine-shaped work riding on
//          the scheduler: an ordered legacy-phase host graph with a migrated
//          gather/compute/commit contributor, plus three-mode equivalence
//          (legacy reference vs serial graph vs parallel graph) on captured
//          inputs and private outputs, and the legacy/rollback path.
//
//=============================================================================//

#include "jobsystem/frame_graph.h"
#include "jobsystem/graph_executor.h"
#include "jobsystem/parallel_executor.h"
#include "jobsystem/pilot_particles.h"

#include <cstdio>
#include <string_view>
#include <vector>

using namespace jobsystem;

static int g_checks = 0, g_failures = 0;
static const char *g_curTest = "";
#define CHECK( cond ) do { ++g_checks; if ( !(cond) ) { ++g_failures; \
	std::printf( "  FAIL [%s] %s:%d: %s\n", g_curTest, __FILE__, __LINE__, #cond ); } } while (0)
#define RUN( fn ) do { g_curTest = #fn; std::printf( "- %s\n", #fn ); fn(); } while (0)

class OrderSink : public ITraceSink
{
public:
	std::vector<uint32_t> runOrder;
	void OnJobState( uint32_t id, const char *, ExecutorToken, JobState s ) override
	{
		if ( s == JobState::Running ) runOrder.push_back( id );
	}
};

static std::vector<double> MakeInputs( int n )
{
	std::vector<double> v( n );
	for ( int i = 0; i < n; ++i ) v[i] = 1.0 + i * 0.25 - ( i % 3 );
	return v;
}

static bool StatesEqual( const ParticleFrameState &a, const ParticleFrameState &b )
{
	return a.boneValue == b.boneValue
	    && a.controlPoints == b.controlPoints
	    && a.perSystemOut == b.perSystemOut
	    && a.changeDigest == b.changeDigest;
}

static FrameContext MakeFrame()
{
	FrameContext f; f.tickTime = 0.015; f.frameTime = 0.015; f.epoch = 42; return f;
}

// Build a full host frame around a pilot bound to `state`.
static Expected<SealedGraph, GraphError> BuildParticleFrame( ParticleFrameState *state,
	ParticlePilotContributor *pilot, FrameCoordinator &fc )
{
	JobHandle p0 = fc.AddLegacyPhase( "apply_input" );
	JobHandle p1 = fc.AddLegacyPhase( "stable_inputs" );
	JobHandle p2 = fc.AddLegacyPhase( "publish" );
	fc.AddLegacyPhase( "cleanup" );
	fc.SetContributorRegion( p1, p2 );
	fc.AddContributor( pilot );
	(void)p0; (void)state;
	return fc.Build( /*runContributors=*/true );
}

static void Test_LegacyPhasesOrderedRollback()
{
	FrameCoordinator fc( MakeFrame() );
	std::vector<int> order;
	auto mk = [&]( int idx ) { return [&order, idx]( JobRunContext & ){ order.push_back( idx ); }; };
	fc.AddLegacyPhase( "p0", mk( 0 ) );
	fc.AddLegacyPhase( "p1", mk( 1 ) );
	fc.AddLegacyPhase( "p2", mk( 2 ) );
	auto sealed = fc.Build( /*runContributors=*/false ); // rollback / legacy-only
	CHECK( sealed.HasValue() );
	if ( !sealed.HasValue() ) return;
	CHECK( sealed.Value().JobCount() == 3 );
	RunOptions opts; opts.frame = fc.Frame();
	DeterministicExecutor().Execute( sealed.Value(), opts );
	CHECK( order.size() == 3 );
	CHECK( order[0] == 0 && order[1] == 1 && order[2] == 2 );
}

static void Test_PilotThreeModeEquivalence()
{
	const int N = 24;
	std::vector<double> inputs = MakeInputs( N );

	// Legacy reference.
	ParticleFrameState ref; ref.inputs = inputs;
	ParticleReferenceUpdate( ref );

	// Serial graph.
	{
		ParticleFrameState s; s.inputs = inputs;
		ParticlePilotContributor pilot( &s );
		FrameCoordinator fc( MakeFrame() );
		auto sealed = BuildParticleFrame( &s, &pilot, fc );
		CHECK( sealed.HasValue() );
		if ( sealed.HasValue() )
		{
			RunOptions opts; opts.frame = fc.Frame();
			RunResult r = DeterministicExecutor().Execute( sealed.Value(), opts );
			CHECK( r.AllSucceeded() );
			CHECK( StatesEqual( s, ref ) );
		}
	}

	// Parallel graph, several worker counts and repetitions.
	for ( int workers = 1; workers <= 4; ++workers )
	{
		for ( int rep = 0; rep < 6; ++rep )
		{
			ParticleFrameState s; s.inputs = inputs;
			ParticlePilotContributor pilot( &s );
			FrameCoordinator fc( MakeFrame() );
			auto sealed = BuildParticleFrame( &s, &pilot, fc );
			CHECK( sealed.HasValue() );
			if ( !sealed.HasValue() ) continue;
			RunOptions opts; opts.frame = fc.Frame();
			RunResult r = ParallelExecutor( workers ).Execute( sealed.Value(), opts );
			CHECK( r.AllSucceeded() );
			CHECK( StatesEqual( s, ref ) );
		}
	}
}

static void Test_PilotResourceDeclarationsSeal()
{
	// The pilot's declared reads/writes (gather write, disjoint per-system
	// writes, ordered commit reads, bone attachment) must pass validation.
	ParticleFrameState s; s.inputs = MakeInputs( 8 );
	ParticlePilotContributor pilot( &s );
	FrameCoordinator fc( MakeFrame() );
	auto sealed = BuildParticleFrame( &s, &pilot, fc );
	CHECK( sealed.HasValue() );
	if ( sealed.HasValue() )
	{
		// 4 phases + gather + bone + 8 compute + commit = 15 jobs.
		CHECK( sealed.Value().JobCount() == 4 + 2 + 8 + 1 );
	}
}

static void Test_PilotCommitRunsAfterAllComputeBeforePublish()
{
	ParticleFrameState s; s.inputs = MakeInputs( 6 );
	ParticlePilotContributor pilot( &s );
	FrameCoordinator fc( MakeFrame() );
	auto sealed = BuildParticleFrame( &s, &pilot, fc );
	CHECK( sealed.HasValue() );
	if ( !sealed.HasValue() ) return;
	OrderSink sink; RunOptions opts; opts.frame = fc.Frame(); opts.trace = &sink;
	DeterministicExecutor().Execute( sealed.Value(), opts );

	// Find indices by name via the sealed graph.
	const SealedGraph &g = sealed.Value();
	int commitPos = -1, publishPos = -1, lastComputePos = -1, firstComputePos = 1 << 30;
	for ( size_t k = 0; k < sink.runOrder.size(); ++k )
	{
		const char *nm = g.GetJob( sink.runOrder[k] ).name;
		std::string_view s2( nm );
		if ( s2 == "particles.commit" ) commitPos = (int)k;
		else if ( s2 == "publish" )     publishPos = (int)k;
		else if ( s2 == "particles.compute" ) { lastComputePos = (int)k; if ( (int)k < firstComputePos ) firstComputePos = (int)k; }
	}
	CHECK( commitPos > lastComputePos );   // commit after every compute
	CHECK( publishPos > commitPos );        // publish after commit
}

int main()
{
	std::printf( "frametest (RFC 0003 frame graph + particle pilot)\n" );
	RUN( Test_LegacyPhasesOrderedRollback );
	RUN( Test_PilotThreeModeEquivalence );
	RUN( Test_PilotResourceDeclarationsSeal );
	RUN( Test_PilotCommitRunsAfterAllComputeBeforePublish );
	std::printf( "\n%d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
