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
	// One bounce per update: 30 updates (0.5 s at 60 Hz) converge the fixtures,
	// and one update fits the desktop budget (quality/budgets/indirect-light-v1.json).
	uint32_t iterationsPerUpdate = 1;
	// A patch change below this (diffuse light units) ends the updates.
	float convergedChange = 1e-5f;
	// Each update gathers every probeStride-th probe block, in turn; once the
	// patches converge, the remaining blocks are gathered before the updates
	// stop, so a converged publication is complete.
	uint32_t probeStride = 2;
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
		m_probePhase = 0;
		m_settling = 0;
		m_patchesSettled = false;
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
			for ( auto *l = view.InjectionBegin( s ), *end = view.InjectionEnd( s ); l != end; ++l )
				for ( int c = 0; c < 3; ++c )
					m_injection[size_t( l->patch ) * 3 + c] += scale * l->light[c];
		}
		m_dirty = true;
		m_patchesSettled = false;
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
		for ( uint32_t k = 0; !m_patchesSettled && k < m_options.iterationsPerUpdate; ++k )
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
		// The probes gather the patches' reflected light: this update's share.
		m_reflectInjection = false;
		Run( executor, "radiosity.reflect", uint32_t( m_blockChange.size() ), &ReflectBlock );
		const uint32_t stride = std::max( 1u, m_options.probeStride );
		const uint32_t blocks = ( layout.probeCount + kProbeBlock - 1 ) / kProbeBlock;
		m_probePhase = ( m_probePhase + 1 ) % stride;
		Run( executor, "radiosity.probes", ( blocks + stride - 1 - m_probePhase ) / stride,
		    &ProbeBlock );
		// Every probe's total: its indirect plus the switched sources' direct
		// light, which follows a switch at once.
		Run( executor, "radiosity.direct", blocks, &DirectBlock );
		if ( !m_patchesSettled )
		{
			if ( m_lastChange < m_options.convergedChange )
			{
				// Settled: the other phases gather the settled patches next.
				m_patchesSettled = true;
				m_settling = stride - 1;
			}
		}
		else if ( m_settling > 0 )
			--m_settling;
		if ( m_patchesSettled && m_settling == 0 )
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
		return m_composer.Compose( base, m_probeTotal.data(), m_probeIndirect.data(),
		    m_transfer->layout.probeCount, executor );
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
			float sum[3] = { self.m_injection[size_t( p ) * 3],
			    self.m_injection[size_t( p ) * 3 + 1], self.m_injection[size_t( p ) * 3 + 2] };
			for ( auto *l = view.TransferBegin( p ), *end = view.TransferEnd( p ); l != end; ++l )
			{
				const float *q = reflected + size_t( l->patch ) * 3;
				sum[0] += l->factor * q[0];
				sum[1] += l->factor * q[1];
				sum[2] += l->factor * q[2];
			}
			for ( int c = 0; c < 3; ++c )
			{
				largest =
				    std::max( largest, std::fabs( sum[c] - self.m_change[size_t( p ) * 3 + c] ) );
				self.m_next[size_t( p ) * 3 + c] = sum[c];
			}
		}
		self.m_blockChange[block] = largest;
	}

	static void ProbeBlock( void *context, uint32_t item )
	{
		RadiositySolver &self = *static_cast<RadiositySolver *>( context );
		const mapcontainer::RadiosityTransferView view = self.m_transfer->View();
		const auto &layout = self.m_transfer->layout;
		const uint32_t block =
		    item * std::max( 1u, self.m_options.probeStride ) + self.m_probePhase;
		const uint32_t begin = block * kProbeBlock;
		const uint32_t end = std::min( begin + kProbeBlock, layout.probeCount );
		const float *reflected = self.m_reflected.data();
		for ( uint32_t i = begin; i < end; ++i )
		{
			// Per channel, the nine coefficients: contiguous, so they vectorize.
			float sh[3][9] = {};
			for ( auto *l = view.GatherBegin( i ), *end = view.GatherEnd( i ); l != end; ++l )
			{
				const float *q = reflected + size_t( l->patch ) * 3;
				for ( int c = 0; c < 3; ++c )
					for ( int k = 0; k < 9; ++k )
						sh[c][k] += l->sh[k] * q[c];
			}
			float *indirect = &self.m_probeIndirect[size_t( i ) * kTexels * 3];
			for ( uint32_t t = 0; t < kTexels; ++t )
			{
				const float *basis = &self.m_basis[t * 9];
				for ( int c = 0; c < 3; ++c )
				{
					float value = 0.0f;
					for ( int k = 0; k < 9; ++k )
						value += basis[k] * sh[c][k];
					indirect[t * 3 + c] = value;
				}
			}
		}
	}

	static void DirectBlock( void *context, uint32_t block )
	{
		RadiositySolver &self = *static_cast<RadiositySolver *>( context );
		const mapcontainer::RadiosityTransferView view = self.m_transfer->View();
		const auto &layout = self.m_transfer->layout;
		const uint32_t begin = block * kProbeBlock;
		const uint32_t end = std::min( begin + kProbeBlock, layout.probeCount );
		for ( uint32_t i = begin; i < end; ++i )
		{
			const float *indirect = &self.m_probeIndirect[size_t( i ) * kTexels * 3];
			float *total = &self.m_probeTotal[size_t( i ) * kTexels * 3];
			for ( uint32_t k = 0; k < kTexels * 3; ++k )
				total[k] = indirect[k];
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
	ChangeComposer m_composer;
	float m_lastChange = 0.0f;
	uint64_t m_iterations = 0;
	bool m_dirty = false;
	bool m_reflectInjection = false;
	uint32_t m_probePhase = 0;
	uint32_t m_settling = 0;
	bool m_patchesSettled = false;
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
				    m_published =
				        PublishedVolume{ ++m_epoch, m_solver.Compose( *m_baked, executor ) };
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
