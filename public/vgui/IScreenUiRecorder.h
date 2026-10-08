//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VGuiScreenUiRecorder001 (RFC 0010 "UI draw-list contract", RFC
//          0016 K8 UI cohort): the material system surface records what
//          VGUI paints to the screen into a draw list (render.ui-draw-list.v1,
//          render/ui_draw_list.h) and hands each segment of it to a consumer
//          the application root sets (the engine: the render core's
//          dynamic draws of claimed materials). A segment the consumer does
//          not take, a draw whose material the consumer refuses, and every
//          line the surface draws through the material system as before, at
//          the same point of the frame.
//
//          The surface serves it through its QueryInterface, as
//          VGuiWorldPanelRecorder001.
//
//=============================================================================//

#ifndef ISCREENUIRECORDER_H
#define ISCREENUIRECORDER_H
#ifdef _WIN32
#pragma once
#endif

#include "render/ui_draw_list.h"

class IMaterial;

#define VGUI_SCREEN_UI_RECORDER_INTERFACE_VERSION "VGuiScreenUiRecorder001"

// Main thread, while the surface paints to the screen.
class IScreenUiConsumer
{
public:
	// Whether the consumer draws the screen UI of the paint starting now.
	virtual bool TakesScreenUi() = 0;
	// The consumer's key for a material it draws list commands with, or 0
	// with the reason in pszWhy: the surface draws that material's
	// primitives through the material system. Valid for the paint.
	virtual unsigned ScreenUiMaterial( IMaterial *pMaterial, char *pszWhy, int nWhySize ) = 0;
	// Draws a segment at this point of the frame's stream: materialKeys[i]
	// is the key of the list's material i. False when it did not take it:
	// the surface draws the segment itself.
	virtual bool DrawScreenUi(
	    const ui_draw_list::ListView &list, const unsigned *materialKeys ) = 0;

protected:
	~IScreenUiConsumer() {}
};

struct ScreenUiStats
{
	unsigned long long segments;      // segments handed to the consumer and taken
	unsigned long long declined;      // segments the consumer did not take
	unsigned long long commands;      // commands in taken segments
	unsigned long long materialDraws; // draws the consumer refused, drawn by the material system
	char lastReason[256];             // why the last such draw was refused
};

class IScreenUiRecorder
{
public:
	// Null stops recording: the surface draws through the material system.
	virtual void SetConsumer( IScreenUiConsumer *pConsumer ) = 0;
	virtual void GetStats( ScreenUiStats *pStats ) const = 0;

protected:
	~IScreenUiRecorder() {}
};

#endif // ISCREENUIRECORDER_H
