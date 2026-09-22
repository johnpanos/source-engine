//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for a monotonic clock (RFC 0001 foundation
//			capability "Monotonic clock": monotonic timestamps, elapsed-time
//			conversion, and resolution).
//
//			Portable code that needs elapsed time depends on this contract, not
//			on a native timer or an OS check. A native provider is backed by the
//			platform's monotonic timer; the deterministic test provider is backed
//			by virtual time so subsystem tests are reproducible (RFC 0001:
//			"clocks and delayed scheduling are injectable so tests can use virtual
//			time"). Wall-clock/civil time is a SEPARATE capability and is
//			deliberately not part of this contract.
//
//			The contract is behavioral. It states no tick unit: timestamps are
//			opaque and only meaningful when turned into nanoseconds through the
//			same clock's ElapsedNanoseconds().
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_CLOCK_H
#define PLATFORM_CONTRACTS_CLOCK_H

// Contract header: standard library only (fixed-width types). No tier0/tier1,
// no native SDK, no OS-selection macros. Must compile under linux-headless-core.
#include <cstdint>

namespace platform
{

// An opaque monotonic timestamp. Its tick SCALE has no portable meaning; only
// the producing clock converts a pair of timestamps into elapsed nanoseconds.
// Within a single clock, ticks are a non-decreasing counter, so two timestamps
// from the same clock may be ordered by comparing `ticks` (a <= b means a was
// produced no later than b). Timestamps from different clocks are not comparable.
struct MonotonicTimestamp
{
	std::uint64_t ticks = 0;
};

// A monotonic time source. "Monotonic" means Now() never returns a value that
// converts to negative elapsed time relative to an earlier Now(): time does not
// move backward, though two calls closer together than the resolution may return
// equal values.
class IMonotonicClock
{
public:
	virtual ~IMonotonicClock() = default;

	// The current monotonic timestamp. Successive calls are non-decreasing in
	// the clock's ordering.
	virtual MonotonicTimestamp Now() const = 0;

	// The smallest positive increment the clock can distinguish, in nanoseconds.
	// MUST be strictly greater than zero.
	virtual std::uint64_t ResolutionNanoseconds() const = 0;

	// Nanoseconds elapsed from `begin` to `end`, where `begin` was produced no
	// later than `end` by THIS clock. Contract:
	//   - ElapsedNanoseconds(t, t) == 0;
	//   - the result is non-negative for begin <= end (hence the unsigned type);
	//   - monotonic in end: for a <= b <= c, elapsed(a,b) <= elapsed(a,c).
	// Passing timestamps from a different clock, or end before begin, is a
	// programmer error.
	virtual std::uint64_t ElapsedNanoseconds(
		MonotonicTimestamp begin, MonotonicTimestamp end ) const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_CLOCK_H
