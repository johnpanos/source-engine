//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device gyroscope, sampled on its own thread (Android NDK).
//
// The sensor thread owns every NDK sensor object and integrates each sample
// into running rotation totals using the samples' own timestamps. It also
// tracks which way is up (the gravity sensor, or the accelerometer where there
// is none) for turning in player space. The owner thread only stores requests
// into atomics and reads the results, so it never waits on the sensor thread
// except when Shutdown joins it.
//
//===========================================================================//

#ifndef GYRO_SENSOR_H
#define GYRO_SENSOR_H
#ifdef _WIN32
#pragma once
#endif

#include <stdint.h>

class CGyroSensor
{
public:
	CGyroSensor();
	~CGyroSensor();

	// Finds the device gyroscope and starts the (idle) sensor thread. Returns
	// false when the platform has no gyroscope. Owner thread only.
	bool Init();
	// Stops and joins the sensor thread. Owner thread only.
	void Shutdown();
	bool IsAvailable() const;

	// The sampling period to request, in microseconds; 0 turns the sensor off.
	// Periods shorter than the hardware minimum are raised to it. Owner thread.
	void SetSamplePeriod( int nPeriodUs );
	// Turns the sensor off while the application is in the background. Any thread.
	void SetSuspended( bool bSuspended );

	// Device rotation in radians since the previous call, about the axes of the
	// device's natural orientation (x right, y up, z out of the screen;
	// counter-clockwise positive). Owner thread only.
	void ConsumeRotation( float rotation[3] );
	// The direction away from gravity in the same axes, not normalized. False
	// until the sensor has measured it since it was last turned on. Any thread.
	bool GetUp( float up[3] ) const;
	// Gyroscope samples received since Init. Any thread.
	uint32_t SampleCount() const;

private:
	struct State;
	State *m_pState;
};

#endif // GYRO_SENSOR_H
