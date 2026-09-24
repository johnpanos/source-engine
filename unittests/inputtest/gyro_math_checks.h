//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the gyro policy (inputsystem/gyro_math.h) and the gyro
//          aiming policy (public/inputsystem/gyro_aim.h). The checks take the
//          policy as function pointers so the sensitivity suite can run them
//          against deliberately wrong policies.
//
//=============================================================================//

#ifndef UNITTESTS_INPUTTEST_GYRO_MATH_CHECKS_H
#define UNITTESTS_INPUTTEST_GYRO_MATH_CHECKS_H

#include "inputsystem/gyro_math.h"
#include "inputsystem/gyro_aim.h"

#include <cmath>
#include <cstdio>
#include <limits>

namespace gyrotest
{

struct Sample
{
	int64_t timestampNs;
	float rate[3];
	bool resetBefore; // the sensor was re-registered before this sample
};

struct Policy
{
	int ( *samplePeriodUs )( float );
	int ( *displayRotation )( int, bool );
	void ( *deviceToScreen )( const float[3], int, float[3] );
	void ( *integrate )( const Sample *, int, double[3] );
	// Filters samples at the given times (seconds) through a CUpFilter-like
	// policy with the time constant, writing the last output to up[3].
	void ( *filterUp )( double, const double *, const float ( * )[3], int, float[3] );
	float ( *turn )( int, const float[3], const float * );
	void ( *tighten )( float &, float &, float, float );
};

inline void FilterUpReal( double flTimeConstant, const double *seconds, const float ( *samples )[3],
    int count, float up[3] )
{
	gyro::CUpFilter filter( flTimeConstant );
	for ( int i = 0; i < count; ++i )
	{
		if ( seconds[i] < 0.0 )
		{
			// A negative time marks a Reset before the next sample.
			filter.Reset();
			continue;
		}
		filter.AddSample(
		    static_cast<int64_t>( std::llround( seconds[i] * 1e9 ) ), samples[i], up );
	}
}

inline void IntegrateReal( const Sample *samples, int count, double rotation[3] )
{
	gyro::CRateIntegrator integrator;
	for ( int i = 0; i < count; ++i )
	{
		if ( samples[i].resetBefore )
			integrator.Reset();
		integrator.AddSample( samples[i].timestampNs, samples[i].rate, rotation );
	}
}

inline Policy RealPolicy()
{
	Policy policy;
	policy.samplePeriodUs = gyro::SamplePeriodUs;
	policy.displayRotation = gyro::DisplayRotation;
	policy.deviceToScreen = gyro::DeviceToScreen;
	policy.integrate = IntegrateReal;
	policy.filterUp = FilterUpReal;
	policy.turn = gyroaim::Turn;
	policy.tighten = gyroaim::Tighten;
	return policy;
}

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

inline bool Near( double a, double b, double tolerance = 1e-6 )
{
	return std::fabs( a - b ) < tolerance;
}

// Two samples per displayed frame; an unknown refresh rate is taken as 60 Hz.
inline void CheckSamplePeriod( const Policy &policy, Tally &tally )
{
	tally.Check( policy.samplePeriodUs( 120.f ) == 4167, "120 Hz samples at 240 Hz (4167 us)" );
	tally.Check( policy.samplePeriodUs( 60.f ) == 8333, "60 Hz samples at 120 Hz (8333 us)" );
	tally.Check( policy.samplePeriodUs( 90.f ) == 5556, "90 Hz samples at 180 Hz (5556 us)" );
	tally.Check( policy.samplePeriodUs( 144.f ) == 3472, "144 Hz samples at 288 Hz (3472 us)" );
	tally.Check( policy.samplePeriodUs( 0.f ) == 8333, "an unknown refresh rate uses 60 Hz" );
	tally.Check( policy.samplePeriodUs( -1.f ) == 8333, "a negative refresh rate uses 60 Hz" );
	tally.Check( policy.samplePeriodUs( std::numeric_limits<float>::quiet_NaN() ) == 8333,
	    "a NaN refresh rate uses 60 Hz" );
}

// SDL reports orientation relative to portrait; Android's rotation is relative
// to the display's natural orientation.
inline void CheckDisplayRotation( const Policy &policy, Tally &tally )
{
	tally.Check( policy.displayRotation( 0, false ) == 0, "portrait on a phone is rotation 0" );
	tally.Check( policy.displayRotation( 90, false ) == 90, "landscape on a phone is rotation 90" );
	tally.Check( policy.displayRotation( 180, false ) == 180,
	    "flipped portrait on a phone is rotation 180" );
	tally.Check( policy.displayRotation( 270, false ) == 270,
	    "flipped landscape on a phone is rotation 270" );
	tally.Check( policy.displayRotation( 90, true ) == 0,
	    "landscape on a landscape-natural tablet is rotation 0" );
	tally.Check( policy.displayRotation( 180, true ) == 90,
	    "flipped portrait on a landscape-natural tablet is rotation 90" );
	tally.Check( policy.displayRotation( 270, true ) == 180,
	    "flipped landscape on a landscape-natural tablet is rotation 180" );
	tally.Check( policy.displayRotation( 0, true ) == 270,
	    "portrait on a landscape-natural tablet is rotation 270" );
}

inline void Screen( const Policy &policy, float x, float y, float z, int rotation, float out[3] )
{
	const float device[3] = { x, y, z };
	policy.deviceToScreen( device, rotation, out );
}

inline bool ScreenIs( const float screen[3], float pitch, float yaw, float roll )
{
	return Near( screen[0], pitch ) && Near( screen[1], yaw ) && Near( screen[2], roll );
}

// Each case is derived from how the device is physically held, independently
// of the remap table: in landscape (rotation 90) the device's top edge points
// left, so screen-up is device +x and screen-right is device -y.
inline void CheckDeviceToScreen( const Policy &policy, Tally &tally )
{
	float s[3];

	Screen( policy, 0.f, 1.f, 0.f, 0, s );
	tally.Check( ScreenIs( s, 0.f, 1.f, 0.f ), "portrait: turning about device y is yaw" );
	Screen( policy, 1.f, 0.f, 0.f, 0, s );
	tally.Check( ScreenIs( s, 1.f, 0.f, 0.f ), "portrait: tilting about device x is pitch" );

	Screen( policy, 1.f, 0.f, 0.f, 90, s );
	tally.Check( ScreenIs( s, 0.f, 1.f, 0.f ),
	    "landscape: turning left (about device +x, screen-up) is positive yaw" );
	Screen( policy, 0.f, -1.f, 0.f, 90, s );
	tally.Check( ScreenIs( s, 1.f, 0.f, 0.f ),
	    "landscape: tilting the top edge back (about device -y, screen-right) looks up" );

	Screen( policy, -1.f, 0.f, 0.f, 270, s );
	tally.Check( ScreenIs( s, 0.f, 1.f, 0.f ),
	    "flipped landscape: turning left (about device -x) is positive yaw" );
	Screen( policy, 0.f, 1.f, 0.f, 270, s );
	tally.Check( ScreenIs( s, 1.f, 0.f, 0.f ),
	    "flipped landscape: tilting the top edge back (about device +y) looks up" );

	Screen( policy, 0.f, -1.f, 0.f, 180, s );
	tally.Check( ScreenIs( s, 0.f, 1.f, 0.f ), "upside-down portrait: device -y is screen-up" );
	Screen( policy, -1.f, 0.f, 0.f, 180, s );
	tally.Check( ScreenIs( s, 1.f, 0.f, 0.f ), "upside-down portrait: device -x is screen-right" );

	const int rotations[] = { 0, 90, 180, 270 };
	for ( int r = 0; r < 4; ++r )
	{
		Screen( policy, 0.f, 0.f, 1.f, rotations[r], s );
		tally.Check( ScreenIs( s, 0.f, 0.f, 1.f ), "roll about the screen normal is unchanged" );

		// A rotation of axes preserves the rotation's magnitude.
		Screen( policy, 0.3f, -0.4f, 1.2f, rotations[r], s );
		const double length = std::sqrt( s[0] * s[0] + s[1] * s[1] + s[2] * s[2] );
		tally.Check( Near( length, 1.3, 1e-5 ), "the remap preserves the rotation's magnitude" );
	}
}

inline Sample MakeSample( double seconds, float x, float y, float z, bool resetBefore = false )
{
	Sample sample;
	sample.timestampNs = static_cast<int64_t>( std::llround( seconds * 1e9 ) );
	sample.rate[0] = x;
	sample.rate[1] = y;
	sample.rate[2] = z;
	sample.resetBefore = resetBefore;
	return sample;
}

inline void CheckIntegration( const Policy &policy, Tally &tally )
{
	const int kRate = 240;
	Sample samples[kRate + 1];

	// A constant rate for one second at 240 Hz.
	for ( int i = 0; i <= kRate; ++i )
		samples[i] = MakeSample( double( i ) / kRate, 1.f, 2.f, -3.f );
	double rotation[3] = { 0.0, 0.0, 0.0 };
	policy.integrate( samples, kRate + 1, rotation );
	tally.Check( Near( rotation[0], 1.0 ) && Near( rotation[1], 2.0 ) && Near( rotation[2], -3.0 ),
	    "a constant rate integrates to rate times time on each axis" );

	// A linear ramp: the trapezoid rule is exact.
	for ( int i = 0; i <= 100; ++i )
		samples[i] = MakeSample( i / 100.0, float( i / 100.0 ), 0.f, 0.f );
	rotation[0] = rotation[1] = rotation[2] = 0.0;
	policy.integrate( samples, 101, rotation );
	tally.Check( Near( rotation[0], 0.5 ), "a rate ramping 0 to 1 rad/s over 1 s turns 0.5 rad" );

	// One sample carries no interval.
	rotation[0] = 0.0;
	samples[0] = MakeSample( 5.0, 10.f, 0.f, 0.f );
	policy.integrate( samples, 1, rotation );
	tally.Check( Near( rotation[0], 0.0 ), "a single sample turns nothing" );

	// A gap longer than the limit is not integrated across.
	rotation[0] = 0.0;
	samples[0] = MakeSample( 0.0, 1.f, 0.f, 0.f );
	samples[1] = MakeSample( 0.5, 1.f, 0.f, 0.f );
	samples[2] = MakeSample( 0.51, 1.f, 0.f, 0.f );
	policy.integrate( samples, 3, rotation );
	tally.Check( Near( rotation[0], 0.01 ), "a 0.5 s gap is skipped; later samples still count" );

	// Duplicate and backwards timestamps add nothing.
	rotation[0] = 0.0;
	samples[0] = MakeSample( 1.0, 1.f, 0.f, 0.f );
	samples[1] = MakeSample( 1.0, 1.f, 0.f, 0.f );
	samples[2] = MakeSample( 0.99, 1.f, 0.f, 0.f );
	policy.integrate( samples, 3, rotation );
	tally.Check( Near( rotation[0], 0.0 ), "duplicate or backwards timestamps turn nothing" );

	// A reset (the sensor was re-registered) starts a new interval.
	rotation[0] = 0.0;
	samples[0] = MakeSample( 0.0, 1.f, 0.f, 0.f );
	samples[1] = MakeSample( 0.05, 1.f, 0.f, 0.f, true );
	samples[2] = MakeSample( 0.06, 1.f, 0.f, 0.f );
	policy.integrate( samples, 3, rotation );
	tally.Check( Near( rotation[0], 0.01 ), "no rotation is integrated across a sensor restart" );
}

// The direction away from gravity is smoothed by the samples' own times.
inline void CheckUpFilter( const Policy &policy, Tally &tally )
{
	const int kMax = 1024;
	static double seconds[kMax];
	static float samples[kMax][3];
	float up[3];

	// Without a time constant each sample passes through.
	seconds[0] = 0.0;
	samples[0][0] = 0.f, samples[0][1] = 9.8f, samples[0][2] = 0.f;
	seconds[1] = 0.01;
	samples[1][0] = 1.f, samples[1][1] = 2.f, samples[1][2] = 3.f;
	policy.filterUp( 0.0, seconds, samples, 2, up );
	tally.Check( Near( up[0], 1.0 ) && Near( up[1], 2.0 ) && Near( up[2], 3.0 ),
	    "a zero time constant passes samples through" );

	// The first sample is taken as is, whatever the time constant.
	policy.filterUp( 0.15, seconds, samples, 1, up );
	tally.Check( Near( up[1], 9.8, 1e-5 ), "the first sample is taken as is" );

	// A step from 0 to 1 held for one time constant reaches 1 - 1/e, at any
	// sample rate: 100 Hz and 400 Hz agree.
	const int rates[] = { 100, 400 };
	for ( int r = 0; r < 2; ++r )
	{
		const double tau = 0.15;
		const int steps = static_cast<int>( tau * rates[r] + 0.5 );
		seconds[0] = 0.0;
		samples[0][0] = samples[0][1] = samples[0][2] = 0.f;
		for ( int i = 1; i <= steps; ++i )
		{
			seconds[i] = double( i ) / rates[r];
			samples[i][0] = 0.f, samples[i][1] = 1.f, samples[i][2] = 0.f;
		}
		policy.filterUp( tau, seconds, samples, steps + 1, up );
		tally.Check( Near( up[1], 1.0 - std::exp( -1.0 ), 1e-3 ),
		    r == 0 ? "a step reaches 1 - 1/e after one time constant (100 Hz)"
		           : "a step reaches 1 - 1/e after one time constant (400 Hz)" );
	}

	// After Reset or a long gap the next sample is taken as is.
	seconds[0] = 0.0;
	samples[0][0] = samples[0][1] = samples[0][2] = 0.f;
	seconds[1] = -1.0;
	seconds[2] = 0.01;
	samples[2][0] = 0.f, samples[2][1] = 5.f, samples[2][2] = 0.f;
	policy.filterUp( 0.15, seconds, samples, 3, up );
	tally.Check( Near( up[1], 5.0, 1e-5 ), "the first sample after Reset is taken as is" );
	seconds[1] = 0.5;
	samples[1][0] = 0.f, samples[1][1] = 5.f, samples[1][2] = 0.f;
	policy.filterUp( 0.15, seconds, samples, 2, up );
	tally.Check( Near( up[1], 5.0, 1e-5 ), "a sample after a 0.5 s gap is taken as is" );

	// A duplicate or backwards timestamp changes nothing.
	seconds[0] = 1.0;
	samples[0][0] = 0.f, samples[0][1] = 1.f, samples[0][2] = 0.f;
	seconds[1] = 0.99;
	samples[1][0] = 0.f, samples[1][1] = 7.f, samples[1][2] = 0.f;
	seconds[2] = 0.99;
	samples[2][0] = 0.f, samples[2][1] = 7.f, samples[2][2] = 0.f;
	policy.filterUp( 0.15, seconds, samples, 3, up );
	tally.Check( Near( up[1], 1.0, 1e-5 ), "backwards or duplicate timestamps change nothing" );
}

inline bool NearF( float a, float b, float tolerance = 1e-5f )
{
	return std::fabs( a - b ) < tolerance;
}

// Turning. The cases come from how the device is held, independently of the
// formula: rotation and up are in screen axes (right, up, out of the screen).
inline void CheckTurn( const Policy &policy, Tally &tally )
{
	const float kHalf = 0.70710678f;
	const float upright[3] = { 0.f, 9.81f, 0.f };  // screen vertical, as held at eye level
	const float flat[3] = { 0.f, 0.f, 9.81f };     // lying face up
	const float tilted[3] = { 0.f, kHalf, kHalf }; // top edge tilted 45 degrees back
	const int P = gyroaim::TURN_PLAYER_SPACE;

	// Screen upright: turning the body is yaw; steering (roll) does not turn.
	const float yawLeft[3] = { 0.f, 0.5f, 0.f };
	const float rollLeft[3] = { 0.f, 0.f, 0.5f };
	tally.Check( NearF( policy.turn( P, yawLeft, upright ), 0.5f ),
	    "player space, upright: turning the device left turns left by as much" );
	tally.Check( NearF( policy.turn( P, rollLeft, upright ), 0.f ),
	    "player space, upright: steering the device does not turn" );

	// Lying flat: spinning the device on the table (roll) is the turn.
	tally.Check( NearF( policy.turn( P, rollLeft, flat ), 0.5f ),
	    "player space, flat: spinning the device counter-clockwise turns left" );
	tally.Check( NearF( policy.turn( P, yawLeft, flat ), 0.f ),
	    "player space, flat: rotation about screen-up (now horizontal) does not turn" );

	// Tilted back 45 degrees, the player turns their body 0.4 rad about world
	// up: the device sees it split between yaw and roll, and the view turns by
	// all of it (yaw alone would give 71%).
	const float bodyLeft[3] = { 0.f, 0.4f * kHalf, 0.4f * kHalf };
	const float bodyRight[3] = { 0.f, -0.4f * kHalf, -0.4f * kHalf };
	tally.Check( NearF( policy.turn( P, bodyLeft, tilted ), 0.4f ),
	    "player space, tilted: turning the body left turns the view by the full angle" );
	tally.Check( NearF( policy.turn( P, bodyRight, tilted ), -0.4f ),
	    "player space, tilted: turning the body right turns right" );
	tally.Check( NearF( policy.turn( gyroaim::TURN_YAW, bodyLeft, tilted ), 0.4f * kHalf ),
	    "yaw mode, tilted: only the yaw component turns" );

	// Tilted back 45 degrees, twisting the device about its own screen-up axis
	// still turns by (almost) the full amount: the 45 degree relax.
	const float twist[3] = { 0.f, 0.3f, 0.f };
	const float twistTurn = policy.turn( P, twist, tilted );
	tally.Check( twistTurn > 0.3f * 0.99f && twistTurn <= 0.3f + 1e-6f,
	    "player space, tilted: twisting about the device's own up axis turns fully" );

	// Pitching never turns, whichever way is up.
	const float pitchUp[3] = { 0.6f, 0.f, 0.f };
	const float sideways[3] = { 9.81f, 0.f, 0.f };
	tally.Check( NearF( policy.turn( P, pitchUp, tilted ), 0.f ) &&
	                 NearF( policy.turn( P, pitchUp, sideways ), 0.f ),
	    "player space: pitching the device does not turn" );

	// A device tilted sideways reduces the world-up share of its yaw.
	const float leaning[3] = { 0.5f, 0.8660254f, 0.f }; // rolled 30 degrees
	tally.Check( NearF( policy.turn( P, yawLeft, leaning ), 0.5f ),
	    "player space, rolled 30 degrees: the relax keeps a full turn" );

	// Only the direction of up matters.
	const float tiltedLong[3] = { 0.f, 9.81f * kHalf, 9.81f * kHalf };
	tally.Check(
	    NearF( policy.turn( P, bodyLeft, tiltedLong ), policy.turn( P, bodyLeft, tilted ) ),
	    "player space does not depend on the length of up" );

	// Without a known up, player space turns like yaw mode.
	const float zero[3] = { 0.f, 0.f, 0.f };
	tally.Check( NearF( policy.turn( P, bodyLeft, NULL ), 0.4f * kHalf ) &&
	                 NearF( policy.turn( P, bodyLeft, zero ), 0.4f * kHalf ),
	    "player space without up falls back to yaw" );

	// The other modes.
	const float both[3] = { 0.1f, 0.2f, 0.3f };
	tally.Check(
	    NearF( policy.turn( gyroaim::TURN_YAW, both, flat ), 0.2f ), "yaw mode turns by yaw" );
	tally.Check(
	    NearF( policy.turn( gyroaim::TURN_ROLL, both, flat ), 0.3f ), "roll mode turns by roll" );
	tally.Check( NearF( policy.turn( gyroaim::TURN_YAW_AND_ROLL, both, flat ), 0.5f ),
	    "yaw-and-roll mode adds them" );
	tally.Check( NearF( policy.turn( 99, both, flat ), 0.2f ), "an unknown mode turns by yaw" );
}

// Tightening: slow rotation is scaled down smoothly, fast rotation is not.
inline void CheckTighten( const Policy &policy, Tally &tally )
{
	const float kThreshold = 0.02f; // rad/s
	const float kFrame = 1.f / 120.f;
	float turn, pitch;

	// Fast aiming is untouched.
	turn = 0.5f * kFrame, pitch = -0.2f * kFrame;
	policy.tighten( turn, pitch, kFrame, kThreshold );
	tally.Check( NearF( turn, 0.5f * kFrame, 1e-9f ) && NearF( pitch, -0.2f * kFrame, 1e-9f ),
	    "rotation above the threshold is unchanged" );

	// Half the threshold speed is scaled by one half, keeping direction.
	turn = 0.6f * 0.5f * kThreshold * kFrame, pitch = 0.8f * 0.5f * kThreshold * kFrame;
	policy.tighten( turn, pitch, kFrame, kThreshold );
	tally.Check( NearF( turn, 0.6f * 0.25f * kThreshold * kFrame, 1e-10f ) &&
	                 NearF( pitch, 0.8f * 0.25f * kThreshold * kFrame, 1e-10f ),
	    "half the threshold speed moves a quarter as far, in the same direction" );

	// The speed is of turn and pitch together: a fast turn keeps a slow pitch.
	turn = 3.f * kThreshold * kFrame, pitch = 0.5f * kThreshold * kFrame;
	policy.tighten( turn, pitch, kFrame, kThreshold );
	tally.Check( NearF( pitch, 0.5f * kThreshold * kFrame, 1e-10f ),
	    "the threshold applies to the combined speed, not each axis" );

	// No step at the threshold.
	turn = 0.999f * kThreshold * kFrame, pitch = 0.f;
	policy.tighten( turn, pitch, kFrame, kThreshold );
	tally.Check(
	    turn > 0.99f * kThreshold * kFrame, "just below the threshold is nearly unchanged" );

	// Speed, not angle: the same angle over a shorter interval is faster.
	turn = 0.5f * kThreshold * kFrame, pitch = 0.f;
	policy.tighten( turn, pitch, kFrame / 4.f, kThreshold );
	tally.Check( NearF( turn, 0.5f * kThreshold * kFrame, 1e-10f ),
	    "an angle made in a quarter of the time is fast enough to pass" );

	// Nothing to scale, or nothing to scale by.
	turn = 0.f, pitch = 0.f;
	policy.tighten( turn, pitch, kFrame, kThreshold );
	tally.Check( turn == 0.f && pitch == 0.f, "no rotation stays none" );
	turn = 0.001f, pitch = 0.f;
	policy.tighten( turn, pitch, 0.f, kThreshold );
	tally.Check( turn == 0.001f, "a zero interval leaves the rotation unchanged" );
	policy.tighten( turn, pitch, kFrame, 0.f );
	tally.Check( turn == 0.001f, "a zero threshold turns tightening off" );
}

inline void CheckAll( const Policy &policy, Tally &tally )
{
	CheckSamplePeriod( policy, tally );
	CheckDisplayRotation( policy, tally );
	CheckDeviceToScreen( policy, tally );
	CheckIntegration( policy, tally );
	CheckUpFilter( policy, tally );
	CheckTurn( policy, tally );
	CheckTighten( policy, tally );
}

} // namespace gyrotest

#endif // UNITTESTS_INPUTTEST_GYRO_MATH_CHECKS_H
