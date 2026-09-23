//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Fit scalar SH L1 irradiance from four directional bake samples.
//          The caller supplies the flat and three RNM sample directions from
//          the canonical lighting layout; this math does not own that layout.
//
//===========================================================================//

#ifndef RENDER_PBR_SH_L1_H
#define RENDER_PBR_SH_L1_H

#include <array>
#include <cmath>
#include <utility>

namespace render::pbr
{
struct IrradianceDirection
{
	float x;
	float y;
	float z;
};

// Coefficients already include the SH basis and clamped-cosine factors:
// E(n) = constant + x*n.x + y*n.y + z*n.z. No positive clamp is applied here.
struct ShL1Irradiance
{
	float constant;
	float x;
	float y;
	float z;
};

[[nodiscard]] inline float EvaluateShL1(
    const ShL1Irradiance &fit, const IrradianceDirection &normal )
{
	return fit.constant + fit.x * normal.x + fit.y * normal.y + fit.z * normal.z;
}

// Four noncoplanar directions determine one affine L1 irradiance fit. Partial
// pivoting keeps the solve stable for arbitrary tangent frames. Failure leaves
// the caller's output unchanged; no approximate result is published.
[[nodiscard]] inline bool FitShL1( const std::array<IrradianceDirection, 4> &directions,
    const std::array<float, 4> &irradiance, ShL1Irradiance &out )
{
	double rows[4][5] = {};
	for ( int row = 0; row < 4; ++row )
	{
		const IrradianceDirection &n = directions[row];
		if ( !std::isfinite( n.x ) || !std::isfinite( n.y ) || !std::isfinite( n.z ) ||
		     !std::isfinite( irradiance[row] ) )
			return false;
		rows[row][0] = 1.0;
		rows[row][1] = n.x;
		rows[row][2] = n.y;
		rows[row][3] = n.z;
		rows[row][4] = irradiance[row];
	}
	for ( int column = 0; column < 4; ++column )
	{
		int pivot = column;
		for ( int row = column + 1; row < 4; ++row )
		{
			if ( std::fabs( rows[row][column] ) > std::fabs( rows[pivot][column] ) )
				pivot = row;
		}
		if ( std::fabs( rows[pivot][column] ) < 1e-7 )
			return false;
		if ( pivot != column )
		{
			for ( int entry = column; entry < 5; ++entry )
				std::swap( rows[pivot][entry], rows[column][entry] );
		}
		const double divisor = rows[column][column];
		for ( int entry = column; entry < 5; ++entry )
			rows[column][entry] /= divisor;
		for ( int row = 0; row < 4; ++row )
		{
			if ( row == column )
				continue;
			const double scale = rows[row][column];
			for ( int entry = column; entry < 5; ++entry )
				rows[row][entry] -= scale * rows[column][entry];
		}
	}
	const ShL1Irradiance result = { static_cast<float>( rows[0][4] ),
	    static_cast<float>( rows[1][4] ), static_cast<float>( rows[2][4] ),
	    static_cast<float>( rows[3][4] ) };
	if ( !std::isfinite( result.constant ) || !std::isfinite( result.x ) ||
	     !std::isfinite( result.y ) || !std::isfinite( result.z ) )
		return false;
	out = result;
	return true;
}
} // namespace render::pbr

#endif // RENDER_PBR_SH_L1_H
