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

// The provider aborts if destroyed with threads unjoined; leaked, it never is.
#if defined( _WIN32 )
platform::IWin32Threads &Threads()
{
	static platform::IWin32Threads &threads = Leak( platform::CreateWin32Threads() );
	return threads;
}
#else
platform::IPosixThreads &Threads()
{
	static platform::IPosixThreads &threads = Leak( platform::CreatePosixThreads() );
	return threads;
}
#endif

std::uint64_t MonotonicNanoseconds()
{
	return MonotonicClock().ElapsedNanoseconds( platform::MonotonicTimestamp(), MonotonicClock().Now() );
}

#if defined( _WIN32 )
const platform::IWin32ProcessEnvironment &ProcessEnvironment()
{
	static const platform::IWin32ProcessEnvironment &environment =
		Leak( platform::CreateWin32ProcessEnvironment() );
	return environment;
}
#else
const platform::IProcessEnvironment &ProcessEnvironment()
{
	static const platform::IProcessEnvironment &environment =
		Leak( platform::CreateSystemProcessEnvironment() );
	return environment;
}
#endif

bool HasEnvironmentVariable( const char *name )
{
	return ProcessEnvironment().VariableLength( name ) >= 0;
}

platform::IDebugOutput *DebugOutput()
{
#if defined( _WIN32 )
	static platform::IDebugOutput *output = &Leak( platform::CreateWin32DebuggerOutput() );
#elif defined( ANDROID ) || defined( __ANDROID__ )
	static platform::IDebugOutput *output = &Leak( platform::CreateAndroidLogDebugOutput( "SRCENG" ) );
#else
	static platform::IDebugOutput *output = nullptr;
#endif
	return output;
}

double SecondsSinceStart()
{
	const TimeBase &base = Base();
	return static_cast<double>( base.clock.ElapsedNanoseconds( base.start, base.clock.Now() ) ) * 1e-9;
}

} // namespace tier0_facade
