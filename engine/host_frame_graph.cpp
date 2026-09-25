//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Runs the host frame's legacy phases as an ordered serial graph
//          (RFC 0003 R10). The phases themselves are in host_frame_phases.h,
//          shared with the host frame oracle test.
//
//=============================================================================//

#include "tier0/etwprof.h"
#include "tier0/vprof.h"
#include "tier1/utlvector.h"
#include "vstdlib/jobthread.h"
#include "vstdlib/jobgraph_frame.h"
#include "datacache/imdlcache.h"
#include "toolframework/itoolframework.h"

#include "host.h"
#include "host_jmp.h"
#include "host_state.h"
#include "server.h"
#include "sv_main.h"
#include "sv_log.h"
#include "client.h"
#include "cl_main.h"
#include "cl_pred.h"
#include "cdll_engine_int.h"
#include "cmd.h"
#include "demo.h"
#include "net.h"
#include "screen.h"
#include "modelloader.h"
#include "hltvserver.h"
#include "hltvtest.h"
#include "sys.h"
#include "sys_dll.h"
#include "sys_mainwind.h"
#include "testscriptmgr.h"
#include "ienginetoolinternal.h"

// The phases time their segments with host.cpp's frame-segment timer.
static const HostFrameSegment_t FRAME_SEGMENT_CMD_EXECUTE = HOST_FRAME_SEGMENT_CMD_EXECUTE;
static const HostFrameSegment_t FRAME_SEGMENT_CLDLL = HOST_FRAME_SEGMENT_CLDLL;
static struct CHostFrameSegmentTimer
{
	void StartFrameSegment( HostFrameSegment_t segment ) { Host_StartFrameSegment( segment ); }
	void EndFrameSegment( HostFrameSegment_t segment ) { Host_EndFrameSegment( segment ); }
	void MarkSwapTime() { Host_MarkSwapTime(); }
} g_HostTimes;

#include "host_frame_phases.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

HostFrameExit_t Host_RunFrameGraph( HostFrameState_t &state )
{
	// Graph and phase storage persist across frames: the sealed graph is reused
	// while the frame shape is unchanged. Host thread only.
	static jobsystem::SerialFrameGraph *s_pFrameGraph = CreateSerialFrameGraph();
	static CUtlVector<jobsystem::FramePhaseDesc> s_FramePhases;

	return HostFrame_RunGraph( s_pFrameGraph, state, s_FramePhases );
}
