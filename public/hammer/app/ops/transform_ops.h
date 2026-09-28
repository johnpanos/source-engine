//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Geometric transforms of map objects (RFC 0002, hammer.app;
//			"Transform operations declare supported node kinds and validate the
//			complete selection before committing"). One entry point applies an
//			affine map to what the ids stand for:
//
//			  * solids: every side's authored points, the texture (locked or
//			    not, per TransformOptions), and displacement data (start
//			    position, normals and offsets);
//			  * entities: "origin", and orientation ("angles", or the legacy
//			    yaw-only "angle") composed with the map's rotation part;
//			  * groups: their members.
//
//			A map that would make any solid degenerate refuses the whole
//			operation. Mirrors reverse face winding, so side points are
//			reordered to keep planes pointing out. Convenience operations
//			(translate, rotate, scale to a box, mirror, snap, align) build the
//			map and call the same entry point.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_TRANSFORM_OPS_H
#define HAMMER_APP_OPS_TRANSFORM_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/transform.h"

#include <vector>

namespace hammer::app::ops
{

struct TransformOptions
{
	bool textureLock = true;
};

// Applies 'xf' to every object 'ids' stand for.
EditResult TransformObjects( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const mapgeometry::Affine &xf, const TransformOptions &options = {} );

EditResult Translate( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const mapgeometry::Vec3d &delta, const TransformOptions &options = {} );

// Rotates 'degrees' about world axis 0/1/2 through 'pivot'.
EditResult Rotate( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, int axis,
    double degrees, const mapgeometry::Vec3d &pivot, const TransformOptions &options = {} );

// Maps box 'from' onto box 'to' per axis (the 2D scale handles). A 'to' with a
// zero or negative extent where 'from' has one refuses.
EditResult ScaleToBox( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const scene::Box &from, const scene::Box &to, const TransformOptions &options = {} );

// Mirrors across the plane through 'pivot' normal to world axis 0/1/2.
EditResult Mirror( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, int axis,
    const mapgeometry::Vec3d &pivot, const TransformOptions &options = {} );

// Moves the objects so the minimum corner of their combined bounds lands on
// the grid (legacy "snap selection to grid"). Nothing when already aligned.
EditResult SnapToGrid( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    double grid, const TransformOptions &options = {} );

// Moves each top-level object along world axis 0/1/2 so its minimum (or
// maximum) face lines up with the combined bounds' minimum (or maximum), the
// legacy "align objects" commands.
EditResult AlignObjects( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    int axis, bool toMaximum, const TransformOptions &options = {} );

// The solid after 'xf' (points, texture, displacement), or nothing when the
// result is degenerate. Exposed for tools that preview a transform.
std::optional<scene::Solid> TransformedSolid(
    const scene::Solid &solid, const mapgeometry::Affine &xf, const TransformOptions &options );
// The entity after 'xf' (origin and orientation keys; for an info_overlay
// also its basis and uv corners, see TransformedOverlay).
scene::Entity TransformedEntity( const scene::Entity &entity, const mapgeometry::Affine &xf );

// An info_overlay's basis after 'xf': the origin is mapped as a point; for a
// rigid map (rotation, mirror) the U, V and normal axes are mapped as vectors;
// otherwise the axes are kept (normalized) and each uv corner is mapped through
// the linear part and projected back onto them (legacy CMapOverlay::
// DoTransform). Other entities, and overlays with malformed basis keys, are
// returned unchanged.
scene::Entity TransformedOverlay( const scene::Entity &overlay, const mapgeometry::Affine &xf );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_TRANSFORM_OPS_H
