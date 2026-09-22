//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Named owner of the editor's grid-rounding policy (RFC 0002,
//			hammer.geometry). This is the single authoritative definition of the
//			rounding rule that legacy hammer used through hammer_mathlib's
//			V_rint: round half AWAY from zero (not the round-half-to-even that
//			std::rint / std::nearbyint produce under the default rounding mode).
//
//			Snapping and any other extracted operation that must reproduce legacy
//			grid behavior routes through this owner instead of re-deriving the
//			rule, so the rounding policy has one place to change and to test.
//
//=============================================================================//

#ifndef HAMMER_GEOMETRY_ROUNDING_H
#define HAMMER_GEOMETRY_ROUNDING_H

namespace hammer::geometry
{

// Rounds to the nearest integral value, with halves rounded away from zero.
// Preserves hammer_mathlib.cpp V_rint exactly: floor(f + 0.5) for f > 0,
// ceil(f - 0.5) for f < 0, and 0 for f == 0. This differs from std::rint, which
// rounds halves to even; the difference is observable at exact .5 boundaries
// (e.g. RoundHalfAwayFromZero( 0.5f ) == 1.0f, std::rint( 0.5f ) == 0.0f).
float RoundHalfAwayFromZero( float value );

} // namespace hammer::geometry

#endif // HAMMER_GEOMETRY_ROUNDING_H
