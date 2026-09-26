//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the gamepad rumble oracle (gamepad_rumble_checks.h).
//          Each broken policy differs from the real one in one plausible way;
//          the oracle must fail it while the real policy passes.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/inputtest/gamepad_rumble_checks.h"

#include <cstdio>

namespace
{

// Truncates instead of rounding (and so turns sub-step levels off).
int LevelTruncate( float flMotor )
{
	if ( !( flMotor > 0.f ) || !std::isfinite( flMotor ) )
		return 0;
	return flMotor >= 1.f ? gamepadrumble::kMaxLevel : (int)( flMotor * gamepadrumble::kMaxLevel );
}

// A real lease with different timing.
template <int LeaseMs, int RenewMarginMs, int MinIntervalMs, int MinStep>
gamepadrumbletest::Lease *CreateTimedLease()
{
	class TimedLease : public gamepadrumbletest::Lease
	{
	public:
		TimedLease() : m_Lease( Timing() ) {}
		gamepadrumble::Command Update( float flLeft, float flRight, int64_t nNowMs ) override
		{
			return m_Lease.Update( flLeft, flRight, nNowMs );
		}
		void Reset() override { m_Lease.Reset(); }

	private:
		static gamepadrumble::Timing Timing()
		{
			gamepadrumble::Timing timing = { LeaseMs, RenewMarginMs, MinIntervalMs, MinStep };
			return timing;
		}
		gamepadrumble::CRumbleLease m_Lease;
	};
	return new TimedLease;
}

// A real lease whose commands (or Reset) are altered.
enum Alteration
{
	SWAP_MOTORS,      // left drives the high-frequency motor
	AVERAGE_MOTORS,   // both motors at their average, as the SDL_Haptic path does
	PLAY_EVERY_FRAME, // resends the levels on every nonzero request
	NEVER_STOP,       // leaves the lease to run out instead of stopping
	STOP_WHILE_IDLE,  // stops on every zero request
	IGNORE_RESET,     // keeps believing the replaced gamepad still rumbles
	INFINITE_LEASE,   // plays until told otherwise (SDL_HAPTIC_INFINITY)
};

template <Alteration Kind> gamepadrumbletest::Lease *CreateAlteredLease()
{
	class AlteredLease : public gamepadrumbletest::Lease
	{
	public:
		gamepadrumble::Command Update( float flLeft, float flRight, int64_t nNowMs ) override
		{
			gamepadrumble::Command command = m_Lease.Update( flLeft, flRight, nNowMs );
			const int nLow = gamepadrumble::MotorLevel( flLeft );
			const int nHigh = gamepadrumble::MotorLevel( flRight );
			if ( Kind == SWAP_MOTORS && command.m_Kind == gamepadrumble::Command::PLAY )
			{
				const int nSwap = command.m_nLow;
				command.m_nLow = command.m_nHigh;
				command.m_nHigh = nSwap;
			}
			if ( Kind == AVERAGE_MOTORS && command.m_Kind == gamepadrumble::Command::PLAY )
				command.m_nLow = command.m_nHigh = ( command.m_nLow + command.m_nHigh ) / 2;
			if ( Kind == PLAY_EVERY_FRAME && ( nLow > 0 || nHigh > 0 ) )
				command = gamepadrumble::Command{ gamepadrumble::Command::PLAY, nLow, nHigh, 1000 };
			if ( Kind == NEVER_STOP && command.m_Kind == gamepadrumble::Command::STOP )
				command.m_Kind = gamepadrumble::Command::NONE;
			if ( Kind == STOP_WHILE_IDLE && nLow == 0 && nHigh == 0 )
				command.m_Kind = gamepadrumble::Command::STOP;
			if ( Kind == INFINITE_LEASE && command.m_Kind == gamepadrumble::Command::PLAY )
				command.m_nDurationMs = 1 << 30;
			return command;
		}
		void Reset() override
		{
			if ( Kind != IGNORE_RESET )
				m_Lease.Reset();
		}

	private:
		gamepadrumble::CRumbleLease m_Lease;
	};
	return new AlteredLease;
}

bool OracleRejects( const char *name, const gamepadrumbletest::Policy &policy )
{
	gamepadrumbletest::Tally tally;
	tally.verbose = false;
	gamepadrumbletest::CheckAll( policy, tally );
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

	const gamepadrumbletest::Policy real = gamepadrumbletest::RealPolicy();
	expect( !OracleRejects( "real policy", real ), "the real policy passes the oracle" );

	gamepadrumbletest::Policy broken = real;
	broken.motorLevel = LevelTruncate;
	expect( OracleRejects( "truncated level", broken ), "truncating the motor level is rejected" );

	struct Case
	{
		const char *name;
		gamepadrumbletest::Lease *( *createLease )();
		const char *what;
	};
	const Case cases[] = {
		{ "swapped motors", CreateAlteredLease<SWAP_MOTORS>, "swapping the motors is rejected" },
		{ "averaged motors", CreateAlteredLease<AVERAGE_MOTORS>, "averaging the motors is rejected" },
		{ "plays every frame", CreateAlteredLease<PLAY_EVERY_FRAME>,
		    "restarting the rumble every frame is rejected" },
		{ "never stops", CreateAlteredLease<NEVER_STOP>, "leaving a stop to the lease is rejected" },
		{ "stops while idle", CreateAlteredLease<STOP_WHILE_IDLE>,
		    "stopping on every idle frame is rejected" },
		{ "ignores reset", CreateAlteredLease<IGNORE_RESET>, "ignoring a replaced gamepad is rejected" },
		{ "infinite lease", CreateAlteredLease<INFINITE_LEASE>, "an unending play is rejected" },
		{ "unbounded lease", CreateTimedLease<60000, 250, 20, 655>, "a one-minute lease is rejected" },
		{ "renews at expiry", CreateTimedLease<1000, 0, 20, 655>,
		    "renewing only when the lease runs out is rejected" },
		{ "no rate limit", CreateTimedLease<1000, 250, 0, 1>, "sending every level change is rejected" },
		{ "slow updates", CreateTimedLease<1000, 250, 200, 655>, "a 200 ms update interval is rejected" },
	};
	for ( const Case &c : cases )
	{
		broken = real;
		broken.createLease = c.createLease;
		expect( OracleRejects( c.name, broken ), c.what );
	}

	return testing::ReportConformance( checks, failures );
}
