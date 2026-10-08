//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Optional owner of primitives buffered ahead of the render context.
//
//          D3D9's shader API flushed its buffered mesh before any other draw
//          or copy, so buffered output kept its place in the frame. The
//          surface's screen UI list (vgui/IScreenUiRecorder.h) is buffered
//          the same way: while an owner is set, the render context flushes it
//          on the main thread before a material bind, a screen-space
//          rectangle or a render-target copy, so a panel that draws through
//          the render context between surface calls stays in paint order.
//
//=============================================================================//

#ifndef IMATERIALSYSTEMBUFFEREDPRIMITIVES_H
#define IMATERIALSYSTEMBUFFEREDPRIMITIVES_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

#define MATERIALSYSTEM_BUFFERED_PRIMITIVES_INTERFACE_VERSION "VMaterialSystemBufferedPrimitives001"

// Main thread only.
class IMaterialBufferedPrimitivesOwner
{
public:
	// Hands what the owner has buffered to the render context now. Called
	// before the render context's own draw; the owner may draw through the
	// render context here (it is not called again while it does).
	virtual void FlushBufferedPrimitives() = 0;

protected:
	virtual ~IMaterialBufferedPrimitivesOwner() {}
};

class IMaterialSystemBufferedPrimitives
{
public:
	// Null clears the owner. One owner at a time; main thread only.
	virtual void SetBufferedPrimitivesOwner( IMaterialBufferedPrimitivesOwner *pOwner ) = 0;

protected:
	virtual ~IMaterialSystemBufferedPrimitives() {}
};

#endif // IMATERIALSYSTEMBUFFEREDPRIMITIVES_H
