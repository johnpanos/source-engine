//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/vertex_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/vertex_ops.h"

#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace hammer::app::ops
{

using mapgeometry::Vec3d;

namespace
{

constexpr double kVertexEpsilon = 1.0e-4;

int IndexOf( const std::vector<Vec3d> &list, const Vec3d &v )
{
	for ( std::size_t i = 0; i < list.size(); ++i )
	{
		if ( mapgeometry::NearlyEqual( list[i], v, kVertexEpsilon ) )
		{
			return static_cast<int>( i );
		}
	}
	return -1;
}

} // namespace

std::vector<Vec3d> SolidVertexList( const scene::Solid &solid )
{
	std::vector<Vec3d> out;
	for ( const mapgeometry::BrushFace &face : scene::BuildGeometry( solid ).faces )
	{
		for ( const Vec3d &v : face.vertices )
		{
			if ( IndexOf( out, v ) < 0 )
			{
				out.push_back( v );
			}
		}
	}
	return out;
}

std::vector<std::pair<int, int>> SolidEdgeList( const scene::Solid &solid )
{
	const std::vector<Vec3d> vertices = SolidVertexList( solid );
	std::set<std::pair<int, int>> edges;
	for ( const mapgeometry::BrushFace &face : scene::BuildGeometry( solid ).faces )
	{
		for ( std::size_t i = 0; i < face.vertices.size(); ++i )
		{
			int a = IndexOf( vertices, face.vertices[i] );
			int b = IndexOf( vertices, face.vertices[( i + 1 ) % face.vertices.size()] );
			if ( a > b )
			{
				std::swap( a, b );
			}
			if ( a >= 0 && a != b )
			{
				edges.insert( { a, b } );
			}
		}
	}
	return std::vector<std::pair<int, int>>( edges.begin(), edges.end() );
}

std::optional<scene::Solid> RebuildFromVertices(
    const scene::Solid &original, const std::vector<Vec3d> &points )
{
	const std::optional<std::vector<mapgeometry::Plane>> hull =
	    mapgeometry::ConvexHullPlanes( points );
	if ( !hull )
	{
		return std::nullopt;
	}
	// Every point must be a corner of the hull: on at least three hull planes
	// whose normals span 3D. A point strictly inside, or inside a face or on an
	// edge, would be absorbed, so the requested shape is not a convex solid
	// with those vertices and the move is refused.
	for ( const Vec3d &p : points )
	{
		std::vector<Vec3d> normals;
		for ( const mapgeometry::Plane &plane : *hull )
		{
			if ( std::fabs( mapgeometry::PlaneDistance( plane, p ) ) <= mapgeometry::kPlaneEpsilon )
			{
				normals.push_back( plane.normal );
			}
		}
		bool corner = false;
		for ( std::size_t i = 0; i < normals.size() && !corner; ++i )
		{
			for ( std::size_t j = i + 1; j < normals.size() && !corner; ++j )
			{
				for ( std::size_t k = j + 1; k < normals.size() && !corner; ++k )
				{
					corner = std::fabs( mapgeometry::Dot( normals[i],
					             mapgeometry::Cross( normals[j], normals[k] ) ) ) > 1.0e-6;
				}
			}
		}
		if ( !corner )
		{
			return std::nullopt;
		}
	}

	std::vector<mapgeometry::Plane> oldPlanes;
	for ( const scene::Side &side : original.sides )
	{
		oldPlanes.push_back( side.Plane() );
	}
	scene::Solid out = original;
	out.sides.clear();
	std::set<std::uint32_t> usedIds;
	for ( const mapgeometry::Plane &plane : *hull )
	{
		// Same plane: keep the side's data and id. Otherwise the closest normal
		// lends its texture, and the side gets a fresh id.
		int same = -1;
		int closest = 0;
		double best = -2.0;
		for ( std::size_t i = 0; i < oldPlanes.size(); ++i )
		{
			const double dot = mapgeometry::Dot( oldPlanes[i].normal, plane.normal );
			if ( dot > 0.9999 &&
			     std::fabs( oldPlanes[i].dist - plane.dist ) < mapgeometry::kPlaneEpsilon )
			{
				same = static_cast<int>( i );
			}
			if ( dot > best )
			{
				best = dot;
				closest = static_cast<int>( i );
			}
		}
		scene::Side side;
		if ( same >= 0 && usedIds.insert( original.sides[same].vmfId ).second )
		{
			side = original.sides[same];
		}
		else
		{
			// A changed face: its texture, not its displacement data or id.
			side.texture = original.sides[closest].texture;
		}
		side.points = scene::PointsFromPlane( plane );
		out.sides.push_back( std::move( side ) );
	}
	return scene::NormalizeSides( out );
}

EditResult MoveVertices( scene::DocumentEdit &edit, scene::ObjectId solidId,
    const std::vector<int> &indices, const Vec3d &delta )
{
	const scene::Solid *solid = edit.FindSolid( solidId );
	if ( !solid )
	{
		return Reject( "not a solid" );
	}
	if ( indices.empty() || delta == Vec3d() )
	{
		return NothingToDo( "no vertices to move" );
	}
	std::vector<Vec3d> points = SolidVertexList( *solid );
	for ( int i : indices )
	{
		if ( i < 0 || i >= static_cast<int>( points.size() ) )
		{
			return Reject( "vertex index out of range" );
		}
	}
	const std::set<int> chosen( indices.begin(), indices.end() );
	for ( int i : chosen )
	{
		points[i] += delta;
	}
	// Merging two vertices is legal (the hull drops the duplicate), so dedupe.
	std::vector<Vec3d> unique;
	for ( const Vec3d &p : points )
	{
		if ( IndexOf( unique, p ) < 0 )
		{
			unique.push_back( p );
		}
	}
	std::optional<scene::Solid> rebuilt = RebuildFromVertices( *solid, unique );
	if ( !rebuilt )
	{
		return Reject( "the move would make the solid concave or flat" );
	}
	for ( scene::Side &side : rebuilt->sides )
	{
		if ( side.vmfId == 0 )
		{
			side.vmfId = edit.AllocateVmfId();
		}
	}
	*edit.MutableSolid( solidId ) = std::move( *rebuilt );
	return {};
}

EditResult PushFace( scene::DocumentEdit &edit, const scene::FaceRef &face, double distance )
{
	const scene::Solid *solid = edit.FindSolid( face.solid );
	const scene::Side *side = solid ? solid->FindSide( face.side ) : nullptr;
	if ( !side )
	{
		return Reject( "unknown face" );
	}
	if ( distance == 0.0 )
	{
		return NothingToDo( "zero distance" );
	}
	scene::Solid moved = *solid;
	scene::Side *target = moved.FindSide( face.side );
	const Vec3d n = side->Plane().normal;
	for ( Vec3d &p : target->points )
	{
		p += n * distance;
	}
	std::optional<scene::Solid> normalized = scene::NormalizeSides( moved );
	if ( !normalized || !normalized->FindSide( face.side ) )
	{
		return Reject( "the face cannot move that far" );
	}
	// Faces that vanished leave; the others keep their ids.
	*edit.MutableSolid( face.solid ) = std::move( *normalized );
	return {};
}

EditResult ExtrudeFace( scene::DocumentEdit &edit, const scene::FaceRef &face, double distance,
    scene::ObjectId &created )
{
	const scene::Solid *solid = edit.FindSolid( face.solid );
	const scene::Side *side = solid ? solid->FindSide( face.side ) : nullptr;
	if ( !side )
	{
		return Reject( "unknown face" );
	}
	if ( distance <= 0.0 )
	{
		return Reject( "extrude distance must be positive" );
	}
	std::vector<Vec3d> polygon;
	for ( const mapgeometry::BrushFace &f : scene::BuildGeometry( *solid ).faces )
	{
		if ( solid->sides[static_cast<std::size_t>( f.sourcePlane )].vmfId == face.side )
		{
			polygon = f.vertices;
		}
	}
	if ( polygon.size() < 3 )
	{
		return Reject( "the face has no polygon" );
	}
	const Vec3d n = side->Plane().normal;
	std::vector<Vec3d> points = polygon;
	for ( const Vec3d &p : polygon )
	{
		points.push_back( p + n * distance );
	}
	scene::Solid shell;
	shell.owner = solid->owner;
	shell.group = solid->group;
	shell.editor = solid->editor;
	scene::Side template_;
	template_.texture = side->texture;
	shell.sides.push_back( template_ );
	std::optional<scene::Solid> prism = RebuildFromVertices( shell, points );
	if ( !prism )
	{
		return Reject( "the extrusion is degenerate" );
	}
	for ( scene::Side &s : prism->sides )
	{
		s.vmfId = 0;
	}
	prism->vmfId = 0;
	created = edit.Add( std::move( *prism ) );
	return {};
}

} // namespace hammer::app::ops
