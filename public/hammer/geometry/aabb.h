//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Axis-aligned bounding box value type for the strict Hammer geometry
//			module (RFC 0002, hammer.geometry). This is a dependency-free
//			extraction of hammer/BoundBox: no MFC, no tier0/platform.h, no PCH,
//			no GPU. It owns its own geometric value type so it can compile and
//			test on the Linux headless core profile.
//
//			Semantics are preserved from the legacy BoundBox class byte-for-byte
//			in float arithmetic and operation order. The Source Vector <-> Vec3
//			conversion at the legacy boundary is a separate, named policy owned by
//			the source/mfc adapter; it is not part of this module.
//
//=============================================================================//

#ifndef HAMMER_GEOMETRY_AABB_H
#define HAMMER_GEOMETRY_AABB_H

#include <cstddef>

namespace hammer::geometry
{

// Axis indices, preserving hammer_mathlib.h's AXIS_X/AXIS_Y/AXIS_Z ordering.
enum Axis
{
	kAxisX = 0,
	kAxisY = 1,
	kAxisZ = 2,
};

// The sentinel legacy ResetBounds uses (mapdefs.h COORD_NOTINIT). A reset box has
// mins = +kUninitialized and maxs = -kUninitialized, so it is deliberately
// invalid until the first UpdateBounds call.
constexpr float kUninitialized = 99999.0f;

//-----------------------------------------------------------------------------
// Purpose: Minimal float 3-vector owned by the geometry module. Components match
//			Source Vector precision (float), so boundary conversion is exact.
//-----------------------------------------------------------------------------
struct Vec3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	constexpr Vec3() = default;

	constexpr Vec3( float in_x, float in_y, float in_z ) : x( in_x ), y( in_y ), z( in_z ) {}

	// Index access mirrors legacy pt[0]/pt[1]/pt[2] usage. The caller passes a
	// valid axis; out-of-range access is a programming error, not a runtime path.
	constexpr float &operator[]( std::size_t axis ) { return ( &x )[axis]; }

	constexpr float operator[]( std::size_t axis ) const { return ( &x )[axis]; }
};

//-----------------------------------------------------------------------------
// Purpose: Axis-aligned bounding box. Direct extraction of BoundBox; every
//			method preserves the legacy computation exactly.
//-----------------------------------------------------------------------------
class AxisAlignedBox
{
public:
	// Constructs a reset (deliberately invalid) box, like BoundBox::BoundBox().
	AxisAlignedBox();

	// Constructs from explicit mins/maxs without validation, like the legacy
	// two-argument constructor.
	AxisAlignedBox( const Vec3 &mins, const Vec3 &maxs );

	// Sets the box to the uninitialized sentinel so UpdateBounds seeds it.
	void ResetBounds();

	// Sets mins/maxs outright (ResetBounds + UpdateBounds equivalent).
	void SetBounds( const Vec3 &mins, const Vec3 &maxs );

	void GetBounds( Vec3 &mins, Vec3 &maxs ) const;

	// Expands the box to include a point / another box's extent.
	void UpdateBounds( const Vec3 &point );
	void UpdateBounds( const Vec3 &mins, const Vec3 &maxs );
	void UpdateBounds( const AxisAlignedBox &other );

	void GetBoundsCenter( Vec3 &center ) const;
	void GetBoundsSize( Vec3 &size ) const;

	// Overlap test with open faces: boxes that only touch do NOT intersect,
	// preserving the legacy >=/<= comparisons.
	bool IsIntersectingBox( const Vec3 &mins, const Vec3 &maxs ) const;

	// True when this box is fully contained within [mins, maxs] (inclusive).
	bool IsInsideBox( const Vec3 &mins, const Vec3 &maxs ) const;

	bool ContainsPoint( const Vec3 &point ) const;

	// True when maxs >= mins on every axis.
	bool IsValidBox() const;

	// Snaps the minimum corner to the grid while preserving the box size.
	// A grid size <= 0 is an invalid input (degenerate) and is rejected: the box
	// is left unchanged. The legacy code assumed a positive grid.
	void SnapToGrid( int gridSize );

	// Rotates the box 90 degrees about the given axis, preserving legacy math.
	void Rotate90( int axis );

	Vec3 mins;
	Vec3 maxs;
};

} // namespace hammer::geometry

#endif // HAMMER_GEOMETRY_AABB_H
