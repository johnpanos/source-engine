//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The screen UI drawn by the render core (RFC 0016 render.pass.ui): the
// engine sets itself as the material system surface's screen UI consumer
// (vgui/IScreenUiRecorder.h) once VGUI is up, and clears it before VGUI goes.
//
//=============================================================================//

#ifndef RENDER_CORE_UI_H
#define RENDER_CORE_UI_H

void EngineScreenUi_Install();
void EngineScreenUi_Remove();

#endif // RENDER_CORE_UI_H
