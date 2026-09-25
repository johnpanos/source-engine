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
//          Phases. Reference: the field converges under the baked
//          configuration (scalars 1, baked light directions, no proxies);
//          meanwhile the seed stays published. Live: when the configuration
//          (style scalars, light directions, proxies) changes, the producer
//          updates with a hysteresis that starts fast and settles, publishing
//          every completed update, then stops dispatching until the next
//          change: a still scene costs nothing.
//
//          GPU lifetime. Four field and parameter buffers form a ring; at
//          most two dispatches are in flight, so the buffer the CPU reads was
//          written by a completed dispatch and nothing in flight writes it.
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
#include <vector>

namespace indirect_light
{

// The trace program's parameter block (sdf_probe_trace.comp, std430).
struct SdfTraceParams
{
	static constexpr uint32_t kMaxLights = 16;
	static constexpr uint32_t kMaxProxies = 16;
	float sdfOrigin[4];
	uint32_t sdfDims[4];
	float probeOrigin[4];
	float probeSpacing[4];
	uint32_t probeDims[4];
	uint32_t counts[4];
	float sourceScale[64];
	struct Light
	{
		float kind[4];
		float rgb[4];
		float a[4];
		float b[4];
		float c[4];
	} lights[kMaxLights];
	struct Box
	{
		float lo[4];
		float hi[4];
	} proxies[kMaxProxies];
};
static_assert( sizeof( SdfTraceParams ) == 2144, "sdf_probe_trace.comp Params" );

// The static world's triangles from validated WMSH bytes (its float3
// positions and 32-bit indices); null for invalid bytes.
[[nodiscard]] inline std::shared_ptr<const WorldGeometry> WorldGeometryFromMesh(
    const void *bytes, size_t size )
{
	mapcontainer::WorldMeshSummary summary = {};
	if ( mapcontainer::ValidateWorldMesh( bytes, size, &summary ) != mapcontainer::WorldMeshError::Ok ||
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
		if ( scene.baked->layout.gridCount != 1 || scene.sdf->layout.lightCount > SdfTraceParams::kMaxLights )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		m_gpu = scene.gpu;
		m_baked = scene.baked;
		m_sdf = scene.sdf;
		const mapcontainer::ProbeGridLayout &grid = m_baked->layout.grids[0];
		m_probes = grid.probeCount;
		const size_t field = size_t( m_probes ) * kTexels * 3 * 16;
		std::vector<uint32_t> made;
		const auto make = [&]( size_t bytes, gpu_compute::BufferUse use )
		{
			const uint32_t buffer = m_gpu->CreateBuffer( bytes, use );
			if ( buffer )
				made.push_back( buffer );
			return buffer;
		};
		using gpu_compute::BufferUse;
		m_voxels = make( m_sdf->bytes.size() - m_sdf->layout.voxelOffset, BufferUse::Upload );
		m_positions = make( size_t( m_probes ) * 16, BufferUse::Upload );
		for ( uint32_t k = 0; k < kRing; ++k )
		{
			// The CPU reads each field back to compose the published volume.
			m_field[k] = make( field, BufferUse::Readback );
			m_params[k] = make( sizeof( SdfTraceParams ), BufferUse::Upload );
		}
		using gpu_compute::Binding;
		const Binding bindings[8] = { Binding::Buffer, Binding::Buffer, Binding::Buffer,
			Binding::Buffer, Binding::Buffer, Binding::Scene, Binding::Buffer, Binding::Buffer };
		m_program = m_gpu->CreateProgram(
		    rayQuery ? "ray-query-probe-trace" : "sdf-probe-trace", bindings, rayQuery ? 8 : 5, 8 );
		size_t expected = 2 + 2 * kRing;
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
				std::memcpy( m_gpu->Map( m_worldIndices ), world.indices.data(),
				    world.indices.size() * 4 );
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
		WriteProbePositions( grid );
		for ( uint32_t k = 0; k < kRing; ++k )
			std::memset( m_gpu->Map( m_field[k] ), 0, field );
		m_lights.resize( m_sdf->layout.lightCount );
		for ( uint32_t i = 0; i < m_sdf->layout.lightCount; ++i )
			std::memcpy( &m_lights[i],
			    m_sdf->bytes.data() + m_sdf->layout.lightOffset + i * sizeof( mapcontainer::SdfLight ),
			    sizeof( mapcontainer::SdfLight ) );
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
		const size_t voxelCount = size_t( m_sdf->layout.dims[0] ) * m_sdf->layout.dims[1] *
		                          m_sdf->layout.dims[2];
		for ( size_t v = 0; v < voxelCount; ++v )
		{
			uint16_t source;
			std::memcpy( &source, voxels + v * mapcontainer::kSdfVoxelBytes + 14, 2 );
			if ( source < 64 )
				m_usedStyles |= 1ull << source;
		}
		m_reference.clear();
		m_phase = Phase::Reference;
		m_updates = 0;
		m_active = false;
		m_written = 0;
		m_consumed = 0;
		m_inFlight.clear();
		m_lastSerial = 0;
		m_frame = 0;
		m_configHash = BakedConfigHash();
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
		// Completed dispatches: the newest one's field is readable.
		const uint64_t completed = m_gpu->CompletedSerial();
		bool fresh = false;
		while ( !m_inFlight.empty() && m_inFlight.front().serial <= completed )
		{
			m_readable = m_inFlight.front().written;
			m_inFlight.pop_front();
			fresh = true;
		}
		if ( fresh )
			Consume();
		// This frame's configuration.
		Config config = LiveConfig( work, lights );
		if ( m_phase == Phase::Live )
		{
			const uint64_t hash = config.Hash();
			if ( hash != m_configHash )
			{
				m_configHash = hash;
				m_updates = 0;
				m_active = true;
			}
		}
		else
			config = BakedConfig();
		const bool wanted =
		    ( m_phase == Phase::Reference && m_updates < kReferenceUpdates ) || m_active;
		if ( wanted && m_inFlight.size() < kMaxInFlight )
			Dispatch( config );
	}

	[[nodiscard]] std::optional<PublishedVolume> Published() const override { return m_published; }

	[[nodiscard]] RetireTicket End() override
	{
		RetireTicket ticket{ m_lastSerial };
		if ( m_gpu )
		{
			for ( uint32_t buffer : { m_voxels, m_positions } )
				m_gpu->Retire( buffer, m_lastSerial );
			for ( uint32_t k = 0; k < kRing; ++k )
			{
				m_gpu->Retire( m_field[k], m_lastSerial );
				m_gpu->Retire( m_params[k], m_lastSerial );
			}
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

	// Diagnostics for tests: the phase, and the updates since the last change.
	[[nodiscard]] bool Live() const { return m_phase == Phase::Live; }
	[[nodiscard]] bool Active() const { return m_active; }
	[[nodiscard]] uint32_t UpdatesSinceChange() const { return m_updates; }
	// The converged baked-scene field: per probe, per texel, total, indirect
	// and analytic direct (4 floats each).
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
		float directions[SdfTraceParams::kMaxLights][3];
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
			mix( directions, sizeof( directions ) );
			for ( const Proxy &proxy : proxies )
				mix( &proxy, sizeof( proxy ) );
			return h;
		}
	};

	struct InFlight
	{
		uint64_t serial;
		uint32_t written;
	};

	void WriteProbePositions( const mapcontainer::ProbeGridLayout &grid )
	{
		const mapcontainer::ProbeVolumeLayout &layout = m_baked->layout;
		float *out = static_cast<float *>( m_gpu->Map( m_positions ) );
		const uint32_t row = grid.tilesPerRow * mapcontainer::kProbeVisibilityTile;
		for ( uint32_t i = 0; i < grid.probeCount; ++i )
		{
			const uint32_t index[3] = { i % grid.dims[0], ( i / grid.dims[0] ) % grid.dims[1],
				i / ( grid.dims[0] * grid.dims[1] ) };
			uint16_t state[4];
			std::memcpy( state,
			    m_baked->bytes.data() + layout.atlasOffset +
			        ( uint64_t( grid.stateOrigin[1] + i / row ) * layout.atlasWidth +
			            grid.stateOrigin[0] + i % row ) *
			            8,
			    sizeof( state ) );
			for ( int k = 0; k < 3; ++k )
				out[i * 4 + k] = grid.origin[k] + float( index[k] ) * grid.spacing[k] +
				                 mapcontainer::HalfToFloat( state[k] );
			out[i * 4 + 3] = mapcontainer::HalfToFloat( state[3] );
		}
	}

	Config BakedConfig() const
	{
		Config config;
		std::fill( std::begin( config.scale ), std::end( config.scale ), 1.0f );
		for ( size_t l = 0; l < SdfTraceParams::kMaxLights; ++l )
			for ( int k = 0; k < 3; ++k )
				config.directions[l][k] = l < m_lights.size() ? m_lights[l].a[k] : 0.0f;
		return config;
	}

	uint64_t BakedConfigHash() const { return BakedConfig().Hash(); }

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
						config.directions[l][k] = light.direction[k];
		for ( size_t p = 0; p < work.proxies.size() && p < SdfTraceParams::kMaxProxies; ++p )
			config.proxies.push_back( work.proxies[p] );
		return config;
	}

	void Dispatch( const Config &config )
	{
		const uint32_t slot = ( m_written + 1 ) % kRing;
		SdfTraceParams &params = *static_cast<SdfTraceParams *>( m_gpu->Map( m_params[slot] ) );
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
		}
		params.sdfOrigin[3] = sdf.voxel;
		params.sdfDims[3] = uint32_t( m_lights.size() );
		params.probeOrigin[3] =
		    mapcontainer::kProbeNormalBias *
		    std::min( grid.spacing[0], std::min( grid.spacing[1], grid.spacing[2] ) );
		params.probeSpacing[3] = sdf.maxDistance;
		params.probeDims[3] = m_probes;
		params.counts[0] = uint32_t( config.proxies.size() );
		std::copy( std::begin( config.scale ), std::end( config.scale ), params.sourceScale );
		for ( size_t l = 0; l < m_lights.size(); ++l )
		{
			const mapcontainer::SdfLight &light = m_lights[l];
			const float scale = light.style >= 0 ? config.scale[light.style] : 1.0f;
			params.lights[l].kind[0] = float( light.kind );
			for ( int k = 0; k < 3; ++k )
			{
				params.lights[l].rgb[k] = light.rgb[k] * scale;
				params.lights[l].a[k] = light.kind == uint32_t( mapcontainer::SdfLightKind::Distant )
				                            ? config.directions[l][k]
				                            : light.a[k];
				params.lights[l].b[k] = light.b[k];
				params.lights[l].c[k] = light.c[k];
			}
		}
		for ( size_t p = 0; p < config.proxies.size(); ++p )
		{
			for ( int k = 0; k < 3; ++k )
			{
				params.proxies[p].lo[k] = config.proxies[p].lo[k];
				params.proxies[p].hi[k] = config.proxies[p].hi[k];
			}
			params.proxies[p].lo[3] = config.proxies[p].reflectance;
		}
		// Hysteresis, in both phases: half-and-half blending first, so each
		// update propagates a bounce (the error falls by about (1 + rho) / 2 per
		// update), then a running mean of the settled updates, which averages
		// the ray noise away.
		const float alpha = m_updates < kPropagationUpdates
		                        ? 0.5f
		                        : std::max( 0.02f, 1.0f / float( m_updates - kPropagationUpdates + 2 ) );
		struct
		{
			float alpha;
			uint32_t frame;
		} push = { alpha, ++m_frame };
		const bool rayQuery = m_mode == TraceMode::RayQuery;
		if ( rayQuery && ( !m_scene || config.proxies != m_sceneProxies ) && !BuildScene( config ) )
			return;
		const uint32_t buffers[8] = { m_voxels, m_params[slot], m_positions,
			m_field[m_written % kRing], m_field[slot], m_scene, m_worldVertices, m_worldIndices };
		const uint64_t serial = m_gpu->QueueDispatch(
		    m_program, buffers, rayQuery ? 8 : 5, &push, sizeof( push ), m_probes, 1, 1 );
		if ( !serial )
			return;
		m_written = m_written + 1;
		m_lastSerial = serial;
		m_inFlight.push_back( { serial, slot } );
		++m_updates;
	}

	// The ray-query scene for this configuration's proxies: the world plus a
	// unit cube per proxy (customIndex 1 + its index, as the program reads
	// it). The previous scene retires behind the last dispatch that used it.
	bool BuildScene( const Config &config )
	{
		std::vector<gpu_compute::SceneInstance> instances( 1 + config.proxies.size() );
		instances[0].geometry = m_worldGeometry;
		for ( size_t p = 0; p < config.proxies.size(); ++p )
		{
			const Proxy &proxy = config.proxies[p];
			gpu_compute::SceneInstance &instance = instances[1 + p];
			instance.geometry = m_cubeGeometry;
			instance.customIndex = uint32_t( 1 + p );
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

	void Consume()
	{
		const float *field = static_cast<const float *>( m_gpu->Map( m_field[m_readable] ) );
		const size_t count = size_t( m_probes ) * kTexels;
		if ( m_phase == Phase::Reference )
		{
			if ( m_updates < kReferenceUpdates || !m_inFlight.empty() )
				return;
			m_reference.assign( field, field + count * 12 );
			m_phase = Phase::Live;
			m_updates = 0;
			return;
		}
		if ( !m_active )
			return;
		m_total.resize( count * 3 );
		m_indirect.resize( count * 3 );
		for ( uint32_t probe = 0; probe < m_probes; ++probe )
		{
			const float *baseTotal = m_composer.BaseInterior( *m_baked, m_probes, probe, 0 );
			const float *baseIndirect = m_composer.BaseInterior( *m_baked, m_probes, probe, 1 );
			for ( uint32_t k = 0; k < kTexels * 3; ++k )
			{
				const size_t t = size_t( probe ) * kTexels + k / 3;
				const int c = int( k % 3 );
				m_total[t * 3 + c] = Change( field[t * 12 + c], m_reference[t * 12 + c], baseTotal[k] );
				m_indirect[t * 3 + c] =
				    Change( field[t * 12 + 4 + c], m_reference[t * 12 + 4 + c], baseIndirect[k] );
			}
		}
		m_published =
		    PublishedVolume{ ++m_epoch, m_composer.Compose( *m_baked, m_total.data(),
		                                    m_indirect.data(), m_probes, nullptr ) };
		if ( m_updates >= kActiveUpdates )
			m_active = false;
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
	static constexpr float kCubePositions[24] = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1,
		0, 1, 1, 1, 1, 0, 1, 1 };
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
	std::vector<float> m_overrides;
	std::vector<float> m_reference;
	std::vector<float> m_total;
	std::vector<float> m_indirect;
	ChangeComposer m_composer;
	uint32_t m_probes = 0;
	uint32_t m_voxels = 0;
	uint32_t m_positions = 0;
	uint32_t m_field[kRing] = {};
	uint32_t m_params[kRing] = {};
	uint32_t m_program = 0;
	uint32_t m_written = 0;
	uint32_t m_readable = 0;
	uint32_t m_consumed = 0;
	std::deque<InFlight> m_inFlight;
	uint64_t m_lastSerial = 0;
	uint64_t m_configHash = 0;
	uint32_t m_updates = 0;
	uint32_t m_frame = 0;
	bool m_active = false;
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
