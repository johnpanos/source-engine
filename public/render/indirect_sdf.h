//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDF-traced producer (RFC 0011 G6, render.indirect-light.v1):
//          probes trace rays through the map's signed distance volume (SDFV)
//          and this frame's proxy boxes on the GPU (the renderer's
//          "sdf-probe-trace" program through render/gpu_compute.h), so moved
//          lights and moving occluders change indirect light.
//
//          Publication (render.indirect-policy.v1 BakedPlusDelta, as the
//          radiosity producer): the baked volume plus the change of the
//          traced field from its own converged estimate of the baked scene.
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
//          End retires every buffer behind the last dispatch's serial.
//
//===========================================================================//

#ifndef RENDER_INDIRECT_SDF_H
#define RENDER_INDIRECT_SDF_H

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

class SdfTracedProducer final : public IProducer
{
public:
	static constexpr uint32_t kConvergenceFrames = 60; // 1 s at 60 Hz
	static constexpr uint32_t kTexels = 36;
	static constexpr uint32_t kRing = 4;
	static constexpr uint32_t kMaxInFlight = 2;
	static constexpr uint32_t kPropagationUpdates = 24;
	static constexpr uint32_t kReferenceUpdates = 64;
	static constexpr uint32_t kActiveUpdates = 72;

	[[nodiscard]] ProducerCaps Caps() const override
	{
		ProducerCaps caps;
		caps.kind = ProducerKind::SdfTraced;
		caps.responds = kLightIntensity | kLightMotion | kGeometryMotion | kEmission;
		caps.policies = PolicyBit( indirect_policy::Policy::BakedPlusDelta );
		caps.requiredFeatures = 1u << 2; // RenderFeature::kComputeShaders
		caps.convergenceFrames = kConvergenceFrames;
		caps.warmupFrames = kReferenceUpdates + 2 * kMaxInFlight + 4;
		caps.seedTolerance = 0.0f;
		caps.responseTolerance = 0.1f;
		return caps;
	}

	[[nodiscard]] foundation::Expected<void, IndirectError> Begin(
	    const IndirectScene &scene, const PublishedVolume &, IResourceTracker & ) override
	{
		if ( !scene.baked || !scene.sdf )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		if ( scene.policy != indirect_policy::Policy::BakedPlusDelta )
			return foundation::MakeUnexpected( IndirectError::UnsupportedPolicy );
		if ( !scene.gpu || !scene.gpu->Capabilities().compute )
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
		const auto make = [&]( size_t bytes )
		{
			const uint32_t buffer = m_gpu->CreateBuffer( bytes );
			if ( buffer )
				made.push_back( buffer );
			return buffer;
		};
		m_voxels = make( m_sdf->bytes.size() - m_sdf->layout.voxelOffset );
		m_positions = make( size_t( m_probes ) * 16 );
		for ( uint32_t k = 0; k < kRing; ++k )
		{
			m_field[k] = make( field );
			m_params[k] = make( sizeof( SdfTraceParams ) );
		}
		m_program = m_gpu->CreateProgram( "sdf-probe-trace", 5, 8 );
		if ( made.size() != 2 + 2 * kRing || !m_program )
		{
			for ( uint32_t buffer : made )
				m_gpu->Retire( buffer, 0 );
			if ( m_program )
				m_gpu->Retire( m_program, 0 );
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
		}
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
		const uint32_t buffers[5] = { m_voxels, m_params[slot], m_positions,
			m_field[m_written % kRing], m_field[slot] };
		const uint64_t serial =
		    m_gpu->QueueDispatch( m_program, buffers, 5, &push, sizeof( push ), m_probes, 1, 1 );
		if ( !serial )
			return;
		m_written = m_written + 1;
		m_lastSerial = serial;
		m_inFlight.push_back( { serial, slot } );
		++m_updates;
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
		for ( size_t t = 0; t < count; ++t )
			for ( int c = 0; c < 3; ++c )
			{
				m_total[t * 3 + c] = field[t * 12 + c] - m_reference[t * 12 + c];
				m_indirect[t * 3 + c] = field[t * 12 + 4 + c] - m_reference[t * 12 + 4 + c];
			}
		m_published =
		    PublishedVolume{ ++m_epoch, m_composer.Compose( *m_baked, m_total.data(),
		                                    m_indirect.data(), m_probes, nullptr ) };
		if ( m_updates >= kActiveUpdates )
			m_active = false;
	}

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

} // namespace indirect_light

#endif // RENDER_INDIRECT_SDF_H
