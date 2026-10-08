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

#include <cstdint>
#include <memory>

namespace platform
{

// CLOCK_MONOTONIC; ticks are nanoseconds.
[[nodiscard]] std::unique_ptr<IMonotonicClock> CreatePosixMonotonicClock();

// CLOCK_REALTIME; local breakdown through localtime_r and tm_gmtoff.
[[nodiscard]] std::unique_ptr<IWallClock> CreatePosixWallClock();

// The POSIX thread provider's backend-only extension: native identity for a
// legacy facade whose exports return native values (Tier 0's ThreadHandle_t
// and ThreadId_t are pthread_t; R103). Not part of platform.thread.v1, and
// declared only in this backend header, so portable code never sees it.
// Native values are pthread_t as an integer (a pointer on Apple).
class IPosixThreads : public IThreads
{
public:
	// The pthread_t of a thread this provider started and has not released.
	virtual bool NativeOf( ThreadHandle thread, std::uintptr_t &native ) const = 0;

	// The calling thread's pthread_self().
	virtual std::uintptr_t CurrentNative() const = 0;

	// Whether the thread `native` names still exists (pthread_kill( t, 0 )).
	virtual bool IsNativeAlive( std::uintptr_t native ) const = 0;

	// Waits for `native` to end: a thread this provider started is joined and
	// released; any other joinable pthread is joined directly. kInvalidArgument
	// when it cannot be joined (unknown, detached, or the caller itself).
	virtual ThreadResult JoinNative( std::uintptr_t native ) = 0;

	// Releases a thread this provider started without waiting: it runs to its
	// end and its resources go with it. The handle is invalid afterwards.
	virtual ThreadResult Detach( ThreadHandle thread ) = 0;

	// Detach by native value; kInvalidArgument when this provider does not hold
	// a thread with that value (already joined or released, or never its own).
	virtual ThreadResult DetachNative( std::uintptr_t native ) = 0;

	// Names the thread `native` names (UTF-8, truncated like SetCurrentName).
	// kUnsupported where only the calling thread can be named (Apple) and
	// `native` is another thread.
	virtual ThreadResult SetNativeName( std::uintptr_t native, const char *name ) = 0;

	// pthread_kill( native, signal ). A signal whose action ends the process
	// (SIGKILL) ends the process, whichever thread it is sent to.
	virtual ThreadResult SignalNative( std::uintptr_t native, int signal ) = 0;
};

// pthreads. Sleeps against CLOCK_MONOTONIC, the clock CreatePosixMonotonicClock
// reads. Destroying the provider while a started thread is unjoined aborts.
[[nodiscard]] std::unique_ptr<IPosixThreads> CreatePosixThreads();

// mmap/mprotect. Decommit maps fresh anonymous pages over the range, so a
// recommit reads zero on every POSIX kernel (MADV_DONTNEED does not zero on
// Apple). Destroying the provider releases any reservation still held.
[[nodiscard]] std::unique_ptr<IVirtualMemory> CreatePosixVirtualMemory();

// A snapshot of argv and environ, taken now. `argv` may be null when argc is 0
// (a mobile container that supplies none).
[[nodiscard]] std::unique_ptr<IProcessEnvironment> CreatePosixProcessEnvironment(
    int argc, const char *const *argv );

// The arguments as the OS reports them to the process itself, for code that is
// never handed argv (a legacy facade such as Tier 0): /proc/self/cmdline on
// Linux and Android, _NSGetArgv on Apple, none elsewhere (ArgumentCount() 0).
// The environment is snapshotted as by CreatePosixProcessEnvironment.
[[nodiscard]] std::unique_ptr<IProcessEnvironment> CreateSystemProcessEnvironment();

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

// _Unwind_Backtrace, which every POSIX toolchain here has (glibc, bionic,
// Apple, musl). Safe from any thread.
[[nodiscard]] std::unique_ptr<IStackCapture> CreatePosixStackCapture();

// SIGALRM and alarm(2): one process-wide watchdog (the signal is the
// process's). Fire runs in the signal handler, so it must be
// async-signal-safe. Disarm restores the default SIGALRM action. Destroying
// the provider disarms it.
[[nodiscard]] std::unique_ptr<IWatchdog> CreatePosixWatchdog();

// A reporter with no capture (IsAvailable() false), for profiles without one.
[[nodiscard]] std::unique_ptr<ICrashReporter> CreateUnavailableCrashReporter();

} // namespace platform

#endif // PLATFORM_POSIX_FOUNDATION_PROVIDERS_H
