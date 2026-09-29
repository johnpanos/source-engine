//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The engine's side of the render core; see render_core_host.h.
//
//=============================================================================//

#include "render_core_host.h"

#include "render_core_world.h"

#include "engine/render_core_binding.h"
#include "render/composition/render_core.h"
#include "render/frame/renderer.h"
#include "render/scene/scene.h"
#include "ivideomode.h"
#include "tier0/dbg.h"
#include "tier1/convar.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

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
	std::vector<int> worldLeaves; // snapshot index -> leaf, or -(prop + 1) (RFC 0016 K5)
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
	RenderCoreWorld_BeginFrame();
}

void RenderCoreHost_EndFrame()
{
	RenderCoreHostState &host = Host();
	if ( !host.bound || !host.inFrame )
		return;
	host.inFrame = false;
	RenderCoreWorld_EndFrame();
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
	if ( !host.bound )
		return;
	host.worldScene = host.sceneFactory.create();
	host.worldLeaves.clear();
	if ( host.worldScene )
		RenderCoreWorld_LevelInit();
}

void RenderCoreHost_LevelShutdown()
{
	Host().worldLeaves.clear();
	Host().worldScene.reset();
}

bool RenderCoreHost_SetWorldInstances( const float *pBoxes, const int *pCodes, int nCount )
{
	RenderCoreHostState &host = Host();
	if ( !host.worldScene || nCount <= 0 )
		return false;
	render::scene::ChangeSet changes;
	std::vector<int> leaves;
	for ( int i = 0; i < nCount; ++i )
	{
		const float *box = pBoxes + i * 6;
		render::scene::MeshInstanceDesc desc;
		desc.localBounds = { { box[0], box[1], box[2] }, { box[3], box[4], box[5] } };
		changes.Add( host.worldScene->Reserve(), desc );
		leaves.push_back( pCodes[i] );
	}
	if ( !host.worldScene->Commit( changes ) )
		return false;
	// Instances commit in order, so snapshot index i has code leaves[i].
	host.worldLeaves = std::move( leaves );
	return true;
}

int RenderCoreHost_WorldInstanceCount()
{
	return int( Host().worldLeaves.size() );
}

namespace
{

// The legacy view's visibility (render.visibility.v1): the BSP traversal's
// visible leaves and the client's drawn props for the view. It only removes.
class LegacyWorldVisibility final : public render::scene::IVisibilityProvider
{
public:
	LegacyWorldVisibility( const std::vector<int> &instanceCode, const unsigned char *pLeaf,
	    int nLeafCount, const unsigned char *pProp, int nPropCount )
	    : m_InstanceCode( instanceCode ), m_Leaf( pLeaf ), m_LeafCount( nLeafCount ),
	      m_Prop( pProp ), m_PropCount( nPropCount )
	{
	}
	void Filter( const render::scene::SceneSnapshot &, const render::scene::SceneView &,
	    std::vector<std::uint32_t> &candidates ) override
	{
		std::erase_if( candidates,
		    [this]( std::uint32_t index )
		    {
			    if ( index >= m_InstanceCode.size() )
				    return true;
			    const int code = m_InstanceCode[index];
			    if ( code >= 0 )
				    return code >= m_LeafCount || !m_Leaf[code];
			    const int prop = -code - 1;
			    return prop >= m_PropCount || !m_Prop[prop];
		    } );
	}

private:
	const std::vector<int> &m_InstanceCode;
	const unsigned char *m_Leaf;
	int m_LeafCount;
	const unsigned char *m_Prop;
	int m_PropCount;
};

} // namespace

bool RenderCoreHost_CullWorld( const float *pPlanes, int nPlanes, const unsigned char *pVisibleLeaf,
    int nLeafCount, const unsigned char *pVisibleProp, int nPropCount, int *pDrawnCodes,
    int *pDrawnCount, int *pFrustumCulled, int *pProviderCulled, int *pPooledEqual )
{
	RenderCoreHostState &host = Host();
	*pDrawnCount = 0;
	if ( !host.worldScene || host.worldLeaves.empty() || !host.sceneFactory.makeView ||
	     !host.sceneFactory.buildDrawList )
		return false;
	render::scene::ViewDesc desc;
	desc.viewBit = 32; // every instance
	render::math::Frustum frustum;
	for ( render::math::Plane &plane : frustum.planes )
		plane = { { 0.0f, 0.0f, 0.0f }, 1.0f }; // open
	const int count = MIN( nPlanes, int( std::size( frustum.planes ) ) );
	for ( int i = 0; i < count; ++i )
	{
		const float *plane = pPlanes + i * 4;
		frustum.planes[i] = { { plane[0], plane[1], plane[2] }, -plane[3] };
	}
	desc.frustum = frustum;
	const render::scene::SceneView view = host.sceneFactory.makeView( desc );
	LegacyWorldVisibility provider(
	    host.worldLeaves, pVisibleLeaf, nLeafCount, pVisibleProp, nPropCount );
	const auto snapshot = host.worldScene->Snapshot();
	const render::scene::DrawList list =
	    host.sceneFactory.buildDrawList( *snapshot, view, &provider );
	for ( const render::scene::DrawItem &item : list.items )
		pDrawnCodes[( *pDrawnCount )++] = host.worldLeaves[item.instance];
	*pFrustumCulled = int( list.frustumCulled );
	*pProviderCulled = int( list.providerCulled );
	*pPooledEqual = -1;
	if ( host.sceneFactory.buildDrawListPooled )
	{
		auto pooled = host.sceneFactory.buildDrawListPooled(
		    host.sceneFactory.context, *snapshot, view, &provider );
		bool equal = pooled.HasValue() && pooled.Value().items.size() == list.items.size() &&
		             pooled.Value().frustumCulled == list.frustumCulled &&
		             pooled.Value().providerCulled == list.providerCulled;
		for ( std::size_t i = 0; equal && i < list.items.size(); ++i )
		{
			const render::scene::DrawItem &a = list.items[i];
			const render::scene::DrawItem &b = pooled.Value().items[i];
			equal = a.instance == b.instance && a.material == b.material && a.mesh == b.mesh &&
			        a.depth == b.depth;
		}
		*pPooledEqual = equal ? 1 : 0;
	}
	return true;
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
