//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the RFC 0001 monotonic-clock capability
//			(platform::IMonotonicClock). Every provider that claims the contract
//			-- the deterministic virtual-time backend here and a real native
//			monotonic clock when it lands -- runs THIS predicate.
//
//			See dynamic_library_conformance.h for the pattern. A fake certifies
//			contract semantics; native providers additionally certify OS timer
//			behavior. The negative test proves the predicate is not vacuous.
//
//=============================================================================//

#ifndef PLATFORMTEST_CLOCK_CONFORMANCE_H
#define PLATFORMTEST_CLOCK_CONFORMANCE_H

#include "platform/contracts/clock.h"

#include <cstdio>

namespace platformtest
{

struct ClockReport
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

#define CK_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

// Drives a monotonic clock through the RFC 0001 contract. The clock must advance
// between the samples this predicate takes; the deterministic backend advances
// on each Now(), a native backend advances by wall time. `sampleCount` samples
// are collected in call order.
inline ClockReport RunMonotonicClockConformance(
	const platform::IMonotonicClock &clock, int sampleCount = 8 )
{
	using platform::MonotonicTimestamp;

	ClockReport r;

	// Resolution must be a valid, positive value.
	CK_CHECK( r, clock.ResolutionNanoseconds() > 0 );

	// Collect a sequence of samples.
	const int kMaxSamples = 32;
	if ( sampleCount > kMaxSamples )
	{
		sampleCount = kMaxSamples;
	}
	if ( sampleCount < 2 )
	{
		sampleCount = 2;
	}
	MonotonicTimestamp samples[kMaxSamples];
	for ( int i = 0; i < sampleCount; ++i )
	{
		samples[i] = clock.Now();
	}

	// Never moves backward: ticks are a non-decreasing counter within one clock.
	for ( int i = 1; i < sampleCount; ++i )
	{
		CK_CHECK( r, samples[i].ticks >= samples[i - 1].ticks );
	}

	// Zero elapsed for a timestamp against itself.
	for ( int i = 0; i < sampleCount; ++i )
	{
		CK_CHECK( r, clock.ElapsedNanoseconds( samples[i], samples[i] ) == 0 );
	}

	// Monotonic in the end point: for a fixed begin, later ends never report
	// less elapsed time than earlier ends.
	for ( int j = 1; j < sampleCount; ++j )
	{
		const std::uint64_t prev =
			clock.ElapsedNanoseconds( samples[0], samples[j - 1] );
		const std::uint64_t cur =
			clock.ElapsedNanoseconds( samples[0], samples[j] );
		CK_CHECK( r, cur >= prev );
	}

	// Additivity across an intermediate point holds for a well-formed conversion:
	// elapsed(a,c) == elapsed(a,b) + elapsed(b,c). (The deterministic backend is
	// exact; native backends satisfy this because the conversion is linear in the
	// tick delta.)
	for ( int k = 2; k < sampleCount; ++k )
	{
		const std::uint64_t ac = clock.ElapsedNanoseconds( samples[0], samples[k] );
		const std::uint64_t ab = clock.ElapsedNanoseconds( samples[0], samples[k - 1] );
		const std::uint64_t bc =
			clock.ElapsedNanoseconds( samples[k - 1], samples[k] );
		CK_CHECK( r, ac == ab + bc );
	}

	return r;
}

inline int RunClockPositive( const char *suiteName,
	const platform::IMonotonicClock &clock, int sampleCount = 8 )
{
	ClockReport r = RunMonotonicClockConformance( clock, sampleCount );
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n",
			suiteName, r.failures, r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", suiteName, r.checks );
	return 0;
}

} // namespace platformtest

#endif // PLATFORMTEST_CLOCK_CONFORMANCE_H
