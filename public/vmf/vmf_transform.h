//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared VMF geometry transform for map placement (RFC 0002,
//			content.vmf). Placing a VMF fragment into a map -- whether a
//			func_instance (the editor's instancing) or a prefab (the editor's
//			prefab) -- rotates and translates its world solids and entities by a
//			Source QAngle + origin. This module is the ONE owner of that transform
//			(DRY): both instancing and prefab route through it rather than each
//			re-deriving the rotation matrix and plane/origin math. Strict, MFC-free,
//			GPU-free.
//
//			Rotation is the Source QAngle convention (pitch about Y, yaw about Z,
//			roll about X), degrees. A world point maps as rotate-then-translate.
//
//=============================================================================//

#ifndef VMF_VMF_TRANSFORM_H
#define VMF_VMF_TRANSFORM_H

#include "kvtext/keyvalues.h"
#include "mapgeometry/brush.h" // mapgeometry::Vec3d

namespace vmf
{

// A row-major 3x3 rotation matrix (identity by default).
struct Mat3
{
	double m[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
};

// Source QAngle (pitch, yaw, roll in degrees) -> rotation matrix.
Mat3 AngleMatrix( double pitch, double yaw, double roll );

// Matrix product (compose two rotations: applying 'b' then 'a').
Mat3 Multiply( const Mat3 &a, const Mat3 &b );

// Rotate a vector (no translation).
mapgeometry::Vec3d Rotate( const Mat3 &r, const mapgeometry::Vec3d &v );

// Place a point: rotate then translate by 'origin'.
mapgeometry::Vec3d Place(
    const Mat3 &r, const mapgeometry::Vec3d &origin, const mapgeometry::Vec3d &p );

// A copy of a "solid" block with every side plane's three points placed by
// (r, origin). Non-plane data (materials, texture axes) is preserved verbatim.
kvtext::KeyValueNode TransformSolid(
    const kvtext::KeyValueNode &solid, const Mat3 &r, const mapgeometry::Vec3d &origin );

// A copy of an entity block with its "origin" placed by (r, origin). Entity angle
// composition is a declared later increment (only position moves).
kvtext::KeyValueNode TransformEntity(
    const kvtext::KeyValueNode &entity, const Mat3 &r, const mapgeometry::Vec3d &origin );

} // namespace vmf

#endif // VMF_VMF_TRANSFORM_H
