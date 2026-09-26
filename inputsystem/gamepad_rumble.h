//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Turns the per-frame two-motor rumble request into commands for a
//          gamepad's rumble (SDL_RumbleGamepad): the only rumble a controller
//          has on iOS, tvOS and Android, and the two-motor one on desktop.
//
// Pure policy used by the device glue (joystick_sdl.cpp) and tested by
// unittests/inputtest.
//
//===========================================================================//

#ifndef GAMEPAD_RUMBLE_H
#define GAMEPAD_RUMBLE_H
#ifdef _WIN32
#pragma once
#endif

#include <stdint.h>

namespace gamepadrumble
{

// SDL_RumbleGamepad's motor range; 0 means off.
const int kMaxLevel = 65535;

struct Timing
{
	// Every play command lasts this long and is renewed while the request
	// holds, so a stalled, crashed or backgrounded game stops rumbling by
	// itself.
	int m_nLeaseMs;
	// Renew once less than this much of the lease is left.
	int m_nRenewMarginMs;
	// Level changes are sent no more often than this (a play command on
	// Apple's controllers builds a new haptic pattern).
	int m_nMinIntervalMs;
	// Changes smaller than this wait for the next renewal.
	int m_nMinStep;
};

// 1000 ms leases renewed with 250 ms left, changes at most every 20 ms, and
// a 1% step.
Timing DefaultTiming();

// The level for one motor: 0 (off) for zero, negative and non-finite values,
// kMaxLevel from 1 up, otherwise the nearest level.
int MotorLevel( float flMotor );

struct Command
{
	enum Kind
	{
		NONE, // nothing to send
		PLAY, // rumble at m_nLow/m_nHigh for m_nDurationMs, replacing any rumble
		STOP, // stop the rumble
	};
	Kind m_Kind;
	int m_nLow;  // the left (low-frequency, heavy) motor
	int m_nHigh; // the right (high-frequency, light) motor
	int m_nDurationMs;
};

// Tracks what the gamepad is playing and decides what to send for each rumble
// request. Times are milliseconds on a monotonic clock.
class CRumbleLease
{
public:
	explicit CRumbleLease( const Timing &timing = DefaultTiming() );

	// A request of (0, 0) stops the rumble.
	Command Update( float flLeft, float flRight, int64_t nNowMs );
	// Forgets the playback, after the gamepad was connected or removed.
	void Reset();
	bool IsPlaying() const { return m_bPlaying; }

private:
	Command Play( int nLow, int nHigh, int64_t nNowMs );
	bool LevelChanged( int nFrom, int nTo ) const;

	Timing m_Timing;
	bool m_bPlaying;
	int m_nLow;
	int m_nHigh;
	int64_t m_nSentMs;
	int64_t m_nLeaseEndMs;
};

} // namespace gamepadrumble

#endif // GAMEPAD_RUMBLE_H
