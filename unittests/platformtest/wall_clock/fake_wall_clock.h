//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic backend for platform::IWallClock. Now() returns a
//			settable instant; local time is UTC plus one fixed offset (no
//			daylight-saving rules). This is a CONFORMING provider: the positive
//			subject of the shared wall-clock suite.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_WALL_CLOCK_H
#define PLATFORMTEST_FAKE_WALL_CLOCK_H

#include "platform/contracts/wall_clock.h"

namespace platformtest
{

// Days since 1970-01-01 to a proleptic Gregorian date (H. Hinnant's
// civil_from_days). The suite checks it with the inverse algorithm.
inline void CivilFromDays( std::int64_t z, int &year, int &month, int &day )
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

class CFakeWallClock : public platform::IWallClock
{
public:
	explicit CFakeWallClock( std::int64_t nowUnixNs = 1759881600LL * 1000000000LL,
	    int localOffsetSeconds = 2 * 3600, bool hasLocalZone = true )
	    : m_now( nowUnixNs ), m_offset( localOffsetSeconds ), m_hasLocal( hasLocalZone )
	{
	}

	platform::WallTime Now() const override
	{
		platform::WallTime t;
		t.unixNanoseconds = m_now;
		return t;
	}

	bool ToCivil(
	    platform::WallTime time, platform::CivilZone zone, platform::CivilTime &out ) const override
	{
		if ( zone == platform::CivilZone::kLocal && !m_hasLocal )
		{
			return false;
		}
		const int offset = zone == platform::CivilZone::kLocal ? m_offset : 0;
		return Convert( time.unixNanoseconds, offset, out );
	}

	void Set( std::int64_t unixNs ) { m_now = unixNs; }

	// Floor-divides to seconds, shifts by the offset and breaks the result down.
	static bool Convert( std::int64_t unixNs, int offsetSeconds, platform::CivilTime &out )
	{
		const std::int64_t kNs = 1000000000LL;
		std::int64_t secs = unixNs / kNs;
		std::int64_t nanos = unixNs % kNs;
		if ( nanos < 0 )
		{
			nanos += kNs;
			--secs;
		}
		secs += offsetSeconds;
		std::int64_t days = secs / 86400;
		std::int64_t rem = secs % 86400;
		if ( rem < 0 )
		{
			rem += 86400;
			--days;
		}
		platform::CivilTime c;
		CivilFromDays( days, c.year, c.month, c.day );
		c.hour = static_cast<int>( rem / 3600 );
		c.minute = static_cast<int>( rem / 60 % 60 );
		c.second = static_cast<int>( rem % 60 );
		c.nanosecond = static_cast<int>( nanos );
		c.utcOffsetSeconds = offsetSeconds;
		out = c;
		return true;
	}

private:
	std::int64_t m_now;
	int m_offset;
	bool m_hasLocal;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_WALL_CLOCK_H
