//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the device-vibrator oracle
//          (vibrator_policy_checks.h). Each broken policy differs from the
//          real one in one plausible way; the oracle must fail it while the
//          real policy passes.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/inputtest/vibrator_policy_checks.h"

#include <climits>
#include <cstdio>

namespace
{

// Averages the motors, as the SDL gamepad path does: one motor at half strength.
float MixAverage( float flLeft, float flRight )
{
	return 0.5f * ( vibrator::MixMotors( flLeft, 0.f ) + vibrator::MixMotors( 0.f, flRight ) );
}

// Takes the stronger motor: two motors feel no stronger than one.
float MixMax( float flLeft, float flRight )
{
	float flL = vibrator::MixMotors( flLeft, 0.f );
	float flR = vibrator::MixMotors( 0.f, flRight );
	return flL > flR ? flL : flR;
}

// Truncates instead of rounding (and so turns sub-step intensities off).
int LevelTruncate( float flIntensity )
{
	if ( !( flIntensity > 0.f ) )
		return 0;
	return flIntensity >= 1.f ? vibrator::kMaxAmplitude
	                          : (int)( flIntensity * vibrator::kMaxAmplitude );
}

// A real stream with different timing.
template <int LeaseMs, int RenewMarginMs, int MinIntervalMs, int MinStep, int SettleMs>
vibratortest::Stream *CreateTimedStream()
{
	class TimedStream : public vibratortest::Stream
	{
	public:
		TimedStream() : m_Stream( Timing() ) {}
		vibrator::Command Update( float flLeft, float flRight, int64_t nNowMs ) override
		{
			return m_Stream.Update( flLeft, flRight, nNowMs );
		}
		void Reset() override { m_Stream.Reset(); }

	private:
		static vibrator::Timing Timing()
		{
			vibrator::Timing timing = { LeaseMs, RenewMarginMs, MinIntervalMs, MinStep, SettleMs };
			return timing;
		}
		vibrator::CRumbleStream m_Stream;
	};
	return new TimedStream;
}

// A real stream whose commands (or Reset) are altered.
enum Alteration
{
	PLAY_EVERY_FRAME, // resends the amplitude on every nonzero request
	NEVER_STOP,       // leaves the one-shot to run out instead of cancelling
	STOP_WHILE_IDLE,  // cancels on every zero request
	IGNORE_RESET,     // keeps believing the cancelled vibration still plays
};

template <Alteration Kind> vibratortest::Stream *CreateAlteredStream()
{
	class AlteredStream : public vibratortest::Stream
	{
	public:
		vibrator::Command Update( float flLeft, float flRight, int64_t nNowMs ) override
		{
			vibrator::Command command = m_Stream.Update( flLeft, flRight, nNowMs );
			int nLevel = vibrator::AmplitudeLevel( vibrator::MixMotors( flLeft, flRight ) );
			if ( Kind == PLAY_EVERY_FRAME && nLevel > 0 )
				command = vibrator::Command{ vibrator::Command::PLAY, nLevel, 1000 };
			if ( Kind == NEVER_STOP && command.m_Kind == vibrator::Command::STOP )
				command.m_Kind = vibrator::Command::NONE;
			if ( Kind == STOP_WHILE_IDLE && nLevel == 0 )
				command.m_Kind = vibrator::Command::STOP;
			return command;
		}
		void Reset() override
		{
			if ( Kind != IGNORE_RESET )
				m_Stream.Reset();
		}

	private:
		vibrator::CRumbleStream m_Stream;
	};
	return new AlteredStream;
}

bool OracleRejects( const char *name, const vibratortest::Policy &policy )
{
	vibratortest::Tally tally;
	tally.verbose = false;
	vibratortest::CheckAll( policy, tally );
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

	const vibratortest::Policy real = vibratortest::RealPolicy();
	expect( !OracleRejects( "real policy", real ), "the real policy passes the oracle" );

	vibratortest::Policy broken = real;
	broken.mixMotors = MixAverage;
	expect( OracleRejects( "average mix", broken ), "averaging the motors is rejected" );

	broken = real;
	broken.mixMotors = MixMax;
	expect( OracleRejects( "max mix", broken ), "taking the stronger motor is rejected" );

	broken = real;
	broken.amplitudeLevel = LevelTruncate;
	expect( OracleRejects( "truncated level", broken ), "truncating the amplitude is rejected" );

	broken = real;
	broken.createStream = CreateTimedStream<60000, 250, 20, 8, 100>;
	expect( OracleRejects( "unbounded lease", broken ), "a one-minute one-shot is rejected" );

	broken = real;
	broken.createStream = CreateTimedStream<1000, 0, 20, 8, 100>;
	expect( OracleRejects( "renews at expiry", broken ),
	    "renewing only when the one-shot runs out is rejected" );

	broken = real;
	broken.createStream = CreateTimedStream<1000, 250, 0, 1, 0>;
	expect(
	    OracleRejects( "no rate limit", broken ), "sending every amplitude change is rejected" );

	broken = real;
	broken.createStream = CreateTimedStream<1000, 250, 20, 8, INT_MAX / 2>;
	expect( OracleRejects( "never settles", broken ), "never applying small changes is rejected" );

	broken = real;
	broken.createStream = CreateTimedStream<1000, 250, 200, 8, 100>;
	expect( OracleRejects( "slow updates", broken ), "a 200 ms update interval is rejected" );

	broken = real;
	broken.createStream = CreateAlteredStream<PLAY_EVERY_FRAME>;
	expect( OracleRejects( "plays every frame", broken ),
	    "restarting the actuator every frame is rejected" );

	broken = real;
	broken.createStream = CreateAlteredStream<NEVER_STOP>;
	expect( OracleRejects( "never stops", broken ), "leaving a stop to the lease is rejected" );

	broken = real;
	broken.createStream = CreateAlteredStream<STOP_WHILE_IDLE>;
	expect(
	    OracleRejects( "stops while idle", broken ), "cancelling on every idle frame is rejected" );

	broken = real;
	broken.createStream = CreateAlteredStream<IGNORE_RESET>;
	expect( OracleRejects( "ignores reset", broken ), "ignoring an external cancel is rejected" );

	return testing::ReportConformance( checks, failures );
}
