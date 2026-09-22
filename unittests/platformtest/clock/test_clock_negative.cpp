//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity (negative-provider) check for the RFC 0001 monotonic-clock
//			conformance suite (PLAT-CLOCK-001, Q-FOUNDATION). Feeds the SAME shared
//			predicate deliberately-broken clocks -- each violating one clause --
//			and asserts every one is caught while the conforming backend passes.
//
//			Passes (exit 0) when the oracle distinguishes conforming from broken.
//
//			Build/run: tools/quality/conformance.py check --suite platform.clock.sensitivity
//
//=============================================================================//

#include "clock_conformance.h"
#include "fake_clock.h"

#include "platform/contracts/clock.h"

#include <cstdio>

namespace
{

// DEFECT: time moves backward -- each Now() decreases the counter.
class CBackwardClock : public platform::IMonotonicClock
{
public:
	platform::MonotonicTimestamp Now() const override
	{
		platform::MonotonicTimestamp t;
		t.ticks = m_current;
		m_current -= 1000; // moves backward
		return t;
	}
	std::uint64_t ResolutionNanoseconds() const override { return 1000; }
	std::uint64_t ElapsedNanoseconds( platform::MonotonicTimestamp begin,
		platform::MonotonicTimestamp end ) const override
	{
		return end.ticks - begin.ticks;
	}

private:
	mutable std::uint64_t m_current = 1000000;
};

// DEFECT: resolution is reported as zero.
class CZeroResolutionClock : public platform::IMonotonicClock
{
public:
	platform::MonotonicTimestamp Now() const override
	{
		platform::MonotonicTimestamp t;
		t.ticks = m_current;
		m_current += 1000;
		return t;
	}
	std::uint64_t ResolutionNanoseconds() const override { return 0; }
	std::uint64_t ElapsedNanoseconds( platform::MonotonicTimestamp begin,
		platform::MonotonicTimestamp end ) const override
	{
		return end.ticks - begin.ticks;
	}

private:
	mutable std::uint64_t m_current = 0;
};

// DEFECT: elapsed conversion is clamped to a tiny cap, breaking additivity
// (elapsed(a,c) != elapsed(a,b) + elapsed(b,c) once deltas exceed the cap).
class CClampedElapsedClock : public platform::IMonotonicClock
{
public:
	platform::MonotonicTimestamp Now() const override
	{
		platform::MonotonicTimestamp t;
		t.ticks = m_current;
		m_current += 1000;
		return t;
	}
	std::uint64_t ResolutionNanoseconds() const override { return 1000; }
	std::uint64_t ElapsedNanoseconds( platform::MonotonicTimestamp begin,
		platform::MonotonicTimestamp end ) const override
	{
		const std::uint64_t delta = end.ticks - begin.ticks;
		return delta < 1 ? delta : 1; // clamp to 1
	}

private:
	mutable std::uint64_t m_current = 0;
};

struct Case
{
	platform::IMonotonicClock *clock;
	const char *name;
};

bool Caught( platform::IMonotonicClock &clock )
{
	platformtest::ClockReport r = platformtest::RunMonotonicClockConformance( clock, 8 );
	return r.failures > 0;
}

} // namespace

int main()
{
	int failures = 0;

	// 1) The conforming virtual-time backend must PASS.
	{
		platformtest::CFakeMonotonicClock good;
		platformtest::ClockReport r =
			platformtest::RunMonotonicClockConformance( good, 8 );
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: conforming clock rejected by suite (%d/%d); "
				"first: %s (line %d)\n",
				r.failures, r.checks, r.firstFailure, r.firstFailureLine );
			++failures;
		}
	}

	// 2) Every broken clock must be CAUGHT.
	CBackwardClock backward;
	CZeroResolutionClock zeroRes;
	CClampedElapsedClock clamped;
	const Case cases[] = {
		{ &backward, "backward-time" },
		{ &zeroRes, "zero-resolution" },
		{ &clamped, "non-additive-elapsed" },
	};
	for ( const Case &c : cases )
	{
		if ( !Caught( *c.clock ) )
		{
			std::printf( "FAIL: broken clock '%s' was NOT caught by the suite\n",
				c.name );
			++failures;
		}
	}

	if ( failures == 0 )
	{
		std::printf( "ok test_clock_negative: suite accepts conforming and rejects "
			"all %zu broken clocks\n", sizeof( cases ) / sizeof( cases[0] ) );
		return 0;
	}
	return 1;
}
