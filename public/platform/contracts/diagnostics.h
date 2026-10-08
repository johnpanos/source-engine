//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contracts for native diagnostics (RFC 0001 foundation
//			capability "Diagnostics": native debug output and optional
//			crash/minidump integration).
//
//			IDebugOutput is the native sink a logging owner writes through
//			(stderr, OutputDebugString, os_log, logcat); it is not a logging
//			framework and applies no filtering. ICrashReporter is OPTIONAL: a
//			provider that cannot capture reports says so through IsAvailable()
//			and refuses every call by name, rather than pretending. Installing
//			the native crash handler is the provider's start step, owned by the
//			composition root; consumers only annotate and request reports.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_DIAGNOSTICS_H
#define PLATFORM_CONTRACTS_DIAGNOSTICS_H

// Contract header: standard library only. No tier0/tier1, no native SDK, no
// OS-selection macros. Must compile under linux-headless-core.

namespace platform
{

enum class DiagnosticSeverity
{
	kInfo = 0,
	kWarning,
	kError,
};

// A native debug-output sink. Safe to call from any thread. Each Write is
// delivered whole (never interleaved with another Write) and, per thread, in
// call order. The message is UTF-8 and is delivered byte for byte, embedded
// newlines included. A text sink (stderr) frames each message with a severity
// tag and a line terminator; that framing is the sink's, and a reader that
// removes it recovers the message and severity exactly. A null message is
// ignored.
class IDebugOutput
{
public:
	virtual ~IDebugOutput() = default;
	virtual void Write( DiagnosticSeverity severity, const char *message ) = 0;
};

enum class CrashReportResult
{
	kOk = 0,
	kInvalidArgument, // malformed key, oversized value, null reason or buffer
	kUnsupported,     // IsAvailable() is false
	kFailed,          // the platform could not write the report
};

// Annotation keys: 1 to kCrashKeyMaxBytes of [A-Za-z0-9_.-].
constexpr int kCrashKeyMaxBytes = 64;
// Annotation values: UTF-8, at most this many bytes.
constexpr int kCrashValueMaxBytes = 1024;
// At most this many annotations are held at once, so a crash handler can keep
// them in fixed storage it reads without allocating.
constexpr int kCrashMaxAnnotations = 32;

class ICrashReporter
{
public:
	virtual ~ICrashReporter() = default;

	// False when the platform has no crash capture. Then every other call
	// returns kUnsupported or -1 and changes nothing.
	virtual bool IsAvailable() const = 0;

	// Sets or replaces an annotation attached to every later report, including
	// a crash. A null value removes the key (removing an absent key is kOk).
	// Adding a key beyond kCrashMaxAnnotations is kInvalidArgument.
	virtual CrashReportResult SetAnnotation( const char *key, const char *value ) = 0;

	// Same convention as platform.paths.v1: writes the value and returns its
	// length, or -1 without writing when absent, unavailable or it does not fit.
	virtual int GetAnnotation( const char *key, char *buffer, int bufferSize ) const = 0;

	// Writes a report of the current process now, without terminating it, and
	// writes its id (non-empty, NUL-terminated, unique per report) into
	// `idBuffer`. On anything but kOk `idBuffer` is unchanged.
	virtual CrashReportResult WriteReport(
	    const char *reason, char *idBuffer, int idBufferSize ) = 0;
};

// The return addresses of the calling thread's stack (R103: Tier 0's
// GetCallStack, assert backtraces and the memory ledger).
class IStackCapture
{
public:
	virtual ~IStackCapture() = default;

	// Writes up to `maxFrames` return addresses, innermost first, starting at
	// the function that called CaptureStack (the provider's own frames are not
	// included), and returns how many. Never more than maxFrames; 0 when
	// maxFrames <= 0 or frames is null. Safe to call from any thread.
	virtual int CaptureStack( void **frames, int maxFrames ) = 0;
};

// A watchdog: unless disarmed in time, calls `fire( context )` once after the
// armed delay (R103: Tier 0's Plat_*WatchdogTimer). Where a provider fires
// from a signal handler (POSIX), `fire` must be async-signal-safe. A provider
// without one reports IsSupported() false and Arm returns false, changing
// nothing.
class IWatchdog
{
public:
	virtual ~IWatchdog() = default;

	virtual bool IsSupported() const = 0;

	// Arms (or re-arms, replacing the previous delay and callback) for
	// `seconds` > 0. Returns false for seconds == 0, a null `fire`, or when
	// unsupported.
	virtual bool Arm( unsigned seconds, void ( *fire )( void *context ), void *context ) = 0;

	// Cancels a pending fire; harmless when not armed.
	virtual void Disarm() = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_DIAGNOSTICS_H
