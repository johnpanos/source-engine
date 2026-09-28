//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Constructive solid operations on convex solids (RFC 0002,
//			hammer.app): clipping by a plane (the Clip tool), carving (subtract
//			one set of solids from others), and hollowing (legacy "Make
//			Hollow": carve each solid by its inward-offset copy and group the
//			walls). All results are convex solids built from the originals'
//			sides plus cap sides, so untouched faces keep their textures and
//			persistent side ids.
//
//			Piece identity: the first piece of a split solid keeps the solid's
//			id (so selections and references follow it); further pieces are new
//			solids with the same owner, group and editor data and fresh side
//			ids.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_CSG_OPS_H
#define HAMMER_APP_OPS_CSG_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"

#include <vector>

namespace hammer::app::ops
{

enum class ClipKeep
{
	Front, // the part on the plane's outside (its normal points into it)
	Back,  // the part behind the plane
	Both,  // split into two solids
};

// Clips every solid the ids stand for that spans 'plane'; solids entirely on
// one side are left alone. New cap faces get 'capTexture's material and
// scales with world-aligned axes. 'created' receives new solid ids.
EditResult ClipSolids( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const mapgeometry::Plane &plane, ClipKeep keep, const scene::FaceTexture &capTexture,
    std::vector<scene::ObjectId> *created = nullptr );

// Subtracts the solids 'carvers' stand for from the solids 'targets' stand for
// (carvers themselves are never carved). A target that does not overlap any
// carver is untouched. New faces take the carving face's texture. Nothing
// when no target overlaps.
EditResult Carve( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &carvers,
    const std::vector<scene::ObjectId> &targets, std::vector<scene::ObjectId> *created = nullptr );

// Replaces each solid with walls of 'thickness' (negative: walls grow outward)
// grouped together, inside the solid's previous group. 'groups' receives the
// new groups. Refuses when a solid is too thin for the walls.
EditResult Hollow( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    double thickness, std::vector<scene::ObjectId> *groups = nullptr );

// The pieces of 'solid' split by 'plane': [back, front], each present when that
// side has a closed piece. Exposed for tools that preview a clip.
struct SplitResult
{
	std::optional<scene::Solid> back;
	std::optional<scene::Solid> front;
};
SplitResult SplitSolid( const scene::Solid &solid, const mapgeometry::Plane &plane,
    const scene::FaceTexture &capTexture );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_CSG_OPS_H
