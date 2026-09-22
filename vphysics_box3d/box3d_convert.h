//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conversions between Source (VPhysics contract) and Box3D values.
//
// Box3D runs in Source units: the module sets Box3D's length scale to inches
// per meter once at load (see main.cpp), so positions, extents and tolerances
// pass through unscaled and only representation changes here. Masses stay in
// kilograms. Angles are the one real unit change: VPhysics speaks degrees and
// QAngle / local-space angular velocity, Box3D speaks radians, quaternions
// and world-space angular velocity.
//
//=============================================================================//
#ifndef BOX3D_CONVERT_H
#define BOX3D_CONVERT_H

#include "mathlib/mathlib.h"
#include "mathlib/vector.h"
#include "box3d/math_functions.h"

// Game units per meter; the value Box3D's length scale is set to.
const float kBox3DInchesPerMeter = 1.0f / 0.0254f;

inline b3Vec3 ToB3( const Vector &v )
{
	b3Vec3 out = { v.x, v.y, v.z };
	return out;
}

inline Vector FromB3( const b3Vec3 &v )
{
	return Vector( v.x, v.y, v.z );
}

inline b3Quat ToB3( const QAngle &angles )
{
	Quaternion q;
	AngleQuaternion( angles, q );
	b3Quat out = { { q.x, q.y, q.z }, q.w };
	return out;
}

inline QAngle FromB3( const b3Quat &q )
{
	Quaternion source( q.v.x, q.v.y, q.v.z, q.s );
	QAngle angles;
	QuaternionAngles( source, angles );
	return angles;
}

inline b3Transform ToB3Transform( const Vector &origin, const QAngle &angles )
{
	b3Transform out;
	out.p = ToB3( origin );
	out.q = ToB3( angles );
	return out;
}

// World-space radians/second <-> VPhysics local-space degrees/second.
inline AngularImpulse AngularFromB3( const b3Vec3 &worldRadians, const b3Quat &rotation )
{
	b3Vec3 local = b3InvRotateVector( rotation, worldRadians );
	return AngularImpulse( RAD2DEG( local.x ), RAD2DEG( local.y ), RAD2DEG( local.z ) );
}

inline b3Vec3 AngularToB3( const AngularImpulse &localDegrees, const b3Quat &rotation )
{
	b3Vec3 local = { DEG2RAD( localDegrees.x ), DEG2RAD( localDegrees.y ), DEG2RAD( localDegrees.z ) };
	return b3RotateVector( rotation, local );
}

#endif // BOX3D_CONVERT_H
