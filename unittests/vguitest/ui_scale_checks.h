//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the VGUI UI scale policy (vguimatsurface/UIScale.h).
//          The checks take the policy as function pointers so the sensitivity
//          suite can run them against deliberately wrong policies.
//
//=============================================================================//

#ifndef UNITTESTS_VGUITEST_UI_SCALE_CHECKS_H
#define UNITTESTS_VGUITEST_UI_SCALE_CHECKS_H

#include "vguimatsurface/UIScale.h"

#include <cmath>
#include <cstdio>

namespace uiscaletest
{

struct Policy
{
	float ( *computeScale )( const uiscale::Inputs & );
	int ( *unitsCoveringPixels )( int, float );
	int ( *unitAtPixel )( int, float );
	int ( *pixelAtUnit )( int, float );
	int ( *pixelsToUnitsRounded )( int, float );
	int ( *unitsToPixelsRounded )( int, float );
};

struct Tally
{
	unsigned long checks = 0;
	unsigned long failures = 0;
	bool verbose = true;

	void Check( bool condition, const char *what )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			if ( verbose )
				std::printf( "FAIL: %s\n", what );
		}
	}
};

inline uiscale::Inputs MakeInputs(
    float display, float user, int bbWide, int bbTall, int drawWide, int drawTall )
{
	uiscale::Inputs inputs = {};
	inputs.displayScale = display;
	inputs.userScale = user;
	inputs.backBufferWide = bbWide;
	inputs.backBufferTall = bbTall;
	inputs.drawableWide = drawWide;
	inputs.drawableTall = drawTall;
	return inputs;
}

inline bool Near( float a, float b )
{
	return std::fabs( a - b ) < 1e-4f;
}

// Scale selection: the display scale, the user override, presentation of a
// back buffer at another size, the limits and the minimum UI extent.
inline void CheckScaleSelection( const Policy &policy, Tally &tally )
{
	const auto scale = [&]( float display, float user, int bbw, int bbt, int dw, int dt ) {
		return policy.computeScale( MakeInputs( display, user, bbw, bbt, dw, dt ) );
	};
	tally.Check( Near( scale( 1.0f, 0.0f, 1920, 1080, 1920, 1080 ), 1.0f ),
	    "an unscaled display selects scale 1" );
	tally.Check( Near( scale( 1.5f, 0.0f, 2816, 1694, 2816, 1694 ), 1.5f ),
	    "a 150% display with a matching back buffer selects 1.5" );
	tally.Check( Near( scale( 1.25f, 0.0f, 2400, 1350, 2400, 1350 ), 1.25f ),
	    "a 125% display selects 1.25" );
	tally.Check( Near( scale( 1.5f, 0.0f, 1920, 1080, 2880, 1620 ), 1.0f ),
	    "a back buffer magnified 1.5x to the window already provides the display's scale" );
	tally.Check( Near( scale( 2.0f, 0.0f, 1920, 1080, 2880, 1620 ), 1.33f ),
	    "a 200% display over a back buffer magnified 1.5x selects 1.33" );
	tally.Check( Near( scale( 1.5f, 0.0f, 2816, 1694, 2816, 1700 ), 1.5f ),
	    "a presentation within 2% of 1:1 is treated as 1:1" );
	tally.Check( Near( scale( 1.5f, 2.0f, 2816, 1694, 2816, 1694 ), 2.0f ),
	    "ui_scale overrides the display scale" );
	tally.Check( Near( scale( 1.5f, 0.75f, 2816, 1694, 2816, 1694 ), 0.75f ),
	    "ui_scale can make the UI smaller than the display scale" );
	tally.Check( Near( scale( 0.0f, 0.0f, 1920, 1080, 0, 0 ), 1.0f ),
	    "an unknown display scale and window select scale 1" );
	tally.Check( Near( scale( 1.5f, 0.0f, 0, 0, 0, 0 ), 1.5f ),
	    "before a video mode exists the display scale applies" );
	tally.Check( Near( scale( 1.0f, 0.1f, 1920, 1080, 1920, 1080 ), uiscale::kMinScale ),
	    "ui_scale is limited below" );
	tally.Check( Near( scale( 1.0f, 9.0f, 7680, 4320, 7680, 4320 ), uiscale::kMaxScale ),
	    "ui_scale is limited above" );
	tally.Check( Near( scale( 2.0f, 0.0f, 1280, 720, 1280, 720 ), 1.5f ),
	    "a scale never leaves fewer than 480 UI units of height" );
	tally.Check( Near( scale( 1.0f, 3.0f, 1440, 1440, 1440, 1440 ), 2.25f ),
	    "a scale never leaves fewer than 640 UI units of width" );
	tally.Check( Near( scale( 1.5f, 0.0f, 600, 400, 600, 400 ), 1.0f ),
	    "a screen smaller than the UI minimum stays unscaled" );
	tally.Check( Near( scale( 1.5f, 0.75f, 600, 400, 600, 400 ), 0.75f ),
	    "a small screen still honours a smaller ui_scale" );
	tally.Check( Near( scale( 1.0f, 1.337f, 2560, 1440, 2560, 1440 ), 1.34f ),
	    "scales are quantized to hundredths" );
}

// Screen extents: the UI covers every pixel, without a spare unit, and is exact
// at scale 1.
inline void CheckExtents( const Policy &policy, Tally &tally )
{
	tally.Check( policy.unitsCoveringPixels( 2816, 1.5f ) == 1878 &&
	                 policy.unitsCoveringPixels( 1694, 1.5f ) == 1130,
	    "a 2816x1694 screen at 1.5 is 1878x1130 UI units" );
	tally.Check( policy.unitsCoveringPixels( 2880, 1.5f ) == 1920 &&
	                 policy.unitsCoveringPixels( 1620, 1.5f ) == 1080,
	    "a 2880x1620 screen at 1.5 is exactly 1920x1080 UI units" );

	static const float kScales[] = { 0.5f, 0.75f, 1.0f, 1.1f, 1.25f, 1.33f, 1.5f, 1.75f, 2.0f,
		2.25f, 2.5f, 3.0f, 4.0f };
	bool covers = true, tight = true, exact = true;
	for ( float s : kScales )
	{
		for ( int pixels = 1; pixels <= 4096; ++pixels )
		{
			const int units = policy.unitsCoveringPixels( pixels, s );
			// Within a hundredth of a pixel: the policy tolerates float error.
			if ( units * s < pixels - 1e-2f )
				covers = false;
			if ( ( units - 1 ) * s >= pixels - 1e-4f )
				tight = false;
			if ( s == 1.0f && units != pixels )
				exact = false;
		}
	}
	tally.Check( covers, "UnitsCoveringPixels reaches the last pixel" );
	tally.Check( tight, "UnitsCoveringPixels adds no spare unit" );
	tally.Check( exact, "UnitsCoveringPixels is the identity at scale 1" );
}

// Cursor coordinates: a pixel maps to the unit that contains it, and warping to
// a unit lands on a pixel inside it.
inline void CheckCoordinates( const Policy &policy, Tally &tally )
{
	static const float kScales[] = { 1.0f, 1.1f, 1.25f, 1.33f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f,
		4.0f };
	bool contains = true, roundTrip = true, identity = true;
	for ( float s : kScales )
	{
		for ( int pixel = -8; pixel <= 4096; ++pixel )
		{
			const int unit = policy.unitAtPixel( pixel, s );
			if ( unit * s > pixel + 1e-3f || ( unit + 1 ) * s <= pixel + 1e-3f )
				contains = false;
			if ( s == 1.0f && unit != pixel )
				identity = false;
		}
		for ( int unit = -4; unit <= 3000; ++unit )
		{
			if ( policy.unitAtPixel( policy.pixelAtUnit( unit, s ), s ) != unit )
				roundTrip = false;
		}
	}
	tally.Check( contains, "UnitAtPixel returns the unit containing the pixel" );
	tally.Check( roundTrip, "warping to a unit's pixel reads back the same unit" );
	tally.Check( identity, "cursor coordinates are unchanged at scale 1" );
}

// Font sizes: a UI font size rasterizes at the nearest pixel size and its
// metrics read back as the same UI size.
inline void CheckFontSizes( const Policy &policy, Tally &tally )
{
	tally.Check( policy.unitsToPixelsRounded( 13, 1.5f ) == 20 &&
	                 policy.unitsToPixelsRounded( 12, 1.5f ) == 18 &&
	                 policy.unitsToPixelsRounded( 12, 1.25f ) == 15,
	    "font sizes rasterize at the nearest pixel size" );
	tally.Check( policy.pixelsToUnitsRounded( 20, 1.5f ) == 13 &&
	                 policy.pixelsToUnitsRounded( 7, 2.0f ) == 4,
	    "pixel metrics round to the nearest UI unit" );
	static const float kScales[] = { 1.0f, 1.1f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f, 4.0f };
	bool roundTrip = true;
	for ( float s : kScales )
	{
		for ( int tall = 1; tall <= 256; ++tall )
		{
			if ( policy.pixelsToUnitsRounded( policy.unitsToPixelsRounded( tall, s ), s ) != tall )
				roundTrip = false;
		}
	}
	tally.Check( roundTrip, "a font's UI size reads back from its pixel size" );
}

inline void CheckAll( const Policy &policy, Tally &tally )
{
	CheckScaleSelection( policy, tally );
	CheckExtents( policy, tally );
	CheckCoordinates( policy, tally );
	CheckFontSizes( policy, tally );
}

inline Policy RealPolicy()
{
	Policy policy;
	policy.computeScale = uiscale::ComputeScale;
	policy.unitsCoveringPixels = uiscale::UnitsCoveringPixels;
	policy.unitAtPixel = uiscale::UnitAtPixel;
	policy.pixelAtUnit = uiscale::PixelAtUnit;
	policy.pixelsToUnitsRounded = uiscale::PixelsToUnitsRounded;
	policy.unitsToPixelsRounded = uiscale::UnitsToPixelsRounded;
	return policy;
}

} // namespace uiscaletest

#endif // UNITTESTS_VGUITEST_UI_SCALE_CHECKS_H
