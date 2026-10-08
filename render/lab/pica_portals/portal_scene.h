//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: pica_portals: the lab's test chamber, its two portals and the
//			camera path, built the same way on the host and on the 3DS.
//
//			The chamber is a 512 x 256 x 1024 room (x, y up, z) with
//			tessellated walls, floor and ceiling and a field of crates, split
//			into chunks of at most a few hundred triangles so that each chunk
//			is one draw a view can cull. Orange sits on the north wall and blue
//			on the south wall, facing each other, so each portal's view shows
//			the other one (recursion).
//
//=============================================================================//

#ifndef RENDER_LAB_PICA_PORTALS_PORTAL_SCENE_H
#define RENDER_LAB_PICA_PORTALS_PORTAL_SCENE_H

#include "portal_math.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace pica_portals
{

// 24 bytes: position, texture coordinate (port convention: v = 0 is row 0),
// colour (kUnorm8x4).
struct Vertex
{
	float x, y, z;
	float u, v;
	std::uint8_t rgba[4];
};
static_assert( sizeof( Vertex ) == 24 );

struct Chunk
{
	std::uint32_t firstIndex = 0;
	std::uint32_t indexCount = 0;
	std::int32_t vertexOffset = 0; // indices are relative to it (16-bit)
	Box bounds;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices;
	std::vector<Chunk> chunks;
	std::uint32_t triangles = 0;
};

inline constexpr int kEllipseSegments = 24;

// The portal's ellipse (a fan: centre, then the rim's points, closed) and its
// rim ring, in world space slightly in front of the wall. The fan's texture
// coordinates are the portal-local (u, v), v down: what the portal-plane
// technique samples.
struct PortalGeometry
{
	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices;
	std::uint32_t fanFirst = 0, fanCount = 0;    // indices of the interior
	std::uint32_t rimFirst = 0, rimCount = 0;    // indices of the rim ring
	std::array<V3, kEllipseSegments> boundary{}; // the interior's rim, world space
};

struct Scene
{
	Mesh mesh;
	std::array<Portal, 2> portals;
	std::array<PortalGeometry, 2> geometry;
};

namespace detail
{

inline void Colour( Vertex &v, std::uint32_t rgb, float light )
{
	const float l = std::clamp( light, 0.0f, 1.0f );
	v.rgba[0] = std::uint8_t( float( ( rgb >> 16 ) & 255 ) * l );
	v.rgba[1] = std::uint8_t( float( ( rgb >> 8 ) & 255 ) * l );
	v.rgba[2] = std::uint8_t( float( rgb & 255 ) * l );
	v.rgba[3] = 255;
}

// Baked "light": brighter near the ceiling lights and the room's middle.
inline float Light( V3 p )
{
	const float h = 0.55f + 0.45f * ( p.y / 256.0f );
	const float centre = 1.0f - 0.25f * std::min( 1.0f, std::fabs( p.z ) / 512.0f );
	return h * centre;
}

class Builder
{
public:
	explicit Builder( Mesh &mesh ) : m_Mesh( mesh ) {}

	// A grid of quads on the plane origin + s*su + t*sv, s, t in [0, 1],
	// facing su x sv, split into chunks of `chunk` x `chunk` quads.
	void Grid( V3 origin, V3 su, V3 sv, int nu, int nv, int chunk, std::uint32_t rgb, float texels )
	{
		for ( int cu = 0; cu < nu; cu += chunk )
			for ( int cv = 0; cv < nv; cv += chunk )
			{
				Begin();
				const int eu = std::min( nu, cu + chunk ), ev = std::min( nv, cv + chunk );
				const int width = eu - cu + 1;
				for ( int j = cv; j <= ev; ++j )
					for ( int i = cu; i <= eu; ++i )
					{
						const float s = float( i ) / nu, t = float( j ) / nv;
						const V3 p = origin + su * s + sv * t;
						Vertex v{ p.x, p.y, p.z, s * texels * Length( su ) / 64.0f,
						    t * texels * Length( sv ) / 64.0f, {} };
						Colour( v, rgb, Light( p ) );
						Add( v );
					}
				for ( int j = 0; j < ev - cv; ++j )
					for ( int i = 0; i < eu - cu; ++i )
					{
						const std::uint16_t a = std::uint16_t( j * width + i );
						Quad( a, std::uint16_t( a + 1 ), std::uint16_t( a + 1 + width ),
						    std::uint16_t( a + width ) );
					}
				End();
			}
	}

	// An axis-aligned crate, each face split 2 x 2.
	void Crate( V3 lo, V3 hi, std::uint32_t rgb )
	{
		Begin();
		const V3 size = hi - lo;
		const V3 faces[6][3] = { { { lo.x, lo.y, hi.z }, { size.x, 0, 0 }, { 0, size.y, 0 } }, // +z
		    { { hi.x, lo.y, lo.z }, { -size.x, 0, 0 }, { 0, size.y, 0 } },                     // -z
		    { { hi.x, lo.y, hi.z }, { 0, 0, -size.z }, { 0, size.y, 0 } },                     // +x
		    { { lo.x, lo.y, lo.z }, { 0, 0, size.z }, { 0, size.y, 0 } },                      // -x
		    { { lo.x, hi.y, hi.z }, { size.x, 0, 0 }, { 0, 0, -size.z } },                     // +y
		    { { lo.x, lo.y, lo.z }, { size.x, 0, 0 }, { 0, 0, size.z } } };                    // -y
		for ( const auto &f : faces )
		{
			const std::uint32_t base = std::uint32_t( m_Pending.size() );
			for ( int j = 0; j <= 2; ++j )
				for ( int i = 0; i <= 2; ++i )
				{
					const V3 p = f[0] + f[1] * ( i * 0.5f ) + f[2] * ( j * 0.5f );
					Vertex v{ p.x, p.y, p.z, i * 0.5f, 1 - j * 0.5f, {} };
					Colour( v, rgb, Light( p ) * ( f[2].y != 0 ? 0.85f : 1.0f ) );
					Add( v );
				}
			for ( int j = 0; j < 2; ++j )
				for ( int i = 0; i < 2; ++i )
				{
					const std::uint16_t a = std::uint16_t( base + j * 3 + i );
					Quad(
					    a, std::uint16_t( a + 1 ), std::uint16_t( a + 4 ), std::uint16_t( a + 3 ) );
				}
		}
		End();
	}

private:
	void Begin()
	{
		m_Pending.clear();
		m_PendingIndices.clear();
	}
	void Add( const Vertex &v ) { m_Pending.push_back( v ); }
	// Counter-clockwise seen from the front (the port's default front face).
	void Quad( std::uint16_t a, std::uint16_t b, std::uint16_t c, std::uint16_t d )
	{
		for ( std::uint16_t i : { a, b, c, a, c, d } )
			m_PendingIndices.push_back( i );
	}
	void End()
	{
		Chunk chunk;
		chunk.firstIndex = std::uint32_t( m_Mesh.indices.size() );
		chunk.indexCount = std::uint32_t( m_PendingIndices.size() );
		chunk.vertexOffset = std::int32_t( m_Mesh.vertices.size() );
		chunk.bounds = { { 1e9f, 1e9f, 1e9f }, { -1e9f, -1e9f, -1e9f } };
		for ( const Vertex &v : m_Pending )
		{
			chunk.bounds.lo = { std::min( chunk.bounds.lo.x, v.x ),
			    std::min( chunk.bounds.lo.y, v.y ), std::min( chunk.bounds.lo.z, v.z ) };
			chunk.bounds.hi = { std::max( chunk.bounds.hi.x, v.x ),
			    std::max( chunk.bounds.hi.y, v.y ), std::max( chunk.bounds.hi.z, v.z ) };
			m_Mesh.vertices.push_back( v );
		}
		for ( std::uint16_t i : m_PendingIndices )
			m_Mesh.indices.push_back( i );
		m_Mesh.triangles += chunk.indexCount / 3;
		m_Mesh.chunks.push_back( chunk );
	}

	Mesh &m_Mesh;
	std::vector<Vertex> m_Pending;
	std::vector<std::uint16_t> m_PendingIndices;
};

inline PortalGeometry Ellipse( const Portal &p, std::uint32_t rimRgb )
{
	PortalGeometry g;
	const V3 lift = p.N * 0.25f;
	const float ra = p.width * 0.5f, rb = p.height * 0.5f;
	auto local = [&]( float a, float b, float scale )
	{
		return p.Local( a * scale, b * scale ) + lift;
	};
	auto push = [&]( V3 at, float a, float b, std::uint32_t rgb )
	{
		Vertex v{ at.x, at.y, at.z, 0.5f + a / p.width, 0.5f - b / p.height, {} };
		Colour( v, rgb, 1.0f );
		g.vertices.push_back( v );
	};
	// Interior fan: centre, then the rim points.
	push( local( 0, 0, 1 ), 0, 0, 0xFFFFFF );
	for ( int i = 0; i < kEllipseSegments; ++i )
	{
		const float angle = 6.2831853f * float( i ) / kEllipseSegments;
		const float a = ra * std::cos( angle ), b = rb * std::sin( angle );
		g.boundary[std::size_t( i )] = local( a, b, 1 );
		push( g.boundary[std::size_t( i )], a, b, 0xFFFFFF );
	}
	g.fanFirst = 0;
	for ( int i = 0; i < kEllipseSegments; ++i )
		for ( int k : { 0, 1 + i, 1 + ( i + 1 ) % kEllipseSegments } )
			g.indices.push_back( std::uint16_t( k ) );
	g.fanCount = std::uint32_t( g.indices.size() );
	// Rim ring between scale 1 and 1.15, a little further out of the wall.
	const std::uint16_t ring = std::uint16_t( g.vertices.size() );
	for ( int i = 0; i < kEllipseSegments; ++i )
	{
		const float angle = 6.2831853f * float( i ) / kEllipseSegments;
		const float a = ra * std::cos( angle ), b = rb * std::sin( angle );
		push( local( a, b, 1.0f ) + p.N * 0.05f, a, b, rimRgb );
		push( local( a, b, 1.15f ) + p.N * 0.05f, a, b, rimRgb );
	}
	g.rimFirst = std::uint32_t( g.indices.size() );
	for ( int i = 0; i < kEllipseSegments; ++i )
	{
		const int j = ( i + 1 ) % kEllipseSegments;
		const std::uint16_t a = std::uint16_t( ring + 2 * i ),
		                    b = std::uint16_t( ring + 2 * i + 1 );
		const std::uint16_t c = std::uint16_t( ring + 2 * j ),
		                    d = std::uint16_t( ring + 2 * j + 1 );
		for ( std::uint16_t k : { a, c, d, a, d, b } )
			g.indices.push_back( k );
	}
	g.rimCount = std::uint32_t( g.indices.size() ) - g.rimFirst;
	return g;
}

} // namespace detail

inline constexpr std::uint32_t kOrange = 0xFF8A10, kBlue = 0x2A8CFF;

inline Scene BuildScene()
{
	Scene scene;
	detail::Builder b( scene.mesh );
	const float X = 256, Y = 256, Z = 512;
	// Floor (faces +y), ceiling (faces -y), walls facing inwards; 16-unit quads.
	b.Grid( { -X, 0, Z }, { 2 * X, 0, 0 }, { 0, 0, -2 * Z }, 32, 64, 8, 0xB8B8B0, 16 );
	b.Grid( { -X, Y, -Z }, { 2 * X, 0, 0 }, { 0, 0, 2 * Z }, 32, 64, 8, 0x707078, 16 );
	b.Grid( { -X, 0, -Z }, { 2 * X, 0, 0 }, { 0, Y, 0 }, 32, 16, 8, 0xE8E8E0, 8 ); // north
	b.Grid( { X, 0, Z }, { -2 * X, 0, 0 }, { 0, Y, 0 }, 32, 16, 8, 0xE0E0E8, 8 );  // south
	b.Grid( { X, 0, -Z }, { 0, 0, 2 * Z }, { 0, Y, 0 }, 64, 16, 8, 0xD8E0D8, 8 );  // east
	b.Grid( { -X, 0, Z }, { 0, 0, -2 * Z }, { 0, Y, 0 }, 64, 16, 8, 0xE0D8D8, 8 ); // west
	// A field of crates and four pillars.
	const std::uint32_t palette[] = { 0xC04040, 0x40A040, 0x4060C0, 0xC0A040, 0x9050B0, 0x40B0B0 };
	int n = 0;
	for ( int gz = -3; gz <= 3; ++gz )
		for ( int gx = -2; gx <= 2; ++gx )
		{
			if ( ( gx + gz ) % 2 != 0 )
				continue;
			const float cx = gx * 96.0f + float( ( gz * 37 ) % 23 ), cz = gz * 128.0f + 40.0f;
			const float s = 20.0f + float( ( n * 13 ) % 3 ) * 8.0f;
			const float hgt = 24.0f + float( ( n * 7 ) % 4 ) * 20.0f;
			b.Crate( { cx - s, 0, cz - s }, { cx + s, hgt, cz + s }, palette[n % 6] );
			++n;
		}
	for ( float px : { -160.0f, 160.0f } )
		for ( float pz : { -256.0f, 256.0f } )
			b.Crate( { px - 16, 0, pz - 16 }, { px + 16, Y, pz + 16 }, 0xA0A0A8 );

	scene.portals[0] = { { -96, 72, -Z }, { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, 64, 112 };
	scene.portals[1] = { { 128, 72, Z }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, 64, 112 };
	scene.geometry[0] = detail::Ellipse( scene.portals[0], kOrange );
	scene.geometry[1] = detail::Ellipse( scene.portals[1], kBlue );
	return scene;
}

// -- The camera path --------------------------------------------------------

struct Camera
{
	V3 eye;
	V3 forward;
};

struct Segment
{
	std::string_view name;
	int frames;
};

inline constexpr Segment kSegments[] = {
    { "approach", 1 }, // scaled by the frame count per segment
    { "turn", 1 },
    { "strafe", 1 },
    { "close", 1 },
    { "blue", 1 },
};
inline constexpr int kSegmentCount = int( std::size( kSegments ) );

inline V3 Lerp( V3 a, V3 b, float t )
{
	return a + ( b - a ) * t;
}

inline V3 YawPitch( float yawDegrees, float pitchDegrees )
{
	const float y = yawDegrees * 0.01745329f, p = pitchDegrees * 0.01745329f;
	return { -std::sin( y ) * std::cos( p ), std::sin( p ), -std::cos( y ) * std::cos( p ) };
}

// Frame `f` of segment `s` (t in [0, 1)): what each segment exercises.
//   approach: walking at the orange portal from mid-room;
//   turn: standing still and looking around with the portal in view (eye
//         fixed: a portal-plane texture is still valid);
//   strafe: sliding past the portal at an angle;
//   close: walking up to within 6 units of the portal (it fills the screen);
//   blue: the other portal from the room's middle.
inline Camera PathCamera( const Scene &scene, int segment, float t )
{
	const V3 orange = scene.portals[0].c, blue = scene.portals[1].c;
	switch ( segment )
	{
	case 0:
		return { Lerp( { 40, 64, 300 }, { -80, 64, -220 }, t ), YawPitch( 4 - 8 * t, 0 ) };
	case 1:
	{
		const float a = std::sin( t * 6.2831853f );
		return { { -80, 64, -220 }, YawPitch( 18 * a, 6 * std::cos( t * 6.2831853f ) ) };
	}
	case 2:
	{
		const V3 eye = Lerp( { -230, 64, -380 }, { 70, 64, -380 }, t );
		return { eye, orange - eye };
	}
	case 3:
	{
		const V3 eye = Lerp( { -96, 66, -400 }, { -96, 66, -506 }, t );
		return { eye, orange - V3{ 0, 2, 0 } - eye };
	}
	default:
	{
		const V3 eye = Lerp( { 60, 64, 120 }, { 110, 64, 360 }, t );
		return { eye, blue - eye };
	}
	}
}

} // namespace pica_portals

#endif // RENDER_LAB_PICA_PORTALS_PORTAL_SCENE_H
