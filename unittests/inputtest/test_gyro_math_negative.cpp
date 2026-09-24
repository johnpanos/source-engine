//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the gyro oracle (gyro_math_checks.h). Each broken
//          policy differs from the real one in one plausible way; the oracle
//          must fail it while the real policy passes.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/inputtest/gyro_math_checks.h"

#include <cmath>
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

// A fixed weight per sample: the smoothing then depends on the sample rate.
void FilterUpPerSample(
    double flTimeConstant, const double *seconds, const float ( *samples )[3], int count, float up[3] )
{
	bool bHave = false;
	for ( int i = 0; i < count; ++i )
	{
		if ( seconds[i] < 0.0 )
		{
			bHave = false;
			continue;
		}
		const float flWeight = ( !bHave || flTimeConstant <= 0.0 ) ? 1.f : 0.06f;
		for ( int axis = 0; axis < 3; ++axis )
			up[axis] += flWeight * ( samples[i][axis] - up[axis] );
		bHave = true;
	}
}

// Player space that ignores which way is up (plain yaw).
float TurnIgnoringUp( int nAxis, const float rotation[3], const float *up )
{
	if ( nAxis == gyroaim::TURN_PLAYER_SPACE )
		return rotation[1];
	return gyroaim::Turn( nAxis, rotation, up );
}

// Player space without the relax factor (world yaw only).
float TurnWithoutRelax( int nAxis, const float rotation[3], const float *up )
{
	if ( nAxis != gyroaim::TURN_PLAYER_SPACE || !up )
		return gyroaim::Turn( nAxis, rotation, up );
	const float flLength = std::sqrt( up[0] * up[0] + up[1] * up[1] + up[2] * up[2] );
	if ( flLength <= 0.f )
		return rotation[1];
	return ( rotation[1] * up[1] + rotation[2] * up[2] ) / flLength;
}

// Player space with the relax factor but no cap from the local rotation.
float TurnWithoutCap( int nAxis, const float rotation[3], const float *up )
{
	return TurnWithoutRelax( nAxis, rotation, up ) *
	       ( nAxis == gyroaim::TURN_PLAYER_SPACE && up ? gyroaim::kPlayerSpaceRelax : 1.f );
}

// A hard deadzone instead of a smooth scale.
void TightenDeadzone( float &turn, float &pitch, float flSeconds, float flThreshold )
{
	if ( flSeconds <= 0.f || flThreshold <= 0.f )
		return;
	if ( std::sqrt( turn * turn + pitch * pitch ) / flSeconds < flThreshold )
		turn = pitch = 0.f;
}

// Each axis tightened on its own speed.
void TightenPerAxis( float &turn, float &pitch, float flSeconds, float flThreshold )
{
	float zero = 0.f;
	gyroaim::Tighten( turn, zero, flSeconds, flThreshold );
	zero = 0.f;
	gyroaim::Tighten( pitch, zero, flSeconds, flThreshold );
}

// Tightens by angle per call, as if every call were one 120 Hz frame.
void TightenByAngle( float &turn, float &pitch, float flSeconds, float flThreshold )
{
	if ( flSeconds <= 0.f )
		return;
	gyroaim::Tighten( turn, pitch, 1.f / 120.f, flThreshold );
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

	broken = real;
	broken.filterUp = FilterUpPerSample;
	expect( OracleRejects( "up filter per sample", broken ),
	    "smoothing up by a fixed weight per sample is rejected" );

	broken = real;
	broken.turn = TurnIgnoringUp;
	expect( OracleRejects( "player space ignores up", broken ),
	    "player space that ignores which way is up is rejected" );

	broken = real;
	broken.turn = TurnWithoutRelax;
	expect( OracleRejects( "player space without relax", broken ),
	    "player space without the 45 degree relax is rejected" );

	broken = real;
	broken.turn = TurnWithoutCap;
	expect( OracleRejects( "player space without cap", broken ),
	    "player space that can turn further than the device is rejected" );

	broken = real;
	broken.tighten = TightenDeadzone;
	expect( OracleRejects( "tightening as a deadzone", broken ),
	    "a hard deadzone instead of tightening is rejected" );

	broken = real;
	broken.tighten = TightenPerAxis;
	expect( OracleRejects( "tightening per axis", broken ),
	    "tightening each axis on its own speed is rejected" );

	broken = real;
	broken.tighten = TightenByAngle;
	expect( OracleRejects( "tightening by angle", broken ),
	    "tightening by angle per read instead of speed is rejected" );

	return testing::ReportConformance( checks, failures );
}
