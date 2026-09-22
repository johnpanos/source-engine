//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portable child-process contract for command-line tools.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_TOOL_PROCESS_H
#define PLATFORM_CONTRACTS_TOOL_PROCESS_H

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace platform
{

// One explicit change to the child's inherited environment. A missing value
// removes the variable; an engaged value sets it exactly, including an empty
// string. Names are unique within one request.
struct ToolProcessEnvironmentOverride
{
	std::string name;
	std::optional<std::string> value;

	bool operator==( const ToolProcessEnvironmentOverride &other ) const = default;
};

// Cancellation is borrowed for Run's duration only. Implementations may query
// it from a provider-owned worker thread, so implementations must make the
// observation thread-safe.
class IToolProcessCancellation
{
public:
	virtual ~IToolProcessCancellation() = default;
	virtual bool IsCancellationRequested() const noexcept = 0;
};

struct ToolProcessRequest
{
	// argv[0] is the executable. Every element is passed as one exact argument;
	// providers must never flatten this vector into an unescaped command string.
	std::vector<std::string> argv;
	std::string workingDirectory;
	std::vector<ToolProcessEnvironmentOverride> environment;
	// Execution timeout starts termination. Cancellation timeout is the clean
	// shutdown grace period before forced termination; neither permits a return
	// before the process tree and provider work have been reaped.
	std::chrono::milliseconds executionTimeout{ 0 };
	std::chrono::milliseconds cancellationTimeout{ 0 };
	const IToolProcessCancellation *cancellation = nullptr;
};

enum class ToolProcessCompletion
{
	kExited = 0,
	kInvalidRequest,
	kSpawnFailed,
	kTimedOut,
	kCanceled,
	kProviderFailure,
};

enum class ToolProcessErrorCode
{
	kNone = 0,
	kInvalidArgument,
	kExecutableNotFound,
	kPermissionDenied,
	kResourceExhausted,
	kBusy,
	kCleanupFailed,
	kProviderFailure,
};

struct ToolProcessError
{
	ToolProcessErrorCode code = ToolProcessErrorCode::kNone;
	int providerCode = 0;
	std::string requestedExecutable;
	std::string detail;

	[[nodiscard]] bool IsOk() const noexcept { return code == ToolProcessErrorCode::kNone; }
};

// A complete, owning record of one invocation. stdoutData and stderrData stay
// separate and may contain arbitrary bytes. exitCode is present only for
// kExited. Timeout/cancel return only after cancellation is acknowledged and
// the child process tree is gone. cleanupComplete is required on every return,
// including validation and spawn failures.
struct ToolProcessResult
{
	ToolProcessCompletion completion = ToolProcessCompletion::kProviderFailure;
	std::optional<int> exitCode;
	std::string stdoutData;
	std::string stderrData;
	ToolProcessError error;
	bool cancellationAcknowledged = false;
	bool cleanupComplete = false;

	[[nodiscard]] bool Succeeded() const noexcept
	{
		return completion == ToolProcessCompletion::kExited && exitCode == 0 && error.IsOk() &&
		       cleanupComplete;
	}
};

// Synchronous process provider used by tool orchestrators. Run borrows request
// only for the call and must not mutate it, the parent's cwd, or the parent's
// environment. A valid request has a non-empty argv/argv[0] and working
// directory, positive timeouts, no embedded NUL, and unique non-empty
// environment names without '='. Environment entries override the inherited
// environment only for the child.
class IToolProcessProvider
{
public:
	virtual ~IToolProcessProvider() = default;

	[[nodiscard]] virtual ToolProcessResult Run( const ToolProcessRequest &request ) = 0;

	// Observable ownership state for composition shutdown and conformance. A
	// conforming synchronous Run always returns this to its pre-call value.
	[[nodiscard]] virtual int LiveProcessCount() const noexcept = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_TOOL_PROCESS_H
