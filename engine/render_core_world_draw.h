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
#include "render/draw_phase.h"

void RenderCoreWorldDraw_LevelInit();
void RenderCoreWorldDraw_LevelShutdown();
// R_DrawWorldLists: whether the core may draw this view's world (r_core_world,
// the outermost view, into the back buffer, no fog, not a shadow, SSAO,
// reflection or refraction list). bWorldMeshWorld: the view's opaque world is
// the map's WMSH (r_worldmesh_draw 2 with a resident WMSH): eligible when the
// core holds it as a world stage, else declined by name (r_core_world_stats)
// and drawn by the WMSH path alone.
bool RenderCoreWorldDraw_ViewEligible( unsigned long flags, bool bWorldMeshWorld );
// Whether the core draws this surface when it draws a view.
bool RenderCoreWorldDraw_Takes( SurfaceHandle_t surfID );
// Whether the core lights this surface's runtime light (r_core_world on and
// the core takes it): lighting the core evaluates per pixel (area lights)
// stays out of its lightmap, so each light counts once. Nested views, which
// the legacy stream draws, then show the surface without it (until K8).
bool RenderCoreWorldDraw_OwnsLighting( SurfaceHandle_t surfID );
// Queues the view's visible surfaces the core takes (surface indices) and
// marks the core's slot; from then until EndView the legacy chains skip them.
// waterZOffset: the height the view moves water surfaces by (the client's
// waterZAdjust).
void RenderCoreWorldDraw_BeginView( const unsigned int *pSurfaces, int nCount, float waterZOffset );
// A world stage (RFC 0016 K12: the map's world is its WMSH, and the core
// holds it): BeginStageView after the view is eligible, then the WMSH path
// asks StageView and StageTakesBatch, leaves the batches the core takes
// undrawn, and hands their visible meshlets to DrawStageView at the point of
// the stream where it would have drawn them.
void RenderCoreWorldDraw_BeginStageView();
bool RenderCoreWorldDraw_StageView();
bool RenderCoreWorldDraw_StageTakesBatch( unsigned int batch );
void RenderCoreWorldDraw_DrawStageView( const unsigned int *pMeshlets, int nCount );
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
// The legacy client culled these static props for this opaque view. Once
// the queued core world view accepts the cohort, studiorender omits it.
bool RenderCoreWorldDraw_TakeStaticProps( const unsigned int *props, int count );
bool RenderCoreWorldDraw_DrawsStaticProp( unsigned int prop );
// Whether this model and current view are candidates for a core Studio draw.
// Check before inspecting material overrides or renderable modulation.
bool RenderCoreWorldDraw_CanTakePosedModel( const model_t *model );
// Claims an eligible Studio draw with its engine-generated bone palette.
// Returns false when the model, view or material must stay with studiorender.
bool RenderCoreWorldDraw_TakePosedModel( const model_t *model, int skin,
    const matrix3x4_t *boneToWorld, int boneCount, RenderCoreDrawPhase phase );

#endif // RENDER_CORE_WORLD_DRAW_H
