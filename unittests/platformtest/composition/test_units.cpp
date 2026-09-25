//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: foundation.units conformance (RFC 0006 named conversions, R05).
//
//=============================================================================//

#include "foundation/units.h"
#include "testing/checks.h"

#include <cstring>

namespace
{
using namespace foundation::units;

// The single-precision constants must be bit-identical to the literal
// expressions they replaced in float consumers (Box3D's length scale).
bool SameBits( float a, float b )
{
	return std::memcmp( &a, &b, sizeof( float ) ) == 0;
}

static_assert( kMetersPerSourceUnit == 0.0254 );
static_assert( SourceUnitsToMeters( 100.0 ) == 100.0 * 0.0254 );
static_assert( DegreesToRadians( 180.0 ) == kPi );
} // namespace

int main()
{
	testing::Checks checks;
	const float oldScale = 1.0f / 0.0254f;
	checks.That( SameBits( kSourceUnitsPerMeterF, oldScale ), "float scale is bit-identical" );
	checks.That( SameBits( kSourceUnitsPerMeterF * kSourceUnitsPerMeterF,
	                 ( 1.0f / 0.0254f ) * ( 1.0f / 0.0254f ) ),
	    "float inertia scale is bit-identical" );
	checks.That( SameBits( kMetersPerSourceUnitF, 0.0254f ), "float meters per unit" );
	checks.Near(
	    MetersToSourceUnits( 1.0 ), 39.37007874015748, 1e-12, "one meter in Source units" );
	checks.Near( SourceUnitsToMeters( MetersToSourceUnits( 3.5 ) ), 3.5, 1e-12, "round trip" );
	checks.Near(
	    MetersToSourceUnits( SourceUnitsToMeters( 128.0 ) ), 128.0, 1e-12, "reverse trip" );
	checks.Near( RadiansToDegrees( DegreesToRadians( 37.5 ) ), 37.5, 1e-12, "angle round trip" );
	checks.Near( DegreesToRadians( 90.0 ), kPi / 2.0, 1e-15, "right angle" );
	checks.Equal( kSourceUnitsPerMeter * kMetersPerSourceUnit, 1.0, "reciprocal constants" );
	return checks.Report();
}
