//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Baked direct light blocked by moving geometry (RFC 0011): the
//          lightmap holds each texel's direct light from the map's lights
//          as the bake saw them (a door open); when a moving occluder (a
//          proxy box: a closed door) stands between a texel and a light,
//          that light's share of the texel's direct light is removed from
//          the total the world draws. Producers own indirect light; this
//          owns the baked direct light's visibility, whichever producer runs.
//
//          Build (once per map): each lightmap texel's world position and
//          normal, from the WMSH triangles rasterized in lightmap space, and
//          the map's analytic lights (SDFV records: rectangles sampled 4 x 4,
//          spheres and spots at their centres, distant lights by
//          direction). Compose (when the occluders
//          change): per texel, the light of the samples whose path a proxy
//          blocks, as a share of the light the texel sees (its baked direct
//          light: static occlusion is in the bake and is not re-traced), times
//          its direct layer, subtracted from its total. A texel no path of
//          which a proxy blocks keeps its baked bytes exactly. What a proxy
//          blocks is assumed visible: a proxy hidden behind static geometry
//          from a texel over-darkens it.
//
//===========================================================================//

#ifndef RENDER_DIRECT_OCCLUSION_H
#define RENDER_DIRECT_OCCLUSION_H

#include "mapcontainer/sdf_volume.h"
#include "mapcontainer/world_lightmap.h"
#include "mapcontainer/world_mesh.h"
#include "render/light_set.h"
#include "mapcontainer/world_mesh_format.h"
#include "render/indirect_light.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace indirect_light
{

class DirectOcclusion
{
public:
	static constexpr uint32_t kRectSamples = 4; // per axis
	static constexpr int kDilation = 2;         // texels grown past coverage

	// A range of the world mesh's indices (whole triangles).
	struct IndexRange
	{
		uint32_t first = 0;
		uint32_t count = 0;
	};

	// False (and nothing built) without a direct layer, lights or triangles.
	// With `only`, just the texels of those index ranges' triangles are
	// covered (the surfaces whose baked direct light is still drawn: under
	// RFC 0016's runtime direct light the core draws the others' direct
	// light itself); an empty list builds nothing.
	[[nodiscard]] bool Build( const void *wmsh, size_t wmshSize, const void *lmap, size_t lmapSize,
	    uint32_t lmapVersion, std::span<const mapcontainer::SdfLight> lights,
	    const std::vector<IndexRange> *only = nullptr )
	{
		*this = DirectOcclusion();
		mapcontainer::WorldMeshSummary mesh = {};
		mapcontainer::WorldLightmapBlocks blocks = {};
		mapcontainer::WorldLightmapLayout layout = {};
		// LMAP v3 is block-compressed; recomposition works on its RGBA16F form.
		if ( mapcontainer::ValidateWorldMesh( wmsh, wmshSize, &mesh ) !=
		         mapcontainer::WorldMeshError::Ok ||
		     mapcontainer::ValidateWorldLightmap( lmap, lmapSize, lmapVersion, &blocks ) !=
		         mapcontainer::WorldLightmapError::Ok ||
		     mapcontainer::WorldLightmapLayerIndex(
		         blocks, mapcontainer::WorldLightmapLayer::Direct ) < 0 ||
		     !mapcontainer::DecodeWorldLightmap( lmap, blocks, &m_lmap, &layout ) )
			return false;
		const int total = mapcontainer::WorldLightmapLayerIndex(
		    layout, mapcontainer::WorldLightmapLayer::Total );
		const int direct = mapcontainer::WorldLightmapLayerIndex(
		    layout, mapcontainer::WorldLightmapLayer::Direct );
		if ( total < 0 || direct < 0 )
			return false;
		// The triangles: float3 positions and the lightmap atlas UVs.
		const unsigned char *base = static_cast<const unsigned char *>( wmsh );
		std::vector<float> positions( size_t( mesh.vertexCount ) * 3 );
		std::vector<float> uvs( size_t( mesh.vertexCount ) * 2 );
		for ( uint32_t v = 0; v < mesh.vertexCount; ++v )
		{
			const unsigned char *vertex =
			    base + mesh.sectionOffsets[0] + size_t( v ) * mapcontainer::kWorldMeshVertexSize;
			std::memcpy( &positions[size_t( v ) * 3], vertex, 12 );
			std::memcpy( &uvs[size_t( v ) * 2], vertex + 32, 8 );
		}
		std::vector<uint32_t> indices( mesh.indexCount );
		std::memcpy( indices.data(), base + mesh.sectionOffsets[1], indices.size() * 4 );

		m_layout = layout;
		m_totalIndex = total;
		m_directIndex = direct;
		return BuildLayers( positions, uvs, indices, layout.width, layout.height,
		    Bytes() + layout.layerOffset[total], Bytes() + layout.layerOffset[direct], lights,
		    only );
	}

	// The core, for any source of triangles: xyz positions, lightmap UVs in
	// [0, 1] of the page's flat light, and RGBA16F total and direct layers of
	// width x height texels (rows top first), which must outlive this. With
	// `only`, just those index ranges' triangles (Build).
	[[nodiscard]] bool BuildLayers( const std::vector<float> &positions,
	    const std::vector<float> &uvs, const std::vector<uint32_t> &allIndices, uint32_t width,
	    uint32_t height, const unsigned char *total, const unsigned char *direct,
	    std::span<const mapcontainer::SdfLight> lights,
	    const std::vector<IndexRange> *only = nullptr )
	{
		std::vector<uint32_t> kept;
		if ( only )
		{
			for ( const IndexRange &range : *only )
			{
				if ( range.first > allIndices.size() ||
				     range.count > allIndices.size() - range.first )
					return false;
				const uint32_t whole = range.count - range.count % 3;
				kept.insert( kept.end(), allIndices.begin() + range.first,
				    allIndices.begin() + range.first + whole );
			}
			if ( kept.empty() )
			{
				*this = DirectOcclusion();
				return false;
			}
		}
		const std::vector<uint32_t> &indices = only ? kept : allIndices;
		m_texels.clear();
		m_samples.clear();
		for ( const mapcontainer::SdfLight &light : lights )
			AddLight( light );
		// The bounds of every rectangle sample, for the per-texel prefilter.
		m_hasDistant = false;
		for ( int k = 0; k < 3; ++k )
		{
			m_lightLo[k] = m_distantLo[k] = 1e30f;
			m_lightHi[k] = m_distantHi[k] = -1e30f;
		}
		for ( const Sample &sample : m_samples )
			for ( int k = 0; k < 3; ++k )
			{
				const float point = sample.distant ? sample.point[k] * kFar : sample.point[k];
				// A distant light's paths end kFar away along its direction:
				// its bound is relative to the texel (added per texel below).
				if ( sample.distant )
				{
					m_distantLo[k] = std::min( m_distantLo[k], point );
					m_distantHi[k] = std::max( m_distantHi[k], point );
					m_hasDistant = true;
				}
				else
				{
					m_lightLo[k] = std::min( m_lightLo[k], point );
					m_lightHi[k] = std::max( m_lightHi[k], point );
				}
			}
		if ( m_samples.empty() || !total || !direct || !width || !height )
			return false;
		m_width = width;
		m_height = height;
		m_total = total;
		m_direct = direct;
		m_layerBytes = size_t( width ) * height * 8;
		// A directional page (2:1) keeps its flat light in the left half.
		m_flatWidth = m_width == 2 * m_height ? m_height : m_width;
		Rasterize( positions, uvs, indices );
		Orient();
		Cluster( lights );
		return !m_texels.empty();
	}

	// A rectangle of the total layer, in texels.
	struct Rect
	{
		uint32_t x = 0;
		uint32_t y = 0;
		uint32_t width = 0;
		uint32_t height = 0;
	};
	// The side of the square tiles Recompose reports changed texels in.
	static constexpr uint32_t kDirtyTile = 64;

	[[nodiscard]] bool Ready() const { return !m_texels.empty(); }
	[[nodiscard]] const mapcontainer::WorldLightmapLayout &Layout() const { return m_layout; }
	// The LMAP bytes (layer offsets per Layout()).
	[[nodiscard]] const unsigned char *Bytes() const
	{
		return reinterpret_cast<const unsigned char *>( m_lmap.data() );
	}
	[[nodiscard]] size_t CoveredTexels() const { return m_texels.size(); }

	// The total layer for these occluders (RGBA16F, rows top first), in
	// `out`; the number of texels whose light a proxy blocks.
	size_t Compose( std::span<const Proxy> proxies, IBatchExecutor *executor,
	    std::vector<unsigned char> *out ) const
	{
		out->assign( m_total, m_total + m_layerBytes );
		if ( proxies.empty() )
			return 0;
		Context context{ this, proxies, out->data(), {} };
		const uint32_t blocks = uint32_t( ( m_texels.size() + kBlock - 1 ) / kBlock );
		context.changed.assign( blocks, 0 );
		if ( executor )
			executor->ParallelFor( "indirect.direct-occlusion", blocks, &Block, &context );
		else
			for ( uint32_t b = 0; b < blocks; ++b )
				Block( &context, b );
		size_t changed = 0;
		for ( uint32_t count : context.changed )
			changed += count;
		return changed;
	}

	// Compose for a change of occluders: `total` holds the layer Compose (or
	// Recompose) made for `previous` (an empty `total` stands for the bake:
	// it is filled first); it becomes the layer for `proxies`, equal to
	// Compose( proxies ) byte for byte. Only texels a path through a proxy
	// that appeared, left or moved can reach are recomposed: the texels are
	// clustered in space at Build, and a cluster (centre c, radius r) is
	// visited when some light's segment from c (to the light's centre, grown
	// by the light's radius R) passes within max( r, R ) of such a proxy:
	// every path from its texels to that light lies in that capsule. `dirty`
	// receives the kDirtyTile tiles holding a recomposed texel, rows merged,
	// in rows top first. Returns the recomposed texels.
	size_t Recompose( std::span<const Proxy> previous, std::span<const Proxy> proxies,
	    IBatchExecutor *executor, std::vector<unsigned char> *total,
	    std::vector<Rect> *dirty ) const
	{
		dirty->clear();
		std::vector<Proxy> changed;
		if ( total->size() != m_layerBytes )
		{
			total->assign( m_total, m_total + m_layerBytes );
			changed.assign( proxies.begin(), proxies.end() );
		}
		else
		{
			const auto contains = []( std::span<const Proxy> set, const Proxy &proxy )
			{
				return std::find( set.begin(), set.end(), proxy ) != set.end();
			};
			for ( const Proxy &proxy : proxies )
				if ( !contains( previous, proxy ) )
					changed.push_back( proxy );
			for ( const Proxy &proxy : previous )
				if ( !contains( proxies, proxy ) )
					changed.push_back( proxy );
		}
		if ( changed.empty() || m_clusters.empty() )
			return 0;
		// The clusters a changed proxy can reach: superclusters first, then
		// the clusters of each one reached, on the executor.
		std::vector<std::vector<uint32_t>> reached( m_supers.size() );
		struct Find
		{
			const DirectOcclusion *self;
			const std::vector<Proxy> *changed;
			std::vector<std::vector<uint32_t>> *reached;
		} find{ this, &changed, &reached };
		const auto findClusters = []( void *raw, uint32_t s )
		{
			const Find &f = *static_cast<const Find *>( raw );
			const DirectOcclusion &self = *f.self;
			if ( !self.Reaches( self.m_supers[s], *f.changed ) )
				return;
			const uint32_t first = s * kSuperClusters;
			const uint32_t last =
			    std::min( uint32_t( self.m_clusters.size() ), first + kSuperClusters );
			for ( uint32_t c = first; c < last; ++c )
				if ( self.Reaches( self.m_clusters[c], *f.changed ) )
					( *f.reached )[s].push_back( c );
		};
		if ( executor && m_supers.size() > 1 )
			executor->ParallelFor( "indirect.direct-occlusion.find", uint32_t( m_supers.size() ),
			    findClusters, &find );
		else
			for ( uint32_t s = 0; s < m_supers.size(); ++s )
				findClusters( &find, s );
		std::vector<uint32_t> candidates;
		for ( const std::vector<uint32_t> &clusters : reached )
			candidates.insert( candidates.end(), clusters.begin(), clusters.end() );
		if ( candidates.empty() )
			return 0;
		// The tiles the recomposed texels are in.
		const uint32_t tilesX = ( m_width + kDirtyTile - 1 ) / kDirtyTile;
		const uint32_t tilesY = ( m_height + kDirtyTile - 1 ) / kDirtyTile;
		std::vector<uint8_t> tiles( size_t( tilesX ) * tilesY, 0 );
		size_t texels = 0;
		for ( uint32_t c : candidates )
		{
			const size_t end = std::min( m_texels.size(), size_t( c + 1 ) * kClusterTexels );
			for ( size_t i = size_t( c ) * kClusterTexels; i < end; ++i )
			{
				const uint32_t index = m_texels[i].index;
				tiles[size_t( index / m_width / kDirtyTile ) * tilesX +
				      ( index % m_width ) / kDirtyTile] = 1;
				++texels;
			}
		}
		// Each texel from the bake again, less what the proxies now block.
		struct Redo
		{
			const DirectOcclusion *self;
			std::span<const Proxy> proxies;
			const std::vector<uint32_t> *candidates;
			unsigned char *total;
		} redo{ this, proxies, &candidates, total->data() };
		const auto recompose = []( void *raw, uint32_t k )
		{
			const Redo &r = *static_cast<const Redo *>( raw );
			const uint32_t c = ( *r.candidates )[k];
			const size_t end =
			    std::min( r.self->m_texels.size(), size_t( c + 1 ) * kClusterTexels );
			std::vector<const Proxy *> near;
			std::vector<uint64_t> masks;
			const bool masked = r.self->GroupMasks( r.self->m_clusters[c], r.proxies, &masks );
			for ( size_t i = size_t( c ) * kClusterTexels; i < end; ++i )
			{
				const size_t at = size_t( r.self->m_texels[i].index ) * 8;
				std::memcpy( r.total + at, r.self->m_total + at, 8 );
				if ( masked )
					r.self->ComposeTexelMasked( r.self->m_texels[i], r.proxies, masks, r.total );
				else
					r.self->ComposeTexel( r.self->m_texels[i], r.proxies, r.total, near );
			}
		};
		if ( executor && candidates.size() > 1 )
			executor->ParallelFor( "indirect.direct-occlusion.recompose",
			    uint32_t( candidates.size() ), recompose, &redo );
		else
			for ( uint32_t k = 0; k < candidates.size(); ++k )
				recompose( &redo, k );
		for ( uint32_t ty = 0; ty < tilesY; ++ty )
			for ( uint32_t tx = 0; tx < tilesX; ++tx )
			{
				if ( !tiles[size_t( ty ) * tilesX + tx] )
					continue;
				const uint32_t x = tx * kDirtyTile;
				const uint32_t y = ty * kDirtyTile;
				const uint32_t width = std::min( kDirtyTile, m_width - x );
				const uint32_t height = std::min( kDirtyTile, m_height - y );
				if ( !dirty->empty() && dirty->back().y == y &&
				     dirty->back().x + dirty->back().width == x )
					dirty->back().width += width;
				else
					dirty->push_back( { x, y, width, height } );
			}
		return texels;
	}

private:
	static constexpr uint32_t kClusterTexels = 64; // texels per cluster, in Morton order
	static constexpr uint32_t kSuperClusters = 64; // clusters per supercluster

	// A cluster's bounds: the box of its texels' path origins, as a centre
	// and the radius of the sphere around the box.
	struct Bounds
	{
		float center[3];
		float radius;
	};
	// A light for the cluster test: its centre (a distant light's direction)
	// and the radius of the sphere its samples lie in.
	struct LightBound
	{
		bool distant;
		float point[3];
		float radius;
	};
	static constexpr uint32_t kBlock = 4096;
	static constexpr float kFar = 1.0e6f; // a distant light's segment length
	static constexpr double kPi = 3.14159265358979323846;

	struct Texel
	{
		uint32_t index; // y * width + x
		float position[3];
		float normal[3];
	};
	// A point light sample: a rectangle's patch (position, emitting normal,
	// radiance times its area) or a distant light (direction toward it).
	struct Sample
	{
		bool distant;
		float point[3];
		float normal[3];
		float power;       // luminance: radiance x (projected) area, or distant irradiance
		bool omni = false; // a sphere: no emitting side
		// A spot's vrad cone: cosines of the inner and outer cones and the
		// exponent; inner < -1 when the sample has none.
		float cone[3] = { -2.0f, -2.0f, 1.0f };
		// A sphere's or spot's Source falloff (the SDFV record's; all zero:
		// none), relative to the inverse square Weight applies.
		float attenuation[4] = {};
	};
	struct Context
	{
		const DirectOcclusion *self;
		std::span<const Proxy> proxies;
		unsigned char *total;
		std::vector<uint32_t> changed;
	};

	static float Luminance( const float rgb[3] )
	{
		return 0.2126f * rgb[0] + 0.7152f * rgb[1] + 0.0722f * rgb[2];
	}

	void AddLight( const mapcontainer::SdfLight &light )
	{
		if ( light.kind == uint32_t( mapcontainer::SdfLightKind::Rect ) )
		{
			float normal[3] = { light.b[1] * light.c[2] - light.b[2] * light.c[1],
			    light.b[2] * light.c[0] - light.b[0] * light.c[2],
			    light.b[0] * light.c[1] - light.b[1] * light.c[0] };
			const float area = 4.0f * std::sqrt( normal[0] * normal[0] + normal[1] * normal[1] +
			                                     normal[2] * normal[2] );
			if ( !( area > 0.0f ) )
				return;
			for ( float &n : normal )
				n *= 4.0f / area;
			const uint32_t n = kRectSamples;
			for ( uint32_t i = 0; i < n; ++i )
				for ( uint32_t j = 0; j < n; ++j )
				{
					Sample sample = {};
					const float u = ( 2.0f * ( float( i ) + 0.5f ) / float( n ) ) - 1.0f;
					const float v = ( 2.0f * ( float( j ) + 0.5f ) / float( n ) ) - 1.0f;
					for ( int k = 0; k < 3; ++k )
					{
						sample.point[k] = light.a[k] + u * light.b[k] + v * light.c[k];
						sample.normal[k] = normal[k];
					}
					sample.power = Luminance( light.rgb ) * area / float( n * n );
					m_samples.push_back( sample );
				}
		}
		else if ( light.kind == uint32_t( mapcontainer::SdfLightKind::Distant ) )
		{
			Sample sample = {};
			sample.distant = true;
			for ( int k = 0; k < 3; ++k )
				sample.point[k] = -light.a[k]; // toward the light
			sample.power = Luminance( light.rgb );
			m_samples.push_back( sample );
		}
		else if ( light.kind == uint32_t( mapcontainer::SdfLightKind::Sphere ) ||
		          light.kind == uint32_t( mapcontainer::SdfLightKind::Spot ) )
		{
			// One sample at the centre: these lights are small. A sphere
			// shows every side its disc of pi r^2; a spot is a one-sided disk
			// with vrad's cone.
			const bool sphere = light.kind == uint32_t( mapcontainer::SdfLightKind::Sphere );
			const float radius = sphere ? light.b[0] : light.c[0];
			Sample sample = {};
			for ( int k = 0; k < 3; ++k )
			{
				sample.point[k] = light.a[k];
				sample.normal[k] = sphere ? 0.0f : light.b[k];
			}
			sample.power = Luminance( light.rgb ) * 3.14159265f * radius * radius;
			sample.omni = sphere;
			for ( int k = 0; k < 4; ++k )
				sample.attenuation[k] = light.attenuation[k];
			if ( !sphere )
			{
				sample.cone[0] = light.c[1];
				sample.cone[1] = light.c[2];
				sample.cone[2] = light.reserved[0];
			}
			m_samples.push_back( sample );
		}
		// A dome's direct light is sky, which a box blocks little of: kept.
	}

	static float Half( const unsigned char *p )
	{
		uint16_t half;
		std::memcpy( &half, p, 2 );
		return mapcontainer::HalfToFloat( half );
	}

	// Texel centres covered by a triangle in lightmap space get its
	// interpolated position and its face normal (the last triangle wins).
	void Rasterize( const std::vector<float> &positions, const std::vector<float> &uvs,
	    const std::vector<uint32_t> &indices )
	{
		std::vector<int32_t> slot( size_t( m_width ) * m_height, -1 );
		const size_t vertexCount = std::min( positions.size() / 3, uvs.size() / 2 );
		for ( size_t t = 0; t + 2 < indices.size(); t += 3 )
		{
			float p[3][3], uv[3][2];
			for ( int c = 0; c < 3; ++c )
			{
				const uint32_t index = indices[t + size_t( c )];
				if ( index >= vertexCount )
					return;
				std::memcpy( p[c], &positions[size_t( index ) * 3], 12 );
				uv[c][0] = uvs[size_t( index ) * 2] * float( m_flatWidth );
				uv[c][1] = uvs[size_t( index ) * 2 + 1] * float( m_height );
			}
			float normal[3];
			const float e1[3] = { p[1][0] - p[0][0], p[1][1] - p[0][1], p[1][2] - p[0][2] };
			const float e2[3] = { p[2][0] - p[0][0], p[2][1] - p[0][1], p[2][2] - p[0][2] };
			normal[0] = e1[1] * e2[2] - e1[2] * e2[1];
			normal[1] = e1[2] * e2[0] - e1[0] * e2[2];
			normal[2] = e1[0] * e2[1] - e1[1] * e2[0];
			const float length =
			    std::sqrt( normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
			const float area2 = ( uv[1][0] - uv[0][0] ) * ( uv[2][1] - uv[0][1] ) -
			                    ( uv[2][0] - uv[0][0] ) * ( uv[1][1] - uv[0][1] );
			if ( !( length > 0.0f ) || std::fabs( area2 ) < 1e-12f )
				continue;
			for ( float &n : normal )
				n /= length;
			const int x0 =
			    std::max( 0, int( std::floor( std::min( { uv[0][0], uv[1][0], uv[2][0] } ) ) ) );
			const int x1 = std::min( int( m_flatWidth ) - 1,
			    int( std::ceil( std::max( { uv[0][0], uv[1][0], uv[2][0] } ) ) ) );
			const int y0 =
			    std::max( 0, int( std::floor( std::min( { uv[0][1], uv[1][1], uv[2][1] } ) ) ) );
			const int y1 = std::min( int( m_height ) - 1,
			    int( std::ceil( std::max( { uv[0][1], uv[1][1], uv[2][1] } ) ) ) );
			for ( int y = y0; y <= y1; ++y )
				for ( int x = x0; x <= x1; ++x )
				{
					const float cx = float( x ) + 0.5f, cy = float( y ) + 0.5f;
					const float w1 = ( ( cx - uv[0][0] ) * ( uv[2][1] - uv[0][1] ) -
					                     ( uv[2][0] - uv[0][0] ) * ( cy - uv[0][1] ) ) /
					                 area2;
					const float w2 = ( ( uv[1][0] - uv[0][0] ) * ( cy - uv[0][1] ) -
					                     ( cx - uv[0][0] ) * ( uv[1][1] - uv[0][1] ) ) /
					                 area2;
					const float w0 = 1.0f - w1 - w2;
					if ( w0 < -1e-4f || w1 < -1e-4f || w2 < -1e-4f )
						continue;
					Texel texel = {};
					texel.index = uint32_t( y ) * m_width + uint32_t( x );
					for ( int k = 0; k < 3; ++k )
					{
						texel.position[k] = w0 * p[0][k] + w1 * p[1][k] + w2 * p[2][k];
						texel.normal[k] = normal[k];
					}
					int32_t &at = slot[texel.index];
					if ( at < 0 )
					{
						at = int32_t( m_texels.size() );
						m_texels.push_back( texel );
					}
					else
						m_texels[size_t( at )] = texel;
				}
		}
		// Texels no triangle covers but filtering reads (chart borders, and
		// the seams between a chart's triangles) take a covered neighbour's
		// surface: otherwise they keep their baked light and draw lines.
		for ( int pass = 0; pass < kDilation; ++pass )
		{
			std::vector<Texel> grown;
			for ( uint32_t y = 0; y < m_height; ++y )
				for ( uint32_t x = 0; x < m_flatWidth; ++x )
				{
					const uint32_t index = y * m_width + x;
					if ( slot[index] >= 0 )
						continue;
					for ( int dy = -1; dy <= 1 && slot[index] < 0; ++dy )
						for ( int dx = -1; dx <= 1; ++dx )
						{
							const int nx = int( x ) + dx, ny = int( y ) + dy;
							if ( nx < 0 || ny < 0 || nx >= int( m_flatWidth ) ||
							     ny >= int( m_height ) )
								continue;
							const int32_t neighbour = slot[size_t( ny ) * m_width + size_t( nx )];
							if ( neighbour < 0 || neighbour >= int32_t( m_texels.size() ) )
								continue;
							Texel texel = m_texels[size_t( neighbour )];
							texel.index = index;
							grown.push_back( texel );
							slot[index] = int32_t( m_texels.size() + grown.size() ); // claimed
							break;
						}
				}
			for ( Texel &texel : grown )
			{
				slot[texel.index] = int32_t( m_texels.size() );
				m_texels.push_back( texel );
			}
		}
	}

	// The unoccluded light of the samples at a point with this normal.
	double Unoccluded( const float position[3], const float normal[3] ) const
	{
		double all = 0.0;
		for ( const Sample &sample : m_samples )
		{
			float d[3];
			all += Weight( sample, position, normal, d );
		}
		return all;
	}

	// A sample's unoccluded light at `origin` for `normal`, and the path to
	// it in `d`.
	static float Weight(
	    const Sample &sample, const float origin[3], const float normal[3], float d[3] )
	{
		if ( sample.distant )
		{
			for ( int k = 0; k < 3; ++k )
				d[k] = sample.point[k] * kFar;
			return sample.power *
			       std::max( 0.0f, normal[0] * sample.point[0] + normal[1] * sample.point[1] +
			                           normal[2] * sample.point[2] );
		}
		for ( int k = 0; k < 3; ++k )
			d[k] = sample.point[k] - origin[k];
		const float d2 = std::max( d[0] * d[0] + d[1] * d[1] + d[2] * d[2], 1.0f );
		const float inv = 1.0f / std::sqrt( d2 );
		const float cosSurface = ( normal[0] * d[0] + normal[1] * d[1] + normal[2] * d[2] ) * inv;
		float cosLight =
		    sample.omni
		        ? 1.0f
		        : -( sample.normal[0] * d[0] + sample.normal[1] * d[1] + sample.normal[2] * d[2] ) *
		              inv;
		cosLight = std::max( 0.0f, cosLight );
		float cone = 1.0f;
		if ( sample.cone[0] >= -1.0f && cosLight < sample.cone[0] )
		{
			cone = std::clamp(
			    ( cosLight - sample.cone[1] ) / std::max( sample.cone[0] - sample.cone[1], 1e-6f ),
			    0.0f, 1.0f );
			if ( sample.cone[2] != 0.0f && sample.cone[2] != 1.0f )
				cone = std::pow( cone, sample.cone[2] );
		}
		return sample.power * std::max( 0.0f, cosSurface ) * cosLight * cone / d2 *
		       light_set::AttenuationRelative( d2, sample.attenuation );
	}

	// A triangle's winding does not say which side the bake lit (fan
	// triangulations mix them): each texel faces the side its lights reach.
	void Orient()
	{
		for ( Texel &texel : m_texels )
		{
			const float flipped[3] = { -texel.normal[0], -texel.normal[1], -texel.normal[2] };
			if ( Unoccluded( texel.position, flipped ) >
			     Unoccluded( texel.position, texel.normal ) )
				for ( float &n : texel.normal )
					n = -n;
		}
	}

	// Whether the segment from `a` along `d` (t in [0, 1]) passes through the
	// box. The origin is already lifted off its surface.
	static bool Blocks( const float a[3], const float d[3], const Proxy &box )
	{
		float enter = 0.0f, leave = 1.0f;
		for ( int k = 0; k < 3; ++k )
		{
			if ( std::fabs( d[k] ) < 1e-12f )
			{
				if ( a[k] < box.lo[k] || a[k] > box.hi[k] )
					return false;
				continue;
			}
			float t0 = ( box.lo[k] - a[k] ) / d[k];
			float t1 = ( box.hi[k] - a[k] ) / d[k];
			if ( t0 > t1 )
				std::swap( t0, t1 );
			enter = std::max( enter, t0 );
			leave = std::min( leave, t1 );
			if ( enter > leave )
				return false;
		}
		return true;
	}

	// One texel of Compose: `total` holds its baked light; the share of its
	// direct light the proxies block is removed. True when some was.
	bool ComposeTexel( const Texel &texel, std::span<const Proxy> proxies, unsigned char *total,
	    std::vector<const Proxy *> &near ) const
	{
		const unsigned char *direct = m_direct;
		// A little off the surface, so the texel's own face never blocks.
		float origin[3];
		for ( int k = 0; k < 3; ++k )
			origin[k] = texel.position[k] + texel.normal[k] * 0.5f;
		// Prefilter: only a proxy overlapping the bounds of every path from
		// this texel (the texel, the rectangles' samples, the distant
		// lights' far ends) can block one; most texels have none.
		float lo[3], hi[3];
		for ( int k = 0; k < 3; ++k )
		{
			lo[k] = std::min( origin[k], m_lightLo[k] );
			hi[k] = std::max( origin[k], m_lightHi[k] );
			if ( m_hasDistant )
			{
				lo[k] = std::min( lo[k], origin[k] + m_distantLo[k] );
				hi[k] = std::max( hi[k], origin[k] + m_distantHi[k] );
			}
		}
		near.clear();
		for ( const Proxy &proxy : proxies )
			if ( proxy.lo[0] <= hi[0] && proxy.hi[0] >= lo[0] && proxy.lo[1] <= hi[1] &&
			     proxy.hi[1] >= lo[1] && proxy.lo[2] <= hi[2] && proxy.hi[2] >= lo[2] )
				near.push_back( &proxy );
		if ( near.empty() )
			return false;
		double all = 0.0, blocked = 0.0;
		for ( const Sample &sample : m_samples )
		{
			float d[3];
			const float weight = Weight( sample, origin, texel.normal, d );
			if ( !( weight > 0.0f ) )
				continue;
			all += weight;
			for ( const Proxy *proxy : near )
				if ( Blocks( origin, d, *proxy ) )
				{
					blocked += weight;
					break;
				}
		}
		if ( !( blocked > 0.0 ) || !( all > 0.0 ) )
			return false;
		// Static occlusion is in the bake, not in these samples: the bake's
		// direct light over the samples' unoccluded light (both E / pi) is
		// the part of them the texel actually sees. A moving occluder blocks
		// a share of that part (a door shuts all of the light that came
		// through its doorway), assuming what it blocks was visible.
		const unsigned char *light = direct + size_t( texel.index ) * 8;
		const float baked[3] = { Half( light ), Half( light + 2 ), Half( light + 4 ) };
		const double visible = std::clamp( double( Luminance( baked ) ) * kPi, 1e-9, all );
		const float share = float( std::min( 1.0, blocked / visible ) );
		unsigned char *out = total + size_t( texel.index ) * 8;
		for ( int c = 0; c < 3; ++c )
		{
			const float value =
			    std::max( 0.0f, Half( out + 2 * c ) - share * Half( light + 2 * c ) );
			const uint16_t half = FloatToHalf( value );
			std::memcpy( out + 2 * c, &half, 2 );
		}
		return true;
	}

	// Whether a path from the cluster to some light can pass through one of
	// `proxies`: the segment from its centre to the light's, against each
	// box grown by the larger of the two radii (Recompose).
	bool Reaches( const Bounds &bounds, std::span<const Proxy> proxies ) const
	{
		for ( const LightBound &light : m_lightBounds )
			for ( const Proxy &proxy : proxies )
				if ( Reaches( bounds, light, proxy ) )
					return true;
		return false;
	}

	bool Reaches( const Bounds &bounds, const LightBound &light, const Proxy &proxy ) const
	{
		float d[3];
		for ( int k = 0; k < 3; ++k )
			d[k] = light.distant ? light.point[k] * kFar : light.point[k] - bounds.center[k];
		const float grow = std::max( bounds.radius, light.radius ) + 1.0f;
		Proxy grown = proxy;
		for ( int k = 0; k < 3; ++k )
		{
			grown.lo[k] -= grow;
			grown.hi[k] += grow;
		}
		return Blocks( bounds.center, d, grown );
	}

	// Per light group, the proxies (bits, the first 64) a path from the
	// cluster to one of its samples can pass through. False when there are
	// more proxies than bits (the caller then composes every sample).
	bool GroupMasks(
	    const Bounds &bounds, std::span<const Proxy> proxies, std::vector<uint64_t> *masks ) const
	{
		if ( proxies.size() > 64 )
			return false;
		masks->assign( m_lightBounds.size(), 0 );
		for ( size_t g = 0; g < m_lightBounds.size(); ++g )
			for ( size_t p = 0; p < proxies.size(); ++p )
				if ( Reaches( bounds, m_lightBounds[g], proxies[p] ) )
					( *masks )[g] |= uint64_t( 1 ) << p;
		return true;
	}

	// ComposeTexel with the cluster's GroupMasks: a sample whose group no
	// proxy reaches blocks nothing, so only the others are traced; the
	// unoccluded light (the share's bound) is summed only for a texel some
	// proxy blocks. The sums run in sample order: byte for byte the same.
	bool ComposeTexelMasked( const Texel &texel, std::span<const Proxy> proxies,
	    const std::vector<uint64_t> &masks, unsigned char *total ) const
	{
		float origin[3];
		for ( int k = 0; k < 3; ++k )
			origin[k] = texel.position[k] + texel.normal[k] * 0.5f;
		double blocked = 0.0;
		for ( size_t i = 0; i < m_samples.size(); ++i )
		{
			uint64_t mask = masks[m_sampleGroup[i]];
			if ( !mask )
				continue;
			float d[3];
			const float weight = Weight( m_samples[i], origin, texel.normal, d );
			if ( !( weight > 0.0f ) )
				continue;
			for ( ; mask; mask &= mask - 1 )
				if ( Blocks( origin, d, proxies[size_t( std::countr_zero( mask ) )] ) )
				{
					blocked += weight;
					break;
				}
		}
		if ( !( blocked > 0.0 ) )
			return false;
		double all = 0.0;
		for ( const Sample &sample : m_samples )
		{
			float d[3];
			const float weight = Weight( sample, origin, texel.normal, d );
			if ( weight > 0.0f )
				all += weight;
		}
		if ( !( all > 0.0 ) )
			return false;
		const unsigned char *light = m_direct + size_t( texel.index ) * 8;
		const float baked[3] = { Half( light ), Half( light + 2 ), Half( light + 4 ) };
		const double visible = std::clamp( double( Luminance( baked ) ) * kPi, 1e-9, all );
		const float share = float( std::min( 1.0, blocked / visible ) );
		unsigned char *out = total + size_t( texel.index ) * 8;
		for ( int c = 0; c < 3; ++c )
		{
			const float value =
			    std::max( 0.0f, Half( out + 2 * c ) - share * Half( light + 2 * c ) );
			const uint16_t half = FloatToHalf( value );
			std::memcpy( out + 2 * c, &half, 2 );
		}
		return true;
	}

	// Recompose's clusters: the texels in Morton order of their path
	// origins, kClusterTexels a cluster, kSuperClusters clusters a
	// supercluster; and each light's bound.
	void Cluster( std::span<const mapcontainer::SdfLight> lights )
	{
		float lo[3] = { 1e30f, 1e30f, 1e30f }, hi[3] = { -1e30f, -1e30f, -1e30f };
		const auto origin = []( const Texel &texel, int k )
		{
			return texel.position[k] + texel.normal[k] * 0.5f;
		};
		for ( const Texel &texel : m_texels )
			for ( int k = 0; k < 3; ++k )
			{
				lo[k] = std::min( lo[k], origin( texel, k ) );
				hi[k] = std::max( hi[k], origin( texel, k ) );
			}
		const auto spread = []( uint64_t v )
		{
			v &= 0x1fffff;
			v = ( v | v << 32 ) & 0x1f00000000ffffull;
			v = ( v | v << 16 ) & 0x1f0000ff0000ffull;
			v = ( v | v << 8 ) & 0x100f00f00f00f00full;
			v = ( v | v << 4 ) & 0x10c30c30c30c30c3ull;
			v = ( v | v << 2 ) & 0x1249249249249249ull;
			return v;
		};
		std::vector<std::pair<uint64_t, uint32_t>> keys( m_texels.size() );
		for ( size_t i = 0; i < m_texels.size(); ++i )
		{
			uint64_t code = 0;
			for ( int k = 0; k < 3; ++k )
			{
				const float extent = std::max( hi[k] - lo[k], 1e-3f );
				const uint64_t q = uint64_t(
				    std::clamp( ( origin( m_texels[i], k ) - lo[k] ) / extent, 0.0f, 1.0f ) *
				    2097151.0f );
				code |= spread( q ) << k;
			}
			keys[i] = { code, uint32_t( i ) };
		}
		std::sort( keys.begin(), keys.end() );
		std::vector<Texel> sorted( m_texels.size() );
		for ( size_t i = 0; i < keys.size(); ++i )
			sorted[i] = m_texels[keys[i].second];
		m_texels.swap( sorted );
		const auto bound = [&]( size_t first, size_t last )
		{
			float blo[3] = { 1e30f, 1e30f, 1e30f }, bhi[3] = { -1e30f, -1e30f, -1e30f };
			for ( size_t i = first; i < last; ++i )
				for ( int k = 0; k < 3; ++k )
				{
					blo[k] = std::min( blo[k], origin( m_texels[i], k ) );
					bhi[k] = std::max( bhi[k], origin( m_texels[i], k ) );
				}
			Bounds b = {};
			float r2 = 0.0f;
			for ( int k = 0; k < 3; ++k )
			{
				b.center[k] = 0.5f * ( blo[k] + bhi[k] );
				r2 += 0.25f * ( bhi[k] - blo[k] ) * ( bhi[k] - blo[k] );
			}
			b.radius = std::sqrt( r2 );
			return b;
		};
		m_clusters.clear();
		m_supers.clear();
		for ( size_t first = 0; first < m_texels.size(); first += kClusterTexels )
			m_clusters.push_back(
			    bound( first, std::min( m_texels.size(), first + kClusterTexels ) ) );
		const size_t superTexels = size_t( kClusterTexels ) * kSuperClusters;
		for ( size_t first = 0; first < m_texels.size(); first += superTexels )
			m_supers.push_back( bound( first, std::min( m_texels.size(), first + superTexels ) ) );
		// The light groups: each light's samples (AddLight's), a
		// rectangle's in quadrants of 2 x 2, bounded by a sphere.
		m_lightBounds.clear();
		m_sampleGroup.assign( m_samples.size(), 0 );
		size_t sample = 0;
		for ( const mapcontainer::SdfLight &light : lights )
		{
			const size_t count = SampleCount( light );
			if ( !count || sample + count > m_samples.size() )
			{
				sample += count;
				continue;
			}
			const bool rect = count == size_t( kRectSamples ) * kRectSamples;
			const size_t groups = rect ? 4 : 1;
			for ( size_t g = 0; g < groups; ++g )
			{
				std::vector<size_t> members;
				for ( size_t i = 0; i < count; ++i )
				{
					const size_t row = i / kRectSamples, column = i % kRectSamples;
					const size_t quadrant =
					    ( row * 2 / kRectSamples ) * 2 + column * 2 / kRectSamples;
					if ( !rect || quadrant == g )
						members.push_back( sample + i );
				}
				LightBound b = {};
				b.distant = m_samples[sample].distant;
				for ( size_t i : members )
					for ( int k = 0; k < 3; ++k )
						b.point[k] += m_samples[i].point[k] / float( members.size() );
				for ( size_t i : members )
				{
					float d2 = 0.0f;
					for ( int k = 0; k < 3; ++k )
						d2 += ( m_samples[i].point[k] - b.point[k] ) *
						      ( m_samples[i].point[k] - b.point[k] );
					b.radius = std::max( b.radius, std::sqrt( d2 ) );
					m_sampleGroup[i] = uint32_t( m_lightBounds.size() );
				}
				if ( b.distant )
					b.radius *= kFar; // directions: a spread of far ends
				m_lightBounds.push_back( b );
			}
			sample += count;
		}
	}

	// How many samples AddLight makes for `light`.
	static size_t SampleCount( const mapcontainer::SdfLight &light )
	{
		if ( light.kind == uint32_t( mapcontainer::SdfLightKind::Rect ) )
		{
			const float n[3] = { light.b[1] * light.c[2] - light.b[2] * light.c[1],
			    light.b[2] * light.c[0] - light.b[0] * light.c[2],
			    light.b[0] * light.c[1] - light.b[1] * light.c[0] };
			const float area = 4.0f * std::sqrt( n[0] * n[0] + n[1] * n[1] + n[2] * n[2] );
			return area > 0.0f ? size_t( kRectSamples ) * kRectSamples : 0;
		}
		if ( light.kind == uint32_t( mapcontainer::SdfLightKind::Distant ) ||
		     light.kind == uint32_t( mapcontainer::SdfLightKind::Sphere ) ||
		     light.kind == uint32_t( mapcontainer::SdfLightKind::Spot ) )
			return 1;
		return 0;
	}

	static void Block( void *opaque, uint32_t block )
	{
		Context &context = *static_cast<Context *>( opaque );
		const DirectOcclusion &self = *context.self;
		const size_t begin = size_t( block ) * kBlock;
		const size_t end = std::min( begin + kBlock, self.m_texels.size() );
		uint32_t changed = 0;
		std::vector<const Proxy *> near;
		for ( size_t i = begin; i < end; ++i )
			changed +=
			    self.ComposeTexel( self.m_texels[i], context.proxies, context.total, near ) ? 1 : 0;
		context.changed[block] = changed;
	}

	std::vector<Texel> m_texels;
	std::vector<Sample> m_samples;
	std::vector<Bounds> m_clusters;
	std::vector<Bounds> m_supers;
	std::vector<LightBound> m_lightBounds;
	std::vector<uint32_t> m_sampleGroup; // each sample's light group
	std::vector<std::byte> m_lmap;       // Build's RGBA16F decode of the LMAP
	const unsigned char *m_total = nullptr;
	const unsigned char *m_direct = nullptr;
	size_t m_layerBytes = 0;
	mapcontainer::WorldLightmapLayout m_layout = {};
	int m_totalIndex = -1;
	int m_directIndex = -1;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_flatWidth = 0;
	float m_lightLo[3] = {};
	float m_lightHi[3] = {};
	float m_distantLo[3] = { 1e30f, 1e30f, 1e30f };
	float m_distantHi[3] = { -1e30f, -1e30f, -1e30f };
	bool m_hasDistant = false;
};

// Moving geometry in a probe volume's visibility: the bake's distance
// moments do not know a door is shut, so the world and models would sample a
// probe on its far side through it (light leaking round the door). Each
// active probe's visibility direction that meets a proxy closer than its
// stored mean distance takes the proxy's distance (mean, and its square as
// the second moment), and the tile's octahedral border is rewritten. Returns
// the number of probes changed; a volume no proxy is near is unchanged.
// `cut`, when given, receives the changed probes (the grids' probes in grid
// order). Only probes within a proxy's reach are visited (its box grown by
// the visibility range and the largest relocation the format allows, in grid
// cells); `scanAll` visits every probe (the oracle the tests compare with).
inline size_t OccludeProbeVisibility( Volume &volume, std::span<const Proxy> proxies,
    std::vector<uint32_t> *cut = nullptr, bool scanAll = false, IBatchExecutor *executor = nullptr )
{
	if ( proxies.empty() )
		return 0;
	const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
	constexpr uint32_t kTile = mapcontainer::kProbeVisibilityTile;
	constexpr uint32_t kInterior = kTile - 2;
	unsigned char *atlas = volume.bytes.data() + layout.atlasOffset;
	const auto texel = [&]( uint32_t x, uint32_t y )
	{
		return atlas + ( size_t( y ) * layout.atlasWidth + x ) * 8;
	};
	const auto read = [&]( const unsigned char *p, int c )
	{
		uint16_t half;
		std::memcpy( &half, p + 2 * c, 2 );
		return mapcontainer::HalfToFloat( half );
	};
	const auto write = [&]( unsigned char *p, int c, float value )
	{
		const uint16_t half = FloatToHalf( value );
		std::memcpy( p + 2 * c, &half, 2 );
	};
	// Interior texel centres' directions (probe_volume.py interior_directions).
	float directions[kInterior][kInterior][3];
	for ( uint32_t v = 0; v < kInterior; ++v )
		for ( uint32_t u = 0; u < kInterior; ++u )
		{
			float px = ( float( u ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f;
			float py = ( float( v ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f;
			const float z = 1.0f - std::fabs( px ) - std::fabs( py );
			if ( z < 0.0f )
			{
				const float fx = ( 1.0f - std::fabs( py ) ) * ( px >= 0.0f ? 1.0f : -1.0f );
				const float fy = ( 1.0f - std::fabs( px ) ) * ( py >= 0.0f ? 1.0f : -1.0f );
				px = fx;
				py = fy;
			}
			const float length = std::sqrt( px * px + py * py + z * z );
			directions[v][u][0] = px / length;
			directions[v][u][1] = py / length;
			directions[v][u][2] = z / length;
		}
	size_t changedProbes = 0;
	uint32_t first = 0; // the grid's first probe
	for ( uint32_t g = 0; g < layout.gridCount; first += layout.grids[g].probeCount, ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
		const uint32_t stateRow = grid.tilesPerRow * kTile;
		std::vector<uint32_t> candidates;
		if ( scanAll )
		{
			candidates.resize( grid.probeCount );
			for ( uint32_t i = 0; i < grid.probeCount; ++i )
				candidates[i] = i;
		}
		else
		{
			const float reach = grid.maxDistance + grid.maxRelocation * 1.001f + 1.0e-3f;
			for ( const Proxy &proxy : proxies )
			{
				uint32_t lo[3] = {}, hi[3] = {};
				bool outside = false;
				for ( int k = 0; k < 3 && !outside; ++k )
				{
					const float last = float( grid.dims[k] - 1 );
					const float a = ( proxy.lo[k] - reach - grid.origin[k] ) / grid.spacing[k];
					const float b = ( proxy.hi[k] + reach - grid.origin[k] ) / grid.spacing[k];
					outside = !( b >= 0.0f && a <= last ); // NaN: outside
					if ( !outside )
					{
						lo[k] = uint32_t( std::max( 0.0f, std::floor( a ) ) );
						hi[k] = uint32_t( std::min( last, std::ceil( b ) ) );
					}
				}
				if ( outside )
					continue;
				for ( uint32_t z = lo[2]; z <= hi[2]; ++z )
					for ( uint32_t y = lo[1]; y <= hi[1]; ++y )
						for ( uint32_t x = lo[0]; x <= hi[0]; ++x )
							candidates.push_back( x + grid.dims[0] * ( y + grid.dims[1] * z ) );
			}
			std::sort( candidates.begin(), candidates.end() );
			candidates.erase(
			    std::unique( candidates.begin(), candidates.end() ), candidates.end() );
		}
		// Each probe writes its own tiles: the visited probes run in batches on
		// the executor, and the changed ones are listed in grid order.
		const auto visit = [&]( uint32_t i ) -> bool
		{
			const unsigned char *state =
			    texel( grid.stateOrigin[0] + i % stateRow, grid.stateOrigin[1] + i / stateRow );
			if ( read( state, 3 ) < 0.5f )
				return false;
			const uint32_t index[3] = { i % grid.dims[0], ( i / grid.dims[0] ) % grid.dims[1],
			    i / ( grid.dims[0] * grid.dims[1] ) };
			float probe[3];
			for ( int k = 0; k < 3; ++k )
				probe[k] = grid.origin[k] + float( index[k] ) * grid.spacing[k] + read( state, k );
			// Only proxies within the probe's visibility range. A box farther
			// than the range is entered no nearer than its distance, so it cannot
			// shorten a direction: the texel loop tests the near ones alone
			// (with a margin over float rounding), except under scanAll, the
			// oracle, which tests every proxy for every direction.
			constexpr size_t kMaxNear = 64;
			const Proxy *nearProxies[kMaxNear];
			size_t nearCount = 0;
			bool near = false, overflow = false;
			const float keep = grid.maxDistance * 1.01f + 1.0e-3f;
			for ( const Proxy &proxy : proxies )
			{
				float d2 = 0.0f;
				for ( int k = 0; k < 3; ++k )
				{
					const float e =
					    std::max( { proxy.lo[k] - probe[k], 0.0f, probe[k] - proxy.hi[k] } );
					d2 += e * e;
				}
				near = near || d2 < grid.maxDistance * grid.maxDistance;
				if ( d2 < keep * keep )
				{
					if ( nearCount < kMaxNear )
						nearProxies[nearCount++] = &proxy;
					else
						overflow = true;
				}
			}
			if ( !near )
				return false;
			const bool filtered = !scanAll && !overflow;
			const uint32_t x0 = grid.visibilityOrigin[0] + ( i % grid.tilesPerRow ) * kTile;
			const uint32_t y0 = grid.visibilityOrigin[1] + ( i / grid.tilesPerRow ) * kTile;
			bool changed = false;
			for ( uint32_t v = 0; v < kInterior; ++v )
				for ( uint32_t u = 0; u < kInterior; ++u )
				{
					const float *d = directions[v][u];
					float nearest = grid.maxDistance;
					const size_t tested = filtered ? nearCount : proxies.size();
					for ( size_t p = 0; p < tested; ++p )
					{
						const Proxy &proxy = filtered ? *nearProxies[p] : proxies[p];
						float enter = 0.0f, leave = nearest;
						bool hit = true;
						for ( int k = 0; k < 3 && hit; ++k )
						{
							if ( std::fabs( d[k] ) < 1e-8f )
							{
								hit = probe[k] >= proxy.lo[k] && probe[k] <= proxy.hi[k];
								continue;
							}
							float t0 = ( proxy.lo[k] - probe[k] ) / d[k];
							float t1 = ( proxy.hi[k] - probe[k] ) / d[k];
							if ( t0 > t1 )
								std::swap( t0, t1 );
							enter = std::max( enter, t0 );
							leave = std::min( leave, t1 );
							hit = enter <= leave;
						}
						if ( hit )
							nearest = std::min( nearest, enter );
					}
					unsigned char *at = texel( x0 + 1 + u, y0 + 1 + v );
					const float mean = nearest / grid.maxDistance;
					if ( mean < read( at, 0 ) )
					{
						write( at, 0, mean );
						write( at, 1, std::min( read( at, 1 ), mean * mean ) );
						changed = true;
					}
				}
			if ( !changed )
				return false;
			// The octahedral border (probe_volume.py with_border).
			const auto copy = [&]( uint32_t tx, uint32_t ty, uint32_t sx, uint32_t sy )
			{
				std::memcpy( texel( x0 + tx, y0 + ty ), texel( x0 + 1 + sx, y0 + 1 + sy ), 8 );
			};
			const uint32_t n = kInterior;
			for ( uint32_t k = 0; k < n; ++k )
			{
				copy( 1 + k, 0, n - 1 - k, 0 );
				copy( 1 + k, n + 1, n - 1 - k, n - 1 );
				copy( 0, 1 + k, 0, n - 1 - k );
				copy( n + 1, 1 + k, n - 1, n - 1 - k );
			}
			copy( 0, 0, n - 1, n - 1 );
			copy( n + 1, 0, 0, n - 1 );
			copy( 0, n + 1, n - 1, 0 );
			copy( n + 1, n + 1, 0, 0 );
			return true;
		};
		std::vector<uint8_t> changedFlags( candidates.size(), 0 );
		struct Batch
		{
			const decltype( visit ) *apply;
			const std::vector<uint32_t> *candidates;
			std::vector<uint8_t> *changed;
			size_t block;
		} batch{
		    &visit, &candidates, &changedFlags, ChangeComposer::ProbeBlock( candidates.size() ) };
		const uint32_t blocks = uint32_t( ( candidates.size() + batch.block - 1 ) / batch.block );
		const auto run = []( void *raw, uint32_t b )
		{
			const Batch &c = *static_cast<const Batch *>( raw );
			const size_t end = std::min( c.candidates->size(), size_t( b + 1 ) * c.block );
			for ( size_t k = size_t( b ) * c.block; k < end; ++k )
				( *c.changed )[k] = ( *c.apply )( ( *c.candidates )[k] ) ? 1 : 0;
		};
		if ( executor && blocks > 1 )
			executor->ParallelFor( "indirect.occlude-visibility", blocks, run, &batch );
		else
			for ( uint32_t b = 0; b < blocks; ++b )
				run( &batch, b );
		for ( size_t k = 0; k < candidates.size(); ++k )
		{
			if ( !changedFlags[k] )
				continue;
			++changedProbes;
			if ( cut )
				cut->push_back( first + candidates[k] );
		}
	}
	return changedProbes;
}

} // namespace indirect_light

#endif // RENDER_DIRECT_OCCLUSION_H
