//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the gyro policy (inputsystem/gyro_math.h). The checks
//          take the policy as function pointers so the sensitivity suite can
//          run them against deliberately wrong policies.
//
//=============================================================================//

#ifndef UNITTESTS_INPUTTEST_GYRO_MATH_CHECKS_H
#define UNITTESTS_INPUTTEST_GYRO_MATH_CHECKS_H

#include "inputsystem/gyro_math.h"

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
};

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

inline void CheckAll( const Policy &policy, Tally &tally )
{
	CheckSamplePeriod( policy, tally );
	CheckDisplayRotation( policy, tally );
	CheckDeviceToScreen( policy, tally );
	CheckIntegration( policy, tally );
}

} // namespace gyrotest

#endif // UNITTESTS_INPUTTEST_GYRO_MATH_CHECKS_H
