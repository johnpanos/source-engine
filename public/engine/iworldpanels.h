//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VEngineWorldPanels001 (RFC 0016 render.pass.panels, RFC 0010
//          in-world panels): how the client hands a panel placed in the
//          world (a vgui_screen) to the render core, which draws it as an
//          emissive surface (render.world-panel.v1, render/world_panel.h) in
//          the views it draws. The engine decides which views those are
//          (the core world's rule: r_core_world, the outermost view into the
//          back buffer) and supplies the view's transform and host frame.
//
//          Main thread, while a view renders.
//
//=============================================================================//

#ifndef IWORLDPANELS_H
#define IWORLDPANELS_H
#ifdef _WIN32
#pragma once
#endif

#include "render/composition/render_core_panels.h"
#include "render/world_panel.h"

#define ENGINE_WORLD_PANELS_INTERFACE_VERSION "VEngineWorldPanels001"

class IEngineWorldPanels
{
public:
	// Whether the core draws panels in the view being rendered now. When
	// false the caller draws its panel as before.
	virtual bool CoreDrawsPanels() = 0;
	// Draws the panel in the current view (the frame's first call submits its
	// list; later calls of the frame draw that list). False when the core did
	// not take it (r_core_panels_stats says why): the caller draws it.
	virtual bool DrawPanel( const RenderCorePanel &panel ) = 0;
	// The panel is gone.
	virtual void RemovePanel( unsigned long long id ) = 0;

protected:
	~IEngineWorldPanels() {}
};

#endif // IWORLDPANELS_H
