//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Host frame graph exports (RFC 0003 R10). C++11-clean; see
//          jobsystem/serial_frame_graph.h for the phase and run contract.
//
//=============================================================================//

#ifndef VSTDLIB_JOBGRAPH_FRAME_H
#define VSTDLIB_JOBGRAPH_FRAME_H

#ifdef _WIN32
#pragma once
#endif

#include "vstdlib/vstdlib.h"
#include "jobsystem/serial_frame_graph.h"

// The caller owns the graph and destroys it with DestroySerialFrameGraph.
VSTDLIB_INTERFACE jobsystem::SerialFrameGraph *CreateSerialFrameGraph();
VSTDLIB_INTERFACE void DestroySerialFrameGraph( jobsystem::SerialFrameGraph *pGraph );

// Runs the phases through the graph (SerialFrameGraph::Run) and stores the
// outcome in *pResult. Returns pResult->valid.
VSTDLIB_INTERFACE bool RunSerialFrameGraph( jobsystem::SerialFrameGraph *pGraph,
    const jobsystem::FramePhaseDesc *pPhases, unsigned nPhases,
    jobsystem::SerialFrameRun *pResult );

#endif // VSTDLIB_JOBGRAPH_FRAME_H
