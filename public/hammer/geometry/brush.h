//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Convex brush geometry for the strict Hammer geometry module (RFC 0002,
//			hammer.geometry). A Source VMF stores each brush "solid" as a set of
//			"side" planes, each plane written as three points. The renderable form
//			of a brush is the convex polyhedron that is the intersection of the
//			half-spaces behind those planes; this module derives that polyhedron
//			(per-face polygons with outward normals) from those side planes.
//
//			This is the headless scene geometry the GTK desktop shell and any
//			viewport consume. It depends only on the C++ standard library: no
//			editor module, MFC, tier0, platform.h, PCH, or GPU. Decoding VMF
//			keyvalues blocks into these types is hammer.formats' job
//			(public/hammer/formats/vmf_geometry.h). Geometry is computed in double
//			precision for clip robustness and exposed as float for GPU upload.
//
//			Robustness policy: the outward orientation of each side plane is derived
//			from a point known to be interior to the brush (the centroid of the
//			polytope's own vertices), NOT from the VMF point winding. A brush whose
//			planes do not bound a finite convex region yields no faces rather than
//			an unbounded polygon.
//
//=============================================================================//

#ifndef HAMMER_GEOMETRY_BRUSH_H
#define HAMMER_GEOMETRY_BRUSH_H

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

// A displaced (dispinfo) surface reduced to a renderable triangle mesh: a
// row-major grid of world-space vertices, a per-vertex blend weight (0..255), and
// a triangle index list. This is the presentation form the scene carries; the
// subdivision math lives in hammer.geometry.displacement, and the VMF decoder
// (hammer.formats) fills this from it (kept here so WorldScene needs no dependency
// on that module's header).
struct DisplacementMesh
{
	std::vector<Vec3d> vertices;
	std::vector<double> alphas;
	std::vector<std::array<int, 3>> triangles;
};

// The renderable result of importing a VMF document: every world and brush-entity
// solid resolved to polygons, any displaced faces resolved to surfaces, plus a
// light entity summary and overall bounds.
struct WorldScene
{
	std::vector<BrushSolid> solids;
	std::vector<DisplacementMesh> displacements;
	std::vector<SceneEntity> entities;

	Vec3d mins;
	Vec3d maxs;
	bool bounded = false;

	std::size_t TotalFaces() const;
	std::size_t TotalVertices() const;

	// Appends 'solid' and, when it is bounded, grows the scene bounds to cover it.
	void AddSolid( BrushSolid solid );

	// Appends 'mesh' and grows the scene bounds to cover each of its vertices.
	void AddDisplacement( DisplacementMesh mesh );
};

// Parses three whitespace-separated numbers "x y z" (surrounding whitespace
// allowed). Returns nullopt for a missing, extra, or non-numeric component.
std::optional<Vec3d> ParseVec3( const std::string &text );

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
BrushSolid BuildSolidFromPlanes(
    const std::vector<Plane> &planes, const std::vector<std::string> &materials = {}, int id = 0 );

// Finds the built face of 'solid' that lies on 'plane' (the same plane up to
// normal sign): normals must be near-parallel (|dot| >= 0.999) and the offsets
// must agree within 0.5 units once the sign is reconciled. Among matches the
// most parallel wins (a later face wins a tie). Returns nullptr when none matches.
const BrushFace *FindFaceOnPlane( const BrushSolid &solid, const Plane &plane );

} // namespace hammer::geometry

#endif // HAMMER_GEOMETRY_BRUSH_H
