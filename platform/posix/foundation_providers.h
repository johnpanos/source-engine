//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX providers of the RFC 0001 foundation capabilities (R26):
//			monotonic and wall clocks, threads, virtual memory, the process
//			environment, platform paths, debug output and the crash reporter.
//			Linux, Android and Apple share these; native SDK types stay in
//			platform/posix. Application roots create them and inject the
//			contracts; portable code never includes this header.
//
//=============================================================================//

#ifndef PLATFORM_POSIX_FOUNDATION_PROVIDERS_H
#define PLATFORM_POSIX_FOUNDATION_PROVIDERS_H

#include "platform/contracts/clock.h"
#include "platform/contracts/diagnostics.h"
#include "platform/contracts/module_resolver.h"
#include "platform/contracts/paths.h"
#include "platform/contracts/process_environment.h"
#include "platform/contracts/thread.h"
#include "platform/contracts/virtual_memory.h"
#include "platform/contracts/wall_clock.h"

#include <memory>

namespace platform
{

// CLOCK_MONOTONIC; ticks are nanoseconds.
[[nodiscard]] std::unique_ptr<IMonotonicClock> CreatePosixMonotonicClock();

// CLOCK_REALTIME; local breakdown through localtime_r and tm_gmtoff.
[[nodiscard]] std::unique_ptr<IWallClock> CreatePosixWallClock();

// pthreads. Sleeps against CLOCK_MONOTONIC, the clock CreatePosixMonotonicClock
// reads. Destroying the provider while a started thread is unjoined aborts.
[[nodiscard]] std::unique_ptr<IThreads> CreatePosixThreads();

// mmap/mprotect. Decommit maps fresh anonymous pages over the range, so a
// recommit reads zero on every POSIX kernel (MADV_DONTNEED does not zero on
// Apple). Destroying the provider releases any reservation still held.
[[nodiscard]] std::unique_ptr<IVirtualMemory> CreatePosixVirtualMemory();

// A snapshot of argv and environ, taken now. `argv` may be null when argc is 0
// (a mobile container that supplies none).
[[nodiscard]] std::unique_ptr<IProcessEnvironment> CreatePosixProcessEnvironment(
    int argc, const char *const *argv );

// Locations a root supplies when the OS cannot report them itself (an app
// container gives its data, cache and library directories through SDL or JNI).
// A null entry is unavailable; every value is normalized on creation and a
// value that cannot be normalized makes creation fail (null result).
struct PlatformPathValues
{
	const char *executableFile = nullptr;
	const char *userData = nullptr;
	const char *temp = nullptr;
	const char *nativeLibraryDir = nullptr;
};

// Desktop Linux: the executable from /proc/self/exe, user data from
// $XDG_DATA_HOME or $HOME/.local/share, temp from $TMPDIR or /tmp, native
// libraries beside the executable. Null when the executable cannot be found.
[[nodiscard]] std::unique_ptr<IPlatformPaths> CreateLinuxPlatformPaths();

// Exactly the supplied values (the executable directory derives from the file).
[[nodiscard]] std::unique_ptr<IPlatformPaths> CreateSuppliedPlatformPaths(
    const PlatformPathValues &values );

// Writes each message to `fd` with one write(2) per message, framed as
// "[info] ", "[warn] " or "[error] ", the message, then '\n'. The fd is borrowed
// and must outlive the provider.
[[nodiscard]] std::unique_ptr<IDebugOutput> CreateFdDebugOutput( int fd );

// Android logcat through __android_log_write, one entry per message with the
// severity as its priority. Null on other platforms.
[[nodiscard]] std::unique_ptr<IDebugOutput> CreateAndroidLogDebugOutput( const char *tag );

// Writes reports as text files into `reportDir` (it must exist): the reason,
// pid, time, every annotation and a backtrace. When `installHandler` is true it
// also installs handlers for SIGSEGV, SIGBUS, SIGILL, SIGFPE and SIGABRT on an
// alternate stack that write a report with only async-signal-safe calls, then
// restore the previous handler and re-raise. Only one reporter may hold the
// handlers at a time; a second returns null. Null when `reportDir` is null or
// too long.
[[nodiscard]] std::unique_ptr<ICrashReporter> CreatePosixCrashReporter(
    const char *reportDir, bool installHandler );

// stat(2): follows symbolic links; reads only kPosixBytes paths (any other
// flavor is kMissing).
[[nodiscard]] std::unique_ptr<IFileProbe> CreatePosixFileProbe();

// Native paths from OS strings: bytes as given (null or empty: the empty path),
// and the current working directory (empty when getcwd fails).
NativePath PosixNativePath( const char *bytes );
NativePath PosixCurrentDirectory();

// A reporter with no capture (IsAvailable() false), for profiles without one.
[[nodiscard]] std::unique_ptr<ICrashReporter> CreateUnavailableCrashReporter();

} // namespace platform

#endif // PLATFORM_POSIX_FOUNDATION_PROVIDERS_H
