//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the device-vibrator rumble policy
//          (inputsystem/vibrator_policy.h). The checks take the policy as
//          function pointers so the sensitivity suite can run them against
//          deliberately wrong policies.
//
// Stream checks feed request sequences to a stream and play its commands on
// a model vibrator, then judge what the actuator does: onset and stop
// latency, tracking error, command rate, lease bounds and renewal.
//
//=============================================================================//

#ifndef UNITTESTS_INPUTTEST_VIBRATOR_POLICY_CHECKS_H
#define UNITTESTS_INPUTTEST_VIBRATOR_POLICY_CHECKS_H

#include "inputsystem/vibrator_policy.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace vibratortest
{

// The contract every stream is judged against (vibrator_policy.h).
const int kMaxLeaseMs = 1000;      // a stalled caller stops vibrating within this
const int kMinLeaseAheadMs = 200;  // a frame hitch shorter than this leaves no gap
const int kMinIntervalMs = 20;     // no more than one command per 20 ms of changes
const int kMaxSmallChangeMs = 100; // a persistent small change is applied within this
const int kMaxTrackingError = 8;   // levels, while a signal changes gradually

class Stream
{
public:
	virtual ~Stream() {}
	virtual vibrator::Command Update( float flLeft, float flRight, int64_t nNowMs ) = 0;
	virtual void Reset() = 0;
};

class RealStream : public Stream
{
public:
	vibrator::Command Update( float flLeft, float flRight, int64_t nNowMs ) override
	{
		return m_Stream.Update( flLeft, flRight, nNowMs );
	}
	void Reset() override { m_Stream.Reset(); }

private:
	vibrator::CRumbleStream m_Stream;
};

struct Policy
{
	float ( *mixMotors )( float, float );
	int ( *amplitudeLevel )( float );
	Stream *( *createStream )();
};

inline Stream *CreateRealStream()
{
	return new RealStream;
}

inline Policy RealPolicy()
{
	Policy policy;
	policy.mixMotors = vibrator::MixMotors;
	policy.amplitudeLevel = vibrator::AmplitudeLevel;
	policy.createStream = CreateRealStream;
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

// The actuator: plays the last one-shot until it ends or is cancelled.
struct ModelVibrator
{
	int amplitude = 0;
	int64_t endMs = 0;
	int plays = 0;
	int stops = 0;
	int longestMs = 0;

	void Apply( const vibrator::Command &command, int64_t nowMs )
	{
		if ( command.m_Kind == vibrator::Command::PLAY )
		{
			++plays;
			amplitude = command.m_nAmplitude;
			endMs = nowMs + command.m_nDurationMs;
			if ( command.m_nDurationMs > longestMs )
				longestMs = command.m_nDurationMs;
		}
		else if ( command.m_Kind == vibrator::Command::STOP )
		{
			++stops;
			amplitude = 0;
			endMs = nowMs;
		}
	}

	int AmplitudeAt( int64_t nowMs ) const { return nowMs < endMs ? amplitude : 0; }
};

// Drives a stream and the model vibrator together.
struct Rig
{
	Stream *stream;
	ModelVibrator vibrator;

	explicit Rig( const Policy &policy ) : stream( policy.createStream() ) {}
	~Rig() { delete stream; }

	vibrator::Command Send( float flLeft, float flRight, int64_t nowMs )
	{
		vibrator::Command command = stream->Update( flLeft, flRight, nowMs );
		vibrator.Apply( command, nowMs );
		return command;
	}
};

inline bool Near( float a, float b, float tolerance = 1e-5f )
{
	return std::fabs( a - b ) < tolerance;
}

// Energy sum of the two motors, clamped to [0, 1].
inline void CheckMixMotors( const Policy &policy, Tally &tally )
{
	const float nan = std::numeric_limits<float>::quiet_NaN();
	const float inf = std::numeric_limits<float>::infinity();
	tally.Check( policy.mixMotors( 0.f, 0.f ) == 0.f, "no motors mix to 0" );
	tally.Check(
	    Near( policy.mixMotors( 0.3f, 0.f ), 0.3f ), "the left motor alone keeps its strength" );
	tally.Check(
	    Near( policy.mixMotors( 0.f, 0.3f ), 0.3f ), "the right motor alone keeps its strength" );
	tally.Check(
	    Near( policy.mixMotors( 0.3f, 0.4f ), 0.5f ), "0.3 and 0.4 mix to 0.5 (energy sum)" );
	tally.Check( Near( policy.mixMotors( 0.6f, 0.6f ), 0.848528f ), "0.6 and 0.6 mix to 0.85" );
	tally.Check( policy.mixMotors( 1.f, 1.f ) == 1.f, "both motors full clamp to 1" );
	tally.Check( policy.mixMotors( 2.f, 0.f ) == 1.f, "an overdriven motor clamps to 1" );
	tally.Check( Near( policy.mixMotors( -1.f, 0.3f ), 0.3f ), "a negative motor counts as 0" );
	tally.Check( Near( policy.mixMotors( nan, 0.3f ), 0.3f ), "a NaN motor counts as 0" );
	tally.Check( policy.mixMotors( inf, 0.f ) == 0.f, "an infinite motor counts as 0" );
}

// Rounded to 1..255, with 0 only below half of one step.
inline void CheckAmplitudeLevel( const Policy &policy, Tally &tally )
{
	tally.Check( policy.amplitudeLevel( 0.f ) == 0, "intensity 0 is off" );
	tally.Check( policy.amplitudeLevel( 1.f ) == 255, "intensity 1 is amplitude 255" );
	tally.Check( policy.amplitudeLevel( 0.5f ) == 128, "intensity 0.5 rounds to 128" );
	tally.Check( policy.amplitudeLevel( 0.25f ) == 64, "intensity 0.25 rounds to 64" );
	tally.Check( policy.amplitudeLevel( 1.f / 255.f ) == 1, "one step is amplitude 1" );
	tally.Check( policy.amplitudeLevel( 0.6f / 255.f ) == 1, "0.6 of a step rounds up to 1" );
	tally.Check( policy.amplitudeLevel( 0.4f / 255.f ) == 0, "0.4 of a step is off" );
	tally.Check( policy.amplitudeLevel( 1.5f ) == 255, "intensity above 1 is amplitude 255" );
	tally.Check( policy.amplitudeLevel( -0.1f ) == 0, "a negative intensity is off" );
	tally.Check( policy.amplitudeLevel( std::numeric_limits<float>::quiet_NaN() ) == 0,
	    "a NaN intensity is off" );
}

inline void CheckOnsetAndStop( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	vibrator::Command command = rig.Send( 0.5f, 0.f, 1000 );
	tally.Check( command.m_Kind == vibrator::Command::PLAY && command.m_nAmplitude == 128,
	    "the first nonzero request plays at once, at its amplitude" );
	tally.Check( command.m_nDurationMs > kMinLeaseAheadMs && command.m_nDurationMs <= kMaxLeaseMs,
	    "a play is a bounded one-shot" );

	rig.Send( 0.5f, 0.f, 1016 );
	command = rig.Send( 0.f, 0.f, 1032 );
	tally.Check( command.m_Kind == vibrator::Command::STOP, "a zero request stops at once" );
	tally.Check( rig.vibrator.AmplitudeAt( 1032 ) == 0, "the vibrator is off after the stop" );

	// Idle frames send nothing: SetRumble( 0, 0 ) runs every frame.
	bool bQuiet = true;
	for ( int64_t t = 1048; t < 3000; t += 16 )
		bQuiet = bQuiet && rig.Send( 0.f, 0.f, t ).m_Kind == vibrator::Command::NONE;
	tally.Check( bQuiet, "idle frames send no commands" );

	// A short effect restarts cleanly after a stop.
	command = rig.Send( 0.f, 0.3f, 3000 );
	tally.Check( command.m_Kind == vibrator::Command::PLAY && command.m_nAmplitude == 77,
	    "a new effect after a stop plays at once" );
}

inline void CheckSustained( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	bool bTracks = true, bLeaseAhead = true;
	for ( int64_t t = 0; t <= 5000; t += 16 )
	{
		rig.Send( 1.f, 0.f, t );
		bTracks = bTracks && rig.vibrator.AmplitudeAt( t ) == 255;
		bLeaseAhead = bLeaseAhead && rig.vibrator.endMs - t >= kMinLeaseAheadMs;
	}
	tally.Check( bTracks, "a sustained request vibrates at every frame" );
	tally.Check( bLeaseAhead, "a sustained request is renewed before a frame hitch could gap it" );
	tally.Check( rig.vibrator.plays <= 8, "a sustained request is renewed at most 8 times in 5 s" );
	tally.Check( rig.vibrator.stops == 0, "a sustained request is never cancelled" );
}

// A decaying screen shake at 120 frames per second.
inline void CheckRamp( const Policy &policy, Tally &tally )
{
	Rig rig( policy );
	bool bTracks = true;
	int worst = 0;
	for ( int64_t t = 0; t <= 2000; t += 8 )
	{
		float flShake = 1.f - float( t ) / 2000.f;
		rig.Send( 0.f, flShake, t );
		int target = policy.amplitudeLevel( policy.mixMotors( 0.f, flShake ) );
		int error = std::abs( rig.vibrator.AmplitudeAt( t ) - target );
		if ( error > worst )
			worst = error;
		bTracks = bTracks && error <= kMaxTrackingError;
	}
	tally.Check( bTracks, "a decaying shake is tracked within 8 levels" );
	tally.Check( rig.vibrator.plays <= 2000 / kMinIntervalMs + 1,
	    "a ramp sends no more than one command per 20 ms" );
	tally.Check( rig.vibrator.plays >= 10, "a full-range ramp updates the amplitude" );
	tally.Check( rig.vibrator.AmplitudeAt( 2000 ) == 0, "the ramp ends with the vibrator off" );
	if ( worst > kMaxTrackingError && tally.verbose )
		std::printf( "  worst ramp tracking error: %d levels\n", worst );
}

inline void CheckChanges( const Policy &policy, Tally &tally )
{
	// A large change right after a send waits for the minimum interval.
	{
		Rig rig( policy );
		rig.Send( 0.2f, 0.f, 0 );
		vibrator::Command early = rig.Send( 0.8f, 0.f, 8 );
		rig.Send( 0.8f, 0.f, 16 );
		rig.Send( 0.8f, 0.f, 24 );
		tally.Check( early.m_Kind == vibrator::Command::NONE,
		    "a change within 20 ms of the last send waits" );
		tally.Check( rig.vibrator.AmplitudeAt( 24 ) == 204,
		    "a large change is applied on the first frame after 20 ms" );
	}

	// A small persistent change is applied once it settles.
	{
		Rig rig( policy );
		rig.Send( 0.5f, 0.f, 0 );
		const float flSlightlyMore = 131.f / 255.f;
		for ( int64_t t = 16; t <= 16 + kMaxSmallChangeMs + 16; t += 16 )
			rig.Send( flSlightlyMore, 0.f, t );
		tally.Check( rig.vibrator.AmplitudeAt( 16 + kMaxSmallChangeMs + 16 ) == 131,
		    "a persistent 3-level change is applied within 100 ms" );
	}

	// Small flicker around the playing level does not restart the actuator.
	{
		Rig rig( policy );
		rig.Send( 0.5f, 0.f, 0 );
		for ( int64_t t = 16; t < 600; t += 16 )
			rig.Send( ( t / 16 ) % 2 ? 130.f / 255.f : 0.5f, 0.f, t );
		tally.Check( rig.vibrator.plays == 1, "a 2-level flicker sends nothing" );
	}
}

inline void CheckStallAndReset( const Policy &policy, Tally &tally )
{
	// The caller stops updating (hang, blocked main thread): the vibrator
	// stops by itself, and the next request starts it again.
	{
		Rig rig( policy );
		for ( int64_t t = 0; t <= 3000; t += 16 )
			rig.Send( 0.6f, 0.f, t );
		tally.Check( rig.vibrator.longestMs <= kMaxLeaseMs, "no one-shot is longer than 1 s" );
		tally.Check( rig.vibrator.AmplitudeAt( 3000 + kMaxLeaseMs ) == 0,
		    "a stalled caller's vibration ends within 1 s" );
		rig.Send( 0.6f, 0.f, 6000 );
		tally.Check( rig.vibrator.AmplitudeAt( 6000 ) == 153,
		    "the first request after a stall plays again" );
	}

	// Something else cancelled the vibrator (suspend): after Reset the same
	// request plays again.
	{
		Rig rig( policy );
		rig.Send( 0.6f, 0.f, 0 );
		rig.Send( 0.6f, 0.f, 16 );
		rig.stream->Reset();
		rig.vibrator.Apply( vibrator::Command{ vibrator::Command::STOP, 0, 0 }, 20 );
		rig.Send( 0.6f, 0.f, 32 );
		tally.Check(
		    rig.vibrator.AmplitudeAt( 32 ) == 153, "a reset stream replays the current request" );
	}

	// The clock went backwards: the lease end is unknown, so play again.
	{
		Rig rig( policy );
		rig.Send( 0.6f, 0.f, 10000 );
		rig.vibrator.Apply( vibrator::Command{ vibrator::Command::STOP, 0, 0 }, 10000 );
		vibrator::Command command = rig.Send( 0.6f, 0.f, 9000 );
		tally.Check(
		    command.m_Kind == vibrator::Command::PLAY, "a backwards clock restarts the lease" );
	}
}

inline void CheckAll( const Policy &policy, Tally &tally )
{
	CheckMixMotors( policy, tally );
	CheckAmplitudeLevel( policy, tally );
	CheckOnsetAndStop( policy, tally );
	CheckSustained( policy, tally );
	CheckRamp( policy, tally );
	CheckChanges( policy, tally );
	CheckStallAndReset( policy, tally );
}

} // namespace vibratortest

#endif // UNITTESTS_INPUTTEST_VIBRATOR_POLICY_CHECKS_H
