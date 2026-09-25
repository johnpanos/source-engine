//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Host frame as an ordered serial graph (RFC 0003 R10).
//
//          _Host_RunFrame's hand-ordered body after admission (time
//          accumulation and tick count) is expressed as one legacy node per
//          existing phase on a single sequence lane of a jobsystem frame
//          graph, run serially on the host thread. Each phase body is the
//          original statement block, unchanged; guards that the legacy code
//          evaluated at call time are still evaluated at call time inside the
//          phase. Only the frame shape (tick counts, threaded/unthreaded path,
//          render/dedicated branch) is fixed when the graph is built, and the
//          legacy code fixed each of those before running any phase of it.
//
//          Error exits: Host_EndGame and Host_AbortServer leave a frame with
//          longjmp. A guard around each phase traps them so they never unwind
//          the graph executor, cancels the remaining phases, and reports the
//          exit to _Host_RunFrame, which completes it from its own frame: it
//          returns for host_enddemo and re-issues the longjmp for
//          host_abortserver, as the legacy frame did.
//
//          This header is included by engine/host_frame_graph.cpp after
//          the engine declarations it uses, and by the host frame oracle test
//          with recording stand-ins of the same names.
//
//=============================================================================//

#ifndef HOST_FRAME_PHASES_H
#define HOST_FRAME_PHASES_H

#include <setjmp.h>
#include <string.h>

#include "jobsystem/serial_frame_graph.h"
#include "host_frame_graph.h"

// The includer provides RunSerialFrameGraph (vstdlib/jobgraph_frame.h in the
// engine), CUtlVector and g_HostTimes (the frame-segment timer). Host services
// the phases call that no engine header declares follow; engine/host.cpp
// defines them and the oracle test defines recording stand-ins.
void _Host_RunFrame_Input( float accumulated_extra_samples, bool bFinalTick );
void _Host_RunFrame_Server( bool finaltick );
void _Host_RunFrame_Server_Async( int numticks );
void _Host_RunFrame_Client( bool framefinished );
void _Host_RunFrame_Render();
void _Host_RunFrame_Sound();
void Host_SetClientInSimulation( bool bInSimulation );
void Host_Speeds();
void Host_PostFrameRate( float frameTime );
void Host_CheckDumpMemoryStats( void );
#ifndef SWDS
void CL_ApplyAddAngle();
#endif
extern int host_currentframetick;
extern double host_idealtime;
extern float host_jitterhistory[128];
extern unsigned int host_jitterhistorypos;

//-----------------------------------------------------------------------------
// Phases. Each body is the legacy statement block for that phase.
//-----------------------------------------------------------------------------

static void HostPhase_CmdExecute( HostFrameState_t &, int )
{
	g_HostTimes.StartFrameSegment( FRAME_SEGMENT_CMD_EXECUTE );

	// process console commands
	Cbuf_Execute();

	// initialize networking for dedicated server after commandline & autoexec.cfg have been parsed
	if ( NET_IsDedicated() && !NET_IsMultiplayer() )
		NET_SetMutiplayer( true );

	g_HostTimes.EndFrameSegment( FRAME_SEGMENT_CMD_EXECUTE );
}

static void HostPhase_FrameSetup( HostFrameState_t &s, int )
{
	g_ServerGlobalVariables.interpolation_amount = 0.0f;
#ifndef SWDS
	g_ClientGlobalVariables.interpolation_amount = 0.0f;

	cl.insimulation = true;
#endif

	host_frameticks = s.numticks;
	host_currentframetick = 0;

#if !defined( SWDS )
	// This is to make the tool do both sim + rendering on the initial frame
	// cl.IsActive changes in the loop below, as does scr_nextdrawtick
	// We're just caching off the state here so that we have a consistent return value
	// for enginetool->IsInGame the entire frame
	g_pEngineToolInternal->SetIsInGame( cl.IsActive() && ( scr_nextdrawtick == 0 ) );
#endif
	s.pGameJob = NULL;
}

//-------------------------------------
// Unthreaded path (dedicated servers and unthreaded listen servers)
//-------------------------------------

static void HostPhase_SimSetup( HostFrameState_t &s, int )
{
#ifndef SWDS
	if ( g_ClientDLL )
	{
		g_ClientDLL->IN_SetSampleTime( host_frametime );
	}
	g_ClientGlobalVariables.simTicksThisFrame = 1;
#endif
	cl.m_tickRemainder = s.host_remainder;
	g_ServerGlobalVariables.simTicksThisFrame = 1;
	cl.SetFrameTime( host_frametime );
}

static void HostPhase_TickBegin( HostFrameState_t &s, int tick )
{
	// Emit an ETW event every simulation frame.
	ETWSimFrameMark( sv.IsDedicated() );

	double now = Plat_FloatTime();
	float jitter = now - host_idealtime;

	// Track jitter (delta between ideal time and actual tick execution time)
	host_jitterhistory[host_jitterhistorypos] = jitter;
	host_jitterhistorypos = ( host_jitterhistorypos + 1 ) % ARRAYSIZE( host_jitterhistory );

	// Very slowly decay "ideal" towards current wall clock unless delta is large
	if ( fabs( jitter ) > 1.0f )
	{
		host_idealtime = now;
	}
	else
	{
		host_idealtime = 0.99 * host_idealtime + 0.01 * now;
	}

	// process any asynchronous network traffic (TCP), set net_time
	NET_RunFrame( now );

	// Only send updates on final tick so we don't re-encode network data multiple times per frame unnecessarily
	s.bFinalTick = ( tick == ( s.numticks - 1 ) );

	// initialize networking for dedicated server after commandline & autoexec.cfg have been parsed
	if ( NET_IsDedicated() && !NET_IsMultiplayer() )
		NET_SetMutiplayer( true );

	g_ServerGlobalVariables.tickcount = sv.m_nTickCount;
	// NOTE:  Do we want do this at start or end of this loop?
	++host_tickcount;
	++host_currentframetick;
#ifndef SWDS
	g_ClientGlobalVariables.tickcount = cl.GetClientTickCount();

	// Make sure state is correct
	CL_CheckClientState();
#endif
}

static void HostPhase_TickInput( HostFrameState_t &s, int )
{
	//-------------------
	// input processing
	//-------------------
	_Host_RunFrame_Input( s.prevremainder, s.bFinalTick );
	s.prevremainder = 0;
}

static void HostPhase_TickServer( HostFrameState_t &s, int )
{
	//-------------------
	//
	// server operations
	//
	//-------------------

	_Host_RunFrame_Server( s.bFinalTick );

	// Additional networking ops for SPLITPACKET stuff (99.9% of the time this will be an empty list of work)
	NET_SendQueuedPackets();
}

static void HostPhase_TickClient( HostFrameState_t &s, int )
{
	//-------------------
	//
	// client operations
	//
	//-------------------
#ifndef SWDS
	if ( !sv.IsDedicated() )
	{
		_Host_RunFrame_Client( s.bFinalTick );
	}

	toolframework->Think( s.bFinalTick );
#endif
}

static void HostPhase_TickEnd( HostFrameState_t &, int )
{
	host_idealtime += host_state.interval_per_tick;
}

static void HostPhase_HLTV( HostFrameState_t &, int )
{
	// run HLTV if active
	if ( hltv )
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "hltv->RunFrame()" );
		hltv->RunFrame();
	}

	if ( hltvtest )
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "hltvtest->RunFrame()" );
		hltvtest->RunFrame();
	}

#if defined( REPLAY_ENABLED )
	// run replay if active
	if ( replay )
	{
		replay->RunFrame();
	}

	// Update server-side replay history manager
	if ( sv.IsDedicated() && g_pServerReplayContext && g_pServerReplayContext->IsInitialized() )
	{
		g_pServerReplayContext->Think();
	}
#endif
}

static void HostPhase_TimedemoClient( HostFrameState_t &s, int )
{
#ifndef SWDS
	// This is a hack to let timedemo pull messages from the queue faster than every 15 msec
	// Also when demoplayer is skipping packets to a certain tick we should process the queue
	// as quickly as we can.
	if ( s.numticks == 0 && ( demoplayer->IsPlayingTimeDemo() || demoplayer->IsSkipping() ) )
	{
		_Host_RunFrame_Client( true );
	}
#endif
}

static void HostPhase_ClientInterpolation( HostFrameState_t &, int )
{
#ifndef SWDS
	if ( !sv.IsDedicated() )
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "Host_SetClientInSimulation" );

		// This causes cl.gettime() to return the true clock being used for rendering (tickcount * rate + remainder)
		Host_SetClientInSimulation( false );
		// Now allow for interpolation on client
		g_ClientGlobalVariables.interpolation_amount =
		    ( cl.m_tickRemainder / host_state.interval_per_tick );

#if defined( REPLAY_ENABLED )
		// Update client-side replay history manager - called here since interpolation_amount is set
		if ( g_pClientReplayContext && g_pClientReplayContext->IsInitialized() )
		{
			g_pClientReplayContext->Think();
		}
#endif

		//-------------------
		// Run prediction if it hasn't been run yet
		//-------------------
		// If we haven't predicted/simulated the player (multiplayer with prediction enabled and
		//  not a listen server with zero frame lag, then go ahead and predict now
		CL_RunPrediction( PREDICTION_NORMAL );

		CL_ApplyAddAngle();

		// The mouse is always simulated for the current frame's time
		// This makes updates smooth in every case
		// continuous controllers affecting the view are also simulated this way
		// but they have a cap applied by IN_SetSampleTime() so they are not also
		// simulated during input gathering
		CL_ExtraMouseUpdate( g_ClientGlobalVariables.frametime );
	}
#endif
#if defined( REPLAY_ENABLED )
	// Let the replay system think
	if ( g_pReplay )
	{
		g_pReplay->Think();
	}
#endif
}

//-------------------------------------
// Threaded listen-server path
//-------------------------------------
#ifndef SWDS

static void HostPhase_ThreadedSetup( HostFrameState_t &s, int )
{
	HostFrameCarry_t &carry = *s.pCarry;

	s.clientticks = carry.numticks_last_frame;
	cl.m_tickRemainder = carry.host_remainder_last_frame;
	cl.SetFrameTime( carry.last_frame_time );
	if ( g_ClientDLL )
	{
		g_ClientDLL->IN_SetSampleTime( carry.last_frame_time );
	}

	carry.last_frame_time = host_frametime;

	s.serverticks = s.numticks;
	g_ClientGlobalVariables.simTicksThisFrame = s.clientticks;
	g_ServerGlobalVariables.simTicksThisFrame = s.serverticks;
	g_ServerGlobalVariables.tickcount = sv.m_nTickCount;
}

static void HostPhase_ThreadedClientTick( HostFrameState_t &s, int tick )
{
	// process any asynchronous network traffic (TCP), set net_time
	NET_RunFrame( Plat_FloatTime() );

	// Only send updates on final tick so we don't re-encode network data multiple times per frame unnecessarily
	bool bFinalTick = ( tick == ( s.clientticks - 1 ) );

	// initialize networking for dedicated server after commandline & autoexec.cfg have been parsed
	if ( NET_IsDedicated() && !NET_IsMultiplayer() )
		NET_SetMutiplayer( true );
	g_ClientGlobalVariables.tickcount = cl.GetClientTickCount();

	// Make sure state is correct
	CL_CheckClientState();
	// Additional networking ops for SPLITPACKET stuff (99.9% of the time this will be an empty list of work)
	NET_SendQueuedPackets();
	//-------------------
	//
	// client operations
	//
	//-------------------
	if ( !sv.IsDedicated() )
	{
		_Host_RunFrame_Client( bFinalTick );
	}
	toolframework->Think( bFinalTick );
}

static void HostPhase_ThreadedTimedemo( HostFrameState_t &s, int )
{
	// This is a hack to let timedemo pull messages from the queue faster than every 15 msec
	// Also when demoplayer is skipping packets to a certain tick we should process the queue
	// as quickly as we can.
	if ( s.clientticks == 0 && ( demoplayer->IsPlayingTimeDemo() || demoplayer->IsSkipping() ) )
	{
		_Host_RunFrame_Client( true );
	}
}

static void HostPhase_ThreadedPrediction( HostFrameState_t &s, int )
{
	// This causes cl.gettime() to return the true clock being used for rendering (tickcount * rate + remainder)
	Host_SetClientInSimulation( false );
	// Now allow for interpolation on client
	g_ClientGlobalVariables.interpolation_amount =
	    ( cl.m_tickRemainder / host_state.interval_per_tick );

	//-------------------
	// Run prediction if it hasn't been run yet
	//-------------------
	// If we haven't predicted/simulated the player (multiplayer with prediction enabled and
	//  not a listen server with zero frame lag, then go ahead and predict now
	CL_RunPrediction( PREDICTION_NORMAL );

	CL_ApplyAddAngle();

	Host_SetClientInSimulation( true );

	// THREADED: Run Input
	// -------------------
	s.saveTick = g_ClientGlobalVariables.tickcount;
}

static void HostPhase_ThreadedInputTick( HostFrameState_t &s, int tick )
{
	// NOTE:  Do we want do this at start or end of this loop?
	++host_tickcount;
	++host_currentframetick;
	g_ClientGlobalVariables.tickcount = host_tickcount;
	bool bFinalTick = tick == ( s.serverticks - 1 ) ? true : false;
	_Host_RunFrame_Input( s.prevremainder, bFinalTick );
	s.prevremainder = 0;
	// process any asynchronous network traffic (TCP), set net_time
	NET_RunFrame( Plat_FloatTime() );
}

static void HostPhase_ThreadedInputEnd( HostFrameState_t &s, int )
{
	HostFrameCarry_t &carry = *s.pCarry;

	Host_SetClientInSimulation( false );

	// The mouse is always simulated for the current frame's time
	// This makes updates smooth in every case
	// continuous controllers affecting the view are also simulated this way
	// but they have a cap applied by IN_SetSampleTime() so they are not also
	// simulated during input gathering
	CL_ExtraMouseUpdate( g_ClientGlobalVariables.frametime );

	g_ClientGlobalVariables.tickcount = s.saveTick;
	carry.numticks_last_frame = s.numticks;
	carry.host_remainder_last_frame = s.host_remainder;
}

static void HostPhase_ThreadedServerSubmit( HostFrameState_t &s, int )
{
	// THREADED: Run Server
	// -------------------
	// set net_time once before running the server
	NET_SetTime( Plat_FloatTime() );
	s.pGameJob = new CFunctorJob( CreateFunctor( _Host_RunFrame_Server_Async, s.serverticks ) );

	g_pThreadPool->AddJob( s.pGameJob );
}

#endif // !SWDS

//-------------------------------------
// Common tail
//-------------------------------------

static void HostPhase_Log( HostFrameState_t &, int )
{
	g_Log.RunFrame();
}

static void HostPhase_Render( HostFrameState_t &, int )
{
	//-------------------
	// rendering
	//-------------------
	_Host_RunFrame_Render();
}

static void HostPhase_Sound( HostFrameState_t &, int )
{
	//-------------------
	// sound
	//-------------------
	_Host_RunFrame_Sound();

#ifndef DEDICATED
	if ( g_bVCRSingleStep )
	{
		VCR_EnterPausedState();
	}
#endif
}

static void HostPhase_DynamicModels( HostFrameState_t &, int )
{
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "modelloader->UpdateDynamicModels" );
	VPROF( "UpdateDynamicModels" );
	CMDLCacheCriticalSection critsec( g_pMDLCache );
	modelloader->UpdateDynamicModels();
}

static void HostPhase_ClientDLLUpdate( HostFrameState_t &, int )
{
	//-------------------
	// simulation
	//-------------------
	g_HostTimes.MarkSwapTime();
#ifndef SWDS
	if ( !sv.IsDedicated() )
	{
		VPROF( "_Host_RunFrame - ClientDLL_Update" );
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame - ClientDLL_Update" );

		// Client-side simulation
		g_HostTimes.StartFrameSegment( FRAME_SEGMENT_CLDLL );

		ClientDLL_Update();

		g_HostTimes.EndFrameSegment( FRAME_SEGMENT_CLDLL );
	}
#endif
}

static void HostPhase_AsyncServerJoin( HostFrameState_t &s, int )
{
	CJob *pGameJob = s.pGameJob;
	if ( pGameJob )
	{
		{
			VPROF_BUDGET( "WaitForAsyncServer", "AsyncServer" );
			if ( Host_IsSinglePlayerGame() )
			{
				// Run the server frame here if no worker has started it, otherwise
				// wait for it. The wait may run only this job itself, never other
				// queued work (IThreadPool::YieldWait), so queued AI operations
				// cannot execute at this point of the host frame.
				pGameJob->ExecuteAndRelease();
			}
			else
			{
				pGameJob->WaitForFinishAndRelease();
			}
		}
		s.pGameJob = NULL;
		SV_FrameExecuteThreadDeferred();
	}
}

static void HostPhase_FrameEnd( HostFrameState_t &s, int )
{
	//-------------------
	// time
	//-------------------

	Host_Speeds();

	Host_UpdateMapList();

	host_framecount++;
#if !defined( SWDS )
	if ( !demoplayer->IsPlaybackPaused() )
#endif
	{
		host_time = host_tickcount * host_state.interval_per_tick + cl.m_tickRemainder;
	}

	Host_PostFrameRate( host_frametime );

	if ( s.checkheap )
	{
#ifdef _WIN32
		if ( _heapchk() != _HEAPOK )
		{
			Sys_Error( "_Host_RunFrame (bottom):  _heapchk() != _HEAPOK\n" );
		}
#endif
	}

	Host_CheckDumpMemoryStats();

	GetTestScriptMgr()->CheckPoint( "frame_end" );
}

//-----------------------------------------------------------------------------
// Error-exit guard and graph construction
//-----------------------------------------------------------------------------

typedef void ( *HostPhaseFn_t )( HostFrameState_t &, int );

// Runs one phase with host_enddemo/host_abortserver pointing here, so a
// longjmp from the phase unwinds only legacy frames above this guard. The
// graph executor below it is never unwound. The outer jump targets are
// restored before returning either way.
static bool HostFrame_GuardedPhase( HostPhaseFn_t fn, HostFrameState_t &s, int arg )
{
	jmp_buf outerEndDemo;
	jmp_buf outerAbortServer;
	memcpy( outerEndDemo, host_enddemo, sizeof( jmp_buf ) );
	memcpy( outerAbortServer, host_abortserver, sizeof( jmp_buf ) );

	if ( setjmp( host_enddemo ) )
	{
		memcpy( host_enddemo, outerEndDemo, sizeof( jmp_buf ) );
		memcpy( host_abortserver, outerAbortServer, sizeof( jmp_buf ) );
		s.exit = HOST_FRAME_EXIT_ENDDEMO;
		return false;
	}
	if ( setjmp( host_abortserver ) )
	{
		memcpy( host_enddemo, outerEndDemo, sizeof( jmp_buf ) );
		memcpy( host_abortserver, outerAbortServer, sizeof( jmp_buf ) );
		s.exit = HOST_FRAME_EXIT_ABORTSERVER;
		return false;
	}

	fn( s, arg );

	memcpy( host_enddemo, outerEndDemo, sizeof( jmp_buf ) );
	memcpy( host_abortserver, outerAbortServer, sizeof( jmp_buf ) );
	return true;
}

template <HostPhaseFn_t FN> static bool HostFrame_RunPhase( void *pState, int arg )
{
	return HostFrame_GuardedPhase( FN, *static_cast<HostFrameState_t *>( pState ), arg );
}

static void HostFrame_AddPhase( CUtlVector<jobsystem::FramePhaseDesc> &phases, const char *pszName,
    bool ( *run )( void *, int ), HostFrameState_t &s, int arg = 0 )
{
	jobsystem::FramePhaseDesc phase;
	phase.name = pszName;
	phase.run = run;
	phase.context = &s;
	phase.arg = arg;
	phases.AddToTail( phase );
}

#define HOST_FRAME_PHASE( phases, name, s, arg )                                                   \
	HostFrame_AddPhase( phases, #name, &HostFrame_RunPhase<&HostPhase_##name>, s, arg )

// Emits the frame's phases in the legacy call order.
static void HostFrame_BuildPhases(
    HostFrameState_t &s, CUtlVector<jobsystem::FramePhaseDesc> &phases )
{
	phases.RemoveAll();

	HOST_FRAME_PHASE( phases, CmdExecute, s, 0 );
	HOST_FRAME_PHASE( phases, FrameSetup, s, 0 );

	if ( !s.threaded )
	{
		HOST_FRAME_PHASE( phases, SimSetup, s, 0 );
		for ( int tick = 0; tick < s.numticks; tick++ )
		{
			HOST_FRAME_PHASE( phases, TickBegin, s, tick );
			HOST_FRAME_PHASE( phases, TickInput, s, tick );
			HOST_FRAME_PHASE( phases, TickServer, s, tick );
			HOST_FRAME_PHASE( phases, TickClient, s, tick );
			HOST_FRAME_PHASE( phases, TickEnd, s, tick );
		}
		HOST_FRAME_PHASE( phases, HLTV, s, 0 );
		HOST_FRAME_PHASE( phases, TimedemoClient, s, 0 );
		HOST_FRAME_PHASE( phases, ClientInterpolation, s, 0 );
	}
#ifndef SWDS
	else
	{
		HOST_FRAME_PHASE( phases, ThreadedSetup, s, 0 );
		for ( int tick = 0; tick < s.pCarry->numticks_last_frame; tick++ )
		{
			HOST_FRAME_PHASE( phases, ThreadedClientTick, s, tick );
		}
		HOST_FRAME_PHASE( phases, ThreadedTimedemo, s, 0 );
		HOST_FRAME_PHASE( phases, ThreadedPrediction, s, 0 );
		for ( int tick = 0; tick < s.numticks; tick++ )
		{
			HOST_FRAME_PHASE( phases, ThreadedInputTick, s, tick );
		}
		HOST_FRAME_PHASE( phases, ThreadedInputEnd, s, 0 );
		HOST_FRAME_PHASE( phases, ThreadedServerSubmit, s, 0 );
	}
#endif

	HOST_FRAME_PHASE( phases, Log, s, 0 );
	if ( s.shouldrender )
	{
		HOST_FRAME_PHASE( phases, Render, s, 0 );
		HOST_FRAME_PHASE( phases, Sound, s, 0 );
	}
	else
	{
		HOST_FRAME_PHASE( phases, DynamicModels, s, 0 );
	}
	HOST_FRAME_PHASE( phases, ClientDLLUpdate, s, 0 );
	HOST_FRAME_PHASE( phases, AsyncServerJoin, s, 0 );
	HOST_FRAME_PHASE( phases, FrameEnd, s, 0 );
}

#undef HOST_FRAME_PHASE

// Runs the post-admission part of one host frame through the serial graph.
// The caller completes a reported error exit (see the file comment).
static HostFrameExit_t HostFrame_RunGraph( jobsystem::SerialFrameGraph *pGraph, HostFrameState_t &s,
    CUtlVector<jobsystem::FramePhaseDesc> &phases )
{
	s.pGameJob = NULL;
	s.clientticks = 0;
	s.serverticks = 0;
	s.saveTick = 0;
	s.bFinalTick = false;
	s.exit = HOST_FRAME_COMPLETED;

	HostFrame_BuildPhases( s, phases );

	jobsystem::SerialFrameRun run;
	if ( !RunSerialFrameGraph( pGraph, phases.Base(), phases.Count(), &run ) )
		return HOST_FRAME_GRAPH_FAILED;
	return s.exit;
}

#endif // HOST_FRAME_PHASES_H
