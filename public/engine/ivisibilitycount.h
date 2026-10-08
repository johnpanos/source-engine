//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VEngineVisibilityCount001 (RFC 0016 render.pass.visibility, K8
//          sprites cohort): the client's pixel visibility proxies
//          (c_pixel_visibility.cpp) counted by the render core under
//          occlusion queries, in place of occlusionproxy draws under the
//          material system's.
//
//          Main thread.
//
//=============================================================================//

#ifndef IVISIBILITYCOUNT_H
#define IVISIBILITYCOUNT_H
#ifdef _WIN32
#pragma once
#endif

#define ENGINE_VISIBILITY_COUNT_INTERFACE_VERSION "VEngineVisibilityCount001"

class IEngineVisibilityCount
{
public:
	// Whether the core counts (when false the caller counts as before).
	virtual bool CoreCounts() = 0;
	// Queues the proxy (the apex then four base corners, in clip space as the
	// view's transform gives them) drawn in the frame's current viewport (x,
	// y, width, height, min and max depth), in frame order. 0 when not
	// queued: the caller counts as before.
	virtual unsigned Queue( const float points[5][4], const float viewport[6] ) = 0;
	// 1 with the samples visible and possible, 0 while pending, -1 failed.
	virtual int Result( unsigned id, int *pVisible, int *pPossible ) = 0;

protected:
	~IEngineVisibilityCount() {}
};

#endif // IVISIBILITYCOUNT_H
