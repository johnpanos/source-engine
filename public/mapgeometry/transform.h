//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Linear and affine transforms of map space (RFC 0002,
//			world.map-geometry): Source Euler angles (pitch, yaw, roll in
//			degrees) to and from rotation matrices, rotations about world axes,
//			scales and mirrors, and the application of an affine map to points,
//			directions and planes.
//
//			Conventions. Matrices act on column vectors (p' = M p). Source angles
//			follow the engine's AngleMatrix: the matrix columns are the rotated
//			forward (+X), left (+Y) and up (+Z) axes. A plane is transformed by
//			carrying three points on it and rebuilding it, so non-uniform scales
//			and mirrors keep the plane exact; a mirror flips handedness, and the
//			caller decides whether that reverses winding (see Affine::Mirrors).
//
//=============================================================================//

#ifndef MAPGEOMETRY_TRANSFORM_H
#define MAPGEOMETRY_TRANSFORM_H

#include "mapgeometry/brush.h"

#include <optional>

namespace mapgeometry
{

struct Mat3
{
	double m[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

	static Mat3 Identity() { return Mat3{}; }
	// Diagonal scale.
	static Mat3 Scale( const Vec3d &s );
	// Right-handed rotation of 'degrees' about world axis 0 (X), 1 (Y) or 2 (Z).
	static Mat3 AxisRotation( int axis, double degrees );
	// Negates world axis 0/1/2 (a reflection through the plane normal to it).
	static Mat3 Mirror( int axis );
};

Mat3 Multiply( const Mat3 &a, const Mat3 &b );
Mat3 Transpose( const Mat3 &a );
double Determinant( const Mat3 &a );
Vec3d Apply( const Mat3 &a, const Vec3d &v );

// Source's AngleMatrix: rotation from (pitch, yaw, roll) degrees.
Mat3 AngleMatrix( double pitch, double yaw, double roll );

struct EulerAngles
{
	double pitch = 0.0;
	double yaw = 0.0;
	double roll = 0.0;
};

// Source's MatrixAngles: the (pitch, yaw, roll) whose AngleMatrix is 'rotation'.
// Gimbal lock (forward vertical) resolves roll to 0. 'rotation' must be a proper
// rotation; the result is unspecified otherwise.
EulerAngles MatrixToAngles( const Mat3 &rotation );

// p' = linear * p + translation.
struct Affine
{
	Mat3 linear;
	Vec3d translation;

	static Affine Identity() { return Affine{}; }
	static Affine Translation( const Vec3d &delta );
	// 'linear' applied about 'pivot' (the pivot is a fixed point).
	static Affine About( const Mat3 &linear, const Vec3d &pivot );

	Vec3d Point( const Vec3d &p ) const;
	// Transforms a direction (no translation). Not renormalized.
	Vec3d Direction( const Vec3d &d ) const;
	// True when the map reverses handedness (negative determinant).
	bool Mirrors() const;
	// True when the linear part is the identity.
	bool IsTranslation() const;
};

// Composition: (a * b)(p) == a(b(p)).
Affine Compose( const Affine &a, const Affine &b );

// The inverse map, or nothing for a singular linear part.
std::optional<Affine> Inverse( const Affine &a );

// Rebuilds 'plane' after mapping three of its points through 'xf'. The result's
// normal points out of the image of the original interior (the original
// outside half-space maps to the new outside half-space). Nothing for a
// singular map.
std::optional<Plane> TransformPlane( const Affine &xf, const Plane &plane );

} // namespace mapgeometry

#endif // MAPGEOMETRY_TRANSFORM_H
