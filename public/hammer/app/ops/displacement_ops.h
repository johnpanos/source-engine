//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Displacement editing (RFC 0002, hammer.app; legacy tooldisplace,
//			mapdisp and dispsew): turning quad faces into displacements and
//			back, changing their power, sculpting and alpha painting with a
//			spherical brush, elevation, and sewing coincident vertices across
//			neighboring displacements.
//
//			Model. A vertex's displaced position is
//			  base(i, j) + offset(i, j) + normal(i, j) * distance(i, j)
//			             + faceNormal * elevation
//			(mapgeometry displacement.h). Sculpting edits the displacement
//			vector normal * distance and stores it back as a unit normal and a
//			distance; offsets are left alone. After every geometric edit the
//			legacy walkable (normal z >= 0.7) and buildable (>= 0.8) triangle
//			tags are recomputed; the "force" override bits are preserved.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_DISPLACEMENT_OPS_H
#define HAMMER_APP_OPS_DISPLACEMENT_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"

#include <optional>
#include <vector>

namespace hammer::app::ops
{

// Legacy triangle tag bits (public/builddisp.h COREDISPTRI_TAG_*).
constexpr int kDispTagWalkable = 1 << 0;
constexpr int kDispTagForceWalkableBit = 1 << 1;
constexpr int kDispTagForceWalkableValue = 1 << 2;
constexpr int kDispTagBuildable = 1 << 3;
constexpr int kDispTagForceBuildableBit = 1 << 4;
constexpr int kDispTagForceBuildableValue = 1 << 5;

// Makes each face a flat displacement of 'power' (2..4). Faces must bound
// quads and not already be displaced. The start position is the face corner
// with the smallest (x, y, z), so the grid orientation is deterministic.
EditResult CreateDisplacement(
    scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces, int power );

// Removes the displacement from each face.
EditResult DestroyDisplacement(
    scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces );

// Resamples each displacement to 'power' (2..4): displacement vectors, offsets
// and alphas are bilinearly interpolated over the grid; allowed vertices reset.
EditResult SetDisplacementPower(
    scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces, int power );

enum class SculptMode
{
	Raise,  // push along the direction by amount
	Lower,  // pull along the direction by amount
	Set,    // set the displacement along the direction to amount
	Smooth, // move toward the average of grid neighbors (amount in 0..1)
};

struct SculptBrush
{
	mapgeometry::Vec3d center;
	double radius = 32.0;
	double amount = 8.0;
	SculptMode mode = SculptMode::Raise;
	// The paint direction; the face normal when absent (legacy default).
	std::optional<mapgeometry::Vec3d> direction;
	// Linear falloff to zero at the radius; otherwise full strength inside.
	bool falloff = true;
};

// Applies the brush to every vertex of the displacements within its radius
// (measured to the displaced position). Nothing when no vertex is in reach.
EditResult Sculpt(
    scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces, const SculptBrush &brush );

enum class AlphaMode
{
	Set,
	Raise,
	Lower,
};
// Paints vertex alpha (the blend weight, clamped to 0..255) within 'radius'.
EditResult PaintAlpha( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    const mapgeometry::Vec3d &center, double radius, double value, AlphaMode mode,
    bool falloff = true );

EditResult SetDisplacementElevation(
    scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces, double elevation );

// Sews the displacements: vertices whose undisplaced positions coincide (within
// 0.01 units) across different faces move to the average of their displaced
// positions, closing cracks along shared edges and corners. Vertices of
// different powers are sewn where their grids coincide.
EditResult SewDisplacements( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces );

// The legacy tags for a displaced surface's triangles, keeping the force bits
// of 'previous' (same length) when given.
std::vector<int> ComputeTriangleTags(
    const scene::Solid &solid, std::size_t sideIndex, const std::vector<int> *previous = nullptr );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_DISPLACEMENT_OPS_H
