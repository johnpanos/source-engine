//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Render-start frame graph oracle (RFC 0003 R21/R30, RFC 0005
//          Q-JOBS).
//
//          The client's render-start blocks (game/client/
//          client_render_start_steps.h) and the legacy OnRenderStart body
//          (renderstart_legacy_oracle.h, extracted from the pre-change source)
//          are compiled against the same recording stand-ins. Seeded scenarios
//          (bone and particle batch sizes, tools, frame time) run the legacy
//          body, the legacy node table, and the declared frame graph in serial
//          and pooled modes. Legacy table and serial graph must match the
//          legacy trace exactly; the pooled graph must match it after each
//          batch's items are sorted and runner brackets are removed, with every
//          item run once between its gather and commit. The declared edges of
//          the real table must leave no node pair unordered. Mutated tables
//          (dropped, swapped or duplicated nodes) must be detected.
//
//=============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "jobsystem/declared_frame_graph.h"
#include "jobsystem/worker_backend.h"
#include "testing/conformance_result.h"

static int s_checks = 0;
static int s_failures = 0;

static void Check( bool value, const char *expression, int line )
{
	++s_checks;
	if ( !value )
	{
		++s_failures;
		printf( "FAIL line %d: %s\n", line, expression );
		fflush( stdout );
	}
}
#define CHECK( expression ) Check( ( expression ), #expression, __LINE__ )

//=============================================================================
// Recording (items may record from worker threads in pooled runs)
//=============================================================================

static std::mutex s_traceMutex;
static std::vector<std::string> *s_pTrace = NULL;

static void Record( const std::string &event )
{
	std::lock_guard<std::mutex> lock( s_traceMutex );
	if ( s_pTrace )
		s_pTrace->push_back( event );
}

//=============================================================================
// Scenario
//=============================================================================

struct Frame
{
	int boneItems;     // previous-frame bone setups batched this frame (0 = none)
	int particleItems; // new particle effects simulated this frame
	bool tools;
	float frametime;
};

static const Frame *s_pFrame = NULL;

static Frame MakeFrame( unsigned seed )
{
	unsigned state = seed * 2654435761u + 7u;
	auto next = [&]
	{
		state = state * 1664525u + 1013904223u;
		return state >> 8;
	};
	Frame f;
	f.boneItems = ( next() % 3 == 0 ) ? 0 : (int)( 2 + next() % 40 );
	f.particleItems = (int)( next() % 50 );
	f.tools = next() % 4 == 0;
	f.frametime = 0.001f * (float)( 5 + next() % 30 );
	return f;
}

//=============================================================================
// Stand-ins for the client services the blocks call
//=============================================================================

#define PORTAL 1
#define REPLAY_ENABLED 1
#define VPROF( name ) ( (void)0 )
#define VPROF_( name, detail, group, assertAccounted, flags ) ( (void)0 )
#define VPROF_BUDGET( name, group ) ( (void)0 )
#define MDLCACHE_CRITICAL_SECTION() ( (void)0 )
#define MDLCACHE_COARSE_LOCK() ( (void)0 )
#define PREDICTION_TRACKVALUECHANGESCOPE( name ) ( (void)0 )
#define ARRAYSIZE( p ) ( sizeof( p ) / sizeof( p[0] ) )
#define Assert( x ) ( (void)0 )
#define PARTITION_ALL_CLIENT_EDICTS 0x1f
#define VPROF_BUDGETGROUP_CLIENT_SIM 0
#define VPROF_BUDGETGROUP_PARTICLE_SIMULATION 0
#define BUDGETFLAG_CLIENT 0

// Without FP_EXCEPTIONS_ENABLED the real enabler does nothing.
struct FPExceptionEnabler
{
	FPExceptionEnabler() {}
	~FPExceptionEnabler() {}
};

struct FakePortalRender
{
	void UpdatePortalPixelVisibility() { Record( "portal.pixelvis" ); }
};
static FakePortalRender s_portalRender;
FakePortalRender *g_pPortalRender = &s_portalRender;

struct FakePartition
{
	void SuppressLists( int mask, bool b )
	{
		Record( "partition.suppress " + std::to_string( mask ) + " " + std::to_string( b ) );
	}
};
static FakePartition s_partition;
FakePartition *partition = &s_partition;

void Rope_ResetCounters()
{
	Record( "Rope_ResetCounters" );
}

class C_BaseEntity
{
public:
	static void SetAbsQueriesValid( bool b )
	{
		Record( std::string( "SetAbsQueriesValid " ) + ( b ? "1" : "0" ) );
	}
	static void InterpolateServerEntities() { Record( "InterpolateServerEntities" ); }
	static void EnableAbsRecomputations( bool b )
	{
		Record( std::string( "EnableAbsRecomputations " ) + ( b ? "1" : "0" ) );
	}
	static void MarkAimEntsDirty() { Record( "MarkAimEntsDirty" ); }
	static void CalcAimEntPositions() { Record( "CalcAimEntPositions" ); }
	static void ToolRecordEntities() { Record( "ToolRecordEntities" ); }
	static void AddVisibleEntities() { Record( "AddVisibleEntities" ); }
};

// The previous-frame bone setup cohort: ThreadedBoneSetup is its three parts
// with the items in order, as in the client.
class C_BaseAnimating : public C_BaseEntity
{
public:
	static void InvalidateBoneCaches() { Record( "InvalidateBoneCaches" ); }
	static void PushAllowBoneAccess( bool a, bool b, const char *tag )
	{
		Record(
		    std::string( "PushAllowBoneAccess " ) + ( a ? "1" : "0" ) + ( b ? "1" : "0" ) + tag );
	}
	static void UpdateClientSideAnimations() { Record( "UpdateClientSideAnimations" ); }

	static void ThreadedBoneSetupBegin()
	{
		s_items = s_pFrame->boneItems;
		Record( "bones.begin " + std::to_string( s_items ) );
	}
	static unsigned ThreadedBoneSetupCount() { return (unsigned)s_items; }
	static void ThreadedBoneSetupItem( unsigned i )
	{
		Record( "bones.item " + std::to_string( i ) );
	}
	static void ThreadedBoneSetupRunnerBegin() { Record( "bones.runner.begin" ); }
	static void ThreadedBoneSetupRunnerEnd() { Record( "bones.runner.end" ); }
	static void ThreadedBoneSetupEnd()
	{
		Record( "bones.end" );
		s_items = 0;
	}
	static void ThreadedBoneSetup()
	{
		ThreadedBoneSetupBegin();
		if ( s_items )
		{
			ThreadedBoneSetupRunnerBegin();
			for ( int i = 0; i < s_items; i++ )
				ThreadedBoneSetupItem( (unsigned)i );
			ThreadedBoneSetupRunnerEnd();
		}
		ThreadedBoneSetupEnd();
	}

	static int s_items;
};
int C_BaseAnimating::s_items = 0;

struct FakeInput
{
	void CAM_Think() { Record( "CAM_Think" ); }
};
static FakeInput s_input;
FakeInput *input = &s_input;

struct FakeView
{
	void OnRenderStart() { Record( "view.OnRenderStart" ); }
};
static FakeView s_view;
FakeView *view = &s_view;

struct FakeRopeManager
{
	void OnRenderStart() { Record( "RopeManager.OnRenderStart" ); }
};
static FakeRopeManager s_ropeManager;
FakeRopeManager *RopeManager()
{
	return &s_ropeManager;
}

void ProcessOnDataChangedEvents()
{
	Record( "ProcessOnDataChangedEvents" );
}
float g_SmokeFogOverlayAlpha = 1.0f;
struct FakeColorCorrectionMgr
{
	void ResetColorCorrectionWeights()
	{
		Record( "ResetColorCorrectionWeights alpha=" + std::to_string( g_SmokeFogOverlayAlpha ) );
	}
};
static FakeColorCorrectionMgr s_colorCorrection;
FakeColorCorrectionMgr *g_pColorCorrectionMgr = &s_colorCorrection;
void SimulateEntities()
{
	Record( "SimulateEntities" );
}
void PhysicsSimulate()
{
	Record( "PhysicsSimulate" );
}

struct FakeEngine
{
	void FireEvents() { Record( "engine.FireEvents" ); }
};
static FakeEngine s_engine;
FakeEngine *engine = &s_engine;
struct FakeTempEnts
{
	void Update() { Record( "tempents.Update" ); }
};
static FakeTempEnts s_tempents;
FakeTempEnts *tempents = &s_tempents;
struct FakeBeams
{
	void UpdateTempEntBeams() { Record( "beams.UpdateTempEntBeams" ); }
};
static FakeBeams s_beams;
FakeBeams *beams = &s_beams;
void SetBeamCreationAllowed( bool b )
{
	Record( std::string( "SetBeamCreationAllowed " ) + ( b ? "1" : "0" ) );
}

struct FakeGlobals
{
	float frametime;
};
static FakeGlobals s_globals;
FakeGlobals *gpGlobals = &s_globals;

// The particle cohort: Simulate is its three parts with the items in order.
struct FakeParticleMgr
{
	int items = 0;
	void SimulateBegin( float dt )
	{
		items = s_pFrame->particleItems;
		Record( "particles.begin " + std::to_string( dt ) + " " + std::to_string( items ) );
	}
	unsigned SimulateBatchCount() const { return (unsigned)items; }
	void SimulateBatchItem( unsigned i ) { Record( "particles.item " + std::to_string( i ) ); }
	void SimulateEnd() { Record( "particles.end" ); }
	void Simulate( float dt )
	{
		SimulateBegin( dt );
		for ( int i = 0; i < items; i++ )
			SimulateBatchItem( (unsigned)i );
		SimulateEnd();
	}
};
static FakeParticleMgr s_particleMgr;
FakeParticleMgr *ParticleMgr()
{
	return &s_particleMgr;
}

bool ToolsEnabled()
{
	return s_pFrame->tools;
}

struct FakeReplayRagdoll
{
	const char *name;
	void Think() { Record( std::string( name ) + ".Think" ); }
};
struct CReplayRagdollRecorder
{
	static FakeReplayRagdoll &Instance()
	{
		static FakeReplayRagdoll s_recorder = { "ReplayRagdollRecorder" };
		return s_recorder;
	}
};
struct CReplayRagdollCache
{
	static FakeReplayRagdoll &Instance()
	{
		static FakeReplayRagdoll s_cache = { "ReplayRagdollCache" };
		return s_cache;
	}
};

//=============================================================================
// The implementations under comparison
//=============================================================================

#include "renderstart_legacy_oracle.h" // OnRenderStart (legacy)
#include "../../game/client/client_render_start_steps.h"

// Workers start before the caller's host nodes, like the engine pool bridge.
class ThreadBackend final : public jobsystem::IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ) {}
	int WorkerCount() const override { return m_workers; }
	void ParallelFor( int count, const std::function<void( int )> &body ) override
	{
		ParallelForWithCaller( count, body, [] {} );
	}
	void ParallelForWithCaller( int count, const std::function<void( int )> &body,
	    const std::function<void()> &caller ) override
	{
		std::atomic<int> next( 0 );
		auto drain = [&]
		{
			for ( int i = next++; i < count; i = next++ )
				body( i );
		};
		std::vector<std::thread> threads;
		for ( int w = 0; w < m_workers; ++w )
			threads.emplace_back( drain );
		caller();
		drain();
		for ( std::thread &t : threads )
			t.join();
	}

private:
	int m_workers;
};

enum Path
{
	PATH_LEGACY_BODY,
	PATH_LEGACY_TABLE,
	PATH_GRAPH_SERIAL,
	PATH_GRAPH_POOLED,
};

static const jobsystem::FrameNodeDesc *s_pNodes = s_RenderStartNodes;
static unsigned s_nNodes = ARRAYSIZE( s_RenderStartNodes );

static std::vector<std::string> RunFrame(
    const Frame &f, Path path, jobsystem::DeclaredFrameGraph &graph, ThreadBackend &backend )
{
	std::vector<std::string> trace;
	s_pFrame = &f;
	gpGlobals->frametime = f.frametime;
	g_SmokeFogOverlayAlpha = 1.0f;
	s_pTrace = &trace;
	switch ( path )
	{
	case PATH_LEGACY_BODY:
		OnRenderStart();
		break;
	case PATH_LEGACY_TABLE:
		CRenderStartSteps::RunHostNodes(
		    s_RenderStartLegacyNodes, ARRAYSIZE( s_RenderStartLegacyNodes ) );
		break;
	case PATH_GRAPH_SERIAL:
	case PATH_GRAPH_POOLED:
	{
		jobsystem::DeclaredFrameRun run = graph.Run( s_pNodes, s_nNodes, &backend,
		    path == PATH_GRAPH_SERIAL ? jobsystem::FRAME_GRAPH_SERIAL
		                              : jobsystem::FRAME_GRAPH_POOLED );
		if ( !run.valid )
			trace.push_back( "graph invalid" );
		break;
	}
	}
	s_pTrace = NULL;
	return trace;
}

// Pooled runs: items of a batch may run in any order on any runner. Remove
// runner brackets and sort each run of consecutive items; the host events
// and the positions of the batches must then match the legacy trace.
static std::vector<std::string> Normalize( std::vector<std::string> trace )
{
	std::vector<std::string> out;
	for ( const std::string &e : trace )
	{
		if ( e == "bones.runner.begin" || e == "bones.runner.end" )
			continue;
		out.push_back( e );
	}
	// Items of different batches are never adjacent (host events separate
	// them), so each run of consecutive item events is one batch.
	auto isItem = []( const std::string &e )
	{
		return e.find( ".item " ) != std::string::npos;
	};
	auto number = []( const std::string &e )
	{
		return atoi( e.c_str() + e.rfind( ' ' ) + 1 );
	};
	for ( size_t i = 0; i < out.size(); )
	{
		size_t j = i;
		while ( j < out.size() && isItem( out[j] ) )
			++j;
		if ( j == i )
		{
			++i;
			continue;
		}
		std::sort( out.begin() + i, out.begin() + j,
		    [&]( const std::string &a, const std::string &b )
		    {
			    return number( a ) < number( b );
		    } );
		i = j;
	}
	return out;
}

static void TestEquivalence( int nFrames )
{
	jobsystem::DeclaredFrameGraph graph;
	ThreadBackend backend( 3 );
	int mismatches = 0, boneBatches = 0, particleBatches = 0, pooledSplit = 0;
	for ( int seed = 1; seed <= nFrames; seed++ )
	{
		const Frame f = MakeFrame( (unsigned)seed );
		const std::vector<std::string> legacy = RunFrame( f, PATH_LEGACY_BODY, graph, backend );
		const std::vector<std::string> table = RunFrame( f, PATH_LEGACY_TABLE, graph, backend );
		const std::vector<std::string> serial = RunFrame( f, PATH_GRAPH_SERIAL, graph, backend );
		const std::vector<std::string> pooled = RunFrame( f, PATH_GRAPH_POOLED, graph, backend );
		const bool ok = !legacy.empty() && table == legacy && serial == legacy &&
		                Normalize( pooled ) == Normalize( legacy );
		CHECK( ok );
		if ( !ok && ++mismatches <= 2 )
		{
			printf( "seed %d: legacy %zu table %zu serial %zu pooled %zu events\n", seed,
			    legacy.size(), table.size(), serial.size(), pooled.size() );
			const std::vector<std::string> a = Normalize( legacy ), b = Normalize( pooled );
			for ( size_t i = 0; i < a.size() || i < b.size(); i++ )
			{
				const std::string x = i < a.size() ? a[i] : "<end>",
				                  y = i < b.size() ? b[i] : "<end>";
				if ( x != y )
				{
					printf( "  first difference at %zu: legacy '%s' pooled '%s'\n", i, x.c_str(),
					    y.c_str() );
					break;
				}
			}
		}
		boneBatches += f.boneItems ? 1 : 0;
		particleBatches += f.particleItems ? 1 : 0;
		int runnerBegins = 0;
		for ( const std::string &e : pooled )
			runnerBegins += e == "bones.runner.begin" ? 1 : 0;
		pooledSplit += runnerBegins > 1 ? 1 : 0;
	}
	printf( "equivalence: %d frames, %d bone batches, %d particle batches, %d pooled bone batches "
	        "split across runners\n",
	    nFrames, boneBatches, particleBatches, pooledSplit );
	CHECK( boneBatches > 0 && particleBatches > 0 );
	CHECK( pooledSplit > 0 ); // the pooled path really distributed items
}

// The declared edges of the real table order every pair of nodes.
static void TestDeclaredOrder()
{
	jobsystem::DeclaredFrameGraph graph;
	ThreadBackend backend( 3 );
	Frame f = MakeFrame( 1 );
	s_pFrame = &f;
	std::vector<std::string> trace;
	s_pTrace = &trace;
	CHECK( graph
	        .Run( s_RenderStartNodes, ARRAYSIZE( s_RenderStartNodes ), &backend,
	            jobsystem::FRAME_GRAPH_POOLED )
	        .valid );
	s_pTrace = NULL;
	printf( "declared order: %u overlapping node pairs of %u nodes\n", graph.OverlappingPairs(),
	    (unsigned)ARRAYSIZE( s_RenderStartNodes ) );
	CHECK( graph.OverlappingPairs() == 0 );
}

// Mutated node tables must be detected by the comparison.
static void TestSensitivity()
{
	const unsigned n = ARRAYSIZE( s_RenderStartNodes );
	std::vector<jobsystem::FrameNodeDesc> base( s_RenderStartNodes, s_RenderStartNodes + n );
	jobsystem::DeclaredFrameGraph graph;
	ThreadBackend backend( 3 );
	int checked = 0;
	for ( int kind = 0; kind < 3; kind++ )
	{
		for ( unsigned k = 0; k + ( kind == 0 ? 1 : 0 ) < n; k++ )
		{
			std::vector<jobsystem::FrameNodeDesc> mutated = base;
			if ( kind == 0 )
				std::swap( mutated[k], mutated[k + 1] );
			else if ( kind == 1 )
				mutated.erase( mutated.begin() + k );
			else
				mutated.insert( mutated.begin() + k, mutated[k] );
			s_pNodes = mutated.data();
			s_nNodes = (unsigned)mutated.size();
			bool detected = false;
			for ( int seed = 1; seed <= 200 && !detected; seed++ )
			{
				const Frame f = MakeFrame( (unsigned)seed );
				detected = RunFrame( f, PATH_GRAPH_SERIAL, graph, backend ) !=
				           RunFrame( f, PATH_LEGACY_BODY, graph, backend );
			}
			// Replay is an empty block without REPLAY_ENABLED only; here it records.
			if ( !detected )
				printf( "undetected mutation %d at node %u (%s)\n", kind, k, base[k].name );
			CHECK( detected );
			checked++;
		}
	}
	s_pNodes = s_RenderStartNodes;
	s_nNodes = n;
	printf( "sensitivity: %d mutated tables\n", checked );
}

int main( int argc, char **argv )
{
	int nFrames = 2000;
	if ( argc > 2 && strcmp( argv[1], "--frames" ) == 0 )
		nFrames = atoi( argv[2] );
	TestEquivalence( nFrames );
	TestDeclaredOrder();
	TestSensitivity();
	return testing::ReportConformance( s_checks, s_failures );
}
