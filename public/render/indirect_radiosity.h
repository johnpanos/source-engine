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
//          kLightMotion nor kGeometryMotion. Open portals (RFC 0011 G10) add
//          dynamic links through their apertures: patch and probe links to the
//          patches beyond, and the map's SDFV lights beyond, sampled at their
//          images; unoccluded, as the transfer holds no geometry to test.
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

	// RFC 0011 G10: the open portals. Each portal's receivers (patches and
	// probes in front of it) gather, through its aperture, the patches in
	// front of its linked portal at their images (the inverse of its
	// transform): patch links with the form factor cos cos A / (pi d^2 + A),
	// probe links with the SH of that small source (the gather's convention:
	// BAND_SCALE_k Omega Y_k). A link carries its source's whole reflected
	// light, the bake's plus the change, since none of it is in the bake.
	// The transfer holds no geometry to test visibility against: the space
	// in front of each portal is taken as open. Changing the set restarts
	// the updates; `probes` are the volume's probe positions (xyz, w active).
	//
	// The analytic lights (the map's SDFV records, SetPortalLights) are not
	// patches: their direct light through a portal is sampled at their
	// images, 16 points each, into the receivers' injection and the probes'
	// light, at their styles' current scalars (`styleScalars`, 1 when absent).
	void SetPortals( std::span<const Portal> portals, std::span<const float> probes,
	    std::span<const float> styleScalars = {} )
	{
		std::vector<float> scalars( m_portalLights.size(), 1.0f );
		for ( size_t l = 0; l < m_portalLights.size(); ++l )
		{
			const int32_t style = m_portalLights[l].style;
			if ( style >= 0 && size_t( style ) < styleScalars.size() )
				scalars[l] = styleScalars[size_t( style )];
		}
		const bool same = portals.size() == m_portals.size() &&
		                  std::equal( portals.begin(), portals.end(), m_portals.begin() );
		if ( same && ( m_portals.empty() || scalars == m_portalLightScalars ) )
			return;
		if ( !same )
		{
			m_portals.assign( portals.begin(), portals.end() );
			BuildPortalLinks( probes );
			if ( !m_portals.empty() && m_absoluteReflected.empty() )
				SolveAbsolute();
		}
		m_portalLightScalars = scalars;
		BuildPortalLightDirect( probes );
		m_dirty = true;
		m_patchesSettled = false;
	}
	void SetPortalLights( std::vector<mapcontainer::SdfLight> lights )
	{
		m_portalLights = std::move( lights );
	}
	[[nodiscard]] size_t PortalLinks() const { return m_portalLinks.size(); }
	[[nodiscard]] size_t PortalGatherLinks() const { return m_portalGather.size(); }

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
	struct PortalLink
	{
		uint32_t patch;
		float factor;
	};
	struct PortalGatherLink
	{
		uint32_t patch;
		float sh[9];
	};

	static float Dot( const float a[3], const float b[3] )
	{
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	}

	// Portal k's inverse transform (rigid) applied to a point / vector.
	static void InverseVector( const Portal &k, const float v[3], float out[3] )
	{
		for ( int c = 0; c < 3; ++c )
			out[c] = k.toLinked[c] * v[0] + k.toLinked[4 + c] * v[1] + k.toLinked[8 + c] * v[2];
	}
	static void InversePoint( const Portal &k, const float p[3], float out[3] )
	{
		const float q[3] = { p[0] - k.toLinked[3], p[1] - k.toLinked[7], p[2] - k.toLinked[11] };
		InverseVector( k, q, out );
	}
	// The linked portal's frame: its origin (the image of k's) and forward.
	static void LinkedFrame( const Portal &k, float origin[3], float forward[3] )
	{
		for ( int r = 0; r < 3; ++r )
		{
			origin[r] = k.toLinked[r * 4 + 3];
			forward[r] = 0.0f;
			for ( int c = 0; c < 3; ++c )
			{
				origin[r] += k.toLinked[r * 4 + c] * k.origin[c];
				forward[r] -= k.toLinked[r * 4 + c] * k.forward[c];
			}
		}
	}
	// Whether the segment from `from` to `to` passes through portal k's
	// rectangle, entering from its front.
	static bool ThroughAperture( const Portal &k, const float from[3], const float to[3] )
	{
		float d[3], o[3];
		for ( int c = 0; c < 3; ++c )
		{
			d[c] = to[c] - from[c];
			o[c] = k.origin[c] - from[c];
		}
		const float facing = Dot( d, k.forward );
		if ( facing >= 0.0f )
			return false;
		const float t = Dot( o, k.forward ) / facing;
		if ( t <= 0.0f || t >= 1.0f )
			return false;
		float local[3];
		for ( int c = 0; c < 3; ++c )
			local[c] = from[c] + d[c] * t - k.origin[c];
		return std::fabs( Dot( local, k.right ) ) <= k.halfWidth &&
		       std::fabs( Dot( local, k.up ) ) <= k.halfHeight;
	}

	void BuildPortalLinks( std::span<const float> probes )
	{
		static const float kBand[9] = { 1.0f, 2.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f, 0.25f,
			0.25f, 0.25f, 0.25f, 0.25f };
		const mapcontainer::RadiosityTransferView view = m_transfer->View();
		const auto &layout = m_transfer->layout;
		std::vector<std::vector<PortalLink>> rows( layout.patchCount );
		std::vector<std::vector<PortalGatherLink>> probeRows( layout.probeCount );
		for ( const Portal &k : m_portals )
		{
			float linkedOrigin[3], linkedForward[3];
			LinkedFrame( k, linkedOrigin, linkedForward );
			// The sources: patches in front of the linked portal, at their
			// images behind this one.
			struct Image
			{
				uint32_t patch;
				float position[3];
				float normal[3];
				float area;
			};
			std::vector<Image> images;
			for ( uint32_t q = 0; q < layout.patchCount; ++q )
			{
				const mapcontainer::RadiosityPatch &patch = view.Patch( q );
				float offset[3];
				for ( int c = 0; c < 3; ++c )
					offset[c] = patch.position[c] - linkedOrigin[c];
				if ( Dot( offset, linkedForward ) <= 0.0f )
					continue;
				Image image;
				image.patch = q;
				InversePoint( k, patch.position, image.position );
				InverseVector( k, patch.normal, image.normal );
				image.area = patch.area;
				images.push_back( image );
			}
			const auto link = [&]( const float at[3], const float normal[3], const Image &image,
			                      float *omega, float direction[3] )
			{
				float d[3];
				for ( int c = 0; c < 3; ++c )
					d[c] = image.position[c] - at[c];
				const float d2 = Dot( d, d );
				if ( !( d2 > 1e-8f ) || !ThroughAperture( k, at, image.position ) )
					return false;
				const float length = std::sqrt( d2 );
				for ( int c = 0; c < 3; ++c )
					direction[c] = d[c] / length;
				const float cosSource = -Dot( image.normal, direction );
				const float cosReceiver = normal ? Dot( normal, direction ) : 1.0f;
				if ( cosSource <= 0.0f || cosReceiver <= 0.0f )
					return false;
				*omega = image.area * cosSource / ( d2 + image.area / float( M_PI ) );
				return true;
			};
			for ( uint32_t p = 0; p < layout.patchCount; ++p )
			{
				const mapcontainer::RadiosityPatch &patch = view.Patch( p );
				float offset[3];
				for ( int c = 0; c < 3; ++c )
					offset[c] = patch.position[c] - k.origin[c];
				if ( Dot( offset, k.forward ) <= 0.0f )
					continue;
				for ( const Image &image : images )
				{
					float omega, direction[3];
					if ( !link( patch.position, patch.normal, image, &omega, direction ) )
						continue;
					// The form factor: the solid angle times the receiver's cosine, over pi.
					const float factor =
					    omega * Dot( patch.normal, direction ) / float( M_PI );
					if ( factor > 1e-7f )
						rows[p].push_back( { image.patch, factor } );
				}
			}
			for ( uint32_t i = 0; i < layout.probeCount && i * 4 + 3 < probes.size(); ++i )
			{
				if ( probes[i * 4 + 3] < 0.5f )
					continue;
				const float *at = &probes[i * 4];
				float offset[3];
				for ( int c = 0; c < 3; ++c )
					offset[c] = at[c] - k.origin[c];
				if ( Dot( offset, k.forward ) <= 0.0f )
					continue;
				for ( const Image &image : images )
				{
					float omega, direction[3];
					if ( !link( at, nullptr, image, &omega, direction ) )
						continue;
					PortalGatherLink gather;
					gather.patch = image.patch;
					float basis[9];
					ShBasis( direction, basis );
					for ( int b = 0; b < 9; ++b )
						gather.sh[b] = kBand[b] * omega * basis[b];
					probeRows[i].push_back( gather );
				}
			}
		}
		m_portalRow.assign( 1, 0 );
		m_portalLinks.clear();
		for ( const auto &row : rows )
		{
			m_portalLinks.insert( m_portalLinks.end(), row.begin(), row.end() );
			m_portalRow.push_back( uint32_t( m_portalLinks.size() ) );
		}
		m_portalGatherRow.assign( 1, 0 );
		m_portalGather.clear();
		for ( const auto &row : probeRows )
		{
			m_portalGather.insert( m_portalGather.end(), row.begin(), row.end() );
			m_portalGatherRow.push_back( uint32_t( m_portalGather.size() ) );
		}
	}

	// One sample of an analytic light at its image through portal k: the
	// point, its emitting normal (zero: a sphere, facing every way), and its
	// radiance times the area it stands for.
	struct LightSample
	{
		float position[3];
		float normal[3];
		float power[3];
	};

	static void PortalLightSamples(
	    const Portal &k, const mapcontainer::SdfLight &light, std::vector<LightSample> *out )
	{
		out->clear();
		const auto cross = []( const float a[3], const float b[3], float o[3] )
		{
			o[0] = a[1] * b[2] - a[2] * b[1];
			o[1] = a[2] * b[0] - a[0] * b[2];
			o[2] = a[0] * b[1] - a[1] * b[0];
		};
		const auto add = [&]( const float at[3], const float normal[3], float weight )
		{
			LightSample sample;
			InversePoint( k, at, sample.position );
			InverseVector( k, normal, sample.normal );
			for ( int c = 0; c < 3; ++c )
				sample.power[c] = light.rgb[c] * weight;
			out->push_back( sample );
		};
		const uint32_t kind = light.kind;
		if ( kind == uint32_t( mapcontainer::SdfLightKind::Rect ) )
		{
			float axes[3];
			cross( light.b, light.c, axes );
			const float length = std::sqrt( Dot( axes, axes ) );
			if ( !( length > 0.0f ) )
				return;
			const float normal[3] = { axes[0] / length, axes[1] / length, axes[2] / length };
			for ( int u = 0; u < 4; ++u )
				for ( int v = 0; v < 4; ++v )
				{
					const float su = ( float( u ) + 0.5f ) / 2.0f - 1.0f;
					const float sv = ( float( v ) + 0.5f ) / 2.0f - 1.0f;
					float at[3];
					for ( int c = 0; c < 3; ++c )
						at[c] = light.a[c] + su * light.b[c] + sv * light.c[c];
					add( at, normal, 4.0f * length / 16.0f );
				}
		}
		else if ( kind == uint32_t( mapcontainer::SdfLightKind::Sphere ) )
		{
			const float zero[3] = {};
			add( light.a, zero, float( M_PI ) * light.b[0] * light.b[0] );
		}
		else if ( kind == uint32_t( mapcontainer::SdfLightKind::Spot ) )
		{
			// The disk (radius c.x, facing b), 16 stratified points; the cone
			// is applied per receiver (reserved[0] its exponent).
			float t0[3], t1[3];
			const float helper[3] = { 0.0f, 0.0f, 1.0f }, other[3] = { 1.0f, 0.0f, 0.0f };
			cross( light.b, std::fabs( light.b[2] ) < 0.9f ? helper : other, t0 );
			const float l0 = std::sqrt( Dot( t0, t0 ) );
			for ( float &x : t0 )
				x /= l0;
			cross( light.b, t0, t1 );
			for ( int i = 0; i < 16; ++i )
			{
				const float radius = light.c[0] * std::sqrt( ( float( i ) + 0.5f ) / 16.0f );
				const float phi = 2.39996323f * float( i );
				float at[3];
				for ( int c = 0; c < 3; ++c )
					at[c] = light.a[c] + radius * ( std::cos( phi ) * t0[c] +
					                                   std::sin( phi ) * t1[c] );
				add( at, light.b, float( M_PI ) * light.c[0] * light.c[0] / 16.0f );
			}
		}
	}

	// vrad's spot cone (as the trace program's): 1 inside the inner angle, 0
	// outside the outer, the exponent between.
	static float SpotCone( const mapcontainer::SdfLight &light, float cosLight )
	{
		if ( light.kind != uint32_t( mapcontainer::SdfLightKind::Spot ) || cosLight >= light.c[1] )
			return 1.0f;
		float cone = ( cosLight - light.c[2] ) / std::max( light.c[1] - light.c[2], 1e-6f );
		cone = std::min( 1.0f, std::max( 0.0f, cone ) );
		if ( light.reserved[0] != 0.0f && light.reserved[0] != 1.0f )
			cone = std::pow( cone, light.reserved[0] );
		return cone;
	}

	// The analytic lights' direct light through the open portals, at the
	// receivers (patch injection) and the probes (per texel).
	void BuildPortalLightDirect( std::span<const float> probes )
	{
		const mapcontainer::RadiosityTransferView view = m_transfer->View();
		const auto &layout = m_transfer->layout;
		m_portalInjection.assign( size_t( layout.patchCount ) * 3, 0.0f );
		m_portalProbe.assign( size_t( layout.probeCount ) * kTexels * 3, 0.0f );
		if ( m_portals.empty() || m_portalLights.empty() )
			return;
		float texels[kTexels][3];
		for ( uint32_t v = 0; v < kInterior; ++v )
			for ( uint32_t u = 0; u < kInterior; ++u )
			{
				const float oct[2] = { ( float( u ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f,
				    ( float( v ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f };
				OctDecode( oct, texels[v * kInterior + u] );
			}
		std::vector<LightSample> samples;
		for ( const Portal &k : m_portals )
			for ( size_t l = 0; l < m_portalLights.size(); ++l )
			{
				const mapcontainer::SdfLight &light = m_portalLights[l];
				PortalLightSamples( k, light, &samples );
				// The light's samples through the aperture from `at`: the
				// direction and E / pi toward it for a facing normal.
				const auto each = [&]( const float at[3], auto &&use )
				{
					float offset[3];
					for ( int c = 0; c < 3; ++c )
						offset[c] = at[c] - k.origin[c];
					if ( Dot( offset, k.forward ) <= 0.0f )
						return;
					for ( const LightSample &sample : samples )
					{
						if ( !ThroughAperture( k, at, sample.position ) )
							continue;
						float d[3];
						for ( int c = 0; c < 3; ++c )
							d[c] = sample.position[c] - at[c];
						const float d2 = std::max( Dot( d, d ), 1e-4f );
						const float length = std::sqrt( d2 );
						const float w[3] = { d[0] / length, d[1] / length, d[2] / length };
						const bool sphere = sample.normal[0] == 0.0f && sample.normal[1] == 0.0f &&
						                    sample.normal[2] == 0.0f;
						float cosLight = sphere ? 1.0f : -Dot( w, sample.normal );
						if ( cosLight <= 0.0f )
							continue;
						const float cone = SpotCone( light, cosLight );
						const float scale = m_portalLightScalars[l] * cone * cosLight /
						                    ( float( M_PI ) * d2 );
						const float value[3] = { sample.power[0] * scale, sample.power[1] * scale,
						    sample.power[2] * scale };
						use( w, value );
					}
				};
				for ( uint32_t p = 0; p < layout.patchCount; ++p )
				{
					const mapcontainer::RadiosityPatch &patch = view.Patch( p );
					each( patch.position, [&]( const float w[3], const float value[3] )
					    {
						    const float cosine = Dot( patch.normal, w );
						    if ( cosine > 0.0f )
							    for ( int c = 0; c < 3; ++c )
								    m_portalInjection[size_t( p ) * 3 + c] += value[c] * cosine;
					    } );
				}
				for ( uint32_t i = 0; i < layout.probeCount && i * 4 + 3 < probes.size(); ++i )
				{
					if ( probes[i * 4 + 3] < 0.5f )
						continue;
					float *out = &m_portalProbe[size_t( i ) * kTexels * 3];
					each( &probes[i * 4], [&]( const float w[3], const float value[3] )
					    {
						    for ( uint32_t t = 0; t < kTexels; ++t )
						    {
							    const float cosine = Dot( texels[t], w );
							    if ( cosine > 0.0f )
								    for ( int c = 0; c < 3; ++c )
									    out[t * 3 + c] += value[c] * cosine;
						    }
					    } );
				}
			}
	}

	// The bake's absolute patch light H( r ) (Jacobi to convergence), and
	// its reflected light rho H: what portal links carry besides the change.
	void SolveAbsolute()
	{
		const mapcontainer::RadiosityTransferView view = m_transfer->View();
		const auto &layout = m_transfer->layout;
		const size_t size = size_t( layout.patchCount ) * 3;
		std::vector<float> injection( size, 0.0f ), light( size, 0.0f ), next( size, 0.0f ),
		    reflected( size, 0.0f );
		for ( uint32_t s = 0; s < layout.sourceCount; ++s )
			for ( auto *l = view.InjectionBegin( s ), *end = view.InjectionEnd( s ); l != end; ++l )
				for ( int c = 0; c < 3; ++c )
					injection[size_t( l->patch ) * 3 + c] += m_reference[s] * l->light[c];
		light = injection;
		for ( int iteration = 0; iteration < 1000; ++iteration )
		{
			for ( size_t k = 0; k < size; ++k )
				reflected[k] = m_albedo[k] * light[k];
			float largest = 0.0f;
			for ( uint32_t p = 0; p < layout.patchCount; ++p )
			{
				float sum[3] = { injection[size_t( p ) * 3], injection[size_t( p ) * 3 + 1],
				    injection[size_t( p ) * 3 + 2] };
				for ( auto *l = view.TransferBegin( p ), *end = view.TransferEnd( p ); l != end;
				      ++l )
					for ( int c = 0; c < 3; ++c )
						sum[c] += l->factor * reflected[size_t( l->patch ) * 3 + c];
				for ( int c = 0; c < 3; ++c )
				{
					largest = std::max( largest, std::fabs( sum[c] - light[size_t( p ) * 3 + c] ) );
					next[size_t( p ) * 3 + c] = sum[c];
				}
			}
			light.swap( next );
			if ( largest < m_options.convergedChange )
				break;
		}
		m_absoluteReflected.resize( size );
		for ( size_t k = 0; k < size; ++k )
			m_absoluteReflected[k] = m_albedo[k] * light[k];
	}

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
			if ( !self.m_portalInjection.empty() )
				for ( int c = 0; c < 3; ++c )
					sum[c] += self.m_portalInjection[size_t( p ) * 3 + c];
			for ( auto *l = view.TransferBegin( p ), *end = view.TransferEnd( p ); l != end; ++l )
			{
				const float *q = reflected + size_t( l->patch ) * 3;
				sum[0] += l->factor * q[0];
				sum[1] += l->factor * q[1];
				sum[2] += l->factor * q[2];
			}
			// Through open portals: the source's whole reflected light.
			if ( !self.m_portalLinks.empty() )
				for ( uint32_t e = self.m_portalRow[p]; e < self.m_portalRow[p + 1]; ++e )
				{
					const PortalLink &l = self.m_portalLinks[e];
					const float *q = reflected + size_t( l.patch ) * 3;
					const float *h = self.m_absoluteReflected.data() + size_t( l.patch ) * 3;
					for ( int c = 0; c < 3; ++c )
						sum[c] += l.factor * ( q[c] + h[c] );
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
			if ( !self.m_portalGather.empty() )
				for ( uint32_t e = self.m_portalGatherRow[i]; e < self.m_portalGatherRow[i + 1]; ++e )
				{
					const PortalGatherLink &l = self.m_portalGather[e];
					const float *q = reflected + size_t( l.patch ) * 3;
					const float *h = self.m_absoluteReflected.data() + size_t( l.patch ) * 3;
					for ( int c = 0; c < 3; ++c )
						for ( int k = 0; k < 9; ++k )
							sh[c][k] += l.sh[k] * ( q[c] + h[c] );
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
					// Lights through open portals: not in the bake, so the
					// indirect layer's (G10).
					if ( !self.m_portalProbe.empty() )
						value += self.m_portalProbe[( size_t( i ) * kTexels + t ) * 3 + c];
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
	std::vector<Portal> m_portals;             // G10: the open portals
	std::vector<uint32_t> m_portalRow;         // patch p's portal links [row[p], row[p + 1])
	std::vector<PortalLink> m_portalLinks;
	std::vector<uint32_t> m_portalGatherRow;   // probe i's portal gather links
	std::vector<PortalGatherLink> m_portalGather;
	std::vector<float> m_absoluteReflected;    // rho H( r ), for portal links
	std::vector<mapcontainer::SdfLight> m_portalLights; // the SDFV's analytic lights
	std::vector<float> m_portalLightScalars;
	std::vector<float> m_portalInjection;      // their light through the portals, per patch
	std::vector<float> m_portalProbe;          // and per probe texel
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
		caps.responds = kLightIntensity | kEmission | kPortalTransport;
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
		m_probePositions = ProbePositions( *m_baked );
		// The analytic lights a portal's far side may hold (G10), from the
		// map's SDFV when it has one.
		std::vector<mapcontainer::SdfLight> portalLights;
		if ( scene.sdf )
		{
			const mapcontainer::SdfVolumeLayout &f = scene.sdf->layout;
			portalLights.resize( f.lightCount );
			std::memcpy( portalLights.data(), scene.sdf->bytes.data() + f.lightOffset,
			    portalLights.size() * sizeof( mapcontainer::SdfLight ) );
		}
		m_solver.Init( scene.transfer, std::vector<float>( layout.sourceCount, 1.0f ), m_options );
		m_solver.SetPortalLights( std::move( portalLights ) );
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
		m_solver.SetPortals( work.portals, m_probePositions, lights.styleScalars );
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
	std::vector<float> m_probePositions; // xyz, w active: the portal gather's probes
	std::optional<PublishedVolume> m_published;
	uint64_t m_epoch = 0;
};

} // namespace indirect_light

#endif // RENDER_INDIRECT_RADIOSITY_H
