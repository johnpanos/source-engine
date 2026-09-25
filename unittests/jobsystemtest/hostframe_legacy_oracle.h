//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the host frame graph test: the legacy hand-ordered
//          _Host_RunFrame body after admission, extracted mechanically from
//          engine/host.cpp at f7eb1cf1 (lines 2809-3252). The only
//          substitutions: the admission locals (numticks, prevremainder,
//          shouldrender, host_remainder) are parameters, and the threaded
//          path's function statics are the oracle's resettable carry state.
//          Do not edit the body; regenerate it from the source revision.
//
//=============================================================================//

// clang-format off
static void LegacyHostFrameBody( int numticks, double prevremainder, bool shouldrender, double host_remainder )
{
	{
		// Profile scope, protect from setjmp() problems
		VPROF( "_Host_RunFrame" );
		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "_Host_RunFrame" );

		g_HostTimes.StartFrameSegment( FRAME_SEGMENT_CMD_EXECUTE );

		// process console commands
		Cbuf_Execute ();

		// initialize networking for dedicated server after commandline & autoexec.cfg have been parsed
		if ( NET_IsDedicated() && !NET_IsMultiplayer() )
			NET_SetMutiplayer( true );

		g_HostTimes.EndFrameSegment( FRAME_SEGMENT_CMD_EXECUTE );

		// Msg( "Running %i ticks (%f remainder) for frametime %f total %f tick %f delta %f\n", numticks, remainder, host_frametime, host_time );
		g_ServerGlobalVariables.interpolation_amount = 0.0f;
#ifndef SWDS
		g_ClientGlobalVariables.interpolation_amount = 0.0f;

		cl.insimulation = true;
#endif

		host_frameticks = numticks;
		host_currentframetick = 0;

#if !defined( SWDS )
		// This is to make the tool do both sim + rendering on the initial frame
		// cl.IsActive changes in the loop below, as does scr_nextdrawtick
		// We're just caching off the state here so that we have a consistent return value
		// for enginetool->IsInGame the entire frame
		g_pEngineToolInternal->SetIsInGame( cl.IsActive() && ( scr_nextdrawtick == 0 ) );
#endif
		CJob *pGameJob = NULL;

// threaded path only supported in listen server
#ifndef SWDS
		if ( !IsEngineThreaded() )
#endif
		{
#ifndef SWDS
			if ( g_ClientDLL )
			{
				g_ClientDLL->IN_SetSampleTime(host_frametime);
			}
			g_ClientGlobalVariables.simTicksThisFrame = 1;
#endif
			cl.m_tickRemainder = host_remainder;
			g_ServerGlobalVariables.simTicksThisFrame = 1;
			cl.SetFrameTime( host_frametime );
			for ( int tick = 0; tick < numticks; tick++ )
			{ 
				// Emit an ETW event every simulation frame.
				ETWSimFrameMark( sv.IsDedicated() );

				double now = Plat_FloatTime();
				float jitter = now - host_idealtime;

				// Track jitter (delta between ideal time and actual tick execution time)
				host_jitterhistory[ host_jitterhistorypos ] = jitter;
				host_jitterhistorypos = ( host_jitterhistorypos + 1 ) % ARRAYSIZE(host_jitterhistory);

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
				bool bFinalTick = ( tick == (numticks - 1) );

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
				//-------------------
				// input processing
				//-------------------
				_Host_RunFrame_Input( prevremainder, bFinalTick );
				prevremainder = 0;
				//-------------------
				//
				// server operations
				//
				//-------------------

				_Host_RunFrame_Server( bFinalTick );

				// Additional networking ops for SPLITPACKET stuff (99.9% of the time this will be an empty list of work)
				NET_SendQueuedPackets();
				//-------------------
				//
				// client operations
				//
				//-------------------
#ifndef SWDS
				if ( !sv.IsDedicated() )
				{
					_Host_RunFrame_Client( bFinalTick );
				}

				toolframework->Think( bFinalTick );
#endif

				host_idealtime += host_state.interval_per_tick;
			}
			
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

#ifndef SWDS
			// This is a hack to let timedemo pull messages from the queue faster than every 15 msec
			// Also when demoplayer is skipping packets to a certain tick we should process the queue
			// as quickly as we can.
			if ( numticks == 0 && ( demoplayer->IsPlayingTimeDemo() || demoplayer->IsSkipping() ) )
			{
				_Host_RunFrame_Client( true );
			}

			if ( !sv.IsDedicated() )
			{
				tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "Host_SetClientInSimulation" );

				// This causes cl.gettime() to return the true clock being used for rendering (tickcount * rate + remainder)
				Host_SetClientInSimulation( false );
				// Now allow for interpolation on client
				g_ClientGlobalVariables.interpolation_amount = ( cl.m_tickRemainder / host_state.interval_per_tick );

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
#if LOG_FRAME_OUTPUT
			if ( !cl.IsPaused() || !sv.IsPaused() )
			{
				Msg("=============SIM: CLIENT %5d + %d, SERVER %5d + %d\t REM: %.2f\n", cl.GetClientTickCount(), numticks, sv.m_nTickCount, numticks, host_remainder*1000.0f );
			}
#endif
		}
#ifndef SWDS
		else
		{
			int &numticks_last_frame = g_OracleCarry.numticks_last_frame;
			float &host_remainder_last_frame = g_OracleCarry.host_remainder_last_frame;
			float &last_frame_time = g_OracleCarry.last_frame_time;

			int clientticks;
			int serverticks;

			clientticks = numticks_last_frame;
			cl.m_tickRemainder = host_remainder_last_frame;
			cl.SetFrameTime( last_frame_time );
			if ( g_ClientDLL )
			{
				g_ClientDLL->IN_SetSampleTime(last_frame_time);
			}

			last_frame_time = host_frametime;

			serverticks = numticks;
			g_ClientGlobalVariables.simTicksThisFrame = clientticks;
			g_ServerGlobalVariables.simTicksThisFrame = serverticks;
			g_ServerGlobalVariables.tickcount = sv.m_nTickCount;

			// THREADED: Run Client
			// -------------------
			for ( int tick = 0; tick < clientticks; tick++ )
			{ 
				// process any asynchronous network traffic (TCP), set net_time
				NET_RunFrame(  Plat_FloatTime() );

				// Only send updates on final tick so we don't re-encode network data multiple times per frame unnecessarily
				bool bFinalTick = ( tick == (clientticks - 1) );

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
			// This is a hack to let timedemo pull messages from the queue faster than every 15 msec
			// Also when demoplayer is skipping packets to a certain tick we should process the queue
			// as quickly as we can.
			if ( clientticks == 0 && ( demoplayer->IsPlayingTimeDemo() || demoplayer->IsSkipping() ) )
			{
				_Host_RunFrame_Client( true );
			}

			// This causes cl.gettime() to return the true clock being used for rendering (tickcount * rate + remainder)
			Host_SetClientInSimulation( false );
			// Now allow for interpolation on client
			g_ClientGlobalVariables.interpolation_amount = ( cl.m_tickRemainder / host_state.interval_per_tick );

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
			int saveTick = g_ClientGlobalVariables.tickcount;

			for ( int tick = 0; tick < serverticks; tick++ )
			{
				// NOTE:  Do we want do this at start or end of this loop?
				++host_tickcount;
				++host_currentframetick;
				g_ClientGlobalVariables.tickcount = host_tickcount;
				bool bFinalTick = tick==(serverticks-1) ? true : false;
				_Host_RunFrame_Input( prevremainder, bFinalTick );
				prevremainder = 0;
				// process any asynchronous network traffic (TCP), set net_time
				NET_RunFrame(  Plat_FloatTime() );
			}

			Host_SetClientInSimulation( false );

			// The mouse is always simulated for the current frame's time
			// This makes updates smooth in every case
			// continuous controllers affecting the view are also simulated this way
			// but they have a cap applied by IN_SetSampleTime() so they are not also
			// simulated during input gathering
			CL_ExtraMouseUpdate( g_ClientGlobalVariables.frametime );

			g_ClientGlobalVariables.tickcount = saveTick;
			numticks_last_frame = numticks;
			host_remainder_last_frame = host_remainder;

			// THREADED: Run Server
			// -------------------
			// set net_time once before running the server
			NET_SetTime( Plat_FloatTime() );
			pGameJob = new CFunctorJob( CreateFunctor( _Host_RunFrame_Server_Async, serverticks ) );
			
			g_pThreadPool->AddJob( pGameJob );
#if LOG_FRAME_OUTPUT
			if ( !cl.IsPaused() || !sv.IsPaused() )
			{
				Msg("=============SIM: CLIENT %5d + %d, SERVER %5d + %d\t REM: %.2f\n", cl.GetClientTickCount(), clientticks, sv.m_nTickCount, serverticks, host_remainder*1000.0f );
			}
#endif
		}
#endif	// SWDS

		g_Log.RunFrame();

		if ( shouldrender )
		{
#if LOG_FRAME_OUTPUT
			if ( !cl.IsPaused() || !sv.IsPaused() )
			{
				static float lastFrameTime = 0;
				float frametime = g_ClientGlobalVariables.curtime - lastFrameTime;
				Msg("RENDER AT: %6.4f: %.2fms [%.2fms implicit] frametime\n", 
					g_ClientGlobalVariables.curtime, g_ClientGlobalVariables.frametime*1000.0f, frametime * 1000.0f);
				lastFrameTime = g_ClientGlobalVariables.curtime;
			}
#endif
			//-------------------
			// rendering
			//-------------------
			_Host_RunFrame_Render();

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
		else
		{
			tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "modelloader->UpdateDynamicModels" );
			VPROF( "UpdateDynamicModels" );
			CMDLCacheCriticalSection critsec( g_pMDLCache );
			modelloader->UpdateDynamicModels();
		}

		//-------------------
		// simulation
		//-------------------
		g_HostTimes.MarkSwapTime( );
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
		if ( pGameJob )
		{
			{
				VPROF_BUDGET( "WaitForAsyncServer", "AsyncServer" );
				if ( Host_IsSinglePlayerGame() )
				{
					// This should change to a YieldWait if the server starts wanting to parallel process. If
					// so, will need some route for the server to queue up work it wants to execute outside
					// its frame, otherwise some of it would be performed during the yield. Right now
					// need to wait for server so we don't stall on queued AI operations (toml 7/3/2007)
					pGameJob->ExecuteAndRelease();
				}
				else
				{
					pGameJob->WaitForFinishAndRelease();
				}
			}
			SV_FrameExecuteThreadDeferred();
		}

		//-------------------
		// time
		//-------------------

		Host_Speeds();

		Host_UpdateMapList();

		host_framecount++;
#if !defined(SWDS)
		if ( !demoplayer->IsPlaybackPaused() )
#endif
		{
			host_time = host_tickcount * host_state.interval_per_tick + cl.m_tickRemainder;
		}

		Host_PostFrameRate( host_frametime );

		if ( host_checkheap )
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
	} // Profile scope, protect from setjmp() problems

	Host_ShowIPCCallCount();
}
// clang-format on
