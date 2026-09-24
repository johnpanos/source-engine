//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the UI scale oracle (ui_scale_checks.h). Each broken
//          policy differs from the real one in one plausible way; the oracle
//          must fail it while the real policy passes.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/vguitest/ui_scale_checks.h"

#include <cmath>
#include <cstdio>

namespace
{

// Ignores how the back buffer is presented (a magnified back buffer would get
// the display scale twice).
float ScaleIgnoringPresentation( const uiscale::Inputs &inputs )
{
	uiscale::Inputs unpresented = inputs;
	unpresented.drawableWide = unpresented.drawableTall = 0;
	return uiscale::ComputeScale( unpresented );
}

// Lets the display scale win over the user's override.
float ScaleIgnoringOverride( const uiscale::Inputs &inputs )
{
	uiscale::Inputs noOverride = inputs;
	noOverride.userScale = 0.0f;
	return uiscale::ComputeScale( noOverride );
}

// Truncates the screen extent, leaving the last pixels outside the UI.
int TruncatedExtent( int pixels, float scale )
{
	return static_cast<int>( pixels / scale );
}

// Warps to the nearest pixel of the unit's origin, which can belong to the
// previous unit.
int NearestPixelAtUnit( int unit, float scale )
{
	return static_cast<int>( std::floor( unit * scale + 0.5f ) );
}

// Rasterizes fonts at the truncated pixel size.
int TruncatedFontPixels( int units, float scale )
{
	return static_cast<int>( units * scale );
}

bool OracleRejects( const char *name, const uiscaletest::Policy &policy )
{
	uiscaletest::Tally tally;
	tally.verbose = false;
	uiscaletest::CheckAll( policy, tally );
	std::printf( "%s: %lu of %lu checks failed\n", name, tally.failures, tally.checks );
	return tally.failures > 0;
}

} // namespace

int main()
{
	unsigned long checks = 0, failures = 0;
	const auto expect = [&]( bool condition, const char *what ) {
		++checks;
		if ( !condition )
		{
			++failures;
			std::printf( "FAIL: %s\n", what );
		}
	};

	const uiscaletest::Policy real = uiscaletest::RealPolicy();
	expect( !OracleRejects( "real policy", real ), "the real policy passes the oracle" );

	uiscaletest::Policy broken = real;
	broken.computeScale = ScaleIgnoringPresentation;
	expect( OracleRejects( "ignores presentation", broken ),
	    "a policy ignoring back buffer presentation is rejected" );

	broken = real;
	broken.computeScale = ScaleIgnoringOverride;
	expect( OracleRejects( "ignores ui_scale", broken ),
	    "a policy ignoring ui_scale is rejected" );

	broken = real;
	broken.unitsCoveringPixels = TruncatedExtent;
	expect( OracleRejects( "truncated extent", broken ),
	    "a truncated screen extent is rejected" );

	broken = real;
	broken.pixelAtUnit = NearestPixelAtUnit;
	expect( OracleRejects( "nearest-pixel warp", broken ),
	    "a cursor warp that reads back another unit is rejected" );

	broken = real;
	broken.unitsToPixelsRounded = TruncatedFontPixels;
	expect( OracleRejects( "truncated font size", broken ),
	    "a truncated font raster size is rejected" );

	return testing::ReportConformance( checks, failures );
}
