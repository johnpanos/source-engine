//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The host frame's render stage as ordered steps (RFC 0003 R10).
//
//          _Host_RunFrame_Render (with Host_UpdateScreen and SCR_UpdateScreen
//          inside it) was one hand-ordered body. Its statement blocks are the
//          steps below, unchanged and in their original order. Three
//          sequencers run the same steps: _Host_RunFrame_Render (every step,
//          the host_frame_graph 0 path), SCR_UpdateScreen (the screen steps,
//          for its other callers), and the host frame graph, which runs each
//          step as its own node. The early returns of SCR_UpdateScreen are
//          the admission step's result: the remaining screen steps run only
//          when it admitted the frame, evaluated when they run.
//
//          Profiling scopes (VPROF/tmZone) that spanned several blocks now
//          span one step each.
//
//          This header is included by engine/host_render.cpp after the
//          engine declarations it uses, and by the host frame oracle test
//          with recording stand-ins of the same names.
//
//=============================================================================//

#ifndef HOST_RENDER_STEPS_H
#define HOST_RENDER_STEPS_H

#include "host_frame_graph.h" // HostRenderState_t, HostRenderStep_t

//-----------------------------------------------------------------------------
// Render stage prologue (_Host_RunFrame_Render)
//-----------------------------------------------------------------------------

static void HostRender_Begin( HostRenderState_t &r )
{
	VPROF( "_Host_RunFrame_Render" );
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame_Render" );
	Host_TraceFrameEvent( "Render", 0 );

	CheckSpecialCheatVars();

	r.nOrgNoRendering = mat_norendering.GetInt();

	if ( cl_takesnapshot )
	{
		// turn off no-rendering mode, if taking screenshot
		mat_norendering.SetValue( 0 );
	}

	// update video if not running in background
	g_HostTimes.StartFrameSegment( FRAME_SEGMENT_RENDER );

	CL_LatchInterpolationAmount();
}

// Host_UpdateScreen's own prologue.
static void HostRender_UpdateScreenBegin( HostRenderState_t & )
{
	VPROF( "_Host_RunFrame_Render - UpdateScreen" );
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame_Render - UpdateScreen" );

#ifdef _DEBUG
	if( r_ForceRestore.GetInt() )
	{
		ForceMatSysRestore();
		r_ForceRestore.SetValue(0);
	}
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
// Screen update (SCR_UpdateScreen)
//-----------------------------------------------------------------------------

// The early-out checks. Returns whether the rest of the screen update runs.
static bool HostRender_ScreenAdmitChecks()
{
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s", "SCR_UpdateScreen" );

	R_StudioCheckReinitLightingCache();

	// Always force the Gamma Table to be rebuilt. Otherwise,
	// we'll load textures with an all white gamma lookup table.
	V_CheckGamma();

	// This is a HACK to let things settle for a bit on level start
	// NOTE: If you remove scr_nextdrawtick, remove it from enginetool.cpp too
	if ( scr_nextdrawtick != 0 )
	{
		if ( host_tickcount < scr_nextdrawtick )
			return false;

		scr_nextdrawtick = 0;
	}

	if ( scr_disabled_for_loading )
	{
		if ( !Host_IsSinglePlayerGame() )
		{
			V_RenderVGuiOnly();
		}
		return false;
	}

	if ( !scr_initialized || !con_initialized )
	{
		// not initialized yet
		return false;
	}

	SCR_ShowVCRPlaybackAmount();

	// Let demo system overwrite view origin/angles during playback
	if ( demoplayer->IsPlayingBack() )
	{
		demoplayer->InterpolateViewpoint();
	}

	if ( !VideoMode_UpdateWindowSize() )
		return false;
	return true;
}

static void HostRender_ScreenAdmit( HostRenderState_t &r )
{
	r.bScreenAdmitted = HostRender_ScreenAdmitChecks();
}

static void HostRender_ScreenBeginFrame( HostRenderState_t & )
{
	materials->BeginFrame( host_frametime );
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "EngineVGui_Simulate" );
		EngineVGui()->Simulate();
	}
}

static void HostRender_ClientRenderStart( HostRenderState_t & )
{
	ClientDLL_FrameStageNotify( FRAME_RENDER_START );
}

static void HostRender_EngineFrameBegin( HostRenderState_t & )
{
	// Simulation meant to occur before any views are rendered
	// This needs to happen before the client DLL is called because the client DLL depends on
	// some of the setup in FRAME_RENDER_START.
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "FrameBegin" );

		g_EngineRenderer->FrameBegin();
		toolframework->RenderFrameBegin();
	}

	cl.UpdateAreaBits_BackwardsCompatible();

	Shader_BeginRendering();
}

static void HostRender_ViewRender( HostRenderState_t & )
{
	// Draw world, etc.
	V_RenderView();
}

static void HostRender_Present( HostRenderState_t & )
{
	CL_TakeSnapshotAndSwap();

#if defined( REPLAY_ENABLED )
	if ( g_pReplay )
	{
		g_pReplay->CL_Render();
	}
#endif
}

static void HostRender_ClientRenderEnd( HostRenderState_t & )
{
	ClientDLL_FrameStageNotify( FRAME_RENDER_END );
}

static void HostRender_EngineFrameEnd( HostRenderState_t & )
{
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "FrameEnd" );

		toolframework->RenderFrameEnd();

		g_EngineRenderer->FrameEnd();
	}
}

static void HostRender_DynamicModels( HostRenderState_t & )
{
	// moved dynamic model update here because this takes the materials lock
	// and materials->EndFrame() is where we will synchronize anyway.
	// Moved here to leave as much of the frame as possible to overlap threads in the case
	// where we actually have models to load here
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "modelloader->UpdateDynamicModels" );
		VPROF( "UpdateDynamicModels" );
		CMDLCacheCriticalSection critsec( g_pMDLCache );
		modelloader->UpdateDynamicModels();
	}
}

static void HostRender_MaterialsEndFrame( HostRenderState_t & )
{
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "materials_EndFrame" );

		materials->EndFrame();
	}
}

//-----------------------------------------------------------------------------
// Render stage epilogue (_Host_RunFrame_Render)
//-----------------------------------------------------------------------------

static void HostRender_DecayLights( HostRenderState_t & )
{
	VPROF( "_Host_RunFrame_Render - CL_DecayLights" );
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame_Render - CL_DecayLights" );
	CL_DecayLights ();
}

static void HostRender_End( HostRenderState_t &r )
{
	g_HostTimes.EndFrameSegment( FRAME_SEGMENT_RENDER );

	saverestore->OnFrameRendered();

#ifdef USE_SDL
	if ( g_pLauncherMgr )
	{
		g_pLauncherMgr->OnFrameRendered();
	}
#endif

	mat_norendering.SetValue( r.nOrgNoRendering );
}

//-----------------------------------------------------------------------------
// Step table, in legacy order
//-----------------------------------------------------------------------------

static const HostRenderStep_t g_HostRenderSteps[HOST_RENDER_STEP_COUNT] = {
	{ "RenderBegin", HostRender_Begin, false },
	{ "UpdateScreenBegin", HostRender_UpdateScreenBegin, false },
	{ "ScreenAdmit", HostRender_ScreenAdmit, false },
	{ "ScreenBeginFrame", HostRender_ScreenBeginFrame, true },
	{ "ClientRenderStart", HostRender_ClientRenderStart, true },
	{ "EngineFrameBegin", HostRender_EngineFrameBegin, true },
	{ "ViewRender", HostRender_ViewRender, true },
	{ "Present", HostRender_Present, true },
	{ "ClientRenderEnd", HostRender_ClientRenderEnd, true },
	{ "EngineFrameEnd", HostRender_EngineFrameEnd, true },
	{ "DynamicModels", HostRender_DynamicModels, true },
	{ "MaterialsEndFrame", HostRender_MaterialsEndFrame, true },
	{ "DecayLights", HostRender_DecayLights, false },
	{ "RenderEnd", HostRender_End, false },
};

// Runs one step; a screen step runs only if the screen update was admitted.
static void HostRender_RunStep( int iStep, HostRenderState_t &r )
{
	const HostRenderStep_t &step = g_HostRenderSteps[iStep];
	const bool bRun = !step.bScreenStep || r.bScreenAdmitted;
	Host_TraceFrameEvent( step.name, bRun ? 1 : 0 );
	if ( bRun )
		step.run( r );
}

#endif // HOST_RENDER_STEPS_H
