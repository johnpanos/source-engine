//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's RFC 0011 indirect-light host: the map's producer
//          switcher, r_indirect_producer, and the volume consumers sample.
//
//          The host runs on the main thread. CPU producers' jobs run in
//          order on it; a job's data-parallel batches go through the job
//          system (RunThreadPoolJobBatch), serially by default (the budget's
//          one worker, and the pooled mode's oracle) or on the engine's pool
//          (r_indirect_executor 1). Their volumes are CPU data, uploaded to
//          the renderer when the published generation changes, with the
//          change from the bake for the world under BakedPlusDelta. The
//          renderer retires each replaced upload behind its own completion
//          serial, so the host's resource tracker only models frame latency
//          for producers that own none.
//
//===========================================================================//

#include "indirect_light_host.h"
#include "indirect_light_defaults.h"

#include "bspflags.h"
#include "cmodel_engine.h"
#include "convar.h"
#include "host.h"
#include "render.h"
#include "lightcache.h"
#include "materialsystem/imaterialsystem.h"
#include "gl_lightmap.h"
#include "gl_model_private.h"
#include "icliententity.h"
#include "icliententitylist.h"
#include "render/direct_occlusion.h"
#include "render/gpu_compute.h"
#include "render/indirect_radiosity.h"
#include "render/indirect_sdf.h"
#include "render/indirect_portals.h"
#include "render/indirect_switcher.h"
#include "render/light_set.h"
#include "render/world_mesh_upload.h"
#include "tier0/dbg.h"
#include "tier0/platform.h"
#include "tier1/interface.h"
#include "vstdlib/jobgraph_parallel.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
using namespace indirect_light;

void OnProducerChanged( IConVar *var, const char *oldValue, float oldFloat );
void ReportOffered();

ConVar r_indirect_producer( "r_indirect_producer", "auto", FCVAR_ARCHIVE,
    "Indirect-light producer (RFC 0011): auto (the product profile's default: the first of its "
    "producers this map and device offer), baked, radiosity, sdf or rayquery; only the values "
    "in r_indirect_producer_offered are available on this device and map",
    OnProducerChanged );
ConVar r_indirect_producer_offered( "r_indirect_producer_offered", "baked", FCVAR_NONE,
    "The r_indirect_producer values this device and map offer (read-only; set per map)" );
// The contract's scripted fake, for switching tests only (a cheat: never offered to players).
ConVar r_indirect_test_fake( "r_indirect_test_fake", "0", FCVAR_CHEAT,
    "Offer the scripted fake producer (\"fake\") for switching tests; takes effect at the next "
    "map" );

ConVar r_indirect_executor( "r_indirect_executor", "0", FCVAR_NONE,
    "How CPU indirect-light producers run their batches (RFC 0003 job system): 0 serially on "
    "the main thread (one worker, the pooled mode's oracle), 1 on the engine's worker pool; "
    "both produce identical volumes" );
ConVar r_indirect_portals( "r_indirect_portals", "1", FCVAR_CHEAT,
    "Producers that carry light through open portal pairs are given them (RFC 0011 G10; 0: "
    "not, the negative control of the portal-light test)" );
ConVar r_indirect_shadows( "r_indirect_shadows", "1", FCVAR_CHEAT,
    "Unbaked lights are shadowed by the map's SDF (RFC 0011 G9; 0: unshadowed, the negative "
    "control of the swing test)" );
ConVar r_indirect_occlusion( "r_indirect_occlusion", "1", FCVAR_CHEAT,
    "Moving geometry (drawn brush entities) blocks the baked direct light and the probe "
    "visibility behind it (0: the bake's, the negative control of the door test)" );
ConVar r_indirect_report( "r_indirect_report", "0", FCVAR_NONE,
    "Print each indirect-light update: the producer's CPU time and the published volume's mean "
    "indirect light, and the traced producers' focus when it changes (2: its probes too)" );

ConVar r_indirect_focus( "r_indirect_focus", "1", FCVAR_NONE,
    "Traced producers (sdf, rayquery) update the probes around the camera every update (those "
    "in the clusters it can see and the clusters touching them) and the others in turn; 0: every "
    "probe every update" );
ConVar r_indirect_probe_budget( "r_indirect_probe_budget", "128", FCVAR_NONE,
    "Probes outside the camera's focus a traced producer updates per update" );
ConVar r_indirect_focus_radius( "r_indirect_focus_radius", "1500", FCVAR_NONE,
    "The focus radius (units) where the map's visibility does not narrow the focus" );

// The traced producers' focus (FrameWork::focusProbes): the probes around
// the camera. Each active probe's clusters (its open leaves'), and each cluster's neighbours
// (clusters whose open leaves touch its own: the room beyond a doorway), are
// found once per map. Per frame the focus is the probes in the clusters the
// camera's cluster can see (its PVS) and their neighbours; where that does
// not narrow the map (no visibility, the camera outside every cluster, or
// most probes in view anyway, as in a pipeline map whose one collision leaf
// holds every room) it is the probes within r_indirect_focus_radius.
class ProbeFocus
{
public:
	void Build( const Volume &volume )
	{
		*this = ProbeFocus();
		m_positions = ProbePositions( volume );
		const size_t probes = m_positions.size() / 4;
		m_clusters = CM_NumClusters();
		// A probe's clusters: those of the open leaves at its centre and half a
		// spacing along each axis (a probe inside a brush or on a wall still
		// lights the rooms beside it).
		const mapcontainer::ProbeGridLayout &grid = volume.layout.grids[0];
		m_probeClusters.assign( probes * kProbeSamples, -1 );
		for ( size_t p = 0; p < probes; ++p )
		{
			if ( m_positions[p * 4 + 3] < 0.5f )
				continue;
			++m_active;
			for ( int k = 0; k < kProbeSamples; ++k )
			{
				Vector at( m_positions[p * 4], m_positions[p * 4 + 1], m_positions[p * 4 + 2] );
				if ( k > 0 )
					at[( k - 1 ) / 2] += ( k % 2 ? 0.5f : -0.5f ) * grid.spacing[( k - 1 ) / 2];
				m_probeClusters[p * kProbeSamples + k] = CM_LeafCluster( CM_PointLeafnum( at ) );
			}
		}
	}

	// The clusters' neighbours, from the world's leaves (loaded after the
	// map's indirect-light data, so found at the first update).
	void BuildNeighbours()
	{
		m_neighbours.assign( size_t( std::max( m_clusters, 0 ) ), {} );
		const worldbrushdata_t *world = host_state.worldbrush;
		if ( !world || m_clusters <= 1 )
			return;
		m_haveNeighbours = true;
		struct Leaf
		{
			Vector lo, hi;
			int cluster;
		};
		std::vector<Leaf> leaves;
		for ( int i = 0; i < world->numleafs; ++i )
		{
			const mleaf_t &leaf = world->leafs[i];
			if ( ( leaf.contents & CONTENTS_SOLID ) || leaf.cluster < 0 ||
			     leaf.cluster >= m_clusters )
				continue;
			const Vector margin( 1.0f, 1.0f, 1.0f );
			leaves.push_back( { leaf.m_vecCenter - leaf.m_vecHalfDiagonal - margin,
			    leaf.m_vecCenter + leaf.m_vecHalfDiagonal + margin, leaf.cluster } );
		}
		std::sort( leaves.begin(), leaves.end(),
		    []( const Leaf &a, const Leaf &b )
		    {
			    return a.lo.x < b.lo.x;
		    } );
		for ( size_t i = 0; i < leaves.size(); ++i )
			for ( size_t j = i + 1; j < leaves.size() && leaves[j].lo.x <= leaves[i].hi.x; ++j )
			{
				const Leaf &a = leaves[i], &b = leaves[j];
				if ( a.cluster == b.cluster || a.lo.y > b.hi.y || b.lo.y > a.hi.y ||
				     a.lo.z > b.hi.z || b.lo.z > a.hi.z )
					continue;
				m_neighbours[a.cluster].push_back( b.cluster );
				m_neighbours[b.cluster].push_back( a.cluster );
			}
		for ( std::vector<int> &list : m_neighbours )
		{
			std::sort( list.begin(), list.end() );
			list.erase( std::unique( list.begin(), list.end() ), list.end() );
		}
	}

	// This frame's focus; recomputed when the camera changes cluster (or,
	// by distance, moves 64 units).
	std::span<const uint32_t> Update( const Vector &eye )
	{
		if ( !m_haveNeighbours && m_clusters > 1 && host_state.worldbrush )
		{
			BuildNeighbours();
			m_valid = false;
		}
		const int cluster = m_clusters > 1 ? CM_LeafCluster( CM_PointLeafnum( eye ) ) : -1;
		const bool moved = ( eye - m_eye ).LengthSqr() > 64.0f * 64.0f;
		if ( m_valid && cluster == m_cameraCluster && ( !m_byDistance || !moved ) &&
		     r_indirect_focus_radius.GetFloat() == m_radius )
			return m_focus;
		m_valid = true;
		m_cameraCluster = cluster;
		m_eye = eye;
		m_radius = r_indirect_focus_radius.GetFloat();
		m_focus.clear();
		m_byDistance = true;
		if ( cluster >= 0 && cluster < m_clusters )
		{
			// The clusters the camera can see, and their neighbours.
			std::vector<uint8_t> seen( size_t( m_clusters ), 0 );
			const byte *pvs = CM_ClusterPVS( cluster );
			for ( int c = 0; c < m_clusters; ++c )
				if ( pvs[c >> 3] & ( 1 << ( c & 7 ) ) )
					seen[c] = 1;
			seen[cluster] = 1;
			std::vector<uint8_t> near = seen;
			for ( int c = 0; c < m_clusters && size_t( c ) < m_neighbours.size(); ++c )
				if ( seen[c] )
					for ( int n : m_neighbours[c] )
						near[n] = 1;
			for ( size_t p = 0; p * kProbeSamples < m_probeClusters.size(); ++p )
				for ( int k = 0; k < kProbeSamples; ++k )
				{
					const int c = m_probeClusters[p * kProbeSamples + k];
					if ( c >= 0 && c < m_clusters && near[c] )
					{
						m_focus.push_back( uint32_t( p ) );
						break;
					}
				}
			m_byDistance = m_focus.empty() || m_focus.size() * 4 > m_active * 3;
		}
		if ( m_byDistance )
		{
			m_focus.clear();
			const float r2 = m_radius * m_radius;
			for ( size_t p = 0; p * 4 < m_positions.size(); ++p )
			{
				const Vector at(
				    m_positions[p * 4], m_positions[p * 4 + 1], m_positions[p * 4 + 2] );
				if ( m_positions[p * 4 + 3] >= 0.5f && ( at - eye ).LengthSqr() <= r2 )
					m_focus.push_back( uint32_t( p ) );
			}
		}
		if ( r_indirect_report.GetBool() )
			Msg( "indirect light: focus %zu of %zu active probes (%s, camera cluster %d)\n",
			    m_focus.size(), m_active, m_byDistance ? "distance" : "visibility", cluster );
		if ( r_indirect_report.GetInt() >= 2 )
		{
			// The probes themselves, for tools/quality/gi_focus.py to compare.
			std::string list;
			for ( uint32_t p : m_focus )
				list += " " + std::to_string( p );
			Msg( "indirect light: focus probes%s\n", list.c_str() );
		}
		return m_focus;
	}

private:
	static constexpr int kProbeSamples = 7; // the centre and six half-spacing offsets
	std::vector<float> m_positions;
	std::vector<int> m_probeClusters; // kProbeSamples per probe (-1: solid or none)
	std::vector<std::vector<int>> m_neighbours;
	std::vector<uint32_t> m_focus;
	Vector m_eye{ 0, 0, 0 };
	float m_radius = 0.0f;
	int m_clusters = 0;
	int m_cameraCluster = -1;
	size_t m_active = 0;
	bool m_valid = false;
	bool m_byDistance = true;
	bool m_haveNeighbours = false;
};

// Frames in flight the host assumes for producers without device resources.
constexpr uint64_t kFrameLatency = 3;

class FrameTracker final : public IResourceTracker
{
public:
	uint64_t Acquire( size_t bytes ) override
	{
		m_live[++m_nextId] = bytes;
		return m_nextId;
	}
	void Release( uint64_t id, uint64_t afterSerial ) override { m_pending[id] = afterSerial; }
	uint64_t SubmittedSerial() const override { return m_frame; }
	uint64_t CompletedSerial() const override
	{
		return m_frame > kFrameLatency ? m_frame - kFrameLatency : 0;
	}
	void Advance()
	{
		++m_frame;
		for ( auto it = m_pending.begin(); it != m_pending.end(); )
		{
			if ( CompletedSerial() >= it->second )
			{
				m_live.erase( it->first );
				it = m_pending.erase( it );
			}
			else
				++it;
		}
	}
	uint64_t Frame() const { return m_frame; }

private:
	uint64_t m_nextId = 0;
	uint64_t m_frame = 0;
	std::map<uint64_t, size_t> m_live;
	std::map<uint64_t, uint64_t> m_pending;
};

// The frame executor's batches on the job system (jobsystem/parallel_batch.h).
class EngineExecutor final : public IBatchExecutor
{
public:
	explicit EngineExecutor( bool alwaysPooled = false ) : m_alwaysPooled( alwaysPooled ) {}
	void ParallelFor( const char *name, uint32_t count, void ( *body )( void *, uint32_t ),
	    void *context ) override
	{
		jobsystem::BatchDesc desc;
		desc.name = name;
		desc.context = context;
		desc.count = count;
		desc.process = body;
		const bool pooled =
		    ( m_alwaysPooled || r_indirect_executor.GetInt() == 1 ) && g_pThreadPool;
		desc.maxParticipants = pooled ? unsigned( g_pThreadPool->NumThreads() ) + 1 : 1;
		if ( !RunThreadPoolJobBatch( pooled ? g_pThreadPool : nullptr, desc,
		         pooled ? jobsystem::BatchMode::Parallel : jobsystem::BatchMode::Serial ) )
		{
			// Rejected before any item ran (invalid input): run them here.
			for ( uint32_t i = 0; i < count; ++i )
				body( context, i );
		}
	}

private:
	bool m_alwaysPooled;
};

// The SDF shadow of unbaked lights (G9) is a per-pixel sphere trace, as
// unmeasured on the Fold7 as the traced producers: Android draws them
// unshadowed.
#ifdef ANDROID
constexpr bool kSdfProfileSupported = false;
constexpr bool kRayQueryProfileSupported = false;
constexpr bool kShadowFieldProfileSupported = false;
#else
constexpr bool kSdfProfileSupported = true;
constexpr bool kRayQueryProfileSupported = true;
constexpr bool kShadowFieldProfileSupported = true;
#endif

class EngineCatalog final : public IProducerCatalog
{
public:
	bool Offers( ProducerKind kind, const IndirectScene &scene ) const override
	{
		if ( !scene.baked )
			return false;
		if ( kind == ProducerKind::Baked )
			return true;
		if ( kind == ProducerKind::PrecomputedRadiosity )
			return scene.transfer != nullptr;
		// The GPU producers: the map's field and a device that runs compute, on
		// a profile whose budget they were measured within
		// (quality/budgets/indirect-light-v1.json: both unsupported on Android
		// until measured there, RFC 0011 G6.4 and G7.3).
		if ( kind == ProducerKind::SdfTraced )
			return kSdfProfileSupported && scene.sdf != nullptr && scene.gpu != nullptr &&
			       scene.gpu->Capabilities().compute;
		// Ray query: also the world's triangles and a device that has it.
		if ( kind == ProducerKind::RayQuery )
			return kRayQueryProfileSupported && scene.sdf != nullptr && scene.geometry != nullptr &&
			       scene.gpu != nullptr && scene.gpu->Capabilities().rayQuery;
		// The contract's scripted fake: switching tests only, never players.
		return kind == ProducerKind::ScriptedFake && r_indirect_test_fake.GetBool();
	}
	std::unique_ptr<IProducer> Create( ProducerKind kind ) override
	{
		if ( kind == ProducerKind::Baked )
			return std::make_unique<BakedProducer>();
		if ( kind == ProducerKind::PrecomputedRadiosity )
			return std::make_unique<RadiosityProducer>();
		if ( kind == ProducerKind::SdfTraced )
			return std::make_unique<SdfTracedProducer>();
		if ( kind == ProducerKind::RayQuery )
			return std::make_unique<RayQueryProducer>();
		if ( kind == ProducerKind::ScriptedFake )
			return std::make_unique<ScriptedFakeProducer>();
		return nullptr;
	}
};

struct Host
{
	EngineCatalog catalog;
	EngineExecutor executor;
	FrameTracker tracker;
	std::unique_ptr<Switcher> switcher;
	IndirectScene scene;
	std::shared_ptr<const Volume> current;
	std::optional<mapcontainer::ProbeVolumeView> view;
	std::vector<unsigned char> change; // the world's change atlas (BakedPlusDelta)
	std::vector<Proxy> proxies;         // this frame's moving geometry
	// Baked direct light moving geometry blocks, and the occluders the
	// uploaded lightmap was composed for.
	DirectOcclusion occlusion;
	// Occlusion runs on the pool whatever r_indirect_executor says: it is a
	// one-frame burst when a door moves, not a per-frame producer update.
	EngineExecutor occlusionExecutor{ true };
	std::vector<Proxy> occluded;
	std::vector<unsigned char> occludedTotal;
	std::vector<Proxy> visibilityProxies;      // the proxies the consumed volume's visibility has
	bool brushesLit = false;                   // brush entities lit from the consumed volume
	bool shadowFieldUploaded = false;          // the SDFV's distances, for direct-light shadows
	std::vector<LightOverride> lightOverrides; // r_indirect_light_direction
	std::vector<Portal> portals;               // the client's open portals (G10)
	ProbeFocus focus;                          // the traced producers' probes near the camera
	uint64_t uploadedGeneration = 0;
	uint64_t mapSerial = 0;
	bool deviceLost = false;
	bool reverting = false;
};

Host &TheHost()
{
	static Host host;
	return host;
}

gpu_compute::IGpuCompute *GpuCompute()
{
	return materials ? static_cast<gpu_compute::IGpuCompute *>(
	                       materials->QueryInterface( gpu_compute::kGpuComputeInterface ) )
	                 : nullptr;
}

// Moving geometry the static field lacks: every drawn brush entity (a door,
// a func_brush), as its world-space bounds. The producer takes the first
// kMaxProxies.
void GatherProxies( std::vector<Proxy> *out )
{
	out->clear();
	if ( !entitylist )
		return;
	const int highest = entitylist->GetHighestEntityIndex();
	for ( int i = 1; i <= highest && out->size() < SdfTraceParams::kMaxProxies; ++i )
	{
		IClientEntity *entity = entitylist->GetClientEntity( i );
		if ( !entity || entity->IsDormant() || !entity->ShouldDraw() )
			continue;
		const model_t *model = entity->GetModel();
		if ( !model || model->type != mod_brush )
			continue;
		Vector lo, hi;
		entity->GetRenderBoundsWorldspace( lo, hi );
		Proxy proxy;
		for ( int k = 0; k < 3; ++k )
		{
			proxy.lo[k] = lo[k];
			proxy.hi[k] = hi[k];
		}
		out->push_back( proxy );
	}
}

world_mesh_gpu::IWorldMeshUpload *Uploader();

// Re-composes and uploads the world's lightmap when the occluders change:
// the bake's total less the direct light they block (DirectOcclusion).
void ApplyOcclusion( Host &host )
{
	static const std::vector<Proxy> none;
	const std::vector<Proxy> &proxies = r_indirect_occlusion.GetBool() ? host.proxies : none;
	if ( !host.occlusion.Ready() || host.deviceLost || proxies == host.occluded )
		return;
	world_mesh_gpu::IWorldMeshUpload *uploader = Uploader();
	if ( !uploader || !uploader->IsResident() )
		return;
	const double started = Plat_FloatTime();
	const size_t blocked =
	    host.occlusion.Compose( proxies, &host.occlusionExecutor, &host.occludedTotal );
	const mapcontainer::WorldLightmapLayout &layout = host.occlusion.Layout();
	world_mesh_gpu::WorldLightmapUploadRequest request;
	request.width = layout.width;
	request.height = layout.height;
	request.layerCount = layout.layerCount;
	for ( uint32_t i = 0; i < layout.layerCount; ++i )
	{
		request.roles[i] = static_cast<world_mesh_gpu::WorldLightmapRole>( layout.roles[i] );
		request.layers[i] = layout.roles[i] == mapcontainer::WorldLightmapLayer::Total
		                        ? static_cast<const void *>( host.occludedTotal.data() )
		                        : host.occlusion.Bytes() + layout.layerOffset[i];
	}
	if ( !uploader->UploadLightmap( request ) )
	{
		Warning( "indirect light: the occluded lightmap upload failed; the bake stays\n" );
		return;
	}
	host.occluded = proxies;
	if ( r_indirect_report.GetBool() )
		Msg( "indirect light: %zu occluder(s) block baked direct light at %zu texels (%.2f ms)\n",
		    proxies.size(), blocked, ( Plat_FloatTime() - started ) * 1000.0 );
}

world_mesh_gpu::IWorldMeshUpload *Uploader()
{
	return materials ? static_cast<world_mesh_gpu::IWorldMeshUpload *>(
	                       materials->QueryInterface( world_mesh_gpu::kWorldMeshUploadInterface ) )
	                 : nullptr;
}

// Makes `frame` what consumers sample: the CPU view (the ambient cube) and
// the renderer's per-pixel copy.
void Consume( const FrameVolume &frame )
{
	Host &host = TheHost();
	if ( !frame.volume || frame.generation == host.uploadedGeneration )
		return;
	host.uploadedGeneration = frame.generation;
	host.current = frame.volume;
	// Moving geometry in the probes' visibility: a closed door stops probes
	// on its far side from lighting through it (a copy; the producer's
	// volume stays as published).
	host.visibilityProxies = host.proxies;
	if ( !host.proxies.empty() && r_indirect_occlusion.GetBool() )
	{
		auto occluded = std::make_shared<Volume>( *frame.volume );
		if ( OccludeProbeVisibility( *occluded, host.proxies ) )
			host.current = occluded;
	}
	host.view.emplace( host.current->bytes.data(), host.current->layout );
	// Every model's ambient cube is re-evaluated from the new volume, and
	// brush entities without baked light (doors) are lit from it.
	R_StudioInitLightingCache();
	host.brushesLit = R_RelightBrushEntitiesFromProbes( &*host.view );
	world_mesh_gpu::IWorldMeshUpload *uploader = Uploader();
	if ( host.deviceLost || !uploader || !uploader->IsResident() )
		return;
	const mapcontainer::ProbeVolumeLayout &layout = host.current->layout;
	std::vector<float> table( layout.gridCount * mapcontainer::kProbeGridTableFloats );
	mapcontainer::WriteProbeGridTable( layout, table.data() );
	world_mesh_gpu::ProbeVolumeUploadRequest request;
	request.atlasWidth = layout.atlasWidth;
	request.atlasHeight = layout.atlasHeight;
	request.atlas = host.current->bytes.data() + layout.atlasOffset;
	request.gridCount = layout.gridCount;
	request.tableFloats = mapcontainer::kProbeGridTableFloats;
	request.gridTable = table.data();
	// BakedPlusDelta: the world adds the change from the bake.
	if ( frame.policy == indirect_policy::Policy::BakedPlusDelta && host.scene.baked &&
	     host.current != host.scene.baked &&
	     ChangeAtlas( *host.current, *host.scene.baked, &host.change ) )
		request.deltaAtlas = host.change.data();
	if ( !uploader->UploadProbeVolume( request ) )
		Warning( "indirect light: probe volume upload failed; models use the ambient cube\n" );
}

void ReportOffered()
{
	Host &host = TheHost();
	std::string offered;
	for ( ProducerKind kind : { ProducerKind::Baked, ProducerKind::PrecomputedRadiosity,
	          ProducerKind::SdfTraced, ProducerKind::RayQuery, ProducerKind::ScriptedFake } )
	{
		if ( host.catalog.Offers( kind, host.scene ) )
		{
			if ( !offered.empty() )
				offered += ' ';
			offered += ProducerName( kind );
		}
	}
	r_indirect_producer_offered.SetValue( offered.c_str() );
}

// A setting's producer: a name, or `auto` for the product profile's first
// offered default (engine/indirect_light_defaults.h; baked closes the list).
bool ResolveProducer( const char *value, ProducerKind *out )
{
	if ( value && std::strcmp( value, "auto" ) == 0 )
	{
		Host &host = TheHost();
		for ( const char *name : kIndirectDefaultProducers )
		{
			ProducerKind kind;
			if ( ParseProducer( name, &kind ) && host.catalog.Offers( kind, host.scene ) )
			{
				*out = kind;
				return true;
			}
		}
		*out = ProducerKind::Baked;
		return true;
	}
	return ParseProducer( value, out );
}

void OnProducerChanged( IConVar *var, const char *oldValue, float )
{
	Host &host = TheHost();
	if ( host.reverting || !host.switcher )
		return;
	ConVarRef ref( var );
	ProducerKind kind;
	const char *requested = ref.GetString();
	const auto revert = [&]( const char *reason )
	{
		Warning( "r_indirect_producer \"%s\" unavailable (%s); keeping %s\n", requested, reason,
		    ProducerName( host.switcher->Selected() ) );
		host.reverting = true;
		ref.SetValue( oldValue );
		host.reverting = false;
	};
	ReportOffered();
	if ( !ResolveProducer( requested, &kind ) )
	{
		revert( "unknown-producer" );
		return;
	}
	const auto selected = host.switcher->Select( kind );
	if ( !selected )
	{
		revert( IndirectErrorName( selected.Error() ) );
		return;
	}
	Msg( "indirect light: switching to %s\n", ProducerName( kind ) );
}

// A scene light's direction by light style (a sun moved for a test); the
// producers that claim LightDirection trace it.
CON_COMMAND_F( r_indirect_light_direction,
    "r_indirect_light_direction <style> <x> <y> <z>: the direction light of style <style> "
    "travels, for producers that respond to light direction; 'clear' restores the baked ones",
    FCVAR_CHEAT )
{
	Host &host = TheHost();
	if ( args.ArgC() == 2 && !Q_stricmp( args[1], "clear" ) )
	{
		host.lightOverrides.clear();
		return;
	}
	if ( args.ArgC() != 5 )
	{
		Msg( "usage: r_indirect_light_direction <style> <x> <y> <z> | clear\n" );
		return;
	}
	LightOverride light;
	light.style = atoi( args[1] );
	Vector direction( atof( args[2] ), atof( args[3] ), atof( args[4] ) );
	if ( VectorNormalize( direction ) <= 0.0f )
	{
		Msg( "r_indirect_light_direction: zero direction\n" );
		return;
	}
	for ( int k = 0; k < 3; ++k )
		light.direction[k] = direction[k];
	for ( LightOverride &existing : host.lightOverrides )
		if ( existing.style == light.style )
		{
			existing = light;
			return;
		}
	host.lightOverrides.push_back( light );
}

} // namespace

void IndirectLight_BeginMap( const IndirectLightMapData &map )
{
	const unsigned char *prbv = map.prbv, *rtrn = map.rtrn, *sdfv = map.sdfv;
	const size_t size = map.prbvSize, rtrnSize = map.rtrnSize, sdfvSize = map.sdfvSize;
	Host &host = TheHost();
	IndirectLight_EndMap();
	auto baked = Volume::FromBytes( std::vector<unsigned char>( prbv, prbv + size ) );
	if ( !baked )
		return;
	host.scene = IndirectScene();
	host.scene.mapSerial = ++host.mapSerial;
	host.scene.baked = baked;
	if ( rtrn && rtrnSize )
	{
		host.scene.transfer =
		    Transfer::FromBytes( std::vector<unsigned char>( rtrn, rtrn + rtrnSize ), *baked );
		if ( host.scene.transfer )
		{
			const mapcontainer::RadiosityTransferLayout &t = host.scene.transfer->layout;
			Msg( "indirect light: RTRN %u sources, %u patches, %u + %u links, %u KB\n",
			    t.sourceCount, t.patchCount, t.transferLinks, t.gatherLinks,
			    unsigned( rtrnSize / 1024 ) );
		}
		else
			Warning( "indirect light: RTRN rejected (malformed, or not baked for this map's "
			         "PRBV); radiosity is not offered\n" );
	}
	if ( sdfv && sdfvSize )
	{
		host.scene.sdf = SdfData::FromBytes( std::vector<unsigned char>( sdfv, sdfv + sdfvSize ) );
		if ( host.scene.sdf )
		{
			const mapcontainer::SdfVolumeLayout &f = host.scene.sdf->layout;
			Msg( "indirect light: SDFV %ux%ux%u voxels of %.1f units, %u lights, %u KB\n",
			    f.dims[0], f.dims[1], f.dims[2], f.voxel, f.lightCount,
			    unsigned( sdfvSize / 1024 ) );
		}
		else
			Warning( "indirect light: SDFV rejected (malformed); the SDF producer is not "
			         "offered\n" );
	}
	if ( map.wmsh && map.wmshSize && host.scene.sdf )
		host.scene.geometry = WorldGeometryFromMesh( map.wmsh, map.wmshSize );
	host.scene.gpu = GpuCompute();
	host.occluded.clear();
	if ( host.scene.sdf )
		host.focus.Build( *baked );
	if ( host.scene.sdf && map.wmsh && map.lmap )
	{
		const mapcontainer::SdfVolumeLayout &f = host.scene.sdf->layout;
		std::vector<mapcontainer::SdfLight> lights( f.lightCount );
		std::memcpy( lights.data(), host.scene.sdf->bytes.data() + f.lightOffset,
		    lights.size() * sizeof( mapcontainer::SdfLight ) );
		if ( host.occlusion.Build(
		         map.wmsh, map.wmshSize, map.lmap, map.lmapSize, map.lmapVersion, lights ) )
			Msg( "indirect light: baked direct light follows moving geometry (%zu texels, %u "
			     "light(s))\n",
			    host.occlusion.CoveredTexels(), f.lightCount );
	}
	host.switcher = std::make_unique<Switcher>( host.catalog, host.tracker );
	ReportOffered();
	ProducerKind requested = ProducerKind::Baked;
	const char *saved = r_indirect_producer.GetString();
	const bool parsed = ResolveProducer( saved, &requested );
	const auto begun =
	    host.switcher->BeginMap( host.scene, parsed ? requested : ProducerKind::Baked );
	if ( !parsed || !begun )
	{
		// The saved value stays; the default is used and the substitution recorded.
		Warning( "indirect light: r_indirect_producer \"%s\" is not offered here (%s); using baked "
		         "(offered: %s)\n",
		    saved, parsed ? IndirectErrorName( begun.Error() ) : "unknown-producer",
		    r_indirect_producer_offered.GetString() );
	}
	FrameWork work;
	work.frameSerial = host.tracker.Frame() + 1;
	work.resources = &host.tracker;
	work.executor = &host.executor;
	const light_set::Snapshot none;
	Consume( host.switcher->Frame( work, none ) );
	for ( auto &job : work.jobs )
		job();
	host.tracker.Advance();
	Msg( "indirect light: map %llu, producer %s%s (offered: %s)\n",
	    (unsigned long long)host.scene.mapSerial, ProducerName( host.switcher->Active() ),
	    std::strcmp( saved, "auto" ) == 0 ? " (auto)" : "",
	    r_indirect_producer_offered.GetString() );
}

void IndirectLight_EndMap()
{
	Host &host = TheHost();
	if ( host.switcher )
		host.switcher->EndMap();
	host.switcher.reset();
	R_RelightBrushEntitiesFromProbes( nullptr );
	host.brushesLit = false;
	host.shadowFieldUploaded = false;
	host.view.reset();
	host.current.reset();
	host.change.clear();
	host.proxies.clear();
	host.lightOverrides.clear();
	host.portals.clear();
	host.focus = ProbeFocus();
	host.occlusion = DirectOcclusion();
	host.occluded.clear();
	host.occludedTotal.clear();
	host.visibilityProxies.clear();
	host.uploadedGeneration = 0;
	host.scene = IndirectScene();
}

// The SDFV's distances shadow the unbaked lights (G9): uploaded once per
// map, removed while r_indirect_shadows is 0.
static void SyncShadowField( Host &host )
{
	const bool wanted =
	    kShadowFieldProfileSupported && host.scene.sdf && r_indirect_shadows.GetBool();
	if ( wanted == host.shadowFieldUploaded || host.deviceLost )
		return;
	world_mesh_gpu::IWorldMeshUpload *uploader = Uploader();
	if ( !uploader || !uploader->IsResident() )
		return;
	host.shadowFieldUploaded = wanted;
	world_mesh_gpu::ShadowFieldUploadRequest field;
	std::vector<uint16_t> distances;
	if ( wanted )
	{
		const mapcontainer::SdfVolumeLayout &f = host.scene.sdf->layout;
		distances.resize( size_t( f.dims[0] ) * f.dims[1] * f.dims[2] );
		const unsigned char *voxel = host.scene.sdf->bytes.data() + f.voxelOffset;
		for ( size_t i = 0; i < distances.size(); ++i )
			std::memcpy( &distances[i], voxel + i * mapcontainer::kSdfVoxelBytes, 2 );
		std::memcpy( field.origin, f.origin, sizeof( field.origin ) );
		field.voxel = f.voxel;
		std::memcpy( field.dims, f.dims, sizeof( field.dims ) );
		field.distances = distances.data();
	}
	if ( !uploader->UploadShadowField( field ) && wanted )
		Warning( "indirect light: shadow field upload failed; unbaked lights are unshadowed\n" );
}

void IndirectLight_Frame( const light_set::Snapshot &lights )
{
	Host &host = TheHost();
	if ( !host.switcher )
		return;
	SyncShadowField( host );
	FrameWork work;
	work.frameSerial = host.tracker.Frame() + 1;
	work.resources = &host.tracker;
	work.executor = &host.executor;
	const size_t proxiesBefore = host.proxies.size();
	GatherProxies( &host.proxies );
	if ( r_indirect_report.GetBool() && host.proxies.size() != proxiesBefore )
	{
		Msg( "indirect light: %zu moving brush proxies\n", host.proxies.size() );
		for ( const Proxy &proxy : host.proxies )
			Msg( "  proxy (%.1f %.1f %.1f) - (%.1f %.1f %.1f)\n", proxy.lo[0], proxy.lo[1],
			    proxy.lo[2], proxy.hi[0], proxy.hi[1], proxy.hi[2] );
	}
	work.proxies = host.proxies;
	work.lightOverrides = host.lightOverrides;
	if ( r_indirect_portals.GetBool() )
		work.portals = host.portals;
	if ( host.scene.sdf && r_indirect_focus.GetBool() )
		work.focusProbes = host.focus.Update( MainViewOrigin() );
	work.probeBudget = uint32_t( std::max( 0, r_indirect_probe_budget.GetInt() ) );
	ApplyOcclusion( host );
	const FrameVolume frame = host.switcher->Frame( work, lights );
	// The jobs run in order on this thread; their batches on the executor.
	const double started = Plat_FloatTime();
	for ( auto &job : work.jobs )
		job();
	const double updateMs = ( Plat_FloatTime() - started ) * 1000.0;
	host.tracker.Advance();
	// New proxies re-consume the same volume with their visibility.
	if ( host.proxies != host.visibilityProxies )
		host.uploadedGeneration = 0;
	const bool published = frame.volume && frame.generation != host.uploadedGeneration;
	Consume( frame );
	// The volume the map loaded with, once its brush lightmaps exist.
	if ( !host.brushesLit && host.view )
		host.brushesLit = R_RelightBrushEntitiesFromProbes( &*host.view );
	if ( r_indirect_report.GetBool() && ( !work.jobs.empty() || published ) )
		Msg( "indirect light: frame %llu %s update %.3f ms (%s), generation %llu, policy %d, "
		     "indirect mean %.5f\n",
		    (unsigned long long)work.frameSerial, ProducerName( frame.producer ), updateMs,
		    r_indirect_executor.GetInt() == 1 ? "pooled" : "serial",
		    (unsigned long long)frame.generation, int( frame.policy ),
		    frame.volume ? frame.volume->MeanIrradiance( 1 ) : 0.0f );
}

const mapcontainer::ProbeVolumeView *IndirectLight_CurrentVolume()
{
	Host &host = TheHost();
	return host.view ? &*host.view : nullptr;
}

void IndirectLight_Background()
{
	if ( !TheHost().switcher )
		return;
	TheHost().switcher->Background();
	Msg( "indirect light: background (%s stops scheduling)\n",
	    ProducerName( TheHost().switcher->Active() ) );
}

void IndirectLight_Resume()
{
	if ( !TheHost().switcher )
		return;
	TheHost().switcher->Resume();
	Msg( "indirect light: resume (%s)\n", ProducerName( TheHost().switcher->Active() ) );
}

void IndirectLight_DeviceLost()
{
	Host &host = TheHost();
	host.deviceLost = true;
	if ( host.switcher )
		host.switcher->DeviceLost();
}

void IndirectLight_DeviceRestored()
{
	Host &host = TheHost();
	host.deviceLost = false;
	// The recreated device has the bake's lightmap: re-compose for the
	// occluders on the next frame.
	host.occluded.clear();
	if ( !host.switcher )
		return;
	const auto recovered = host.switcher->DeviceRecovered();
	if ( !recovered )
		Warning( "indirect light: %s is unavailable after device recovery (%s); using baked\n",
		    r_indirect_producer.GetString(), IndirectErrorName( recovered.Error() ) );
	// The recreated device gets the volume and the shadow field again.
	host.uploadedGeneration = 0;
	host.shadowFieldUploaded = false;
	FrameWork work;
	work.frameSerial = host.tracker.Frame() + 1;
	work.resources = &host.tracker;
	work.executor = &host.executor;
	const light_set::Snapshot none;
	const FrameVolume frame = host.switcher->Frame( work, none );
	for ( auto &job : work.jobs )
		job();
	host.tracker.Advance();
	Consume( frame );
}

// The client's open portal pairs (render/indirect_portals.h), copied as the
// next frame's portals.
class CIndirectLightPortals final : public indirect_portals::IIndirectLightPortals
{
public:
	void SetOpenPortals( const indirect_portals::PortalInput *portals, int count ) override
	{
		std::vector<Portal> &out = TheHost().portals;
		const size_t before = out.size();
		out.clear();
		for ( int i = 0; portals && i < count && i < indirect_portals::kMaxOpenPortals; ++i )
		{
			const indirect_portals::PortalInput &in = portals[i];
			Portal portal;
			std::memcpy( portal.origin, in.origin, sizeof( portal.origin ) );
			std::memcpy( portal.forward, in.forward, sizeof( portal.forward ) );
			std::memcpy( portal.right, in.right, sizeof( portal.right ) );
			std::memcpy( portal.up, in.up, sizeof( portal.up ) );
			portal.halfWidth = in.halfWidth;
			portal.halfHeight = in.halfHeight;
			std::memcpy( portal.toLinked, in.toLinked, sizeof( portal.toLinked ) );
			out.push_back( portal );
		}
		if ( r_indirect_report.GetBool() && out.size() != before )
			Msg( "indirect light: %zu open portal(s)\n", out.size() );
	}
};

static CIndirectLightPortals s_IndirectLightPortals;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE( CIndirectLightPortals, indirect_portals::,
    IIndirectLightPortals, indirect_portals::kIndirectLightPortalsVersion, s_IndirectLightPortals );
