//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for physical threads (RFC 0001 foundation
//			capability "Threading": thread creation, identifiers, naming,
//			priority and sleep).
//
//			This is the layer beneath platform.task-runner.v1. Feature code posts
//			work to runners and never starts threads; only the providers that own
//			a pool or an API with real thread affinity (an executor's workers, an
//			audio callback thread, the render sequence) start threads here. A
//			native provider wraps the OS thread API; the deterministic test
//			provider runs each entry when it is joined, on a virtual thread id.
//
//			Synchronization primitives (mutexes, condition variables, atomics)
//			stay compile-time std:: types; they do not need runtime substitution.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_THREAD_H
#define PLATFORM_CONTRACTS_THREAD_H

// Contract header: standard library only. No tier0/tier1, no native SDK, no
// OS-selection macros. Must compile under linux-headless-core.
#include <cstddef>
#include <cstdint>

namespace platform
{

// Identifies a thread for the provider's lifetime. 0 is never a valid id.
struct ThreadId
{
	std::uint64_t value = 0;
};

// A started, not yet joined thread. 0 is never a valid handle.
struct ThreadHandle
{
	std::uint64_t value = 0;
};

// Relative scheduling priority. A provider maps these onto what the platform
// allows, or reports kUnsupported; it never needs elevated privileges.
enum class ThreadPriority
{
	kLow = 0,
	kNormal,
	kHigh,
};

enum class ThreadResult
{
	kOk = 0,
	kInvalidArgument,   // null entry, unknown or already-joined handle, joining self
	kUnsupported,       // the platform does not offer this optional behavior
	kResourceExhausted, // the platform refused to create another thread
};

// The longest name, in bytes excluding the NUL, every provider keeps intact.
// Longer names are truncated to this many bytes (Linux's limit is 15).
constexpr int kThreadNameMaxBytes = 15;

using ThreadEntry = void ( * )( void *context );

struct ThreadOptions
{
	const char *name = nullptr; // optional; applied before `entry` runs
	ThreadPriority priority = ThreadPriority::kNormal;
	std::size_t stackBytes = 0; // 0 = the platform default; otherwise a minimum
};

class IThreads
{
public:
	virtual ~IThreads() = default;

	// Starts a thread that calls entry( context ) exactly once. On kOk, `out`
	// holds a handle that must be joined exactly once before the provider is
	// destroyed. On failure `out` is unchanged and `entry` never runs. The
	// options' name, when set, is the thread's name when `entry` starts; an
	// unsupported priority is not a failure.
	virtual ThreadResult Start(
	    const ThreadOptions &options, ThreadEntry entry, void *context, ThreadHandle &out ) = 0;

	// Waits until the thread's entry has returned and releases the handle.
	// Everything the entry wrote happens-before Join returns. Joining an unknown
	// or already-joined handle, or a thread joining itself, is kInvalidArgument.
	virtual ThreadResult Join( ThreadHandle thread ) = 0;

	// The calling thread's id: nonzero, stable for that thread, and distinct
	// from every other live thread's id.
	virtual ThreadId CurrentId() const = 0;

	// The id the started thread sees from CurrentId(). Zero for an unknown handle.
	virtual ThreadId IdOf( ThreadHandle thread ) const = 0;

	// Names the calling thread for debuggers and profilers (UTF-8, truncated to
	// kThreadNameMaxBytes). kUnsupported when the platform has no thread names.
	virtual ThreadResult SetCurrentName( const char *name ) = 0;

	// Writes the calling thread's name (NUL-terminated) and returns its length,
	// or -1 without writing when names are unsupported, no name was set, the
	// buffer is null or too small.
	virtual int GetCurrentName( char *buffer, int bufferSize ) const = 0;

	// kOk or kUnsupported. Never fails for lack of privilege.
	virtual ThreadResult SetCurrentPriority( ThreadPriority priority ) = 0;

	// Blocks the calling thread for at least `nanoseconds` of the provider's
	// monotonic clock. Sleep(0) yields.
	virtual void SleepFor( std::uint64_t nanoseconds ) = 0;

	// Hardware threads available to this process; at least 1.
	virtual unsigned HardwareConcurrency() const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_THREAD_H
