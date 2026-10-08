//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for wall-clock (civil/UTC) time (RFC 0001
//			foundation capability "Wall clock": civil/UTC time where required,
//			kept separate from monotonic time).
//
//			Wall time can jump (NTP correction, a user changing the clock), so it
//			is never used to measure durations; that is platform.clock.v1's job.
//			Consumers are things that show or record a date: save-game stamps,
//			logs, demo metadata. A native provider reads the system clock and
//			zone database; the deterministic test provider returns a settable
//			instant and a fixed local offset.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_WALL_CLOCK_H
#define PLATFORM_CONTRACTS_WALL_CLOCK_H

// Contract header: standard library only. No tier0/tier1, no native SDK, no
// OS-selection macros. Must compile under linux-headless-core.
#include <cstdint>

namespace platform
{

// An instant as nanoseconds since 1970-01-01T00:00:00Z, not counting leap
// seconds (POSIX time). Negative values are instants before the epoch. The
// representable range is about 1677-09-21 to 2262-04-11.
struct WallTime
{
	std::int64_t unixNanoseconds = 0;
};

// Which zone a civil breakdown is expressed in.
enum class CivilZone
{
	kUtc = 0,
	kLocal,
};

// A broken-down civil time. Fields use human ranges (month 1-12, day 1-31).
// utcOffsetSeconds is local minus UTC for the instant; it is 0 for kUtc.
struct CivilTime
{
	int year = 1970;
	int month = 1;
	int day = 1;
	int hour = 0;
	int minute = 0;
	int second = 0;     // 0-59; leap seconds are not represented
	int nanosecond = 0; // 0-999999999
	int utcOffsetSeconds = 0;
};

class IWallClock
{
public:
	virtual ~IWallClock() = default;

	// The current instant. Successive calls MAY decrease (the system clock
	// can be set back); measure durations with IMonotonicClock instead.
	virtual WallTime Now() const = 0;

	// Breaks `time` down in `zone` (proleptic Gregorian calendar). kUtc always
	// succeeds. kLocal returns false and leaves `out` unchanged when the zone
	// cannot be resolved for that instant (a provider without zone data reports
	// every local lookup as unavailable).
	virtual bool ToCivil( WallTime time, CivilZone zone, CivilTime &out ) const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_WALL_CLOCK_H
