//========= Portal 2 reconstruction ============================================//
//
// Purpose: Paint (stick surface / taunt camera) aware client input.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_PAINT_INPUT_H
#define C_PAINT_INPUT_H

#ifdef _WIN32
#pragma once
#endif

#include "input.h"

//-----------------------------------------------------------------------------
// Purpose: Input handler that rotates the view around the player's current
//			up vector (wall/ceiling walking) and drives the taunt camera.
//-----------------------------------------------------------------------------
class C_Paint_Input : public CInput
{
public:
	C_Paint_Input() {}
	~C_Paint_Input() {}

protected:
	virtual void ApplyMouse( int nSlot, QAngle &viewangles, CUserCmd *cmd, float mouse_x, float mouse_y );
	virtual void JoyStickTurn( CUserCmd *cmd, float &yaw, float &pitch, float frametime, bool bAbsoluteYaw, bool bAbsolutePitch );
};

#endif // C_PAINT_INPUT_H
