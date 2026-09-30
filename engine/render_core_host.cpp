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
#include <cstring>
#include <iterator>
#include <string>
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
	IRenderCoreWorld *world = nullptr;
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

// RFC 0014: the render core's debug controls. They are read once per frame
// into the FrameDesc (DebugControlsFromConVars); no shader, pass or engine
// module reads them otherwise. The renderer validates the frame's value and
// keeps the last valid one when a new one is refused.
void DebugProgramChanged( IConVar *var, const char *, float );

ConVar cl_render_debug_view( "cl_render_debug_view", "0", FCVAR_CHEAT,
    "Render core debug view (RFC 0014): 0 off; 1 albedo, 2 world normal, 3 normal map, "
    "4 roughness, 5 filtered roughness, 6 metalness, 7 AO, 8 baked light, 9 direct light, "
    "10 image specular, 11 SSR, 12 emission, 13 UV checker, 14 vertex color, 15 linear depth, "
    "16 NaN/Inf/negative, 17 over-range. What the core does not draw shows a grey hatch." );
ConVar cl_render_debug_view_program( "cl_render_debug_view_program", "", FCVAR_CHEAT,
    "Applies the debug view, BRDF mode and overrides to one core program; others draw flat "
    "grey. ? lists the programs.",
    DebugProgramChanged );
ConVar cl_render_debug_view_scale(
    "cl_render_debug_view_scale", "1", FCVAR_CHEAT, "Linear exposure of the radiometric views." );
ConVar cl_render_debug_view_range( "cl_render_debug_view_range", "4096", FCVAR_CHEAT,
    "Divisor of the linear depth view, in units." );
ConVar cl_render_debug_view_threshold( "cl_render_debug_view_threshold", "1", FCVAR_CHEAT,
    "Luminance threshold of the over-range view." );
ConVar cl_render_debug_brdf( "cl_render_debug_brdf", "0", FCVAR_CHEAT,
    "0 full; 1 diffuse lobe only; 2 specular lobe only; 3 energy compensation off; 4 the "
    "split-sum table sample." );
ConVar cl_render_debug_furnace( "cl_render_debug_furnace", "0", FCVAR_CHEAT,
    "Albedo 1 in a uniform environment of radiance 1; direct lights off." );
ConVar cl_render_debug_term( "cl_render_debug_term", "", FCVAR_CHEAT,
    "Comma-separated lighting-model terms to turn off: clustered, sun, area, projected, baked, "
    "probes, ibl, ssr, ao, specular_occlusion, emission, volumetric." );
ConVar cl_render_debug_force_roughness( "cl_render_debug_force_roughness", "-1", FCVAR_CHEAT,
    "-1 off; otherwise the roughness every material takes." );
ConVar cl_render_debug_force_metalness( "cl_render_debug_force_metalness", "-1", FCVAR_CHEAT,
    "-1 off; otherwise the metalness every material takes." );
ConVar cl_render_debug_legacy( "cl_render_debug_legacy", "0", FCVAR_CHEAT,
    "0 off; 2 skips the legacy stream, leaving the grey hatch where the core draws nothing." );

void DebugProgramChanged( IConVar *var, const char *, float )
{
	ConVarRef program( var );
	if ( std::strcmp( program.GetString(), "?" ) != 0 )
		return;
	RenderCoreHostState &host = Host();
	if ( host.renderer )
	{
		Msg( "Render core programs:" );
		for ( std::size_t i = 0; i < host.renderer->DebugProgramCount(); ++i )
			Msg( " %s", host.renderer->DebugProgramName( i ) );
		Msg( "\n" );
	}
	program.SetValue( "" );
}

render::frame::DebugControls DebugControlsFromConVars( const RenderCoreHostState &host )
{
	render::frame::DebugControls debug;
	debug.view = (uint32)cl_render_debug_view.GetInt();
	V_strncpy( debug.program, cl_render_debug_view_program.GetString(), sizeof( debug.program ) );
	if ( V_strlen( cl_render_debug_view_program.GetString() ) >= (int)sizeof( debug.program ) )
		std::memset( debug.program, 'x', sizeof( debug.program ) ); // refused as unterminated
	debug.viewScale = cl_render_debug_view_scale.GetFloat();
	debug.viewRange = cl_render_debug_view_range.GetFloat();
	debug.viewThreshold = cl_render_debug_view_threshold.GetFloat();
	debug.brdf = (uint32)cl_render_debug_brdf.GetInt();
	debug.furnace = cl_render_debug_furnace.GetBool();
	char unknown[64];
	uint32 terms = 0;
	if ( !host.renderer->ParseDebugTerms(
	         cl_render_debug_term.GetString(), &terms, unknown, sizeof( unknown ) ) )
	{
		static std::string s_Reported;
		if ( s_Reported != cl_render_debug_term.GetString() )
		{
			Warning(
			    "cl_render_debug_term: %s is not a term; the frame keeps its terms.\n", unknown );
			s_Reported = cl_render_debug_term.GetString();
		}
		terms = ~0u; // no term: the renderer refuses the value and keeps the last
	}
	debug.termsOff = terms;
	debug.forceRoughness = cl_render_debug_force_roughness.GetFloat();
	debug.forceMetalness = cl_render_debug_force_metalness.GetFloat();
	debug.legacy = render::frame::DebugLegacy( (uint32)cl_render_debug_legacy.GetInt() );
	return debug;
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
	host.world = pBinding->world;
	host.bound = true;
	return true;
}

bool RenderCoreHost_IsBound()
{
	return Host().bound;
}

IRenderCoreWorld *RenderCoreHost_World()
{
	return Host().bound ? Host().world : nullptr;
}

// RFC 0016 K12: each world mesh upload the renderer's provider accepts
// reaches the render core's world stage too (IRenderCoreWorld::StageUpload),
// so the core draws a BSP2 map's world from the same lightmap, probes and
// reflection probes, and follows their changes. Drawing stays the provider's.
class CWorldMeshUploadTee final : public world_mesh_gpu::IWorldMeshUpload
{
public:
	world_mesh_gpu::IWorldMeshUpload *m_pProvider = nullptr;
	world_mesh_gpu::IWorldMeshUpload *m_pStage = nullptr;

	bool Upload( const world_mesh_gpu::WorldMeshUploadRequest &request ) override
	{
		return m_pProvider->Upload( request );
	}
	bool UploadLightmap( const world_mesh_gpu::WorldLightmapUploadRequest &request ) override
	{
		const bool accepted = m_pProvider->UploadLightmap( request );
		if ( accepted )
			m_pStage->UploadLightmap( request );
		return accepted;
	}
	bool UploadProbeVolume( const world_mesh_gpu::ProbeVolumeUploadRequest &request ) override
	{
		const bool accepted = m_pProvider->UploadProbeVolume( request );
		if ( accepted )
			m_pStage->UploadProbeVolume( request );
		return accepted;
	}
	bool UploadShadowField( const world_mesh_gpu::ShadowFieldUploadRequest &request ) override
	{
		return m_pProvider->UploadShadowField( request );
	}
	bool UploadReflectionProbes(
	    const world_mesh_gpu::ReflectionProbesUploadRequest &request ) override
	{
		const bool accepted = m_pProvider->UploadReflectionProbes( request );
		if ( accepted )
			m_pStage->UploadReflectionProbes( request );
		return accepted;
	}
	bool DrawBatch( uint32_t firstIndex, uint32_t indexCount ) override
	{
		return m_pProvider->DrawBatch( firstIndex, indexCount );
	}
	void Release() override
	{
		m_pProvider->Release();
		m_pStage->Release();
	}
	bool IsResident() const override { return m_pProvider->IsResident(); }
};

world_mesh_gpu::IWorldMeshUpload *RenderCoreHost_WorldMeshUpload()
{
	RenderCoreHostState &host = Host();
	world_mesh_gpu::IWorldMeshUpload *pProvider =
	    host.capabilities ? host.capabilities->WorldMeshUpload() : nullptr;
	world_mesh_gpu::IWorldMeshUpload *pStage =
	    pProvider && host.bound && host.world ? host.world->StageUpload() : nullptr;
	if ( !pStage )
		return pProvider;
	static CWorldMeshUploadTee s_Tee;
	s_Tee.m_pProvider = pProvider;
	s_Tee.m_pStage = pStage;
	return &s_Tee;
}

// RFC 0016 K12: the frame's light set reaches the renderer's consumer and the
// render core's world stage, which lights the world it draws with it.
class CLightSetTee final : public light_set::ILightSetConsumer
{
public:
	light_set::ILightSetConsumer *m_pConsumer = nullptr;
	light_set::ILightSetConsumer *m_pStage = nullptr;

	void PublishLightSet( const light_set::Snapshot &snapshot ) override
	{
		if ( m_pConsumer )
			m_pConsumer->PublishLightSet( snapshot );
		m_pStage->PublishLightSet( snapshot );
	}
};

light_set::ILightSetConsumer *RenderCoreHost_LightSetConsumer()
{
	RenderCoreHostState &host = Host();
	light_set::ILightSetConsumer *pConsumer =
	    host.capabilities ? host.capabilities->LightSetConsumer() : nullptr;
	light_set::ILightSetConsumer *pStage =
	    host.bound && host.world ? host.world->StageLights() : nullptr;
	if ( !pStage )
		return pConsumer;
	static CLightSetTee s_Tee;
	s_Tee.m_pConsumer = pConsumer;
	s_Tee.m_pStage = pStage;
	return &s_Tee;
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
	desc.debug = DebugControlsFromConVars( host );
	const uint64 refusedBefore = host.renderer->Totals().debugRejected;
	host.inFrame = host.renderer->BeginFrame( desc ).HasValue();
	if ( host.inFrame )
	{
		// A refused debug value prints why, once per reason.
		static std::string s_Reported;
		if ( host.renderer->Totals().debugRejected != refusedBefore )
		{
			const char *why = host.renderer->LastDebugRejection().message;
			if ( s_Reported != why )
			{
				Warning( "Render core: %s; the frame keeps its debug controls.\n", why );
				s_Reported = why;
			}
		}
		else
			s_Reported.clear();
		// Under a pixel view the core draws the frame alone from its first
		// slot (RFC 0014).
		if ( host.world )
			host.world->BeginFrame();
	}
	RenderCoreWorld_BeginFrame();
}

void RenderCoreHost_MarkFrameEnd()
{
	RenderCoreHostState &host = Host();
	if ( host.bound && host.inFrame && host.world )
		host.world->EndFrame();
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
