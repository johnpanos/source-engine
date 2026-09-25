//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Host frame graph equivalence oracle (RFC 0003 R10, RFC 0005
//          Q-JOBS).
//
//          The engine's host frame kernel (engine/host_frame_graph.h) and the
//          legacy hand-ordered _Host_RunFrame body (hostframe_legacy_oracle.h,
//          extracted from the pre-change source) are compiled against the same
//          recording stand-ins for every engine service they call. Seeded
//          scenarios drive both through multi-frame sequences covering the
//          unthreaded and threaded paths, dedicated and listen servers, tick
//          counts 0-4, render/no-render, timedemo/skip hacks, HLTV appearing
//          mid-frame, single-player async-server joins, carry-over state, and
//          Host_EndGame / Host_AbortServer longjmps from inside phases. Each
//          call records its arguments plus a snapshot of the observable host
//          state; the traces must be identical. Mutated phase lists (swapped,
//          dropped, duplicated phases) must be detected. The serial frame
//          graph facade's own contract is checked directly.
//
//=============================================================================//

#include <math.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <chrono>
#include <string>
#include <vector>

#include "jobsystem/serial_frame_graph.h"
#include "testing/conformance_result.h"
#include "../../engine/host_frame_graph.h" // frame state types

static unsigned long s_checks = 0;
static unsigned long s_failures = 0;

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
// Recording
//=============================================================================

static std::vector<std::string> *s_pTrace = NULL;
static std::string Snapshot();
static std::string RenderSnapshot();

static void Record( const char *fmt, ... )
{
	char buffer[256];
	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );
	if ( s_pTrace )
		s_pTrace->push_back( std::string( buffer ) + " | " + Snapshot() );
}

//=============================================================================
// Scenario script (deterministic per seed)
//=============================================================================

struct Rng
{
	unsigned state;
	unsigned Next()
	{
		state = state * 1664525u + 1013904223u;
		return state >> 8;
	}
	int Range( int lo, int hi ) { return lo + (int)( Next() % (unsigned)( hi - lo + 1 ) ); }
	bool Chance( int percent ) { return (int)( Next() % 100u ) < percent; }
};

enum InjectKind
{
	INJECT_NONE,
	INJECT_ENDDEMO,
	INJECT_ABORTSERVER,
};

struct FrameScript
{
	int numticks;
	double prevremainder;
	double host_remainder;
	bool shouldrender;
	float frametime;
	// A primitive longjmps out of the frame on its n-th call.
	InjectKind inject;
	const char *injectAt;
	int injectCall;
	// Mid-frame world changes (runtime guards must see them when they run).
	int hltvAppearsOnServerTick;
	bool timedemo;
	bool skipping;
	bool playbackPaused;
	// Render stage guards (SCR_UpdateScreen early-outs and friends).
	bool disabledForLoading;
	bool screenInitialized;
	bool consoleInitialized;
	bool demoPlayingBack;
	bool windowSizeOk;
	bool takeSnapshot;
	int noRendering;
	bool launcherMgr;
	int nextDrawTickSet; // written to scr_nextdrawtick before the frame when nonzero
};

struct Scenario
{
	unsigned seed;
	bool dedicated;
	bool threaded;
	bool singlePlayer;
	bool hasClientDLL;
	bool hltvAtStart;
	bool hltvTest;
	bool netDedicated;
	bool active;
	int nextdrawtick;
	std::vector<FrameScript> frames;
};

static Scenario MakeScenario( unsigned seed )
{
	Rng rng = { seed * 2654435761u + 1u };
	Scenario sc;
	sc.seed = seed;
	sc.dedicated = rng.Chance( 25 );
	sc.threaded = !sc.dedicated && rng.Chance( 50 );
	sc.singlePlayer = rng.Chance( 50 );
	sc.hasClientDLL = rng.Chance( 85 );
	sc.hltvAtStart = rng.Chance( 20 );
	sc.hltvTest = rng.Chance( 10 );
	sc.netDedicated = sc.dedicated && rng.Chance( 70 );
	sc.active = rng.Chance( 80 );
	sc.nextdrawtick = rng.Chance( 70 ) ? 0 : 5;

	static const char *s_injectSites[] = { "Cbuf_Execute", "Input", "Server", "Client", "Render",
	    "ClientDLL_Update", "ServerAsync", "CL_RunPrediction", "SV_FrameExecuteThreadDeferred",
	    "FrameStageNotify", "V_RenderView", "CL_DecayLights" };

	const int nFrames = rng.Range( 1, 6 );
	for ( int i = 0; i < nFrames; i++ )
	{
		FrameScript f;
		f.numticks = rng.Range( 0, 4 );
		f.prevremainder = rng.Range( 0, 14 ) * 0.001;
		f.host_remainder = rng.Range( 0, 14 ) * 0.001;
		f.shouldrender = !sc.dedicated;
		f.frametime = 0.001f * rng.Range( 5, 40 );
		f.inject = INJECT_NONE;
		f.injectAt = NULL;
		f.injectCall = 0;
		if ( rng.Chance( 15 ) )
		{
			f.inject = rng.Chance( 50 ) ? INJECT_ENDDEMO : INJECT_ABORTSERVER;
			f.injectAt = s_injectSites[rng.Range(
			    0, (int)( sizeof( s_injectSites ) / sizeof( s_injectSites[0] ) ) - 1 )];
			f.injectCall = rng.Range( 1, 2 );
		}
		f.hltvAppearsOnServerTick = rng.Chance( 20 ) ? rng.Range( 1, 3 ) : 0;
		f.timedemo = rng.Chance( 15 );
		f.skipping = rng.Chance( 10 );
		f.playbackPaused = rng.Chance( 10 );
		f.disabledForLoading = rng.Chance( 10 );
		f.screenInitialized = !rng.Chance( 5 );
		f.consoleInitialized = !rng.Chance( 5 );
		f.demoPlayingBack = rng.Chance( 20 );
		f.windowSizeOk = !rng.Chance( 10 );
		f.takeSnapshot = rng.Chance( 15 );
		f.noRendering = rng.Chance( 20 ) ? 1 : 0;
		f.launcherMgr = !rng.Chance( 10 );
		f.nextDrawTickSet = rng.Chance( 10 ) ? rng.Range( 1, 12 ) : 0;
		sc.frames.push_back( f );
	}
	return sc;
}

static const Scenario *s_pScenario = NULL;
static const FrameScript *s_pFrame = NULL;

//=============================================================================
// Stand-ins for the engine services used by the legacy body and the kernel
//=============================================================================

#define VPROF( name ) ( (void)0 )
#define VPROF_BUDGET( name, group ) ( (void)0 )
#define tmZone( ... ) ( (void)0 )
#define ARRAYSIZE( p ) ( sizeof( p ) / sizeof( p[0] ) )
#define LOG_FRAME_OUTPUT 0
#define PREDICTION_NORMAL 1

jmp_buf host_enddemo;
jmp_buf host_abortserver;

// Injected error exits, as Host_EndGame / Host_AbortServer would perform.
static int s_injectCounter = 0;
static void MaybeInject( const char *site )
{
	if ( !s_pFrame || s_pFrame->inject == INJECT_NONE || strcmp( site, s_pFrame->injectAt ) != 0 )
		return;
	if ( ++s_injectCounter != s_pFrame->injectCall )
		return;
	if ( s_pFrame->inject == INJECT_ENDDEMO )
	{
		Record( "Host_EndGame longjmp" );
		longjmp( host_enddemo, 1 );
	}
	Record( "Host_AbortServer longjmp" );
	longjmp( host_abortserver, 1 );
}

enum
{
	FRAME_SEGMENT_INPUT = 0,
	FRAME_SEGMENT_CLIENT,
	FRAME_SEGMENT_SERVER,
	FRAME_SEGMENT_RENDER,
	FRAME_SEGMENT_SOUND,
	FRAME_SEGMENT_CLDLL,
	FRAME_SEGMENT_CMD_EXECUTE,
};

struct FakeHostTimes
{
	void StartFrameSegment( int i ) { Record( "StartFrameSegment %d", i ); }
	void EndFrameSegment( int i ) { Record( "EndFrameSegment %d", i ); }
	void MarkSwapTime() { Record( "MarkSwapTime" ); }
} g_HostTimes;

struct FakeGlobals
{
	float interpolation_amount;
	int simTicksThisFrame;
	int tickcount;
	float frametime;
};
FakeGlobals g_ServerGlobalVariables;
FakeGlobals g_ClientGlobalVariables;

struct FakeClientState
{
	bool insimulation;
	float m_tickRemainder;
	float frameTime;
	int clientTick;
	void SetFrameTime( float t )
	{
		frameTime = t;
		Record( "cl.SetFrameTime %.6f", t );
	}
	int GetClientTickCount() { return clientTick; }
	void UpdateAreaBits_BackwardsCompatible() { Record( "cl.UpdateAreaBits" ); }
	bool IsActive() { return s_pScenario->active; }
	bool IsPaused() { return false; }
} cl;

struct FakeServer
{
	int m_nTickCount;
	bool IsDedicated() { return s_pScenario->dedicated; }
	bool IsPaused() { return false; }
} sv;

struct FakeHostState
{
	float interval_per_tick;
} host_state;

float host_frametime;
float host_time;
int host_frameticks;
int host_tickcount;
int host_currentframetick;
int host_framecount;
int scr_nextdrawtick;
double host_idealtime;
float host_jitterhistory[128];
unsigned int host_jitterhistorypos;
static bool host_checkheap = false;
bool g_bVCRSingleStep = false;
static bool s_netMultiplayer = false;
static bool g_bThreadedEngine = false;
#define IsEngineThreaded() ( g_bThreadedEngine )
static double s_clock = 0.0;

static std::string Snapshot()
{
	char buffer[512];
	snprintf( buffer, sizeof( buffer ),
	    "ht=%d cft=%d hft=%d hfc=%d svt=%d sg=%d/%d/%.4f cg=%d/%d/%.4f/%.4f ins=%d rem=%.5f "
	    "clft=%.5f "
	    "ideal=%.5f jpos=%u htime=%.5f mp=%d clt=%d",
	    host_tickcount, host_currentframetick, host_frameticks, host_framecount, sv.m_nTickCount,
	    g_ServerGlobalVariables.tickcount, g_ServerGlobalVariables.simTicksThisFrame,
	    g_ServerGlobalVariables.interpolation_amount, g_ClientGlobalVariables.tickcount,
	    g_ClientGlobalVariables.simTicksThisFrame, g_ClientGlobalVariables.interpolation_amount,
	    g_ClientGlobalVariables.frametime, cl.insimulation ? 1 : 0, cl.m_tickRemainder,
	    cl.frameTime, host_idealtime, host_jitterhistorypos, host_time, s_netMultiplayer ? 1 : 0,
	    cl.clientTick );
	return std::string( buffer ) + RenderSnapshot();
}

void Cbuf_Execute()
{
	Record( "Cbuf_Execute" );
	MaybeInject( "Cbuf_Execute" );
}
bool NET_IsDedicated()
{
	return s_pScenario->netDedicated;
}
bool NET_IsMultiplayer()
{
	return s_netMultiplayer;
}
void NET_SetMutiplayer( bool b )
{
	s_netMultiplayer = b;
	Record( "NET_SetMutiplayer %d", b ? 1 : 0 );
}
void NET_RunFrame( double t )
{
	Record( "NET_RunFrame %.6f", t );
}
void NET_SendQueuedPackets()
{
	Record( "NET_SendQueuedPackets" );
}
void NET_SetTime( double t )
{
	Record( "NET_SetTime %.6f", t );
}
void ETWSimFrameMark( bool b )
{
	Record( "ETWSimFrameMark %d", b ? 1 : 0 );
}

// A jittery clock: occasional large jumps exercise the ideal-time reset.
double Plat_FloatTime()
{
	s_clock += ( ( (int)( s_clock * 1000.0 ) % 7 ) == 3 ) ? 1.5 : 0.013;
	return s_clock;
}

void CL_CheckClientState()
{
	Record( "CL_CheckClientState" );
}
void CL_RunPrediction( int mode )
{
	Record( "CL_RunPrediction %d", mode );
	MaybeInject( "CL_RunPrediction" );
}
void CL_ApplyAddAngle()
{
	Record( "CL_ApplyAddAngle" );
}
void CL_ExtraMouseUpdate( float t )
{
	Record( "CL_ExtraMouseUpdate %.6f", t );
}
void ClientDLL_Update()
{
	Record( "ClientDLL_Update" );
	MaybeInject( "ClientDLL_Update" );
}
void SV_FrameExecuteThreadDeferred()
{
	Record( "SV_FrameExecuteThreadDeferred" );
	MaybeInject( "SV_FrameExecuteThreadDeferred" );
}
bool Host_IsSinglePlayerGame()
{
	return s_pScenario->singlePlayer;
}
void Host_SetClientInSimulation( bool b )
{
	cl.insimulation = b;
	Record( "Host_SetClientInSimulation %d", b ? 1 : 0 );
}
void Host_Speeds()
{
	Record( "Host_Speeds" );
}
void Host_UpdateMapList()
{
	Record( "Host_UpdateMapList" );
}
void Host_PostFrameRate( float t )
{
	Record( "Host_PostFrameRate %.6f", t );
}
void Host_CheckDumpMemoryStats( void )
{
	Record( "Host_CheckDumpMemoryStats" );
}
void Host_ShowIPCCallCount()
{
	Record( "Host_ShowIPCCallCount" );
}
void Sys_Error( const char *, ... )
{
	Record( "Sys_Error" );
}
void VCR_EnterPausedState()
{
	Record( "VCR_EnterPausedState" );
}

struct FakeHltv
{
	const char *name;
	void RunFrame() { Record( "%s.RunFrame", name ); }
};
static FakeHltv s_hltv = { "hltv" };
static FakeHltv s_hltvTest = { "hltvtest" };
FakeHltv *hltv = NULL;
FakeHltv *hltvtest = NULL;

void _Host_RunFrame_Input( float accumulated_extra_samples, bool bFinalTick )
{
	Record( "Input %.6f %d", accumulated_extra_samples, bFinalTick ? 1 : 0 );
	MaybeInject( "Input" );
}

static int s_serverCalls = 0;
void _Host_RunFrame_Server( bool finaltick )
{
	++sv.m_nTickCount;
	Record( "Server %d", finaltick ? 1 : 0 );
	if ( s_pFrame && ++s_serverCalls == s_pFrame->hltvAppearsOnServerTick )
		hltv = &s_hltv;
	MaybeInject( "Server" );
}

void _Host_RunFrame_Server_Async( int numticks )
{
	Record( "ServerAsync %d", numticks );
	MaybeInject( "ServerAsync" );
	for ( int tick = 0; tick < numticks; tick++ )
		_Host_RunFrame_Server( tick == numticks - 1 );
}

void _Host_RunFrame_Client( bool framefinished )
{
	++cl.clientTick;
	Record( "Client %d", framefinished ? 1 : 0 );
	MaybeInject( "Client" );
}
void _Host_RunFrame_Sound()
{
	Record( "Sound" );
}

struct FakeEngineTool
{
	void SetIsInGame( bool b ) { Record( "SetIsInGame %d", b ? 1 : 0 ); }
};
static FakeEngineTool s_engineTool;
FakeEngineTool *g_pEngineToolInternal = &s_engineTool;

struct FakeClientDLL
{
	void IN_SetSampleTime( float t ) { Record( "IN_SetSampleTime %.6f", t ); }
};
static FakeClientDLL s_clientDLL;
FakeClientDLL *g_ClientDLL = NULL;

struct FakeToolFramework
{
	void Think( bool b ) { Record( "toolframework.Think %d", b ? 1 : 0 ); }
	void RenderFrameBegin() { Record( "toolframework.RenderFrameBegin" ); }
	void RenderFrameEnd() { Record( "toolframework.RenderFrameEnd" ); }
};
static FakeToolFramework s_toolFramework;
FakeToolFramework *toolframework = &s_toolFramework;

struct FakeDemoPlayer
{
	bool IsPlayingBack() { return s_pFrame && s_pFrame->demoPlayingBack; }
	void InterpolateViewpoint() { Record( "demoplayer.InterpolateViewpoint" ); }
	bool IsPlayingTimeDemo() { return s_pFrame && s_pFrame->timedemo; }
	bool IsSkipping() { return s_pFrame && s_pFrame->skipping; }
	bool IsPlaybackPaused() { return s_pFrame && s_pFrame->playbackPaused; }
};
static FakeDemoPlayer s_demoPlayer;
FakeDemoPlayer *demoplayer = &s_demoPlayer;

struct FakeLog
{
	void RunFrame() { Record( "g_Log.RunFrame" ); }
} g_Log;

struct FakeModelLoader
{
	void UpdateDynamicModels() { Record( "UpdateDynamicModels" ); }
};
static FakeModelLoader s_modelLoader;
FakeModelLoader *modelloader = &s_modelLoader;

struct FakeMDLCache
{
};
static FakeMDLCache s_mdlCache;
FakeMDLCache *g_pMDLCache = &s_mdlCache;
struct CMDLCacheCriticalSection
{
	explicit CMDLCacheCriticalSection( FakeMDLCache * ) { Record( "MDLCache lock" ); }
	~CMDLCacheCriticalSection() { Record( "MDLCache unlock" ); }
};

//-----------------------------------------------------------------------------
// Render stage services (legacy _Host_RunFrame_Render / SCR_UpdateScreen and
// engine/host_render_steps.h call the same ones)
//-----------------------------------------------------------------------------

#define USE_SDL 1
#define TELEMETRY_LEVEL0 0
#define TMZF_NONE 0

enum ClientFrameStage_t
{
	FRAME_RENDER_START = 5,
	FRAME_RENDER_END = 6,
};

struct FakeConVar
{
	const char *name;
	int value;
	int GetInt()
	{
		Record( "%s.GetInt", name );
		return value;
	}
	void SetValue( int v )
	{
		value = v;
		Record( "%s.SetValue %d", name, v );
	}
};
FakeConVar mat_norendering = { "mat_norendering", 0 };
bool cl_takesnapshot = false;
bool scr_disabled_for_loading = false;
bool scr_initialized = true;
bool con_initialized = true;

void CheckSpecialCheatVars()
{
	Record( "CheckSpecialCheatVars" );
	MaybeInject( "Render" );
}
void CL_LatchInterpolationAmount()
{
	Record( "CL_LatchInterpolationAmount" );
}
void R_StudioCheckReinitLightingCache()
{
	Record( "R_StudioCheckReinitLightingCache" );
}
bool V_CheckGamma()
{
	Record( "V_CheckGamma" );
	return true;
}
void V_RenderVGuiOnly()
{
	Record( "V_RenderVGuiOnly" );
}
void SCR_ShowVCRPlaybackAmount()
{
	Record( "SCR_ShowVCRPlaybackAmount" );
}
bool VideoMode_UpdateWindowSize()
{
	Record( "VideoMode_UpdateWindowSize" );
	return !s_pFrame || s_pFrame->windowSizeOk;
}
struct FakeMaterials
{
	void BeginFrame( float t ) { Record( "materials.BeginFrame %.6f", t ); }
	void EndFrame() { Record( "materials.EndFrame" ); }
};
static FakeMaterials s_materials;
FakeMaterials *materials = &s_materials;
struct FakeEngineVGui
{
	void Simulate() { Record( "EngineVGui.Simulate" ); }
};
static FakeEngineVGui s_engineVGui;
FakeEngineVGui *EngineVGui()
{
	return &s_engineVGui;
}
void ClientDLL_FrameStageNotify( ClientFrameStage_t stage )
{
	Record( "ClientDLL_FrameStageNotify %d", (int)stage );
	MaybeInject( "FrameStageNotify" );
}
struct FakeEngineRenderer
{
	void FrameBegin() { Record( "g_EngineRenderer.FrameBegin" ); }
	void FrameEnd() { Record( "g_EngineRenderer.FrameEnd" ); }
};
static FakeEngineRenderer s_engineRenderer;
FakeEngineRenderer *g_EngineRenderer = &s_engineRenderer;
void Shader_BeginRendering()
{
	Record( "Shader_BeginRendering" );
}
void V_RenderView()
{
	Record( "V_RenderView" );
	MaybeInject( "V_RenderView" );
}
void CL_TakeSnapshotAndSwap()
{
	Record( "CL_TakeSnapshotAndSwap" );
}
void CL_DecayLights()
{
	Record( "CL_DecayLights" );
	MaybeInject( "CL_DecayLights" );
}
struct FakeSaveRestore
{
	void OnFrameRendered() { Record( "saverestore.OnFrameRendered" ); }
};
static FakeSaveRestore s_saveRestore;
FakeSaveRestore *saverestore = &s_saveRestore;
struct FakeLauncherMgr
{
	void OnFrameRendered() { Record( "launcher.OnFrameRendered" ); }
};
static FakeLauncherMgr s_launcherMgr;
FakeLauncherMgr *g_pLauncherMgr = NULL;
// Live capture only; not part of the compared behavior.
void Host_TraceFrameEvent( const char *, int )
{
}
void Host_TraceServerJob( bool )
{
}
void SCR_UpdateScreen( void );
void _Host_RunFrame_Render(); // the verbatim legacy stage (hostframe_render_legacy_oracle.h)

static std::string RenderSnapshot()
{
	char buffer[64];
	snprintf( buffer, sizeof( buffer ), " ndt=%d nr=%d", scr_nextdrawtick, mat_norendering.value );
	return buffer;
}

struct FakeTestScriptMgr
{
	void CheckPoint( const char *name ) { Record( "CheckPoint %s", name ); }
};
static FakeTestScriptMgr s_testScriptMgr;
FakeTestScriptMgr *GetTestScriptMgr()
{
	return &s_testScriptMgr;
}

// Async server job: the threaded path submits the server frame and joins it.
struct FakeFunctor
{
	void ( *fn )( int );
	int arg;
};
static FakeFunctor CreateFunctor( void ( *fn )( int ), int arg )
{
	FakeFunctor functor = { fn, arg };
	return functor;
}

class CJob
{
public:
	virtual ~CJob() {}
	virtual void Run() = 0;
	void ExecuteAndRelease()
	{
		Record( "job.ExecuteAndRelease" );
		Run();
		delete this;
	}
	void WaitForFinishAndRelease()
	{
		Record( "job.WaitForFinishAndRelease" );
		Run();
		delete this;
	}
};

class CFunctorJob : public CJob
{
public:
	explicit CFunctorJob( const FakeFunctor &functor ) : m_functor( functor ) {}
	virtual void Run() { m_functor.fn( m_functor.arg ); }

private:
	FakeFunctor m_functor;
};

struct FakeThreadPool
{
	void AddJob( CJob * ) { Record( "pool.AddJob" ); }
};
static FakeThreadPool s_threadPool;
FakeThreadPool *g_pThreadPool = &s_threadPool;

// The minimal CUtlVector surface the kernel uses.
template <typename T> class CUtlVector
{
public:
	void RemoveAll() { m_items.clear(); }
	void AddToTail( const T &item ) { m_items.push_back( item ); }
	T *Base() { return m_items.empty() ? NULL : &m_items[0]; }
	int Count() const { return (int)m_items.size(); }
	std::vector<T> m_items;
};

// Phase-list mutation for the sensitivity runs; applied by the stand-in for
// the vstdlib export, so the real kernel builds the list being mutated.
enum Mutation
{
	MUTATE_NONE,
	MUTATE_SWAP,      // swap phases k and k+1
	MUTATE_DROP,      // omit phase k
	MUTATE_DUPLICATE, // run phase k twice
};
static Mutation s_mutation = MUTATE_NONE;
static unsigned s_mutationIndex = 0;
static bool s_mutationApplied = false;
static bool s_mutationTouchesNoOp = false;

bool RunSerialFrameGraph( jobsystem::SerialFrameGraph *pGraph,
    const jobsystem::FramePhaseDesc *pPhases, unsigned nPhases, jobsystem::SerialFrameRun *pResult )
{
	if ( s_mutation == MUTATE_NONE || s_mutationIndex + 1 >= nPhases )
	{
		*pResult = pGraph->Run( pPhases, nPhases );
		return pResult->valid;
	}
	// A mutation that moves a phase with no behavior in this build (the
	// _DEBUG-only UpdateScreenBegin step) is equivalent, not undetectable: that
	// frame runs unmutated.
	const unsigned last = s_mutation == MUTATE_SWAP ? s_mutationIndex + 1 : s_mutationIndex;
	for ( unsigned i = s_mutationIndex; i <= last; i++ )
	{
		if ( strcmp( pPhases[i].name, "UpdateScreenBegin" ) == 0 )
		{
			s_mutationTouchesNoOp = true;
			*pResult = pGraph->Run( pPhases, nPhases );
			return pResult->valid;
		}
	}
	std::vector<jobsystem::FramePhaseDesc> mutated( pPhases, pPhases + nPhases );
	s_mutationApplied = true;
	switch ( s_mutation )
	{
	case MUTATE_SWAP:
	{
		jobsystem::FramePhaseDesc tmp = mutated[s_mutationIndex];
		mutated[s_mutationIndex] = mutated[s_mutationIndex + 1];
		mutated[s_mutationIndex + 1] = tmp;
		break;
	}
	case MUTATE_DROP:
		mutated.erase( mutated.begin() + s_mutationIndex );
		break;
	case MUTATE_DUPLICATE:
		mutated.insert( mutated.begin() + s_mutationIndex, mutated[s_mutationIndex] );
		break;
	default:
		break;
	}
	*pResult = pGraph->Run( mutated.data(), (unsigned)mutated.size() );
	return pResult->valid;
}

//=============================================================================
// The two implementations under comparison
//=============================================================================

struct OracleCarry
{
	int numticks_last_frame;
	float host_remainder_last_frame;
	float last_frame_time;
};
static OracleCarry g_OracleCarry;

#include "hostframe_legacy_oracle.h"
#include "hostframe_render_legacy_oracle.h"
#include "../../engine/host_render_steps.h"
#include "../../engine/host_frame_phases.h"

// As engine/gl_screen.cpp exports them.
void Host_RunRenderStep( int iStep, HostRenderState_t &state )
{
	HostRender_RunStep( iStep, state );
}
const char *Host_GetRenderStepName( int iStep )
{
	return g_HostRenderSteps[iStep].name;
}

static HostFrameCarry_t g_GraphCarry;
static jobsystem::SerialFrameGraph *s_pGraph = NULL;
static CUtlVector<jobsystem::FramePhaseDesc> s_phases;
static bool s_jumpTargetsRestored = true;

static void LegacyFrame( const FrameScript &f )
{
	if ( setjmp( host_enddemo ) )
	{
		Record( "frame.exit enddemo" );
		return;
	}
	LegacyHostFrameBody( f.numticks, f.prevremainder, f.shouldrender, f.host_remainder );
}

static void GraphFrame( const FrameScript &f )
{
	if ( setjmp( host_enddemo ) )
	{
		Record( "frame.exit enddemo (unguarded)" );
		return;
	}
	jmp_buf harnessEndDemo, harnessAbortServer;
	memcpy( harnessEndDemo, host_enddemo, sizeof( jmp_buf ) );
	memcpy( harnessAbortServer, host_abortserver, sizeof( jmp_buf ) );

	HostFrameState_t state;
	state.numticks = f.numticks;
	state.prevremainder = f.prevremainder;
	state.host_remainder = f.host_remainder;
	state.shouldrender = f.shouldrender;
	state.threaded = IsEngineThreaded();
	state.checkheap = host_checkheap;
	state.pCarry = &g_GraphCarry;
	const HostFrameExit_t frameExit = HostFrame_RunGraph( s_pGraph, state, s_phases );

	// Every guard restored the caller's jump targets.
	if ( memcmp( harnessEndDemo, host_enddemo, sizeof( jmp_buf ) ) != 0 ||
	     memcmp( harnessAbortServer, host_abortserver, sizeof( jmp_buf ) ) != 0 )
	{
		s_jumpTargetsRestored = false;
	}

	switch ( frameExit )
	{
	case HOST_FRAME_COMPLETED:
		break;
	case HOST_FRAME_EXIT_ENDDEMO:
		Record( "frame.exit enddemo" );
		return;
	case HOST_FRAME_EXIT_ABORTSERVER:
		longjmp( host_abortserver, 1 );
	case HOST_FRAME_GRAPH_FAILED:
		Record( "frame.graph failed" );
		return;
	}
	Host_ShowIPCCallCount();
}

static void ResetWorld( const Scenario &sc )
{
	s_pScenario = &sc;
	s_pFrame = NULL;
	memset( &g_ServerGlobalVariables, 0, sizeof( g_ServerGlobalVariables ) );
	memset( &g_ClientGlobalVariables, 0, sizeof( g_ClientGlobalVariables ) );
	g_ClientGlobalVariables.frametime = 0.015f;
	cl.insimulation = false;
	cl.m_tickRemainder = 0;
	cl.frameTime = 0;
	cl.clientTick = 100;
	sv.m_nTickCount = 200;
	host_state.interval_per_tick = 0.015f;
	host_time = 0;
	host_frameticks = host_tickcount = host_currentframetick = host_framecount = 0;
	scr_nextdrawtick = sc.nextdrawtick;
	host_idealtime = 0;
	memset( host_jitterhistory, 0, sizeof( host_jitterhistory ) );
	host_jitterhistorypos = 0;
	s_netMultiplayer = false;
	g_bThreadedEngine = sc.threaded;
	g_ClientDLL = sc.hasClientDLL ? &s_clientDLL : NULL;
	hltv = sc.hltvAtStart ? &s_hltv : NULL;
	hltvtest = sc.hltvTest ? &s_hltvTest : NULL;
	s_clock = 1000.0;
	memset( &g_OracleCarry, 0, sizeof( g_OracleCarry ) );
	memset( &g_GraphCarry, 0, sizeof( g_GraphCarry ) );
}

enum Path
{
	PATH_LEGACY,
	PATH_GRAPH,
};

static void RunOneFrame( Path path, const FrameScript &f )
{
	if ( setjmp( host_abortserver ) )
	{
		Record( "frame.exit abortserver" );
		return;
	}
	if ( path == PATH_LEGACY )
		LegacyFrame( f );
	else
		GraphFrame( f );
}

static std::vector<std::string> RunScenario( const Scenario &sc, Path path )
{
	std::vector<std::string> trace;
	s_pTrace = &trace;
	ResetWorld( sc );
	for ( size_t i = 0; i < sc.frames.size(); i++ )
	{
		const FrameScript &f = sc.frames[i];
		s_pFrame = &f;
		s_injectCounter = 0;
		s_serverCalls = 0;
		host_frametime = f.frametime;
		scr_disabled_for_loading = f.disabledForLoading;
		scr_initialized = f.screenInitialized;
		con_initialized = f.consoleInitialized;
		cl_takesnapshot = f.takeSnapshot;
		mat_norendering.value = f.noRendering;
		g_pLauncherMgr = f.launcherMgr ? &s_launcherMgr : NULL;
		if ( f.nextDrawTickSet )
			scr_nextdrawtick = f.nextDrawTickSet;
		Record( "frame.begin %u numticks=%d", (unsigned)i, f.numticks );
		RunOneFrame( path, f );
		const OracleCarry carry =
		    path == PATH_LEGACY
		        ? g_OracleCarry
		        : OracleCarry{ g_GraphCarry.numticks_last_frame,
		              g_GraphCarry.host_remainder_last_frame, g_GraphCarry.last_frame_time };
		Record( "frame.end carry=%d/%.5f/%.5f", carry.numticks_last_frame,
		    carry.host_remainder_last_frame, carry.last_frame_time );
	}
	s_pTrace = NULL;
	s_pFrame = NULL;
	return trace;
}

// Index of the first differing entry, or -1.
static int FirstDivergence( const std::vector<std::string> &a, const std::vector<std::string> &b )
{
	const size_t n = a.size() < b.size() ? a.size() : b.size();
	for ( size_t i = 0; i < n; i++ )
	{
		if ( a[i] != b[i] )
			return (int)i;
	}
	return a.size() == b.size() ? -1 : (int)n;
}

//=============================================================================
// Tests
//=============================================================================

struct Coverage
{
	int threaded, unthreaded, dedicated, enddemo, abortserver, zeroTicks, multiTicks, hltvMidFrame,
	    asyncJoins;
	int renderStages, screenUpdates, vguiOnly, renderExits;
};

static void TestEquivalence( int nScenarios, Coverage &coverage )
{
	int nMismatch = 0;
	for ( int seed = 1; seed <= nScenarios; seed++ )
	{
		const Scenario sc = MakeScenario( (unsigned)seed );
		const std::vector<std::string> legacy = RunScenario( sc, PATH_LEGACY );
		const std::vector<std::string> graph = RunScenario( sc, PATH_GRAPH );
		const int divergence = FirstDivergence( legacy, graph );
		CHECK( divergence < 0 && !legacy.empty() );
		if ( divergence >= 0 )
		{
			if ( ++nMismatch <= 3 )
			{
				printf( "seed %d diverges at event %d:\n  legacy: %s\n  graph:  %s\n", seed,
				    divergence,
				    divergence < (int)legacy.size() ? legacy[divergence].c_str() : "<end>",
				    divergence < (int)graph.size() ? graph[divergence].c_str() : "<end>" );
			}
		}

		coverage.threaded += sc.threaded ? 1 : 0;
		coverage.unthreaded += sc.threaded ? 0 : 1;
		coverage.dedicated += sc.dedicated ? 1 : 0;
		for ( size_t i = 0; i < legacy.size(); i++ )
		{
			const std::string &event = legacy[i];
			coverage.renderStages += event.compare( 0, 21, "CheckSpecialCheatVars" ) == 0 ? 1 : 0;
			coverage.screenUpdates += event.compare( 0, 20, "materials.BeginFrame" ) == 0 ? 1 : 0;
			coverage.vguiOnly += event.compare( 0, 16, "V_RenderVGuiOnly" ) == 0 ? 1 : 0;
			if ( i > 0 && event.find( "longjmp" ) != std::string::npos &&
			     ( legacy[i - 1].compare( 0, 12, "V_RenderView" ) == 0 ||
			         legacy[i - 1].compare( 0, 26, "ClientDLL_FrameStageNotify" ) == 0 ||
			         legacy[i - 1].compare( 0, 14, "CL_DecayLights" ) == 0 ) )
			{
				coverage.renderExits++;
			}
			coverage.enddemo += event.compare( 0, 18, "frame.exit enddemo" ) == 0 ? 1 : 0;
			coverage.abortserver += event.compare( 0, 22, "frame.exit abortserver" ) == 0 ? 1 : 0;
			coverage.hltvMidFrame += event.compare( 0, 13, "hltv.RunFrame" ) == 0 ? 1 : 0;
			coverage.asyncJoins += event.compare( 0, 11, "ServerAsync" ) == 0 ? 1 : 0;
		}
		for ( const FrameScript &f : sc.frames )
		{
			coverage.zeroTicks += f.numticks == 0 ? 1 : 0;
			coverage.multiTicks += f.numticks > 1 ? 1 : 0;
		}
	}
	if ( nMismatch )
		printf( "%d of %d scenarios diverged\n", nMismatch, nScenarios );
	CHECK( s_jumpTargetsRestored );
}

static void TestCoverage( const Coverage &coverage )
{
	// The generated scenarios reached every behavior class being compared.
	printf(
	    "coverage: threaded %d unthreaded %d dedicated %d enddemo %d abortserver %d zero-tick %d "
	    "multi-tick %d hltv-frames %d async-joins %d\n",
	    coverage.threaded, coverage.unthreaded, coverage.dedicated, coverage.enddemo,
	    coverage.abortserver, coverage.zeroTicks, coverage.multiTicks, coverage.hltvMidFrame,
	    coverage.asyncJoins );
	printf( "render coverage: stages %d screen-updates %d vgui-only %d render-exits %d\n",
	    coverage.renderStages, coverage.screenUpdates, coverage.vguiOnly, coverage.renderExits );
	CHECK( coverage.renderStages > 0 );
	CHECK( coverage.screenUpdates > 0 );
	CHECK( coverage.screenUpdates < coverage.renderStages ); // some screen updates rejected
	CHECK( coverage.vguiOnly > 0 );
	CHECK( coverage.renderExits > 0 );
	CHECK( coverage.threaded > 0 );
	CHECK( coverage.unthreaded > 0 );
	CHECK( coverage.dedicated > 0 );
	CHECK( coverage.enddemo > 0 );
	CHECK( coverage.abortserver > 0 );
	CHECK( coverage.zeroTicks > 0 );
	CHECK( coverage.multiTicks > 0 );
	CHECK( coverage.hltvMidFrame > 0 );
	CHECK( coverage.asyncJoins > 0 );
}

// Mutated phase lists must be detected by the comparison.
static void TestSensitivity( int nScenarios )
{
	static const Mutation s_mutations[] = { MUTATE_SWAP, MUTATE_DROP, MUTATE_DUPLICATE };
	int nChecked = 0, nEquivalent = 0;
	for ( Mutation mutation : s_mutations )
	{
		// Every position of the longest frames (render steps included).
		for ( unsigned index = 0; index < 40; index++ )
		{
			s_mutation = mutation;
			s_mutationIndex = index;
			s_mutationApplied = false;
			s_mutationTouchesNoOp = false;
			bool bDetected = false;
			for ( int seed = 1; seed <= nScenarios && !bDetected; seed++ )
			{
				const Scenario sc = MakeScenario( (unsigned)seed );
				bDetected = FirstDivergence( RunScenario( sc, PATH_LEGACY ),
				                RunScenario( sc, PATH_GRAPH ) ) >= 0;
			}
			s_mutation = MUTATE_NONE;
			nEquivalent += s_mutationTouchesNoOp ? 1 : 0;
			if ( !s_mutationApplied )
				continue; // no frame that long, or only equivalent placements
			nChecked++;
			if ( !bDetected )
				printf( "undetected mutation %d at phase %u\n", (int)mutation, index );
			CHECK( bDetected );
		}
	}
	printf( "sensitivity: %d phase-list mutations checked (%d also had an equivalent placement "
	        "on the no-op step, run unmutated)\n",
	    nChecked, nEquivalent );
	CHECK( nChecked >= 110 );
}

//-----------------------------------------------------------------------------
// SerialFrameGraph facade contract
//-----------------------------------------------------------------------------

static std::vector<int> s_order;

static bool Append( void *, int arg )
{
	s_order.push_back( arg );
	return true;
}

static bool Stop( void *, int arg )
{
	s_order.push_back( arg );
	return false;
}

static void TestFacade()
{
	jobsystem::SerialFrameGraph graph;
	jobsystem::FramePhaseDesc phases[5];
	static const char *s_names[] = { "a", "b", "c", "d", "e" };
	for ( int i = 0; i < 5; i++ )
	{
		phases[i].name = s_names[i];
		phases[i].run = &Append;
		phases[i].context = NULL;
		phases[i].arg = i;
	}

	s_order.clear();
	jobsystem::SerialFrameRun run = graph.Run( phases, 5 );
	CHECK( run.valid && !run.stopped && run.executed == 5 );
	CHECK( s_order == std::vector<int>( { 0, 1, 2, 3, 4 } ) );
	CHECK( graph.SealCount() == 1 );

	// Same shape, new args: reused graph, args read per run.
	for ( int i = 0; i < 5; i++ )
		phases[i].arg = 10 + i;
	s_order.clear();
	run = graph.Run( phases, 5 );
	CHECK( run.valid && run.executed == 5 );
	CHECK( s_order == std::vector<int>( { 10, 11, 12, 13, 14 } ) );
	CHECK( graph.SealCount() == 1 );

	// Different shape rebuilds.
	s_order.clear();
	run = graph.Run( phases, 3 );
	CHECK( run.valid && run.executed == 3 && graph.SealCount() == 2 );

	// A phase that returns false cancels every later phase.
	phases[2].run = &Stop;
	s_order.clear();
	run = graph.Run( phases, 5 );
	CHECK( run.valid && run.stopped && run.executed == 3 );
	CHECK( s_order == std::vector<int>( { 10, 11, 12 } ) );
	CHECK( graph.SealCount() == 3 );
	phases[2].run = &Append;

	// Invalid input runs nothing.
	s_order.clear();
	phases[1].run = NULL;
	run = graph.Run( phases, 5 );
	CHECK( !run.valid && run.executed == 0 && s_order.empty() );
	phases[1].run = &Append;
	phases[3].name = "";
	run = graph.Run( phases, 5 );
	CHECK( !run.valid && run.executed == 0 && s_order.empty() );
	phases[3].name = s_names[3];
	run = graph.Run( NULL, 2 );
	CHECK( !run.valid && run.executed == 0 );

	// An empty frame is valid.
	run = graph.Run( NULL, 0 );
	CHECK( run.valid && run.executed == 0 );
}

// The kernel reuses one sealed graph per frame shape.
static void TestKernelGraphReuse()
{
	jobsystem::SerialFrameGraph graph;
	jobsystem::SerialFrameGraph *pSaved = s_pGraph;
	s_pGraph = &graph;
	Scenario sc = MakeScenario( 7 );
	sc.dedicated = false;
	sc.threaded = false;
	sc.frames.clear();
	for ( int i = 0; i < 6; i++ )
	{
		FrameScript f = MakeScenario( 3 ).frames[0];
		f.numticks = 1;
		f.shouldrender = true;
		f.inject = INJECT_NONE;
		sc.frames.push_back( f );
	}
	RunScenario( sc, PATH_GRAPH );
	CHECK( graph.SealCount() == 1 );
	sc.frames[3].numticks = 2;
	RunScenario( sc, PATH_GRAPH );
	CHECK( graph.SealCount() == 3 ); // 1 -> 2 ticks -> back to 1 tick
	s_pGraph = pSaved;
}

// Measurement, not a gate: per-frame cost of running a frame's phases through
// the serial graph versus calling the same functions in a loop. Phase counts
// match the kernel's unthreaded frames with 1 and 4 ticks.
static volatile int s_sink = 0;
static bool Nop( void *, int arg )
{
	s_sink = s_sink + arg;
	return true;
}

static void MeasureOverhead()
{
	static const char *s_names[] = { "p00", "p01", "p02", "p03", "p04", "p05", "p06", "p07", "p08",
	    "p09", "p10", "p11", "p12", "p13", "p14", "p15", "p16", "p17", "p18", "p19", "p20", "p21",
	    "p22", "p23", "p24", "p25", "p26", "p27", "p28", "p29", "p30", "p31" };
	static const unsigned s_counts[] = { 16, 31 };
	for ( unsigned count : s_counts )
	{
		std::vector<jobsystem::FramePhaseDesc> phases( count );
		for ( unsigned i = 0; i < count; i++ )
		{
			phases[i].name = s_names[i];
			phases[i].run = &Nop;
			phases[i].context = NULL;
			phases[i].arg = (int)i;
		}
		jobsystem::SerialFrameGraph graph;
		const int kFrames = 20000;
		double best[2] = { 1e30, 1e30 };
		for ( int round = 0; round < 5; round++ )
		{
			auto start = std::chrono::steady_clock::now();
			for ( int frame = 0; frame < kFrames; frame++ )
				graph.Run( phases.data(), count );
			auto mid = std::chrono::steady_clock::now();
			for ( int frame = 0; frame < kFrames; frame++ )
			{
				for ( unsigned i = 0; i < count; i++ )
				{
					if ( !phases[i].run( phases[i].context, phases[i].arg ) )
						break;
				}
			}
			auto end = std::chrono::steady_clock::now();
			const double graphNs =
			    std::chrono::duration<double, std::nano>( mid - start ).count() / kFrames;
			const double loopNs =
			    std::chrono::duration<double, std::nano>( end - mid ).count() / kFrames;
			best[0] = graphNs < best[0] ? graphNs : best[0];
			best[1] = loopNs < best[1] ? loopNs : best[1];
		}
		printf( "overhead: %u phases: graph %.0f ns/frame, direct loop %.0f ns/frame (%.1f "
		        "ns/phase added)\n",
		    count, best[0], best[1], ( best[0] - best[1] ) / count );
	}
}

int main( int argc, char **argv )
{
	int nScenarios = 4000;
	if ( argc > 2 && strcmp( argv[1], "--scenarios" ) == 0 )
		nScenarios = atoi( argv[2] );

	jobsystem::SerialFrameGraph graph;
	s_pGraph = &graph;

	TestFacade();

	Coverage coverage;
	memset( &coverage, 0, sizeof( coverage ) );
	TestEquivalence( nScenarios, coverage );
	TestCoverage( coverage );
	TestSensitivity( 400 );
	TestKernelGraphReuse();
	MeasureOverhead();

	return testing::ReportConformance( s_checks, s_failures );
}
