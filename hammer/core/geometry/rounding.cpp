//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the grid-rounding policy owner (RFC 0002). Ported
//			verbatim from hammer_mathlib.cpp V_rint to preserve legacy snapping
//			behavior exactly. No tier0/MFC/PCH dependencies.
//
//=============================================================================//

#include "hammer/geometry/rounding.h"

#include <cmath>

namespace hammer::geometry
{

float RoundHalfAwayFromZero( float value )
{
	if ( value > 0.0f )
	{
		return static_cast<float>( std::floor( value + 0.5f ) );
	}
	if ( value < 0.0f )
	{
		return static_cast<float>( std::ceil( value - 0.5f ) );
	}
	return 0.0f;
}

} // namespace hammer::geometry
