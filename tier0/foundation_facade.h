//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Tier 0's private instances of the RFC 0001 foundation providers
//			(R103, "Tier 0 facade over the foundation providers"). Tier 0's
//			exports answer through these; each mechanism has this one owner.
//			There is no setter and no registry: new code takes the contracts by
//			injection from its composition root, and this facade exists only for
//			Tier 0's legacy callers and mods.
//
//=============================================================================//

#ifndef TIER0_FOUNDATION_FACADE_H
#define TIER0_FOUNDATION_FACADE_H

#include "platform/contracts/clock.h"
#include "platform/contracts/process_environment.h"
#include "platform/contracts/thread.h"
#include "platform/contracts/wall_clock.h"

#if defined( _WIN32 )
#include "../platform/win32/foundation_providers.h"
#else
#include "../platform/posix/foundation_providers.h"
#endif

namespace tier0_facade
{

// The process's monotonic clock. Created on first use; never destroyed (Tier 0
// answers until the process ends, including from static destructors).
const platform::IMonotonicClock &MonotonicClock();

// Seconds since Tier 0 first asked the clock: the time base of Plat_FloatTime.
double SecondsSinceStart();

// The monotonic clock's reading in nanoseconds (Plat_MonotonicNanoseconds).
std::uint64_t MonotonicNanoseconds();

// The process's wall clock (civil time, deadlines for CLOCK_REALTIME waits).
const platform::IWallClock &WallClock();

// The process's thread provider (sleep, names, priorities, ids), with the
// backend extension the legacy exports need for native identity.
// The process environment: arguments as the OS reports them, a snapshot of the
// environment taken on first use, and the live debugger state.
#if defined( _WIN32 )
const platform::IWin32ProcessEnvironment &ProcessEnvironment();
#else
const platform::IProcessEnvironment &ProcessEnvironment();
#endif

// True when the variable is set (to any value, empty included), as getenv's
// non-null result was.
bool HasEnvironmentVariable( const char *name );

#if defined( _WIN32 )
platform::IWin32Threads &Threads();
#else
platform::IPosixThreads &Threads();
#endif

} // namespace tier0_facade

#endif // TIER0_FOUNDATION_FACADE_H
