//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The independent oracle for area lights (render.area-light.v1):
//          numerical integration over a rectangle, for any weight. The
//          contract's suite judges the diffuse form factor with it; a
//          per-pixel evaluation (RFC 0016 K7's LTC, diffuse and GGX) is
//          judged against its BRDF integrated the same way.
//
//===========================================================================//

#ifndef RENDERTEST_AREA_LIGHT_ORACLE_H
#define RENDERTEST_AREA_LIGHT_ORACLE_H

#include "render/area_light.h"

#include <cmath>

namespace area_light_oracle
{

// The integral over the rectangle's emitting side of
// weight( unit direction p -> x ) * cos_e / d^2 dA (the solid-angle measure),
// by the midpoint rule on cells x cells. `weight` returns what the receiver
// makes of light arriving from that direction (for the diffuse form factor:
// max(0, n . dir) / pi; for a BRDF: f(dir, view) max(0, n . dir)). Points
// behind a one-sided light's front give nothing.
template <typename Weight>
double IntegrateOverRect( const area_light::Rect &rect, const float p[3], Weight weight, int cells )
{
	float normal[3];
	area_light::Normal( rect, normal );
	const double cellArea = double( area_light::Area( rect ) ) / ( double( cells ) * cells );
	double sum = 0.0;
	for ( int i = 0; i < cells; ++i )
		for ( int j = 0; j < cells; ++j )
		{
			const double s = -1.0 + ( 2.0 * i + 1.0 ) / cells;
			const double t = -1.0 + ( 2.0 * j + 1.0 ) / cells;
			double d[3];
			for ( int k = 0; k < 3; ++k )
				d[k] = rect.center[k] + s * rect.halfU[k] + t * rect.halfV[k] - p[k];
			const double d2 = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
			const double len = std::sqrt( d2 );
			double cosE = -( d[0] * normal[0] + d[1] * normal[1] + d[2] * normal[2] ) / len;
			if ( rect.twoSided )
				cosE = std::fabs( cosE );
			if ( cosE <= 0.0 )
				continue;
			const float dir[3] = { float( d[0] / len ), float( d[1] / len ), float( d[2] / len ) };
			sum += weight( dir ) * cosE * cellArea / d2;
		}
	return sum;
}

// The diffuse form factor (the contract's FormFactor) by integration.
inline double IntegratedFormFactor(
    const area_light::Rect &rect, const float p[3], const float n[3], int cells )
{
	return IntegrateOverRect(
	    rect, p,
	    [n]( const float dir[3] )
	    {
		    const double cosR =
		        double( dir[0] ) * n[0] + double( dir[1] ) * n[1] + double( dir[2] ) * n[2];
		    return cosR > 0.0 ? cosR / area_light::kPi : 0.0;
	    },
	    cells );
}

} // namespace area_light_oracle

#endif // RENDERTEST_AREA_LIGHT_ORACLE_H
