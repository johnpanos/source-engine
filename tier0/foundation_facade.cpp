//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Tier 0's private instances of the foundation providers (R103).
//
//=============================================================================//

#include "foundation_facade.h"

#if defined( _WIN32 )
#include "../platform/win32/foundation_providers.h"
#else
#include "../platform/posix/foundation_providers.h"
#endif

namespace tier0_facade
{
namespace
{

// Leaked on purpose: Tier 0's exports may run during static destruction, after
// any ordinary static owner would have released the provider.
template <typename T> T &Leak( std::unique_ptr<T> provider )
{
	return *provider.release();
}

struct TimeBase
{
	const platform::IMonotonicClock &clock;
	platform::MonotonicTimestamp start;
};

const TimeBase &Base()
{
	static const TimeBase base{ MonotonicClock(), MonotonicClock().Now() };
	return base;
}

} // namespace

const platform::IMonotonicClock &MonotonicClock()
{
#if defined( _WIN32 )
	static const platform::IMonotonicClock &clock = Leak( platform::CreateWin32MonotonicClock() );
#else
	static const platform::IMonotonicClock &clock = Leak( platform::CreatePosixMonotonicClock() );
#endif
	return clock;
}

const platform::IWallClock &WallClock()
{
#if defined( _WIN32 )
	static const platform::IWallClock &clock = Leak( platform::CreateWin32WallClock() );
#else
	static const platform::IWallClock &clock = Leak( platform::CreatePosixWallClock() );
#endif
	return clock;
}

platform::IThreads &Threads()
{
	// The provider aborts if destroyed with threads unjoined; leaked, it never is.
#if defined( _WIN32 )
	static platform::IThreads &threads = Leak( platform::CreateWin32Threads() );
#else
	static platform::IThreads &threads = Leak( platform::CreatePosixThreads() );
#endif
	return threads;
}

std::uint64_t MonotonicNanoseconds()
{
	return MonotonicClock().ElapsedNanoseconds( platform::MonotonicTimestamp(), MonotonicClock().Now() );
}

double SecondsSinceStart()
{
	const TimeBase &base = Base();
	return static_cast<double>( base.clock.ElapsedNanoseconds( base.start, base.clock.Now() ) ) * 1e-9;
}

} // namespace tier0_facade
