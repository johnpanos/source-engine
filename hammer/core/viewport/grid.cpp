//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor grid (RFC 0002, hammer.viewport). See
//			public/hammer/viewport/grid.h.
//
//=============================================================================//

#include "hammer/viewport/grid.h"

#include <algorithm>
#include <cmath>

namespace hammer::viewport
{

using mapgeometry::Vec3d;

namespace
{

// Legacy V_rint: floor(f + 0.5) above zero, ceil(f - 0.5) below.
double RoundHalfAway( double value )
{
	if ( value > 0.0 )
	{
		return std::floor( value + 0.5 );
	}
	if ( value < 0.0 )
	{
		return std::ceil( value - 0.5 );
	}
	return 0.0;
}

bool IsPowerOfTwo( int value )
{
	return value > 0 && ( value & ( value - 1 ) ) == 0;
}

// Multiples of 'step' in [lo, hi]: first index and count (0 when empty).
struct LineRange
{
	double first = 0.0;
	double count = 0.0;
};

LineRange RangeOf( double lo, double hi, double step )
{
	const double first = std::ceil( lo / step );
	const double last = std::floor( hi / step );
	return { first, last >= first ? last - first + 1.0 : 0.0 };
}

bool MultipleOf( double world, int every )
{
	if ( every <= 0 )
	{
		return false;
	}
	return std::fmod( std::fabs( world ), static_cast<double>( every ) ) == 0.0;
}

GridLineKind Classify( double world, const GridDisplay &display )
{
	if ( display.axisLines && world == 0.0 )
	{
		return GridLineKind::Axis;
	}
	if ( MultipleOf( world, display.blockEvery ) )
	{
		return GridLineKind::Block;
	}
	if ( MultipleOf( world, display.majorEvery ) )
	{
		return GridLineKind::Major;
	}
	return GridLineKind::Minor;
}

} // namespace

bool GridPolicy::SetSize( int size )
{
	if ( !IsPowerOfTwo( size ) || size < kMinSize || size > kMaxSize )
	{
		return false;
	}
	m_size = size;
	return true;
}

bool GridPolicy::StepFiner()
{
	const int next = std::max( m_size / 2, kMinSize );
	const bool changed = next != m_size;
	m_size = next;
	return changed;
}

bool GridPolicy::StepCoarser()
{
	const int next = std::min( m_size * 2, kMaxSize );
	const bool changed = next != m_size;
	m_size = next;
	return changed;
}

double GridPolicy::Snap( double value ) const
{
	if ( !m_enabled || !std::isfinite( value ) )
	{
		return value;
	}
	return RoundHalfAway( value / m_size ) * m_size;
}

Vec3d GridPolicy::SnapPoint( const Vec3d &point, AxisMask mask ) const
{
	return Vec3d( mask.x ? Snap( point.x ) : point.x, mask.y ? Snap( point.y ) : point.y,
	    mask.z ? Snap( point.z ) : point.z );
}

double DisplayedStep( const Camera2D &camera, const GridPolicy &grid, const GridDisplay &display )
{
	double step = grid.Size();
	const double minSpacing =
	    std::isfinite( display.minSpacingPixels ) ? display.minSpacingPixels : 0.0;
	// Zoom >= Camera2D::kMinZoom, so this ends within ~20 doublings.
	while ( step * camera.Zoom() < minSpacing && step < 1.0e18 )
	{
		step *= 2.0;
	}
	return step;
}

std::vector<GridLine> VisibleLines(
    const Camera2D &camera, const GridPolicy &grid, const GridDisplay &display )
{
	std::vector<GridLine> lines;
	if ( !display.show || !camera.HasArea() )
	{
		return lines;
	}

	PlaneRect rect = camera.VisibleRect();
	if ( std::isfinite( display.worldExtent ) && display.worldExtent > 0.0 )
	{
		const double extent = display.worldExtent;
		rect.min.u = std::max( rect.min.u, -extent );
		rect.min.v = std::max( rect.min.v, -extent );
		rect.max.u = std::min( rect.max.u, extent );
		rect.max.v = std::min( rect.max.v, extent );
	}
	if ( rect.min.u > rect.max.u || rect.min.v > rect.max.v )
	{
		return lines;
	}

	const double cap = static_cast<double>( std::max<std::size_t>( display.maxLinesPerAxis, 1 ) );
	double step = DisplayedStep( camera, grid, display );
	LineRange us = RangeOf( rect.min.u, rect.max.u, step );
	LineRange vs = RangeOf( rect.min.v, rect.max.v, step );
	while ( ( us.count > cap || vs.count > cap ) && step < 1.0e18 )
	{
		step *= 2.0;
		us = RangeOf( rect.min.u, rect.max.u, step );
		vs = RangeOf( rect.min.v, rect.max.v, step );
	}

	lines.reserve( static_cast<std::size_t>( us.count + vs.count ) );
	for ( double i = 0.0; i < us.count; i += 1.0 )
	{
		const double u = ( us.first + i ) * step + 0.0; // + 0.0 turns -0 into 0
		lines.push_back( { Classify( u, display ), GridLineOrientation::Vertical, u,
		    camera.PlaneToScreen( { u, 0.0 } ).x } );
	}
	for ( double i = 0.0; i < vs.count; i += 1.0 )
	{
		const double v = ( vs.first + i ) * step + 0.0;
		lines.push_back( { Classify( v, display ), GridLineOrientation::Horizontal, v,
		    camera.PlaneToScreen( { 0.0, v } ).y } );
	}
	return lines;
}

} // namespace hammer::viewport
