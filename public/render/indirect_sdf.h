//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The traced producers (render.indirect-light.v1): probes trace
//          rays on the GPU through render/gpu_compute.h, so moved lights and
//          moving occluders change indirect light. The SDF-traced producer
//          (RFC 0011 G6) sphere-traces the map's signed distance volume
//          (SDFV) and this frame's proxy boxes ("sdf-probe-trace"); the
//          ray-query producer (G7) traces the world's triangles and the
//          proxies as a ray-query scene ("ray-query-probe-trace"), rebuilt
//          when the proxies change, and needs a device with ray query. Both
//          take surface attributes and lights from the SDFV.
//
//          Publication (render.indirect-policy.v1 BakedPlusDelta, as the
//          radiosity producer): the baked volume plus the change of the
//          traced field from its own converged estimate of the baked scene,
//          per texel: light removed as the ratio of the field to that
//          estimate (the bake times it), light added as their difference.
//          The traced field's bias then cancels, and the unchanged scene is
//          the bake byte for byte.
//
//          Phases, per probe. Reference: the probe converges under the
//          baked configuration (scalars 1, baked light directions, no
//          proxies) and that field is its reference; until then it publishes
//          the bake. Live: when the configuration (style scalars, light
//          directions, proxies) changes, every referenced probe updates with
//          a hysteresis that starts fast and settles, publishing every
//          completed update, then stops: a still scene costs nothing.
//
//          Scheduling. The host names the probes to update every update (its
//          focus: the camera's surroundings, FrameWork::focusProbes); the
//          others take turns, FrameWork::probeBudget an update. Without a
//          focus every probe with work updates every update. An update is a
//          live dispatch over every probe (a probe not updated carries its
//          estimate forward) and, while probes still build their reference,
//          a reference dispatch over those under the baked configuration.
//
//          Lights. The SDFV's light cells list the lights each region can
//          see; the program lights a probe and a ray hit with its cell's.
//
//          GPU lifetime. Four field, parameter, light and schedule buffers
//          form a ring; at most two updates are in flight, so the field the
//          CPU reads was written by a completed update and nothing in flight
//          writes it.
//          End retires every buffer behind the last dispatch's serial, and a
//          replaced ray-query scene retires behind the last dispatch that
//          traced it.
//
//===========================================================================//

#ifndef RENDER_INDIRECT_SDF_H
#define RENDER_INDIRECT_SDF_H

#include "mapcontainer/world_mesh.h"
#include "mapcontainer/world_mesh_format.h"
#include "render/indirect_light.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <deque>
#include <memory>
#include <span>
#include <vector>

namespace indirect_light
{

// The trace program's parameter block (sdf_probe_trace.comp, std430).
struct SdfTraceParams
{
	static constexpr uint32_t kMaxProxies = 16;
	float sdfOrigin[4];
	uint32_t sdfDims[4];
	float probeOrigin[4];
	float probeSpacing[4];
	uint32_t probeDims[4];
	uint32_t counts[4]; // x proxies, y the ray-query cull mask
	float cellOrigin[4];
	uint32_t cellDims[4];
	float sourceScale[64];
	struct Box
	{
		float lo[4];
		float hi[4];
	} proxies[kMaxProxies];
};
static_assert( sizeof( SdfTraceParams ) == 896, "sdf_probe_trace.comp Params" );

// One light of the trace program's light buffer (sdf_probe_trace.comp Light).
struct SdfTraceLight
{
	float kind[4];
	float rgb[4];
	float a[4];
	float b[4];
	float c[4];
	float d[4];
};
static_assert( sizeof( SdfTraceLight ) == 96, "sdf_probe_trace.comp Light" );

// Ray-query instance masks: the world, and the moving proxies (a reference
// trace, under the baked scene, sees the world only).
static constexpr uint32_t kWorldInstanceMask = 0x1;
static constexpr uint32_t kProxyInstanceMask = 0x2;

// Each probe's world position (xyz) and whether it is active (w 1 or 0), from
// a volume's first grid and its relocation offsets.
[[nodiscard]] inline std::vector<float> ProbePositions( const Volume &volume )
{
	const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
	const mapcontainer::ProbeGridLayout &grid = layout.grids[0];
	std::vector<float> out( size_t( grid.probeCount ) * 4 );
	const uint32_t row = grid.tilesPerRow * mapcontainer::kProbeVisibilityTile;
	for ( uint32_t i = 0; i < grid.probeCount; ++i )
	{
		const uint32_t index[3] = { i % grid.dims[0], ( i / grid.dims[0] ) % grid.dims[1],
		    i / ( grid.dims[0] * grid.dims[1] ) };
		uint16_t state[4];
		std::memcpy( state,
		    volume.bytes.data() + layout.atlasOffset +
		        ( uint64_t( grid.stateOrigin[1] + i / row ) * layout.atlasWidth +
		            grid.stateOrigin[0] + i % row ) *
		            8,
		    sizeof( state ) );
		for ( int k = 0; k < 3; ++k )
			out[i * 4 + k] = grid.origin[k] + float( index[k] ) * grid.spacing[k] +
			                 mapcontainer::HalfToFloat( state[k] );
		out[i * 4 + 3] = mapcontainer::HalfToFloat( state[3] );
	}
	return out;
}

// The SDFV's light cells for the trace program: per cell its first entry
// (then the entry count), and the entries' light indices. A version-1
// volume has one cell over its voxels listing every light.
struct SdfLightCells
{
	float origin[3] = {};
	float size = 1.0f;
	uint32_t dims[3] = { 1, 1, 1 };
	std::vector<uint32_t> first;
	std::vector<uint32_t> lights;

	[[nodiscard]] static SdfLightCells From( const SdfData &sdf )
	{
		const mapcontainer::SdfVolumeLayout &f = sdf.layout;
		SdfLightCells cells;
		if ( f.cellDims[0] == 0 )
		{
			const float extent = float( std::max( f.dims[0], std::max( f.dims[1], f.dims[2] ) ) );
			for ( int k = 0; k < 3; ++k )
				cells.origin[k] = f.origin[k] - 0.5f * f.voxel;
			cells.size = extent * f.voxel;
			cells.first = { 0, f.lightCount };
			for ( uint32_t l = 0; l < f.lightCount; ++l )
				cells.lights.push_back( l );
			return cells;
		}
		for ( int k = 0; k < 3; ++k )
		{
			cells.origin[k] = f.cellOrigin[k];
			cells.dims[k] = f.cellDims[k];
		}
		cells.size = f.cellSize;
		const size_t count = size_t( f.cellDims[0] ) * f.cellDims[1] * f.cellDims[2];
		cells.first.resize( count + 1 );
		std::memcpy( cells.first.data(), sdf.bytes.data() + f.cellOffset, ( count + 1 ) * 4 );
		cells.lights.resize( f.cellEntries );
		for ( uint32_t e = 0; e < f.cellEntries; ++e )
		{
			uint16_t light;
			std::memcpy( &light, sdf.bytes.data() + f.cellEntryOffset + size_t( e ) * 2, 2 );
			cells.lights[e] = light;
		}
		return cells;
	}
};

// The static world's triangles from validated WMSH bytes (its float3
// positions and 32-bit indices); null for invalid bytes.
[[nodiscard]] inline std::shared_ptr<const WorldGeometry> WorldGeometryFromMesh(
    const void *bytes, size_t size )
{
	mapcontainer::WorldMeshSummary summary = {};
	if ( mapcontainer::ValidateWorldMesh( bytes, size, &summary ) !=
	         mapcontainer::WorldMeshError::Ok ||
	     summary.indexCount == 0 || summary.indexCount % 3 )
		return nullptr;
	auto geometry = std::make_shared<WorldGeometry>();
	const unsigned char *base = static_cast<const unsigned char *>( bytes );
	geometry->positions.resize( size_t( summary.vertexCount ) * 3 );
	for ( uint32_t v = 0; v < summary.vertexCount; ++v )
		std::memcpy( &geometry->positions[size_t( v ) * 3],
		    base + summary.sectionOffsets[0] + size_t( v ) * mapcontainer::kWorldMeshVertexSize,
		    sizeof( float ) * 3 );
	geometry->indices.resize( summary.indexCount );
	std::memcpy( geometry->indices.data(), base + summary.sectionOffsets[1],
	    size_t( summary.indexCount ) * sizeof( uint32_t ) );
	return geometry;
}

// How a traced producer's rays find surfaces.
enum class TraceMode
{
	Sdf,      // sphere tracing the SDFV ("sdf-probe-trace", G6)
	RayQuery, // ray queries against the world's triangles ("ray-query-probe-trace", G7)
};

// The traced producers (SdfTracedProducer, RayQueryProducer): the same probe
// update, publication and lifetime; only the tracer differs.
class TracedProducer : public IProducer
{
public:
	explicit TracedProducer( TraceMode mode ) : m_mode( mode ) {}

	static constexpr uint32_t kConvergenceFrames = 60; // 1 s at 60 Hz
	static constexpr uint32_t kTexels = 36;
	static constexpr uint32_t kRing = 4;
	static constexpr uint32_t kMaxInFlight = 2;
	static constexpr uint32_t kPropagationUpdates = 24;
	static constexpr uint32_t kReferenceUpdates = 64;
	static constexpr uint32_t kActiveUpdates = 72;
	// Below this reference irradiance a darkening is absolute, not relative.
	static constexpr float kRelativeFloor = 1e-4f;

	[[nodiscard]] ProducerCaps Caps() const override
	{
		ProducerCaps caps;
		caps.kind = m_mode == TraceMode::Sdf ? ProducerKind::SdfTraced : ProducerKind::RayQuery;
		caps.responds = kLightIntensity | kLightMotion | kGeometryMotion | kEmission;
		caps.policies = PolicyBit( indirect_policy::Policy::BakedPlusDelta );
		// RenderFeature::kComputeShaders, and kRayQuery for ray queries.
		caps.requiredFeatures = ( 1u << 2 ) | ( m_mode == TraceMode::RayQuery ? 1u << 7 : 0u );
		// For the probes the host focuses on (every probe without a focus);
		// the others converge at the host's probe budget.
		caps.convergenceFrames = kConvergenceFrames;
		// A dispatch completes kMaxInFlight frames after its submission, so
		// the reference phase takes (kMaxInFlight + 1) / kMaxInFlight frames
		// per update.
		caps.warmupFrames =
		    kReferenceUpdates * ( kMaxInFlight + 1 ) / kMaxInFlight + 2 * kMaxInFlight + 4;
		caps.seedTolerance = 0.0f;
		caps.responseTolerance = 0.1f;
		return caps;
	}

	[[nodiscard]] foundation::Expected<void, IndirectError> Begin(
	    const IndirectScene &scene, const PublishedVolume &, IResourceTracker & ) override
	{
		const bool rayQuery = m_mode == TraceMode::RayQuery;
		if ( !scene.baked || !scene.sdf ||
		     ( rayQuery && ( !scene.geometry || scene.geometry->indices.empty() ) ) )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		if ( scene.policy != indirect_policy::Policy::BakedPlusDelta )
			return foundation::MakeUnexpected( IndirectError::UnsupportedPolicy );
		if ( !scene.gpu || !scene.gpu->Capabilities().compute ||
		     ( rayQuery && !scene.gpu->Capabilities().rayQuery ) )
			return foundation::MakeUnexpected( IndirectError::MissingFeature );
		if ( scene.baked->layout.gridCount != 1 || scene.sdf->layout.lightCount == 0 )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		m_gpu = scene.gpu;
		m_baked = scene.baked;
		m_sdf = scene.sdf;
		const mapcontainer::ProbeGridLayout &grid = m_baked->layout.grids[0];
		m_probes = grid.probeCount;
		const size_t field = size_t( m_probes ) * kTexels * 3 * 16;
		m_lights.resize( m_sdf->layout.lightCount );
		for ( uint32_t i = 0; i < m_sdf->layout.lightCount; ++i )
			std::memcpy( &m_lights[i],
			    m_sdf->bytes.data() + m_sdf->layout.lightOffset +
			        i * sizeof( mapcontainer::SdfLight ),
			    sizeof( mapcontainer::SdfLight ) );
		const SdfLightCells cells = SdfLightCells::From( *m_sdf );
		std::vector<uint32_t> made;
		const auto make = [&]( size_t bytes, gpu_compute::BufferUse use )
		{
			const uint32_t buffer = m_gpu->CreateBuffer( bytes, use );
			if ( buffer )
				made.push_back( buffer );
			return buffer;
		};
		using gpu_compute::BufferUse;
		const size_t lightBytes = m_lights.size() * sizeof( SdfTraceLight );
		m_voxels = make( m_sdf->bytes.size() - m_sdf->layout.voxelOffset, BufferUse::Upload );
		m_positions = make( size_t( m_probes ) * 16, BufferUse::Upload );
		m_bakedParams = make( sizeof( SdfTraceParams ), BufferUse::Upload );
		m_bakedLights = make( lightBytes, BufferUse::Upload );
		m_cellFirst = make( cells.first.size() * 4, BufferUse::Upload );
		m_cellLights = make( std::max<size_t>( cells.lights.size(), 1 ) * 4, BufferUse::Upload );
		for ( uint32_t k = 0; k < kRing; ++k )
		{
			// The CPU reads each field back to compose the published volume.
			m_field[k] = make( field, BufferUse::Readback );
			m_params[k] = make( sizeof( SdfTraceParams ), BufferUse::Upload );
			m_liveLights[k] = make( lightBytes, BufferUse::Upload );
			m_liveSchedule[k] = make( size_t( m_probes ) * 8, BufferUse::Upload );
			m_referenceSchedule[k] = make( size_t( m_probes ) * 8, BufferUse::Upload );
		}
		using gpu_compute::Binding;
		std::vector<Binding> bindings = { Binding::Buffer, Binding::Buffer, Binding::Buffer,
		    Binding::Buffer, Binding::Buffer };
		if ( rayQuery )
			bindings.insert( bindings.end(), { Binding::Scene, Binding::Buffer, Binding::Buffer } );
		bindings.insert( bindings.end(), 4, Binding::Buffer ); // lights, cells, entries, schedule
		m_program = m_gpu->CreateProgram( rayQuery ? "ray-query-probe-trace" : "sdf-probe-trace",
		    bindings.data(), uint32_t( bindings.size() ), 8 );
		size_t expected = 6 + 5 * kRing;
		if ( rayQuery && m_program )
		{
			// The world's triangles: the scene's instance 0, and the vertices
			// and indices the program reads a hit's normal from; a unit cube
			// scaled into place for each proxy.
			const WorldGeometry &world = *scene.geometry;
			const uint32_t vertices = uint32_t( world.positions.size() / 3 );
			m_worldVertices = make( size_t( vertices ) * 16, BufferUse::Upload );
			m_worldIndices = make( world.indices.size() * 4, BufferUse::Upload );
			expected += 2;
			if ( m_worldVertices && m_worldIndices )
			{
				float *out = static_cast<float *>( m_gpu->Map( m_worldVertices ) );
				for ( uint32_t v = 0; v < vertices; ++v )
				{
					for ( int k = 0; k < 3; ++k )
						out[v * 4 + k] = world.positions[v * 3 + k];
					out[v * 4 + 3] = 1.0f;
				}
				std::memcpy(
				    m_gpu->Map( m_worldIndices ), world.indices.data(), world.indices.size() * 4 );
			}
			m_worldGeometry = m_gpu->CreateGeometry( world.positions.data(), vertices,
			    world.indices.data(), uint32_t( world.indices.size() ) );
			m_cubeGeometry = m_gpu->CreateGeometry( kCubePositions, 8, kCubeIndices, 36 );
		}
		if ( made.size() != expected || !m_program ||
		     ( rayQuery && ( !m_worldGeometry || !m_cubeGeometry ) ) )
		{
			for ( uint32_t buffer : made )
				m_gpu->Retire( buffer, 0 );
			for ( uint32_t resource : { m_program, m_worldGeometry, m_cubeGeometry } )
				if ( resource )
					m_gpu->Retire( resource, 0 );
			m_program = m_worldGeometry = m_cubeGeometry = 0;
			m_gpu = nullptr;
			return foundation::MakeUnexpected( IndirectError::MissingFeature );
		}
		std::memcpy( m_gpu->Map( m_voxels ), m_sdf->bytes.data() + m_sdf->layout.voxelOffset,
		    m_sdf->bytes.size() - m_sdf->layout.voxelOffset );
		const std::vector<float> positions = ProbePositions( *m_baked );
		std::memcpy( m_gpu->Map( m_positions ), positions.data(), positions.size() * 4 );
		for ( uint32_t k = 0; k < kRing; ++k )
			std::memset( m_gpu->Map( m_field[k] ), 0, field );
		std::memcpy( m_gpu->Map( m_cellFirst ), cells.first.data(), cells.first.size() * 4 );
		if ( !cells.lights.empty() )
			std::memcpy( m_gpu->Map( m_cellLights ), cells.lights.data(), cells.lights.size() * 4 );
		m_cells = cells;
		m_cells.first.clear();
		m_cells.lights.clear();
		m_overrides.assign( 64, -1.0f );
		// The styles this scene's light comes from: its lights' and its
		// emissive voxels'. Only their scalars are the configuration; another
		// style animating (the stock flicker patterns) must not restart the
		// update, or the probes are re-traced and re-blended every frame.
		m_usedStyles = 0;
		for ( const mapcontainer::SdfLight &light : m_lights )
			if ( light.style >= 0 && light.style < 64 )
				m_usedStyles |= 1ull << light.style;
		const unsigned char *voxels = m_sdf->bytes.data() + m_sdf->layout.voxelOffset;
		const size_t voxelCount =
		    size_t( m_sdf->layout.dims[0] ) * m_sdf->layout.dims[1] * m_sdf->layout.dims[2];
		for ( size_t v = 0; v < voxelCount; ++v )
		{
			uint16_t source;
			std::memcpy( &source, voxels + v * mapcontainer::kSdfVoxelBytes + 14, 2 );
			if ( source < 64 )
				m_usedStyles |= 1ull << source;
		}
		// The baked scene's parameters and lights: every reference trace's.
		WriteParams( *static_cast<SdfTraceParams *>( m_gpu->Map( m_bakedParams ) ), BakedConfig(),
		    kWorldInstanceMask );
		WriteLights( static_cast<SdfTraceLight *>( m_gpu->Map( m_bakedLights ) ), BakedConfig() );
		// Per probe: an inactive probe (inside geometry) is never traced and
		// its reference is its zero field.
		m_active.assign( m_probes, 0 );
		m_referenced.assign( m_probes, 0 );
		m_referenceUpdates.assign( m_probes, 0 );
		m_liveUpdates.assign( m_probes, kActiveUpdates );
		m_reference.assign( size_t( m_probes ) * kTexels * 12, 0.0f );
		m_unreferenced = 0;
		for ( uint32_t p = 0; p < m_probes; ++p )
		{
			m_active[p] = positions[p * 4 + 3] >= 0.5f;
			m_referenced[p] = !m_active[p];
			m_unreferenced += m_active[p];
		}
		m_phase = m_unreferenced ? Phase::Reference : Phase::Live;
		m_updates = 0;
		m_cursor = 0;
		m_written = 0;
		m_recent.clear();
		m_marks.assign( m_probes, 0 );
		m_mark = 0;
		m_inFlight.clear();
		m_lastSerial = 0;
		m_frame = 0;
		m_bakedHash = BakedConfig().Hash();
		m_configHash = m_bakedHash;
		m_scene = 0;
		m_sceneProxies.clear();
		m_published = PublishedVolume{ ++m_epoch, m_baked };
		return {};
	}

	void Schedule( FrameWork &work, const light_set::Snapshot &lights ) override
	{
		if ( !m_gpu )
			return;
		for ( const SceneChange &change : work.changes )
			if ( change.response & ( kLightIntensity | kEmission ) )
				for ( uint32_t s = 0; s < 64; ++s )
					if ( change.target == kEverySource || change.target == s )
						m_overrides[s] = change.value;
		// Completed updates, oldest first: their references, then the newest
		// field's publication.
		const uint64_t completed = m_gpu->CompletedSerial();
		bool fresh = false, live = false;
		uint32_t readable = 0;
		while ( !m_inFlight.empty() && m_inFlight.front().serial <= completed )
		{
			const InFlight &done = m_inFlight.front();
			CaptureReferences( done );
			readable = done.written;
			live = live || done.live;
			m_inFlight.pop_front();
			fresh = true;
		}
		if ( fresh && live )
			Publish( readable );
		// This frame's configuration: a change restarts every referenced
		// probe's live updates.
		const Config config = LiveConfig( work, lights );
		const uint64_t hash = config.Hash();
		if ( hash != m_configHash )
		{
			m_configHash = hash;
			m_updates = 0;
			for ( uint32_t p = 0; p < m_probes; ++p )
				if ( m_active[p] && m_referenced[p] )
					m_liveUpdates[p] = 0;
		}
		if ( m_inFlight.size() < kMaxInFlight )
			Dispatch( config, work.focusProbes, work.probeBudget );
	}

	[[nodiscard]] std::optional<PublishedVolume> Published() const override { return m_published; }

	[[nodiscard]] RetireTicket End() override
	{
		RetireTicket ticket{ m_lastSerial };
		if ( m_gpu )
		{
			for ( uint32_t buffer :
			    { m_voxels, m_positions, m_bakedParams, m_bakedLights, m_cellFirst, m_cellLights } )
				m_gpu->Retire( buffer, m_lastSerial );
			for ( uint32_t k = 0; k < kRing; ++k )
				for ( uint32_t buffer : { m_field[k], m_params[k], m_liveLights[k],
				          m_liveSchedule[k], m_referenceSchedule[k] } )
					m_gpu->Retire( buffer, m_lastSerial );
			m_gpu->Retire( m_program, m_lastSerial );
			// The scene before the geometry it instances (same serial).
			for ( uint32_t resource :
			    { m_scene, m_worldGeometry, m_cubeGeometry, m_worldVertices, m_worldIndices } )
				if ( resource )
					m_gpu->Retire( resource, m_lastSerial );
		}
		m_scene = m_worldGeometry = m_cubeGeometry = m_worldVertices = m_worldIndices = 0;
		m_gpu = nullptr;
		m_published.reset();
		m_baked.reset();
		return ticket;
	}

	// Diagnostics for tests: the phase (every active probe referenced), live
	// work pending, the updates since the last change, and probe updates
	// (traced probes, all dispatches) so far.
	[[nodiscard]] bool Live() const { return m_phase == Phase::Live; }
	[[nodiscard]] bool Active() const
	{
		for ( uint32_t p = 0; p < m_probes; ++p )
			if ( m_active[p] && m_referenced[p] && m_liveUpdates[p] < kActiveUpdates )
				return true;
		return false;
	}
	[[nodiscard]] uint32_t UpdatesSinceChange() const { return m_updates; }
	[[nodiscard]] uint64_t ProbeUpdates() const { return m_probeUpdates; }
	[[nodiscard]] bool Referenced( uint32_t probe ) const
	{
		return probe < m_probes && m_referenced[probe];
	}
	[[nodiscard]] uint32_t LiveUpdates( uint32_t probe ) const
	{
		return probe < m_probes ? m_liveUpdates[probe] : 0;
	}
	// The converged baked-scene field: per probe, per texel, total, indirect
	// and analytic direct (4 floats each); a probe's is zero until captured.
	[[nodiscard]] const std::vector<float> &ReferenceField() const { return m_reference; }

private:
	enum class Phase
	{
		Reference,
		Live,
	};

	struct Config
	{
		float scale[64];
		std::vector<float> directions; // 3 per light
		std::vector<Proxy> proxies;

		uint64_t Hash() const
		{
			uint64_t h = 1469598103934665603ull;
			const auto mix = [&]( const void *p, size_t n )
			{
				const unsigned char *b = static_cast<const unsigned char *>( p );
				for ( size_t i = 0; i < n; ++i )
					h = ( h ^ b[i] ) * 1099511628211ull;
			};
			mix( scale, sizeof( scale ) );
			mix( directions.data(), directions.size() * sizeof( float ) );
			for ( const Proxy &proxy : proxies )
				mix( &proxy, sizeof( proxy ) );
			return h;
		}
	};

	// One update: its serial and field slot, the probes whose reference its
	// completion captures, and whether it traced live probes.
	struct InFlight
	{
		uint64_t serial;
		uint32_t written;
		std::vector<uint32_t> referencedNow;
		bool live;
	};

	// A probe's blend weight for its next update: half-and-half first, so
	// each update propagates a bounce (the error falls by about (1 + rho) / 2
	// per update), then a running mean of the settled updates, which averages
	// the ray noise away.
	static float Alpha( uint32_t updates )
	{
		return updates < kPropagationUpdates
		           ? 0.5f
		           : std::max( 0.02f, 1.0f / float( updates - kPropagationUpdates + 2 ) );
	}

	Config BakedConfig() const
	{
		Config config;
		std::fill( std::begin( config.scale ), std::end( config.scale ), 1.0f );
		config.directions.resize( m_lights.size() * 3 );
		for ( size_t l = 0; l < m_lights.size(); ++l )
			for ( int k = 0; k < 3; ++k )
				config.directions[l * 3 + k] = m_lights[l].a[k];
		return config;
	}

	Config LiveConfig( const FrameWork &work, const light_set::Snapshot &lights ) const
	{
		Config config = BakedConfig();
		for ( uint32_t s = 0; s < 64; ++s )
		{
			if ( !( m_usedStyles & ( 1ull << s ) ) )
				continue;
			if ( m_overrides[s] >= 0.0f )
				config.scale[s] = m_overrides[s];
			else if ( s < lights.styleScalars.size() )
				config.scale[s] = lights.styleScalars[s];
		}
		for ( const LightOverride &light : work.lightOverrides )
			for ( size_t l = 0; l < m_lights.size(); ++l )
				if ( m_lights[l].style == light.style &&
				     m_lights[l].kind == uint32_t( mapcontainer::SdfLightKind::Distant ) )
					for ( int k = 0; k < 3; ++k )
						config.directions[l * 3 + k] = light.direction[k];
		for ( size_t p = 0; p < work.proxies.size() && p < SdfTraceParams::kMaxProxies; ++p )
			config.proxies.push_back( work.proxies[p] );
		return config;
	}

	void WriteParams( SdfTraceParams &params, const Config &config, uint32_t mask ) const
	{
		std::memset( &params, 0, sizeof( params ) );
		const mapcontainer::SdfVolumeLayout &sdf = m_sdf->layout;
		const mapcontainer::ProbeGridLayout &grid = m_baked->layout.grids[0];
		for ( int k = 0; k < 3; ++k )
		{
			params.sdfOrigin[k] = sdf.origin[k];
			params.sdfDims[k] = sdf.dims[k];
			params.probeOrigin[k] = grid.origin[k];
			params.probeSpacing[k] = grid.spacing[k];
			params.probeDims[k] = grid.dims[k];
			params.cellOrigin[k] = m_cells.origin[k];
			params.cellDims[k] = m_cells.dims[k];
		}
		params.sdfOrigin[3] = sdf.voxel;
		params.sdfDims[3] = uint32_t( m_lights.size() );
		params.probeOrigin[3] =
		    mapcontainer::kProbeNormalBias *
		    std::min( grid.spacing[0], std::min( grid.spacing[1], grid.spacing[2] ) );
		params.probeSpacing[3] = sdf.maxDistance;
		params.probeDims[3] = m_probes;
		params.cellOrigin[3] = m_cells.size;
		params.counts[0] = uint32_t( config.proxies.size() );
		params.counts[1] = mask;
		std::copy( std::begin( config.scale ), std::end( config.scale ), params.sourceScale );
		for ( size_t p = 0; p < config.proxies.size(); ++p )
		{
			for ( int k = 0; k < 3; ++k )
			{
				params.proxies[p].lo[k] = config.proxies[p].lo[k];
				params.proxies[p].hi[k] = config.proxies[p].hi[k];
			}
			params.proxies[p].lo[3] = config.proxies[p].reflectance;
		}
	}

	void WriteLights( SdfTraceLight *out, const Config &config ) const
	{
		for ( size_t l = 0; l < m_lights.size(); ++l )
		{
			const mapcontainer::SdfLight &light = m_lights[l];
			const float scale = light.style >= 0 ? config.scale[light.style] : 1.0f;
			SdfTraceLight &record = out[l];
			std::memset( &record, 0, sizeof( record ) );
			record.kind[0] = float( light.kind );
			for ( int k = 0; k < 3; ++k )
			{
				record.rgb[k] = light.rgb[k] * scale;
				record.a[k] = light.kind == uint32_t( mapcontainer::SdfLightKind::Distant )
				                  ? config.directions[l * 3 + k]
				                  : light.a[k];
				record.b[k] = light.b[k];
				record.c[k] = light.c[k];
			}
			record.d[0] = light.reserved[0];
			record.d[1] = light.reserved[1];
		}
	}

	// Whether probe p has work: its reference to build, or live updates to
	// run after a change (a probe whose reference is in flight waits).
	bool NeedsReference( uint32_t p ) const
	{
		return m_active[p] && !m_referenced[p] && m_referenceUpdates[p] < kReferenceUpdates;
	}
	bool NeedsLive( uint32_t p ) const
	{
		return m_active[p] && m_referenced[p] && m_liveUpdates[p] < kActiveUpdates;
	}

	// One update: the probes to trace are the focus's (every probe without a
	// focus) and, round-robin, `budget` more. The live dispatch traces the
	// referenced ones under this frame's configuration and carries forward
	// (a weight of 0) every probe the ring slot it writes is stale for: those
	// the last kRing - 1 updates traced (each slot holds every probe's
	// estimate as of its own update, so only those changed since). The
	// reference dispatch then traces the unreferenced ones under the baked
	// scene.
	void Dispatch( const Config &config, std::span<const uint32_t> focus, uint32_t budget )
	{
		std::vector<uint32_t> chosen;
		const auto consider = [&]( uint32_t p )
		{
			if ( p < m_probes && ( NeedsReference( p ) || NeedsLive( p ) ) )
				chosen.push_back( p );
		};
		if ( focus.empty() )
			for ( uint32_t p = 0; p < m_probes; ++p )
				consider( p );
		else
		{
			std::vector<uint8_t> taken( m_probes, 0 );
			for ( uint32_t p : focus )
				if ( p < m_probes && !taken[p] )
				{
					taken[p] = 1;
					consider( p );
				}
			for ( uint32_t n = 0, added = 0; n < m_probes && added < budget; ++n )
			{
				const uint32_t p = ( m_cursor + n ) % m_probes;
				if ( taken[p] || !( NeedsReference( p ) || NeedsLive( p ) ) )
					continue;
				chosen.push_back( p );
				++added;
				m_cursor = ( p + 1 ) % m_probes;
			}
		}
		if ( chosen.empty() )
			return;
		const bool rayQuery = m_mode == TraceMode::RayQuery;
		if ( rayQuery && ( !m_scene || config.proxies != m_sceneProxies ) && !BuildScene( config ) )
			return;
		const uint32_t slot = ( m_written + 1 ) % kRing;
		uint32_t *liveSchedule = static_cast<uint32_t *>( m_gpu->Map( m_liveSchedule[slot] ) );
		uint32_t *referenceSchedule =
		    static_cast<uint32_t *>( m_gpu->Map( m_referenceSchedule[slot] ) );
		// The probes this slot is stale for, less those traced now.
		++m_mark;
		for ( uint32_t p : chosen )
			m_marks[p] = m_mark;
		uint32_t lives = 0;
		for ( const std::vector<uint32_t> &recent : m_recent )
			for ( uint32_t p : recent )
				if ( m_marks[p] != m_mark )
				{
					m_marks[p] = m_mark;
					liveSchedule[lives * 2] = p;
					liveSchedule[lives * 2 + 1] = 0; // 0.0f: carried forward
					++lives;
				}
		InFlight update{ 0, slot, {}, false };
		uint32_t references = 0;
		for ( uint32_t p : chosen )
		{
			float alpha;
			if ( NeedsLive( p ) )
			{
				alpha = Alpha( m_liveUpdates[p]++ );
				liveSchedule[lives * 2] = p;
				std::memcpy( &liveSchedule[lives * 2 + 1], &alpha, 4 );
				++lives;
				update.live = true;
			}
			else
			{
				alpha = Alpha( m_referenceUpdates[p]++ );
				referenceSchedule[references * 2] = p;
				std::memcpy( &referenceSchedule[references * 2 + 1], &alpha, 4 );
				++references;
				if ( m_referenceUpdates[p] == kReferenceUpdates )
					update.referencedNow.push_back( p );
			}
		}
		WriteParams( *static_cast<SdfTraceParams *>( m_gpu->Map( m_params[slot] ) ), config,
		    kWorldInstanceMask | kProxyInstanceMask );
		WriteLights( static_cast<SdfTraceLight *>( m_gpu->Map( m_liveLights[slot] ) ), config );
		const uint32_t previous = m_field[m_written % kRing];
		const auto queue = [&]( uint32_t params, uint32_t lights, uint32_t schedule, uint32_t count )
		{
			struct
			{
				uint32_t entries;
				uint32_t frame;
			} push = { count, ++m_frame };
			std::vector<uint32_t> buffers = { m_voxels, params, m_positions, previous,
			    m_field[slot] };
			if ( rayQuery )
				buffers.insert( buffers.end(), { m_scene, m_worldVertices, m_worldIndices } );
			buffers.insert( buffers.end(), { lights, m_cellFirst, m_cellLights, schedule } );
			return m_gpu->QueueDispatch( m_program, buffers.data(), uint32_t( buffers.size() ),
			    &push, sizeof( push ), count, 1, 1 );
		};
		uint64_t serial = 0;
		if ( lives )
			serial = queue( m_params[slot], m_liveLights[slot], m_liveSchedule[slot], lives );
		if ( ( serial || !lives ) && references )
			serial = queue( m_bakedParams, m_bakedLights, m_referenceSchedule[slot], references );
		if ( !serial )
			return;
		m_recent.push_back( chosen );
		if ( m_recent.size() > kRing - 1 )
			m_recent.pop_front();
		m_written = m_written + 1;
		m_lastSerial = serial;
		m_probeUpdates += chosen.size();
		update.serial = serial;
		m_inFlight.push_back( std::move( update ) );
		++m_updates;
	}

	// The ray-query scene for this configuration's proxies: the world plus a
	// unit cube per proxy (customIndex 1 + its index, as the program reads
	// it; masked apart from the world so reference traces skip them). The
	// previous scene retires behind the last dispatch that used it.
	bool BuildScene( const Config &config )
	{
		std::vector<gpu_compute::SceneInstance> instances( 1 + config.proxies.size() );
		instances[0].geometry = m_worldGeometry;
		instances[0].mask = kWorldInstanceMask;
		for ( size_t p = 0; p < config.proxies.size(); ++p )
		{
			const Proxy &proxy = config.proxies[p];
			gpu_compute::SceneInstance &instance = instances[1 + p];
			instance.geometry = m_cubeGeometry;
			instance.customIndex = uint32_t( 1 + p );
			instance.mask = kProxyInstanceMask;
			for ( int k = 0; k < 3; ++k )
			{
				for ( int c = 0; c < 3; ++c )
					instance.transform[k * 4 + c] = k == c ? proxy.hi[k] - proxy.lo[k] : 0.0f;
				instance.transform[k * 4 + 3] = proxy.lo[k];
			}
		}
		const uint32_t scene = m_gpu->CreateScene( instances.data(), uint32_t( instances.size() ) );
		if ( !scene )
			return false;
		if ( m_scene )
			m_gpu->Retire( m_scene, m_lastSerial );
		m_scene = scene;
		m_sceneProxies = config.proxies;
		return true;
	}

	// A completed update's references: each probe that finished its baked
	// scene's convergence keeps that field; under a changed configuration it
	// then needs live updates.
	void CaptureReferences( const InFlight &done )
	{
		if ( done.referencedNow.empty() )
			return;
		const float *field = static_cast<const float *>( m_gpu->Map( m_field[done.written] ) );
		const size_t stride = size_t( kTexels ) * 12;
		for ( uint32_t p : done.referencedNow )
		{
			std::copy( field + p * stride, field + ( p + 1 ) * stride,
			    m_reference.begin() + std::ptrdiff_t( p * stride ) );
			m_referenced[p] = 1;
			m_liveUpdates[p] = m_configHash == m_bakedHash ? kActiveUpdates : 0;
			--m_unreferenced;
		}
		if ( !m_unreferenced && m_phase == Phase::Reference )
		{
			m_phase = Phase::Live;
			m_updates = 0;
		}
	}

	// The published volume from a completed field: each referenced probe's
	// change from its reference applied to the bake; an unreferenced probe
	// stays the bake.
	void Publish( uint32_t readable )
	{
		const float *field = static_cast<const float *>( m_gpu->Map( m_field[readable] ) );
		const size_t count = size_t( m_probes ) * kTexels;
		m_total.assign( count * 3, 0.0f );
		m_indirect.assign( count * 3, 0.0f );
		for ( uint32_t probe = 0; probe < m_probes; ++probe )
		{
			if ( !m_referenced[probe] || !m_active[probe] )
				continue;
			const float *baseTotal = m_composer.BaseInterior( *m_baked, m_probes, probe, 0 );
			const float *baseIndirect = m_composer.BaseInterior( *m_baked, m_probes, probe, 1 );
			for ( uint32_t k = 0; k < kTexels * 3; ++k )
			{
				const size_t t = size_t( probe ) * kTexels + k / 3;
				const int c = int( k % 3 );
				m_total[t * 3 + c] =
				    Change( field[t * 12 + c], m_reference[t * 12 + c], baseTotal[k] );
				m_indirect[t * 3 + c] =
				    Change( field[t * 12 + 4 + c], m_reference[t * 12 + 4 + c], baseIndirect[k] );
			}
		}
		m_published = PublishedVolume{ ++m_epoch,
		    m_composer.Compose( *m_baked, m_total.data(), m_indirect.data(), m_probes, nullptr ) };
	}

	// The change of one texel from the bake. Light removed is relative (the
	// bake times the field's ratio to its reference), so the field's bias
	// cancels and a probe the field sees go dark goes dark; light added is
	// absolute (a dark baked texel has no ratio).
	static float Change( float now, float reference, float base )
	{
		if ( now >= reference || reference <= kRelativeFloor )
			return now - reference;
		return base * ( std::max( now, 0.0f ) / reference - 1.0f );
	}

	// A unit cube, outward-wound, for proxy instances.
	static constexpr float kCubePositions[24] = {
	    0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1 };
	static constexpr uint32_t kCubeIndices[36] = { 0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0,
	    5, 4, 3, 7, 6, 3, 6, 2, 0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5 };

	TraceMode m_mode;
	uint64_t m_usedStyles = 0; // bit s: light style s lights this scene
	uint32_t m_worldVertices = 0;
	uint32_t m_worldIndices = 0;
	uint32_t m_worldGeometry = 0;
	uint32_t m_cubeGeometry = 0;
	uint32_t m_scene = 0;
	std::vector<Proxy> m_sceneProxies;
	gpu_compute::IGpuCompute *m_gpu = nullptr;
	std::shared_ptr<const Volume> m_baked;
	std::shared_ptr<const SdfData> m_sdf;
	std::vector<mapcontainer::SdfLight> m_lights;
	SdfLightCells m_cells; // the grid only; its lists are on the GPU
	std::vector<float> m_overrides;
	std::vector<float> m_reference;
	std::vector<float> m_total;
	std::vector<float> m_indirect;
	std::vector<uint8_t> m_active;
	std::vector<uint8_t> m_referenced;
	std::vector<uint32_t> m_referenceUpdates;
	std::vector<uint32_t> m_liveUpdates;
	uint32_t m_unreferenced = 0;
	uint32_t m_cursor = 0;
	// The probes each of the last kRing - 1 updates traced (what the next
	// slot is stale for), and a scratch mark per probe.
	std::deque<std::vector<uint32_t>> m_recent;
	std::vector<uint32_t> m_marks;
	uint32_t m_mark = 0;
	uint64_t m_probeUpdates = 0;
	ChangeComposer m_composer;
	uint32_t m_probes = 0;
	uint32_t m_voxels = 0;
	uint32_t m_positions = 0;
	uint32_t m_bakedParams = 0;
	uint32_t m_bakedLights = 0;
	uint32_t m_cellFirst = 0;
	uint32_t m_cellLights = 0;
	uint32_t m_field[kRing] = {};
	uint32_t m_params[kRing] = {};
	uint32_t m_liveLights[kRing] = {};
	uint32_t m_liveSchedule[kRing] = {};
	uint32_t m_referenceSchedule[kRing] = {};
	uint32_t m_program = 0;
	uint32_t m_written = 0;
	std::deque<InFlight> m_inFlight;
	uint64_t m_lastSerial = 0;
	uint64_t m_configHash = 0;
	uint64_t m_bakedHash = 0;
	uint32_t m_updates = 0;
	uint32_t m_frame = 0;
	Phase m_phase = Phase::Reference;
	std::optional<PublishedVolume> m_published;
	uint64_t m_epoch = 0;
};

class SdfTracedProducer final : public TracedProducer
{
public:
	SdfTracedProducer() : TracedProducer( TraceMode::Sdf ) {}
};

class RayQueryProducer final : public TracedProducer
{
public:
	RayQueryProducer() : TracedProducer( TraceMode::RayQuery ) {}
};

} // namespace indirect_light

#endif // RENDER_INDIRECT_SDF_H
