//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The engine's side of the render core; see render_core_host.h.
//
//=============================================================================//

#include "render_core_host.h"

#include "engine/render_core_binding.h"
#include "render/composition/render_core.h"
#include "render/frame/renderer.h"
#include "render/scene/scene.h"
#include "ivideomode.h"
#include "tier0/dbg.h"
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{

struct RenderCoreHostState
{
	bool bound = false;
	render::frame::IRenderer *renderer = nullptr;
	render::scene::SceneFactory sceneFactory;
	const char *deviceName = nullptr;
	render::legacy::ILegacyCapabilities *capabilities = nullptr;
	std::unique_ptr<render::scene::IRenderScene> worldScene;
	bool inFrame = false;
	uint64 frame = 0;
	uint64 failedFrames = 0;
	uint64 unmatchedViews = 0;
};

RenderCoreHostState &Host()
{
	static RenderCoreHostState s_State;
	return s_State;
}

} // namespace

DLL_EXPORT bool Engine_BindRenderCore( const RenderCoreBinding *pBinding )
{
	RenderCoreHostState &host = Host();
	if ( host.bound )
	{
		Warning( "Engine_BindRenderCore: a render core is already bound.\n" );
		return false;
	}
	if ( !pBinding || !pBinding->renderer || !pBinding->sceneFactory.create )
	{
		Warning( "Engine_BindRenderCore: the binding is incomplete.\n" );
		return false;
	}
	host.renderer = pBinding->renderer;
	host.sceneFactory = pBinding->sceneFactory;
	host.deviceName = pBinding->deviceName;
	host.capabilities = pBinding->capabilities;
	host.bound = true;
	return true;
}

bool RenderCoreHost_IsBound()
{
	return Host().bound;
}

world_mesh_gpu::IWorldMeshUpload *RenderCoreHost_WorldMeshUpload()
{
	RenderCoreHostState &host = Host();
	return host.capabilities ? host.capabilities->WorldMeshUpload() : nullptr;
}

light_set::ILightSetConsumer *RenderCoreHost_LightSetConsumer()
{
	RenderCoreHostState &host = Host();
	return host.capabilities ? host.capabilities->LightSetConsumer() : nullptr;
}

gpu_compute::IGpuCompute *RenderCoreHost_GpuCompute()
{
	RenderCoreHostState &host = Host();
	return host.capabilities ? host.capabilities->GpuCompute() : nullptr;
}

void RenderCoreHost_BeginFrame()
{
	RenderCoreHostState &host = Host();
	if ( !host.bound || host.inFrame )
		return;
	render::frame::FrameDesc desc;
	desc.frame = ++host.frame;
	desc.width = videomode ? (uint32)MAX( 1, videomode->GetModeWidth() ) : 1u;
	desc.height = videomode ? (uint32)MAX( 1, videomode->GetModeHeight() ) : 1u;
	host.inFrame = host.renderer->BeginFrame( desc ).HasValue();
}

void RenderCoreHost_EndFrame()
{
	RenderCoreHostState &host = Host();
	if ( !host.bound || !host.inFrame )
		return;
	host.inFrame = false;
	auto result = host.renderer->EndFrame();
	if ( !result && host.failedFrames++ == 0 )
		Warning( "Render core: frame %llu failed (status %u).\n", (unsigned long long)host.frame,
		    (unsigned int)result.Error().status );
}

void RenderCoreHost_MarkViewBegin()
{
	RenderCoreHostState &host = Host();
	if ( host.bound && host.inFrame )
		host.renderer->MarkStage( render::frame::Stage::kViewBegin );
}

void RenderCoreHost_MarkViewEnd()
{
	RenderCoreHostState &host = Host();
	if ( host.bound && host.inFrame )
		host.renderer->MarkStage( render::frame::Stage::kViewEnd );
}

void RenderCoreHost_LevelInit()
{
	RenderCoreHostState &host = Host();
	if ( host.bound )
		host.worldScene = host.sceneFactory.create();
}

void RenderCoreHost_LevelShutdown()
{
	Host().worldScene.reset();
}

CON_COMMAND( r_core_stats, "Prints what the render core ran (RFC 0016)." )
{
	RenderCoreHostState &host = Host();
	if ( !host.bound )
	{
		Msg( "render core: not bound\n" );
		return;
	}
	const render::frame::FrameStats &totals = host.renderer->Totals();
	Msg( "render core: device %s frames %llu failed %llu views %llu stages %llu "
	     "order-violations %llu passes %llu culled %llu transitions %llu transients %llu "
	     "reused %llu world-scene %s\n",
	    host.deviceName ? host.deviceName : "?", (unsigned long long)totals.frames,
	    (unsigned long long)totals.failedFrames, (unsigned long long)totals.views,
	    (unsigned long long)totals.stagesMarked, (unsigned long long)totals.orderViolations,
	    (unsigned long long)totals.passes, (unsigned long long)totals.culledPasses,
	    (unsigned long long)totals.transitions, (unsigned long long)totals.transientsCreated,
	    (unsigned long long)totals.transientsReused, host.worldScene ? "live" : "none" );
}
