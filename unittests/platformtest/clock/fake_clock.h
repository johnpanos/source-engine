//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic virtual-time backend for platform::IMonotonicClock
//			(RFC 0001 injectable test clock). Tick unit is nanoseconds. Each Now()
//			advances virtual time by a fixed step so a subsystem test observes
//			reproducible forward motion without sleeping or touching a real timer.
//
//			This is a CONFORMING provider: the positive subject of the shared
//			clock conformance suite.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_CLOCK_H
#define PLATFORMTEST_FAKE_CLOCK_H

#include "platform/contracts/clock.h"

namespace platformtest
{

class CFakeMonotonicClock : public platform::IMonotonicClock
{
public:
	// resolutionNs is the reported resolution; stepNs is how far virtual time
	// advances on each Now(). Ticks are nanoseconds, so conversion is exact.
	explicit CFakeMonotonicClock( std::uint64_t resolutionNs = 1000,
		std::uint64_t stepNs = 1000 )
		: m_resolutionNs( resolutionNs ), m_stepNs( stepNs )
	{
	}

	platform::MonotonicTimestamp Now() const override
	{
		platform::MonotonicTimestamp t;
		t.ticks = m_current;
		m_current += m_stepNs;
		return t;
	}

	std::uint64_t ResolutionNanoseconds() const override { return m_resolutionNs; }

	std::uint64_t ElapsedNanoseconds( platform::MonotonicTimestamp begin,
		platform::MonotonicTimestamp end ) const override
	{
		return end.ticks - begin.ticks; // ticks are nanoseconds
	}

	// Explicit control for tests that want to set virtual time directly.
	void Advance( std::uint64_t ns ) { m_current += ns; }

private:
	mutable std::uint64_t m_current = 0;
	std::uint64_t m_resolutionNs;
	std::uint64_t m_stepNs;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_CLOCK_H
