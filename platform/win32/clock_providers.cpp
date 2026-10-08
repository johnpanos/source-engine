//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 monotonic and wall clocks (platform.clock.v1,
//			platform.wall-clock.v1).
//
//=============================================================================//

#include "foundation_providers.h"

#include "win32_text.h"

namespace platform
{
namespace
{

constexpr std::int64_t kNsPerSecond = 1000000000LL;
// 100 ns FILETIME intervals between 1601-01-01 and 1970-01-01.
constexpr std::int64_t kEpochDelta100ns = 116444736000000000LL;

class CWin32MonotonicClock final : public IMonotonicClock
{
public:
	CWin32MonotonicClock()
	{
		LARGE_INTEGER f;
		QueryPerformanceFrequency( &f );
		m_frequency = static_cast<std::uint64_t>( f.QuadPart );
	}

	MonotonicTimestamp Now() const override
	{
		LARGE_INTEGER c;
		QueryPerformanceCounter( &c );
		// Ticks are nanoseconds: the counter converted once, monotonically, so
		// elapsed time is a subtraction and exactly additive.
		const std::uint64_t counter = static_cast<std::uint64_t>( c.QuadPart );
		MonotonicTimestamp t;
		t.ticks = counter / m_frequency * 1000000000ULL +
		          counter % m_frequency * 1000000000ULL / m_frequency;
		return t;
	}

	std::uint64_t ResolutionNanoseconds() const override
	{
		const std::uint64_t ns = ( 1000000000ULL + m_frequency - 1 ) / m_frequency;
		return ns > 0 ? ns : 1;
	}

	std::uint64_t ElapsedNanoseconds(
	    MonotonicTimestamp begin, MonotonicTimestamp end ) const override
	{
		return end.ticks - begin.ticks;
	}

private:
	std::uint64_t m_frequency = 1;
};

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

std::int64_t DaysFromCivil( int y, int m, int d )
{
	y -= m <= 2 ? 1 : 0;
	const std::int64_t era = ( y >= 0 ? y : y - 399 ) / 400;
	const std::int64_t yoe = y - era * 400;
	const std::int64_t doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
	return era * 146097 + yoe * 365 + yoe / 4 - yoe / 100 + doy - 719468;
}

class CWin32WallClock final : public IWallClock
{
public:
	WallTime Now() const override
	{
		FILETIME ft;
		GetSystemTimePreciseAsFileTime( &ft );
		const std::int64_t intervals =
		    ( static_cast<std::int64_t>( ft.dwHighDateTime ) << 32 ) | ft.dwLowDateTime;
		WallTime t;
		t.unixNanoseconds = ( intervals - kEpochDelta100ns ) * 100;
		return t;
	}

	bool ToCivil( WallTime time, CivilZone zone, CivilTime &out ) const override
	{
		std::int64_t seconds = time.unixNanoseconds / kNsPerSecond;
		std::int64_t rem = time.unixNanoseconds % kNsPerSecond;
		if ( rem < 0 )
		{
			rem += kNsPerSecond;
			--seconds;
		}
		std::int64_t days = seconds / 86400;
		std::int64_t secOfDay = seconds % 86400;
		if ( secOfDay < 0 )
		{
			secOfDay += 86400;
			--days;
		}
		CivilTime utc;
		CivilFromDays( days, utc.year, utc.month, utc.day );
		utc.hour = static_cast<int>( secOfDay / 3600 );
		utc.minute = static_cast<int>( secOfDay / 60 % 60 );
		utc.second = static_cast<int>( secOfDay % 60 );
		utc.nanosecond = static_cast<int>( rem );
		if ( zone == CivilZone::kUtc )
		{
			out = utc;
			return true;
		}

		// SYSTEMTIME covers years 1601-30827.
		if ( utc.year < 1601 )
		{
			return false;
		}
		SYSTEMTIME st{};
		st.wYear = static_cast<WORD>( utc.year );
		st.wMonth = static_cast<WORD>( utc.month );
		st.wDay = static_cast<WORD>( utc.day );
		st.wHour = static_cast<WORD>( utc.hour );
		st.wMinute = static_cast<WORD>( utc.minute );
		st.wSecond = static_cast<WORD>( utc.second );
		// The current zone's rules for that year (daylight-saving dates differ by
		// year), then the conversion that every Windows version and Wine provide.
		DYNAMIC_TIME_ZONE_INFORMATION dynamicZone{};
		TIME_ZONE_INFORMATION zoneInfo{};
		if ( GetDynamicTimeZoneInformation( &dynamicZone ) == TIME_ZONE_ID_INVALID ||
		     !GetTimeZoneInformationForYear( st.wYear, &dynamicZone, &zoneInfo ) )
		{
			return false;
		}
		SYSTEMTIME local{};
		if ( !SystemTimeToTzSpecificLocalTime( &zoneInfo, &st, &local ) )
		{
			return false;
		}
		CivilTime c;
		c.year = local.wYear;
		c.month = local.wMonth;
		c.day = local.wDay;
		c.hour = local.wHour;
		c.minute = local.wMinute;
		c.second = local.wSecond;
		c.nanosecond = utc.nanosecond;
		const std::int64_t localSeconds = DaysFromCivil( c.year, c.month, c.day ) * 86400 +
		                                  c.hour * 3600 + c.minute * 60 + c.second;
		c.utcOffsetSeconds = static_cast<int>( localSeconds - seconds );
		out = c;
		return true;
	}
};

} // namespace

std::unique_ptr<IMonotonicClock> CreateWin32MonotonicClock()
{
	return std::make_unique<CWin32MonotonicClock>();
}

std::unique_ptr<IWallClock> CreateWin32WallClock()
{
	return std::make_unique<CWin32WallClock>();
}

} // namespace platform
