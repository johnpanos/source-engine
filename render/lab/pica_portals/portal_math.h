//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: pica_portals (the 3DS portal lab): the portable geometry every
//			portal technique of the lab shares, checked on the host by
//			portal_math_test.cpp and used on the 3DS by portal_lab.cpp.
//
//			Port conventions (render.device.v2, D13): clip depth 0 to w (D3D
//			style), clip +Y is the target's row 0. Matrices are row-major and
//			act on column vectors (clip = M * world), so a PVS1 program reads
//			their rows as four dp4 registers.
//
//			A portal is a rectangle on a wall (centre c, right R, up U, normal
//			N pointing into the room, R x U = N) with an ellipse inscribed in
//			it. Entering portal i's back side leaves portal j's front side:
//			Transfer(i, j) maps i's local (a, b, d) to j's local (-a, b, -d).
//
//=============================================================================//

#ifndef RENDER_LAB_PICA_PORTALS_PORTAL_MATH_H
#define RENDER_LAB_PICA_PORTALS_PORTAL_MATH_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace pica_portals
{

struct V3
{
	float x = 0, y = 0, z = 0;
};

inline V3 operator+( V3 a, V3 b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
inline V3 operator-( V3 a, V3 b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
inline V3 operator*( V3 a, float s )
{
	return { a.x * s, a.y * s, a.z * s };
}
inline float Dot( V3 a, V3 b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline V3 Cross( V3 a, V3 b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
inline float Length( V3 a )
{
	return std::sqrt( Dot( a, a ) );
}
inline V3 Normalize( V3 a )
{
	return a * ( 1.0f / Length( a ) );
}

struct V4
{
	float x = 0, y = 0, z = 0, w = 0;
};

struct M4
{
	float m[4][4] = {};

	static M4 Identity()
	{
		M4 r;
		for ( int i = 0; i < 4; ++i )
			r.m[i][i] = 1;
		return r;
	}
};

inline M4 operator*( const M4 &a, const M4 &b )
{
	M4 r;
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
		{
			float s = 0;
			for ( int k = 0; k < 4; ++k )
				s += a.m[i][k] * b.m[k][j];
			r.m[i][j] = s;
		}
	return r;
}

inline V4 operator*( const M4 &a, V4 v )
{
	const float in[4] = { v.x, v.y, v.z, v.w };
	float out[4];
	for ( int i = 0; i < 4; ++i )
		out[i] = a.m[i][0] * in[0] + a.m[i][1] * in[1] + a.m[i][2] * in[2] + a.m[i][3] * in[3];
	return { out[0], out[1], out[2], out[3] };
}

inline V4 Point( V3 p )
{
	return { p.x, p.y, p.z, 1 };
}

inline V3 TransformPoint( const M4 &a, V3 p )
{
	const V4 r = a * Point( p );
	return { r.x, r.y, r.z };
}

// A general inverse (cofactors); the matrices here are well conditioned.
inline M4 Inverse( const M4 &a )
{
	const float *m = &a.m[0][0];
	float inv[16];
	inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
	         m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
	inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
	         m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
	inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
	         m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
	inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
	          m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
	inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
	         m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
	inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
	         m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
	inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
	         m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
	inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
	          m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
	inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
	         m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
	inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
	         m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
	inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
	          m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
	          m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
	inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
	         m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
	inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
	         m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
	          m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
	inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
	          m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
	const float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	M4 r;
	for ( int i = 0; i < 16; ++i )
		( &r.m[0][0] )[i] = inv[i] / det;
	return r;
}

// A right-handed view: the camera looks along forward, +Y up on screen.
inline M4 LookAt( V3 eye, V3 forward, V3 up )
{
	const V3 f = Normalize( forward );
	const V3 s = Normalize( Cross( f, up ) );
	const V3 u = Cross( s, f );
	M4 r = M4::Identity();
	const V3 rows[3] = { s, u, f * -1.0f };
	for ( int i = 0; i < 3; ++i )
	{
		r.m[i][0] = rows[i].x;
		r.m[i][1] = rows[i].y;
		r.m[i][2] = rows[i].z;
		r.m[i][3] = -Dot( rows[i], eye );
	}
	return r;
}

// An off-centre frustum in view space (the camera looks down -Z): the near
// rectangle [l, r] x [b, t] at distance n maps to NDC [-1, 1]^2 and depth 0,
// distance f to depth 1.
inline M4 Frustum( float l, float r, float b, float t, float n, float f )
{
	M4 p;
	p.m[0][0] = 2 * n / ( r - l );
	p.m[0][2] = ( r + l ) / ( r - l );
	p.m[1][1] = 2 * n / ( t - b );
	p.m[1][2] = ( t + b ) / ( t - b );
	p.m[2][2] = f / ( n - f );
	p.m[2][3] = n * f / ( n - f );
	p.m[3][2] = -1;
	return p;
}

inline M4 Perspective( float fovY, float aspect, float n, float f )
{
	const float t = n * std::tan( fovY * 0.5f );
	return Frustum( -t * aspect, t * aspect, -t, t, n, f );
}

// A plane a x + b y + c z + d: positive on its kept side.
struct Plane
{
	float a = 0, b = 0, c = 0, d = 0;
	float Distance( V3 p ) const { return a * p.x + b * p.y + c * p.z + d; }
};

// Plane transform for a point transform M: a point X of M's source space lies
// on the result exactly when M X lies on `plane`.
inline Plane PullBack( const M4 &m, Plane plane )
{
	const float p[4] = { plane.a, plane.b, plane.c, plane.d };
	float r[4];
	for ( int j = 0; j < 4; ++j )
		r[j] = p[0] * m.m[0][j] + p[1] * m.m[1][j] + p[2] * m.m[2][j] + p[3] * m.m[3][j];
	return { r[0], r[1], r[2], r[3] };
}

// Lengyel's oblique near plane for D3D-style depth: the projection's near
// plane becomes `clip` (view space, kept side positive), its far plane passes
// through the frustum's far corner opposite the plane, and nothing else moves.
inline M4 ObliqueNear( const M4 &projection, Plane clip )
{
	const M4 inverse = Inverse( projection );
	const auto sign = []( float v )
	{
		return v > 0 ? 1.0f : v < 0 ? -1.0f : 0.0f;
	};
	const V4 q = inverse * V4{ sign( clip.a ), sign( clip.b ), 1, 1 };
	const float scale = 1.0f / ( clip.a * q.x + clip.b * q.y + clip.c * q.z + clip.d * q.w );
	M4 r = projection;
	r.m[2][0] = clip.a * scale;
	r.m[2][1] = clip.b * scale;
	r.m[2][2] = clip.c * scale;
	r.m[2][3] = clip.d * scale;
	return r;
}

// NDC rectangle [x0, x1] x [y0, y1] stretched to [-1, 1]^2, depth unchanged:
// a projection cropped to a portal's screen rectangle (the viewport then
// covers only that rectangle, so the clipper discards everything outside it).
inline M4 Crop( float x0, float x1, float y0, float y1 )
{
	M4 s = M4::Identity();
	s.m[0][0] = 2 / ( x1 - x0 );
	s.m[0][3] = -( x1 + x0 ) / ( x1 - x0 );
	s.m[1][1] = 2 / ( y1 - y0 );
	s.m[1][3] = -( y1 + y0 ) / ( y1 - y0 );
	return s;
}

// The six kept-side planes of a clip matrix (D3D depth: 0 <= z <= w).
inline std::array<Plane, 6> FrustumPlanes( const M4 &c )
{
	auto row = [&]( float s, int j )
	{
		return Plane{ c.m[3][0] + s * c.m[j][0], c.m[3][1] + s * c.m[j][1],
		    c.m[3][2] + s * c.m[j][2], c.m[3][3] + s * c.m[j][3] };
	};
	return { row( 1, 0 ), row( -1, 0 ), row( 1, 1 ), row( -1, 1 ),
	    Plane{ c.m[2][0], c.m[2][1], c.m[2][2], c.m[2][3] }, row( -1, 2 ) };
}

struct Box
{
	V3 lo, hi;
};

// False only when the box is wholly outside one plane (conservative).
inline bool Intersects( const std::array<Plane, 6> &planes, const Box &box )
{
	for ( const Plane &p : planes )
	{
		const V3 v{ p.a >= 0 ? box.hi.x : box.lo.x, p.b >= 0 ? box.hi.y : box.lo.y,
		    p.c >= 0 ? box.hi.z : box.lo.z };
		if ( p.Distance( v ) < 0 )
			return false;
	}
	return true;
}

template <std::size_t N> bool IntersectsAll( const std::array<Plane, N> &planes, const Box &box )
{
	for ( const Plane &p : planes )
	{
		const V3 v{ p.a >= 0 ? box.hi.x : box.lo.x, p.b >= 0 ? box.hi.y : box.lo.y,
		    p.c >= 0 ? box.hi.z : box.lo.z };
		if ( p.Distance( v ) < 0 )
			return false;
	}
	return true;
}

struct Portal
{
	V3 c, R, U, N; // centre, right, up, normal (into the room); R x U = N
	float width = 64, height = 112;

	// Local (a, b, d) to world: columns R, U, N, c.
	M4 Frame() const
	{
		M4 f = M4::Identity();
		const V3 cols[4] = { R, U, N, c };
		for ( int j = 0; j < 4; ++j )
		{
			f.m[0][j] = cols[j].x;
			f.m[1][j] = cols[j].y;
			f.m[2][j] = cols[j].z;
		}
		return f;
	}

	M4 InverseFrame() const
	{
		M4 f = M4::Identity();
		const V3 rows[3] = { R, U, N };
		for ( int i = 0; i < 3; ++i )
		{
			f.m[i][0] = rows[i].x;
			f.m[i][1] = rows[i].y;
			f.m[i][2] = rows[i].z;
			f.m[i][3] = -Dot( rows[i], c );
		}
		return f;
	}

	V3 Local( float a, float b ) const { return c + R * a + U * b; }
	// Corners: lower left, lower right, upper left.
	V3 LowerLeft() const { return Local( -width / 2, -height / 2 ); }
	V3 LowerRight() const { return Local( width / 2, -height / 2 ); }
	V3 UpperLeft() const { return Local( -width / 2, height / 2 ); }
	V3 UpperRight() const { return Local( width / 2, height / 2 ); }
	float Facing( V3 eye ) const { return Dot( eye - c, N ); }
};

// Entering `from`'s back leaves `to`'s front: (a, b, d) -> (-a, b, -d).
inline M4 Transfer( const Portal &from, const Portal &to )
{
	M4 turn = M4::Identity();
	turn.m[0][0] = -1;
	turn.m[2][2] = -1;
	return to.Frame() * turn * from.InverseFrame();
}

// The stencil and screen-texture techniques' remote view: the camera's clip
// matrix composed with the inverse transfer (exit-room points are seen behind
// the entry), with the near plane moved onto the entry plane, pushed `offset`
// units past it so the exit's own wall (coplanar with the exit) is clipped.
inline M4 RemoteClip(
    const M4 &projection, const M4 &view, const Portal &entry, const Portal &exit, float offset )
{
	const M4 back = Inverse( Transfer( entry, exit ) );
	// World plane behind the entry: -N.x + N.c - offset > 0.
	const Plane behind{ -entry.N.x, -entry.N.y, -entry.N.z, Dot( entry.N, entry.c ) - offset };
	const Plane inView = PullBack( Inverse( view ), behind );
	return ObliqueNear( projection, inView ) * view * back;
}

// The portal-plane technique (novel here): Kooima's generalized perspective
// projection from the transferred eye through the exit rectangle. The image
// plane is the portal itself, so texel (u, v) of the result is what the eye
// sees through the entry's local point (u, v) for any camera orientation: the
// texture depends on the eye's position alone, needs no oblique clipping (the
// near plane is the exit plane, pushed `offset` into the room) and maps onto
// the entry's ellipse with ordinary affine coordinates.
inline M4 PortalPlaneClip(
    V3 eye, const Portal &entry, const Portal &exit, float offset, float far )
{
	const M4 transfer = Transfer( entry, exit );
	const V3 e = TransformPoint( transfer, eye );
	const V3 pa = TransformPoint( transfer, entry.LowerLeft() );
	const V3 pb = TransformPoint( transfer, entry.LowerRight() );
	const V3 pc = TransformPoint( transfer, entry.UpperLeft() );
	const V3 vr = Normalize( pb - pa );
	const V3 vu = Normalize( pc - pa );
	const V3 vn = Normalize( Cross( vr, vu ) ); // from the plane towards the eye
	const V3 va = pa - e, vb = pb - e, vc = pc - e;
	const float d = -Dot( vn, va );
	const float n = d + offset;
	const float l = Dot( vr, va ) * n / d, r = Dot( vr, vb ) * n / d;
	const float b = Dot( vu, va ) * n / d, t = Dot( vu, vc ) * n / d;
	M4 basis = M4::Identity();
	const V3 rows[3] = { vr, vu, vn };
	for ( int i = 0; i < 3; ++i )
	{
		basis.m[i][0] = rows[i].x;
		basis.m[i][1] = rows[i].y;
		basis.m[i][2] = rows[i].z;
		basis.m[i][3] = -Dot( rows[i], e );
	}
	return Frustum( l, r, b, t, n, far ) * basis;
}

// A clip matrix whose depth is `depth` wherever it draws (row 2 a multiple
// of row 3): the stencil technique's depth reset inside the portal's mask.
// Just short of 1, so that the far clip plane (z <= w) cannot drop it.
inline M4 AtFarPlane( const M4 &clip, float depth = 0.99999f )
{
	M4 r = clip;
	for ( int j = 0; j < 4; ++j )
		r.m[2][j] = clip.m[3][j] * depth;
	return r;
}

// A screen rectangle in whole pixels (row 0 at the top) and its NDC bounds.
struct PixelRect
{
	int left = 0, top = 0, right = 0, bottom = 0;
	bool Empty() const { return right <= left || bottom <= top; }
	int Width() const { return right - left; }
	int Height() const { return bottom - top; }
	float X0( int w ) const { return 2.0f * left / w - 1; }
	float X1( int w ) const { return 2.0f * right / w - 1; }
	float Y0( int h ) const { return 1 - 2.0f * bottom / h; }
	float Y1( int h ) const { return 1 - 2.0f * top / h; }
};

// A convex polygon in clip space, clipped to the near plane (z >= 0).
template <std::size_t N> struct ClipPolygon
{
	std::array<V4, N + 4> v{};
	std::size_t count = 0;
};

template <std::size_t N>
ClipPolygon<N> ClipNear( const std::array<V4, N> &in, std::size_t count, float epsilon = 1e-4f )
{
	ClipPolygon<N> out;
	for ( std::size_t i = 0; i < count; ++i )
	{
		const V4 a = in[i], b = in[( i + 1 ) % count];
		const float da = a.z - epsilon * a.w, db = b.z - epsilon * b.w;
		if ( da >= 0 )
			out.v[out.count++] = a;
		if ( ( da >= 0 ) != ( db >= 0 ) )
		{
			const float t = da / ( da - db );
			out.v[out.count++] = { a.x + ( b.x - a.x ) * t, a.y + ( b.y - a.y ) * t,
			    a.z + ( b.z - a.z ) * t, a.w + ( b.w - a.w ) * t };
		}
	}
	return out;
}

// The screen rectangle covering a clip-space polygon (already near-clipped),
// clamped to a w x h target; empty when it misses the target.
template <std::size_t N> PixelRect ScreenRect( const ClipPolygon<N> &polygon, int w, int h )
{
	if ( polygon.count < 3 )
		return {};
	float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f;
	for ( std::size_t i = 0; i < polygon.count; ++i )
	{
		const V4 &p = polygon.v[i];
		const float x = p.x / p.w, y = p.y / p.w;
		x0 = std::min( x0, x );
		x1 = std::max( x1, x );
		y0 = std::min( y0, y );
		y1 = std::max( y1, y );
	}
	PixelRect r;
	r.left = std::max( 0, int( std::floor( ( x0 + 1 ) * 0.5f * w ) ) );
	r.right = std::min( w, int( std::ceil( ( x1 + 1 ) * 0.5f * w ) ) );
	r.top = std::max( 0, int( std::floor( ( 1 - y1 ) * 0.5f * h ) ) );
	r.bottom = std::min( h, int( std::ceil( ( 1 - y0 ) * 0.5f * h ) ) );
	if ( r.Empty() )
		return {};
	return r;
}

// The part of a portal's rectangle a clip matrix shows: its portal-plane
// rectangle (u right, v down, both 0 to 1) and the screen extent of that part
// in pixels of a w x h target. The rectangle is clipped to the frustum's sides
// and near plane with its coordinates carried along, so it is conservative
// for the visible part and exact at the screen's edges.
struct VisibleRect
{
	bool visible = false;
	float u0 = 0, u1 = 1, v0 = 0, v1 = 1;
	float pixelsWide = 0, pixelsHigh = 0;
};

inline VisibleRect VisiblePortalRect( const M4 &clip, const Portal &portal, int w, int h )
{
	struct P
	{
		V4 c;
		float u, v;
	};
	std::array<P, 12> a{}, b{};
	std::size_t n = 0;
	const V3 corners[4] = {
	    portal.UpperLeft(), portal.UpperRight(), portal.LowerRight(), portal.LowerLeft() };
	const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for ( int i = 0; i < 4; ++i )
		a[n++] = { clip * Point( corners[i] ), uv[i][0], uv[i][1] };
	// Kept side: x + w, w - x, y + w, w - y, z (all >= 0).
	auto distance = []( const V4 &c, int plane )
	{
		switch ( plane )
		{
		case 0:
			return c.w + c.x;
		case 1:
			return c.w - c.x;
		case 2:
			return c.w + c.y;
		case 3:
			return c.w - c.y;
		default:
			return c.z;
		}
	};
	for ( int plane = 0; plane < 5 && n > 0; ++plane )
	{
		std::size_t m = 0;
		for ( std::size_t i = 0; i < n; ++i )
		{
			const P &p = a[i], &q = a[( i + 1 ) % n];
			const float dp = distance( p.c, plane ), dq = distance( q.c, plane );
			if ( dp >= 0 )
				b[m++] = p;
			if ( ( dp >= 0 ) != ( dq >= 0 ) )
			{
				const float t = dp / ( dp - dq );
				b[m++] = { { p.c.x + ( q.c.x - p.c.x ) * t, p.c.y + ( q.c.y - p.c.y ) * t,
				               p.c.z + ( q.c.z - p.c.z ) * t, p.c.w + ( q.c.w - p.c.w ) * t },
				    p.u + ( q.u - p.u ) * t, p.v + ( q.v - p.v ) * t };
			}
		}
		std::swap( a, b );
		n = m;
	}
	VisibleRect r;
	if ( n < 3 )
		return r;
	r.visible = true;
	r.u0 = r.v0 = 1;
	r.u1 = r.v1 = 0;
	float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f;
	for ( std::size_t i = 0; i < n; ++i )
	{
		r.u0 = std::min( r.u0, a[i].u );
		r.u1 = std::max( r.u1, a[i].u );
		r.v0 = std::min( r.v0, a[i].v );
		r.v1 = std::max( r.v1, a[i].v );
		const float x = a[i].c.x / a[i].c.w, y = a[i].c.y / a[i].c.w;
		x0 = std::min( x0, x );
		x1 = std::max( x1, x );
		y0 = std::min( y0, y );
		y1 = std::max( y1, y );
	}
	r.u0 = std::clamp( r.u0, 0.0f, 1.0f );
	r.u1 = std::clamp( r.u1, 0.0f, 1.0f );
	r.v0 = std::clamp( r.v0, 0.0f, 1.0f );
	r.v1 = std::clamp( r.v1, 0.0f, 1.0f );
	r.pixelsWide = ( x1 - x0 ) * 0.5f * float( w );
	r.pixelsHigh = ( y1 - y0 ) * 0.5f * float( h );
	if ( r.u1 <= r.u0 || r.v1 <= r.v0 )
		r.visible = false;
	return r;
}

// A portal-plane clip matrix cropped to the rectangle [u0, u1] x [v0, v1]
// (v down) of the portal: the texture then holds only that part.
inline M4 CropToPortalRect( const M4 &planeClip, float u0, float u1, float v0, float v1 )
{
	return Crop( 2 * u0 - 1, 2 * u1 - 1, 1 - 2 * v1, 1 - 2 * v0 ) * planeClip;
}

// The cone of what can be seen through a portal (a tight cull, novel here
// only in being cheap enough for the 3DS's CPU): planes through the
// transferred eye and the edges of an octagon circumscribing the entry's
// ellipse, carried to the exit, so the eight planes hold the ellipse; with
// the exit plane itself, in the exit room's world space.
inline std::array<Plane, 9> PortalCone( V3 eye, const Portal &entry, const Portal &exit )
{
	const M4 transfer = Transfer( entry, exit );
	const V3 e = TransformPoint( transfer, eye );
	const float grow = 1.0f / std::cos( 3.14159265f / 8 );
	std::array<V3, 8> p;
	for ( int i = 0; i < 8; ++i )
	{
		const float angle = 6.2831853f * ( float( i ) + 0.5f ) / 8;
		p[std::size_t( i )] =
		    TransformPoint( transfer, entry.Local( entry.width * 0.5f * grow * std::cos( angle ),
		                                  entry.height * 0.5f * grow * std::sin( angle ) ) );
	}
	std::array<Plane, 9> planes;
	for ( int i = 0; i < 8; ++i )
	{
		const V3 a = p[std::size_t( i )], b = p[std::size_t( ( i + 1 ) % 8 )];
		V3 n = Normalize( Cross( a - e, b - e ) );
		// Orient: a point well inside the cone (through the exit's centre) is kept.
		const V3 inside = exit.c + ( exit.c - e ) * 1.0f;
		if ( Dot( n, inside - e ) < 0 )
			n = n * -1.0f;
		planes[std::size_t( i )] = { n.x, n.y, n.z, -Dot( n, e ) };
	}
	planes[8] = { exit.N.x, exit.N.y, exit.N.z, -Dot( exit.N, exit.c ) };
	return planes;
}

} // namespace pica_portals

#endif // RENDER_LAB_PICA_PORTALS_PORTAL_MATH_H
