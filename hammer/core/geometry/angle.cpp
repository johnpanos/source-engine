//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the angle numeric policies (RFC 0002). Ported from
//			hammer_mathlib fixang/lineangle, preserving the exact legacy math
//			(including the legacy RAD2DEG expansion) so results match bit-for-bit.
//			No tier0/MFC/PCH dependencies.
//
//=============================================================================//

#include "hammer/geometry/angle.h"

#include <cmath>

namespace hammer::geometry
{

namespace
{

// Legacy mathlib.h constants, reproduced so LineAngleDegrees matches the legacy
// RAD2DEG( x ) == (float)(x) * (float)(180.f / M_PI_F) expansion exactly.
constexpr double kPi = 3.14159265358979323846;

} // namespace

float NormalizeAngleDegrees( float degrees )
{
	// fixang: single-step wrap, double literals as in the legacy source.
	if ( degrees < 0.0 )
	{
		return static_cast<float>( degrees + 360.0 );
	}
	if ( degrees > 359.9 )
	{
		return static_cast<float>( degrees - 360.0 );
	}
	return degrees;
}

float LineAngleDegrees( float x1, float y1, float x2, float y2 )
{
	const float x = x2 - x1;
	const float y = y2 - y1;

	if ( !x && !y )
	{
		return 0.0f;
	}

	const float radiansToDegrees = 180.0f / static_cast<float>( kPi );
	const double radians = std::atan2( static_cast<double>( y ), static_cast<double>( x ) );
	return static_cast<float>( radians ) * radiansToDegrees;
}

} // namespace hammer::geometry
