//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the RFC 0001 threading capability
//			(platform::IThreads). Every provider that claims the contract runs
//			THIS predicate. It is safe under real concurrency: everything an
//			entry reports goes through atomics or is read after Join, and the
//			only wait is a bounded poll that yields through SleepFor(0).
//
//			`clock` is the monotonic clock the provider sleeps against.
//
//=============================================================================//

#ifndef PLATFORMTEST_THREAD_CONFORMANCE_H
#define PLATFORMTEST_THREAD_CONFORMANCE_H

#include "platform/contracts/clock.h"
#include "platform/contracts/thread.h"

#include <atomic>
#include <cstdio>
#include <cstring>

namespace platformtest
{

struct ThreadReport
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

#define TH_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

struct ThreadProbe
{
	platform::IThreads *threads = nullptr;
	std::atomic<int> runs{ 0 };
	std::atomic<std::uint64_t> seenId{ 0 };
	std::atomic<std::uint64_t> handle{ 0 }; // published by the starter after Start
	std::atomic<int> selfJoin{ -1 };
	char name[64] = {};
	int nameLength = -2;
	int nameSmallBuffer = -2;
	platform::ThreadResult renameResult = platform::ThreadResult::kOk;
	char renamed[64] = {};
	int renamedLength = -2;
	platform::ThreadResult priorityResult = platform::ThreadResult::kInvalidArgument;
	int payload = 0; // plain write, read after Join
	bool testSelfJoin = false;
};

inline void ThreadProbeEntry( void *context )
{
	ThreadProbe &p = *static_cast<ThreadProbe *>( context );
	p.runs.fetch_add( 1 );
	p.seenId.store( p.threads->CurrentId().value );
	p.nameLength = p.threads->GetCurrentName( p.name, sizeof( p.name ) );
	char tiny[2] = {};
	p.nameSmallBuffer = p.threads->GetCurrentName( tiny, sizeof( tiny ) );
	p.renameResult = p.threads->SetCurrentName( "renamed" );
	p.renamedLength = p.threads->GetCurrentName( p.renamed, sizeof( p.renamed ) );
	p.priorityResult = p.threads->SetCurrentPriority( platform::ThreadPriority::kLow );
	if ( p.testSelfJoin )
	{
		for ( int i = 0; i < 1000000 && p.handle.load() == 0; ++i )
		{
			p.threads->SleepFor( 0 );
		}
		platform::ThreadHandle self;
		self.value = p.handle.load();
		p.selfJoin.store( static_cast<int>( p.threads->Join( self ) ) );
	}
	p.payload = 42;
}

inline ThreadReport RunThreadConformance(
    platform::IThreads &threads, const platform::IMonotonicClock &clock )
{
	using platform::ThreadHandle;
	using platform::ThreadOptions;
	using platform::ThreadResult;

	ThreadReport r;

	TH_CHECK( r, threads.HardwareConcurrency() >= 1 );

	const std::uint64_t mainId = threads.CurrentId().value;
	TH_CHECK( r, mainId != 0 );
	TH_CHECK( r, threads.CurrentId().value == mainId );

	// A null entry is refused and leaves `out` unchanged.
	{
		ThreadHandle out;
		out.value = 777;
		TH_CHECK( r, threads.Start( ThreadOptions(), nullptr, nullptr, out ) ==
		                 ThreadResult::kInvalidArgument );
		TH_CHECK( r, out.value == 777 );
	}

	// A named thread: runs once, sees its own id and name, cannot join itself,
	// and its writes are visible after Join.
	const bool namesSupported = threads.SetCurrentName( "conformance" ) == ThreadResult::kOk;
	{
		ThreadProbe p;
		p.threads = &threads;
		p.testSelfJoin = true;
		ThreadOptions options;
		options.name = "worker-a";
		options.priority = platform::ThreadPriority::kHigh;
		ThreadHandle h;
		TH_CHECK( r, threads.Start( options, ThreadProbeEntry, &p, h ) == ThreadResult::kOk );
		TH_CHECK( r, h.value != 0 );
		const std::uint64_t idOf = threads.IdOf( h ).value;
		p.handle.store( h.value );
		TH_CHECK( r, idOf != 0 && idOf != mainId );
		TH_CHECK( r, threads.Join( h ) == ThreadResult::kOk );
		TH_CHECK( r, p.runs.load() == 1 );
		TH_CHECK( r, p.payload == 42 );
		TH_CHECK( r, p.seenId.load() == idOf );
		TH_CHECK( r, p.selfJoin.load() == static_cast<int>( ThreadResult::kInvalidArgument ) );
		TH_CHECK( r, p.priorityResult == ThreadResult::kOk ||
		                 p.priorityResult == ThreadResult::kUnsupported );
		if ( namesSupported )
		{
			TH_CHECK( r, p.nameLength == 8 && std::strcmp( p.name, "worker-a" ) == 0 );
			TH_CHECK( r, p.nameSmallBuffer == -1 );
			TH_CHECK( r, p.renameResult == ThreadResult::kOk );
			TH_CHECK( r, p.renamedLength == 7 && std::strcmp( p.renamed, "renamed" ) == 0 );
		}
		else
		{
			TH_CHECK( r, p.nameLength == -1 );
			TH_CHECK( r, p.renameResult == ThreadResult::kUnsupported );
			TH_CHECK( r, p.renamedLength == -1 );
		}

		// The handle is released: a second Join and IdOf both refuse it.
		TH_CHECK( r, threads.Join( h ) == ThreadResult::kInvalidArgument );
		TH_CHECK( r, threads.IdOf( h ).value == 0 );
	}

	// Unknown handles.
	{
		ThreadHandle zero;
		ThreadHandle bogus;
		bogus.value = 0xdeadbeefULL;
		TH_CHECK( r, threads.Join( zero ) == ThreadResult::kInvalidArgument );
		TH_CHECK( r, threads.Join( bogus ) == ThreadResult::kInvalidArgument );
		TH_CHECK( r, threads.IdOf( bogus ).value == 0 );
	}

	// A long name keeps exactly its first kThreadNameMaxBytes bytes.
	if ( namesSupported )
	{
		ThreadProbe p;
		p.threads = &threads;
		ThreadOptions options;
		options.name = "a-very-long-thread-name";
		ThreadHandle h;
		TH_CHECK( r, threads.Start( options, ThreadProbeEntry, &p, h ) == ThreadResult::kOk );
		TH_CHECK( r, threads.Join( h ) == ThreadResult::kOk );
		TH_CHECK( r, p.nameLength == platform::kThreadNameMaxBytes );
		TH_CHECK( r,
		    std::strncmp( p.name, "a-very-long-thread-name", platform::kThreadNameMaxBytes ) == 0 );
	}

	// Several live threads have distinct ids, none the caller's; each runs once.
	{
		const int kCount = 4;
		ThreadProbe probes[kCount];
		ThreadHandle handles[kCount];
		std::uint64_t ids[kCount] = {};
		for ( int i = 0; i < kCount; ++i )
		{
			probes[i].threads = &threads;
			TH_CHECK( r, threads.Start( ThreadOptions(), ThreadProbeEntry, &probes[i],
			                 handles[i] ) == ThreadResult::kOk );
			ids[i] = threads.IdOf( handles[i] ).value;
		}
		for ( int i = 0; i < kCount; ++i )
		{
			TH_CHECK( r, threads.Join( handles[i] ) == ThreadResult::kOk );
			TH_CHECK( r, probes[i].runs.load() == 1 );
			TH_CHECK( r, probes[i].seenId.load() == ids[i] );
			TH_CHECK( r, ids[i] != 0 && ids[i] != mainId );
			for ( int j = 0; j < i; ++j )
			{
				TH_CHECK( r, ids[i] != ids[j] );
			}
		}
	}

	// The caller's identity is unchanged by starting and joining threads.
	TH_CHECK( r, threads.CurrentId().value == mainId );

	// Sleep lasts at least the request on the provider's clock.
	{
		const std::uint64_t kRequest = 2000000; // 2 ms
		const platform::MonotonicTimestamp before = clock.Now();
		threads.SleepFor( kRequest );
		const platform::MonotonicTimestamp after = clock.Now();
		TH_CHECK( r, clock.ElapsedNanoseconds( before, after ) >= kRequest );
		threads.SleepFor( 0 ); // yields; must return
	}

	return r;
}

} // namespace platformtest

#endif // PLATFORMTEST_THREAD_CONFORMANCE_H
