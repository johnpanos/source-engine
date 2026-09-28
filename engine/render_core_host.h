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

// Host render steps (host_render_steps.h): EngineFrameBegin and EngineFrameEnd.
void RenderCoreHost_BeginFrame();
void RenderCoreHost_EndFrame();

// CRender::Push3DView and PopView of a 3D view (gl_rmain.cpp).
void RenderCoreHost_MarkViewBegin();
void RenderCoreHost_MarkViewEnd();

// R_LevelInit and R_LevelShutdown (gl_rmisc.cpp): the world's scene lives
// from one to the other.
void RenderCoreHost_LevelInit();
void RenderCoreHost_LevelShutdown();

#endif // RENDER_CORE_HOST_H
