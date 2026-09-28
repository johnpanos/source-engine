//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Vertex and face editing of convex solids (RFC 0002, hammer.app;
//			the Vertex tool and Source 2's face push/extrude). A solid is edited
//			through its vertex set: moving vertices rebuilds the solid as the
//			convex hull of the new set. A move that would make the solid
//			concave (a moved vertex falls inside the hull) or degenerate is
//			refused, never silently repaired.
//
//			Faces of the rebuilt solid keep the texture and persistent id of the
//			old side with the same plane; a face whose plane changed takes the
//			texture of the old side whose normal is closest (legacy morph keeps
//			face textures as faces move).
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_VERTEX_OPS_H
#define HAMMER_APP_OPS_VERTEX_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"

#include <vector>

namespace hammer::app::ops
{

// The solid's unique vertices, in a stable order (face order, first seen).
std::vector<mapgeometry::Vec3d> SolidVertexList( const scene::Solid &solid );

// The solid's unique edges as vertex index pairs into SolidVertexList.
std::vector<std::pair<int, int>> SolidEdgeList( const scene::Solid &solid );

// The solid rebuilt around 'points' (their convex hull), inheriting textures
// and side ids from 'original'. Nothing when the points do not span a volume
// or some point is not a corner of the hull (inside it, or inside one of its
// faces or edges): the result would not keep that vertex.
std::optional<scene::Solid> RebuildFromVertices(
    const scene::Solid &original, const std::vector<mapgeometry::Vec3d> &points );

// Moves the vertices at 'indices' (into SolidVertexList) by 'delta'.
EditResult MoveVertices( scene::DocumentEdit &edit, scene::ObjectId solid,
    const std::vector<int> &indices, const mapgeometry::Vec3d &delta );

// Moves a face along its outward normal by 'distance' (negative pulls it in),
// keeping the other faces' planes (a push/pull). The face must survive.
EditResult PushFace( scene::DocumentEdit &edit, const scene::FaceRef &face, double distance );

// Source 2 face extrude: a new solid swept from the face polygon along its
// outward normal by 'distance' (> 0), in the face solid's owner and group, with
// the face's texture on every side. 'created' receives it.
EditResult ExtrudeFace( scene::DocumentEdit &edit, const scene::FaceRef &face, double distance,
    scene::ObjectId &created );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_VERTEX_OPS_H
