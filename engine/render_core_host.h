//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The engine's side of the render core (RFC 0016 A.7): it drives the core's
// frames around the host's render steps, marks every 3D view the engine
// pushes, and owns the world's render scene for the loaded map. Every call
// does nothing when no core is bound (public/engine/render_core_binding.h).
//
// Main thread only, like the host render steps that call it.
//
//=============================================================================//

#ifndef RENDER_CORE_HOST_H
#define RENDER_CORE_HOST_H

namespace world_mesh_gpu
{
class IWorldMeshUpload;
}
namespace light_set
{
class ILightSetConsumer;
}
namespace gpu_compute
{
class IGpuCompute;
}

bool RenderCoreHost_IsBound();

// The legacy backend's optional capabilities, through the core's legacy
// frontend (render/legacy/capabilities.h): world mesh and light set calls
// reach the backend in frame order. Null when no core is bound
// (-norendercore) or the backend lacks the capability.
world_mesh_gpu::IWorldMeshUpload *RenderCoreHost_WorldMeshUpload();
light_set::ILightSetConsumer *RenderCoreHost_LightSetConsumer();
gpu_compute::IGpuCompute *RenderCoreHost_GpuCompute();
// The legacy backend's compute service, for the ray-query producer alone:
// the core's device port has no acceleration structures yet (RFC 0016 K12).
gpu_compute::IGpuCompute *RenderCoreHost_RayQueryGpuCompute();
// RFC 0016 K5: the BSP world drawn by the core (render_core_world.h in
// public/render/composition); null when no core is bound.
class IRenderCoreWorld;
class IRenderCorePanels;
IRenderCoreWorld *RenderCoreHost_World();
// The in-world panels drawn by the core (RFC 0016 render.pass.panels,
// render_core_panels.h); null when unbound.
IRenderCorePanels *RenderCoreHost_Panels();

// Host render steps (host_render_steps.h): EngineFrameBegin and EngineFrameEnd.
void RenderCoreHost_BeginFrame();
void RenderCoreHost_EndFrame();
// After the frame's last draw and before its present (RFC 0014: the frame's
// last slot, where cl_render_debug_legacy 1 tints what the core did not draw).
void RenderCoreHost_MarkFrameEnd();

// CRender::Push3DView and PopView of a 3D view (gl_rmain.cpp).
void RenderCoreHost_MarkViewBegin();
void RenderCoreHost_MarkViewEnd();

// R_LevelInit and R_LevelShutdown (gl_rmisc.cpp): the world's scene lives
// from one to the other.
void RenderCoreHost_LevelInit();
void RenderCoreHost_LevelShutdown();

// RFC 0016 K5, for render_core_world.cpp: the world scene's instances
// (nCount boxes as min x y z, max x y z, each with its code: a BSP leaf's
// index, or -(p + 1) for static prop p), and the core's culling of them for
// one view: pPlanes holds nPlanes planes as normal x y z and dist (inside
// when normal . p >= dist); pVisibleLeaf and pVisibleProp flag what the
// legacy view found visible, the view's visibility provider (the BSP
// traversal for leaves; the client's prop list for props). Writes the drawn
// codes (at most the instance count) and the draw list's culled counts, and
// whether the pooled builder's list equals it item for item (1 or 0; -1
// without a pooled builder); false without a world scene.
bool RenderCoreHost_SetWorldInstances( const float *pBoxes, const int *pCodes, int nCount );
bool RenderCoreHost_CullWorld( const float *pPlanes, int nPlanes, const unsigned char *pVisibleLeaf,
    int nLeafCount, const unsigned char *pVisibleProp, int nPropCount, int *pDrawnCodes,
    int *pDrawnCount, int *pFrustumCulled, int *pProviderCulled, int *pPooledEqual );
int RenderCoreHost_WorldInstanceCount();

#endif // RENDER_CORE_HOST_H
