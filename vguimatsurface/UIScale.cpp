//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: UI scale policy for the material system surface (see UIScale.h).
//
//=============================================================================//

#include "UIScale.h"

#include <cmath>

namespace uiscale
{

float ComputeScale( const Inputs &inputs )
{
	float chosen = 1.0f;
	if ( inputs.userScale > 0.0f )
		chosen = inputs.userScale;
	else if ( inputs.displayScale > 0.0f )
		chosen = inputs.displayScale;

	// A back buffer smaller than the window is magnified on presentation, so
	// each of its pixels already covers more of the display.
	float presentation = 1.0f;
	if ( inputs.backBufferTall > 0 && inputs.drawableTall > 0 )
	{
		presentation = static_cast<float>( inputs.backBufferTall ) / inputs.drawableTall;
		if ( std::fabs( presentation - 1.0f ) < 0.02f )
			presentation = 1.0f;
	}

	float scale = std::floor( chosen * presentation * 100.0f + 0.5f ) / 100.0f;
	if ( scale < kMinScale )
		scale = kMinScale;
	if ( scale > kMaxScale )
		scale = kMaxScale;

	if ( inputs.backBufferWide > 0 && inputs.backBufferTall > 0 )
	{
		const float fitWide = static_cast<float>( inputs.backBufferWide ) / kMinUIWide;
		const float fitTall = static_cast<float>( inputs.backBufferTall ) / kMinUITall;
		float fit = fitWide < fitTall ? fitWide : fitTall;
		if ( fit < 1.0f )
			fit = 1.0f;
		if ( scale > fit )
			scale = fit;
	}
	return scale;
}

int UnitsCoveringPixels( int pixels, float scale )
{
	if ( scale == 1.0f )
		return pixels;
	return static_cast<int>( std::ceil( pixels / scale - 1e-3f ) );
}

int UnitAtPixel( int pixel, float scale )
{
	if ( scale == 1.0f )
		return pixel;
	return static_cast<int>( std::floor( pixel / scale + 1e-4f ) );
}

int PixelAtUnit( int unit, float scale )
{
	if ( scale == 1.0f )
		return unit;
	// The first pixel inside the unit; a unit of at least one pixel always has one.
	return static_cast<int>( std::ceil( unit * scale - 1e-4f ) );
}

int PixelsToUnitsRounded( int pixels, float scale )
{
	if ( scale == 1.0f )
		return pixels;
	return static_cast<int>( std::floor( pixels / scale + 0.5f ) );
}

int UnitsToPixelsRounded( int units, float scale )
{
	if ( scale == 1.0f )
		return units;
	return static_cast<int>( std::floor( units * scale + 0.5f ) );
}

} // namespace uiscale
