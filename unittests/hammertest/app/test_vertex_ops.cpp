//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app vertex and face editing (RFC 0002, R08 domain logic):
//			vertex and edge lists, moving a vertex (a sheared box), moving an
//			edge (a wedge), merging vertices, face push/pull keeping side ids,
//			and face extrusion into a new solid. Unchanged faces keep their
//			side ids and textures. Negative checks: a move that makes the solid
//			concave or flat refuses with nothing staged, bad indices, unknown
//			faces, pushing a face through the solid, non-positive extrusion.
//
//=============================================================================//

#include "hammer/app/ops/vertex_ops.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;

namespace
{

int Find( const std::vector<Vec3d> &list, const Vec3d &v )
{
	for ( std::size_t i = 0; i < list.size(); ++i )
		if ( list[i] == v )
			return static_cast<int>( i );
	return -1;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::MapDocument doc;
	scene::ObjectId box;
	{
		scene::DocumentEdit edit( doc );
		box = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) }, tex ) );
		scene::CommitEdit( doc, edit );
	}
	const scene::Solid &original = *doc.FindSolid( box );
	const std::vector<Vec3d> verts = SolidVertexList( original );
	checks.Equal( verts.size(), std::size_t( 8 ), "a box has 8 vertices" );
	checks.Equal( SolidEdgeList( original ).size(), std::size_t( 12 ), "a box has 12 edges" );
	const std::uint32_t bottomId = original.sides[1].vmfId; // -Z

	// Move one top corner outward along X: the top and +X faces tilt.
	{
		scene::DocumentEdit edit( doc );
		const int corner = Find( verts, Vec3d( 64, 64, 64 ) );
		checks.That(
		    MoveVertices( edit, box, { corner }, Vec3d( 16, 0, 0 ) ).HasValue(), "move a vertex" );
		const scene::Solid &s = *edit.FindSolid( box );
		const std::vector<Vec3d> moved = SolidVertexList( s );
		checks.That(
		    Find( moved, Vec3d( 80, 64, 64 ) ) >= 0 && Find( moved, Vec3d( 64, 64, 64 ) ) < 0,
		    "the vertex moved" );
		checks.That( s.FindSide( bottomId ) != nullptr, "an unchanged face keeps its side id" );
		checks.That( scene::ValidateEdit( edit ).empty(), "the result validates" );
		bool textured = true;
		for ( const scene::Side &side : s.sides )
			textured = textured && side.texture.material == tex.material;
		checks.That( textured, "tilted faces keep a texture" );
	}

	// Move the top +X edge up: a convex house shape.
	{
		scene::DocumentEdit edit( doc );
		const int a = Find( verts, Vec3d( 64, 0, 64 ) );
		const int b = Find( verts, Vec3d( 64, 64, 64 ) );
		checks.That(
		    MoveVertices( edit, box, { a, b }, Vec3d( -32, 0, 32 ) ).HasValue(), "move an edge" );
		checks.Equal( SolidVertexList( *edit.FindSolid( box ) ).size(), std::size_t( 8 ),
		    "still eight vertices" );
	}

	// Merge: collapse the top face to a line -> a wedge-like prism.
	{
		scene::DocumentEdit edit( doc );
		const int a = Find( verts, Vec3d( 64, 0, 64 ) );
		const int b = Find( verts, Vec3d( 64, 64, 64 ) );
		checks.That(
		    MoveVertices( edit, box, { a, b }, Vec3d( -64, 0, 0 ) ).HasValue(), "merge vertices" );
		const scene::Solid &s = *edit.FindSolid( box );
		checks.Equal( SolidVertexList( s ).size(), std::size_t( 6 ), "a wedge has six vertices" );
		checks.Equal( s.sides.size(), std::size_t( 5 ), "and five faces" );
	}

	// Negative: pushing a corner inward makes it concave.
	{
		scene::DocumentEdit edit( doc );
		const int corner = Find( verts, Vec3d( 64, 64, 64 ) );
		checks.That(
		    !MoveVertices( edit, box, { corner }, Vec3d( -32, -32, -32 ) ) && edit.Finish().Empty(),
		    "a concave move refuses with nothing staged (negative)" );
		// Onto the middle of the bottom face: coplanar, not a corner.
		checks.That(
		    !MoveVertices( edit, box, { Find( verts, Vec3d( 0, 0, 0 ) ) }, Vec3d( 32, 32, 0 ) ),
		    "a vertex moved inside a face is refused, not absorbed (negative)" );
		checks.That( !MoveVertices( edit, box, { 42 }, Vec3d( 1, 0, 0 ) ), "bad index (negative)" );
		checks.That( MoveVertices( edit, box, {}, Vec3d( 1, 0, 0 ) ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "no vertices (negative)" );
		// Flatten: move the whole top down to the bottom.
		std::vector<int> top;
		for ( std::size_t i = 0; i < verts.size(); ++i )
			if ( verts[i].z == 64 )
				top.push_back( static_cast<int>( i ) );
		checks.That( !MoveVertices( edit, box, top, Vec3d( 0, 0, -64 ) ),
		    "a flat result refuses (negative)" );
	}

	// Push/pull a face.
	{
		scene::DocumentEdit edit( doc );
		const scene::FaceRef top{ box, original.sides[0].vmfId };
		checks.That( PushFace( edit, top, 32 ).HasValue(), "push the top up" );
		checks.That( scene::SolidBounds( *edit.FindSolid( box ) )->maxs.z == 96, "taller" );
		checks.That( edit.FindSolid( box )->FindSide( top.side ) != nullptr,
		    "the pushed face keeps its id" );
		checks.That( PushFace( edit, top, -64 ).HasValue(), "pull it down" );
		checks.That( scene::SolidBounds( *edit.FindSolid( box ) )->maxs.z == 32, "shorter" );
		checks.That( !PushFace( edit, top, -64 ), "pulling through the solid refuses (negative)" );
		checks.That( !PushFace( edit, { box, 99999 }, 8 ), "unknown face (negative)" );
	}

	// Extrude a face into a new solid.
	{
		scene::DocumentEdit edit( doc );
		const scene::FaceRef side{ box, original.sides[3].vmfId }; // +X
		scene::ObjectId created;
		checks.That( ExtrudeFace( edit, side, 32, created ).HasValue(), "extrude" );
		const std::optional<scene::Box> b = scene::SolidBounds( *edit.FindSolid( created ) );
		checks.That( b && b->mins == Vec3d( 64, 0, 0 ) && b->maxs == Vec3d( 96, 64, 64 ),
		    "extruded prism bounds" );
		checks.That( scene::ValidateEdit( edit ).empty(), "the extrusion validates" );
		checks.That( *edit.FindSolid( box ) == original, "the source solid is untouched" );
		checks.That( !ExtrudeFace( edit, side, 0, created ), "zero distance (negative)" );
	}

	return checks.Report();
}
