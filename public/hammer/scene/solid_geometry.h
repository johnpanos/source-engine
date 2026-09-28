//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Geometry of document solids (RFC 0002, hammer.scene): the convex
//			polygons a solid's sides bound, bounds, and the rules that turn
//			planes and polygons back into authored side points. Every consumer
//			(viewport, picking, operations, codecs) builds solid geometry here,
//			so one tolerance policy applies (mapgeometry's).
//
//=============================================================================//

#ifndef HAMMER_SCENE_SOLID_GEOMETRY_H
#define HAMMER_SCENE_SOLID_GEOMETRY_H

#include "hammer/scene/map_objects.h"

#include <array>
#include <optional>
#include <vector>

namespace hammer::scene
{

// A double-precision axis-aligned box.
struct Box
{
	mapgeometry::Vec3d mins;
	mapgeometry::Vec3d maxs;

	mapgeometry::Vec3d Center() const;
	mapgeometry::Vec3d Size() const;
	bool Contains( const mapgeometry::Vec3d &p, double epsilon = 0.0 ) const;
	// True when the boxes overlap (touching counts when 'epsilon' >= 0).
	bool Intersects( const Box &other, double epsilon = 0.0 ) const;
	// True when 'other' lies inside this box.
	bool Encloses( const Box &other, double epsilon = 0.0 ) const;
	void Extend( const mapgeometry::Vec3d &p );
	void Extend( const Box &other );

	friend bool operator==( const Box &, const Box & ) = default;
};

// A box around a single point.
Box PointBox( const mapgeometry::Vec3d &p );

// The solid's face polygons. Each face's 'sourcePlane' is the index of the side
// it came from and 'material' that side's material. Sides that bound no face
// (redundant planes) produce none; a solid whose sides bound no closed volume
// produces no faces at all. Vertex coordinates within
// mapgeometry::kIntegerSnapEpsilon of an integer are snapped to it.
mapgeometry::BrushSolid BuildGeometry( const Solid &solid );

// Bounds of the solid's vertices; nothing when it has no faces.
std::optional<Box> SolidBounds( const Solid &solid );

// Where a ray enters a solid: the ray parameter (in 'direction' units), the
// index into Solid::sides of the side it enters, the point and that side's
// outward normal. Nothing when it misses or starts inside.
struct SolidRayHit
{
	double t = 0.0;
	std::size_t side = 0;
	mapgeometry::Vec3d point;
	mapgeometry::Vec3d normal;
};
std::optional<SolidRayHit> RayEnterSolid(
    const Solid &solid, const mapgeometry::Vec3d &origin, const mapgeometry::Vec3d &direction );

// The four corners of side 'sideIndex''s face, in winding order; nothing when
// the face does not have exactly four vertices.
std::optional<std::array<mapgeometry::Vec3d, 4>> QuadCorners(
    const Solid &solid, std::size_t sideIndex );

// Authored side points for a face polygon given counter-clockwise as seen from
// outside (BrushFace order): three of its vertices, ordered so Side::Plane()
// reproduces the polygon's outward plane. Requires three or more vertices.
std::array<mapgeometry::Vec3d, 3> PointsFromPolygon( const std::vector<mapgeometry::Vec3d> &ccw );

// Three points on 'plane' whose Side::Plane() is 'plane' (normal outward).
std::array<mapgeometry::Vec3d, 3> PointsFromPlane( const mapgeometry::Plane &plane );

// Re-derives every side's points from the face it bounds and drops sides that
// bound no face. Returns nothing (and leaves the solid untouched) when the
// sides do not bound a closed solid with at least four faces.
std::optional<Solid> NormalizeSides( const Solid &solid );

// An axis-aligned box solid whose sides carry 'texture' (with world-aligned
// axes per face; see mapgeometry::WorldAlignedTextureAxes). Side VMF ids are
// zero (allocated when the solid is added to a document).
Solid MakeBoxSolid( const Box &box, const FaceTexture &texture );

// World-aligned texture axes (legacy rule) for a face with 'normal', keeping
// 'texture's material, scales and lightmap scale; shifts are reset to zero.
FaceTexture WorldAlignedTexture( const FaceTexture &texture, const mapgeometry::Vec3d &normal );

} // namespace hammer::scene

#endif // HAMMER_SCENE_SOLID_GEOMETRY_H
