//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderCoreBinding::ui (RFC 0016 K8 UI cohort; RFC 0010's UI
//			draw-list consumer on the core): the screen UI's draw list
//			(render.ui-draw-list.v1, render/ui_draw_list.h) drawn by the
//			render core, each command a dynamic draw of its claimed material
//			(render.pass.world), as the engine sees it. Plain structs and
//			material system types only, and no render namespace, as
//			render_core_panels.h.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_RENDER_CORE_UI_H
#define RENDER_COMPOSITION_RENDER_CORE_UI_H

#include "render/composition/render_core_world.h"
#include "render/ui_draw_list.h"

struct RenderCoreUiStats
{
	unsigned long long submitted; // lists the core took
	unsigned long long refused;   // lists not taken (lastRefusal says why): the caller drew them
	unsigned long long commands;  // commands in the lists taken
	char lastRefusal[256];
};

class IRenderCoreUi
{
public:
	// Main thread: whether the core draws this material's commands (it claims
	// it as a dynamic draw's); false with the reason in why.
	virtual bool ClaimsMaterial(
	    const RenderCoreWorldMaterial &material, char *why, int whySize ) = 0;
	// Main thread, in frame order with the render context's calls: the list
	// drawn at this point of the frame's stream, its slot marked here;
	// materials[i] is the list's material i. False when the core did not
	// take it (no backend slots, a malformed list, a material it refuses;
	// lastRefusal says why): then the caller draws it. A list taken and not
	// drawn is a failed world view (r_core_world_strict's).
	virtual bool DrawList(
	    const ui_draw_list::ListView &list, const RenderCoreWorldMaterial *materials ) = 0;
	virtual void GetStats( RenderCoreUiStats *out ) const = 0;

protected:
	~IRenderCoreUi() = default;
};

#endif // RENDER_COMPOSITION_RENDER_CORE_UI_H
