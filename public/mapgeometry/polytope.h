//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Convex polytope predicates and constructions for brush editing
//			(RFC 0002, world.map-geometry): which side of a plane a point set
//			lies on, point containment, and the planes of the convex hull of a
//			point set (the vertex tool's rebuild). Solids themselves are built
//			by BuildSolidFromPlanes (brush.h); these helpers never allocate a
//			solid and state their tolerances.
//
//=============================================================================//

#ifndef MAPGEOMETRY_POLYTOPE_H
#define MAPGEOMETRY_POLYTOPE_H

#include "mapgeometry/brush.h"

#include <optional>
#include <vector>

namespace mapgeometry
{

// Distance within which a point counts as lying on a plane.
constexpr double kPlaneEpsilon = 0.01;

enum class PlaneSide
{
	Front,    // every point is outside (positive) or on the plane, at least one outside
	Back,     // every point is inside (negative) or on the plane, at least one inside
	On,       // every point is on the plane (or the set is empty)
	Spanning, // points on both sides
};

PlaneSide ClassifyPoints( const std::vector<Vec3d> &points, const Plane &plane,
    double epsilon = kPlaneEpsilon );

// Every vertex of every face of 'solid'.
std::vector<Vec3d> SolidVertices( const BrushSolid &solid );

// The plane with the opposite orientation (its outside is the original inside).
Plane Flipped( const Plane &plane );

// The plane through 'point' with unit outward 'normal' (normalized here).
Plane PlaneThrough( const Vec3d &point, const Vec3d &normal );

// True when 'p' is inside or on every plane (within 'epsilon').
bool InsideAll( const std::vector<Plane> &planes, const Vec3d &p, double epsilon = kPlaneEpsilon );

// True when the planes bound a closed solid with at least four faces inside
// the map coordinate range (+/-65536); an open plane set is not closed.
bool IsClosedSolid( const std::vector<Plane> &planes );

// The outward planes of the convex hull of 'points', one per hull face (coplanar
// hull triangles merge into one plane). Nothing when the points do not span a
// volume (fewer than four points, or all coplanar within 'epsilon').
std::optional<std::vector<Plane>> ConvexHullPlanes(
    const std::vector<Vec3d> &points, double epsilon = kPlaneEpsilon );

// Distance within which a computed vertex coordinate is snapped to the nearest
// integer: clipping large face quads leaves ~1e-11 noise on exact corners.
constexpr double kIntegerSnapEpsilon = 1.0e-4;

// Snaps every face vertex coordinate within 'epsilon' of an integer to it and
// recomputes the solid's bounds. Face planes are left as built.
void SnapNearIntegers( BrushSolid &solid, double epsilon = kIntegerSnapEpsilon );

// True when two convex solids' interiors overlap by more than 'epsilon': no
// face plane of either separates them. (Separating-axis test on face normals
// and edge cross products.)
bool SolidsOverlap( const BrushSolid &a, const BrushSolid &b, double epsilon = kPlaneEpsilon );

} // namespace mapgeometry

#endif // MAPGEOMETRY_POLYTOPE_H
