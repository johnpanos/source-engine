//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: pica_portals: the host oracle of the lab's portal geometry
//			(portal_math.h, portal_scene.h). Every technique the 3DS binary
//			compares rests on these identities; a seeded defect
//			(PORTAL_MATH_SENSITIVITY=1..6) must make this program fail.
//
//			  g++ -std=c++20 -I render/lab/pica_portals
//			      render/lab/pica_portals/portal_math_test.cpp
//
//=============================================================================//

#include "portal_math.h"
#include "portal_scene.h"

#include <cstdio>
#include <random>

#ifndef PORTAL_MATH_SENSITIVITY
#define PORTAL_MATH_SENSITIVITY 0
#endif

using namespace pica_portals;

namespace
{

int g_checks = 0, g_failures = 0;

void Check( bool ok, const char *what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "FAIL pica_portals.math: %s\n", what );
	}
}

bool Near( float a, float b, float tolerance )
{
	return std::fabs( a - b ) <= tolerance;
}

V3 Ndc( const M4 &clip, V3 p, float *depth = nullptr )
{
	const V4 c = clip * Point( p );
	if ( depth )
		*depth = c.z / c.w;
	return { c.x / c.w, c.y / c.w, c.w };
}

// The seeded defects: each breaks one identity a technique depends on.
M4 PlaneClip( V3 eye, const Portal &entry, const Portal &exit )
{
	M4 clip = PortalPlaneClip( eye, entry, exit, 0.5f, 2048 );
	if ( PORTAL_MATH_SENSITIVITY == 1 ) // the texture mirrored left to right
		for ( int j = 0; j < 4; ++j )
			clip.m[0][j] = -clip.m[0][j];
	return clip;
}

M4 Remote( const M4 &projection, const M4 &view, const Portal &entry, const Portal &exit )
{
	if ( PORTAL_MATH_SENSITIVITY == 2 ) // no oblique near plane
		return projection * view * Inverse( Transfer( entry, exit ) );
	return RemoteClip( projection, view, entry, exit, 0.5f );
}

M4 TransferOf( const Portal &a, const Portal &b )
{
	if ( PORTAL_MATH_SENSITIVITY == 3 ) // the exit's frame without the half turn
		return b.Frame() * a.InverseFrame();
	return Transfer( a, b );
}

M4 CropOf( float x0, float x1, float y0, float y1 )
{
	if ( PORTAL_MATH_SENSITIVITY == 4 ) // the crop's y bounds swapped
		return Crop( x0, x1, y1, y0 );
	return Crop( x0, x1, y0, y1 );
}

VisibleRect VisibleOf( const M4 &clip, const Portal &portal )
{
	VisibleRect r = VisiblePortalRect( clip, portal, 400, 240 );
	if ( PORTAL_MATH_SENSITIVITY == 5 ) // the rectangle cut short on the right
		r.u1 = r.u0 + ( r.u1 - r.u0 ) * 0.9f;
	return r;
}

std::array<Plane, 9> ConeOf( V3 eye, const Portal &entry, const Portal &exit )
{
	if ( PORTAL_MATH_SENSITIVITY == 6 ) // the octagon inscribed, not circumscribed
	{
		Portal small = entry;
		small.width *= 0.9238795f;
		small.height *= 0.9238795f;
		return PortalCone( eye, small, exit );
	}
	return PortalCone( eye, entry, exit );
}

} // namespace

int main()
{
	const Scene scene = BuildScene();
	const Portal &orange = scene.portals[0], &blue = scene.portals[1];

	// The transfer: centres to centres, the entry's front to the exit's back,
	// right to left, up to up, and back again.
	{
		const M4 t = TransferOf( orange, blue );
		const V3 c = TransformPoint( t, orange.c );
		Check( Length( c - blue.c ) < 1e-3f, "the transfer maps the entry's centre to the exit's" );
		const V3 front = TransformPoint( t, orange.c + orange.N * 10 );
		Check( Near( blue.Facing( front ), -10, 1e-3f ),
		    "a point 10 in front of the entry is 10 behind the exit" );
		const V3 right = TransformPoint( t, orange.c + orange.R * 10 ) - blue.c;
		Check( Near( Dot( right, blue.R ), -10, 1e-3f ), "the entry's right is the exit's left" );
		const V3 up = TransformPoint( t, orange.c + orange.U * 10 ) - blue.c;
		Check( Near( Dot( up, blue.U ), 10, 1e-3f ), "up stays up" );
		const M4 round = Transfer( blue, orange ) * Transfer( orange, blue );
		bool identity = true;
		for ( int i = 0; i < 4; ++i )
			for ( int j = 0; j < 4; ++j )
				identity = identity && Near( round.m[i][j], i == j ? 1.0f : 0.0f, 1e-4f );
		Check( identity, "through one portal and back is the identity" );
	}

	const float aspect = 400.0f / 240.0f;
	const M4 projection = Perspective( 55 * 0.01745329f, aspect, 2, 2048 );
	std::mt19937 random( 7 );
	std::uniform_real_distribution<float> unit( 0, 1 );

	// The portal-plane projection: the entry's corners land on the texture's
	// corners, at any eye in front of the portal.
	{
		bool corners = true, plane = true;
		for ( int k = 0; k < 64; ++k )
		{
			const V3 eye{ -200 + 400 * unit( random ), 10 + 200 * unit( random ),
			    -500 + 800 * unit( random ) };
			const M4 clip = PlaneClip( eye, orange, blue );
			const M4 t = TransferOf( orange, blue );
			const V3 ll = Ndc( clip, TransformPoint( t, orange.LowerLeft() ) );
			const V3 ur = Ndc( clip, TransformPoint( t, orange.UpperRight() ) );
			corners = corners && Near( ll.x, -1, 1e-3f ) && Near( ll.y, -1, 1e-3f ) &&
			          Near( ur.x, 1, 1e-3f ) && Near( ur.y, 1, 1e-3f );
			// The exit's own wall (its plane) lies before the near plane.
			float depth = 0;
			(void)Ndc( clip, blue.c + blue.R * 7 + blue.U * 3, &depth );
			plane = plane && depth < 0;
		}
		Check( corners, "portal plane: the entry's corners are the texture's corners" );
		Check( plane, "portal plane: the exit wall is in front of the near plane (clipped)" );
	}

	// The three techniques agree: for an exit-room point X seen through the
	// entry, (1) the stencil technique draws X at screen point s; (2) the
	// portal-plane texture shows X at the uv of the entry point on the ray
	// eye -> s; (3) the cropped projection puts X at the crop of s.
	{
		bool screen = true, texture = true, crop = true, depth = true;
		int tested = 0;
		for ( int k = 0; k < 4000 && tested < 400; ++k )
		{
			const V3 eye{ -200 + 400 * unit( random ), 20 + 150 * unit( random ),
			    -480 + 600 * unit( random ) };
			const V3 at = orange.c - eye;
			const M4 view = LookAt( eye, at, { 0, 1, 0 } );
			// A point in the room in front of the blue portal.
			const V3 x = blue.c + blue.R * ( -200 + 400 * unit( random ) ) +
			             blue.U * ( -70 + 180 * unit( random ) ) +
			             blue.N * ( 1 + 900 * unit( random ) );
			// The ray from the eye towards x's image behind the entry must cross
			// the entry's rectangle.
			const V3 behind = TransformPoint( Inverse( TransferOf( orange, blue ) ), x );
			const V3 dir = behind - eye;
			const float s = orange.Facing( eye ) / -Dot( dir, orange.N );
			if ( s <= 0 || s >= 1 )
				continue;
			const V3 hit = eye + dir * s;
			const float a = Dot( hit - orange.c, orange.R ), b = Dot( hit - orange.c, orange.U );
			if ( std::fabs( a ) > orange.width / 2 || std::fabs( b ) > orange.height / 2 )
				continue;
			const M4 main = projection * view;
			const V3 sHit = Ndc( main, hit );
			// Only what is on screen: the oblique projection keeps the
			// original frustum's sides, nothing outside them.
			if ( std::fabs( sHit.x ) > 1 || std::fabs( sHit.y ) > 1 || sHit.z <= 0 )
				continue;
			++tested;
			float dx = 0;
			const V3 sStencil = Ndc( Remote( projection, view, orange, blue ), x, &dx );
			screen =
			    screen && Near( sStencil.x, sHit.x, 2e-3f ) && Near( sStencil.y, sHit.y, 2e-3f );
			depth = depth && dx > 0 && dx < 1;
			const V3 uv = Ndc( PlaneClip( eye, orange, blue ), x );
			texture = texture && Near( uv.x, 2 * ( a / orange.width ), 2e-3f ) &&
			          Near( uv.y, 2 * ( b / orange.height ), 2e-3f );
			const float x0 = sHit.x - 0.2f, x1 = sHit.x + 0.3f, y0 = sHit.y - 0.1f,
			            y1 = sHit.y + 0.4f;
			const V3 c =
			    Ndc( CropOf( x0, x1, y0, y1 ) * Remote( projection, view, orange, blue ), x );
			crop = crop && Near( c.x, ( 2 * sHit.x - x0 - x1 ) / ( x1 - x0 ), 3e-3f ) &&
			       Near( c.y, ( 2 * sHit.y - y0 - y1 ) / ( y1 - y0 ), 3e-3f );
		}
		Check( tested >= 200, "enough exit-room points are seen through the portal" );
		Check( screen, "stencil: an exit-room point is drawn where its ray crosses the entry" );
		Check( depth, "stencil: the oblique projection keeps the exit room inside depth 0..1" );
		Check( texture, "portal plane: the texture holds the point at the crossing's uv" );
		Check( crop, "crop: the cropped projection is the screen position stretched" );
	}

	// The oblique near plane clips what lies behind the exit (between the eye's
	// image and the exit wall) and the exit's coplanar wall.
	{
		const V3 eye{ -60, 64, -300 };
		const M4 view = LookAt( eye, orange.c - eye, { 0, 1, 0 } );
		const M4 remote = Remote( projection, view, orange, blue );
		float d = 0;
		(void)Ndc( remote, blue.c + blue.N * -20, &d );
		Check( d < 0, "stencil: a point behind the exit is in front of the near plane" );
		(void)Ndc( remote, blue.c + blue.R * 5, &d );
		Check( d < 0, "stencil: the exit's wall is clipped" );
		(void)Ndc( remote, blue.c + blue.N * 5, &d );
		Check( d > 0, "stencil: a point 5 into the exit room is kept" );
	}

	// The visible part of a portal: every on-screen point of the portal lies
	// in the rectangle, and the cropped portal-plane projection puts it where
	// the rectangle's texture expects it.
	{
		bool contains = true, cropped = true, partial = false;
		int tested = 0;
		for ( int k = 0; k < 300; ++k )
		{
			const V3 eye{ -150 + 300 * unit( random ), 30 + 80 * unit( random ),
			    -505 + 300 * unit( random ) };
			const V3 at = orange.c + orange.R * ( -60 + 120 * unit( random ) ) +
			              orange.U * ( -60 + 120 * unit( random ) );
			const M4 main = projection * LookAt( eye, at - eye, { 0, 1, 0 } );
			const VisibleRect r = VisibleOf( main, orange );
			if ( !r.visible )
				continue;
			partial = partial || r.u1 - r.u0 < 0.9f || r.v1 - r.v0 < 0.9f;
			const M4 crop =
			    CropToPortalRect( PlaneClip( eye, orange, blue ), r.u0, r.u1, r.v0, r.v1 );
			const M4 t = TransferOf( orange, blue );
			for ( int s = 0; s < 64; ++s )
			{
				const float u = unit( random ), v = unit( random );
				const V3 p =
				    orange.Local( ( u - 0.5f ) * orange.width, ( 0.5f - v ) * orange.height );
				const V4 c = main * Point( p );
				if ( c.z <= 0 || std::fabs( c.x ) > c.w || std::fabs( c.y ) > c.w )
					continue;
				++tested;
				contains = contains && u >= r.u0 - 1e-3f && u <= r.u1 + 1e-3f &&
				           v >= r.v0 - 1e-3f && v <= r.v1 + 1e-3f;
				const V3 n = Ndc( crop, TransformPoint( t, p ) );
				const float tu = ( n.x + 1 ) * 0.5f, tv = ( 1 - n.y ) * 0.5f;
				cropped = cropped && Near( tu, ( u - r.u0 ) / ( r.u1 - r.u0 ), 2e-3f ) &&
				          Near( tv, ( v - r.v0 ) / ( r.v1 - r.v0 ), 2e-3f );
			}
		}
		Check(
		    tested > 1000 && partial, "visible rect: tested on views that show part of a portal" );
		Check( contains, "visible rect: every on-screen point of the portal is inside it" );
		Check( cropped, "visible rect: the cropped plane texture maps the rectangle to 0..1" );
	}

	// The portal cone never culls what the portal shows: every exit-room point
	// seen through the entry (sampled as above) is inside all nine planes.
	{
		bool kept = true, culls = false;
		int tested = 0;
		for ( int k = 0; k < 20000 && tested < 2000; ++k )
		{
			const V3 eye{ -200 + 400 * unit( random ), 20 + 150 * unit( random ),
			    -480 + 600 * unit( random ) };
			const V3 x = blue.c + blue.R * ( -250 + 500 * unit( random ) ) +
			             blue.U * ( -70 + 180 * unit( random ) ) +
			             blue.N * ( 1 + 900 * unit( random ) );
			const auto cone = ConeOf( eye, orange, blue );
			const V3 behind = TransformPoint( Inverse( TransferOf( orange, blue ) ), x );
			const V3 dir = behind - eye;
			const float s = orange.Facing( eye ) / -Dot( dir, orange.N );
			bool inside = true;
			for ( const Plane &p : cone )
				inside = inside && p.Distance( x ) >= -1e-2f;
			if ( s <= 0 || s >= 1 )
				continue;
			const V3 hit = eye + dir * s;
			const float a = Dot( hit - orange.c, orange.R ) / ( orange.width / 2 ),
			            b = Dot( hit - orange.c, orange.U ) / ( orange.height / 2 );
			if ( a * a + b * b > 1 )
			{
				culls = culls || !inside;
				continue;
			}
			++tested;
			kept = kept && inside;
		}
		Check( tested >= 1000, "cone: enough points seen through the ellipse" );
		Check( kept, "cone: no point seen through the ellipse is culled" );
		Check( culls, "cone: points outside the ellipse's view are culled" );
	}

	// Frustum culling: planes of a clip matrix keep what projects inside it.
	{
		const V3 eye{ 0, 64, 300 };
		const M4 clip = projection * LookAt( eye, { 0, 0, -1 }, { 0, 1, 0 } );
		const auto planes = FrustumPlanes( clip );
		Check(
		    Intersects( planes, { { -10, 50, 0 }, { 10, 70, 20 } } ), "cull: a box ahead is kept" );
		Check( !Intersects( planes, { { -10, 50, 320 }, { 10, 70, 340 } } ),
		    "cull: a box behind the eye is culled" );
		Check( !Intersects( planes, { { 900, 50, 0 }, { 920, 70, 20 } } ),
		    "cull: a box far to the right is culled" );
	}

	// The scene: chunks are small enough for 16-bit indices, and both
	// portals sit on their walls facing into the room.
	{
		bool fits = true;
		for ( std::size_t i = 0; i < scene.mesh.chunks.size(); ++i )
		{
			const Chunk &c = scene.mesh.chunks[i];
			const std::size_t next = i + 1 < scene.mesh.chunks.size()
			                             ? std::size_t( scene.mesh.chunks[i + 1].vertexOffset )
			                             : scene.mesh.vertices.size();
			fits = fits && next - std::size_t( c.vertexOffset ) <= 65536;
		}
		Check( fits, "scene: every chunk addresses at most 65536 vertices" );
		std::printf( "INFO scene %zu vertices, %u triangles, %zu chunks\n",
		    scene.mesh.vertices.size(), scene.mesh.triangles, scene.mesh.chunks.size() );
		Check( Near( std::fabs( orange.c.z ), 512, 1e-3f ) &&
		           Near( std::fabs( blue.c.z ), 512, 1e-3f ),
		    "scene: the portals are on the end walls" );
		Check( orange.Facing( { 0, 64, 0 } ) > 0 && blue.Facing( { 0, 64, 0 } ) > 0,
		    "scene: both portals face the room" );
	}

	std::printf( "CONFORMANCE %d %d\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
