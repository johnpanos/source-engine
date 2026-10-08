//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): statistics, quality, GPU timers and costs.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

void CoreWorld::GetStats( RenderCoreWorldStats *out ) const
{
	if ( !out )
		return;
	const pass::world::WorldStats stats = m_Pass.Stats();
	*out = RenderCoreWorldStats{};
	out->materials = stats.materials;
	out->claimedMaterials = stats.claimedMaterials;
	out->surfaces = stats.surfaces;
	out->claimedSurfaces = stats.claimedSurfaces;
	out->viewsQueued = stats.viewsQueued;
	out->viewsDrawn = stats.viewsDrawn;
	out->viewsFailed = stats.viewsFailed;
	out->viewsSkipped = stats.viewsSkipped;
	out->surfacesDrawn = stats.surfacesDrawn;
	out->staticInstancesQueued = stats.staticInstancesQueued;
	out->staticDrawsDrawn = stats.staticDrawsDrawn;
	out->prepassListBuilds = stats.prepassListBuilds;
	out->prepassListReuses = stats.prepassListReuses;
	out->prepassListRetries = stats.prepassListRetries;
	out->posedModelsQueued = stats.posedModelsQueued;
	out->posedDrawsDrawn = stats.posedDrawsDrawn;
	// Model geometry: the published blocks, the CPU geometry they share and
	// the per-frame skinning copies the host's posed models need.
	for ( std::size_t i = 0; i < m_StaticMeshes.size(); ++i )
	{
		const pass::world::WorldData::StaticMesh &mesh = m_StaticMeshes[i];
		out->modelLevels += unsigned( mesh.lodCount() );
		for ( const pass::world::WorldData::StaticMeshLod &level : mesh.lods )
		{
			// One allocation per block, shared with the world the pass holds.
			out->modelStagingBytes +=
			    std::uint64_t( level.vertexCount ) * sizeof( material::SurfaceModelVertex ) +
			    std::uint64_t( level.indexCount ) * sizeof( std::uint32_t );
		}
		if ( i >= m_ModelPoseSources.size() || m_ModelPoseSources[i].poseToBone.empty() )
			continue;
		++out->modelPoseSources;
		for ( const std::vector<pass::skinning::SkinVertex> &level :
		    m_ModelPoseSources[i].vertices )
			out->modelPoseBytes +=
			    std::uint64_t( level.size() ) * sizeof( pass::skinning::SkinVertex );
	}
	out->dynamicDrawsDrawn = stats.dynamicDrawsDrawn;
	out->dynamicDrawsRefused = stats.dynamicDrawsRefused;
	out->gpuViews = stats.gpuViews;
	out->gpuIndirectDraws = stats.gpuIndirectDraws;
	out->gpuFallbacks = stats.gpuFallbacks;
	out->gpuOcclusionViews = stats.gpuOcclusionViews;
	out->gpuPyramids = stats.gpuPyramids;
	out->gpuPrepassViews = stats.gpuPrepassViews;
	out->gpuModelViews = stats.gpuModelViews;
	std::snprintf( out->lastRefusal, sizeof( out->lastRefusal ), "%s", stats.lastRefusal.c_str() );
	out->debugHatches = m_Hatches.load( std::memory_order_relaxed );
	out->debugTints = m_Tints.load( std::memory_order_relaxed );
	out->debugViewsRedrawn = m_Redrawn.load( std::memory_order_relaxed );
	out->stageLights =
	    unsigned( pass::lights::MergeMapLights( m_Lights.lights, m_MapLights ).size() );
	out->stageLitViews = m_StageLitViews;
	out->stageLightingBuilds = m_StageLightingBuilds.load( std::memory_order_relaxed );
	out->stageSharedViews = m_SharedStageViews.load( std::memory_order_relaxed );
	out->stageRuntimeDirect = m_StageRuntimeDirect.load( std::memory_order_relaxed ) ? 1u : 0u;
	out->volumetricMedium = m_Media ? 1u : 0u;
	out->volumetricViews = m_VolumetricViews.load( std::memory_order_relaxed );
	out->volumetricRefused = m_VolumetricRefused.load( std::memory_order_relaxed );
	out->projectorsLit = m_ProjectorsLit;
	out->projectorsRefused = m_ProjectorsRefused.load( std::memory_order_relaxed );
	out->cutoutShadowDraws = stats.cutoutShadowDraws;
	out->cutoutShadowRefused = stats.cutoutShadowRefused;
	out->cutoutShadowNotResident = stats.cutoutShadowNotResident;
	out->ssrViews = m_SsrViews.load( std::memory_order_relaxed );
	out->ssrRefused = m_SsrRefused.load( std::memory_order_relaxed );
	std::snprintf( out->lastFailure, sizeof( out->lastFailure ), "%s", stats.lastFailure.c_str() );
	std::size_t used = 0;
	for ( const auto &[reason, count] : stats.gaps )
	{
		if ( used + 1 >= sizeof( out->gaps ) )
			break;
		const int written = std::snprintf(
		    out->gaps + used, sizeof( out->gaps ) - used, "%u %s\n", count, reason.c_str() );
		if ( written < 0 )
			break;
		used = std::min( sizeof( out->gaps ) - 1, used + std::size_t( written ) );
	}
	used = 0;
	for ( const auto &[name, count] : stats.claimed )
	{
		if ( used + 1 >= sizeof( out->claimed ) )
			break;
		const int written = std::snprintf(
		    out->claimed + used, sizeof( out->claimed ) - used, "%u %s\n", count, name.c_str() );
		if ( written < 0 )
			break;
		used = std::min( sizeof( out->claimed ) - 1, used + std::size_t( written ) );
	}
}

void CoreWorld::SetQuality( const RenderCoreWorldQuality &quality )
{
	m_DynamicDraws.store( quality.dynamicDraws, std::memory_order_relaxed );
	m_CoreOnly = quality.coreOnly;
	m_AoQuality.store( std::clamp( quality.ambientOcclusion, 0, 4 ), std::memory_order_relaxed );
	m_ShadowQuality.store( std::clamp( quality.shadows, 0, 3 ), std::memory_order_relaxed );
	m_DepthPrepass.store( quality.depthPrepass != 0, std::memory_order_relaxed );
	m_GpuSubmission.store( quality.gpuSubmission, std::memory_order_relaxed );
	m_ShadowMovers.store( quality.shadowMovers != 0, std::memory_order_relaxed );
	m_ShadowPcss.store( quality.shadowPcss != 0, std::memory_order_relaxed );
	m_RuntimeDirect.store( quality.runtimeDirect != 0, std::memory_order_relaxed );
	m_VolumetricOn.store( quality.volumetric != 0, std::memory_order_relaxed );
	m_SsrOn.store( quality.ssr != 0, std::memory_order_relaxed );
	m_AreaLightsOn.store( quality.areaLights != 0, std::memory_order_relaxed );
	m_ProbeBounce.store( quality.probeBounce != 0, std::memory_order_relaxed );
	m_StageRuntimeDirect.store(
	    quality.runtimeDirect != 0 && m_StageHasIndirect.load( std::memory_order_relaxed ),
	    std::memory_order_relaxed );
}

void CoreWorld::SetGpuTimers( bool enabled )
{
	m_GpuTimersOn.store( enabled, std::memory_order_relaxed );
}

unsigned int CoreWorld::TakeGpuTimes( char *out, unsigned int size )
{
	if ( !out || size == 0 )
		return 0;
	out[0] = '\0';
	graph::PassTimerReport report;
	{
		std::lock_guard<std::mutex> guard( m_TimersLock );
		if ( !m_Timers )
			return 0;
		report = m_Timers->Take();
	}
	if ( report.frames == 0 )
		return 0;
	std::size_t used = 0;
	const double frames = double( report.frames );
	for ( const graph::PassTime &pass : report.passes )
	{
		if ( used + 1 >= size )
			break;
		const int written = std::snprintf( out + used, size - used, "%u %.3f %.2f %s\n", pass.depth,
		    pass.milliseconds / frames, double( pass.count ) / frames, pass.name.c_str() );
		if ( written < 0 )
			break;
		used = std::min( std::size_t( size ) - 1, used + std::size_t( written ) );
	}
	// The same views' CPU recording (shadows, prepass, lit world, GTAO).
	const std::uint64_t recordNs = m_RecordNs.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t recordViews = m_RecordViews.exchange( 0, std::memory_order_relaxed );
	if ( recordViews && used + 1 < size )
	{
		const int written = std::snprintf( out + used, size - used,
		    "0 %.3f %.2f core world view (CPU recording, render sequence)\n",
		    double( recordNs ) * 1e-6 / frames, double( recordViews ) / frames );
		if ( written > 0 )
			used = std::min( std::size_t( size ) - 1, used + std::size_t( written ) );
	}
	// The views' lights by baked shadow mask (CPU counts).
	const std::uint64_t masked = m_MaskedLights.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t maskedMovers = m_MaskedMoverLights.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t unmasked = m_UnmaskedLights.exchange( 0, std::memory_order_relaxed );
	if ( masked + maskedMovers + unmasked && used + 1 < size )
	{
		const int written = std::snprintf( out + used, size - used,
		    "0 0 %.2f view lights with baked shadow masks (count; %.2f with a mover in reach, "
		    "%.2f without a mask, %.2f of them world lights, per frame)\n",
		    double( masked ) / frames, double( maskedMovers ) / frames, double( unmasked ) / frames,
		    double( m_UnmaskedWorldLights.exchange( 0, std::memory_order_relaxed ) ) / frames );
		if ( written > 0 )
			used = std::min( std::size_t( size ) - 1, used + std::size_t( written ) );
	}
	// The shadow tiles drawn, kept, and shared across views (CPU counts; the
	// time is the shadow-depth passes').
	const std::uint64_t tilesDrawn = m_ShadowTilesDrawn.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t tilesKept = m_ShadowTilesKept.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t tilesShared = m_ShadowTilesShared.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t unshadowed =
	    m_ShadowLightsUnshadowed.exchange( 0, std::memory_order_relaxed );
	if ( tilesDrawn + tilesKept + tilesShared + unshadowed && used + 1 < size )
	{
		const int written = std::snprintf( out + used, size - used,
		    "0 0 %.2f shadow tiles drawn (count; %.2f kept, %.2f shared across views, "
		    "%.2f with movers, %.2f shadowed lights without tiles per frame)\n",
		    double( tilesDrawn ) / frames, double( tilesKept ) / frames,
		    double( tilesShared ) / frames,
		    double( m_ShadowTilesMoving.exchange( 0, std::memory_order_relaxed ) ) / frames,
		    double( unshadowed ) / frames );
		if ( written > 0 )
			used = std::min( std::size_t( size ) - 1, used + std::size_t( written ) );
	}
	if ( report.overflowed && used + 1 < size )
		std::snprintf(
		    out + used, size - used, "0 0 %u (timestamps dropped)\n", report.overflowed );
	return report.frames;
}

void CoreWorld::ReadCosts( RenderCoreCostReport *out )
{
	if ( !out )
		return;
	*out = {};
	std::lock_guard<std::mutex> guard( m_TimersLock );
	if ( !m_Timers )
		return;
	out->available = true;
	out->supported = m_Timers->Supported();
	out->currentFrame = m_TimersFrame;
	graph::PassTimerReport report = m_Timers->Latest();
	out->frame = report.lastFrame;
	out->dropped = report.overflowed;
	auto resourceSample = []( const graph::PassTimerReport &source )
	{
		RenderCoreResourceSample sample;
		const auto &activity = source.resources;
		sample.supported = activity.supported;
		sample.frame = source.lastFrame;
		sample.created = activity.Created();
		sample.destroyed = activity.Destroyed();
		sample.released = activity.releaseRequests;
		sample.bufferBytes = activity.bufferBytes;
		sample.live = activity.live;
		sample.pending = activity.pending;
		sample.buffers = activity.created[std::size_t( device::ResourceKind::kBuffer )];
		sample.textures = activity.created[std::size_t( device::ResourceKind::kTexture )];
		sample.groups = activity.created[std::size_t( device::ResourceKind::kBindGroup )];
		sample.other = sample.created - sample.buffers - sample.textures - sample.groups;
		return sample;
	};
	out->resources = resourceSample( report );
	for ( const auto &sample : m_Timers->Recent() )
	{
		if ( out->historyCount == RenderCoreCostReport::kCapacity )
			break;
		out->history[out->historyCount++] = resourceSample( sample );
	}
	// Hottest inclusive scopes first; retain an explicit omitted count.
	std::stable_sort( report.passes.begin(), report.passes.end(),
	    []( const graph::PassTime &a, const graph::PassTime &b )
	    {
		    return std::max( a.cpuMilliseconds, a.milliseconds ) >
		           std::max( b.cpuMilliseconds, b.milliseconds );
	    } );
	out->count = std::min<unsigned int>( RenderCoreCostReport::kCapacity, report.passes.size() );
	out->omitted = static_cast<unsigned int>( report.passes.size() ) - out->count;
	for ( unsigned int i = 0; i < out->count; ++i )
	{
		const graph::PassTime &pass = report.passes[i];
		RenderCoreCostRow &row = out->rows[i];
		std::snprintf( row.name, sizeof( row.name ), "%s", pass.name.c_str() );
		row.depth = pass.depth;
		row.cpuMilliseconds = pass.cpuMilliseconds;
		row.gpuMilliseconds = pass.milliseconds;
		row.resourcesSupported = pass.resources.supported;
		row.created = pass.resources.Created();
		row.destroyed = pass.resources.Destroyed();
		row.bufferBytes = pass.resources.bufferBytes;
	}
}

} // namespace render::composition
