//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderCoreBinding::luminance (RFC 0016 render.pass.luminance, K8
//			post cohort): the luminance counts behind the client's auto
//			exposure on the core, as the engine sees it. Plain types and no
//			render namespace, as render_core_panels.h.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_RENDER_CORE_LUMINANCE_H
#define RENDER_COMPOSITION_RENDER_CORE_LUMINANCE_H

class ITexture;

struct RenderCoreLuminanceStats
{
	unsigned long long queued;
	unsigned long long refused;
	unsigned long long recorded;
	unsigned long long resolved;
	unsigned long long failed;
	char lastFailure[256];
};

class IRenderCoreLuminance
{
public:
	// Main thread, in frame order with the render context's calls: counts
	// the texels of texture x0..x1, y0..y1 (inclusive) whose luminance
	// (render/pass/luminance/luminance.h) times scale lies in [minimum,
	// maximum], as the stream leaves the texture at this point; its slot is
	// marked here. A nonzero id, or 0 when the core did not take it.
	virtual unsigned Queue( ITexture *texture, int x0, int y0, int x1, int y1, float minimum,
	    float maximum, float scale ) = 0;
	// Main thread: the count (>= 0) once its frame completed on the GPU, -1
	// while pending, -2 when the query failed. A count is returned once.
	virtual int Result( unsigned id ) = 0;
	virtual void GetStats( RenderCoreLuminanceStats *out ) const = 0;

protected:
	~IRenderCoreLuminance() = default;
};

#endif // RENDER_COMPOSITION_RENDER_CORE_LUMINANCE_H
