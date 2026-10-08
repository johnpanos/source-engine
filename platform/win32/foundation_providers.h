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
#include "platform/contracts/module_resolver.h"
#include "platform/contracts/paths.h"
#include "platform/contracts/process_environment.h"
#include "platform/contracts/thread.h"
#include "platform/contracts/virtual_memory.h"
#include "platform/contracts/wall_clock.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace platform
{

// QueryPerformanceCounter; ticks are counter units converted exactly.
[[nodiscard]] std::unique_ptr<IMonotonicClock> CreateWin32MonotonicClock();

// GetSystemTimePreciseAsFileTime; local breakdown through the current dynamic
// time zone (SystemTimeToTzSpecificLocalTimeEx).
[[nodiscard]] std::unique_ptr<IWallClock> CreateWin32WallClock();

// The Win32 thread provider's backend-only extension: native identity and the
// native operations a legacy facade's exports perform (Tier 0's ThreadHandle_t
// is a HANDLE the caller owns, ThreadId_t a thread id; R103). Not part of
// platform.thread.v1 and declared only in this backend header. Handles are
// HANDLE values as void*, ids are DWORDs.
class IWin32Threads : public IThreads
{
public:
	using NativeProc = unsigned( __stdcall * )( void * );

	// Starts `proc( arg )` as a thread whose exit code is proc's result (the
	// _beginthreadex contract), optionally suspended. `callerHandle` is a
	// duplicate the caller owns and closes with CloseNative; the provider keeps
	// its own until Join, Detach or DetachNativeId.
	virtual ThreadResult StartNative( std::size_t stackBytes, NativeProc proc, void *arg,
		bool suspended, ThreadHandle &out, void *&callerHandle, unsigned long &id ) = 0;

	virtual unsigned long CurrentNativeId() const = 0;  // GetCurrentThreadId
	virtual void *CurrentPseudoHandle() const = 0;       // GetCurrentThread

	enum class WaitResult
	{
		kSignaled = 0,
		kTimeout,
		kFailed,
	};
	// WaitForSingleObject on a thread handle; kTimeout after `timeoutMs`
	// (0xFFFFFFFF waits forever).
	virtual WaitResult WaitNative( void *handle, unsigned long timeoutMs ) = 0;

	virtual bool IsNativeHandleRunning( void *handle ) const = 0; // exit code STILL_ACTIVE
	virtual bool IsNativeIdRunning( unsigned long id ) const = 0;  // OpenThread on the id
	virtual int GetNativePriority( void *handle ) const = 0;
	virtual bool SetNativePriority( void *handle, int priority ) = 0;
	virtual void SetNativeAffinity( void *handle, std::uintptr_t mask ) = 0;
	virtual bool TerminateNative( void *handle, unsigned long exitCode ) = 0;
	virtual bool ResumeNative( void *handle ) = 0;
	virtual bool SuspendNative( void *handle ) = 0;
	// OpenThread( access, FALSE, id ): a new handle the caller closes with CloseNative.
	virtual void *OpenNative( unsigned long id, unsigned long access ) = 0;
	virtual bool CloseNative( void *handle ) = 0;

	// Releases the provider's own handle of a thread it started, by id; the
	// thread runs on. kInvalidArgument when the provider holds no such thread.
	virtual ThreadResult DetachNativeId( unsigned long id ) = 0;
	virtual ThreadResult Detach( ThreadHandle thread ) = 0;
};

// _beginthreadex. Sleeps against QueryPerformanceCounter. Destroying the
// provider while a started thread is unjoined aborts.
[[nodiscard]] std::unique_ptr<IWin32Threads> CreateWin32Threads();

// VirtualAlloc/VirtualProtect/VirtualFree. Destroying the provider releases any
// reservation still held.
[[nodiscard]] std::unique_ptr<IVirtualMemory> CreateWin32VirtualMemory();

// A snapshot of GetCommandLineW (split as CommandLineToArgvW does) and the
// environment block, converted to UTF-8. Names compare case-sensitively, as the
// contract requires on every platform; drive pseudo-variables ("=C:") are not
// variables and are skipped.
// Backend-only extension: the raw command line exactly as the OS holds it
// (quoting included), for a legacy facade whose exports return it (Tier 0's
// Plat_GetCommandLine; R103). The pointers live as long as the process.
class IWin32ProcessEnvironment : public IProcessEnvironment
{
public:
	virtual const wchar_t *RawCommandLineW() const = 0; // GetCommandLineW
	virtual const char *RawCommandLineA() const = 0;	   // GetCommandLineA (the ANSI code page)
};

[[nodiscard]] std::unique_ptr<IWin32ProcessEnvironment> CreateWin32ProcessEnvironment();

// The executable from GetModuleFileNameW, user data from the Local AppData
// known folder, temp from GetTempPathW, native libraries beside the
// executable; '\' becomes '/'. Null when the executable cannot be found.
[[nodiscard]] std::unique_ptr<IPlatformPaths> CreateWin32PlatformPaths();

// Writes each message to `file` (a borrowed HANDLE) with one WriteFile per
// message, framed as "[info] ", "[warn] " or "[error] ", the message, "\n".
[[nodiscard]] std::unique_ptr<IDebugOutput> CreateWin32HandleDebugOutput( void *file );

// OutputDebugStringA, one call per message, unframed: the debugger channel is a
// record sink without severity, which it drops (for an attached debugger).
[[nodiscard]] std::unique_ptr<IDebugOutput> CreateWin32DebuggerOutput();

// GetFileAttributesW: follows reparse points the way CreateFileW does; reads
// only kWindowsUtf16 paths (any other flavor is kMissing).
[[nodiscard]] std::unique_ptr<IFileProbe> CreateWin32FileProbe();

// Native paths from OS strings (null or empty: the empty path), and the
// current directory.
NativePath Win32NativePath( const wchar_t *units );
NativePath Win32CurrentDirectory();

// RtlCaptureStackBackTrace. Safe from any thread.
[[nodiscard]] std::unique_ptr<IStackCapture> CreateWin32StackCapture();

// Windows has no watchdog here (Tier 0's were always stubs): IsSupported()
// is false and Arm refuses.
[[nodiscard]] std::unique_ptr<IWatchdog> CreateWin32Watchdog();

// Backend-only (R103): installs the process's unhandled-exception filter,
// which calls `callback( exceptionCode, EXCEPTION_POINTERS * )` and then
// continues the search, as Tier 0's minidump filter did. Null removes it.
// This provider is the filter's one owner in a process that uses it; it is
// not combined with CreateWin32CrashReporter's handler.
void SetUnhandledExceptionCallback( void ( *callback )( unsigned long code, void *exceptionPointers ) );

// Writes text reports into `reportDir` (UTF-8, must exist): the reason, pid,
// time, annotations and a backtrace. With `installHandler` it also installs an
// unhandled-exception filter that writes a report and then lets the process
// terminate with the original exception code. One handler owner at a time; a
// second returns null.
[[nodiscard]] std::unique_ptr<ICrashReporter> CreateWin32CrashReporter(
    const char *reportDir, bool installHandler );

} // namespace platform

#endif // PLATFORM_WIN32_FOUNDATION_PROVIDERS_H
