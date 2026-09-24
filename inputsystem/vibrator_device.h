//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device's own vibrator (Android), when it has amplitude control.
//
// SDL3 exposes the phone vibrator as a haptic device, but plays it without
// vibration attributes (so the system files it under "unknown" rather than
// media, ignoring the user's media-vibration intensity) and with no way to
// ask whether the actuator can vary its strength. This drives
// android.os.Vibrator through JNI instead.
//
//===========================================================================//

#ifndef VIBRATOR_DEVICE_H
#define VIBRATOR_DEVICE_H
#ifdef _WIN32
#pragma once
#endif

class CDeviceVibrator
{
public:
	CDeviceVibrator();
	~CDeviceVibrator();

	// Finds the device vibrator. Returns false when there is none or it can
	// only switch on and off (no amplitude control). Owner thread only.
	bool Init();
	// Stops the vibrator and releases it. No SetSuspended may be in flight.
	// Owner thread only.
	void Shutdown();
	bool IsAvailable() const;

	// Vibrates at nAmplitude (1..255) for nDurationMs, replacing any current
	// vibration; ignored while suspended. A platform failure makes IsAvailable
	// false. Owner thread only.
	void Play( int nAmplitude, int nDurationMs );
	// Stops any vibration. Any thread, between Init and Shutdown.
	void Cancel();

	// Stops the vibration and ignores Play while the application is in the
	// background. Any thread, between Init and Shutdown.
	void SetSuspended( bool bSuspended );
	bool IsSuspended() const;

private:
	struct State;
	State *m_pState;
};

#endif // VIBRATOR_DEVICE_H
