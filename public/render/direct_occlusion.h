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
//          distant lights by direction). Compose (when the occluders
//          change): per texel, the share of its unoccluded analytic light
//          whose path a proxy blocks, times its direct layer, subtracted
//          from its total. A texel no path of which a proxy blocks keeps its
//          baked bytes exactly. Static occlusion is already in the bake and
//          is not re-traced, so the share is of the light the texel would
//          receive without static occluders.
//
//===========================================================================//

#ifndef RENDER_DIRECT_OCCLUSION_H
#define RENDER_DIRECT_OCCLUSION_H

#include "mapcontainer/sdf_volume.h"
#include "mapcontainer/world_lightmap.h"
#include "mapcontainer/world_mesh.h"
#include "mapcontainer/world_mesh_format.h"
#include "render/indirect_light.h"

#include <algorithm>
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

	// False (and nothing built) without a direct layer, lights or triangles.
	[[nodiscard]] bool Build( const void *wmsh, size_t wmshSize, const void *lmap, size_t lmapSize,
	    uint32_t lmapVersion, std::span<const mapcontainer::SdfLight> lights )
	{
		*this = DirectOcclusion();
		mapcontainer::WorldMeshSummary mesh = {};
		mapcontainer::WorldLightmapLayout layout = {};
		if ( mapcontainer::ValidateWorldMesh( wmsh, wmshSize, &mesh ) !=
		         mapcontainer::WorldMeshError::Ok ||
		     mapcontainer::ValidateWorldLightmap( lmap, lmapSize, lmapVersion, &layout ) !=
		         mapcontainer::WorldLightmapError::Ok )
			return false;
		const int total = mapcontainer::WorldLightmapLayerIndex(
		    layout, mapcontainer::WorldLightmapLayer::Total );
		const int direct = mapcontainer::WorldLightmapLayerIndex(
		    layout, mapcontainer::WorldLightmapLayer::Direct );
		if ( total < 0 || direct < 0 )
			return false;
		for ( const mapcontainer::SdfLight &light : lights )
			AddLight( light );
		if ( m_samples.empty() )
			return false;
		m_width = layout.width;
		m_height = layout.height;
		m_layout = layout;
		m_totalIndex = total;
		m_directIndex = direct;
		const unsigned char *bytes = static_cast<const unsigned char *>( lmap );
		m_lmap.assign( bytes, bytes + lmapSize );
		// A directional page (2:1) keeps its flat light in the left half.
		m_flatWidth = m_width == 2 * m_height ? m_height : m_width;
		Rasterize( static_cast<const unsigned char *>( wmsh ), mesh );
		return !m_texels.empty();
	}

	[[nodiscard]] bool Ready() const { return !m_texels.empty(); }
	[[nodiscard]] const mapcontainer::WorldLightmapLayout &Layout() const { return m_layout; }
	// The LMAP bytes (layer offsets per Layout()).
	[[nodiscard]] const unsigned char *Bytes() const { return m_lmap.data(); }
	[[nodiscard]] size_t CoveredTexels() const { return m_texels.size(); }

	// The total layer for these occluders (RGBA16F, rows top first), in
	// `out`; the number of texels whose light a proxy blocks.
	size_t Compose( std::span<const Proxy> proxies, IBatchExecutor *executor,
	    std::vector<unsigned char> *out ) const
	{
		const unsigned char *total = m_lmap.data() + m_layout.layerOffset[m_totalIndex];
		out->assign( total, total + m_layout.layerBytes );
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

private:
	static constexpr uint32_t kBlock = 4096;

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
		float power; // luminance: radiance x patch area, or distant irradiance
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
	void Rasterize( const unsigned char *wmsh, const mapcontainer::WorldMeshSummary &mesh )
	{
		std::vector<int32_t> slot( size_t( m_width ) * m_height, -1 );
		const unsigned char *vertices = wmsh + mesh.sectionOffsets[0];
		const unsigned char *indices = wmsh + mesh.sectionOffsets[1];
		for ( uint32_t t = 0; t + 2 < mesh.indexCount; t += 3 )
		{
			float p[3][3], uv[3][2];
			for ( int c = 0; c < 3; ++c )
			{
				uint32_t index;
				std::memcpy( &index, indices + size_t( t + c ) * 4, 4 );
				if ( index >= mesh.vertexCount )
					return;
				const unsigned char *v = vertices + size_t( index ) * mapcontainer::kWorldMeshVertexSize;
				std::memcpy( p[c], v, 12 );
				std::memcpy( uv[c], v + 32, 8 );
				uv[c][0] *= float( m_flatWidth );
				uv[c][1] *= float( m_height );
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
			const int x0 = std::max( 0, int( std::floor( std::min( { uv[0][0], uv[1][0], uv[2][0] } ) ) ) );
			const int x1 = std::min( int( m_flatWidth ) - 1,
			    int( std::ceil( std::max( { uv[0][0], uv[1][0], uv[2][0] } ) ) ) );
			const int y0 = std::max( 0, int( std::floor( std::min( { uv[0][1], uv[1][1], uv[2][1] } ) ) ) );
			const int y1 = std::min(
			    int( m_height ) - 1, int( std::ceil( std::max( { uv[0][1], uv[1][1], uv[2][1] } ) ) ) );
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
	}

	// Whether the segment from `a` along `d` (t in (0, 1)) passes through the box.
	static bool Blocks( const float a[3], const float d[3], const Proxy &box )
	{
		float enter = 1e-4f, leave = 1.0f - 1e-4f;
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

	static void Block( void *opaque, uint32_t block )
	{
		Context &context = *static_cast<Context *>( opaque );
		const DirectOcclusion &self = *context.self;
		const unsigned char *direct = self.m_lmap.data() + self.m_layout.layerOffset[self.m_directIndex];
		const size_t begin = size_t( block ) * kBlock;
		const size_t end = std::min( begin + kBlock, self.m_texels.size() );
		constexpr float kFar = 1.0e6f; // a distant light's segment length
		uint32_t changed = 0;
		for ( size_t i = begin; i < end; ++i )
		{
			const Texel &texel = self.m_texels[i];
			// A little off the surface, so the texel's own face never blocks.
			float origin[3];
			for ( int k = 0; k < 3; ++k )
				origin[k] = texel.position[k] + texel.normal[k] * 0.5f;
			double all = 0.0, blocked = 0.0;
			for ( const Sample &sample : self.m_samples )
			{
				float d[3], weight;
				if ( sample.distant )
				{
					for ( int k = 0; k < 3; ++k )
						d[k] = sample.point[k] * kFar;
					weight = sample.power * std::max( 0.0f, texel.normal[0] * sample.point[0] +
					                                            texel.normal[1] * sample.point[1] +
					                                            texel.normal[2] * sample.point[2] );
				}
				else
				{
					for ( int k = 0; k < 3; ++k )
						d[k] = sample.point[k] - origin[k];
					const float d2 = std::max( d[0] * d[0] + d[1] * d[1] + d[2] * d[2], 1.0f );
					const float inv = 1.0f / std::sqrt( d2 );
					const float cosSurface =
					    ( texel.normal[0] * d[0] + texel.normal[1] * d[1] + texel.normal[2] * d[2] ) * inv;
					const float cosLight =
					    -( sample.normal[0] * d[0] + sample.normal[1] * d[1] + sample.normal[2] * d[2] ) * inv;
					weight = sample.power * std::max( 0.0f, cosSurface ) * std::max( 0.0f, cosLight ) / d2;
				}
				if ( !( weight > 0.0f ) )
					continue;
				all += weight;
				for ( const Proxy &proxy : context.proxies )
					if ( Blocks( origin, d, proxy ) )
					{
						blocked += weight;
						break;
					}
			}
			if ( !( blocked > 0.0 ) || !( all > 0.0 ) )
				continue;
			const float share = float( blocked / all );
			unsigned char *out = context.total + size_t( texel.index ) * 8;
			const unsigned char *light = direct + size_t( texel.index ) * 8;
			for ( int c = 0; c < 3; ++c )
			{
				const float value = std::max( 0.0f, Half( out + 2 * c ) - share * Half( light + 2 * c ) );
				const uint16_t half = FloatToHalf( value );
				std::memcpy( out + 2 * c, &half, 2 );
			}
			++changed;
		}
		context.changed[block] = changed;
	}

	std::vector<Texel> m_texels;
	std::vector<Sample> m_samples;
	std::vector<unsigned char> m_lmap;
	mapcontainer::WorldLightmapLayout m_layout = {};
	int m_totalIndex = -1;
	int m_directIndex = -1;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_flatWidth = 0;
};

} // namespace indirect_light

#endif // RENDER_DIRECT_OCCLUSION_H
