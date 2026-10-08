//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the RFC 0001 wall-clock conformance suite
//			(PLAT-WALLCLOCK-001, Q-FOUNDATION). Feeds the SAME shared predicate
//			broken clocks, each violating one clause, and asserts every one is
//			caught while the conforming backend passes.
//
//			Build/run: tools/quality/conformance.py check --suite platform.wall_clock.sensitivity
//
//=============================================================================//

#include "fake_wall_clock.h"
#include "wall_clock_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

using platform::CivilTime;
using platform::CivilZone;
using platform::WallTime;
using platformtest::CFakeWallClock;

// A conforming clock whose breakdown a subclass may corrupt.
class CBrokenBase : public CFakeWallClock
{
public:
	bool ToCivil( WallTime time, CivilZone zone, CivilTime &out ) const override
	{
		if ( !CFakeWallClock::ToCivil( time, zone, out ) )
		{
			return false;
		}
		Corrupt( time, zone, out );
		return true;
	}

protected:
	virtual void Corrupt( WallTime, CivilZone, CivilTime & ) const {}
};

// DEFECT: months are zero-based.
class CZeroBasedMonth : public CBrokenBase
{
	void Corrupt( WallTime, CivilZone, CivilTime &c ) const override { c.month -= 1; }
};

// DEFECT: century years are always leap years (2100-02-29 exists).
class CJulianLeap : public CBrokenBase
{
	void Corrupt( WallTime t, CivilZone, CivilTime &c ) const override
	{
		if ( c.year % 100 == 0 && c.year % 400 != 0 && c.month == 3 && c.day == 1 &&
		     t.unixNanoseconds > 0 )
		{
			c.month = 2;
			c.day = 29;
		}
	}
};

// DEFECT: the local offset is reported with the wrong sign.
class CFlippedOffset : public CBrokenBase
{
	void Corrupt( WallTime, CivilZone zone, CivilTime &c ) const override
	{
		if ( zone == CivilZone::kLocal )
		{
			c.utcOffsetSeconds = -c.utcOffsetSeconds;
		}
	}
};

// DEFECT: truncates toward zero, so instants before the epoch round the wrong way.
class CTruncatingClock : public CFakeWallClock
{
public:
	bool ToCivil( WallTime time, CivilZone zone, CivilTime &out ) const override
	{
		if ( time.unixNanoseconds < 0 && time.unixNanoseconds % 1000000000LL != 0 )
		{
			time.unixNanoseconds -= time.unixNanoseconds % 1000000000LL;
		}
		return CFakeWallClock::ToCivil( time, zone, out );
	}
};

// DEFECT: drops the sub-second part.
class CDropsNanos : public CBrokenBase
{
	void Corrupt( WallTime, CivilZone, CivilTime &c ) const override { c.nanosecond = 0; }
};

// DEFECT: a failed local lookup still overwrites `out`.
class CClobbersOnRefusal : public CFakeWallClock
{
public:
	CClobbersOnRefusal() : CFakeWallClock( 0, 0, false ) {}
	bool ToCivil( WallTime time, CivilZone zone, CivilTime &out ) const override
	{
		if ( zone == CivilZone::kLocal )
		{
			out = CivilTime();
			return false;
		}
		return CFakeWallClock::ToCivil( time, zone, out );
	}
};

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;

	{
		CFakeWallClock good;
		const platformtest::WallClockReport r = platformtest::RunWallClockConformance( good );
		++checks;
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: conforming clock rejected (%d/%d); first: %s (line %d)\n",
			    r.failures, r.checks, r.firstFailure, r.firstFailureLine );
			++failures;
		}
	}

	CZeroBasedMonth zeroMonth;
	CJulianLeap julian;
	CFlippedOffset flipped;
	CTruncatingClock truncating;
	CDropsNanos dropsNanos;
	CClobbersOnRefusal clobbers;
	struct Case
	{
		const platform::IWallClock *clock;
		const char *name;
	};
	const Case cases[] = {
	    { &zeroMonth, "zero-based-month" },
	    { &julian, "julian-leap-years" },
	    { &flipped, "flipped-local-offset" },
	    { &truncating, "truncates-pre-epoch" },
	    { &dropsNanos, "drops-nanoseconds" },
	    { &clobbers, "clobbers-out-on-refusal" },
	};
	for ( const Case &c : cases )
	{
		++checks;
		if ( platformtest::RunWallClockConformance( *c.clock ).failures == 0 )
		{
			std::printf( "FAIL: broken clock '%s' was NOT caught\n", c.name );
			++failures;
		}
	}
	if ( failures == 0 )
	{
		std::printf( "ok test_wall_clock_negative: all %zu broken clocks caught\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
