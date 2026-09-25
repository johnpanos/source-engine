//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the convex brush and scene geometry declared in
//			public/hammer/geometry/brush.h (RFC 0002, hammer.geometry). See that
//			header for the contract and the outward-orientation robustness policy.
//
//=============================================================================//

#include "hammer/geometry/brush.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

namespace hammer::geometry
{

namespace
{

// A half-space clip needs a tolerance so that vertices lying exactly on a plane
// (shared edges between faces of the same brush) are kept, not eroded away.
constexpr double kOnPlaneEps = 1.0e-4;

// Half-extent of the initial face quad. Source map coordinates are bounded well
// within +/-32768, so a quad this large fully covers any authored face before it
// is clipped down by the neighbouring planes.
constexpr double kQuadExtent = 1.0e5;

// Minimum edge length / triangle area for a face to be considered non-degenerate.
constexpr double kMinArea = 1.0e-3;

Vec3d Sub( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x - b.x, a.y - b.y, a.z - b.z );
}

Vec3d Add( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x + b.x, a.y + b.y, a.z + b.z );
}

Vec3d Scale( const Vec3d &a, double s )
{
	return Vec3d( a.x * s, a.y * s, a.z * s );
}

double Dot( const Vec3d &a, const Vec3d &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3d Cross( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}

double Length( const Vec3d &a )
{
	return std::sqrt( Dot( a, a ) );
}

Vec3d Normalized( const Vec3d &a, bool &ok )
{
	const double len = Length( a );
	if ( len <= 1.0e-12 )
	{
		ok = false;
		return Vec3d();
	}
	ok = true;
	return Scale( a, 1.0 / len );
}

// Sutherland-Hodgman clip of a convex polygon against the half-space
// dot(normal, p) <= dist. Points within kOnPlaneEps of the plane are treated as
// inside, so a face is not eroded along an edge it shares with the clipping plane.
void ClipPolygon( std::vector<Vec3d> &poly, const Plane &clip )
{
	if ( poly.empty() )
	{
		return;
	}

	std::vector<Vec3d> out;
	out.reserve( poly.size() + 1 );

	const std::size_t count = poly.size();
	for ( std::size_t i = 0; i < count; ++i )
	{
		const Vec3d &cur = poly[i];
		const Vec3d &next = poly[( i + 1 ) % count];

		const double dCur = Dot( clip.normal, cur ) - clip.dist;
		const double dNext = Dot( clip.normal, next ) - clip.dist;

		const bool curIn = dCur <= kOnPlaneEps;

		if ( curIn )
		{
			out.push_back( cur );
		}

		// Crossing the plane (strictly, ignoring the on-plane tolerance) inserts
		// the intersection point.
		if ( ( dCur < -kOnPlaneEps && dNext > kOnPlaneEps ) ||
		     ( dCur > kOnPlaneEps && dNext < -kOnPlaneEps ) )
		{
			const double t = dCur / ( dCur - dNext );
			out.push_back( Add( cur, Scale( Sub( next, cur ), t ) ) );
		}
	}

	poly.swap( out );
}

// Removes near-duplicate consecutive vertices left by clipping.
void DedupePolygon( std::vector<Vec3d> &poly )
{
	if ( poly.size() < 2 )
	{
		return;
	}

	std::vector<Vec3d> out;
	out.reserve( poly.size() );
	for ( std::size_t i = 0; i < poly.size(); ++i )
	{
		const Vec3d &cur = poly[i];
		const Vec3d &prev = out.empty() ? poly.back() : out.back();
		if ( Length( Sub( cur, prev ) ) > kOnPlaneEps )
		{
			out.push_back( cur );
		}
	}
	poly.swap( out );
}

// Newell area of a planar polygon, projected onto 'normal'. Used to reject
// degenerate faces and to fix winding to CCW about the outward normal.
double SignedArea( const std::vector<Vec3d> &poly, const Vec3d &normal )
{
	if ( poly.size() < 3 )
	{
		return 0.0;
	}

	Vec3d sum;
	for ( std::size_t i = 0; i < poly.size(); ++i )
	{
		sum = Add( sum, Cross( poly[i], poly[( i + 1 ) % poly.size()] ) );
	}
	return 0.5 * Dot( sum, normal );
}

// Solves for the intersection point of three planes. Returns false when the
// planes are (near) linearly dependent (no unique point).
bool SolveThreePlanes( const Plane &a, const Plane &b, const Plane &c, Vec3d &out )
{
	// Cramer's rule on [na; nb; nc] x = [da; db; dc].
	const Vec3d bc = Cross( b.normal, c.normal );
	const double det = Dot( a.normal, bc );
	if ( std::fabs( det ) < 1.0e-9 )
	{
		return false;
	}
	const Vec3d ca = Cross( c.normal, a.normal );
	const Vec3d ab = Cross( a.normal, b.normal );
	out = Scale(
	    Add( Add( Scale( bc, a.dist ), Scale( ca, b.dist ) ), Scale( ab, c.dist ) ), 1.0 / det );
	return true;
}

// Computes a point strictly interior to the convex region bounded by 'planes',
// independent of the planes' outward/inward orientation. It enumerates the
// polytope's candidate vertices (triple-plane intersections that lie on a single
// consistent side of every plane) and averages them. Returns false when the
// planes bound no finite region (no such vertices).
bool ComputeInteriorPoint( const std::vector<Plane> &planes, Vec3d &out )
{
	Vec3d sum;
	std::size_t count = 0;
	const std::size_t n = planes.size();
	for ( std::size_t i = 0; i < n; ++i )
	{
		for ( std::size_t j = i + 1; j < n; ++j )
		{
			for ( std::size_t k = j + 1; k < n; ++k )
			{
				Vec3d p;
				if ( !SolveThreePlanes( planes[i], planes[j], planes[k], p ) )
				{
					continue;
				}
				// Scale the on-polytope tolerance with the point magnitude so it
				// stays meaningful for large Source map coordinates.
				const double eps =
				    1.0e-4 * ( 1.0 + std::max( std::fabs( p.x ),
				                         std::max( std::fabs( p.y ), std::fabs( p.z ) ) ) );
				bool allBelow = true;
				bool allAbove = true;
				for ( std::size_t m = 0; m < n; ++m )
				{
					const double s = Dot( planes[m].normal, p ) - planes[m].dist;
					if ( s > eps )
					{
						allBelow = false;
					}
					if ( s < -eps )
					{
						allAbove = false;
					}
				}
				if ( allBelow || allAbove )
				{
					sum = Add( sum, p );
					++count;
				}
			}
		}
	}
	if ( count == 0 )
	{
		return false;
	}
	out = Scale( sum, 1.0 / static_cast<double>( count ) );
	return true;
}

bool ParseDouble( const std::string &token, double &out )
{
	if ( token.empty() )
	{
		return false;
	}
	char *end = nullptr;
	const double v = std::strtod( token.c_str(), &end );
	if ( end == token.c_str() || *end != '\0' )
	{
		return false;
	}
	out = v;
	return true;
}

} // namespace

std::optional<Vec3d> ParseVec3( const std::string &text )
{
	double c[3];
	std::size_t idx = 0;
	std::size_t i = 0;
	const std::size_t n = text.size();
	while ( idx < 3 )
	{
		while ( i < n && std::isspace( static_cast<unsigned char>( text[i] ) ) )
		{
			++i;
		}
		const std::size_t start = i;
		while ( i < n && !std::isspace( static_cast<unsigned char>( text[i] ) ) )
		{
			++i;
		}
		if ( i == start )
		{
			return std::nullopt;
		}
		if ( !ParseDouble( text.substr( start, i - start ), c[idx] ) )
		{
			return std::nullopt;
		}
		++idx;
	}
	while ( i < n && std::isspace( static_cast<unsigned char>( text[i] ) ) )
	{
		++i;
	}
	if ( i != n )
	{
		return std::nullopt; // trailing garbage / extra component
	}
	return Vec3d( c[0], c[1], c[2] );
}

std::size_t WorldScene::TotalFaces() const
{
	std::size_t total = 0;
	for ( const BrushSolid &s : solids )
	{
		total += s.faces.size();
	}
	return total;
}

std::size_t WorldScene::TotalVertices() const
{
	std::size_t total = 0;
	for ( const BrushSolid &s : solids )
	{
		for ( const BrushFace &f : s.faces )
		{
			total += f.vertices.size();
		}
	}
	return total;
}

std::optional<std::array<Vec3d, 3>> ParsePlanePoints( const std::string &value )
{
	std::array<Vec3d, 3> points;
	std::size_t found = 0;
	std::size_t i = 0;
	const std::size_t n = value.size();

	while ( i < n )
	{
		if ( value[i] == '(' )
		{
			const std::size_t start = i + 1;
			const std::size_t close = value.find( ')', start );
			if ( close == std::string::npos )
			{
				return std::nullopt;
			}
			if ( found >= 3 )
			{
				return std::nullopt; // more than three points
			}
			const std::optional<Vec3d> p = ParseVec3( value.substr( start, close - start ) );
			if ( !p )
			{
				return std::nullopt;
			}
			points[found++] = *p;
			i = close + 1;
		}
		else if ( value[i] == ')' )
		{
			return std::nullopt; // unbalanced
		}
		else
		{
			++i;
		}
	}

	if ( found != 3 )
	{
		return std::nullopt;
	}
	return points;
}

std::optional<Plane> PlaneFromPoints( const Vec3d &a, const Vec3d &b, const Vec3d &c )
{
	bool ok = false;
	const Vec3d normal = Normalized( Cross( Sub( b, a ), Sub( c, a ) ), ok );
	if ( !ok )
	{
		return std::nullopt; // collinear points
	}
	Plane plane;
	plane.normal = normal;
	plane.dist = Dot( normal, a );
	return plane;
}

BrushSolid BuildSolidFromPlanes(
    const std::vector<Plane> &planes, const std::vector<std::string> &materials, int id )
{
	BrushSolid solid;
	solid.id = id;
	if ( planes.size() < 4 )
	{
		return solid; // a finite convex solid needs at least four planes
	}

	// Interior reference point: the centroid of the polytope's actual vertices,
	// computed independently of the planes' winding. This lets us orient every
	// plane outward regardless of VMF point order. If the planes bound no finite
	// region there is no interior point and the solid has no faces.
	Vec3d interior;
	if ( !ComputeInteriorPoint( planes, interior ) )
	{
		return solid;
	}

	std::vector<Plane> oriented = planes;
	for ( Plane &p : oriented )
	{
		if ( Dot( p.normal, interior ) - p.dist > 0.0 )
		{
			p.normal = Scale( p.normal, -1.0 );
			p.dist = -p.dist;
		}
	}

	for ( std::size_t i = 0; i < oriented.size(); ++i )
	{
		const Plane &face = oriented[i];

		// Build a large quad on this plane, then clip by every other plane.
		bool ok = false;
		Vec3d up = ( std::fabs( face.normal.z ) < 0.9 ) ? Vec3d( 0, 0, 1 ) : Vec3d( 1, 0, 0 );
		Vec3d u = Normalized( Cross( up, face.normal ), ok );
		if ( !ok )
		{
			continue;
		}
		Vec3d v = Cross( face.normal, u );
		const Vec3d center = Scale( face.normal, face.dist );

		std::vector<Vec3d> poly = {
		    Add( center, Add( Scale( u, -kQuadExtent ), Scale( v, -kQuadExtent ) ) ),
		    Add( center, Add( Scale( u, kQuadExtent ), Scale( v, -kQuadExtent ) ) ),
		    Add( center, Add( Scale( u, kQuadExtent ), Scale( v, kQuadExtent ) ) ),
		    Add( center, Add( Scale( u, -kQuadExtent ), Scale( v, kQuadExtent ) ) ),
		};

		for ( std::size_t j = 0; j < oriented.size() && poly.size() >= 3; ++j )
		{
			if ( j == i )
			{
				continue;
			}
			ClipPolygon( poly, oriented[j] );
		}

		DedupePolygon( poly );
		if ( poly.size() < 3 )
		{
			continue; // this plane contributes no face (redundant)
		}

		// Orient CCW about the outward normal so front faces cull consistently.
		if ( SignedArea( poly, face.normal ) < 0.0 )
		{
			std::reverse( poly.begin(), poly.end() );
		}
		if ( std::fabs( SignedArea( poly, face.normal ) ) < kMinArea )
		{
			continue;
		}

		BrushFace bf;
		bf.plane = face;
		bf.vertices = std::move( poly );
		if ( i < materials.size() )
		{
			bf.material = materials[i];
		}
		solid.faces.push_back( std::move( bf ) );
	}

	// Bounds over the resolved vertices.
	for ( const BrushFace &f : solid.faces )
	{
		for ( const Vec3d &p : f.vertices )
		{
			if ( !solid.bounded )
			{
				solid.mins = p;
				solid.maxs = p;
				solid.bounded = true;
			}
			else
			{
				solid.mins = Vec3d( std::min( solid.mins.x, p.x ), std::min( solid.mins.y, p.y ),
				    std::min( solid.mins.z, p.z ) );
				solid.maxs = Vec3d( std::max( solid.maxs.x, p.x ), std::max( solid.maxs.y, p.y ),
				    std::max( solid.maxs.z, p.z ) );
			}
		}
	}

	return solid;
}

void WorldScene::AddSolid( BrushSolid solid )
{
	if ( solid.bounded )
	{
		if ( !bounded )
		{
			mins = solid.mins;
			maxs = solid.maxs;
			bounded = true;
		}
		else
		{
			mins = Vec3d( std::min( mins.x, solid.mins.x ), std::min( mins.y, solid.mins.y ),
			    std::min( mins.z, solid.mins.z ) );
			maxs = Vec3d( std::max( maxs.x, solid.maxs.x ), std::max( maxs.y, solid.maxs.y ),
			    std::max( maxs.z, solid.maxs.z ) );
		}
	}
	solids.push_back( std::move( solid ) );
}

void WorldScene::AddDisplacement( DisplacementMesh mesh )
{
	for ( const Vec3d &p : mesh.vertices )
	{
		if ( !bounded )
		{
			mins = p;
			maxs = p;
			bounded = true;
		}
		else
		{
			mins =
			    Vec3d( std::min( mins.x, p.x ), std::min( mins.y, p.y ), std::min( mins.z, p.z ) );
			maxs =
			    Vec3d( std::max( maxs.x, p.x ), std::max( maxs.y, p.y ), std::max( maxs.z, p.z ) );
		}
	}
	displacements.push_back( std::move( mesh ) );
}

const BrushFace *FindFaceOnPlane( const BrushSolid &solid, const Plane &plane )
{
	const BrushFace *best = nullptr;
	double bestAbsDot = 0.999; // require near-parallel normals
	for ( const BrushFace &face : solid.faces )
	{
		const double d = Dot( face.plane.normal, plane.normal );
		const double ad = std::fabs( d );
		if ( ad < bestAbsDot )
		{
			continue;
		}
		// Same plane: the offset must agree once the sign is reconciled.
		const double expected = ( d > 0.0 ) ? plane.dist : -plane.dist;
		if ( std::fabs( face.plane.dist - expected ) > 0.5 )
		{
			continue;
		}
		bestAbsDot = ad;
		best = &face;
	}
	return best;
}

} // namespace hammer::geometry
