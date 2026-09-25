//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the host frame graph test: the legacy render stage,
//          extracted mechanically from 504ee284: _Host_RunFrame_Render and
//          Host_UpdateScreen (engine/host.cpp lines 2432-2477 and 1906-1921)
//          and SCR_UpdateScreen (engine/gl_screen.cpp lines 212-317). No
//          substitutions. Do not edit the bodies; regenerate them from the
//          source revision.
//
//=============================================================================//

// clang-format off
void Host_UpdateScreen( void )
{
#ifndef SWDS 

#ifdef _DEBUG
	if( r_ForceRestore.GetInt() )
	{
		ForceMatSysRestore();
		r_ForceRestore.SetValue(0);
	}
#endif // _DEBUG

	// Refresh the screen
	SCR_UpdateScreen ();
#endif
}

void SCR_UpdateScreen( void )
{
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s", __FUNCTION__ );

	R_StudioCheckReinitLightingCache();
	
	// Always force the Gamma Table to be rebuilt. Otherwise,
	// we'll load textures with an all white gamma lookup table.
	V_CheckGamma();

	// This is a HACK to let things settle for a bit on level start
	// NOTE: If you remove scr_nextdrawtick, remove it from enginetool.cpp too
	if ( scr_nextdrawtick != 0 )
	{
		if ( host_tickcount < scr_nextdrawtick )
			return;

		scr_nextdrawtick = 0;
	}

	if ( scr_disabled_for_loading )
	{
		if ( !Host_IsSinglePlayerGame() )
		{
			V_RenderVGuiOnly();
		}
		return;
	}

	if ( !scr_initialized || !con_initialized )
	{
		// not initialized yet
		return;				
	}

	SCR_ShowVCRPlaybackAmount();

	// Let demo system overwrite view origin/angles during playback
	if ( demoplayer->IsPlayingBack() )
	{
		demoplayer->InterpolateViewpoint();
	}

	if ( !VideoMode_UpdateWindowSize() )
		return;
	materials->BeginFrame( host_frametime );
	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "EngineVGui_Simulate" );
		EngineVGui()->Simulate();
	}

	ClientDLL_FrameStageNotify( FRAME_RENDER_START );

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
				
	// Draw world, etc.
	V_RenderView();

	CL_TakeSnapshotAndSwap();	   
	
#if defined( REPLAY_ENABLED )
	if ( g_pReplay )
	{
		g_pReplay->CL_Render();
	}
#endif

	ClientDLL_FrameStageNotify( FRAME_RENDER_END );

	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "FrameEnd" );

		toolframework->RenderFrameEnd();

		g_EngineRenderer->FrameEnd();
	}

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

	{
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "materials_EndFrame" );

		materials->EndFrame();
	}
}

void _Host_RunFrame_Render()
{
#ifndef SWDS
	VPROF( "_Host_RunFrame_Render" );
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame_Render" );

	CheckSpecialCheatVars();

	int nOrgNoRendering = mat_norendering.GetInt();

	if ( cl_takesnapshot )
	{
		// turn off no-rendering mode, if taking screenshot
		mat_norendering.SetValue( 0 );
	}

	// update video if not running in background
	g_HostTimes.StartFrameSegment( FRAME_SEGMENT_RENDER );

	CL_LatchInterpolationAmount();

	{
		VPROF( "_Host_RunFrame_Render - UpdateScreen" );
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame_Render - UpdateScreen" );
		Host_UpdateScreen();
	}
	{
		VPROF( "_Host_RunFrame_Render - CL_DecayLights" );
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame_Render - CL_DecayLights" );
		CL_DecayLights ();
	}

	g_HostTimes.EndFrameSegment( FRAME_SEGMENT_RENDER );

	saverestore->OnFrameRendered();

#ifdef USE_SDL
	if ( g_pLauncherMgr )
	{
		g_pLauncherMgr->OnFrameRendered();
	}
#endif

	mat_norendering.SetValue( nOrgNoRendering );
#endif
}
// clang-format on
