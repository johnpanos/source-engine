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
