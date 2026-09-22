//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless displacement (VMF "dispinfo") geometry for the Hammer editor
//			(RFC 0002, hammer.geometry). A displacement turns one quadrilateral
//			brush face into a subdivided, per-vertex-offset surface -- the terrain
//			primitive Source maps rely on. This module is the strict, MFC-free,
//			GPU-free core: it parses a dispinfo keyvalues block and builds the
//			displaced vertex grid + triangle list that a renderer or the scene
//			bridge later consumes. It owns none of the presentation.
//
//			The brush contract (geometry.brush.v1) deliberately excludes
//			displacements as "a later migration"; this is that migration's core.
//			A displacement of power p is a grid of (2^p + 1) x (2^p + 1) vertices.
//			The base surface is the bilinear interpolation of the face's four
//			corners; each grid vertex is then pushed along its recorded normal by
//			its recorded distance, plus a base offset and a face-normal elevation:
//
//			    v(i,j) = bilerp(corners, s, t)
//			             + offset(i,j)
//			             + normal(i,j) * distance(i,j)
//			             + faceNormal * elevation
//
//			The grid's (0,0) corner is the face corner nearest "startposition",
//			matching Hammer/VBSP, so a round-tripped displacement keeps orientation.
//
//=============================================================================//

#ifndef HAMMER_GEOMETRY_DISPLACEMENT_H
#define HAMMER_GEOMETRY_DISPLACEMENT_H

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/brush.h" // Vec3d

#include <array>
#include <optional>
#include <vector>

namespace hammer::geometry
{

// The parsed, validated contents of a VMF "dispinfo" block. Grids are row-major
// with (2^power + 1) rows of (2^power + 1) entries each; row r is VMF key "row<r>".
struct DispInfo
{
	int power = 0;                 // subdivision power; grid side length is 2^power + 1
	int subdiv = 0;                // subdivision flag (0/1); preserved, not yet meshed
	Vec3d startPosition;           // world position identifying the grid's (0,0) corner
	double elevation = 0.0;        // uniform push along the face normal
	std::vector<Vec3d> normals;    // per-vertex displacement direction (side*side)
	std::vector<double> distances; // per-vertex distance along its normal (side*side)
	std::vector<Vec3d> offsets;    // per-vertex base offset; all-zero when absent
	std::vector<double> alphas;    // per-vertex blend weight 0..255 (side*side); 0 when absent
	std::vector<int> triangleTags; // per-triangle tag; 2*(side-1)^2, matching triangle order

	// Grid side length (2^power + 1). Valid only when power is in range.
	int Side() const { return ( 1 << power ) + 1; }
};

// The subdivided, displaced surface: row-major vertices and a triangle index list.
struct DisplacementSurface
{
	int power = 0;
	int side = 0;                              // 2^power + 1
	std::vector<Vec3d> vertices;               // side*side, row-major (r*side + c)
	std::vector<double> vertexAlphas;          // side*side, parallel to vertices (blend weight)
	std::vector<Vec3d> vertexNormals;          // side*side, smooth per-vertex normals (lighting)
	std::vector<std::array<int, 3>> triangles; // 2*(side-1)^2 triangles

	std::size_t VertexCount() const { return vertices.size(); }
};

// Parses a "dispinfo" keyvalues block. Returns nullopt when the block is malformed
// or internally inconsistent: power outside [1, 4]; a normals/distances/offsets
// grid whose row count != side or whose row length != side (offsets may be
// absent). "elevation" and "offsets" are optional; "power", "startposition",
// "normals" and "distances" are required.
std::optional<DispInfo> ParseDispInfo( const formats::KeyValueNode &dispBlock );

// Builds the displaced surface for a quad face. 'corners' are the face's four
// quad corners in winding order; the grid origin is whichever corner is nearest
// 'disp.startPosition'. 'faceNormal' carries the elevation. The base grid is the
// bilinear interpolation of the (origin-rotated) corners.
DisplacementSurface BuildDisplacementSurface(
    const std::array<Vec3d, 4> &corners, const Vec3d &faceNormal, const DispInfo &disp );

} // namespace hammer::geometry

#endif // HAMMER_GEOMETRY_DISPLACEMENT_H
