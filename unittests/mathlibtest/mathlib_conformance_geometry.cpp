//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: mathlib.core geometry section: box/plane classification and
//          frustum culling (the renderer's per-object tests), polygon
//          clipping, closest points and distances, quadratic roots and
//          splines (mathlib_base.cpp).
//
// Classification results are exact contracts. Random cases whose deciding
// corner lies within a small margin of the plane are skipped for exact
// comparison, because the float and double dot products may round to
// opposite sides there; the margin is 1e-4 of the operands' magnitude.
//
//=============================================================================//

#include "mathlib_conformance.h"

#include <algorithm>
#include <vector>

namespace mathconf
{
namespace
{

typedef int ( *BoxPlaneFn )( const Vector &, const Vector &, const cplane_t * );
typedef bool ( *CullFn )( const Vector &, const Vector &, const Frustum_t & );

int RealBoxOnPlaneSide( const Vector &mins, const Vector &maxs, const cplane_t *p ) { return BoxOnPlaneSide( mins, maxs, p ); }
int BadBoxOnPlaneSideSwapped( const Vector &mins, const Vector &maxs, const cplane_t *p )
{
	int s = BoxOnPlaneSide( mins, maxs, p );
	return s == 3 ? 3 : ( s ^ 3 );
}
int BadBoxOnPlaneSideCenter( const Vector &mins, const Vector &maxs, const cplane_t *p )
{
	Vector ctr = ( mins + maxs ) * 0.5f;
	return DotProduct( ctr, p->normal ) >= p->dist ? 1 : 2;
}
bool BadCullSkipsFar( const Vector &mins, const Vector &maxs, const Frustum_t &f ) { return R_CullBoxSkipNear( mins, maxs, f ) && false; }

cplane_t MakePlane( const Vector &n, float d )
{
	cplane_t p;
	p.normal = n;
	p.dist = d;
	p.type = ( n.x == 1.0f ) ? PLANE_X : ( n.y == 1.0f ) ? PLANE_Y : ( n.z == 1.0f ) ? PLANE_Z : PLANE_ANYZ;
	p.signbits = SignbitsForPlane( &p );
	return p;
}

// Reference: 1 if every corner is on or in front, 2 if every corner is
// behind, 3 if both; margin receives the smallest |corner distance|.
int RefBoxPlane( const Vector &mins, const Vector &maxs, const cplane_t &p, double &margin )
{
	double lo = HUGE_VAL, hi = -HUGE_VAL;
	for ( int k = 0; k < 8; ++k )
	{
		double x = ( k & 1 ) ? maxs.x : mins.x, y = ( k & 2 ) ? maxs.y : mins.y, z = ( k & 4 ) ? maxs.z : mins.z;
		double d = x * p.normal.x + y * p.normal.y + z * p.normal.z - p.dist;
		lo = std::min( lo, d );
		hi = std::max( hi, d );
	}
	margin = std::min( std::fabs( lo ), std::fabs( hi ) );
	return ( hi >= 0 ? 1 : 0 ) | ( lo < 0 ? 2 : 0 );
}

void CheckBoxOnPlaneSide( Checks &c, BoxPlaneFn fn )
{
	Rng rng( g_nSeed ^ 0xB0 );
	int compared = 0;
	for ( int i = 0; i < 40000; ++i )
	{
		Vector a = rng.Vec( -1000, 1000 ), b = a + rng.Vec( 0, 200 );
		Vector n = ( i % 4 == 0 ) ? Vector( ( i / 4 ) % 3 == 0, ( i / 4 ) % 3 == 1, ( i / 4 ) % 3 == 2 ) : rng.UnitVec();
		cplane_t p = MakePlane( n, rng.Float( -1200, 1200 ) );
		double margin;
		int want = RefBoxPlane( a, b, p, margin );
		if ( margin < 1e-4 * 2000 )
			continue;
		++compared;
		int got = fn( a, b, &p );
		c.Check( got == want, "geometry.box-on-plane-side", "got %d want %d type %d", got, want, p.type );
	}
	c.Check( compared > 30000, "geometry.box-on-plane-side.coverage", "%d cases compared", compared );
}

void CheckCullBox( Checks &c, CullFn fn )
{
	Rng rng( g_nSeed ^ 0xB1 );
	int culled = 0, kept = 0;
	for ( int i = 0; i < 4000; ++i )
	{
		Vector origin = rng.Vec( -500, 500 );
		QAngle ang = rng.Angles( 89.0f );
		ang.z = 0;
		Frustum_t fr;
		float znear = 4.0f, zfar = 4000.0f;
		GeneratePerspectiveFrustum( origin, ang, znear, zfar, rng.Float( 60, 110 ), rng.Float( 1.0f, 2.0f ), fr );
		for ( int j = 0; j < 8; ++j )
		{
			Vector a = origin + rng.Vec( -3000, 3000 ), b = a + rng.Vec( 1, 300 );
			bool want = false, nearBoundary = false;
			for ( int pl = 0; pl < FRUSTUM_NUMPLANES; ++pl )
			{
				double margin;
				int side = RefBoxPlane( a, b, *fr.GetPlane( pl ), margin );
				nearBoundary = nearBoundary || margin < 0.5;
				want = want || side == 2;
			}
			if ( nearBoundary )
				continue;
			bool got = fn( a, b, fr );
			( want ? culled : kept )++;
			c.Check( got == want, "geometry.cull-box", "got %d want %d", got, want );
		}
	}
	c.Check( culled > 1000 && kept > 1000, "geometry.cull-box.coverage", "culled %d kept %d", culled, kept );
}

// Sutherland-Hodgman in double, keeping the side dot(n, p) >= dist.
std::vector< DVec > RefClip( const std::vector< DVec > &in, const DVec &n, double dist )
{
	std::vector< DVec > out;
	for ( size_t i = 0; i < in.size(); ++i )
	{
		const DVec &p = in[ i ], &q = in[ ( i + 1 ) % in.size() ];
		double dp = p.x * n.x + p.y * n.y + p.z * n.z - dist, dq = q.x * n.x + q.y * n.y + q.z * n.z - dist;
		if ( dp >= 0 )
			out.push_back( p );
		if ( ( dp >= 0 ) != ( dq >= 0 ) )
		{
			double t = dp / ( dp - dq );
			out.push_back( DVec{ p.x + t * ( q.x - p.x ), p.y + t * ( q.y - p.y ), p.z + t * ( q.z - p.z ) } );
		}
	}
	return out;
}

double PolyArea( const std::vector< DVec > &v )
{
	DVec s = { 0, 0, 0 };
	for ( size_t i = 1; i + 1 < v.size(); ++i )
	{
		DVec a = { v[ i ].x - v[ 0 ].x, v[ i ].y - v[ 0 ].y, v[ i ].z - v[ 0 ].z };
		DVec b = { v[ i + 1 ].x - v[ 0 ].x, v[ i + 1 ].y - v[ 0 ].y, v[ i + 1 ].z - v[ 0 ].z };
		s.x += a.y * b.z - a.z * b.y;
		s.y += a.z * b.x - a.x * b.z;
		s.z += a.x * b.y - a.y * b.x;
	}
	return 0.5 * std::sqrt( s.x * s.x + s.y * s.y + s.z * s.z );
}

void RunClipping( Checks &c )
{
	Rng rng( g_nSeed ^ 0xB2 );
	for ( int i = 0; i < 4000; ++i )
	{
		// A random convex polygon: a regular n-gon in a random plane.
		int n = rng.Int( 3, 12 );
		Vector axis = rng.UnitVec(), u, w;
		VectorVectors( axis, u, w );
		Vector ctr = rng.Vec( -500, 500 );
		float radius = rng.Float( 10, 400 );
		Vector in[ 64 ], out[ 64 ];
		std::vector< DVec > din;
		for ( int k = 0; k < n; ++k )
		{
			float t = 2.0f * (float)M_PI * k / n;
			in[ k ] = ctr + u * ( radius * cosf( t ) ) + w * ( radius * sinf( t ) );
			din.push_back( DV( in[ k ] ) );
		}
		Vector pn = rng.UnitVec();
		float pd = DotProduct( pn, ctr ) + rng.Float( -radius, radius );
		const float eps = 0.1f;
		int count = ClipPolyToPlane( in, n, out, pn, pd, eps );
		std::vector< DVec > ref = RefClip( din, DV( pn ), pd );

		bool sideOk = true;
		std::vector< DVec > got;
		for ( int k = 0; k < count; ++k )
		{
			sideOk = sideOk && DotProduct( out[ k ], pn ) - pd >= -eps - 1e-3f;
			got.push_back( DV( out[ k ] ) );
		}
		c.Check( sideOk, "geometry.clip-poly.kept-side" );
		double area = PolyArea( ref );
		// Vertices within eps of the plane count as on it, so a band of the
		// polygon eps / sin( angle between the planes ) wide may be kept or
		// dropped.
		double sinTheta = CrossProduct( axis, pn ).Length();
		double band = std::fmin( 2.0 * eps / std::fmax( sinTheta, 1e-6 ), 2.0 * radius );
		double tol = band * 2 * radius + 1e-3 * radius * radius;
		c.Check( std::fabs( ( count >= 3 ? PolyArea( got ) : 0.0 ) - area ) <= tol, "geometry.clip-poly.area", "got %g want %g",
			count >= 3 ? PolyArea( got ) : 0.0, area );
	}
}

void RunDistances( Checks &c )
{
	Rng rng( g_nSeed ^ 0xB3 );
	for ( int i = 0; i < 10000; ++i )
	{
		Vector p = rng.Vec( -1000, 1000 ), a = rng.Vec( -1000, 1000 ), b = rng.Vec( -1000, 1000 );
		DVec dp = DV( p ), da = DV( a ), db = DV( b );
		DVec ab = { db.x - da.x, db.y - da.y, db.z - da.z };
		double t = ( ( dp.x - da.x ) * ab.x + ( dp.y - da.y ) * ab.y + ( dp.z - da.z ) * ab.z ) / ( ab.x * ab.x + ab.y * ab.y + ab.z * ab.z );
		double ts = std::clamp( t, 0.0, 1.0 );
		DVec q = { da.x + ts * ab.x, da.y + ts * ab.y, da.z + ts * ab.z };
		double dist = std::sqrt( ( q.x - dp.x ) * ( q.x - dp.x ) + ( q.y - dp.y ) * ( q.y - dp.y ) + ( q.z - dp.z ) * ( q.z - dp.z ) );
		Vector closest;
		float tf;
		CalcClosestPointOnLineSegment( p, a, b, closest, &tf );
		c.Sample( "geometry.closest-point-on-segment", DMaxDiff( q, closest ), 4e-3, "" );
		c.Sample( "geometry.distance-to-segment", std::fabs( CalcDistanceToLineSegment( p, a, b ) - dist ), 4e-3, "" );
		CalcClosestPointOnLine( p, a, b, closest, &tf );
		c.Sample( "geometry.closest-point-on-line.t", std::fabs( tf - t ) / ( 1 + std::fabs( t ) ), 2e-5, "" );

		Vector mins, maxs;
		VectorMin( a, b, mins );
		VectorMax( a, b, maxs );
		DVec cl = { std::clamp( dp.x, (double)mins.x, (double)maxs.x ), std::clamp( dp.y, (double)mins.y, (double)maxs.y ),
			std::clamp( dp.z, (double)mins.z, (double)maxs.z ) };
		double d2 = ( cl.x - dp.x ) * ( cl.x - dp.x ) + ( cl.y - dp.y ) * ( cl.y - dp.y ) + ( cl.z - dp.z ) * ( cl.z - dp.z );
		Vector aabbClosest;
		CalcClosestPointOnAABB( mins, maxs, p, aabbClosest );
		c.Check( DMaxDiff( cl, aabbClosest ) == 0.0, "geometry.closest-point-on-aabb" );
		c.Sample( "geometry.sqr-distance-to-aabb", std::fabs( CalcSqrDistanceToAABB( mins, maxs, p ) - d2 ) / ( 1 + d2 ), 4e-7, "" );
	}

	// SolveQuadratic: roots of a x^2 + b x + c.
	for ( int i = 0; i < 10000; ++i )
	{
		double r1 = rng.Double( -100, 100 ), r2 = rng.Double( -100, 100 ), a = rng.Double( 0.1, 10 );
		float fa = (float)a, fb = (float)( -a * ( r1 + r2 ) ), fc = (float)( a * r1 * r2 );
		float g1, g2;
		bool ok = SolveQuadratic( fa, fb, fc, g1, g2 );
		double disc = (double)fb * fb - 4.0 * fa * fc;
		if ( disc < 1e-3 * ( (double)fb * fb ) )
			continue;
		double s = std::sqrt( disc ), w1 = ( -fb - s ) / ( 2 * fa ), w2 = ( -fb + s ) / ( 2 * fa );
		double e = std::fmin( std::fmax( std::fabs( g1 - w1 ), std::fabs( g2 - w2 ) ), std::fmax( std::fabs( g1 - w2 ), std::fabs( g2 - w1 ) ) );
		c.Check( ok, "geometry.solve-quadratic.real" );
		c.Sample( "geometry.solve-quadratic", e / ( 1 + std::fabs( w1 ) + std::fabs( w2 ) ) / ( 1 + std::fabs( fb ) / s ), 4e-6, "" );
	}
	float g1, g2;
	c.Check( !SolveQuadratic( 1, 0, 1, g1, g2 ), "geometry.solve-quadratic.complex" );

	// Catmull-Rom through p2 (t=0) and p3 (t=1), uniform parameterization.
	for ( int i = 0; i < 5000; ++i )
	{
		Vector p[ 4 ] = { rng.Vec( -100, 100 ), rng.Vec( -100, 100 ), rng.Vec( -100, 100 ), rng.Vec( -100, 100 ) };
		float t = rng.Float( 0, 1 );
		Vector out;
		Catmull_Rom_Spline( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ], t, out );
		double t2 = (double)t * t, t3 = t2 * t;
		double w0 = 0.5 * ( -t3 + 2 * t2 - t ), w1 = 0.5 * ( 3 * t3 - 5 * t2 + 2 ), w2 = 0.5 * ( -3 * t3 + 4 * t2 + t ), w3 = 0.5 * ( t3 - t2 );
		DVec want = { w0 * p[ 0 ].x + w1 * p[ 1 ].x + w2 * p[ 2 ].x + w3 * p[ 3 ].x, w0 * p[ 0 ].y + w1 * p[ 1 ].y + w2 * p[ 2 ].y + w3 * p[ 3 ].y,
			w0 * p[ 0 ].z + w1 * p[ 1 ].z + w2 * p[ 2 ].z + w3 * p[ 3 ].z };
		c.Sample( "geometry.catmull-rom", DMaxDiff( want, out ), 2e-4, "" );
	}

	// ComputeTrianglePlane: unit normal, all three points on the plane.
	for ( int i = 0; i < 5000; ++i )
	{
		Vector a = rng.Vec( -1000, 1000 ), b = rng.Vec( -1000, 1000 ), cc = rng.Vec( -1000, 1000 );
		Vector n;
		float d;
		ComputeTrianglePlane( a, b, cc, n, d );
		double e = std::fmax( std::fabs( DotProduct( n, a ) - d ), std::fmax( std::fabs( DotProduct( n, b ) - d ), std::fabs( DotProduct( n, cc ) - d ) ) );
		// A thin triangle's normal is conditioned by 1 / sin( its angle at a ).
		Vector ab = b - a, ac = cc - a;
		double cond = (double)ab.Length() * ac.Length() / std::fmax( CrossProduct( ab, ac ).Length(), 1e-12f );
		c.Sample( "geometry.triangle-plane", e / 1000.0 / cond, 4e-6, "" );
		c.Sample( "geometry.triangle-plane.unit", std::fabs( n.Length() - 1.0 ), 4e-7, "" );
	}
}

} // namespace

void RunGeometrySection( Checks &c )
{
	CheckBoxOnPlaneSide( c, RealBoxOnPlaneSide );
	CheckCullBox( c, R_CullBox );
	RunClipping( c );
	RunDistances( c );

	ExpectRejected( c, "geometry.box-on-plane-side.rejects-swapped-sides", []( Checks &s ) { CheckBoxOnPlaneSide( s, BadBoxOnPlaneSideSwapped ); } );
	ExpectRejected( c, "geometry.box-on-plane-side.rejects-center-test", []( Checks &s ) { CheckBoxOnPlaneSide( s, BadBoxOnPlaneSideCenter ); } );
	ExpectRejected( c, "geometry.cull-box.rejects-never-cull", []( Checks &s ) { CheckCullBox( s, BadCullSkipsFar ); } );
}

} // namespace mathconf
