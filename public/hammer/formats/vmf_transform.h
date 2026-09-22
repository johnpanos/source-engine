//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared VMF geometry transform for the Hammer editor (RFC 0002,
//			hammer.formats). Placing a VMF fragment into a map -- whether a
//			func_instance (hammer.formats instancing) or a prefab (hammer.formats
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

#ifndef HAMMER_FORMATS_VMF_TRANSFORM_H
#define HAMMER_FORMATS_VMF_TRANSFORM_H

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/brush.h" // geometry::Vec3d

namespace hammer::formats
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
geometry::Vec3d Rotate( const Mat3 &r, const geometry::Vec3d &v );

// Place a point: rotate then translate by 'origin'.
geometry::Vec3d Place( const Mat3 &r, const geometry::Vec3d &origin, const geometry::Vec3d &p );

// A copy of a "solid" block with every side plane's three points placed by
// (r, origin). Non-plane data (materials, texture axes) is preserved verbatim.
KeyValueNode TransformSolid(
    const KeyValueNode &solid, const Mat3 &r, const geometry::Vec3d &origin );

// A copy of an entity block with its "origin" placed by (r, origin). Entity angle
// composition is a declared later increment (only position moves).
KeyValueNode TransformEntity(
    const KeyValueNode &entity, const Mat3 &r, const geometry::Vec3d &origin );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VMF_TRANSFORM_H
