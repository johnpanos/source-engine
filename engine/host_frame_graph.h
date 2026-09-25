//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Host frame graph interface (RFC 0003 R10). The host frame after
//          admission runs as an ordered serial graph of the legacy phases
//          (engine/host_frame_phases.h, run by engine/host_frame_graph.cpp);
//          host_frame_graph 0 runs the legacy hand-ordered body instead.
//          Declarations only, so host.h can carry them.
//
//=============================================================================//

#ifndef HOST_FRAME_GRAPH_H
#define HOST_FRAME_GRAPH_H

class CJob;

// The frame segments the phases time; engine/host.cpp owns the timer.
enum HostFrameSegment_t
{
	HOST_FRAME_SEGMENT_CMD_EXECUTE,
	HOST_FRAME_SEGMENT_CLDLL,
	HOST_FRAME_SEGMENT_RENDER,
};

void Host_StartFrameSegment( HostFrameSegment_t segment );
void Host_EndFrameSegment( HostFrameSegment_t segment );
void Host_MarkSwapTime();

// Cross-frame state of the threaded listen-server path. One owner for both the
// legacy body and the graph, so toggling host_frame_graph keeps continuity.
struct HostFrameCarry_t
{
	int numticks_last_frame;
	float host_remainder_last_frame;
	float last_frame_time;
};

enum HostFrameExit_t
{
	HOST_FRAME_COMPLETED,
	HOST_FRAME_EXIT_ENDDEMO,     // Host_EndGame longjmp(host_enddemo)
	HOST_FRAME_EXIT_ABORTSERVER, // Host_AbortServer longjmp(host_abortserver)
	HOST_FRAME_GRAPH_FAILED,     // graph rejected before any phase ran
};

// The render stage's steps (engine/host_render_steps.h), in legacy order.
enum HostRenderStepIndex_t
{
	HOST_RENDER_STEP_BEGIN,
	HOST_RENDER_STEP_UPDATE_SCREEN_BEGIN,
	HOST_RENDER_STEP_SCREEN_ADMIT, // first step of SCR_UpdateScreen
	HOST_RENDER_STEP_SCREEN_BEGIN_FRAME,
	HOST_RENDER_STEP_CLIENT_RENDER_START,
	HOST_RENDER_STEP_ENGINE_FRAME_BEGIN,
	HOST_RENDER_STEP_VIEW_RENDER,
	HOST_RENDER_STEP_PRESENT,
	HOST_RENDER_STEP_CLIENT_RENDER_END,
	HOST_RENDER_STEP_ENGINE_FRAME_END,
	HOST_RENDER_STEP_DYNAMIC_MODELS,
	HOST_RENDER_STEP_MATERIALS_END_FRAME, // last step of SCR_UpdateScreen
	HOST_RENDER_STEP_DECAY_LIGHTS,
	HOST_RENDER_STEP_END,

	HOST_RENDER_STEP_COUNT
};

// Locals the render stage carried between its blocks.
struct HostRenderState_t
{
	int nOrgNoRendering;
	bool bScreenAdmitted; // SCR_UpdateScreen passed its early-out checks
};

struct HostRenderStep_t
{
	const char *name;
	void ( *run )( HostRenderState_t & );
	bool bScreenStep; // runs only when the screen update was admitted
};

// The render stage: every step in order (the host_frame_graph 0 path), and
// one step at a time (the graph). engine/gl_screen.cpp; dedicated builds have
// only the empty _Host_RunFrame_Render in engine/cl_null.cpp.
void _Host_RunFrame_Render();
void Host_RunRenderStep( int iStep, HostRenderState_t &state );
const char *Host_GetRenderStepName( int iStep );

// Host services the render steps use (engine/host.cpp).
void CheckSpecialCheatVars();
void Host_TraceFrameEvent( const char *pszEvent, int nArg );

// Live capture (-hostframetrace): the async server job is in flight, so the
// host thread must not read server state for the trace (it would race with the
// job and differ run to run). Set after submission, cleared after the join.
void Host_TraceServerJob( bool bInFlight );

// Per-frame values the legacy body kept in locals.
struct HostFrameState_t
{
	// Shape and inputs, fixed at admission.
	int numticks;
	double prevremainder;
	double host_remainder;
	bool shouldrender;
	bool threaded;
	bool checkheap; // -heapcheck; fixed at Host_Init

	// Legacy locals shared between phases.
	CJob *pGameJob;
	int clientticks;
	int serverticks;
	int saveTick;
	bool bFinalTick;

	HostRenderState_t render;

	HostFrameCarry_t *pCarry;
	HostFrameExit_t exit;
};

// Threaded listen-server carry-over state, owned by engine/host.cpp.
extern HostFrameCarry_t g_HostFrameCarry;

// Runs the post-admission part of one host frame through the serial graph.
// The caller fills the admission fields of state and completes a reported
// error exit (return for host_enddemo, longjmp for host_abortserver).
HostFrameExit_t Host_RunFrameGraph( HostFrameState_t &state );

#endif // HOST_FRAME_GRAPH_H
