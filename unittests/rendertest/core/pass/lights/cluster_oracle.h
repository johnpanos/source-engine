//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent geometry checks for render.lights.v1 (RFC 0016 K7).
//			This verifies GPU output; it does not assign light lists.
//
//			It shares no code with render.pass.lights. It rebuilds each
//			froxel in double precision from the view's own matrices (tile
//			corners by solving the projection, slice depths from the
//			logarithmic definition) and decides whether a light reaches it:
//
//			- a point light: the exact distance from its center to the froxel
//			  (a convex hexahedron: 0 inside, else the nearest face or edge) is
//			  at most its radius;
//			- a spot light: the same distance test, then an exact rejection of
//			  the cone (a separating plane through the apex for cones up to a
//			  hemisphere; all corners outside for wider ones), then a witness
//			  search: a point of the froxel inside the spot volume, or a ray of
//			  the spot (its volume is the union of rays of its radius inside the
//			  cone) that enters the froxel. Rays are aimed at the froxel's
//			  corners, face centers, center and nearest point, clamped into the
//			  cone, plus a lattice of 145 cone directions. A pair is counted as
//			  reached only with a witness, so every reported false negative is
//			  real; pairs left without a witness are counted apart.
//
//=============================================================================//

#ifndef RENDER_LIGHTS_CLUSTER_ORACLE_H
#define RENDER_LIGHTS_CLUSTER_ORACLE_H

#include "render/pass/lights/clusters.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <random>
#include <span>
#include <string>
#include <vector>

namespace cluster_oracle
{

using render::math::float3;
using render::math::float4x4;
using render::pass::lights::ClusterGrid;
using render::pass::lights::ClusterLimits;
using render::pass::lights::ClusterLists;
using render::pass::lights::ClusterStats;
using render::pass::lights::ClusterViewDesc;
using light_set::LightShape;
using light_set::RuntimeLight;

// Recorded ceilings on the false-positive rates (the share of listed pairs
// the reference finds no reach for). Measured on the default seed at
// introduction (2026-09-28): points 0.0047, spots 0.168 (0.162 to 0.179
// over seeds 1-5); the ceilings leave room for other seeds and fail a
// builder that lists far too much.
constexpr double kPointFalsePositiveCeiling = 0.02;
constexpr double kSpotFalsePositiveCeiling = 0.25;

constexpr double kInf = std::numeric_limits<double>::infinity();
constexpr double kPi = 3.14159265358979323846;

struct D3
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};

inline D3 operator+( const D3 &a, const D3 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
inline D3 operator-( const D3 &a, const D3 &b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
inline D3 operator*( const D3 &a, double s )
{
	return { a.x * s, a.y * s, a.z * s };
}
inline double Dot( const D3 &a, const D3 &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline D3 Cross( const D3 &a, const D3 &b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
inline double Len( const D3 &a )
{
	return std::sqrt( Dot( a, a ) );
}
inline D3 Unit( const D3 &a )
{
	const double l = Len( a );
	return l > 0.0 ? a * ( 1.0 / l ) : a;
}

// ---------------------------------------------------------------------------
// Reference grid

struct RefGrid
{
	std::uint32_t tilesX = 0;
	std::uint32_t tilesY = 0;
	std::uint32_t slices = 0;
	std::uint32_t tile = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	double nearZ = 0.0;
	double farZ = 0.0;
	std::vector<double> depths;
	double p[4][4] = {};
	double view[4][4] = {};
};

inline RefGrid MakeRefGrid( const ClusterViewDesc &desc, const ClusterLimits &limits )
{
	RefGrid g;
	g.tile = limits.tileSizePixels;
	g.width = desc.widthPixels;
	g.height = desc.heightPixels;
	g.tilesX = ( desc.widthPixels + g.tile - 1 ) / g.tile;
	g.tilesY = ( desc.heightPixels + g.tile - 1 ) / g.tile;
	g.slices = limits.depthSlices;
	g.nearZ = desc.nearZ;
	g.farZ = desc.farZ;
	for ( std::uint32_t k = 0; k <= g.slices; ++k )
		g.depths.push_back( g.nearZ * std::pow( g.farZ / g.nearZ, double( k ) / g.slices ) );
	for ( int r = 0; r < 4; ++r )
	{
		const float *row = &desc.projection.rows[r].x;
		const float *v = &desc.view.rows[r].x;
		for ( int c = 0; c < 4; ++c )
		{
			g.p[r][c] = row[c];
			g.view[r][c] = v[c];
		}
	}
	return g;
}

// The view-space point at view distance `depth` under a pixel position
// (top-left origin, y down): solve clip x / w = ndcX and clip y / w = ndcY
// with w = depth and z = -depth.
inline D3 PixelPoint( const RefGrid &g, double px, double py, double depth )
{
	const double ndcX = 2.0 * px / g.width - 1.0;
	const double ndcY = 1.0 - 2.0 * py / g.height;
	const double z = -depth;
	// p[0][0] x + p[0][1] y = ndcX * depth - p[0][2] z - p[0][3]
	const double bx = ndcX * depth - g.p[0][2] * z - g.p[0][3];
	const double by = ndcY * depth - g.p[1][2] * z - g.p[1][3];
	const double det = g.p[0][0] * g.p[1][1] - g.p[0][1] * g.p[1][0];
	return {
	    ( bx * g.p[1][1] - g.p[0][1] * by ) / det, ( g.p[0][0] * by - g.p[1][0] * bx ) / det, z };
}

inline D3 ToView( const RefGrid &g, const float *world )
{
	const double w[4] = { world[0], world[1], world[2], 1.0 };
	D3 v;
	v.x = g.view[0][0] * w[0] + g.view[0][1] * w[1] + g.view[0][2] * w[2] + g.view[0][3];
	v.y = g.view[1][0] * w[0] + g.view[1][1] * w[1] + g.view[1][2] * w[2] + g.view[1][3];
	v.z = g.view[2][0] * w[0] + g.view[2][1] * w[1] + g.view[2][2] * w[2] + g.view[2][3];
	return v;
}

inline D3 DirToView( const RefGrid &g, const float *d )
{
	return { g.view[0][0] * d[0] + g.view[0][1] * d[1] + g.view[0][2] * d[2],
	    g.view[1][0] * d[0] + g.view[1][1] * d[1] + g.view[1][2] * d[2],
	    g.view[2][0] * d[0] + g.view[2][1] * d[1] + g.view[2][2] * d[2] };
}

// Corner c = ix + 2 iy + 4 iz: ix 0 left, iy 0 top, iz 0 near.
constexpr int kFaces[6][4] = {
    { 0, 2, 6, 4 }, // left
    { 1, 3, 7, 5 }, // right
    { 0, 1, 5, 4 }, // top
    { 2, 3, 7, 6 }, // bottom
    { 0, 1, 3, 2 }, // near
    { 4, 5, 7, 6 }, // far
};

struct RefFroxel
{
	D3 c[8];
	D3 n[6]; // inward unit normals
	double d[6] = {};
	D3 center;
	double bound = 0.0;
	double zNear = 0.0;
	double zFar = 0.0;
};

inline RefFroxel MakeFroxel( const RefGrid &g, std::uint32_t x, std::uint32_t y, std::uint32_t k )
{
	RefFroxel f;
	f.zNear = g.depths[k];
	f.zFar = g.depths[k + 1];
	for ( int corner = 0; corner < 8; ++corner )
	{
		const double px = std::min<double>( double( x + ( corner & 1 ) ) * g.tile, g.width );
		const double py =
		    std::min<double>( double( y + ( ( corner >> 1 ) & 1 ) ) * g.tile, g.height );
		f.c[corner] = PixelPoint( g, px, py, ( corner & 4 ) ? f.zFar : f.zNear );
	}
	D3 sum;
	for ( const D3 &c : f.c )
		sum = sum + c;
	f.center = sum * 0.125;
	for ( const D3 &c : f.c )
		f.bound = std::max( f.bound, Len( c - f.center ) );
	for ( int face = 0; face < 6; ++face )
	{
		const D3 &a = f.c[kFaces[face][0]];
		const D3 &b = f.c[kFaces[face][1]];
		const D3 &d = f.c[kFaces[face][3]];
		D3 n = Unit( Cross( b - a, d - a ) );
		if ( Dot( n, f.center - a ) < 0.0 )
			n = n * -1.0;
		f.n[face] = n;
		f.d[face] = -Dot( n, a );
	}
	return f;
}

inline bool InsideFroxel( const RefFroxel &f, const D3 &p, double tolerance = 0.0 )
{
	for ( int face = 0; face < 6; ++face )
	{
		if ( Dot( f.n[face], p ) + f.d[face] < -tolerance )
			return false;
	}
	return true;
}

inline D3 ClosestOnSegment( const D3 &p, const D3 &a, const D3 &b )
{
	const D3 ab = b - a;
	const double l2 = Dot( ab, ab );
	double t = l2 > 0.0 ? Dot( p - a, ab ) / l2 : 0.0;
	t = std::clamp( t, 0.0, 1.0 );
	return a + ab * t;
}

// The froxel's nearest point to p (p itself inside).
inline D3 ClosestPoint( const RefFroxel &f, const D3 &p )
{
	if ( InsideFroxel( f, p ) )
		return p;
	D3 best;
	double bestDistance = kInf;
	for ( int face = 0; face < 6; ++face )
	{
		const D3 &n = f.n[face];
		const D3 q = p - n * ( Dot( n, p ) + f.d[face] );
		// Inward of each edge within the face: toward the face's center.
		const D3 faceCenter = ( f.c[kFaces[face][0]] + f.c[kFaces[face][1]] + f.c[kFaces[face][2]] +
		                          f.c[kFaces[face][3]] ) *
		                      0.25;
		bool inside = true;
		for ( int e = 0; e < 4 && inside; ++e )
		{
			const D3 &a = f.c[kFaces[face][e]];
			const D3 &b = f.c[kFaces[face][( e + 1 ) % 4]];
			const D3 edgeNormal = Cross( n, b - a );
			const double side = Dot( edgeNormal, faceCenter - a ) >= 0.0 ? 1.0 : -1.0;
			inside = side * Dot( edgeNormal, q - a ) >= 0.0;
		}
		if ( inside )
		{
			const double distance = Len( q - p );
			if ( distance < bestDistance )
			{
				bestDistance = distance;
				best = q;
			}
			continue;
		}
		for ( int e = 0; e < 4; ++e )
		{
			const D3 c =
			    ClosestOnSegment( p, f.c[kFaces[face][e]], f.c[kFaces[face][( e + 1 ) % 4]] );
			const double distance = Len( c - p );
			if ( distance < bestDistance )
			{
				bestDistance = distance;
				best = c;
			}
		}
	}
	return best;
}

// Whether the segment a-b enters the froxel (Liang-Barsky over the six planes).
inline bool SegmentHits( const RefFroxel &f, const D3 &a, const D3 &b )
{
	double t0 = 0.0;
	double t1 = 1.0;
	const D3 ab = b - a;
	for ( int face = 0; face < 6; ++face )
	{
		const double start = Dot( f.n[face], a ) + f.d[face];
		const double rate = Dot( f.n[face], ab );
		if ( rate == 0.0 )
		{
			if ( start < 0.0 )
				return false;
			continue;
		}
		const double t = -start / rate;
		if ( rate > 0.0 )
			t0 = std::max( t0, t );
		else
			t1 = std::min( t1, t );
		if ( t0 > t1 )
			return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// Reference lights

struct RefLight
{
	bool clustered = false; // a valid point or spot light
	bool spot = false;
	D3 c;
	double r = kInf;
	D3 axis;
	double cosT = -1.0;
	double sinT = 0.0;
	double theta = kPi;
};

inline RefLight MakeRefLight( const RefGrid &g, const RuntimeLight &light )
{
	RefLight l;
	if ( light.shape == LightShape::Directional )
		return l;
	const auto finite = []( const float *v )
	{
		return std::isfinite( v[0] ) && std::isfinite( v[1] ) && std::isfinite( v[2] );
	};
	if ( !finite( light.position ) || !std::isfinite( light.radius ) || light.radius < 0.0f )
		return l;
	if ( light.shape == LightShape::Spot )
	{
		if ( !finite( light.direction ) || !std::isfinite( light.outerCos ) )
			return l;
		const D3 dir = DirToView( g, light.direction );
		if ( !( Len( dir ) > 0.0 ) )
			return l;
		l.spot = true;
		l.axis = Unit( dir );
		l.cosT = std::clamp<double>( light.outerCos, -1.0, 1.0 );
		l.theta = std::acos( l.cosT );
		l.sinT = std::sin( l.theta );
	}
	l.clustered = true;
	l.c = ToView( g, light.position );
	l.r = light.radius > 0.0f ? double( light.radius ) : kInf;
	return l;
}

inline bool InCone( const RefLight &l, const D3 &p )
{
	const D3 v = p - l.c;
	const double len = Len( v );
	return len == 0.0 || Dot( v, l.axis ) >= l.cosT * len;
}

inline bool InLight( const RefLight &l, const D3 &p )
{
	if ( Len( p - l.c ) > l.r )
		return false;
	return !l.spot || InCone( l, p );
}

enum class Reach
{
	kNo,
	kYes,
	kNoWitness, // a spot the exact rejections left, with no witness found
};

// An orthonormal pair perpendicular to a unit vector.
inline void Perpendiculars( const D3 &a, D3 &e1, D3 &e2 )
{
	const D3 helper = std::fabs( a.x ) < 0.6 ? D3{ 1, 0, 0 } : D3{ 0, 1, 0 };
	e1 = Unit( Cross( a, helper ) );
	e2 = Cross( a, e1 );
}

// The direction of the cone nearest to u (u itself when inside).
inline D3 ClampIntoCone( const RefLight &l, const D3 &u )
{
	const double along = Dot( u, l.axis );
	if ( along >= l.cosT )
		return u;
	D3 b = u - l.axis * along;
	if ( !( Len( b ) > 1e-12 ) )
	{
		D3 e2;
		Perpendiculars( l.axis, b, e2 );
	}
	b = Unit( b );
	return l.axis * l.cosT + b * l.sinT;
}

inline Reach Reaches( const RefFroxel &f, const RefLight &l )
{
	const D3 nearest = ClosestPoint( f, l.c );
	if ( Len( nearest - l.c ) > l.r )
		return Reach::kNo;
	if ( !l.spot )
		return Reach::kYes;

	// Exact rejections of the cone.
	if ( l.cosT < 0.0 )
	{
		// Wider than a hemisphere: the rest of space is a convex cone, so the
		// froxel misses the spot exactly when every corner lies outside it.
		bool allOutside = true;
		for ( const D3 &c : f.c )
			allOutside = allOutside && !InCone( l, c );
		if ( allOutside && !InsideFroxel( f, l.c ) )
			return Reach::kNo;
	}
	else
	{
		// A plane through the apex at 90 degrees plus the half-angle from the
		// axis has the whole cone on one side.
		D3 candidates[10];
		int count = 0;
		candidates[count++] = f.center - l.c;
		for ( const D3 &c : f.c )
			candidates[count++] = c - l.c;
		for ( int i = 0; i < count; ++i )
		{
			const D3 w = candidates[i];
			const D3 across = w - l.axis * Dot( w, l.axis );
			if ( !( Len( across ) > 1e-12 ) )
				continue;
			const D3 n = l.axis * -l.sinT + Unit( across ) * l.cosT;
			bool separated = true;
			for ( const D3 &c : f.c )
				separated = separated && Dot( n, c - l.c ) > 0.0;
			if ( separated )
				return Reach::kNo;
		}
		bool behind = true;
		for ( const D3 &c : f.c )
			behind = behind && Dot( l.axis, c - l.c ) < 0.0;
		if ( behind )
			return Reach::kNo;
	}

	// Witnesses.
	if ( InsideFroxel( f, l.c ) )
		return Reach::kYes;
	D3 targets[16];
	int count = 0;
	for ( const D3 &c : f.c )
		targets[count++] = c;
	for ( const auto &face : kFaces )
		targets[count++] = ( f.c[face[0]] + f.c[face[1]] + f.c[face[2]] + f.c[face[3]] ) * 0.25;
	targets[count++] = f.center;
	targets[count++] = nearest;
	for ( int i = 0; i < count; ++i )
	{
		if ( InLight( l, targets[i] ) )
			return Reach::kYes;
	}
	double reach = l.r;
	if ( std::isinf( reach ) )
	{
		reach = 0.0;
		for ( const D3 &c : f.c )
			reach = std::max( reach, Len( c - l.c ) );
		reach = reach * 2.0 + 1.0;
	}
	const auto ray = [&]( const D3 &u )
	{
		return SegmentHits( f, l.c, l.c + Unit( u ) * reach );
	};
	for ( int i = 0; i < count; ++i )
	{
		if ( ray( ClampIntoCone( l, Unit( targets[i] - l.c ) ) ) )
			return Reach::kYes;
	}
	if ( ray( l.axis ) )
		return Reach::kYes;
	D3 e1;
	D3 e2;
	Perpendiculars( l.axis, e1, e2 );
	for ( int ring = 1; ring <= 6; ++ring )
	{
		const double angle = l.theta * ring / 6.0;
		for ( int step = 0; step < 24; ++step )
		{
			const double azimuth = 2.0 * kPi * step / 24.0;
			const D3 u =
			    l.axis * std::cos( angle ) +
			    ( e1 * std::cos( azimuth ) + e2 * std::sin( azimuth ) ) * std::sin( angle );
			if ( ray( u ) )
				return Reach::kYes;
		}
	}
	return Reach::kNoWitness;
}

// ---------------------------------------------------------------------------
// Seeded scenes

struct Scene
{
	ClusterViewDesc desc;
	ClusterLimits limits;
	std::vector<RuntimeLight> lights;
};

inline float4x4 ViewFromBasis( const D3 &eye, const D3 &right, const D3 &up, const D3 &back )
{
	float4x4 m;
	const D3 axes[3] = { right, up, back };
	for ( int r = 0; r < 3; ++r )
	{
		m.rows[r] = { float( axes[r].x ), float( axes[r].y ), float( axes[r].z ),
		    float( -Dot( axes[r], eye ) ) };
	}
	m.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	return m;
}

// A random scene: camera, projection (a quarter off-center), grid shape,
// 0-256 lights mixing points and spots placed in, straddling the near plane,
// behind and around the view, with tiny to unbounded ranges and cones up to
// 175 degrees.
inline Scene MakeScene( std::mt19937 &random, std::uint32_t maxFroxels = 2500 )
{
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	const auto uniform = [&]( double lo, double hi )
	{
		return lo + ( hi - lo ) * unit( random );
	};
	const auto logUniform = [&]( double lo, double hi )
	{
		return lo * std::pow( hi / lo, unit( random ) );
	};
	Scene scene;

	const double eyeScale = unit( random ) < 0.25 ? 16000.0 : 2000.0;
	const D3 eye = { uniform( -eyeScale, eyeScale ), uniform( -eyeScale, eyeScale ),
	    uniform( -eyeScale, eyeScale ) };
	D3 back = Unit( { uniform( -1, 1 ), uniform( -1, 1 ), uniform( -1, 1 ) } );
	if ( !( Len( back ) > 0.1 ) )
		back = { 0, 0, 1 };
	D3 right;
	D3 up;
	Perpendiculars( back, right, up );
	const double roll = uniform( 0.0, 2.0 * kPi );
	const D3 r2 = right * std::cos( roll ) + up * std::sin( roll );
	up = Cross( back, r2 );
	right = r2;

	const double fov = uniform( 20.0, 130.0 ) * kPi / 180.0;
	const double aspect = uniform( 0.4, 2.5 );
	scene.desc.widthPixels = std::uint32_t( uniform( 32.0, 900.0 ) );
	scene.desc.heightPixels =
	    std::clamp<std::uint32_t>( std::uint32_t( scene.desc.widthPixels / aspect ), 16u, 900u );
	scene.desc.nearZ = float( logUniform( 0.05, 32.0 ) );
	scene.desc.farZ = float( scene.desc.nearZ * logUniform( 4.0, 5000.0 ) );
	scene.desc.view = ViewFromBasis( eye, right, up, back );
	scene.desc.projection = render::math::Perspective( float( fov ),
	    float( scene.desc.widthPixels ) / float( scene.desc.heightPixels ), scene.desc.nearZ,
	    scene.desc.farZ );
	if ( unit( random ) < 0.25 )
	{
		scene.desc.projection.rows[0].z = float( uniform( -0.3, 0.3 ) );
		scene.desc.projection.rows[1].z = float( uniform( -0.3, 0.3 ) );
	}

	const std::uint32_t tiles[] = { 16, 32, 64, 128 };
	scene.limits.tileSizePixels = tiles[random() % 4];
	scene.limits.depthSlices = 1 + random() % 32;
	const auto froxels = [&]
	{
		const std::uint32_t t = scene.limits.tileSizePixels;
		return ( ( scene.desc.widthPixels + t - 1 ) / t ) *
		       ( ( scene.desc.heightPixels + t - 1 ) / t ) * scene.limits.depthSlices;
	};
	while ( froxels() > maxFroxels )
	{
		if ( scene.limits.tileSizePixels < 256 )
			scene.limits.tileSizePixels *= 2;
		else
			scene.limits.depthSlices = std::max( 1u, scene.limits.depthSlices / 2 );
	}
	scene.limits.maxLights = 1024;
	scene.limits.maxLightsPerFroxel = 1024;
	scene.limits.maxLightIndices = 1u << 26;
	scene.limits.maxFroxels = 1u << 20;

	// Lights are placed in view space, then carried to the world.
	const RefGrid g = MakeRefGrid( scene.desc, scene.limits );
	const double nearZ = scene.desc.nearZ;
	const double farZ = scene.desc.farZ;
	const std::uint32_t lightCount = random() % 257;
	for ( std::uint32_t i = 0; i < lightCount; ++i )
	{
		RuntimeLight light;
		light.id = i + 1;
		light.kind = light_set::LightKind::Dynamic;
		const double shape = unit( random );
		light.shape = shape < 0.03  ? LightShape::Directional
		              : shape < 0.5 ? LightShape::Point
		                            : LightShape::Spot;

		const double pick = unit( random );
		double radius;
		if ( pick < 0.1 )
			radius = logUniform( 0.01, 1.0 );
		else if ( pick < 0.2 )
			radius = logUniform( farZ * 0.5, farZ * 4.0 );
		else if ( pick < 0.25 )
			radius = 0.0; // unbounded
		else
			radius = logUniform( nearZ * 0.5, std::max( nearZ, farZ * 0.3 ) );
		light.radius = float( radius );
		const double extent = radius > 0.0 ? radius : farZ;

		D3 v;
		const double where = unit( random );
		const double px = uniform( -0.2, 1.2 ) * g.width;
		const double py = uniform( -0.2, 1.2 ) * g.height;
		if ( where < 0.5 )
			v = PixelPoint( g, px, py, logUniform( nearZ * 0.5, farZ * 1.2 ) );
		else if ( where < 0.65 )
			v = PixelPoint( g, px, py, std::max( 1e-3, nearZ + uniform( -0.9, 0.9 ) * extent ) );
		else if ( where < 0.8 )
			v = PixelPoint( g, px, py, uniform( 0.01, 1.0 ) * nearZ ) +
			    D3{ 0.0, 0.0, uniform( 0.0, std::min( extent, farZ ) ) };
		else
			v = { uniform( -1.5, 1.5 ) * farZ, uniform( -1.5, 1.5 ) * farZ,
			    uniform( -1.5, 0.3 ) * farZ };
		const D3 world = eye + right * v.x + up * v.y + back * v.z;
		light.position[0] = float( world.x );
		light.position[1] = float( world.y );
		light.position[2] = float( world.z );

		if ( light.shape != LightShape::Point )
		{
			D3 dir = Unit( { uniform( -1, 1 ), uniform( -1, 1 ), uniform( -1, 1 ) } );
			if ( unit( random ) < 0.5 )
				dir = Unit( dir * 0.5 - back );
			light.direction[0] = float( dir.x );
			light.direction[1] = float( dir.y );
			light.direction[2] = float( dir.z );
			const double half =
			    unit( random ) < 0.1 ? uniform( 90.0, 175.0 ) : uniform( 1.0, 89.0 );
			light.outerCos = float( std::cos( half * kPi / 180.0 ) );
			light.innerCos = float( std::cos( 0.8 * half * kPi / 180.0 ) );
		}
		scene.lights.push_back( light );
	}
	return scene;
}

// ---------------------------------------------------------------------------
// Builders and the oracle

struct BuildOutput
{
	bool ok = false;
	ClusterGrid grid;
	ClusterLists lists;
	ClusterStats stats;
};

struct Tally
{
	std::uint64_t scenes = 0;
	std::uint64_t froxels = 0;
	std::uint64_t lights = 0;
	std::uint64_t buildFailures = 0;
	std::uint64_t shapeErrors = 0; // grid shape, ranges, index order
	std::uint64_t pointAssigned = 0;
	std::uint64_t pointFalsePositives = 0;
	std::uint64_t spotAssigned = 0;
	std::uint64_t spotFalsePositives = 0;
	std::uint64_t spotNoWitness = 0;
	std::uint64_t reached = 0;
	std::uint64_t falseNegatives = 0;
	std::uint64_t samples = 0;
	std::uint64_t lookupOutside = 0; // FroxelAt returned a froxel not holding the point
	std::uint64_t lookupMisses = 0;  // a light lighting the point missing from its froxel
	std::uint64_t accountingErrors = 0;
	std::string first;

	void Note( const char *what, std::uint64_t scene )
	{
		if ( first.empty() )
		{
			char text[160];
			std::snprintf( text, sizeof( text ), "%s (scene %llu)", what,
			    static_cast<unsigned long long>( scene ) );
			first = text;
		}
	}

	double PointFalsePositiveRate() const
	{
		return pointAssigned ? double( pointFalsePositives ) / double( pointAssigned ) : 0.0;
	}
	double SpotFalsePositiveRate() const
	{
		return spotAssigned ? double( spotFalsePositives ) / double( spotAssigned ) : 0.0;
	}
};

// Checks one scene's lists against the reference: shape, zero false
// negatives, false positives counted, and the shading lookup at random
// points.
inline void CheckScene(
    const Scene &scene, const BuildOutput &built, std::mt19937 &random, Tally &tally )
{
	const std::uint64_t id = tally.scenes++;
	if ( !built.ok )
	{
		++tally.buildFailures;
		tally.Note( "build failed", id );
		return;
	}
	const RefGrid g = MakeRefGrid( scene.desc, scene.limits );
	const ClusterGrid &grid = built.grid;
	const std::uint32_t froxelCount = g.tilesX * g.tilesY * g.slices;
	if ( grid.tilesX != g.tilesX || grid.tilesY != g.tilesY || grid.slices != g.slices ||
	     built.lists.froxels.size() != froxelCount )
	{
		++tally.shapeErrors;
		tally.Note( "grid shape", id );
		return;
	}
	tally.froxels += froxelCount;
	const std::size_t lightCount = scene.lights.size();
	tally.lights += lightCount;

	std::vector<RefLight> lights;
	for ( const RuntimeLight &light : scene.lights )
		lights.push_back( MakeRefLight( g, light ) );

	// Membership, and the list invariants: contiguous ranges in froxel
	// order, ascending indices of clustered lights.
	std::vector<std::uint8_t> listed( std::size_t( froxelCount ) * lightCount, 0 );
	std::uint64_t expectedOffset = 0;
	for ( std::uint32_t f = 0; f < froxelCount; ++f )
	{
		const auto &range = built.lists.froxels[f];
		if ( range.offset != expectedOffset ||
		     std::uint64_t( range.offset ) + range.count > built.lists.lightIndices.size() )
		{
			++tally.shapeErrors;
			tally.Note( "froxel range", id );
			return;
		}
		expectedOffset += range.count;
		std::int64_t previous = -1;
		for ( std::uint32_t i = 0; i < range.count; ++i )
		{
			const std::uint32_t light = built.lists.lightIndices[range.offset + i];
			if ( light >= lightCount || std::int64_t( light ) <= previous ||
			     !lights[light].clustered )
			{
				++tally.shapeErrors;
				tally.Note( "light index", id );
				return;
			}
			previous = light;
			listed[std::size_t( f ) * lightCount + light] = 1;
		}
	}
	if ( expectedOffset != built.lists.lightIndices.size() ||
	     built.stats.assignments != built.lists.lightIndices.size() || built.stats.Overflowed() )
	{
		++tally.shapeErrors;
		tally.Note( "index count", id );
		return;
	}

	std::vector<RefFroxel> froxels( froxelCount );
	for ( std::uint32_t k = 0; k < g.slices; ++k )
		for ( std::uint32_t y = 0; y < g.tilesY; ++y )
			for ( std::uint32_t x = 0; x < g.tilesX; ++x )
				froxels[( k * g.tilesY + y ) * g.tilesX + x] = MakeFroxel( g, x, y, k );

	for ( std::size_t i = 0; i < lightCount; ++i )
	{
		const RefLight &l = lights[i];
		if ( !l.clustered )
			continue;
		const double depth = -l.c.z;
		for ( std::uint32_t f = 0; f < froxelCount; ++f )
		{
			const RefFroxel &fr = froxels[f];
			const bool isListed = listed[std::size_t( f ) * lightCount + i] != 0;
			Reach reach = Reach::kNo;
			const bool possible =
			    std::isinf( l.r ) || ( depth + l.r >= fr.zNear && depth - l.r <= fr.zFar &&
			                             Len( l.c - fr.center ) - fr.bound <= l.r );
			if ( possible )
				reach = Reaches( fr, l );
			if ( reach == Reach::kYes )
			{
				++tally.reached;
				if ( !isListed )
				{
					++tally.falseNegatives;
					tally.Note( "false negative", id );
				}
			}
			if ( isListed )
			{
				if ( l.spot )
				{
					++tally.spotAssigned;
					tally.spotFalsePositives += reach != Reach::kYes;
					tally.spotNoWitness += reach == Reach::kNoWitness;
				}
				else
				{
					++tally.pointAssigned;
					tally.pointFalsePositives += reach != Reach::kYes;
				}
			}
		}
	}

	// The shading lookup: a point inside a light's volume by a margin must
	// find the light in the froxel FroxelAt names, and that froxel must hold
	// the point.
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	for ( int s = 0; s < 64; ++s )
	{
		const double px = unit( random ) * g.width;
		const double py = unit( random ) * g.height;
		const double d = g.nearZ * std::pow( g.farZ / g.nearZ, unit( random ) );
		const D3 p = PixelPoint( g, px, py, d );
		const std::uint32_t f =
		    render::pass::lights::FroxelAt( grid, float( px ), float( py ), float( d ) );
		++tally.samples;
		if ( f >= froxelCount || !InsideFroxel( froxels[f], p, 1e-4 * d ) )
		{
			++tally.lookupOutside;
			tally.Note( "lookup outside its froxel", id );
			continue;
		}
		for ( std::size_t i = 0; i < lightCount; ++i )
		{
			const RefLight &l = lights[i];
			if ( !l.clustered )
				continue;
			const D3 v = p - l.c;
			const double distance = Len( v );
			const double margin = 1e-3 * ( distance + d );
			if ( !std::isinf( l.r ) && distance > l.r - margin )
				continue;
			if ( l.spot )
			{
				if ( distance < margin )
					continue;
				const double angle =
				    std::acos( std::clamp( Dot( v, l.axis ) / distance, -1.0, 1.0 ) );
				if ( angle > l.theta - 1e-3 )
					continue;
			}
			if ( !listed[std::size_t( f ) * lightCount + i] )
			{
				++tally.lookupMisses;
				tally.Note( "lookup miss", id );
			}
		}
	}
}

} // namespace cluster_oracle

#endif // RENDER_LIGHTS_CLUSTER_ORACLE_H
