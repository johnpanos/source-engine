//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Host frame graph exports (RFC 0003 R10).
//
//=============================================================================//

#include "vstdlib/jobgraph_frame.h"

#include "tier0/memdbgon.h"

VSTDLIB_INTERFACE jobsystem::SerialFrameGraph *CreateSerialFrameGraph()
{
	return new jobsystem::SerialFrameGraph;
}

VSTDLIB_INTERFACE void DestroySerialFrameGraph( jobsystem::SerialFrameGraph *pGraph )
{
	delete pGraph;
}

VSTDLIB_INTERFACE bool RunSerialFrameGraph( jobsystem::SerialFrameGraph *pGraph,
    const jobsystem::FramePhaseDesc *pPhases, unsigned nPhases, jobsystem::SerialFrameRun *pResult )
{
	*pResult = pGraph->Run( pPhases, nPhases );
	return pResult->valid;
}
