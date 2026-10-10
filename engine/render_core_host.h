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
// Frame-owned pixel jitter, zero when temporal reconstruction is not selected.
bool RenderCoreHost_TemporalJitter( float *x, float *y );
struct RenderCoreCostReport;
// Null output queries visibility only; otherwise reads the completed core sample.
bool RenderCoreHost_ReadCosts( RenderCoreCostReport *out );
namespace vgui
{
class Panel;
}
vgui::Panel *RenderCoreCostPanel_Create( vgui::Panel *parent );

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
// The screen UI drawn by the core (RFC 0016 render.pass.ui, render_core_ui.h);
// null when unbound.
class IRenderCoreUi;
IRenderCoreUi *RenderCoreHost_Ui();
// The luminance counts behind auto exposure (RFC 0016 render.pass.luminance,
// render_core_luminance.h); null when unbound.
class IRenderCoreLuminance;
IRenderCoreLuminance *RenderCoreHost_Luminance();
// The client's pixel visibility counts (RFC 0016 render.pass.visibility,
// render_core_visibility.h); null when unbound.
class IRenderCoreVisibility;
IRenderCoreVisibility *RenderCoreHost_Visibility();

// Host render steps (host_render_steps.h): EngineFrameBegin and EngineFrameEnd.
void RenderCoreHost_BeginFrame();
void RenderCoreHost_EndFrame();
// After the frame's last draw and before its present (RFC 0014: the frame's
// last slot, where cl_render_debug_legacy 1 tints what the core did not draw).
void RenderCoreHost_MarkFrameEnd();

// CRender::Push3DView and PopView of a 3D view (gl_rmain.cpp).
void RenderCoreHost_MarkViewBegin();
void RenderCoreHost_MarkViewEnd();

// R_LevelInit (gl_rmisc.cpp): registers the map's leaves with the core's scene.
void RenderCoreHost_LevelInit();

#endif // RENDER_CORE_HOST_H
