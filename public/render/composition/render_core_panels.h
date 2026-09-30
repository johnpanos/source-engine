//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderCoreBinding::panels (RFC 0016 render.pass.panels, RFC 0010
//			in-world panels): a UI panel placed in the world, drawn by the
//			render core as an emissive surface (render.world-panel.v1,
//			render/world_panel.h), as the engine sees it. Plain structs and
//			material system types only, and no render namespace, as
//			render_core_world.h.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_RENDER_CORE_PANELS_H
#define RENDER_COMPOSITION_RENDER_CORE_PANELS_H

#include "render/world_panel.h"

class ITexture;

struct RenderCorePanel
{
	unsigned long long id; // stable across frames (the core keeps its image per id)
	world_panel::Placement placement;
	float unitsWide;
	float unitsTall;
	const world_panel::Quad *quads; // Quad::texture indexes textures (or is kWhite)
	unsigned int quadCount;
	ITexture *const *textures;
	unsigned int textureCount;
	world_panel::Resolution resolution; // the list was painted for it
	float emissionScale;                // scene radiance per decoded image value
	// The scene's light at the panel (Source's ambient cube, +x -x +y -y +z
	// -z, linear), which its coatings' albedo reflects.
	float ambientCube[6][3];
};

struct RenderCorePanelStats
{
	unsigned long long submitted; // lists accepted (one per panel and frame)
	unsigned long long refused;
	unsigned long long rasterized; // images rasterized (one per panel and frame drawn)
	unsigned long long viewsQueued;
	unsigned long long viewsDrawn;
	unsigned long long viewsFailed; // claimed panels not drawn: never legacy's
	unsigned long long panelsDrawn;
	unsigned long long textureBytes; // the resident images, mips included
	world_panel::Resolution lastResolution;
	char lastFailure[256];
	char lastRefusal[256]; // why the last panel was not taken
};

class IRenderCorePanels
{
public:
	// Main thread, in a view the core draws (the engine decides which): the
	// panel's list for the host frame (the frame's first call submits it;
	// every later call of the frame draws that same list), drawn in the
	// current view with its world-to-clip (row-major, column vectors, D3D9
	// conventions) and viewport (x, y, width, height, min and max depth),
	// its slot marked here in the frame's stream. False when the core did
	// not take it (no backend slots, a texture that is not resident, a list
	// the pass refuses; lastRefusal says why): then the caller draws it.
	virtual bool DrawPanel( const RenderCorePanel &panel, const float worldToClip[16],
	    const float viewport[6], unsigned long long hostFrame ) = 0;
	// The panel is gone: its image is released behind the frames that used it.
	virtual void RemovePanel( unsigned long long id ) = 0;
	// Views and panels the core failed to draw after taking them, so far.
	virtual unsigned long long Failures() const = 0;
	virtual void GetStats( RenderCorePanelStats *out ) const = 0;

protected:
	~IRenderCorePanels() = default;
};

#endif // RENDER_COMPOSITION_RENDER_CORE_PANELS_H
