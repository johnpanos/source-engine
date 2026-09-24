//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyroscope sample integration and screen-axis remapping.
//
// Pure policy shared by the sensor thread (gyro_sensor.cpp) and the display
// glue (gyro_sdl.cpp), and tested by unittests/inputtest.
//
//===========================================================================//

#ifndef GYRO_MATH_H
#define GYRO_MATH_H
#ifdef _WIN32
#pragma once
#endif

#include <stdint.h>

namespace gyro
{

// Samples further apart than this are not integrated across (sensor restart,
// dropped batch): the rotation during a gap is unknown, not the last rate.
const double kMaxSampleGapSeconds = 0.1;

// Integrates angular-rate samples (radians/second, timestamps in nanoseconds)
// into rotation, trapezoidally between consecutive samples.
class CRateIntegrator
{
public:
	CRateIntegrator();

	// Forgets the previous sample, so the next one starts a new interval.
	void Reset();
	// Adds the rotation since the previous sample to rotation[3].
	void AddSample( int64_t nTimestampNs, const float rate[3], double rotation[3] );

private:
	bool m_bHavePrevious;
	int64_t m_nPreviousTimestampNs;
	float m_flPreviousRate[3];
};

// Smooths gravity or accelerometer samples (the direction away from gravity,
// in any units) with a first-order low-pass timed by the samples' timestamps.
// A time constant of 0 passes samples through. The first sample, and the first
// after Reset or a gap longer than kMaxSampleGapSeconds, is taken as is.
class CUpFilter
{
public:
	explicit CUpFilter( double flTimeConstantSeconds );

	void Reset();
	// Adds a sample and returns the filtered direction in up[3].
	void AddSample( int64_t nTimestampNs, const float sample[3], float up[3] );

private:
	double m_flTimeConstant;
	bool m_bHavePrevious;
	int64_t m_nPreviousTimestampNs;
	float m_flUp[3];
};

// The sampling period, in microseconds, for a display refreshing at
// flRefreshHz: two samples per refresh (240 Hz on a 120 Hz panel), so each
// frame integrates fresh rotation. An unknown rate (<= 0) is taken as 60 Hz.
int SamplePeriodUs( float flRefreshHz );

// Degrees the displayed image is rotated from the display's natural
// orientation (Android's Display.getRotation()), from the rotation relative to
// portrait (0, 90 landscape, 180, 270 landscape flipped) that SDL reports.
int DisplayRotation( int nDegreesFromPortrait, bool bNaturalLandscape );

// Projects rotation about the device's natural axes (x right, y up, z out of
// the screen) onto the displayed screen's axes: screen[0] about screen-right
// (pitch, positive looks up), screen[1] about screen-up (yaw, positive turns
// left), screen[2] about the screen normal (roll, counter-clockwise).
void DeviceToScreen( const float device[3], int nDisplayRotation, float screen[3] );

} // namespace gyro

#endif // GYRO_MATH_H
