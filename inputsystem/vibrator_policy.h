//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Turns the per-frame two-motor rumble request into commands for a
//          single amplitude-controlled vibrator (a phone's haptic actuator).
//
// Pure policy used by the device glue (vibrator_sdl.cpp) and tested by
// unittests/inputtest.
//
//===========================================================================//

#ifndef VIBRATOR_POLICY_H
#define VIBRATOR_POLICY_H
#ifdef _WIN32
#pragma once
#endif

#include <stdint.h>

namespace vibrator
{

// Android's VibrationEffect amplitude range; 0 means off.
const int kMaxAmplitude = 255;

// Intensity for one actuator from the two rumble motors (left: low frequency,
// right: high frequency): their combined energy, sqrt(left^2 + right^2),
// clamped to [0, 1]. Negative and non-finite motor values count as 0.
float MixMotors( float flLeft, float flRight );

// The platform amplitude for an intensity: 0 (off) below half of one step,
// otherwise the nearest of 1..kMaxAmplitude.
int AmplitudeLevel( float flIntensity );

struct Timing
{
	// Every play command is a one-shot of this length, renewed while the
	// request holds, so a stalled or suspended game stops vibrating by itself.
	int m_nLeaseMs;
	// Renew once less than this much of the lease is left.
	int m_nRenewMarginMs;
	// Amplitude changes are sent no more often than this; each send restarts
	// the actuator and is an IPC call.
	int m_nMinIntervalMs;
	// A change of at least this many levels is sent after m_nMinIntervalMs;
	// a smaller one only once it has persisted for m_nSettleMs.
	int m_nMinStep;
	int m_nSettleMs;
};

Timing DefaultTiming();

struct Command
{
	enum Kind
	{
		NONE, // nothing to send
		PLAY, // vibrate at m_nAmplitude for m_nDurationMs, replacing any vibration
		STOP, // cancel the vibration
	};
	Kind m_Kind;
	int m_nAmplitude;
	int m_nDurationMs;
};

// Tracks what the vibrator is playing and decides what to send for each
// rumble request. Times are milliseconds on a monotonic clock.
class CRumbleStream
{
public:
	explicit CRumbleStream( const Timing &timing = DefaultTiming() );

	Command Update( float flLeft, float flRight, int64_t nNowMs );
	// Forgets the playback after something else stopped the vibrator
	// (suspend, disable); the next nonzero request starts a new one.
	void Reset();
	bool IsPlaying() const { return m_bPlaying; }

private:
	Command Play( int nLevel, int64_t nNowMs );

	Timing m_Timing;
	bool m_bPlaying;
	int m_nLevel;			// the amplitude last sent
	int64_t m_nSentMs;		// when it was sent
	int64_t m_nLeaseEndMs;	// when that one-shot ends by itself
	bool m_bDiverged;		// the request has differed from m_nLevel...
	int64_t m_nDivergedMs;	// ...since this time
};

} // namespace vibrator

#endif // VIBRATOR_POLICY_H
