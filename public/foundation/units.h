//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Named unit conversions (RFC 0006 "Results and value types": named
//          quantity/conversion functions over ambiguous literals). The one owner
//          of "one Source unit is one inch, 0.0254 m". Header-only constants and
//          constexpr functions; dialect-neutral (C++11 and later).
//
//=============================================================================//

#ifndef FOUNDATION_UNITS_H
#define FOUNDATION_UNITS_H

namespace foundation
{
namespace units
{

// One Source (world/Hammer) unit is one inch.
constexpr double kMetersPerSourceUnit = 0.0254;
constexpr double kSourceUnitsPerMeter = 1.0 / kMetersPerSourceUnit;

// Single-precision forms, written exactly as the expressions their float
// consumers used before, so they are bit-identical to those consumers' values.
constexpr float kMetersPerSourceUnitF = 0.0254f;
constexpr float kSourceUnitsPerMeterF = 1.0f / 0.0254f;

constexpr double SourceUnitsToMeters( double sourceUnits )
{
	return sourceUnits * kMetersPerSourceUnit;
}
constexpr double MetersToSourceUnits( double meters )
{
	return meters * kSourceUnitsPerMeter;
}

constexpr double kPi = 3.14159265358979323846;
constexpr double DegreesToRadians( double degrees )
{
	return degrees * ( kPi / 180.0 );
}
constexpr double RadiansToDegrees( double radians )
{
	return radians * ( 180.0 / kPi );
}

} // namespace units
} // namespace foundation

#endif // FOUNDATION_UNITS_H
