//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The BSP world drawn by the render core (RFC 0016 K5, render.pass.world),
// behind r_core_world. At level init the world's surfaces, their materials
// and lightmap pages go to the core (RenderCoreBinding::world); the core
// decides which materials its model draws. In each view that draws the world
// into the back buffer, outside nested views, the core draws the visible
// surfaces it takes at a slot of the legacy stream, and the legacy world
// chains skip exactly those surfaces.
//
// Main thread only. Names no render type (see render_core_world.h).
//
//=============================================================================//

#ifndef RENDER_CORE_WORLD_DRAW_H
#define RENDER_CORE_WORLD_DRAW_H

#include "gl_model_private.h"

void RenderCoreWorldDraw_LevelInit();
void RenderCoreWorldDraw_LevelShutdown();
// R_DrawWorldLists: whether the core may draw this view's world (r_core_world,
// the outermost view, into the back buffer, no fog, not a shadow, SSAO,
// reflection or refraction list). bWorldMeshWorld: the view's opaque world is
// the map's WMSH (r_worldmesh_draw 2 with a resident WMSH). Its faces, PBR
// materials and LMAP lighting are not the BSP faces the core holds, so the
// core declines the view by name (r_core_world_stats) until it draws WMSH
// (RFC 0016 K12); the WMSH path alone draws it.
bool RenderCoreWorldDraw_ViewEligible( unsigned long flags, bool bWorldMeshWorld );
// Whether the core draws this surface when it draws a view.
bool RenderCoreWorldDraw_Takes( SurfaceHandle_t surfID );
// Queues the view's visible surfaces the core takes (surface indices) and
// marks the core's slot; from then until EndView the legacy chains skip them.
void RenderCoreWorldDraw_BeginView( const unsigned int *pSurfaces, int nCount );
void RenderCoreWorldDraw_EndView();
// Shader_DrawChainsStatic: whether the core draws this surface in the
// current view.
bool RenderCoreWorldDraw_Skips( SurfaceHandle_t surfID );
// Shader_WorldEnd: whether this view's opaque world must go through the
// legacy chains, which skip per surface (RenderCoreWorldDraw_Skips), rather
// than the WMSH batches, which cannot: the core draws surfaces in this view,
// or r_core_world_isolate is on. The core's world and the legacy world never
// both draw a surface.
bool RenderCoreWorldDraw_ChainsOnly();

#endif // RENDER_CORE_WORLD_DRAW_H
