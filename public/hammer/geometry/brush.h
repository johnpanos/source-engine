//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Convex brush geometry for the strict Hammer geometry module (RFC 0002,
//			hammer.geometry). A Source VMF stores each brush "solid" as a set of
//			"side" planes, each plane written as three points. The renderable form
//			of a brush is the convex polyhedron that is the intersection of the
//			half-spaces behind those planes; this module derives that polyhedron
//			(per-face polygons with outward normals) from the parsed keyvalues tree.
//
//			This is the headless VMF -> scene geometry bridge the GTK desktop shell
//			and any viewport consume. It is dependency-free apart from the C++
//			standard library and hammer.formats: no MFC, tier0, platform.h, PCH, or
//			GPU. Geometry is computed in double precision for clip robustness and
//			exposed as float for GPU upload.
//
//			Robustness policy: the outward orientation of each side plane is derived
//			from a point known to be interior to the brush (the centroid of the
//			side reference points), NOT from the VMF point winding. A brush whose
//			planes do not bound a finite convex region yields no faces rather than
//			an unbounded polygon.
//
//=============================================================================//

#ifndef HAMMER_GEOMETRY_BRUSH_H
#define HAMMER_GEOMETRY_BRUSH_H

#include "hammer/formats/keyvalues.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace hammer::geometry
{

// Double-precision 3-vector used for brush construction. Kept distinct from the
// float Vec3 in aabb.h: that type mirrors legacy BoundBox float semantics, while
// brush clipping needs double precision to stay robust against large Source map
// coordinates. Conversion to render-ready float happens at the GPU boundary.
struct Vec3d
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	constexpr Vec3d() = default;
	constexpr Vec3d( double in_x, double in_y, double in_z ) : x( in_x ), y( in_y ), z( in_z ) {}
};

// A plane in the form dot(normal, p) = dist, with 'normal' unit length. The
// interior half-space of the brush is dot(normal, p) <= dist (normal points out).
struct Plane
{
	Vec3d normal;
	double dist = 0.0;
};

// One face of a brush: a convex, outward-wound polygon lying on 'plane'. 'material'
// is the VMF side material (empty when absent). Vertices are ordered
// counter-clockwise as seen from outside (looking down -normal).
struct BrushFace
{
	Plane plane;
	std::vector<Vec3d> vertices;
	std::string material;
};

// One brush solid: its faces plus the VMF id it came from (0 when absent).
struct BrushSolid
{
	int id = 0;
	std::vector<BrushFace> faces;

	// Axis-aligned bounds over all face vertices. Only meaningful when non-empty.
	Vec3d mins;
	Vec3d maxs;
	bool bounded = false;
};

// A point/brush entity summarised for the shell's outliner. 'origin' is parsed
// from the "origin" key when present. Brush entities also contribute their solids
// to WorldScene::solids, so this list is for identification, not geometry.
struct SceneEntity
{
	std::string classname;
	std::string targetname;
	std::optional<Vec3d> origin;
	std::size_t solidCount = 0;
};

// The renderable result of importing a VMF document: every world and brush-entity
// solid resolved to polygons, plus a light entity summary and overall bounds.
struct WorldScene
{
	std::vector<BrushSolid> solids;
	std::vector<SceneEntity> entities;

	Vec3d mins;
	Vec3d maxs;
	bool bounded = false;

	std::size_t TotalFaces() const;
	std::size_t TotalVertices() const;
};

// Parses a VMF "plane" value: exactly three parenthesised points
//   (x1 y1 z1) (x2 y2 z2) (x3 y3 z3)
// Returns the three points on success. Whitespace between tokens is flexible;
// malformed input (wrong point count, non-numeric component, unbalanced parens)
// returns nullopt without throwing.
std::optional<std::array<Vec3d, 3>> ParsePlanePoints( const std::string &value );

// Builds a plane from three points. The normal is derived from the winding here;
// callers that need a guaranteed outward orientation use BuildSolidFromPlanes,
// which reorients every plane against the brush interior. Returns nullopt when the
// points are collinear (degenerate plane).
std::optional<Plane> PlaneFromPoints( const Vec3d &a, const Vec3d &b, const Vec3d &c );

// Intersects the half-spaces of 'planes' into a convex solid. Each plane is first
// reoriented so its normal points away from the interior centroid, then each
// face polygon is produced by clipping a large quad on that plane against all the
// others. Planes that contribute no face (redundant/degenerate) are dropped. The
// 'materials' vector, when the same length as 'planes', names each face material.
// Returns a solid with no faces when the planes do not bound a finite region.
BrushSolid BuildSolidFromPlanes( const std::vector<Plane> &planes,
                                 const std::vector<std::string> &materials = {}, int id = 0 );

// Extracts a single "solid" keyvalues block into a BrushSolid (parsing its side
// planes and materials). Returns a solid with no faces when the block has fewer
// than four valid side planes or does not bound a finite region.
BrushSolid BuildSolidFromBlock( const formats::KeyValueNode &solidBlock );

// Imports a parsed VMF document (the root whose children are the top-level
// blocks) into a renderable WorldScene: every solid under "world" and under brush
// entities is resolved; entities are summarised. Never throws.
WorldScene BuildSceneFromDocument( const formats::KeyValueNode &root );

} // namespace hammer::geometry

#endif // HAMMER_GEOMETRY_BRUSH_H
