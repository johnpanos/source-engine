//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VEngineLuminanceCount001 (RFC 0016 render.pass.luminance, K8 post
//          cohort): the luminance counts behind the client's auto exposure
//          (the tone-mapping histogram, viewpostprocess.cpp), counted by the
//          render core in place of a luminance_compare draw under an
//          occlusion query.
//
//          Main thread.
//
//=============================================================================//

#ifndef ILUMINANCECOUNT_H
#define ILUMINANCECOUNT_H
#ifdef _WIN32
#pragma once
#endif

class ITexture;

#define ENGINE_LUMINANCE_COUNT_INTERFACE_VERSION "VEngineLuminanceCount001"

class IEngineLuminanceCount
{
public:
	// Whether the core counts (when false the caller counts as before).
	virtual bool CoreCounts() = 0;
	// Queues a count, in frame order, of the texels of pTexture's rectangle
	// x0..x1, y0..y1 (inclusive) whose luminance times scale lies in
	// [minimum, maximum]. 0 when not queued: the caller counts as before.
	virtual unsigned Queue( ITexture *pTexture, int x0, int y0, int x1, int y1, float minimum,
	    float maximum, float scale ) = 0;
	// The count (>= 0) once ready, -1 while pending, -2 when it failed.
	virtual int Result( unsigned id ) = 0;

protected:
	~IEngineLuminanceCount() {}
};

#endif // ILUMINANCECOUNT_H
