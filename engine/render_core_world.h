//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The world's render scene (RFC 0016 K5): one instance per non-solid BSP
// leaf of the loaded map, bounded by the box the legacy traversal last tests
// for it, and one per static prop, bounded by its world render box. The
// render core culls it with the view's own planes; the legacy view's
// visibility is its provider (the BSP traversal's PVS, area bits and area
// frustums for leaves; the props the client drew, after its leaf, fade and
// frustum tests). r_core_cull_capture compares the core's culled leaves and
// props with the legacy ones for every 3D view of a frame that builds a world
// list (tools/render/culling_capture.py). Main thread only.
//
// This file names no render type: the engine's global `render` (ivrenderview.h)
// and the core's namespace cannot meet in one translation unit, so the core
// side lives in render_core_host.cpp and the two exchange plain arrays.
//
//=============================================================================//

#ifndef RENDER_CORE_WORLD_H
#define RENDER_CORE_WORLD_H

// After the world scene is created for a loaded map: adds the leaf instances
// through RenderCoreHost_SetWorldLeaves.
void RenderCoreWorld_LevelInit();

// CRender::Push3DView and PopView: a view's record opens and, at its end, is
// culled and written. Views nest (portal and water views inside the main
// view), so records are a stack.
void RenderCoreWorld_ViewBegin();
void RenderCoreWorld_ViewEnd();
// The number of 3D views open now (1 inside the outermost).
int RenderCoreWorld_ViewDepth();

// Whether a capture records this frame (a cheap test for the prop hook).
bool RenderCoreWorld_Capturing();

// CStaticPropMgr::DrawStaticProps, for a non-shadow draw: the props drawn
// into the current view (indices into the static props).
void RenderCoreWorld_OnStaticPropsDrawn( const int *pProps, int nCount );

// R_BuildWorldLists, after its traversal, for a non-shadow view: the legacy
// visible leaves (indices into the world's leaves) and the view origin.
void RenderCoreWorld_OnWorldList( const unsigned short *pLeaves, int nLeafCount,
    const float *pOrigin, bool bForcedLeaf, bool bVisOverride, bool bWaterReflection );

// Frame boundaries: a capture requested by r_core_cull_capture covers the
// next whole frame and is written at its end.
// True only for an enabled, bound core in a loaded world. No legacy shaders
// are used in this mode.
bool RenderCoreWorldDraw_OnlyCore();
// A world stage supplies runtime light per pixel. Retail BSP core surfaces
// still read CPU dynamic light from their pages until that cohort migrates.
bool RenderCoreWorldDraw_StageOwnsRuntimeLighting();
void RenderCoreWorld_BeginFrame();
void RenderCoreWorld_EndFrame();

#endif // RENDER_CORE_WORLD_H
