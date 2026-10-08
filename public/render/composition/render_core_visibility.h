//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderCoreBinding::visibility (RFC 0016 render.pass.visibility,
//			K8 sprites cohort): the client's pixel visibility proxies counted
//			by the core under occlusion queries, as the engine sees it. Plain
//			types and no render namespace, as render_core_panels.h.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_RENDER_CORE_VISIBILITY_H
#define RENDER_COMPOSITION_RENDER_CORE_VISIBILITY_H

struct RenderCoreVisibilityStats
{
	unsigned long long queued;
	unsigned long long refused;
	unsigned long long recorded;
	unsigned long long resolved;
	unsigned long long failed;
	char lastFailure[256];
};

class IRenderCoreVisibility
{
public:
	// Main thread, in frame order with the render context's calls: the
	// proxy (the apex then four base corners, clip space with D3D9 pixel
	// centers) drawn at this point of the frame's stream in the viewport (x,
	// y, width, height, min and max depth, in the target's pixels), its slot
	// marked here. A nonzero id, or 0 when the core did not take it.
	virtual unsigned Queue( const float points[5][4], const float viewport[6] ) = 0;
	// Main thread: 1 with the samples visible and possible once its frame
	// completed on the GPU (returned once), 0 while pending, -1 when the
	// query failed.
	virtual int Result( unsigned id, long long *visible, long long *possible ) = 0;
	virtual void GetStats( RenderCoreVisibilityStats *out ) const = 0;

protected:
	~IRenderCoreVisibility() = default;
};

#endif // RENDER_COMPOSITION_RENDER_CORE_VISIBILITY_H
