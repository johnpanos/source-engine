//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the gamepad rumble policy (inputsystem/gamepad_rumble.h).
//          The checks take the policy as function pointers so the sensitivity
//          suite can run them against deliberately wrong policies.
//
// Lease checks feed request sequences to a lease and play its commands on a
// model two-motor gamepad, then judge what the motors do: onset and stop
// latency, motor independence, tracking error, command rate, lease bounds and
// renewal.
//
//=============================================================================//

#ifndef UNITTESTS_INPUTTEST_GAMEPAD_RUMBLE_CHECKS_H
#define UNITTESTS_INPUTTEST_GAMEPAD_RUMBLE_CHECKS_H

#include "inputsystem/gamepad_rumble.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace gamepadrumbletest
{

// The contract every lease is judged against (gamepad_rumble.h).
const int kMaxLeaseMs = 1000;        // a stalled caller stops rumbling within this
const int kMinLeaseAheadMs = 200;    // a frame hitch shorter than this leaves no gap
const int kMinIntervalMs = 20;       // no more than one command per 20 ms of changes
const int kMaxTrackingError = 1500;  // levels (2.3% of full), while a signal changes gradually
const int kMaxSmallChangeMs = 1000;  // a persistent small change is applied within this

class Lease
{
public:
	virtual ~Lease() {}
	virtual gamepadrumble::Command Update( float flLeft, float flRight, int64_t nNowMs ) = 0;
	virtual void Reset() = 0;
};

class RealLease : public Lease
{
public:
	gamepadrumble::Command Update( float flLeft, float flRight, int64_t nNowMs ) override
	{
		return m_Lease.Update( flLeft, flRight, nNowMs );
	}
	void Reset() override { m_Lease.Reset(); }

private:
	gamepadrumble::CRumbleLease m_Lease;
};

struct Policy
{
	int ( *motorLevel )( float );
	Lease *( *createLease )();
};

inline Lease *CreateRealLease()
{
	return new RealLease;
}

inline Policy RealPolicy()
{
	Policy policy;
	policy.motorLevel = gamepadrumble::MotorLevel;
	policy.createLease = CreateRealLease;
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

// The gamepad: plays the last command's two levels until it ends or stops,
// as SDL_RumbleGamepad does.
struct ModelGamepad
{
	int low = 0;
	int high = 0;
	int64_t endMs = 0;
	int plays = 0;
	int stops = 0;
	int longestMs = 0;

	void Apply( const gamepadrumble::Command &command, int64_t nowMs )
	{
		if ( command.m_Kind == gamepadrumble::Command::PLAY )
		{
			++plays;
			low = command.m_nLow;
			high = command.m_nHigh;
			endMs = nowMs + command.m_nDurationMs;
			if ( command.m_nDurationMs > longestMs )
				longestMs = command.m_nDurationMs;
		}
		else if ( command.m_Kind == gamepadrumble::Command::STOP )
		{
			++stops;
			low = high = 0;
			endMs = nowMs;
		}
	}

	int LowAt( int64_t nowMs ) const { return nowMs < endMs ? low : 0; }
	int HighAt( int64_t nowMs ) const { return nowMs < endMs ? high : 0; }
};

// Drives a lease and the model gamepad together.
struct Rig
{
	Lease *lease;
	ModelGamepad pad;

	explicit Rig( const Policy &policy ) : lease( policy.createLease() ) {}
	~Rig() { delete lease; }

	gamepadrumble::Command Send( float flLeft, float flRight, int64_t nowMs )
	{
		gamepadrumble::Command command = lease->Update( flLeft, flRight, nowMs );
		pad.Apply( command, nowMs );
		return command;
	}
};

// Rounded to 1..65535, with 0 only below half of one step.
inline void CheckMotorLevel( const Policy &policy, Tally &tally )
{
	const float nan = std::numeric_limits<float>::quiet_NaN();
	const float inf = std::numeric_limits<float>::infinity();
	tally.Check( policy.motorLevel( 0.f ) == 0, "motor 0 is off" );
	tally.Check( policy.motorLevel( 1.f ) == 65535, "motor 1 is level 65535" );
	tally.Check( policy.motorLevel( 0.5f ) == 32768, "motor 0.5 rounds to 32768" );
	tally.Check( policy.motorLevel( 0.25f ) == 16384, "motor 0.25 rounds to 16384" );
	tally.Check( policy.motorLevel( 0.6f / 65535.f ) == 1, "0.6 of a step rounds up to 1" );
	tally.Check( policy.motorLevel( 0.4f / 65535.f ) == 0, "0.4 of a step is off" );
	tally.Check( policy.motorLevel( 2.f ) == 65535, "an overdriven motor clamps to 65535" );
	tally.Check( policy.motorLevel( -0.3f ) == 0, "a negative motor is off" );
	tally.Check( policy.motorLevel( nan ) == 0, "a NaN motor is off" );
	tally.Check( policy.motorLevel( inf ) == 0, "an infinite motor is off" );
}

inline void CheckOnsetAndStop( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	gamepadrumble::Command command = rig.Send( 0.5f, 0.25f, 1000 );
	tally.Check( command.m_Kind == gamepadrumble::Command::PLAY, "the first nonzero request plays at once" );
	tally.Check( command.m_nLow == 32768, "the left motor drives the low-frequency motor" );
	tally.Check( command.m_nHigh == 16384, "the right motor drives the high-frequency motor" );
	tally.Check( command.m_nDurationMs > kMinLeaseAheadMs && command.m_nDurationMs <= kMaxLeaseMs,
	    "a play is a bounded lease" );

	rig.Send( 0.5f, 0.25f, 1016 );
	command = rig.Send( 0.f, 0.f, 1032 );
	tally.Check( command.m_Kind == gamepadrumble::Command::STOP, "a zero request stops at once" );
	tally.Check( rig.pad.LowAt( 1032 ) == 0 && rig.pad.HighAt( 1032 ) == 0,
	    "both motors are off after the stop" );

	// Idle frames send nothing: SetXDeviceRumble( 0, 0 ) runs every frame.
	bool bQuiet = true;
	for ( int64_t t = 1048; t < 3000; t += 16 )
		bQuiet = bQuiet && rig.Send( 0.f, 0.f, t ).m_Kind == gamepadrumble::Command::NONE;
	tally.Check( bQuiet, "idle frames send no commands" );

	command = rig.Send( 0.f, 0.3f, 3000 );
	tally.Check( command.m_Kind == gamepadrumble::Command::PLAY && command.m_nLow == 0 &&
	                 command.m_nHigh == 19661,
	    "a new effect after a stop plays at once, on its own motor" );
}

// The motors are independent: changing one leaves the other.
inline void CheckMotorIndependence( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	rig.Send( 0.2f, 0.f, 0 );
	tally.Check( rig.pad.LowAt( 0 ) == 13107 && rig.pad.HighAt( 0 ) == 0,
	    "the left motor alone runs only the low-frequency motor" );

	for ( int64_t t = 16; t <= 96; t += 16 )
		rig.Send( 0.2f, 0.8f, t );
	tally.Check( rig.pad.LowAt( 96 ) == 13107 && rig.pad.HighAt( 96 ) == 52428,
	    "starting the right motor keeps the left one's level" );

	for ( int64_t t = 112; t <= 200; t += 16 )
		rig.Send( 0.f, 0.8f, t );
	tally.Check( rig.pad.LowAt( 200 ) == 0 && rig.pad.HighAt( 200 ) == 52428,
	    "stopping the left motor keeps the right one running" );
}

inline void CheckSustained( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	bool bTracks = true, bLeaseAhead = true;
	for ( int64_t t = 0; t <= 5000; t += 16 )
	{
		rig.Send( 1.f, 0.5f, t );
		bTracks = bTracks && rig.pad.LowAt( t ) == 65535 && rig.pad.HighAt( t ) == 32768;
		bLeaseAhead = bLeaseAhead && rig.pad.endMs - t >= kMinLeaseAheadMs;
	}
	tally.Check( bTracks, "a sustained request rumbles at every frame" );
	tally.Check( bLeaseAhead, "a sustained request is renewed before a frame hitch could gap it" );
	tally.Check( rig.pad.plays <= 8, "a sustained request is renewed at most 8 times in 5 s" );
	tally.Check( rig.pad.stops == 0, "a sustained request is never stopped" );
	tally.Check( rig.pad.longestMs <= kMaxLeaseMs, "no play outlasts the lease bound" );
}

// A decaying screen shake at 120 frames per second, on both motors.
inline void CheckRamp( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	int worst = 0;
	int64_t lastPlayMs = -1000;
	int minGapMs = 1 << 30;
	for ( int64_t t = 0; t <= 2000; t += 8 )
	{
		const float flLevel = 1.f - t / 2000.f;
		gamepadrumble::Command command = rig.Send( flLevel, 0.5f * flLevel, t );
		if ( command.m_Kind == gamepadrumble::Command::PLAY )
		{
			if ( t - lastPlayMs < minGapMs )
				minGapMs = (int)( t - lastPlayMs );
			lastPlayMs = t;
		}
		if ( flLevel > 0.f )
		{
			const int lowError = std::abs( rig.pad.LowAt( t ) - policy.motorLevel( flLevel ) );
			const int highError = std::abs( rig.pad.HighAt( t ) - policy.motorLevel( 0.5f * flLevel ) );
			worst = lowError > worst ? lowError : worst;
			worst = highError > worst ? highError : worst;
		}
	}
	tally.Check( worst <= kMaxTrackingError, "a ramp is tracked within 2.3% on both motors" );
	tally.Check( minGapMs >= kMinIntervalMs, "changes are sent at most every 20 ms" );
	tally.Check( rig.pad.LowAt( 2000 ) == 0 && rig.pad.HighAt( 2000 ) == 0,
	    "the ramp ends with the motors off" );
}

inline void CheckSmallChange( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	rig.Send( 0.5f, 0.f, 0 );
	int64_t appliedMs = -1;
	for ( int64_t t = 16; t <= 3000; t += 16 )
	{
		rig.Send( 0.505f, 0.f, t );
		if ( appliedMs < 0 && rig.pad.LowAt( t ) == policy.motorLevel( 0.505f ) )
			appliedMs = t;
	}
	tally.Check( appliedMs >= 0 && appliedMs <= kMaxSmallChangeMs,
	    "a persistent small change is applied within a lease" );
}

inline void CheckStallAndReset( const Policy &policy, Tally &tally )
{
	// The caller stops calling (a hitch, a crash, the app in the background):
	// the motors stop by themselves within the lease bound.
	{
		Rig rig( policy );
		for ( int64_t t = 0; t <= 3000; t += 16 )
			rig.Send( 1.f, 1.f, t );
		tally.Check( rig.pad.LowAt( 3000 + kMaxLeaseMs ) == 0 &&
		                 rig.pad.HighAt( 3000 + kMaxLeaseMs ) == 0,
		    "a stalled caller's rumble ends within the lease bound" );
	}

	// The gamepad was replaced: after Reset the same request plays again.
	{
		Rig rig( policy );
		rig.Send( 0.6f, 0.f, 0 );
		rig.Send( 0.6f, 0.f, 16 );
		rig.lease->Reset();
		rig.pad.Apply( gamepadrumble::Command{ gamepadrumble::Command::STOP, 0, 0, 0 }, 20 );
		rig.Send( 0.6f, 0.f, 32 );
		tally.Check( rig.pad.LowAt( 32 ) == 39321, "a reset lease replays the current request" );
	}

	// The clock went backwards: the lease end is unknown, so play again.
	{
		Rig rig( policy );
		rig.Send( 0.6f, 0.f, 10000 );
		gamepadrumble::Command command = rig.Send( 0.6f, 0.f, 9000 );
		tally.Check(
		    command.m_Kind == gamepadrumble::Command::PLAY, "a backwards clock restarts the lease" );
	}
}

inline void CheckAll( const Policy &policy, Tally &tally )
{
	CheckMotorLevel( policy, tally );
	CheckOnsetAndStop( policy, tally );
	CheckMotorIndependence( policy, tally );
	CheckSustained( policy, tally );
	CheckRamp( policy, tally );
	CheckSmallChange( policy, tally );
	CheckStallAndReset( policy, tally );
}

} // namespace gamepadrumbletest

#endif // UNITTESTS_INPUTTEST_GAMEPAD_RUMBLE_CHECKS_H
