//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for the process environment (RFC 0001
//			foundation capability "Process environment": arguments, environment
//			variables, process identifiers and debugger state).
//
//			Read-only by design: the provider takes a snapshot when the root
//			composes it, so lookups are stable and safe from any thread, and no
//			consumer mutates process-global state (setenv is not thread-safe on
//			POSIX). Command-line policy (what "-foo" means) stays with the
//			engine's command-line owner; this contract only reports the strings.
//			A native provider reads argv/environ or GetCommandLineW; mobile
//			providers report what their app container supplies.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_PROCESS_ENVIRONMENT_H
#define PLATFORM_CONTRACTS_PROCESS_ENVIRONMENT_H

// Contract header: standard library only. No tier0/tier1, no native SDK, no
// OS-selection macros. Must compile under linux-headless-core.
#include <cstdint>

namespace platform
{

enum class DebuggerState
{
	kUnknown = 0, // the platform cannot tell (sandboxed, unsupported)
	kNotAttached,
	kAttached,
};

// All strings are UTF-8. The Get* functions follow platform.paths.v1: write a
// NUL-terminated copy and return its length excluding the NUL, or return -1
// without writing anything when the item is absent, the buffer is null, the
// size is non-positive or the value plus NUL does not fit. The *Length
// functions return the length a Get* call needs (excluding the NUL), or -1
// when the item is absent.
class IProcessEnvironment
{
public:
	virtual ~IProcessEnvironment() = default;

	// Number of arguments, including the program name at index 0 when the
	// platform supplies one. Never negative.
	virtual int ArgumentCount() const = 0;
	virtual int ArgumentLength( int index ) const = 0;
	virtual int GetArgument( int index, char *buffer, int bufferSize ) const = 0;

	// Variable names are case-sensitive on every provider, so portable code
	// gets the same answer everywhere. A null or empty name, or one containing
	// '=', is absent. A variable set to the empty string is present with length 0.
	virtual int VariableLength( const char *name ) const = 0;
	virtual int GetVariable( const char *name, char *buffer, int bufferSize ) const = 0;

	// This process's id: nonzero and stable for the process lifetime.
	virtual std::uint64_t ProcessId() const = 0;

	// Whether a debugger is attached now. May change between calls.
	virtual DebuggerState GetDebuggerState() const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_PROCESS_ENVIRONMENT_H
