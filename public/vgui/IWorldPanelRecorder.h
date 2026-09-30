//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VGuiWorldPanelRecorder001 (RFC 0010 in-world panels, RFC 0016
//          render.world-panel.v1): the material system surface paints a
//          panel placed in the world into a draw list (render/world_panel.h)
//          instead of the screen, so the render core can draw it as an
//          emissive surface at the resolution its footprint needs. Text is
//          rasterized at the image's density (a copy of each font at that
//          many pixels per unit), so it is never magnified; the layout is
//          the panel's own, in its units, unchanged.
//
//          The surface serves it through its QueryInterface, so a client
//          finds it with the same appSystemFactory lookup as the surface.
//
//=============================================================================//

#ifndef IWORLDPANELRECORDER_H
#define IWORLDPANELRECORDER_H
#ifdef _WIN32
#pragma once
#endif

#include "render/world_panel.h"
#include "vgui/VGUI.h"

class ITexture;

#define VGUI_WORLD_PANEL_RECORDER_INTERFACE_VERSION "VGuiWorldPanelRecorder001"

// Receives a recording's quads in paint order. pTexture is the quad's
// texture (a material's base texture, or a font's glyph page); null for
// white. quad.texture is not set: the receiver indexes its textures.
class IWorldPanelRecording
{
public:
	virtual void AddQuad( const world_panel::Quad &quad, ITexture *pTexture ) = 0;

protected:
	~IWorldPanelRecording() {}
};

class IWorldPanelRecorder
{
public:
	// Paints `root`, laid out in wide x tall units, once into `pRecording`
	// for an image of texelsPerUnit texels per unit. Nothing reaches the
	// screen. False when the panel paints what a draw list cannot hold (a
	// line, a polygon, a per-vertex fade, a circle) or the surface is already
	// drawing, with the reason in pszWhy: the recording is incomplete and must
	// not be drawn.
	virtual bool RecordPanel( vgui::VPANEL root, int wide, int tall, float texelsPerUnit,
	    IWorldPanelRecording *pRecording, char *pszWhy, int nWhySize ) = 0;

protected:
	~IWorldPanelRecorder() {}
};

#endif // IWORLDPANELRECORDER_H
