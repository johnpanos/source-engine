//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the gyro oracle (gyro_math_checks.h). Each broken
//          policy differs from the real one in one plausible way; the oracle
//          must fail it while the real policy passes.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/inputtest/gyro_math_checks.h"

#include <cstdio>

namespace
{

// Samples once per refresh instead of twice.
int PeriodOncePerRefresh( float flRefreshHz )
{
	return gyro::SamplePeriodUs( flRefreshHz ) * 2;
}

// Ignores a landscape-natural display (tablets get the wrong axes).
int RotationIgnoringNaturalOrientation( int nDegreesFromPortrait, bool )
{
	return gyro::DisplayRotation( nDegreesFromPortrait, false );
}

// Uses the device axes as the screen axes in every orientation.
void RemapIgnoringRotation( const float device[3], int, float screen[3] )
{
	gyro::DeviceToScreen( device, 0, screen );
}

// Swaps the two landscape orientations (a sign error on both axes).
void RemapSwappingLandscapes( const float device[3], int nRotation, float screen[3] )
{
	if ( nRotation == 90 || nRotation == 270 )
		nRotation = 360 - nRotation;
	gyro::DeviceToScreen( device, nRotation, screen );
}

// Rectangle rule with each interval's closing rate.
void IntegrateRectangle( const gyrotest::Sample *samples, int count, double rotation[3] )
{
	for ( int i = 1; i < count; ++i )
	{
		double seconds = ( samples[i].timestampNs - samples[i - 1].timestampNs ) * 1e-9;
		if ( samples[i].resetBefore || seconds <= 0.0 || seconds > gyro::kMaxSampleGapSeconds )
			continue;
		for ( int axis = 0; axis < 3; ++axis )
			rotation[axis] += samples[i].rate[axis] * seconds;
	}
}

// Integrates across gaps and sensor restarts with the last rate.
void IntegrateAcrossGaps( const gyrotest::Sample *samples, int count, double rotation[3] )
{
	for ( int i = 1; i < count; ++i )
	{
		double seconds = ( samples[i].timestampNs - samples[i - 1].timestampNs ) * 1e-9;
		if ( seconds <= 0.0 )
			continue;
		for ( int axis = 0; axis < 3; ++axis )
			rotation[axis] += 0.5 * ( samples[i - 1].rate[axis] + samples[i].rate[axis] ) * seconds;
	}
}

bool OracleRejects( const char *name, const gyrotest::Policy &policy )
{
	gyrotest::Tally tally;
	tally.verbose = false;
	gyrotest::CheckAll( policy, tally );
	std::printf( "%s: %lu of %lu checks failed\n", name, tally.failures, tally.checks );
	return tally.failures > 0;
}

} // namespace

int main()
{
	unsigned long checks = 0, failures = 0;
	const auto expect = [&]( bool condition, const char *what )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			std::printf( "FAIL: %s\n", what );
		}
	};

	const gyrotest::Policy real = gyrotest::RealPolicy();
	expect( !OracleRejects( "real policy", real ), "the real policy passes the oracle" );

	gyrotest::Policy broken = real;
	broken.samplePeriodUs = PeriodOncePerRefresh;
	expect( OracleRejects( "once per refresh", broken ),
	    "sampling once per refresh instead of twice is rejected" );

	broken = real;
	broken.displayRotation = RotationIgnoringNaturalOrientation;
	expect( OracleRejects( "ignores natural orientation", broken ),
	    "ignoring a landscape-natural display is rejected" );

	broken = real;
	broken.deviceToScreen = RemapIgnoringRotation;
	expect( OracleRejects( "ignores rotation", broken ),
	    "a remap ignoring the display rotation is rejected" );

	broken = real;
	broken.deviceToScreen = RemapSwappingLandscapes;
	expect( OracleRejects( "swapped landscapes", broken ),
	    "a remap with the landscape orientations swapped is rejected" );

	broken = real;
	broken.integrate = IntegrateRectangle;
	expect( OracleRejects( "rectangle rule", broken ), "rectangle-rule integration is rejected" );

	broken = real;
	broken.integrate = IntegrateAcrossGaps;
	expect( OracleRejects( "integrates across gaps", broken ),
	    "integrating across gaps and sensor restarts is rejected" );

	return testing::ReportConformance( checks, failures );
}
