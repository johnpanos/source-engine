//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Angle numeric policies for the strict Hammer geometry module
//			(RFC 0002, hammer.geometry owns "numeric policies"). Extracted from
//			hammer_mathlib fixang/lineangle. These preserve the legacy semantics
//			EXACTLY, including their quirks, so that when rotation gizmos and 2D
//			tools are later extracted they share one definition of the rule rather
//			than re-deriving a subtly different one. Dependency-free.
//
//=============================================================================//

#ifndef HAMMER_GEOMETRY_ANGLE_H
#define HAMMER_GEOMETRY_ANGLE_H

namespace hammer::geometry
{

// Single-step degree normalization, preserving hammer_mathlib fixang EXACTLY:
//   a < 0      -> a + 360
//   a > 359.9  -> a - 360
//   otherwise  -> a
// This is deliberately NOT a full modulo. It assumes an input roughly within
// [-360, 720); e.g. NormalizeAngleDegrees( 720 ) == 360, not 0. The quirk is
// preserved so extraction does not change tool behavior; a real full-range
// normalization would be a separately reviewed behavior change.
float NormalizeAngleDegrees( float degrees );

// Angle in degrees of the vector (x2-x1, y2-y1), preserving hammer_mathlib
// lineangle: atan2(dy, dx) converted with the legacy RAD2DEG. A zero-length
// segment returns 0.
float LineAngleDegrees( float x1, float y1, float x2, float y2 );

} // namespace hammer::geometry

#endif // HAMMER_GEOMETRY_ANGLE_H
