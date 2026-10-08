//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the RFC 0001 wall-clock capability
//			(platform::IWallClock). Every provider that claims the contract runs
//			THIS predicate. The oracle is independent of any provider: it maps a
//			civil breakdown back to an instant with days_from_civil (the inverse
//			of the fake's algorithm) and checks fixed calendar vectors.
//
//=============================================================================//

#ifndef PLATFORMTEST_WALL_CLOCK_CONFORMANCE_H
#define PLATFORMTEST_WALL_CLOCK_CONFORMANCE_H

#include "platform/contracts/wall_clock.h"

#include <cstdio>

namespace platformtest
{

struct WallClockReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define WC_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

// Days from 1970-01-01 for a proleptic Gregorian date (H. Hinnant's
// days_from_civil).
inline std::int64_t OracleDaysFromCivil( int y, int m, int d )
{
	y -= m <= 2 ? 1 : 0;
	const std::int64_t era = ( y >= 0 ? y : y - 399 ) / 400;
	const std::int64_t yoe = y - era * 400;
	const std::int64_t doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
	const std::int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return era * 146097 + doe - 719468;
}

inline int OracleDaysInMonth( int y, int m )
{
	static const int kDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	const bool leap = ( y % 4 == 0 && y % 100 != 0 ) || y % 400 == 0;
	return m == 2 && leap ? 29 : kDays[m - 1];
}

// True when every field is in its range.
inline bool CivilFieldsValid( const platform::CivilTime &c )
{
	return c.month >= 1 && c.month <= 12 && c.day >= 1 &&
	       c.day <= OracleDaysInMonth( c.year, c.month ) && c.hour >= 0 && c.hour <= 23 &&
	       c.minute >= 0 && c.minute <= 59 && c.second >= 0 && c.second <= 59 &&
	       c.nanosecond >= 0 && c.nanosecond <= 999999999 && c.utcOffsetSeconds >= -14 * 3600 &&
	       c.utcOffsetSeconds <= 14 * 3600;
}

// True when a breakdown denotes `unixNs`: its civil fields minus its offset
// equal the instant's floored seconds, and its nanosecond the remainder.
// Compared in seconds so the extremes of the range cannot overflow.
inline bool OracleDenotes( const platform::CivilTime &c, std::int64_t unixNs )
{
	const std::int64_t kNs = 1000000000LL;
	std::int64_t secs = unixNs / kNs;
	std::int64_t nanos = unixNs % kNs;
	if ( nanos < 0 )
	{
		nanos += kNs;
		--secs;
	}
	const std::int64_t civilSecs = OracleDaysFromCivil( c.year, c.month, c.day ) * 86400 +
	                               c.hour * 3600 + c.minute * 60 + c.second - c.utcOffsetSeconds;
	return civilSecs == secs && c.nanosecond == nanos;
}

inline bool Matches(
    const platform::CivilTime &c, int y, int mo, int d, int h, int mi, int s, int ns )
{
	return c.year == y && c.month == mo && c.day == d && c.hour == h && c.minute == mi &&
	       c.second == s && c.nanosecond == ns && c.utcOffsetSeconds == 0;
}

inline WallClockReport RunWallClockConformance( const platform::IWallClock &clock )
{
	using platform::CivilTime;
	using platform::CivilZone;
	using platform::WallTime;

	WallClockReport r;
	const std::int64_t kNs = 1000000000LL;

	// Fixed UTC vectors: the epoch, a leap day, the instant before the epoch,
	// a century non-leap year boundary and a sub-second part.
	struct Vector
	{
		std::int64_t unixNs;
		int y, mo, d, h, mi, s, ns;
	};
	const Vector vectors[] = {
	    { 0, 1970, 1, 1, 0, 0, 0, 0 },
	    { 951782400LL * kNs, 2000, 2, 29, 0, 0, 0, 0 },
	    { -1, 1969, 12, 31, 23, 59, 59, 999999999 },
	    { 4107542399LL * kNs, 2100, 2, 28, 23, 59, 59, 0 },
	    { 4107542400LL * kNs, 2100, 3, 1, 0, 0, 0, 0 },
	    { 1759926896LL * kNs + 123456789, 2025, 10, 8, 12, 34, 56, 123456789 },
	    { -9223372036854775807LL - 1, 1677, 9, 21, 0, 12, 43, 145224192 },
	    { 9223372036854775807LL, 2262, 4, 11, 23, 47, 16, 854775807 },
	};
	for ( const Vector &v : vectors )
	{
		CivilTime c;
		WallTime t;
		t.unixNanoseconds = v.unixNs;
		const bool ok = clock.ToCivil( t, CivilZone::kUtc, c );
		WC_CHECK( r, ok );
		WC_CHECK( r, ok && Matches( c, v.y, v.mo, v.d, v.h, v.mi, v.s, v.ns ) );
	}

	// Now() breaks down in UTC into valid fields that denote the same instant.
	const WallTime now = clock.Now();
	{
		CivilTime c;
		const bool ok = clock.ToCivil( now, CivilZone::kUtc, c );
		WC_CHECK( r, ok );
		WC_CHECK( r, ok && CivilFieldsValid( c ) );
		WC_CHECK( r, ok && c.utcOffsetSeconds == 0 );
		WC_CHECK( r, ok && OracleDenotes( c, now.unixNanoseconds ) );
	}

	// Local time, when available, denotes the same instant through its offset,
	// for now and for instants on both sides of a year boundary.
	const std::int64_t localProbes[] = {
	    now.unixNanoseconds, 1767225599LL * kNs, 1767225600LL * kNs, -1 };
	for ( std::int64_t ns : localProbes )
	{
		CivilTime c;
		c.year = 4242;
		WallTime t;
		t.unixNanoseconds = ns;
		if ( clock.ToCivil( t, CivilZone::kLocal, c ) )
		{
			WC_CHECK( r, CivilFieldsValid( c ) );
			WC_CHECK( r, OracleDenotes( c, ns ) );
		}
		else
		{
			WC_CHECK( r, c.year == 4242 ); // refusal leaves `out` unchanged
		}
	}

	// Every UTC breakdown over a sweep of days round-trips through the oracle.
	for ( std::int64_t day = -106750; day <= 106750; day += 97 )
	{
		CivilTime c;
		WallTime t;
		t.unixNanoseconds = ( day * 86400 + 45296 ) * kNs + 7;
		if ( clock.ToCivil( t, CivilZone::kUtc, c ) )
		{
			r.Record( CivilFieldsValid( c ) && OracleDenotes( c, t.unixNanoseconds ),
			    "utc sweep round-trips", __LINE__ );
		}
		else
		{
			r.Record( false, "utc sweep in range refused", __LINE__ );
		}
	}

	return r;
}

} // namespace platformtest

#endif // PLATFORMTEST_WALL_CLOCK_CONFORMANCE_H
