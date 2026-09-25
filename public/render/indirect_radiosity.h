//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The precomputed radiosity producer (RFC 0011 G4,
//          render.indirect-light.v1): a CPU gather over the map's RTRN
//          transfer that publishes the baked volume plus the light that
//          switched sources add or remove (render.indirect-policy.v1's
//          BakedPlusDelta). tools/quality/radiosity_transfer.py documents the
//          transfer's model and is this solver's reference.
//
//          Solver. With sigma the sources' current scalars and r a reference
//          (1, the bake, for the product; 0 for an absolute solve), the patch
//          change X solves
//              X_p = sum_s ( sigma_s - r_s ) D[p, s] + sum_q F[p, q] rho_q X_q,
//          so X = H( sigma ) - H( r ). Each update runs Jacobi iterations
//          warm-started from the previous update: every iteration writes each
//          patch once from the previous iteration's values, one more bounce of
//          light. A probe's change is its gather of rho X (indirect) plus the
//          switched sources' probe direct light (total). Every sum runs in link
//          order and every output has one writer, so the serial and pooled
//          executors produce identical bytes (jobsystem.radiosity).
//
//          Publication. The published volume is the baked one with each
//          probe's irradiance tiles plus the change, clamped at zero; with
//          every scalar at its bake the change is zero and the publication is
//          the bake exactly. Once an update moves no patch by more than
//          convergedChange the producer stops computing until a scalar
//          changes, so a still scene costs nothing.
//
//          Response. It responds to light intensity (style scalars, toggles)
//          and emission (emissive sources' scalars). A moved light needs a
//          shadow term and moved geometry a new transfer, so it claims neither
//          kLightMotion nor kGeometryMotion.
//
//===========================================================================//

#ifndef RENDER_INDIRECT_RADIOSITY_H
#define RENDER_INDIRECT_RADIOSITY_H

#include "render/indirect_light.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

namespace indirect_light
{

struct RadiosityOptions
{
	uint32_t iterationsPerUpdate = 2;
	// A patch change below this (diffuse light units) ends the updates.
	float convergedChange = 1e-5f;
	// The one-bounce sensitivity defect: each patch gathers the injected light
	// only, never the previous iteration's (the furnace must reject it).
	bool oneBounce = false;
};

class RadiositySolver
{
public:
	static constexpr uint32_t kTexels = mapcontainer::kRadiosityProbeTexels;
	static constexpr uint32_t kInterior = mapcontainer::kProbeIrradianceTile - 2;
	static constexpr uint32_t kPatchBlock = 64;
	static constexpr uint32_t kProbeBlock = 8;

	// `reference`: the scalars X is measured from, one per source.
	void Init( std::shared_ptr<const Transfer> transfer, std::vector<float> reference,
	    const RadiosityOptions &options )
	{
		m_transfer = std::move( transfer );
		m_options = options;
		const auto &layout = m_transfer->layout;
		m_reference = std::move( reference );
		m_reference.resize( layout.sourceCount, 1.0f );
		m_scalars = m_reference;
		m_injection.assign( size_t( layout.patchCount ) * 3, 0.0f );
		m_change.assign( size_t( layout.patchCount ) * 3, 0.0f );
		m_next.assign( m_change.size(), 0.0f );
		m_reflected.assign( m_change.size(), 0.0f );
		m_blockChange.assign( ( layout.patchCount + kPatchBlock - 1 ) / kPatchBlock, 0.0f );
		m_probeIndirect.assign( size_t( layout.probeCount ) * kTexels * 3, 0.0f );
		m_probeTotal.assign( m_probeIndirect.size(), 0.0f );
		m_albedo.resize( size_t( layout.patchCount ) * 3 );
		const mapcontainer::RadiosityTransferView view = m_transfer->View();
		for ( uint32_t p = 0; p < layout.patchCount; ++p )
			for ( int c = 0; c < 3; ++c )
				m_albedo[size_t( p ) * 3 + c] = view.Patch( p ).albedo[c];
		// The nine SH functions at the interior texel directions, in the PRBV
		// interior order (row v, column u).
		for ( uint32_t v = 0; v < kInterior; ++v )
		{
			for ( uint32_t u = 0; u < kInterior; ++u )
			{
				const float oct[2] = { ( float( u ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f,
				    ( float( v ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f };
				float d[3];
				OctDecode( oct, d );
				ShBasis( d, &m_basis[( v * kInterior + u ) * 9] );
			}
		}
		m_dirty = false;
		m_iterations = 0;
	}

	[[nodiscard]] uint32_t SourceCount() const { return m_transfer->layout.sourceCount; }
	[[nodiscard]] float Scalar( uint32_t source ) const { return m_scalars[source]; }

	// Sets the sources' current scalars; any change restarts the updates.
	void SetScalars( const std::vector<float> &scalars )
	{
		bool changed = false;
		for ( uint32_t s = 0; s < SourceCount() && s < scalars.size(); ++s )
		{
			if ( scalars[s] != m_scalars[s] )
			{
				m_scalars[s] = scalars[s];
				changed = true;
			}
		}
		if ( !changed )
			return;
		const mapcontainer::RadiosityTransferView view = m_transfer->View();
		std::fill( m_injection.begin(), m_injection.end(), 0.0f );
		for ( uint32_t s = 0; s < SourceCount(); ++s )
		{
			const float scale = m_scalars[s] - m_reference[s];
			if ( scale == 0.0f )
				continue;
			for ( auto *l = view.InjectionBegin( s ); l != view.InjectionEnd( s ); ++l )
				for ( int c = 0; c < 3; ++c )
					m_injection[size_t( l->patch ) * 3 + c] += scale * l->light[c];
		}
		m_dirty = true;
	}

	// True while updates still change the solution.
	[[nodiscard]] bool Dirty() const { return m_dirty; }
	[[nodiscard]] float LastChange() const { return m_lastChange; }
	[[nodiscard]] uint64_t Iterations() const { return m_iterations; }

	// One update: iterationsPerUpdate Jacobi iterations, then the probe
	// gather. Returns false (and does nothing) once converged.
	bool Update( IBatchExecutor *executor )
	{
		if ( !m_dirty )
			return false;
		const auto &layout = m_transfer->layout;
		for ( uint32_t k = 0; k < m_options.iterationsPerUpdate; ++k )
		{
			m_reflectInjection = m_options.oneBounce;
			Run( executor, "radiosity.reflect", uint32_t( m_blockChange.size() ), &ReflectBlock );
			Run( executor, "radiosity.gather", uint32_t( m_blockChange.size() ), &GatherBlock );
			m_next.swap( m_change );
			++m_iterations;
			m_lastChange = 0.0f;
			for ( float change : m_blockChange )
				m_lastChange = std::max( m_lastChange, change );
			if ( m_lastChange < m_options.convergedChange )
				break;
		}
		// The probes gather the patches' reflected light.
		m_reflectInjection = false;
		Run( executor, "radiosity.reflect", uint32_t( m_blockChange.size() ), &ReflectBlock );
		Run( executor, "radiosity.probes", ( layout.probeCount + kProbeBlock - 1 ) / kProbeBlock,
		    &ProbeBlock );
		if ( m_lastChange < m_options.convergedChange )
			m_dirty = false;
		return true;
	}

	// The change at a patch (3 floats) and at a probe's interior texels
	// (36 x 3 floats), from the last update.
	[[nodiscard]] const float *PatchChange( uint32_t patch ) const
	{
		return &m_change[size_t( patch ) * 3];
	}
	[[nodiscard]] const float *ProbeIndirect( uint32_t probe ) const
	{
		return &m_probeIndirect[size_t( probe ) * kTexels * 3];
	}
	[[nodiscard]] const float *ProbeTotal( uint32_t probe ) const
	{
		return &m_probeTotal[size_t( probe ) * kTexels * 3];
	}

	// `base` plus the probes' changes, clamped at zero (the published volume).
	[[nodiscard]] std::shared_ptr<const Volume> Compose(
	    const Volume &base, IBatchExecutor *executor ) const
	{
		auto volume = std::make_shared<Volume>( base );
		ComposeContext context{ this, volume.get() };
		const uint32_t probes = m_transfer->layout.probeCount;
		RunWith( executor, "radiosity.compose", ( probes + kProbeBlock - 1 ) / kProbeBlock,
		    &ComposeBlock, &context );
		return volume;
	}

	static void OctDecode( const float p[2], float out[3] )
	{
		float x = p[0], y = p[1];
		const float z = 1.0f - std::fabs( x ) - std::fabs( y );
		if ( z < 0.0f )
		{
			const float fx = ( 1.0f - std::fabs( p[1] ) ) * ( p[0] >= 0.0f ? 1.0f : -1.0f );
			const float fy = ( 1.0f - std::fabs( p[0] ) ) * ( p[1] >= 0.0f ? 1.0f : -1.0f );
			x = fx;
			y = fy;
		}
		const float length = std::sqrt( x * x + y * y + z * z );
		out[0] = x / length;
		out[1] = y / length;
		out[2] = z / length;
	}

	// The nine real SH of order <= 2 (radiosity_transfer.sh_basis).
	static void ShBasis( const float d[3], float out[9] )
	{
		const float x = d[0], y = d[1], z = d[2];
		out[0] = 0.282094791773878f;
		out[1] = 0.488602511902920f * y;
		out[2] = 0.488602511902920f * z;
		out[3] = 0.488602511902920f * x;
		out[4] = 1.092548430592079f * x * y;
		out[5] = 1.092548430592079f * y * z;
		out[6] = 0.315391565252520f * ( 3.0f * z * z - 1.0f );
		out[7] = 1.092548430592079f * x * z;
		out[8] = 0.546274215296040f * ( x * x - y * y );
	}

private:
	struct ComposeContext
	{
		const RadiositySolver *solver;
		Volume *volume;
	};

	void Run( IBatchExecutor *executor, const char *name, uint32_t count,
	    void ( *body )( void *, uint32_t ) )
	{
		RunWith( executor, name, count, body, this );
	}

	static void RunWith( IBatchExecutor *executor, const char *name, uint32_t count,
	    void ( *body )( void *, uint32_t ), void *context )
	{
		if ( executor )
			executor->ParallelFor( name, count, body, context );
		else
			for ( uint32_t i = 0; i < count; ++i )
				body( context, i );
	}

	// rho X (or, gathering for the one-bounce defect, rho times the injection).
	static void ReflectBlock( void *context, uint32_t block )
	{
		RadiositySolver &self = *static_cast<RadiositySolver *>( context );
		const std::vector<float> &from = self.m_reflectInjection ? self.m_injection : self.m_change;
		const size_t begin = size_t( block ) * kPatchBlock * 3;
		const size_t end = std::min( begin + kPatchBlock * 3, self.m_change.size() );
		for ( size_t k = begin; k < end; ++k )
			self.m_reflected[k] = self.m_albedo[k] * from[k];
	}

	static void GatherBlock( void *context, uint32_t block )
	{
		RadiositySolver &self = *static_cast<RadiositySolver *>( context );
		const mapcontainer::RadiosityTransferView view = self.m_transfer->View();
		const uint32_t begin = block * kPatchBlock;
		const uint32_t end = std::min( begin + kPatchBlock, self.m_transfer->layout.patchCount );
		const float *reflected = self.m_reflected.data();
		float largest = 0.0f;
		for ( uint32_t p = begin; p < end; ++p )
		{
			float sum[3] = { self.m_injection[size_t( p ) * 3], self.m_injection[size_t( p ) * 3 + 1],
			    self.m_injection[size_t( p ) * 3 + 2] };
			for ( auto *l = view.TransferBegin( p ); l != view.TransferEnd( p ); ++l )
			{
				const float *q = reflected + size_t( l->patch ) * 3;
				sum[0] += l->factor * q[0];
				sum[1] += l->factor * q[1];
				sum[2] += l->factor * q[2];
			}
			for ( int c = 0; c < 3; ++c )
			{
				largest = std::max( largest, std::fabs( sum[c] - self.m_change[size_t( p ) * 3 + c] ) );
				self.m_next[size_t( p ) * 3 + c] = sum[c];
			}
		}
		self.m_blockChange[block] = largest;
	}

	static void ProbeBlock( void *context, uint32_t block )
	{
		RadiositySolver &self = *static_cast<RadiositySolver *>( context );
		const mapcontainer::RadiosityTransferView view = self.m_transfer->View();
		const auto &layout = self.m_transfer->layout;
		const uint32_t begin = block * kProbeBlock;
		const uint32_t end = std::min( begin + kProbeBlock, layout.probeCount );
		const float *reflected = self.m_reflected.data();
		for ( uint32_t i = begin; i < end; ++i )
		{
			float sh[9][3] = {};
			for ( auto *l = view.GatherBegin( i ); l != view.GatherEnd( i ); ++l )
			{
				const float *q = reflected + size_t( l->patch ) * 3;
				for ( int k = 0; k < 9; ++k )
					for ( int c = 0; c < 3; ++c )
						sh[k][c] += l->sh[k] * q[c];
			}
			float *indirect = &self.m_probeIndirect[size_t( i ) * kTexels * 3];
			float *total = &self.m_probeTotal[size_t( i ) * kTexels * 3];
			for ( uint32_t t = 0; t < kTexels; ++t )
			{
				const float *basis = &self.m_basis[t * 9];
				for ( int c = 0; c < 3; ++c )
				{
					float value = 0.0f;
					for ( int k = 0; k < 9; ++k )
						value += basis[k] * sh[k][c];
					indirect[t * 3 + c] = value;
					total[t * 3 + c] = value;
				}
			}
			for ( uint32_t s = 0; s < layout.sourceCount; ++s )
			{
				const float scale = self.m_scalars[s] - self.m_reference[s];
				if ( scale == 0.0f )
					continue;
				const float *direct = view.ProbeDirect( s, i );
				for ( uint32_t k = 0; k < kTexels * 3; ++k )
					total[k] += scale * direct[k];
			}
		}
	}

	static void ComposeBlock( void *context, uint32_t block )
	{
		const ComposeContext &compose = *static_cast<const ComposeContext *>( context );
		const RadiositySolver &self = *compose.solver;
		Volume &volume = *compose.volume;
		const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
		const uint32_t probeCount = self.m_transfer->layout.probeCount;
		const uint32_t begin = block * kProbeBlock;
		const uint32_t end = std::min( begin + kProbeBlock, probeCount );
		uint32_t first = 0; // the global index of the grid's first probe
		for ( uint32_t g = 0; g < layout.gridCount; ++g )
		{
			const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
			for ( uint32_t i = std::max( begin, first ); i < std::min( end, first + grid.probeCount );
			      ++i )
			{
				const uint32_t local = i - first;
				for ( uint32_t layer = 0; layer < layout.layerCount && layer < 2; ++layer )
				{
					const float *change = layer == 0 ? self.ProbeTotal( i ) : self.ProbeIndirect( i );
					WriteTile( volume, grid.irradianceOrigin[layer], local, grid.tilesPerRow, change );
				}
			}
			first += grid.probeCount;
		}
	}

	// Adds `change` (36 rgb, interior order) to a probe's 8 x 8 tile: the
	// interior, then the octahedral border (probe_volume.with_border).
	static void WriteTile( Volume &volume, const uint32_t origin[2], uint32_t probe,
	    uint32_t tilesPerRow, const float *change )
	{
		const uint32_t tile = mapcontainer::kProbeIrradianceTile;
		const uint32_t x0 = origin[0] + ( probe % tilesPerRow ) * tile;
		const uint32_t y0 = origin[1] + ( probe / tilesPerRow ) * tile;
		float interior[kInterior][kInterior][3];
		for ( uint32_t v = 0; v < kInterior; ++v )
		{
			for ( uint32_t u = 0; u < kInterior; ++u )
			{
				unsigned char *texel = Texel( volume, x0 + 1 + u, y0 + 1 + v );
				for ( int c = 0; c < 3; ++c )
				{
					uint16_t half;
					std::memcpy( &half, texel + 2 * c, 2 );
					interior[v][u][c] = std::max(
					    0.0f, mapcontainer::HalfToFloat( half ) + change[( v * kInterior + u ) * 3 + c] );
				}
			}
		}
		const auto put = [&]( uint32_t x, uint32_t y, const float rgb[3] )
		{
			unsigned char *texel = Texel( volume, x0 + x, y0 + y );
			for ( int c = 0; c < 3; ++c )
			{
				const uint16_t half = FloatToHalf( rgb[c] );
				std::memcpy( texel + 2 * c, &half, 2 );
			}
		};
		const uint32_t n = kInterior;
		for ( uint32_t v = 0; v < n; ++v )
			for ( uint32_t u = 0; u < n; ++u )
				put( 1 + u, 1 + v, interior[v][u] );
		for ( uint32_t k = 0; k < n; ++k )
		{
			put( 1 + k, 0, interior[0][n - 1 - k] );
			put( 1 + k, n + 1, interior[n - 1][n - 1 - k] );
			put( 0, 1 + k, interior[n - 1 - k][0] );
			put( n + 1, 1 + k, interior[n - 1 - k][n - 1] );
		}
		put( 0, 0, interior[n - 1][n - 1] );
		put( n + 1, 0, interior[n - 1][0] );
		put( 0, n + 1, interior[0][n - 1] );
		put( n + 1, n + 1, interior[0][0] );
	}

	static unsigned char *Texel( Volume &volume, uint32_t x, uint32_t y )
	{
		return volume.bytes.data() + volume.layout.atlasOffset +
		       ( uint64_t( y ) * volume.layout.atlasWidth + x ) * 8;
	}

	std::shared_ptr<const Transfer> m_transfer;
	RadiosityOptions m_options;
	std::vector<float> m_reference;
	std::vector<float> m_scalars;
	std::vector<float> m_albedo;
	std::vector<float> m_injection;
	std::vector<float> m_change;
	std::vector<float> m_next;
	std::vector<float> m_reflected;
	std::vector<float> m_blockChange;
	std::vector<float> m_probeIndirect;
	std::vector<float> m_probeTotal;
	std::array<float, kTexels * 9> m_basis{};
	float m_lastChange = 0.0f;
	uint64_t m_iterations = 0;
	bool m_dirty = false;
	bool m_reflectInjection = false;
};

// The product producer: BakedPlusDelta over the map's transfer. Each source's
// scalar is its light style's (the light set's styleScalars; a fixed source
// stays 1) unless a scripted SceneChange overrides it.
class RadiosityProducer final : public IProducer
{
public:
	static constexpr uint32_t kConvergenceFrames = 30; // 0.5 s at 60 Hz

	explicit RadiosityProducer( const RadiosityOptions &options = RadiosityOptions() )
	    : m_options( options )
	{
	}

	[[nodiscard]] ProducerCaps Caps() const override
	{
		ProducerCaps caps;
		caps.kind = ProducerKind::PrecomputedRadiosity;
		caps.responds = kLightIntensity | kEmission;
		caps.policies = PolicyBit( indirect_policy::Policy::BakedPlusDelta );
		caps.convergenceFrames = kConvergenceFrames;
		caps.seedTolerance = 0.0f;
		caps.responseTolerance = 0.02f;
		return caps;
	}

	[[nodiscard]] foundation::Expected<void, IndirectError> Begin(
	    const IndirectScene &scene, const PublishedVolume &, IResourceTracker & ) override
	{
		if ( !scene.baked || !scene.transfer )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		if ( scene.policy != indirect_policy::Policy::BakedPlusDelta )
			return foundation::MakeUnexpected( IndirectError::UnsupportedPolicy );
		const auto &layout = scene.transfer->layout;
		uint32_t probes = 0;
		for ( uint32_t g = 0; g < scene.baked->layout.gridCount; ++g )
			probes += scene.baked->layout.grids[g].probeCount;
		if ( probes != layout.probeCount ||
		     mapcontainer::ProbeVolumeTopologyHash(
		         scene.baked->bytes.data(), scene.baked->layout ) != layout.prbvHash )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		m_baked = scene.baked;
		m_solver.Init( scene.transfer, std::vector<float>( layout.sourceCount, 1.0f ), m_options );
		m_styles.resize( layout.sourceCount );
		const mapcontainer::RadiosityTransferView view = scene.transfer->View();
		for ( uint32_t s = 0; s < layout.sourceCount; ++s )
			m_styles[s] = view.Source( s ).style;
		m_overrides.assign( layout.sourceCount, -1.0f );
		// Seeded exactly: at the bake's scalars the change is zero.
		m_published = PublishedVolume{ ++m_epoch, m_baked };
		return {};
	}

	void Schedule( FrameWork &work, const light_set::Snapshot &lights ) override
	{
		if ( !m_baked )
			return;
		for ( const SceneChange &change : work.changes )
		{
			if ( !( change.response & ( kLightIntensity | kEmission ) ) )
				continue;
			for ( uint32_t s = 0; s < m_overrides.size(); ++s )
				if ( change.target == kEverySource || change.target == s )
					m_overrides[s] = change.value;
		}
		std::vector<float> scalars( m_overrides.size(), 1.0f );
		for ( uint32_t s = 0; s < scalars.size(); ++s )
		{
			if ( m_overrides[s] >= 0.0f )
				scalars[s] = m_overrides[s];
			else if ( m_styles[s] >= 0 && size_t( m_styles[s] ) < lights.styleScalars.size() )
				scalars[s] = lights.styleScalars[size_t( m_styles[s] )];
		}
		m_solver.SetScalars( scalars );
		if ( !m_solver.Dirty() )
			return;
		IBatchExecutor *executor = work.executor;
		work.jobs.push_back(
		    [this, executor]
		    {
			    if ( m_solver.Update( executor ) )
				    m_published = PublishedVolume{ ++m_epoch, m_solver.Compose( *m_baked, executor ) };
		    } );
	}

	[[nodiscard]] std::optional<PublishedVolume> Published() const override { return m_published; }

	[[nodiscard]] RetireTicket End() override
	{
		m_published.reset();
		m_baked.reset();
		return {}; // CPU data only: the host retires its uploads
	}

	[[nodiscard]] const RadiositySolver &Solver() const { return m_solver; }

private:
	RadiosityOptions m_options;
	RadiositySolver m_solver;
	std::shared_ptr<const Volume> m_baked;
	std::vector<int32_t> m_styles;
	std::vector<float> m_overrides;
	std::optional<PublishedVolume> m_published;
	uint64_t m_epoch = 0;
};

} // namespace indirect_light

#endif // RENDER_INDIRECT_RADIOSITY_H
