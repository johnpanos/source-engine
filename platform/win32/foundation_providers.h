//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 providers of the RFC 0001 foundation capabilities (R26):
//			monotonic and wall clocks, threads, virtual memory, the process
//			environment, platform paths, debug output and the crash reporter.
//			Native handles stay in platform/win32; application roots create the
//			providers and inject the contracts.
//
//=============================================================================//

#ifndef PLATFORM_WIN32_FOUNDATION_PROVIDERS_H
#define PLATFORM_WIN32_FOUNDATION_PROVIDERS_H

#include "platform/contracts/clock.h"
#include "platform/contracts/diagnostics.h"
#include "platform/contracts/paths.h"
#include "platform/contracts/process_environment.h"
#include "platform/contracts/thread.h"
#include "platform/contracts/virtual_memory.h"
#include "platform/contracts/wall_clock.h"

#include <memory>

namespace platform
{

// QueryPerformanceCounter; ticks are counter units converted exactly.
[[nodiscard]] std::unique_ptr<IMonotonicClock> CreateWin32MonotonicClock();

// GetSystemTimePreciseAsFileTime; local breakdown through the current dynamic
// time zone (SystemTimeToTzSpecificLocalTimeEx).
[[nodiscard]] std::unique_ptr<IWallClock> CreateWin32WallClock();

// _beginthreadex. Sleeps against QueryPerformanceCounter. Destroying the
// provider while a started thread is unjoined aborts.
[[nodiscard]] std::unique_ptr<IThreads> CreateWin32Threads();

// VirtualAlloc/VirtualProtect/VirtualFree. Destroying the provider releases any
// reservation still held.
[[nodiscard]] std::unique_ptr<IVirtualMemory> CreateWin32VirtualMemory();

// A snapshot of GetCommandLineW (split as CommandLineToArgvW does) and the
// environment block, converted to UTF-8. Names compare case-sensitively, as the
// contract requires on every platform; drive pseudo-variables ("=C:") are not
// variables and are skipped.
[[nodiscard]] std::unique_ptr<IProcessEnvironment> CreateWin32ProcessEnvironment();

// The executable from GetModuleFileNameW, user data from the Local AppData
// known folder, temp from GetTempPathW, native libraries beside the
// executable; '\' becomes '/'. Null when the executable cannot be found.
[[nodiscard]] std::unique_ptr<IPlatformPaths> CreateWin32PlatformPaths();

// Writes each message to `file` (a borrowed HANDLE) with one WriteFile per
// message, framed as "[info] ", "[warn] " or "[error] ", the message, "\n".
[[nodiscard]] std::unique_ptr<IDebugOutput> CreateWin32HandleDebugOutput( void *file );

// OutputDebugStringW, one call per framed message (for an attached debugger).
[[nodiscard]] std::unique_ptr<IDebugOutput> CreateWin32DebuggerOutput();

// Writes text reports into `reportDir` (UTF-8, must exist): the reason, pid,
// time, annotations and a backtrace. With `installHandler` it also installs an
// unhandled-exception filter that writes a report and then lets the process
// terminate with the original exception code. One handler owner at a time; a
// second returns null.
[[nodiscard]] std::unique_ptr<ICrashReporter> CreateWin32CrashReporter(
    const char *reportDir, bool installHandler );

} // namespace platform

#endif // PLATFORM_WIN32_FOUNDATION_PROVIDERS_H
