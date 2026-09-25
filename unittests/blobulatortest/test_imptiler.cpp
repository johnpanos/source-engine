//========= Portal 2 port ======================================================//
//
// Purpose: Mesh oracle for the clean-room blobulator tiler
//          (blobulator.tiler-mesh; public/blobulator/Implicit/ImpTiler.h).
//
// The tiler polygonizes a metaball field whose lone-particle isosurface is a
// sphere of radius scale * renderRadius. This suite checks the CPU mesh that
// drawSurface() builds, through the tiler's accessors only (no render
// context), against properties that hold for any correct polygonizer:
//
//   geometry   lone-particle vertices lie on the sphere, the field at every
//              vertex equals the threshold, normals are unit and outward,
//              triangle winding agrees with the vertex normals where the grid
//              resolves the surface, and tangent frames are orthonormal
//   tetrahedra every triangle's vertices lie on inside-outside edges of the
//              Kuhn tetrahedron holding it, and it faces the outside corners
//   topology   after welding vertices at bitwise-equal positions (tile seams
//              duplicate vertices, computed identically on both sides), every
//              edge has exactly two triangles of opposite direction, each
//              component encloses a positive volume, and sphere-like ones have
//              Euler characteristic 2
//   tiles      surfaces straddling tile seams (negative coordinates too) close;
//              triangles appear in exactly the tiles a grid-corner
//              classification requires, and polygonizing the tiles one at a
//              time yields the full surface
//   frames     bitwise determinism across frames and tilers, insertion-order
//              invariance of the surface area, scale-0 particles add nothing,
//              the sorted build yields the same triangles, and the factory's
//              acquisition count follows getTiler/returnTiler
//
// Each checker is a function of copied vertex/index arrays, and the negative
// controls apply the same checkers to deliberately corrupted copies (a flipped
// triangle, an inverted surface, a dropped or duplicated triangle, a vertex
// moved off the surface, a one-ulp seam crack, a flipped normal, a non-unit
// tangent, bad indices) to show that each defect is caught.
//
// Building with -DBLOBULATOR_SEED_DEFECT drops one triangle from every mesh
// the suite reads; the manifest's sensitivity row requires that to fail.
//
//=============================================================================//

#include "blobulator/Implicit/ImpTiler.h"
#include "blobulator/Implicit/ImpRenderer.h"

#include "testing/conformance_result.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

int BlobTest_WarningCount();

namespace
{

unsigned long g_nChecks = 0;
unsigned long g_nFailures = 0;

void Check( bool bCondition, const std::string &name, const std::string &detail = std::string() )
{
	++g_nChecks;
	if ( !bCondition )
	{
		++g_nFailures;
		std::printf( "FAIL %s%s%s\n", name.c_str(), detail.empty() ? "" : ": ", detail.c_str() );
	}
}

std::string Format( const char *pszFormat, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
std::string Format( const char *pszFormat, ... )
{
	char buf[512];
	va_list args;
	va_start( args, pszFormat );
	std::vsnprintf( buf, sizeof( buf ), pszFormat, args );
	va_end( args );
	return buf;
}

//-----------------------------------------------------------------------------
// Math (double precision, independent of mathlib)
//-----------------------------------------------------------------------------
struct Vec3
{
	double x, y, z;
};

Vec3 operator+( const Vec3 &a, const Vec3 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
Vec3 operator-( const Vec3 &a, const Vec3 &b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vec3 operator*( const Vec3 &a, double s )
{
	return { a.x * s, a.y * s, a.z * s };
}
double Dot( const Vec3 &a, const Vec3 &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
Vec3 Cross( const Vec3 &a, const Vec3 &b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
double Length( const Vec3 &a )
{
	return std::sqrt( Dot( a, a ) );
}

//-----------------------------------------------------------------------------
// Mesh copies and the checkers
//-----------------------------------------------------------------------------
// Tolerances. The tiler refines each vertex until the field is within 1e-5
// (relative) of the threshold. With the retail radii the relative field slope
// at a lone sphere is |d ln f / d ln r| = 24 x / ( 1 - x ) = 1.42
// (x = ( renderR / cutoffR )^2), so that leaves 7e-6 of the radius; the
// measured maxima (INFO lines) are 7.7e-6 and 1.06e-5. The tolerances are an
// order of magnitude above that and far below the up to 3.4e-3 of the radius
// (14% of the threshold in a cluster) that four fixed refinement steps left.
//   position: 1e-4 of the sphere radius
//   field:    1e-4 of the threshold
//   tangent:  1e-3 for |s| - 1, s . n and |t - n x s|. The normal from
//             VectorNormalize is ~2e-6 short of unit (its FLT_EPSILON bias), and
//             Gram-Schmidt against it leaves s . n ~ 2e-6 / sin( angle ), where
//             the tiler projects only for sin( angle ) >= 1e-2 (measured 2.1e-4;
//             0.94 before the fallback fix).
const double POSITION_TOLERANCE = 1e-4;
const double FIELD_TOLERANCE = 1e-4;
const double TANGENT_TOLERANCE = 1e-3;
const double PI = 3.14159265358979323846;

typedef std::array<float, 3> Float3;

struct Mesh
{
	std::vector<Float3> pos;
	std::vector<Float3> normal;
	std::vector<Float3> color;
	std::vector<Float3> tangentS;
	std::vector<Float3> tangentT;
	std::vector<int> indices;

	int TriangleCount() const { return (int)indices.size() / 3; }
	Vec3 P( int i ) const { return { pos[i][0], pos[i][1], pos[i][2] }; }
	Vec3 N( int i ) const { return { normal[i][0], normal[i][1], normal[i][2] }; }
};

Float3 ToFloat3( const Vector &v )
{
	return { { v.x, v.y, v.z } };
}

Mesh Extract( const ImpTiler &tiler )
{
	Mesh mesh;
	for ( int i = 0; i < tiler.GetVertexCount(); ++i )
	{
		const ImpTiler::Vertex_t &vert = tiler.GetVertex( i );
		mesh.pos.push_back( ToFloat3( vert.pos ) );
		mesh.normal.push_back( ToFloat3( vert.normal ) );
		mesh.color.push_back( ToFloat3( vert.color ) );
		mesh.tangentS.push_back( ToFloat3( vert.tangentS ) );
		mesh.tangentT.push_back( ToFloat3( vert.tangentT ) );
	}
	for ( int t = 0; t < tiler.GetTriangleCount(); ++t )
	{
		const int *pTri = tiler.GetTriangle( t );
		mesh.indices.insert( mesh.indices.end(), pTri, pTri + 3 );
	}
#ifdef BLOBULATOR_SEED_DEFECT
	if ( !mesh.indices.empty() )
		mesh.indices.resize( mesh.indices.size() - 3 );
#endif
	return mesh;
}

// Indices that are in range (a precondition of every other checker).
bool IndicesValid( const Mesh &mesh )
{
	if ( mesh.indices.size() % 3 != 0 )
		return false;
	for ( int nIndex : mesh.indices )
	{
		if ( nIndex < 0 || nIndex >= (int)mesh.pos.size() )
			return false;
	}
	return true;
}

typedef std::array<uint32_t, 3> PosKey;

PosKey KeyOf( const Float3 &p )
{
	PosKey key;
	std::memcpy( key.data(), p.data(), sizeof( key ) );
	return key;
}

// Welded vertex id of every vertex: vertices at bitwise-equal positions share one.
std::vector<int> Weld( const Mesh &mesh, int *pWeldedCount )
{
	std::map<PosKey, int> ids;
	std::vector<int> welded( mesh.pos.size() );
	for ( size_t i = 0; i < mesh.pos.size(); ++i )
	{
		auto it = ids.insert( std::make_pair( KeyOf( mesh.pos[i] ), (int)ids.size() ) ).first;
		welded[i] = it->second;
	}
	*pWeldedCount = (int)ids.size();
	return welded;
}

struct Topology
{
	int nWeldedVertices = 0; // referenced by a triangle
	int nEdges = 0;
	int nFaces = 0;
	int nBoundaryEdges = 0;    // undirected edge with one triangle
	int nNonManifoldEdges = 0; // more than two triangles
	int nMisorientedEdges = 0; // two triangles traversing it the same way
	int nDegenerateFaces = 0;  // a repeated welded vertex
	int nComponents = 0;
	std::vector<int> euler;     // V - E + F per component
	std::vector<double> volume; // enclosed signed volume per component (outward > 0)

	bool Closed() const
	{
		return nBoundaryEdges == 0 && nNonManifoldEdges == 0 && nMisorientedEdges == 0 &&
		       nDegenerateFaces == 0;
	}
	std::string Describe() const
	{
		std::string s = Format( "V=%d E=%d F=%d boundary=%d nonmanifold=%d misoriented=%d "
		                        "degenerate=%d components=%d euler=",
		    nWeldedVertices, nEdges, nFaces, nBoundaryEdges, nNonManifoldEdges, nMisorientedEdges,
		    nDegenerateFaces, nComponents );
		for ( size_t i = 0; i < euler.size(); ++i )
			s += Format( "%s%d", i ? "," : "", euler[i] );
		s += " volume=";
		for ( size_t i = 0; i < volume.size(); ++i )
			s += Format( "%s%.6g", i ? "," : "", volume[i] );
		return s;
	}
};

int FindRoot( std::vector<int> &parent, int i )
{
	while ( parent[i] != i )
	{
		parent[i] = parent[parent[i]];
		i = parent[i];
	}
	return i;
}

Topology AnalyzeTopology( const Mesh &mesh )
{
	Topology topo;
	int nWelded = 0;
	std::vector<int> welded = Weld( mesh, &nWelded );

	struct EdgeUse
	{
		int nForward = 0; // traversed from the lower welded id to the higher
		int nBackward = 0;
	};
	std::map<std::pair<int, int>, EdgeUse> edges;
	std::vector<int> parent( nWelded );
	for ( int i = 0; i < nWelded; ++i )
		parent[i] = i;
	std::vector<char> used( nWelded, 0 );

	topo.nFaces = mesh.TriangleCount();
	for ( int t = 0; t < topo.nFaces; ++t )
	{
		int v[3] = { welded[mesh.indices[3 * t]], welded[mesh.indices[3 * t + 1]],
		    welded[mesh.indices[3 * t + 2]] };
		if ( v[0] == v[1] || v[1] == v[2] || v[0] == v[2] )
		{
			++topo.nDegenerateFaces;
			continue;
		}
		for ( int k = 0; k < 3; ++k )
		{
			int a = v[k], b = v[( k + 1 ) % 3];
			used[a] = 1;
			EdgeUse &use = edges[std::make_pair( std::min( a, b ), std::max( a, b ) )];
			if ( a < b )
				++use.nForward;
			else
				++use.nBackward;
			parent[FindRoot( parent, a )] = FindRoot( parent, b );
		}
	}

	std::map<int, int> componentOf; // root -> component index
	for ( int i = 0; i < nWelded; ++i )
	{
		if ( !used[i] )
			continue;
		++topo.nWeldedVertices;
		int nRoot = FindRoot( parent, i );
		if ( componentOf.insert( std::make_pair( nRoot, (int)componentOf.size() ) ).second )
		{
			topo.euler.push_back( 0 );
			topo.volume.push_back( 0.0 );
		}
		++topo.euler[componentOf[nRoot]];
	}
	topo.nComponents = (int)componentOf.size();

	for ( const auto &edge : edges )
	{
		const EdgeUse &use = edge.second;
		int nUses = use.nForward + use.nBackward;
		++topo.nEdges;
		--topo.euler[componentOf[FindRoot( parent, edge.first.first )]];
		if ( nUses == 1 )
			++topo.nBoundaryEdges;
		else if ( nUses > 2 )
			++topo.nNonManifoldEdges;
		else if ( use.nForward != 1 )
			++topo.nMisorientedEdges;
	}
	// Divergence theorem: the tetrahedra from a reference point to each triangle
	// sum to the enclosed volume, positive when the triangles face outward.
	const Vec3 origin = topo.nFaces > 0 ? mesh.P( mesh.indices[0] ) : Vec3{ 0.0, 0.0, 0.0 };
	for ( int t = 0; t < topo.nFaces; ++t )
	{
		int a = welded[mesh.indices[3 * t]], b = welded[mesh.indices[3 * t + 1]],
		    c = welded[mesh.indices[3 * t + 2]];
		if ( a == b || b == c || a == c )
			continue;
		int nComponent = componentOf[FindRoot( parent, a )];
		++topo.euler[nComponent];
		Vec3 pa = mesh.P( mesh.indices[3 * t] ) - origin,
		     pb = mesh.P( mesh.indices[3 * t + 1] ) - origin,
		     pc = mesh.P( mesh.indices[3 * t + 2] ) - origin;
		topo.volume[nComponent] += Dot( pa, Cross( pb, pc ) ) / 6.0;
	}
	return topo;
}

// Vertices farther than flTolerance from the sphere; the largest error in *pMax.
int CountOffSphere(
    const Mesh &mesh, const Vec3 &center, double flRadius, double flTolerance, double *pMax )
{
	int nBad = 0;
	*pMax = 0.0;
	for ( size_t i = 0; i < mesh.pos.size(); ++i )
	{
		double flErr = std::fabs( Length( mesh.P( (int)i ) - center ) - flRadius );
		*pMax = std::max( *pMax, flErr );
		if ( flErr > flTolerance )
			++nBad;
	}
	return nBad;
}

// Normals that are not unit length or not outward from the center
// (dot( n, p - c ) must exceed 0.99 |p - c|).
int CountBadSphereNormals( const Mesh &mesh, const Vec3 &center )
{
	int nBad = 0;
	for ( size_t i = 0; i < mesh.pos.size(); ++i )
	{
		Vec3 n = mesh.N( (int)i );
		Vec3 d = mesh.P( (int)i ) - center;
		if ( std::fabs( Length( n ) - 1.0 ) > 1e-5 || Dot( n, d ) <= 0.99 * Length( d ) )
			++nBad;
	}
	return nBad;
}

// Triangles whose winding disagrees with a vertex normal: the geometric face
// normal must have a positive dot product with each of its vertex normals.
// Zero-area triangles have no winding to compare and count as violations.
int CountWindingViolations( const Mesh &mesh, double *pMinCos = NULL )
{
	int nBad = 0;
	double flMinCos = 1.0;
	for ( int t = 0; t < mesh.TriangleCount(); ++t )
	{
		int v[3] = { mesh.indices[3 * t], mesh.indices[3 * t + 1], mesh.indices[3 * t + 2] };
		Vec3 face = Cross( mesh.P( v[1] ) - mesh.P( v[0] ), mesh.P( v[2] ) - mesh.P( v[0] ) );
		double flLen = Length( face );
		bool bBad = !( flLen > 0.0 );
		for ( int k = 0; k < 3 && !bBad; ++k )
		{
			double flCos = Dot( face, mesh.N( v[k] ) ) / flLen;
			flMinCos = std::min( flMinCos, flCos );
			bBad = !( flCos > 0.0 );
		}
		nBad += bBad;
	}
	if ( pMinCos )
		*pMinCos = flMinCos;
	return nBad;
}

// Vertices where the tiler's own field differs from the threshold by more
// than flRelTolerance * threshold.
int CountFieldViolations(
    const ImpTiler &tiler, const Mesh &mesh, double flRelTolerance, double *pMax )
{
	int nBad = 0;
	*pMax = 0.0;
	const double flThreshold = tiler.GetThreshold();
	for ( size_t i = 0; i < mesh.pos.size(); ++i )
	{
		Vector p( mesh.pos[i][0], mesh.pos[i][1], mesh.pos[i][2] );
		double flRel = std::fabs( tiler.EvaluateField( p ) - flThreshold ) / flThreshold;
		*pMax = std::max( *pMax, flRel );
		if ( flRel > flRelTolerance )
			++nBad;
	}
	return nBad;
}

// Tangent frames that are not orthonormal with tangentT = normal x tangentS.
int CountBadTangentFrames( const Mesh &mesh, double *pWorst = NULL )
{
	int nBad = 0;
	double flWorst = 0.0;
	for ( size_t i = 0; i < mesh.pos.size(); ++i )
	{
		Vec3 n = mesh.N( (int)i );
		Vec3 s = { mesh.tangentS[i][0], mesh.tangentS[i][1], mesh.tangentS[i][2] };
		Vec3 t = { mesh.tangentT[i][0], mesh.tangentT[i][1], mesh.tangentT[i][2] };
		double flErr = std::max( { std::fabs( Length( s ) - 1.0 ), std::fabs( Dot( s, n ) ),
		    Length( t - Cross( n, s ) ) } );
		flWorst = std::max( flWorst, flErr );
		if ( flErr > TANGENT_TOLERANCE )
			++nBad;
	}
	if ( pWorst )
		*pWorst = flWorst;
	return nBad;
}

// Each triangle against the tetrahedron it was cut from. The grid cubes are
// split into the six Kuhn tetrahedra around the cube's main diagonal (the
// header's "6 per grid cube, shared main diagonal"): the one holding a point
// is the path from corner (0,0,0) that steps first along the axis of the
// largest fractional coordinate. A triangle lies strictly inside its
// tetrahedron, so its centroid finds it. Classifying the four corners with the
// field (inside when above the threshold), every vertex must lie on an edge
// from an inside to an outside corner, and the face normal must point from
// the inside corners' centroid towards the outside corners' centroid. This
// holds however coarsely the grid samples the surface.
struct TetReport
{
	int nWrongSide = 0; // wound towards the inside
	int nOffEdge = 0;   // a vertex not on an inside-outside edge
	int nAmbiguous = 0; // a corner within 1e-5 of the threshold, or a centroid on a split
};

TetReport CheckTetrahedra( const ImpTiler &tiler, const Mesh &mesh )
{
	TetReport report;
	const double flCube = tiler.GetCubeWidth();
	const double flThreshold = tiler.GetThreshold();
	for ( int t = 0; t < mesh.TriangleCount(); ++t )
	{
		Vec3 p[3] = { mesh.P( mesh.indices[3 * t] ), mesh.P( mesh.indices[3 * t + 1] ),
		    mesh.P( mesh.indices[3 * t + 2] ) };
		Vec3 centroid = ( p[0] + p[1] + p[2] ) * ( 1.0 / 3.0 );
		double g[3] = { centroid.x / flCube, centroid.y / flCube, centroid.z / flCube };
		int cell[3];
		double frac[3];
		for ( int i = 0; i < 3; ++i )
		{
			cell[i] = (int)std::floor( g[i] );
			frac[i] = g[i] - cell[i];
		}
		int axes[3] = { 0, 1, 2 };
		std::sort( axes, axes + 3,
		    [&]( int a, int b )
		    {
			    return frac[a] > frac[b];
		    } );
		if ( frac[axes[0]] - frac[axes[1]] < 1e-9 || frac[axes[1]] - frac[axes[2]] < 1e-9 )
		{
			++report.nAmbiguous;
			continue;
		}

		// Corners along the path (0,0,0) -> ... -> (1,1,1) and their classes.
		Vec3 corners[4];
		bool bInside[4];
		bool bAmbiguous = false;
		int step[3] = { 0, 0, 0 };
		for ( int k = 0; k < 4; ++k )
		{
			if ( k > 0 )
				step[axes[k - 1]] = 1;
			corners[k] = { ( cell[0] + step[0] ) * flCube, ( cell[1] + step[1] ) * flCube,
			    ( cell[2] + step[2] ) * flCube };
			double flValue = tiler.EvaluateField(
			    Vector( (float)corners[k].x, (float)corners[k].y, (float)corners[k].z ) );
			bInside[k] = flValue > flThreshold;
			bAmbiguous = bAmbiguous || std::fabs( flValue - flThreshold ) <= 1e-5 * flThreshold;
		}
		if ( bAmbiguous )
		{
			++report.nAmbiguous;
			continue;
		}

		Vec3 in = { 0.0, 0.0, 0.0 }, out = { 0.0, 0.0, 0.0 };
		int nIn = 0, nOut = 0;
		for ( int k = 0; k < 4; ++k )
		{
			if ( bInside[k] )
			{
				in = in + corners[k];
				++nIn;
			}
			else
			{
				out = out + corners[k];
				++nOut;
			}
		}
		if ( nIn == 0 || nOut == 0 )
		{
			++report.nOffEdge; // no surface crosses this tetrahedron
			continue;
		}
		Vec3 outward = out * ( 1.0 / nOut ) - in * ( 1.0 / nIn );
		Vec3 face = Cross( p[1] - p[0], p[2] - p[0] );
		if ( !( Dot( face, outward ) > 0.0 ) )
			++report.nWrongSide;

		for ( int v = 0; v < 3; ++v )
		{
			double flBest = 1e30;
			for ( int a = 0; a < 4; ++a )
				for ( int b = 0; b < 4; ++b )
				{
					if ( !bInside[a] || bInside[b] )
						continue;
					Vec3 edge = corners[b] - corners[a];
					double s = Dot( p[v] - corners[a], edge ) / Dot( edge, edge );
					s = std::min( 1.0, std::max( 0.0, s ) );
					flBest = std::min( flBest, Length( p[v] - ( corners[a] + edge * s ) ) );
				}
			// The float vertex position rounds at ~1e-7 of its magnitude.
			if ( flBest > 1e-4 * flCube + 1e-6 * Length( p[v] ) )
				++report.nOffEdge;
		}
	}
	return report;
}

void CheckTetrahedraOf( const std::string &name, const ImpTiler &tiler, const Mesh &mesh )
{
	TetReport report = CheckTetrahedra( tiler, mesh );
	Check( report.nWrongSide == 0 && report.nOffEdge == 0, name + " triangles-match-tetrahedra",
	    Format(
	        "%d wound inward, %d vertices off their edges", report.nWrongSide, report.nOffEdge ) );
	// Ambiguity needs a corner within 1e-5 of the threshold or a centroid on a
	// split; a few in a large mesh at most.
	Check( report.nAmbiguous * 100 <= mesh.TriangleCount(), name + " tetrahedra-classified",
	    Format( "%d of %d ambiguous", report.nAmbiguous, mesh.TriangleCount() ) );
}

double SurfaceArea( const Mesh &mesh )
{
	double flArea = 0.0;
	for ( int t = 0; t < mesh.TriangleCount(); ++t )
	{
		Vec3 a = mesh.P( mesh.indices[3 * t] ), b = mesh.P( mesh.indices[3 * t + 1] ),
		     c = mesh.P( mesh.indices[3 * t + 2] );
		flArea += 0.5 * Length( Cross( b - a, c - a ) );
	}
	return flArea;
}

// The triangles as position triples (rotated so the smallest key leads, which
// keeps the winding), sorted: equal for meshes with the same oriented triangles.
typedef std::array<PosKey, 3> TriKey;

std::vector<TriKey> TriangleSet( const Mesh &mesh )
{
	std::vector<TriKey> tris;
	for ( int t = 0; t < mesh.TriangleCount(); ++t )
	{
		TriKey key = { KeyOf( mesh.pos[mesh.indices[3 * t]] ),
		    KeyOf( mesh.pos[mesh.indices[3 * t + 1]] ),
		    KeyOf( mesh.pos[mesh.indices[3 * t + 2]] ) };
		int nFirst = (int)( std::min_element( key.begin(), key.end() ) - key.begin() );
		std::rotate( key.begin(), key.begin() + nFirst, key.end() );
		tris.push_back( key );
	}
	std::sort( tris.begin(), tris.end() );
	return tris;
}

bool BitwiseEqual( const Mesh &a, const Mesh &b )
{
	return a.pos == b.pos && a.normal == b.normal && a.color == b.color &&
	       a.tangentS == b.tangentS && a.tangentT == b.tangentT && a.indices == b.indices;
}

//-----------------------------------------------------------------------------
// Tiler setup
//-----------------------------------------------------------------------------
struct Config
{
	const char *pszName;
	float flCubeWidth;
	float flRenderRadius;
	float flCutoffRadius;
};

// c_paintblob_render.cpp: flCubeWidth (8) * r_paintblob_blr_scale (1) times
// r_paintblob_blr_cubewidth (0.8; x2.5 for low-resolution blobs),
// r_paintblob_blr_render_radius (1.3) and r_paintblob_blr_cutoff_radius (5.5).
const Config s_Configs[] = {
    { "retail", 8.0f * 0.8f, 8.0f * 1.3f, 8.0f * 5.5f },
    { "retail-lowres", 8.0f * 0.8f * 2.5f, 8.0f * 1.3f, 8.0f * 5.5f },
    { "fine", 8.0f * 0.2f, 8.0f * 1.3f, 8.0f * 5.5f },
};
const Config &s_Retail = s_Configs[0];
const Config &s_Fine = s_Configs[2];

struct Particle
{
	Vec3 center;
	float flScale;
	Float3 color;
};

Particle MakeParticle( double x, double y, double z, float flScale = 1.0f )
{
	Particle p;
	p.center = { x, y, z };
	p.flScale = flScale;
	p.color = { { 0.25f, 0.5f, 0.75f } };
	return p;
}

void Configure( ImpTiler &tiler, const Config &config )
{
	tiler.SetCubeWidth( config.flCubeWidth );
	tiler.SetRenderRadius( config.flRenderRadius );
	tiler.SetCutoffRadius( config.flCutoffRadius );
}

void Insert( ImpTiler &tiler, const std::vector<Particle> &particles, bool bFourInterpolants )
{
	for ( const Particle &src : particles )
	{
		ImpParticleWithFourInterpolants p;
		p.center = Point3D( (float)src.center.x, (float)src.center.y, (float)src.center.z );
		p.setFieldScale( src.flScale );
		p.interpolants1 = Point3D( src.color[0], src.color[1], src.color[2] );
		p.interpolants2 = Point3D( 0.0f, 0.0f, 1.0f );
		p.interpolants3 = Point3D( 1.0f, 0.0f, 0.0f );
		p.interpolants4 = Point3D( 0.0f, 1.0f, 0.0f );
		if ( bFourInterpolants )
			tiler.insertParticle( &p );
		else
			tiler.insertParticle( static_cast<const ImpParticleWithOneInterpolant *>( &p ) );
	}
}

// One frame: configure, insert, build. The frame is left open for inspection.
Mesh Build( ImpTiler &tiler, const Config &config, const std::vector<Particle> &particles,
    bool bFourInterpolants = true )
{
	Configure( tiler, config );
	tiler.beginFrame( Point3D( 0.0f, 0.0f, 0.0f ), true, false );
	Insert( tiler, particles, bFourInterpolants );
	tiler.drawSurface( false );
	return Extract( tiler );
}

// The particle center as the tiler sees it (a float).
Vec3 FloatCenter( const Particle &p )
{
	return { (float)p.center.x, (float)p.center.y, (float)p.center.z };
}

// Structural checks every non-empty mesh must pass.
void CheckClosedSurface( const std::string &name, const Mesh &mesh, const Topology &topo )
{
	Check( IndicesValid( mesh ), name + " indices-valid" );
	Check( topo.Closed(), name + " closed-oriented-manifold", topo.Describe() );
	bool bOutward = !topo.volume.empty();
	for ( double flVolume : topo.volume )
		bOutward = bOutward && flVolume > 0.0;
	Check( bOutward, name + " faces-outward-positive-volume", topo.Describe() );
}

//-----------------------------------------------------------------------------
// Where a lone sphere's surface must be, from the grid alone
//-----------------------------------------------------------------------------
typedef std::array<int, 3> TileId;

int FloorDiv( int a, int b )
{
	return (int)std::floor( (double)a / b );
}

// A grid corner is inside when nearer the center than the radius by more than
// 1e-4 of it, outside when farther by as much, and ambiguous between. A grid
// cell with a clearly inside and a clearly outside corner must hold surface;
// a cell without both holds none unless one of its corners is ambiguous.
struct SphereCells
{
	bool bAnyInside = false;
	bool bAnyAmbiguous = false;
	std::set<TileId> required; // tiles owning a cell that must hold surface
	std::set<TileId> allowed;  // those and tiles owning a cell with an ambiguous corner
};

SphereCells ClassifySphereCells( const Config &config, const Vec3 &center, double flRadius )
{
	SphereCells cells;
	const double flCube = config.flCubeWidth;
	const double c[3] = { center.x, center.y, center.z };
	int lo[3], hi[3];
	for ( int i = 0; i < 3; ++i )
	{
		lo[i] = (int)std::floor( ( c[i] - flRadius ) / flCube ) - 1;
		hi[i] = (int)std::ceil( ( c[i] + flRadius ) / flCube ) + 1;
	}
	auto classify = [&]( int x, int y, int z ) -> int
	{
		Vec3 corner = { x * flCube, y * flCube, z * flCube };
		double flDist = Length( corner - center );
		if ( flDist < flRadius * ( 1.0 - 1e-4 ) )
			return 1;
		return ( flDist > flRadius * ( 1.0 + 1e-4 ) ) ? -1 : 0;
	};
	for ( int z = lo[2]; z < hi[2]; ++z )
		for ( int y = lo[1]; y < hi[1]; ++y )
			for ( int x = lo[0]; x < hi[0]; ++x )
			{
				bool bIn = false, bOut = false, bAmbiguous = false;
				for ( int k = 0; k < 8; ++k )
				{
					int nClass = classify( x + ( k & 1 ), y + ( ( k >> 1 ) & 1 ), z + ( k >> 2 ) );
					bIn = bIn || nClass > 0;
					bOut = bOut || nClass < 0;
					bAmbiguous = bAmbiguous || nClass == 0;
				}
				cells.bAnyInside = cells.bAnyInside || bIn;
				cells.bAnyAmbiguous = cells.bAnyAmbiguous || bAmbiguous;
				TileId tile = { { FloorDiv( x, ImpTiler::TILE_CELLS ),
				    FloorDiv( y, ImpTiler::TILE_CELLS ), FloorDiv( z, ImpTiler::TILE_CELLS ) } };
				if ( bIn && bOut )
					cells.required.insert( tile );
				if ( ( bIn && bOut ) || bAmbiguous )
					cells.allowed.insert( tile );
			}
	return cells;
}

// The tiles holding triangles (by centroid, which lies inside the triangle's
// grid cell) are all the required tiles and only allowed ones.
void CheckSurfaceTiles(
    const std::string &name, const Config &config, const Mesh &mesh, const SphereCells &cells )
{
	const double flTile = (double)ImpTiler::TILE_CELLS * config.flCubeWidth;
	std::set<TileId> tiles;
	for ( int t = 0; t < mesh.TriangleCount(); ++t )
	{
		Vec3 centroid = ( mesh.P( mesh.indices[3 * t] ) + mesh.P( mesh.indices[3 * t + 1] ) +
		                    mesh.P( mesh.indices[3 * t + 2] ) ) *
		                ( 1.0 / 3.0 );
		tiles.insert( { { (int)std::floor( centroid.x / flTile ),
		    (int)std::floor( centroid.y / flTile ), (int)std::floor( centroid.z / flTile ) } } );
	}
	bool bRequired =
	    std::includes( tiles.begin(), tiles.end(), cells.required.begin(), cells.required.end() );
	bool bAllowed =
	    std::includes( cells.allowed.begin(), cells.allowed.end(), tiles.begin(), tiles.end() );
	Check( bRequired && bAllowed, name + " surface-in-expected-tiles",
	    Format( "%zu tiles with triangles, %zu required, %zu allowed", tiles.size(),
	        cells.required.size(), cells.allowed.size() ) );
}

//-----------------------------------------------------------------------------
// Lone particles
//-----------------------------------------------------------------------------
void CheckLoneParticle(
    const Config &config, float flScale, const Vec3 &center, const char *pszWhere )
{
	std::string name = Format( "lone.%s.s%g.%s", config.pszName, flScale, pszWhere );
	ImpTiler tiler;
	std::vector<Particle> particles = { MakeParticle( center.x, center.y, center.z, flScale ) };
	Mesh mesh = Build( tiler, config, particles );
	Vec3 c = FloatCenter( particles[0] );
	double flRadius = (double)flScale * config.flRenderRadius;

	SphereCells cells = ClassifySphereCells( config, c, flRadius );
	if ( cells.bAnyInside )
		Check( mesh.TriangleCount() > 0, name + " non-empty" );
	else if ( !cells.bAnyAmbiguous )
		Check( mesh.TriangleCount() == 0, name + " empty-without-inside-corner" );
	CheckSurfaceTiles( name, config, mesh, cells );

	if ( mesh.TriangleCount() == 0 )
	{
		std::printf( "INFO %s: empty (corner inside: %d)\n", name.c_str(), (int)cells.bAnyInside );
		return;
	}

	double flMaxPos, flMaxField;
	int nOff = CountOffSphere( mesh, c, flRadius, POSITION_TOLERANCE * flRadius, &flMaxPos );
	Check( nOff == 0, name + " on-sphere",
	    Format( "%d of %zu vertices, max error %.3g (radius %.4g)", nOff, mesh.pos.size(), flMaxPos,
	        flRadius ) );
	int nField = CountFieldViolations( tiler, mesh, FIELD_TOLERANCE, &flMaxField );
	Check( nField == 0, name + " field-at-threshold",
	    Format( "%d vertices, max relative error %.3g", nField, flMaxField ) );
	int nNormals = CountBadSphereNormals( mesh, c );
	Check( nNormals == 0, name + " normals-outward", Format( "%d vertices", nNormals ) );
	double flMinCos, flTangentErr;
	int nWinding = CountWindingViolations( mesh, &flMinCos );
	Check( nWinding == 0, name + " winding-matches-normals",
	    Format( "%d triangles, min cosine %.3g", nWinding, flMinCos ) );
	int nTangent = CountBadTangentFrames( mesh, &flTangentErr );
	Check( nTangent == 0, name + " tangent-frame-orthonormal",
	    Format( "%d vertices, worst error %.3g", nTangent, flTangentErr ) );

	bool bColor = true;
	for ( const Float3 &color : mesh.color )
	{
		for ( int k = 0; k < 3; ++k )
			bColor = bColor && std::fabs( color[k] - particles[0].color[k] ) < 1e-5;
	}
	Check( bColor, name + " color-is-particle-color" );

	Topology topo = AnalyzeTopology( mesh );
	CheckClosedSurface( name, mesh, topo );
	CheckTetrahedraOf( name, tiler, mesh );
	Check( topo.nComponents == 1 && topo.euler.size() == 1 && topo.euler[0] == 2,
	    name + " sphere-topology", topo.Describe() );
	// Chords of a ball stay in it: the enclosed volume cannot exceed the sphere's.
	double flMaxRadius = flRadius * ( 1.0 + POSITION_TOLERANCE );
	double flBallVolume = 4.0 / 3.0 * PI * flMaxRadius * flMaxRadius * flMaxRadius;
	Check( topo.volume.size() == 1 && topo.volume[0] <= flBallVolume,
	    name + " volume-within-sphere",
	    Format(
	        "%.6g of at most %.6g", topo.volume.empty() ? 0.0 : topo.volume[0], flBallVolume ) );
	std::printf( "INFO %s: %zu vertices, %d triangles, %d tiles, max position error %.3g "
	             "(%.3g of radius), max field error %.3g, worst tangent error %.3g, "
	             "min winding cosine %.3g, volume %.4g of sphere\n",
	    name.c_str(), mesh.pos.size(), mesh.TriangleCount(), tiler.getNoTiles(), flMaxPos,
	    flMaxPos / flRadius, flMaxField, flTangentErr, flMinCos,
	    topo.volume.empty()
	        ? 0.0
	        : topo.volume[0] / ( 4.0 / 3.0 * PI * flRadius * flRadius * flRadius ) );
}

void TestLoneParticles()
{
	for ( const Config &config : s_Configs )
	{
		double flTile = (double)ImpTiler::TILE_CELLS * config.flCubeWidth;
		for ( float flScale : { 1.0f, 0.5f } )
		{
			// A grid corner (and the corner of eight tiles).
			CheckLoneParticle( config, flScale, { 0.0, 0.0, 0.0 }, "origin" );
			// Off the grid, inside one tile.
			CheckLoneParticle( config, flScale,
			    { 0.5 * flTile + 0.37 * config.flCubeWidth,
			        0.5 * flTile - 0.21 * config.flCubeWidth,
			        0.5 * flTile + 0.113 * config.flCubeWidth },
			    "offgrid" );
			// Off the grid, at negative coordinates, a quarter cell from a tile seam.
			CheckLoneParticle( config, flScale,
			    { -3.0 * flTile + 0.25 * config.flCubeWidth,
			        -1.0 * flTile - 0.61 * config.flCubeWidth,
			        -2.0 * flTile + 0.49 * config.flCubeWidth },
			    "negative-seam" );
		}
	}
}

//-----------------------------------------------------------------------------
// Tile seams
//-----------------------------------------------------------------------------
void CheckSeamParticle(
    const Config &config, const Vec3 &center, const char *pszWhere, int nMinTiles )
{
	std::string name = Format( "seam.%s.%s", config.pszName, pszWhere );
	ImpTiler tiler;
	std::vector<Particle> particles = { MakeParticle( center.x, center.y, center.z ) };
	Mesh mesh = Build( tiler, config, particles );

	Check( tiler.getNoTiles() > 1, name + " several-tiles", Format( "%d", tiler.getNoTiles() ) );

	// The fixture straddles seams: surface is required in several tiles.
	SphereCells cells =
	    ClassifySphereCells( config, FloatCenter( particles[0] ), config.flRenderRadius );
	Check( cells.required.size() >= (size_t)nMinTiles, name + " fixture-straddles-seams",
	    Format( "%zu required tiles, expected at least %d", cells.required.size(), nMinTiles ) );
	CheckSurfaceTiles( name, config, mesh, cells );

	int nWelded = 0;
	Weld( mesh, &nWelded );
	Check( nWelded < (int)mesh.pos.size(), name + " seam-vertices-duplicated",
	    Format( "%d welded of %zu", nWelded, mesh.pos.size() ) );

	Topology topo = AnalyzeTopology( mesh );
	CheckClosedSurface( name, mesh, topo );
	CheckTetrahedraOf( name, tiler, mesh );
	Check(
	    topo.nComponents == 1 && topo.euler[0] == 2, name + " sphere-topology", topo.Describe() );
	double flMax;
	int nOff = CountOffSphere( mesh, FloatCenter( particles[0] ), config.flRenderRadius,
	    POSITION_TOLERANCE * config.flRenderRadius, &flMax );
	Check( nOff == 0, name + " on-sphere", Format( "%d vertices, max error %.3g", nOff, flMax ) );
	Check( CountWindingViolations( mesh ) == 0, name + " winding-matches-normals" );

	// Polygonizing one tile at a time yields exactly the full triangle set.
	std::vector<TriKey> full = TriangleSet( mesh );
	std::vector<TriKey> pieces;
	for ( int i = 0; i < tiler.getNoTiles(); ++i )
	{
		ImpTiler single;
		single.setTileIndexToDraw( i );
		Mesh part = Build( single, config, particles );
		std::vector<TriKey> set = TriangleSet( part );
		pieces.insert( pieces.end(), set.begin(), set.end() );
	}
	std::sort( pieces.begin(), pieces.end() );
	Check( pieces == full, name + " tiles-independent",
	    Format( "%zu triangles from single tiles, %zu in the full surface", pieces.size(),
	        full.size() ) );
}

void TestSeams()
{
	for ( const Config &config : s_Configs )
	{
		double flTile = (double)ImpTiler::TILE_CELLS * config.flCubeWidth;
		double flCube = config.flCubeWidth;
		const double R = config.flRenderRadius;
		// Exactly on the tile corner at the grid origin: all eight tiles.
		CheckSeamParticle( config, { 0.0, 0.0, 0.0 }, "origin-corner", 8 );
		// Near a tile corner at negative coordinates, off the grid.
		CheckSeamParticle( config,
		    { -flTile + 0.31 * flCube, -2.0 * flTile - 0.47 * flCube,
		        -3.0 * flTile + 0.05 * flCube },
		    "negative-corner", 2 );
		// Near a tile corner with mixed signs, within half a radius of each seam.
		CheckSeamParticle(
		    config, { 2.0 * flTile - 0.12 * R, -flTile + 0.43 * R, 0.29 * R }, "mixed-corner", 2 );
	}
}

//-----------------------------------------------------------------------------
// Several particles
//-----------------------------------------------------------------------------
// bStrictWinding: require every triangle's winding to agree with its vertex
// normals. That holds where the grid resolves the surface (lone and paired
// particles); in a cluster, necks and sheets thinner than a cell put opposed
// normals on one triangle, so there the per-vertex agreement is only reported
// and the tetrahedron and volume checks prove the orientation.
void CheckGeneralSurface( const std::string &name, const ImpTiler &tiler, const Mesh &mesh,
    const Topology &topo, bool bStrictWinding )
{
	CheckClosedSurface( name, mesh, topo );
	CheckTetrahedraOf( name, tiler, mesh );
	double flMaxField;
	int nField = CountFieldViolations( tiler, mesh, FIELD_TOLERANCE, &flMaxField );
	Check( nField == 0, name + " field-at-threshold",
	    Format( "%d vertices, max relative error %.3g", nField, flMaxField ) );
	double flMinCos, flTangentErr;
	int nWinding = CountWindingViolations( mesh, &flMinCos );
	if ( bStrictWinding )
	{
		Check( nWinding == 0, name + " winding-matches-normals",
		    Format( "%d triangles, min cosine %.3g", nWinding, flMinCos ) );
	}
	int nTangent = CountBadTangentFrames( mesh, &flTangentErr );
	Check( nTangent == 0, name + " tangent-frame-orthonormal",
	    Format( "%d vertices, worst error %.3g", nTangent, flTangentErr ) );
	bool bEulerEven = true;
	for ( int nEuler : topo.euler )
		bEulerEven = bEulerEven && nEuler <= 2 && nEuler % 2 == 0;
	Check( bEulerEven, name + " euler-closed-orientable", topo.Describe() );
	std::printf( "INFO %s: %zu vertices, %d triangles, %d tiles, %s, max field error %.3g, "
	             "worst tangent error %.3g, %d triangles against a vertex normal "
	             "(min cosine %.3g)\n",
	    name.c_str(), mesh.pos.size(), mesh.TriangleCount(), tiler.getNoTiles(),
	    topo.Describe().c_str(), flMaxField, flTangentErr, nWinding, flMinCos );
}

void TestTwoParticles()
{
	for ( const Config &config : { s_Retail, s_Fine } )
	{
		const double R = config.flRenderRadius, flCutoff = config.flCutoffRadius;
		struct Case
		{
			const char *pszName;
			double flDistance;
			int nComponents;
		};
		// Bridged: the midpoint field 2 k( (d/2)^2 / Rc^2 ) exceeds the threshold
		// for d < ~2.79 R (retail radii), so 1.5 R bridges and 2.2 R (spheres
		// apart, still bridged) too; 3.4 R leaves two blobs whose fields
		// overlap; 3 cutoffs leaves two independent spheres.
		const Case cases[] = {
		    { "bridged", 1.5 * R, 1 },
		    { "bridged-apart", 2.2 * R, 1 },
		    { "separate-overlapping-fields", 3.4 * R, 2 },
		    { "separate", 3.0 * flCutoff, 2 },
		};
		for ( const Case &c : cases )
		{
			std::string name = Format( "pair.%s.%s", config.pszName, c.pszName );
			Vec3 a = { 0.3 * config.flCubeWidth, -17.1, 5.2 };
			Vec3 b = a + Vec3{ c.flDistance * 0.8, c.flDistance * 0.6, 0.0 };
			std::vector<Particle> particles = {
			    MakeParticle( a.x, a.y, a.z ), MakeParticle( b.x, b.y, b.z ) };
			ImpTiler tiler;
			Mesh mesh = Build( tiler, config, particles );
			Topology topo = AnalyzeTopology( mesh );
			CheckGeneralSurface( name, tiler, mesh, topo, true );
			Check( topo.nComponents == c.nComponents, name + " components",
			    Format( "%d, expected %d", topo.nComponents, c.nComponents ) );
			bool bSpheres = true;
			for ( int nEuler : topo.euler )
				bSpheres = bSpheres && nEuler == 2;
			Check( bSpheres, name + " components-are-spheres", topo.Describe() );

			if ( c.flDistance >= 2.0 * flCutoff )
			{
				// Independent fields: each vertex is on its nearest particle's sphere.
				Vec3 fa = FloatCenter( particles[0] ), fb = FloatCenter( particles[1] );
				int nOff = 0;
				double flMax = 0.0;
				for ( size_t i = 0; i < mesh.pos.size(); ++i )
				{
					Vec3 p = mesh.P( (int)i );
					double flErr =
					    std::fabs( std::min( Length( p - fa ), Length( p - fb ) ) - (double)R );
					flMax = std::max( flMax, flErr );
					nOff += flErr > POSITION_TOLERANCE * R;
				}
				Check( nOff == 0, name + " on-spheres", Format( "max error %.3g", flMax ) );
			}
		}
	}
}

// A seeded cluster of particles of varying scale across tile seams.
std::vector<Particle> Cluster( const Config &config, int nCount, uint32_t nSeed )
{
	uint32_t state = nSeed;
	auto next = [&state]() -> double
	{
		state = state * 1664525u + 1013904223u;
		return ( state >> 8 ) * ( 1.0 / 16777216.0 );
	};
	double flTile = (double)ImpTiler::TILE_CELLS * config.flCubeWidth;
	double flSpread = 2.5 * config.flRenderRadius;
	std::vector<Particle> particles;
	for ( int i = 0; i < nCount; ++i )
	{
		Particle p = MakeParticle( -flTile + ( next() - 0.5 ) * 2.0 * flSpread,
		    ( next() - 0.5 ) * 2.0 * flSpread, flTile + ( next() - 0.5 ) * 2.0 * flSpread,
		    (float)( 0.3 + 0.9 * next() ) );
		p.color = { { (float)next(), (float)next(), (float)next() } };
		particles.push_back( p );
	}
	return particles;
}

void TestCluster()
{
	for ( const Config &config : s_Configs )
	{
		std::string name = Format( "cluster.%s", config.pszName );
		ImpTiler tiler;
		Mesh mesh = Build( tiler, config, Cluster( config, 24, 12345u ) );
		Check( mesh.TriangleCount() > 0, name + " non-empty" );
		Topology topo = AnalyzeTopology( mesh );
		CheckGeneralSurface( name, tiler, mesh, topo, false );
	}
}

//-----------------------------------------------------------------------------
// Frames, determinism and ordering
//-----------------------------------------------------------------------------
void TestDeterminism()
{
	const Config &config = s_Retail;
	std::vector<Particle> particles = Cluster( config, 16, 777u );

	ImpTiler tiler;
	Mesh first = Build( tiler, config, particles );
	tiler.endFrame( false ); // no render context: warns and releases
	Mesh second = Build( tiler, config, particles );
	Check( first.TriangleCount() > 0 && BitwiseEqual( first, second ),
	    "determinism.same-tiler-next-frame" );
	// A frame begun without endFrame releases the previous one first.
	Mesh third = Build( tiler, config, particles );
	Check( BitwiseEqual( first, third ), "determinism.begin-without-end" );

	ImpTiler fresh;
	Check( BitwiseEqual( first, Build( fresh, config, particles ) ), "determinism.fresh-tiler" );

	// The one-interpolant insert gives the same geometry and color.
	ImpTiler one;
	Mesh oneMesh = Build( one, config, particles, false );
	Check( first.pos == oneMesh.pos && first.normal == oneMesh.normal &&
	           first.color == oneMesh.color && first.indices == oneMesh.indices,
	    "determinism.one-interpolant-same-surface" );

	// Reversed insertion changes summation order, not the surface.
	std::vector<Particle> reversed( particles.rbegin(), particles.rend() );
	ImpTiler other;
	Mesh reversedMesh = Build( other, config, reversed );
	double flArea = SurfaceArea( first ), flReversedArea = SurfaceArea( reversedMesh );
	Check( std::fabs( flArea - flReversedArea ) <= 1e-4 * flArea,
	    "determinism.insertion-order-area", Format( "%.9g vs %.9g", flArea, flReversedArea ) );
	Topology topo = AnalyzeTopology( reversedMesh );
	Topology firstTopo = AnalyzeTopology( first );
	Check(
	    topo.Closed() && topo.nComponents == firstTopo.nComponents && topo.euler == firstTopo.euler,
	    "determinism.insertion-order-topology", topo.Describe() + " vs " + firstTopo.Describe() );

	// Scale-0 particles contribute nothing, wherever they are inserted.
	std::vector<Particle> withZero = particles;
	withZero.insert( withZero.begin(),
	    MakeParticle( particles[0].center.x, particles[0].center.y, particles[0].center.z, 0.0f ) );
	withZero.push_back( MakeParticle( 1.0, 2.0, 3.0, 0.0f ) );
	ImpTiler zero;
	Check( BitwiseEqual( first, Build( zero, config, withZero ) ),
	    "scale-zero.no-contribution-in-cluster" );
	for ( const Config &c : s_Configs )
	{
		ImpTiler lone;
		Mesh empty = Build( lone, c, { MakeParticle( 0.3, -0.2, 0.1, 0.0f ) } );
		Check( empty.pos.empty() && empty.indices.empty() && lone.getNoTiles() == 0,
		    Format( "scale-zero.lone-empty.%s", c.pszName ) );
	}

	// The sorted build has the same triangles as the unsorted one.
	ImpTiler sorted;
	Configure( sorted, config );
	sorted.beginFrame( Point3D( 0.0f, 0.0f, 0.0f ), true, false );
	Insert( sorted, particles, true );
	sorted.drawSurfaceSorted( Point3D( 300.0f, -120.0f, 90.0f ), false );
	Mesh sortedMesh = Extract( sorted );
	Check( TriangleSet( sortedMesh ) == TriangleSet( first ), "sorted.same-triangle-set" );
	Check( BitwiseEqual( sortedMesh, first ), "sorted.same-arrays" );

	// endFrame without a render context warns once and releases the frame; an
	// empty frame has nothing to draw and does not warn.
	int nWarnings = BlobTest_WarningCount();
	sorted.endFrame( false );
	Check( BlobTest_WarningCount() == nWarnings + 1, "endframe.no-context-warns" );
	Check(
	    sorted.GetVertexCount() == 0 && sorted.GetTriangleCount() == 0 && sorted.getNoTiles() == 0,
	    "endframe.releases-frame" );
	sorted.endFrame( false );
	Check( BlobTest_WarningCount() == nWarnings + 1, "endframe.empty-frame-silent" );
}

//-----------------------------------------------------------------------------
// Factories
//-----------------------------------------------------------------------------
void TestFactories()
{
	ImpRendererFactory *pRenderers = ImpRendererFactory::s_pInstance;
	ImpTilerFactory *pTilers = ImpTilerFactory::factory;
	Check( pRenderers != NULL && pTilers != NULL, "factory.instances" );
	if ( !pRenderers || !pTilers )
		return;

	const int nBase = pRenderers->GetNumberOfAcquiredTileRenderers();
	ImpTiler *pA = pTilers->getTiler();
	Check( pRenderers->GetNumberOfAcquiredTileRenderers() == nBase + 1, "factory.acquire-counts" );
	ImpTiler *pB = pTilers->getTiler();
	Check( pA && pB && pA != pB, "factory.distinct-tilers" );
	Check(
	    pRenderers->GetNumberOfAcquiredTileRenderers() == nBase + 2, "factory.acquire-counts-two" );

	// A returned tiler is reused and builds the same surface as a fresh one.
	Mesh built = Build( *pB, s_Retail, { MakeParticle( 1.0, 2.0, 3.0 ) } );
	pB->endFrame( false );
	pTilers->returnTiler( pB );
	Check( pRenderers->GetNumberOfAcquiredTileRenderers() == nBase + 1, "factory.release-counts" );
	ImpTiler *pC = pTilers->getTiler();
	Check( pC == pB, "factory.reuses-returned-tiler" );
	ImpTiler fresh;
	Check( BitwiseEqual( Build( *pC, s_Retail, { MakeParticle( 1.0, 2.0, 3.0 ) } ),
	           Build( fresh, s_Retail, { MakeParticle( 1.0, 2.0, 3.0 ) } ) ) &&
	           built.TriangleCount() > 0,
	    "factory.reused-tiler-builds-same-surface" );
	pC->endFrame( false );

	pTilers->returnTiler( pA );
	pTilers->returnTiler( pC );
	Check( pRenderers->GetNumberOfAcquiredTileRenderers() == nBase, "factory.all-returned" );
	pTilers->returnTiler( NULL );
	Check( pRenderers->GetNumberOfAcquiredTileRenderers() == nBase, "factory.return-null-ignored" );
}

//-----------------------------------------------------------------------------
// Negative controls: the checkers must catch each seeded defect
//-----------------------------------------------------------------------------
void TestNegativeControls()
{
	const Config &config = s_Retail;
	// A lone sphere across the tile corner at the origin (seams duplicate vertices).
	std::vector<Particle> particles = { MakeParticle( 0.0, 0.0, 0.0 ) };
	ImpTiler tiler;
	const Mesh mesh = Build( tiler, config, particles );
	const Vec3 c = FloatCenter( particles[0] );
	const double R = config.flRenderRadius;
	double flMax;

	// The unmodified copy passes every checker the controls use.
	Topology base = AnalyzeTopology( mesh );
	bool bBase =
	    mesh.TriangleCount() > 8 && base.Closed() && base.nComponents == 1 && base.euler[0] == 2 &&
	    base.volume[0] > 0.0 && CountWindingViolations( mesh ) == 0 &&
	    CountBadTangentFrames( mesh ) == 0 && CheckTetrahedra( tiler, mesh ).nWrongSide == 0 &&
	    CheckTetrahedra( tiler, mesh ).nOffEdge == 0 && CountBadSphereNormals( mesh, c ) == 0 &&
	    CountOffSphere( mesh, c, R, POSITION_TOLERANCE * R, &flMax ) == 0 &&
	    CountFieldViolations( tiler, mesh, FIELD_TOLERANCE, &flMax ) == 0;
	Check( bBase, "control.baseline-passes", base.Describe() );
	if ( !bBase )
		return;

	// Flipped winding of one triangle.
	{
		Mesh bad = mesh;
		std::swap( bad.indices[1], bad.indices[2] );
		Topology topo = AnalyzeTopology( bad );
		Check( topo.nMisorientedEdges > 0 && !topo.Closed(), "control.flipped-triangle-topology",
		    topo.Describe() );
		Check( CountWindingViolations( bad ) == 1, "control.flipped-triangle-winding" );
		Check(
		    CheckTetrahedra( tiler, bad ).nWrongSide == 1, "control.flipped-triangle-tetrahedron" );
	}
	// The whole surface wound inward: edge orientation stays consistent, so only
	// the enclosed volume shows it.
	{
		Mesh bad = mesh;
		for ( int t = 0; t < bad.TriangleCount(); ++t )
			std::swap( bad.indices[3 * t + 1], bad.indices[3 * t + 2] );
		Topology topo = AnalyzeTopology( bad );
		Check( topo.Closed() && topo.volume.size() == 1 && topo.volume[0] < 0.0 &&
		           CountWindingViolations( bad ) == bad.TriangleCount(),
		    "control.inverted-surface", topo.Describe() );
	}
	// A dropped triangle opens three boundary edges and breaks the Euler number.
	{
		Mesh bad = mesh;
		bad.indices.erase( bad.indices.begin() + 3 * 5, bad.indices.begin() + 3 * 6 );
		Topology topo = AnalyzeTopology( bad );
		Check( topo.nBoundaryEdges == 3 && topo.euler[0] != 2, "control.dropped-triangle",
		    topo.Describe() );
	}
	// A duplicated triangle makes non-manifold edges.
	{
		Mesh bad = mesh;
		bad.indices.insert( bad.indices.end(), mesh.indices.begin(), mesh.indices.begin() + 3 );
		Topology topo = AnalyzeTopology( bad );
		Check( topo.nNonManifoldEdges == 3, "control.duplicated-triangle", topo.Describe() );
	}
	// A vertex moved radially off the surface by 2% of the radius.
	{
		Mesh bad = mesh;
		Vec3 p = bad.P( 7 );
		Vec3 moved = c + ( p - c ) * 1.02;
		bad.pos[7] = { { (float)moved.x, (float)moved.y, (float)moved.z } };
		Check( CountOffSphere( bad, c, R, POSITION_TOLERANCE * R, &flMax ) >= 1,
		    "control.off-surface-position" );
		Check( CountFieldViolations( tiler, bad, FIELD_TOLERANCE, &flMax ) >= 1,
		    "control.off-surface-field" );
		Check( CheckTetrahedra( tiler, bad ).nOffEdge >= 1, "control.off-surface-edge" );
	}
	// A seam crack: one copy of a duplicated seam vertex moved by one ulp.
	{
		int nWelded = 0;
		std::vector<int> welded = Weld( mesh, &nWelded );
		std::vector<int> copies( nWelded, 0 );
		for ( int id : welded )
			++copies[id];
		int nVictim = -1;
		for ( size_t i = 0; i < welded.size() && nVictim < 0; ++i )
		{
			if ( copies[welded[i]] > 1 )
				nVictim = (int)i;
		}
		Check( nVictim >= 0, "control.seam-crack-has-duplicate" );
		if ( nVictim >= 0 )
		{
			Mesh bad = mesh;
			bad.pos[nVictim][0] = std::nextafter( bad.pos[nVictim][0], 1e30f );
			Topology topo = AnalyzeTopology( bad );
			Check(
			    topo.nBoundaryEdges > 0 && !topo.Closed(), "control.seam-crack", topo.Describe() );
		}
	}
	// A flipped vertex normal.
	{
		Mesh bad = mesh;
		for ( int k = 0; k < 3; ++k )
			bad.normal[3][k] = -bad.normal[3][k];
		Check( CountBadSphereNormals( bad, c ) == 1, "control.flipped-normal" );
		Check( CountWindingViolations( bad ) >= 1, "control.flipped-normal-winding" );
	}
	// A tangent frame with a non-unit tangent.
	{
		Mesh bad = mesh;
		for ( int k = 0; k < 3; ++k )
			bad.tangentS[2][k] *= 0.99f;
		Check( CountBadTangentFrames( bad ) == 1, "control.non-unit-tangent" );
	}
	// A second, disjoint copy of the sphere is a second component.
	{
		Mesh bad = mesh;
		int nOffset = (int)bad.pos.size();
		for ( size_t i = 0; i < mesh.pos.size(); ++i )
		{
			Float3 p = mesh.pos[i];
			p[0] += 1000.0f;
			bad.pos.push_back( p );
			bad.normal.push_back( mesh.normal[i] );
		}
		for ( int nIndex : mesh.indices )
			bad.indices.push_back( nIndex + nOffset );
		Topology topo = AnalyzeTopology( bad );
		Check( topo.nComponents == 2 && topo.euler.size() == 2 && topo.euler[1] == 2,
		    "control.second-component", topo.Describe() );
	}
	// Out-of-range and missing indices.
	{
		Mesh bad = mesh;
		bad.indices[4] = (int)bad.pos.size();
		Check( !IndicesValid( bad ), "control.index-out-of-range" );
		bad = mesh;
		bad.indices.pop_back();
		Check( !IndicesValid( bad ), "control.index-count" );
	}
	// A bitwise difference in one normal breaks equality.
	{
		Mesh bad = mesh;
		bad.normal[0][1] = std::nextafter( bad.normal[0][1], 2.0f );
		Check( !BitwiseEqual( bad, mesh ), "control.bitwise-compare" );
	}
}

} // namespace

int main()
{
	TestLoneParticles();
	TestSeams();
	TestTwoParticles();
	TestCluster();
	TestDeterminism();
	TestFactories();
	TestNegativeControls();
	return testing::ReportConformance( g_nChecks, g_nFailures );
}
