//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The engine's side of the render core; see render_core_host.h.
//
//=============================================================================//

#include "render_core_host.h"
#include "dynamic_occlusion.h"

#include "render_core_world.h"
#include "filesystem.h"
#include "filesystem_engine.h"

#include "engine/render_core_binding.h"
#include "render/composition/render_core.h"
#include "render/frame/renderer.h"
#include "render/legacy/temporal_views.h"
#include "render/scene/scene.h"
#include "ivideomode.h"
#include "tier0/dbg.h"
#include "tier1/convar.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
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
	RenderCoreBinding::TemporalControl temporal;
	IRenderCorePanels *panels = nullptr;
	gpu_compute::IGpuCompute *gpuCompute = nullptr; // the core's (RFC 0016 K12)
	std::unique_ptr<render::scene::IRenderScene> worldScene;
	std::vector<int> worldLeaves; // snapshot index -> leaf, or -(prop + 1) (RFC 0016 K5)
	bool inFrame = false;
	uint64 frame = 0;
	uint64 failedFrames = 0;
	uint64 unmatchedViews = 0;
	float temporalScale = 0.0f;
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
    "probes, ibl, ssr, ao, specular_occlusion, emission, volumetric, shadow_visibility, "
    "directional, normal_map, bounce, soft_shadows." );
ConVar cl_render_debug_force_roughness( "cl_render_debug_force_roughness", "-1", FCVAR_CHEAT,
    "-1 off; otherwise the roughness every material takes." );
ConVar cl_render_debug_force_metalness( "cl_render_debug_force_metalness", "-1", FCVAR_CHEAT,
    "-1 off; otherwise the metalness every material takes." );
ConVar cl_render_debug_legacy( "cl_render_debug_legacy", "0", FCVAR_CHEAT,
    "0 off; 2 skips the legacy stream, leaving the grey hatch where the core draws nothing." );
// RFC 0014 D4: GPU timers around every labeled core section, reported by
// cl_render_debug_stats once a second (per-frame means over that second).
ConVar cl_render_debug_gpu_timers( "cl_render_debug_gpu_timers", "0", 0,
    "Time the render core's GPU passes (RFC 0014 D4); cl_render_debug_stats prints them." );
ConVar cl_render_debug_cost( "cl_render_debug_cost", "0", FCVAR_CHEAT,
    "Overlay measured render-core CPU recording and GPU pass costs. Inclusive labeled sections; "
    "excludes game CPU, legacy rendering and present. 0 off, 1 on.", true, 0, true, 1 );
ConVar cl_render_debug_stats( "cl_render_debug_stats", "0", 0,
    "Print the render core's per-pass GPU times (cl_render_debug_gpu_timers) every second." );
// The render core's quality settings (RFC 0016 K12; the video options).
ConVar r_temporal_scale( "r_temporal_scale", "0", FCVAR_ARCHIVE,
    "FSR: 0 off, 1 native AA, 0.666667 quality, 0.588235 balanced, 0.5 performance. "
    "Requires an FSR-capable session.",
    true, 0.0f, true, 1.0f );
ConVar r_core_dynamic_draws( "r_core_dynamic_draws", "0", FCVAR_CHEAT,
    "Experimental rendercore dynamic-material handoff. Explicit opt-in only; "
    "whole-cohort queued rendering and image acceptance are incomplete." );
ConVar r_core_ao_quality( "r_core_ao_quality", "0", FCVAR_ARCHIVE,
    "Render core ambient occlusion (GTAO): 0 off (the default: Source 2's lighting, compiled out), "
    "1 low, 2 medium, 3 high, 4 ultra.",
    true, 0, true, 4 );
ConVar r_core_depth_prepass( "r_core_depth_prepass", "1", FCVAR_ARCHIVE,
    "Render core: draw the world's opaque depth before lighting it, so each pixel is shaded "
    "once (0 lights every fragment the depth test passes)." );
ConVar r_core_world_gpu_submit( "r_core_world_gpu_submit", "0", FCVAR_ARCHIVE,
    "Render core: cull the world's surfaces on the GPU and draw them with one indirect draw per "
    "material and lightmap page (RFC 0016 S3/S4; 0 draws them per surface; 2 also occlusion-culls "
    "them against the view's world depth)." );
ConVar r_core_shadow_movers( "r_core_shadow_movers", "1", FCVAR_ARCHIVE,
    "Render core: moving objects cast shadows over the cached static shadow tiles." );
ConVar r_core_shadow_pcss( "r_core_shadow_pcss", "1", FCVAR_ARCHIVE,
    "Render core: soft shadows (PCSS: penumbrae sized by each light's emitter). 0 gives every "
    "runtime shadow the hard 2x2 filter, which is cheaper (Advanced Video: Soft Shadows "
    "(PCSS)). Static lights with baked shadow masks use their masks either way." );
ConVar r_core_area_lights( "r_core_area_lights", "0", FCVAR_ARCHIVE,
    "Render core: runtime area lights (LTC) for the map's light fixtures and the frame's "
    "emitting surfaces. 0 (default, Source 2's lighting): fixtures light through the bake and "
    "reflect through the probes." );
ConVar r_core_runtime_direct( "r_core_runtime_direct", "1", FCVAR_ARCHIVE,
    "Render core: the world's lightmap is its indirect layer and every light's direct light is "
    "drawn at runtime, shadowed, so moving objects block it (0: the bake's total layer, with "
    "every light's direct light and shadows baked; the Low profile's choice). Applies at the "
    "next frame." );
ConVar r_core_ssr( "r_core_ssr", "0", FCVAR_ARCHIVE,
    "Render core: screen-space reflections over a stage view's glossy surfaces "
    "(render.ssr.v1; 0, the default: the probes alone, Source 2's lighting)." );
ConVar r_core_volumetric( "r_core_volumetric", "1", FCVAR_ARCHIVE,
    "Render core: composite the map's participating media (env_volumetric_fog_volume and "
    "env_volumetric_fog_controller) over its views (0: the term is left out)." );
ConVar r_core_shadow_quality( "r_core_shadow_quality", "2", FCVAR_ARCHIVE,
    "Render core shadows: 0 off, 1 low (2048 atlas), 2 medium (4096), 3 high (8192).", true, 0,
    true, 3 );

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
	debug.costOverlay = cl_render_debug_cost.GetBool();
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

// The game's files for the core (RenderCoreFileSource): the GAME search
// path, VPKs included.
unsigned long long CoreFileSize( const char *path )
{
	FileHandle_t file = g_pFileSystem->Open( path, "rb", "GAME" );
	if ( !file )
		return 0;
	const unsigned long long size = g_pFileSystem->Size( file );
	g_pFileSystem->Close( file );
	return size;
}

bool CoreFileRead( const char *path, void *out, unsigned long long bytes )
{
	FileHandle_t file = g_pFileSystem->Open( path, "rb", "GAME" );
	if ( !file )
		return false;
	const bool read = g_pFileSystem->Size( file ) == bytes &&
	                  g_pFileSystem->Read( out, int( bytes ), file ) == int( bytes );
	g_pFileSystem->Close( file );
	return read;
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
	if ( host.world )
		host.world->SetFileSource( { &CoreFileSize, &CoreFileRead } );
	host.temporal = pBinding->temporal;
	host.panels = pBinding->panels;
	host.gpuCompute = pBinding->gpuCompute;
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

IRenderCorePanels *RenderCoreHost_Panels()
{
	return Host().bound ? Host().panels : nullptr;
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
		// The core's stage reads the total and indirect layers from the full
		// request. The native backend reads the separated layers only for the
		// RuntimeIndirect policy (r_indirect_policy 2) and the indirect debug
		// views (mat_indirect_view 1 and 2); with neither it keeps the total
		// layer alone, so the map's light is not held twice. Both are cheat
		// cvars read at upload: change them, then reload the map.
		world_mesh_gpu::WorldLightmapUploadRequest providerRequest = request;
		static ConVarRef r_indirect_policy( "r_indirect_policy" );
		static ConVarRef mat_indirect_view( "mat_indirect_view" );
		const bool separatedWanted =
		    ( r_indirect_policy.IsValid() && r_indirect_policy.GetInt() == 2 ) ||
		    ( mat_indirect_view.IsValid() &&
		        ( mat_indirect_view.GetInt() == 1 || mat_indirect_view.GetInt() == 2 ) );
		if ( !request.regions && !separatedWanted && m_pStage )
		{
			providerRequest.layerCount = 0;
			for ( uint32_t i = 0; i < request.layerCount; ++i )
			{
				if ( request.roles[i] != world_mesh_gpu::WorldLightmapRole::Total )
					continue;
				providerRequest.layers[providerRequest.layerCount] = request.layers[i];
				providerRequest.roles[providerRequest.layerCount++] = request.roles[i];
			}
			if ( providerRequest.layerCount < request.layerCount )
				Msg( "Render core: LMAP direct/indirect layers kept out of the native backend "
				     "(no consumer; r_indirect_policy 2 or mat_indirect_view 1/2 before load "
				     "keeps them)\n" );
		}
		const bool accepted = m_pProvider->UploadLightmap( providerRequest );
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
		// Both take the probes on their own terms: the frozen backend may
		// decline RPRB v8 (cube arrays) while the core draws them, so the core
		// gets them whatever the backend answers. Accepted when either took them.
		const bool provider = m_pProvider->UploadReflectionProbes( request );
		const bool core = m_pStage->UploadReflectionProbes( request );
		return provider || core;
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
		light_set::Snapshot core = snapshot;
		core.coreTriangles = DynamicOcclusion_CoreTriangles();
		m_pStage->PublishLightSet( core );
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
	// RFC 0016 K12: the producers' compute runs on the core when it offers it.
	RenderCoreHostState &host = Host();
	if ( host.bound && host.gpuCompute )
		return host.gpuCompute;
	return host.capabilities ? host.capabilities->GpuCompute() : nullptr;
}

gpu_compute::IGpuCompute *RenderCoreHost_RayQueryGpuCompute()
{
	RenderCoreHostState &host = Host();
	return host.capabilities ? host.capabilities->GpuCompute() : nullptr;
}

void RenderCoreHost_BeginFrame()
{
	RenderCoreHostState &host = Host();
	if ( !host.bound || host.inFrame )
		return;
	const float temporalScale = r_temporal_scale.GetFloat();
	if ( host.world && std::isfinite( temporalScale ) &&
	     ( temporalScale == 0.0f || ( temporalScale >= 0.5f && temporalScale <= 1.0f ) ) &&
	     temporalScale != host.temporalScale )
	{
		const bool enableChanged = host.world->TemporalEnabled() != ( temporalScale != 0.0f );
		if ( host.temporal.setEnabled &&
		     host.temporal.setEnabled( host.temporal.context, temporalScale != 0.0f ) )
		{
			if ( !enableChanged )
				host.world->ResetTemporalHistory();
			host.temporalScale = temporalScale;
		}
	}
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
		{
			const RenderCoreWorldQuality quality{ r_core_ao_quality.GetInt(),
			    r_core_shadow_quality.GetInt(), r_core_depth_prepass.GetInt(),
			    r_core_shadow_movers.GetInt(), r_core_shadow_pcss.GetInt(),
			    r_core_runtime_direct.GetInt(),
			    RenderCoreWorldDraw_OnlyCore(), r_core_dynamic_draws.GetBool(),
			    r_core_volumetric.GetInt(), r_core_ssr.GetInt(),
			    r_core_world_gpu_submit.GetInt(), r_core_area_lights.GetInt() };
			host.world->SetQuality( quality );
			host.world->BeginFrame();
		}
	}
	RenderCoreWorld_BeginFrame();
}

void RenderCoreHost_MarkFrameEnd()
{
	RenderCoreHostState &host = Host();
	if ( host.bound && host.inFrame && host.world )
		host.world->EndFrame();
}

bool RenderCoreHost_ReadCosts( RenderCoreCostReport *out )
{
	RenderCoreHostState &host = Host();
	if ( !host.bound || !host.world || !host.renderer || !host.renderer->AppliedDebug().costOverlay )
		return false;
	if ( out )
		host.world->ReadCosts( out );
	return true;
}

void RenderCoreHost_EndFrame()
{
	RenderCoreHostState &host = Host();
	if ( !host.bound || !host.inFrame )
		return;
	host.inFrame = false;
	RenderCoreWorld_EndFrame();
	if ( host.world )
	{
		host.world->SetGpuTimers( cl_render_debug_gpu_timers.GetBool() );

		static double s_LastStats = 0.0;
		const double now = Plat_FloatTime();
		if ( cl_render_debug_stats.GetBool() && cl_render_debug_gpu_timers.GetBool() &&
		     now - s_LastStats >= 1.0 )
		{
			s_LastStats = now;
			char times[4096];
			const unsigned int frames = host.world->TakeGpuTimes( times, sizeof( times ) );
			if ( frames )
			{
				// "depth ms count name" lines: indent by depth.
				Msg( "cl_render_debug_stats: core GPU passes, mean of %u frame(s):\n", frames );
				for ( char *line = times; *line; )
				{
					char *end = strchr( line, '\n' );
					if ( end )
						*end = '\0';
					unsigned int depth = 0;
					double ms = 0.0, count = 0.0;
					int name = 0;
					if ( sscanf( line, "%u %lf %lf %n", &depth, &ms, &count, &name ) >= 3 )
						Msg( "  %*s%-*s %7.3f ms  x%.1f\n", int( depth * 2 ), "",
						    32 - int( depth * 2 ), line + name, ms, count );
					if ( !end )
						break;
					line = end + 1;
				}
			}
			else
			{
				Msg( "cl_render_debug_stats: no core GPU pass timed yet (a device without "
				     "timestamps, or no core view drawn).\n" );
			}
		}
	}
	auto result = host.renderer->EndFrame();
	if ( host.world )
		host.world->CommitTemporalFrame( result.HasValue() );
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

CON_COMMAND_F( r_temporal_capture,
    "Capture game FSR inputs/output: <unique file prefix> [reset waits for next history reset]",
    FCVAR_CHEAT )
{
	const bool afterReset = args.ArgC() == 3 && !Q_strcmp( args[2], "reset" );
	if ( ( args.ArgC() != 2 && !afterReset ) || !Host().world ||
	     !Host().world->CaptureTemporalInputs( args[1], afterReset ) )
		Warning( "r_temporal_capture: needs an FSR session, a unique file prefix and optional "
		         "reset; one request at a time\n" );
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

bool RenderCoreHost_TemporalJitter( float *x, float *y )
{
	*x = *y = 0;
	if ( !Host().world || !Host().world->TemporalEnabled() )
		return false;
	Host().world->TemporalJitter( x, y );
	return true;
}

// Separate optional interface; does not alter IVEngineClient's preserved ABI.
namespace
{
class TemporalViews final : public IRenderTemporalViews2
{
public:
	void SelectView( unsigned long long identity ) override
	{
		if ( Host().world )
			Host().world->SelectTemporalView( identity );
	}
	void ResetHistory() override
	{
		if ( Host().world )
			Host().world->ResetTemporalHistory();
	}
	bool Enabled() const override { return Host().world && Host().world->TemporalEnabled(); }
	bool Available() const override
	{
		const auto &control = Host().temporal;
		return control.available && control.available( control.context );
	}
	float RenderScale() const override { return Host().temporalScale; }
	bool Reconstruct( int x, int y, int rw, int rh, int ow, int oh, float dt ) override
	{
		return Host().world && Host().world->ReconstructTemporal( x, y, rw, rh, ow, oh, dt );
	}
};
void *TemporalViewsFactory()
{
	static TemporalViews adapter;
	return &adapter;
}
}
EXPOSE_INTERFACE_FN(
    TemporalViewsFactory, IRenderTemporalViews, RENDER_TEMPORAL_VIEWS_INTERFACE_VERSION );
EXPOSE_INTERFACE_FN(
    TemporalViewsFactory, IRenderTemporalViews2, RENDER_TEMPORAL_VIEWS2_INTERFACE_VERSION );
