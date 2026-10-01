//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Studio's whole-model and two-pass draws, shared by composition and the
// world pass. A phase is claimed only when every surface in that phase can
// be drawn by the core.
//
//=============================================================================//

#ifndef RENDER_DRAW_PHASE_H
#define RENDER_DRAW_PHASE_H

enum class RenderCoreDrawPhase
{
	kAll,
	kOpaque,
	kBlended
};

#endif // RENDER_DRAW_PHASE_H
