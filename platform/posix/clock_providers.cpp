//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX monotonic and wall clocks (platform.clock.v1,
//			platform.wall-clock.v1).
//
//=============================================================================//

#include "foundation_providers.h"

#include <ctime>

namespace platform
{
namespace
{

constexpr std::int64_t kNsPerSecond = 1000000000LL;

class CPosixMonotonicClock final : public IMonotonicClock
{
public:
	MonotonicTimestamp Now() const override
	{
		timespec ts{};
		clock_gettime( CLOCK_MONOTONIC, &ts );
		MonotonicTimestamp t;
		t.ticks = static_cast<std::uint64_t>( ts.tv_sec ) * kNsPerSecond +
		          static_cast<std::uint64_t>( ts.tv_nsec );
		return t;
	}

	std::uint64_t ResolutionNanoseconds() const override
	{
		timespec ts{};
		if ( clock_getres( CLOCK_MONOTONIC, &ts ) != 0 )
		{
			return 1;
		}
		const std::uint64_t ns = static_cast<std::uint64_t>( ts.tv_sec ) * kNsPerSecond +
		                         static_cast<std::uint64_t>( ts.tv_nsec );
		return ns > 0 ? ns : 1;
	}

	std::uint64_t ElapsedNanoseconds(
	    MonotonicTimestamp begin, MonotonicTimestamp end ) const override
	{
		return end.ticks - begin.ticks;
	}
};

// Floor division of an instant into seconds and a non-negative remainder.
void SplitInstant( std::int64_t unixNs, std::int64_t &seconds, int &nanoseconds )
{
	seconds = unixNs / kNsPerSecond;
	std::int64_t rem = unixNs % kNsPerSecond;
	if ( rem < 0 )
	{
		rem += kNsPerSecond;
		--seconds;
	}
	nanoseconds = static_cast<int>( rem );
}

// Proleptic Gregorian date from days since 1970-01-01 (H. Hinnant's
// civil_from_days); used for UTC so every representable instant converts.
void CivilFromDays( std::int64_t z, int &year, int &month, int &day )
{
	z += 719468;
	const std::int64_t era = ( z >= 0 ? z : z - 146096 ) / 146097;
	const std::int64_t doe = z - era * 146097;
	const std::int64_t yoe = ( doe - doe / 1460 + doe / 36524 - doe / 146096 ) / 365;
	const std::int64_t doy = doe - ( 365 * yoe + yoe / 4 - yoe / 100 );
	const std::int64_t mp = ( 5 * doy + 2 ) / 153;
	day = static_cast<int>( doy - ( 153 * mp + 2 ) / 5 + 1 );
	month = static_cast<int>( mp < 10 ? mp + 3 : mp - 9 );
	year = static_cast<int>( yoe + era * 400 + ( month <= 2 ? 1 : 0 ) );
}

class CPosixWallClock final : public IWallClock
{
public:
	WallTime Now() const override
	{
		timespec ts{};
		clock_gettime( CLOCK_REALTIME, &ts );
		WallTime t;
		t.unixNanoseconds = static_cast<std::int64_t>( ts.tv_sec ) * kNsPerSecond + ts.tv_nsec;
		return t;
	}

	bool ToCivil( WallTime time, CivilZone zone, CivilTime &out ) const override
	{
		std::int64_t seconds = 0;
		int nanoseconds = 0;
		SplitInstant( time.unixNanoseconds, seconds, nanoseconds );
		if ( zone == CivilZone::kUtc )
		{
			std::int64_t days = seconds / 86400;
			std::int64_t rem = seconds % 86400;
			if ( rem < 0 )
			{
				rem += 86400;
				--days;
			}
			CivilTime c;
			CivilFromDays( days, c.year, c.month, c.day );
			c.hour = static_cast<int>( rem / 3600 );
			c.minute = static_cast<int>( rem / 60 % 60 );
			c.second = static_cast<int>( rem % 60 );
			c.nanosecond = nanoseconds;
			c.utcOffsetSeconds = 0;
			out = c;
			return true;
		}
		const time_t t = static_cast<time_t>( seconds );
		tm parts{};
		if ( static_cast<std::int64_t>( t ) != seconds || localtime_r( &t, &parts ) == nullptr ||
		     parts.tm_sec > 59 )
		{
			return false; // out of time_t range, or a leap second we cannot represent
		}
		CivilTime c;
		c.year = parts.tm_year + 1900;
		c.month = parts.tm_mon + 1;
		c.day = parts.tm_mday;
		c.hour = parts.tm_hour;
		c.minute = parts.tm_min;
		c.second = parts.tm_sec;
		c.nanosecond = nanoseconds;
		c.utcOffsetSeconds = static_cast<int>( parts.tm_gmtoff );
		out = c;
		return true;
	}
};

} // namespace

std::unique_ptr<IMonotonicClock> CreatePosixMonotonicClock()
{
	return std::make_unique<CPosixMonotonicClock>();
}

std::unique_ptr<IWallClock> CreatePosixWallClock()
{
	return std::make_unique<CPosixWallClock>();
}

} // namespace platform
